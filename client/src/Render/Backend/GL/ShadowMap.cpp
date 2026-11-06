// Sin PCH: GL moderno por glad.
#include "Platform/GL/glad.h"
#include "Platform/GLFunctions.h"
#include "Render/Backend/GL/ShadowMap.h"
#include "Render/Backend/GfxDevice.h"
#include "Render/BufferedDraw.h"
#include "Render/RenderState.h"
#include "Render/ShadowLight.h"
#include <cstdio>
#include <cstring>
#include <vector>

extern "C" void DbgLogPublic(const char*);

namespace ShadowMapGL {
namespace {
struct Batch { GLuint texture; bool textured; std::size_t first, count; };
bool scene = false, casting = true, failed = false;
float strength = 0.5f;
// Niebla por profundidad: modo (0 nada, 1 bruma, 2 agua), color, densidad, inicio.
int fogMode = 0;
float fogColor[3]{}, fogDensity = 0.0f, fogStart = 0.0f;
float inverseView[16], inverseViewProj[16], light[16];
int viewport[4];
std::vector<CasterVertex> casters;
std::vector<Batch> batches;
// Matriz al mundo del modelView actual (se rehace sólo si cambió).
unsigned long long matrixRevision = ~0ull;
float toWorld[16];

const char* const CasterVertexSource = R"GLSL(#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aUV;
uniform mat4 uLight;
out vec2 vUV;
void main() {
    vec4 p = uLight * vec4(aPosition, 1.0);
    gl_Position = vec4(p.xy, p.z * 2.0 - 1.0, 1.0);   // la luz da z de 0 a 1
    vUV = aUV;
}
)GLSL";
// Lo transparente de la textura (hojas, rejas) no proyecta.
const char* const CasterFragmentSource = R"GLSL(#version 330 core
uniform sampler2D uTexture;
uniform bool uTextured;
in vec2 vUV;
void main() { if (uTextured && texture(uTexture, vUV).a < 0.25) discard; }
)GLSL";
const char* const ApplyVertexSource = R"GLSL(#version 330 core
void main(){ vec2 p=vec2(gl_VertexID==1?3.0:-1.0,gl_VertexID==2?3.0:-1.0); gl_Position=vec4(p,0.0,1.0); }
)GLSL";
// Reconstruye la posición de cada píxel desde la profundidad y devuelve el
// factor de luz (se multiplica sobre la imagen). params = intensidad, texel, sesgo.
const char* const ApplyFragmentSource = R"GLSL(#version 330 core
uniform sampler2D uDepth;
uniform sampler2DShadow uShadow;
uniform mat4 uInverseViewProj, uLight;
uniform vec4 uViewport;
uniform vec3 uParams;
out vec4 color;
void main() {
    float d = texelFetch(uDepth, ivec2(gl_FragCoord.xy), 0).r;
    if (d >= 1.0) { color = vec4(1.0); return; }
    vec2 ndc = (gl_FragCoord.xy - uViewport.xy) / uViewport.zw * 2.0 - 1.0;
    vec4 w = uInverseViewProj * vec4(ndc, d * 2.0 - 1.0, 1.0);
    vec4 lp = uLight * vec4(w.xyz / w.w, 1.0);
    vec2 uv = lp.xy * 0.5 + 0.5;
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))) || lp.z > 1.0) { color = vec4(1.0); return; }
    float lit = 0.0;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            lit += texture(uShadow, vec3(uv + vec2(x, y) * uParams.y, lp.z - uParams.z));
    lit /= 9.0;
    float edge = smoothstep(0.0, 0.1, min(min(uv.x, uv.y), min(1.0 - uv.x, 1.0 - uv.y)));
    color = vec4(vec3(mix(1.0, mix(1.0 - uParams.x, 1.0, lit), edge)), 1.0);
}
)GLSL";

// Niebla: misma reconstrucción; uPass 0 multiplica por la transmitancia y 1
// suma el color (mix sin copiar la imagen).
const char* const FogFragmentSource = R"GLSL(#version 330 core
uniform sampler2D uDepth;
uniform mat4 uInverseViewProj;
uniform vec4 uViewport;
uniform vec3 uCamera, uColor, uParams;   // params = modo, densidad, inicio
uniform int uPass;
out vec4 color;
void main() {
    float d = texelFetch(uDepth, ivec2(gl_FragCoord.xy), 0).r;
    vec3 t = vec3(0.0);   // fondo: todo niebla
    if (d < 1.0) {
        vec2 ndc = (gl_FragCoord.xy - uViewport.xy) / uViewport.zw * 2.0 - 1.0;
        vec4 w = uInverseViewProj * vec4(ndc, d * 2.0 - 1.0, 1.0);
        vec3 world = w.xyz / w.w;
        float distance = max(length(world - uCamera) - uParams.z, 0.0);
        if (uParams.x > 1.5) t = exp(-uParams.y * vec3(3.0, 1.8, 1.2) * distance);   // agua: el rojo primero
        else { float x = distance * uParams.y; t = vec3(exp(-x * x)); }
        // El borde del mapa (256 tiles de 100) se cierra con niebla.
        float border = min(min(world.x, world.y), min(25600.0 - world.x, 25600.0 - world.y));
        // Sólo las últimas 3 casillas y lejos del personaje.
        t *= 1.0 - (1.0 - smoothstep(0.0, 300.0, border)) * smoothstep(0.0, 1000.0, distance);
    }
    color = uPass == 0 ? vec4(t, 1.0) : vec4(uColor * (vec3(1.0) - t), 0.0);
}
)GLSL";

struct Programs {
    GLuint caster = 0, apply = 0, fog = 0;
    GLint casterLight = -1, casterTexture = -1, casterTextured = -1;
    GLint depth = -1, shadow = -1, inverseViewProj = -1, light = -1, viewport = -1, params = -1;
    GLint fogDepth = -1, fogInverseViewProj = -1, fogViewport = -1, fogCamera = -1, fogColor = -1, fogParams = -1, fogPass = -1;
} programs;
GLuint casterArray = 0, casterBuffer = 0, applyArray = 0;
GLuint shadowTexture = 0, shadowFramebuffer = 0;
GLuint depthTexture = 0, depthFramebuffer = 0;
int depthWidth = 0, depthHeight = 0;
GLenum depthFormat = 0;

GLuint Compile(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr); glCompileShader(shader);
    GLint ok = 0; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok) return shader;
    char log[1024]{}; glGetShaderInfoLog(shader, sizeof(log), nullptr, log); DbgLogPublic(log);
    glDeleteShader(shader); return 0;
}
GLuint Link(const char* vertex, const char* fragment) {
    const GLuint v = Compile(GL_VERTEX_SHADER, vertex), f = Compile(GL_FRAGMENT_SHADER, fragment);
    GLuint name = 0;
    if (v && f) {
        name = glCreateProgram(); glAttachShader(name, v); glAttachShader(name, f);
        glBindAttribLocation(name, 0, "aPosition"); glBindAttribLocation(name, 1, "aUV");
        glLinkProgram(name);
        GLint ok = 0; glGetProgramiv(name, GL_LINK_STATUS, &ok);
        if (!ok) { char log[1024]{}; glGetProgramInfoLog(name, sizeof(log), nullptr, log); DbgLogPublic(log); glDeleteProgram(name); name = 0; }
    }
    if (v) glDeleteShader(v);
    if (f) glDeleteShader(f);
    return name;
}
bool Ensure() {
    if (programs.caster) return true;
    if (failed || !Platform::GL33Ready()) return false;
    programs.caster = Link(CasterVertexSource, CasterFragmentSource);
    programs.apply = Link(ApplyVertexSource, ApplyFragmentSource);
    programs.fog = Link(ApplyVertexSource, FogFragmentSource);
    if (!programs.caster || !programs.apply || !programs.fog) { failed = true; DbgLogPublic("Sombras (OpenGL): no se pudieron compilar los shaders"); return false; }
    programs.casterLight = glGetUniformLocation(programs.caster, "uLight");
    programs.casterTexture = glGetUniformLocation(programs.caster, "uTexture");
    programs.casterTextured = glGetUniformLocation(programs.caster, "uTextured");
    programs.depth = glGetUniformLocation(programs.apply, "uDepth");
    programs.shadow = glGetUniformLocation(programs.apply, "uShadow");
    programs.inverseViewProj = glGetUniformLocation(programs.apply, "uInverseViewProj");
    programs.light = glGetUniformLocation(programs.apply, "uLight");
    programs.viewport = glGetUniformLocation(programs.apply, "uViewport");
    programs.params = glGetUniformLocation(programs.apply, "uParams");
    programs.fogDepth = glGetUniformLocation(programs.fog, "uDepth");
    programs.fogInverseViewProj = glGetUniformLocation(programs.fog, "uInverseViewProj");
    programs.fogViewport = glGetUniformLocation(programs.fog, "uViewport");
    programs.fogCamera = glGetUniformLocation(programs.fog, "uCamera");
    programs.fogColor = glGetUniformLocation(programs.fog, "uColor");
    programs.fogParams = glGetUniformLocation(programs.fog, "uParams");
    programs.fogPass = glGetUniformLocation(programs.fog, "uPass");

    glGenVertexArrays(1, &casterArray); glGenBuffers(1, &casterBuffer); glGenVertexArrays(1, &applyArray);
    glBindVertexArray(casterArray); glBindBuffer(GL_ARRAY_BUFFER, casterBuffer);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CasterVertex), (void*)offsetof(CasterVertex, position));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CasterVertex), (void*)offsetof(CasterVertex, uv));
    glBindVertexArray(0);

    glGenTextures(1, &shadowTexture); glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, ShadowLight::Size, ShadowLight::Size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glGenFramebuffers(1, &shadowFramebuffer); glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!complete) { failed = true; DbgLogPublic("Sombras (OpenGL): mapa de sombras incompleto"); return false; }
    return true;
}
// Textura de profundidad con el formato del framebuffer de la ventana (el
// blit de profundidad exige el mismo formato).
bool EnsureDepth(int width, int height) {
    if (!depthFormat) {
        GLint depthBits = 0, stencilBits = 0;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencilBits);
        depthFormat = stencilBits ? GL_DEPTH24_STENCIL8 : depthBits >= 32 ? GL_DEPTH_COMPONENT32 : depthBits >= 24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16;
        char line[96]; std::snprintf(line, sizeof(line), "Sombras (OpenGL): profundidad de la ventana %d bits, stencil %d", depthBits, stencilBits);
        DbgLogPublic(line);
    }
    if (depthTexture && depthWidth == width && depthHeight == height) return true;
    if (!depthTexture) { glGenTextures(1, &depthTexture); glGenFramebuffers(1, &depthFramebuffer); }
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    const bool stencil = depthFormat == GL_DEPTH24_STENCIL8;
    glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, width, height, 0, stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT,
                 stencil ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!complete) { failed = true; DbgLogPublic("Sombras (OpenGL): copia de profundidad incompleta"); return false; }
    depthWidth = width; depthHeight = height;
    return true;
}

void Render() {
    const bool shadows = strength > 0.0f && !casters.empty();
    if ((!shadows && !fogMode) || !Ensure()) return;
    const int vx = viewport[0], vy = viewport[1], vw = viewport[2], vh = viewport[3];
    if (vw <= 0 || vh <= 0) return;
    BufferedDraw::Suspend();
    // Estado que se toca y se restaura.
    GLint program = 0, array = 0, arrayBuffer = 0, framebuffer = 0, active = 0, sampler = 0, view[4]{};
    GLint texture0 = 0, texture2 = 0, texture3 = 0, blendSource = 0, blendDestination = 0, depthFunction = GL_LESS;
    GLboolean depthMask = GL_TRUE, colorMask[4]{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &program); glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &array);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer); glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_VIEWPORT, view); glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glGetIntegerv(GL_BLEND_SRC_RGB, &blendSource); glGetIntegerv(GL_BLEND_DST_RGB, &blendDestination);
    GLint blendSourceAlpha = 0, blendDestinationAlpha = 0;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSourceAlpha); glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDestinationAlpha);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask); glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
    glActiveTexture(GL_TEXTURE0); glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture0); glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
    glActiveTexture(GL_TEXTURE2); glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2);
    glActiveTexture(GL_TEXTURE3); glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture3);
    const GLboolean blend = glIsEnabled(GL_BLEND), depthTest = glIsEnabled(GL_DEPTH_TEST), cull = glIsEnabled(GL_CULL_FACE),
                    scissor = glIsEnabled(GL_SCISSOR_TEST), offset = glIsEnabled(GL_POLYGON_OFFSET_FILL);
    GLfloat offsetFactor = 0, offsetUnits = 0;
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &offsetFactor); glGetFloatv(GL_POLYGON_OFFSET_UNITS, &offsetUnits);

    // 1. Mapa de sombras: la geometría guardada, vista desde la luz.
    if (shadows) {
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glViewport(0, 0, ShadowLight::Size, ShadowLight::Size);
    glDisable(GL_BLEND); glDisable(GL_CULL_FACE); glDisable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glDepthMask(GL_TRUE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(2.0f, 4.0f);   // evita que se sombree a sí misma
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(programs.caster);
    glUniformMatrix4fv(programs.casterLight, 1, GL_FALSE, light);
    glUniform1i(programs.casterTexture, 0);
    glActiveTexture(GL_TEXTURE0); glBindSampler(0, 0);
    glBindVertexArray(casterArray); glBindBuffer(GL_ARRAY_BUFFER, casterBuffer);
    glBufferData(GL_ARRAY_BUFFER, casters.size() * sizeof(CasterVertex), casters.data(), GL_STREAM_DRAW);
    int textured = -1;
    for (const Batch& b : batches) {
        if (int(b.textured) != textured) { glUniform1i(programs.casterTextured, b.textured ? 1 : 0); textured = b.textured; }
        if (b.textured) glBindTexture(GL_TEXTURE_2D, b.texture);
        glDrawArrays(GL_TRIANGLES, GLint(b.first), GLsizei(b.count));
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
    }

    // 2. Profundidad de la escena: copia del framebuffer de la ventana.
    bool applied = false;
    if (EnsureDepth(vx + vw, vy + vh)) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, GLuint(framebuffer));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthFramebuffer);
        while (glGetError() != GL_NO_ERROR) {}
        glBlitFramebuffer(vx, vy, vx + vw, vy + vh, vx, vy, vx + vw, vy + vh, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        if (glGetError() == GL_NO_ERROR) {
            glBindFramebuffer(GL_FRAMEBUFFER, GLuint(framebuffer));
            glViewport(vx, vy, vw, vh);
            glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, depthTexture);
            glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, shadowTexture);
            glBindVertexArray(applyArray);
            if (shadows) {
                // 3. Pase de pantalla completa: multiplica la imagen por la luz.
                glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                glUseProgram(programs.apply);
                glUniform1i(programs.depth, 2); glUniform1i(programs.shadow, 3);
                glUniformMatrix4fv(programs.inverseViewProj, 1, GL_FALSE, inverseViewProj);
                glUniformMatrix4fv(programs.light, 1, GL_FALSE, light);
                glUniform4f(programs.viewport, float(vx), float(vy), float(vw), float(vh));
                glUniform3f(programs.params, strength, 1.0f / float(ShadowLight::Size), ShadowLight::Bias);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            if (fogMode) {
                // 4. Niebla: multiplicar por la transmitancia y sumar el color.
                glUseProgram(programs.fog);
                glUniform1i(programs.fogDepth, 2);
                glUniformMatrix4fv(programs.fogInverseViewProj, 1, GL_FALSE, inverseViewProj);
                glUniform4f(programs.fogViewport, float(vx), float(vy), float(vw), float(vh));
                glUniform3f(programs.fogCamera, inverseView[12], inverseView[13], inverseView[14]);
                glUniform3fv(programs.fogColor, 1, fogColor);
                glUniform3f(programs.fogParams, float(fogMode), fogDensity, fogStart);
                glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
                glUniform1i(programs.fogPass, 0); glDrawArrays(GL_TRIANGLES, 0, 3);
                glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
                glUniform1i(programs.fogPass, 1); glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            applied = true;
        } else {
            static bool logged = false;
            if (!logged) { logged = true; DbgLogPublic("Sombras (OpenGL): no se pudo copiar la profundidad de la ventana"); }
        }
    }
    (void)applied;

    glBindFramebuffer(GL_FRAMEBUFFER, GLuint(framebuffer));
    glViewport(view[0], view[1], view[2], view[3]);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, GLuint(texture3));
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, GLuint(texture2));
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, GLuint(texture0)); glBindSampler(0, GLuint(sampler));
    glActiveTexture(GLenum(active));
    glBindVertexArray(GLuint(array)); glBindBuffer(GL_ARRAY_BUFFER, GLuint(arrayBuffer));
    glUseProgram(GLuint(program));
    glBlendFuncSeparate(GLenum(blendSource), GLenum(blendDestination), GLenum(blendSourceAlpha), GLenum(blendDestinationAlpha));
    glDepthMask(depthMask);
    glDepthFunc(GLenum(depthFunction));
    blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
    depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    cull ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    scissor ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
    offset ? glEnable(GL_POLYGON_OFFSET_FILL) : glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(offsetFactor, offsetUnits);
    BufferedDraw::Suspend();
}
}

bool Capturing() {
    if (!scene || !casting || strength <= 0.0f) return false;
    const auto& s = RenderState::Get();
    const bool additive = s.caps[RenderState::CapIndex(Gfx::Cap::Blend)] && s.blendDestination == Gfx::Factor::One;
    return s.caps[RenderState::CapIndex(Gfx::Cap::DepthTest)] && s.depthWrite && !additive;
}
void Capture(const CasterVertex* triangles, std::size_t count) {
    if (!count) return;
    if (matrixRevision != RenderState::MatrixRevision()) {
        ShadowLight::Multiply(inverseView, RenderState::Top(Gfx::Matrix::ModelView), toWorld);
        matrixRevision = RenderState::MatrixRevision();
    }
    const auto& s = RenderState::Get();
    const bool textured = s.caps[RenderState::CapIndex(Gfx::Cap::Texture2D)];
    const GLuint texture = textured ? GLuint(s.texture) : 0u;
    const std::size_t first = casters.size();
    casters.resize(first + count);
    const float* m = toWorld;
    for (std::size_t i = 0; i < count; ++i) {
        const float* p = triangles[i].position;
        CasterVertex& v = casters[first + i];
        v.position[0] = m[0] * p[0] + m[4] * p[1] + m[8] * p[2] + m[12];
        v.position[1] = m[1] * p[0] + m[5] * p[1] + m[9] * p[2] + m[13];
        v.position[2] = m[2] * p[0] + m[6] * p[1] + m[10] * p[2] + m[14];
        v.uv[0] = triangles[i].uv[0]; v.uv[1] = triangles[i].uv[1];
    }
    if (!batches.empty() && batches.back().texture == texture && batches.back().textured == textured &&
        batches.back().first + batches.back().count == first) batches.back().count += count;
    else batches.push_back({texture, textured, first, count});
}
void CaptureImmediate(unsigned mode, const BufferedDraw::Vertex* vertices, std::size_t count) {
    // Triángulos, strips, abanicos y quads (numeración de OpenGL 1.1).
    static std::vector<CasterVertex> triangles;
    triangles.clear();
    auto corner = [&](std::size_t i) {
        const auto& v = vertices[i];
        triangles.push_back({{v.position[0], v.position[1], v.position[2]}, {v.uv[0] / (v.uv[3] ? v.uv[3] : 1.0f), v.uv[1] / (v.uv[3] ? v.uv[3] : 1.0f)}});
    };
    // Lo que se dibuja semitransparente por su color no proyecta.
    for (std::size_t i = 0; i < count; ++i) if (vertices[i].color[3] < 0.99f) return;
    switch (mode) {
    case 4: for (std::size_t i = 0; i + 2 < count; i += 3) { corner(i); corner(i + 1); corner(i + 2); } break;
    case 5: for (std::size_t i = 0; i + 2 < count; ++i) { corner(i); corner(i + 1); corner(i + 2); } break;
    case 6: for (std::size_t i = 1; i + 1 < count; ++i) { corner(0); corner(i); corner(i + 1); } break;
    case 7: for (std::size_t i = 0; i + 3 < count; i += 4) { corner(i); corner(i + 1); corner(i + 2); corner(i); corner(i + 2); corner(i + 3); } break;
    default: return;
    }
    Capture(triangles.data(), triangles.size());
}
}

namespace GfxDevice::GL {
bool SupportsShadowMaps() { return !ShadowMapGL::failed && Platform::GL33Ready(); }
void ShadowScene(bool begin, const float* focus, float shadowStrength) {
    using namespace ShadowMapGL;
    if (begin) {
        scene = false;
        if (!focus || !ShadowLight::Invert(RenderState::Top(Gfx::Matrix::ModelView), inverseView)) return;
        float viewProj[16];
        ShadowLight::Multiply(RenderState::Top(Gfx::Matrix::Projection), RenderState::Top(Gfx::Matrix::ModelView), viewProj);
        if (!ShadowLight::Invert(viewProj, inverseViewProj)) return;
        ShadowLight::ViewProj(focus, light);
        std::memcpy(viewport, RenderState::Get().viewport, sizeof(viewport));
        strength = shadowStrength;
        fogMode = 0;
        casters.clear(); batches.clear();
        matrixRevision = ~0ull;
        scene = true; casting = true;
        return;
    }
    if (!scene) return;
    scene = false;
    Render();
}
void ShadowCasting(bool enabled) { ShadowMapGL::casting = enabled; }
void Atmosphere(int mode, const float* color, float density, float start) {
    using namespace ShadowMapGL;
    if (!scene) return;
    fogMode = mode; fogDensity = density; fogStart = start;
    if (color) std::memcpy(fogColor, color, sizeof(fogColor));
}
}
