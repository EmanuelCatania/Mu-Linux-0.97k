// BMD_DrawMesh.cpp
// FUN_00440d50 @ 0x00440D50  [Kayito: unnamed; S6 equivalent = BMD::RenderMesh]
//
// Draws one mesh slot of a BMD (Bone Mesh Data) model using OpenGL.
// Called __thiscall in binary; reconstructed as __cdecl with explicit bmd_obj.
//
// Parameters (matched to S6 BMD::RenderMesh):
//   bmd_obj   = BMD object pointer (ECX in original)
//   meshIdx   = mesh/bone-slot index (param_1 as float→int)
//   flags     = render mode bitmask (param_2)
//   alpha     = transparency (param_3)
//   blendMesh = blend-mesh index or -1.0f (param_4); reused as mode-tag internally
//   blendLight= blend mesh luminosity (param_5)
//   uvU       = blend mesh texcoord U offset (param_6)
//   uvV       = blend mesh texcoord V offset (param_7)
//   texOverride= texture slot override (param_8, 0xffffffff = use mesh default)
//
// BMD object offsets:
//   +0x28 = Meshs array ptr  (Mesh_t stride 0x28)
//   +0x38 = IndexTexture ptr (texture lookup table)
//   +0x44 = LightEnable (char)
//   +0x48..0x50 = BodyLight[3] (RGB floats)
//   +0x88 = StreamMesh index (char)
//   +0x98 = Skin index (char)
//   +0x99 = HideSkin flag (char)
//
// Mesh_t offsets (stride 0x28):
//   +0x00 = flags/active (char)
//   +0x02 = Texture index (short) → IndexTexture lookup
//   +0x06 = NumNormals (short)
//   +0x0a = NumTriangles (short)
//   +0x18 = TexCoords ptr  (UV array, float[2] per entry)
//   +0x1c = Triangles ptr  (Triangle_t array, stride 0x24)
//   +0x24 = SubMesh ptr (optional, checked for NULL)
//
// Triangle_t offsets (stride 0x24):
//   +0x00 = vertex count (char)
//   +0x0a = first vertex record (shorts)
//       vertex[i][-4] = bone/vertex position index (sVar3)
//       vertex[i][0]  = light vertex index
//       vertex[i][4]  = texcoord index
//
// Global buffers:
//   DAT_077e298c = IntensityTransform[meshIdx * 15000] (float per normal)
//   DAT_060db65c = LightTransform RGB [vertex * 12 bytes = float R,G,B]
//   DAT_0584621c = BoneVertex world positions [vertex * 3 floats]
//   DAT_05828d5c = Chrome UV array [packed: U0,V0,U1,V1,...] (floats)
//   DAT_0839bc8c = WaterTextureNumber (DWORD)
//   DAT_083a7cc8 = Texture table (stride 0x38, byte[0] = bpp/type)
//
// Texture special values:
//   0x12c (300) = BITMAP_HIDE → skip mesh
//   0x12d (301) = BITMAP_SKIN → use BodySkin variant
//   0x041  (65) = BITMAP_WATER → use animated water texture
//
// Render flag bits:
//   0x400 = animated variant → delegate to FUN_004414d0
//   0x001 = RENDER_COLOR (no texture, flat color)
//     0x040 = RENDER_BRIGHT (additive blend)
//     0x080 = RENDER_DARK (subtractive blend)
//   0x004 = RENDER_CHROME (environment map, wavy V texcoords)
//   0x008 = RENDER_OBJ (some object variant)
//   0x200 = RENDER_OIL (wavy U+V chrome variant)
//   0x002 = RENDER_TEXTURE (explicit texture bind)
//   0x010 = RENDER_LIGHTMAP
//
// Mode-tag constants (param_4 reused as local int tag via float bits):
//   MODE_COLOR   = bits of 0x00000002 as float = 1.4013e-45
//   MODE_CHROME  = bits of 0x0000000a as float = 5.60519e-45
//   MODE_TEXTURE = bits of 0x00000004 as float = 2.8026e-45
//   MODE_METAL   = bits of 0x00000080 as float = 8.96831e-44

#include "stdafx.h"
#include "../globals.h"
#include "../functions.h"

// Additional globals used by this function
// DAT_060db65c defined via macro in globals.h (backed by g_BoneLightBuf).
// DAT_05828d5c / DAT_06f433bc / DAT_06f433c0 defined via macros in globals.h
// (chrome UV scratch + transformed-normal buffer). No re-declarar acá.
extern float   _DAT_00552868;  // 0.0001f  (1/10000 for WorldTime wave)
extern float   _DAT_00552544;  // alpha < this → use alpha channel
extern float   _DAT_00552580;  // 0.0f
extern float   _DAT_005528c0;  // chrome U scale
extern float   _DAT_005528b4;  // chrome offset
extern float   _DAT_00552504;  // chrome scale factor
extern float   _DAT_0055256c;  // chrome V offset
extern float   _DAT_005526e4;  // some chrome constant
extern float   _DAT_00552540;  // blend tex scale
extern float   _DAT_00552530;  // blend tex factor

// Render mode tags (see header comment)
#define RENDER_MODE_COLOR   1.4013e-45f
#define RENDER_MODE_TEXTURE 2.8026e-45f
#define RENDER_MODE_CHROME  5.60519e-45f
#define RENDER_MODE_METAL   8.96831e-44f

// GL state helpers — aliases to the actual FUN_ addresses
#define BindTexture(x)          GL_BindTextureSlot(x)
#define EnableAlphaBlend()      GL_SetBlendAdditive()
#define EnableAlphaBlendMinus() GL_SetBlendSrcAlpha()
#define EnableLightMap()        GL_EnableLightMap()
#define EnableAlphaTest(x)      GL_SetBlendSrcOver(x)
#define DisableAlphaBlend()     GL_ResetState()
#define DisableTexture(x)       GL_SetAlphaTest(x)
#define DisableDepthMask()      GL_DisableDepthWrites()

extern "C" { void DbgLogPublic(const char*); }
extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]


static int DecodeMeshIndex(float meshIdx)
{
    unsigned int bits = 0;
    memcpy(&bits, &meshIdx, sizeof(bits));
    if ((bits & 0x7f800000u) == 0 && bits != 0) {
        return (int)bits;
    }
    return (int)meshIdx;
}

void __cdecl FUN_00440d50(void *bmd_obj, float meshIdx, int flags,
                           float alpha, int blendMesh, float blendLight,
                           float uvU, float uvV, unsigned int texOverride)
{
    char *pcVar1;
    int iVar2;
    short sVar3;
    unsigned char bVar4;
    bool bVar5;
    unsigned int uVar6;
    unsigned int uVar7;
    float *pfVar8;
    float fVar9;
    float *pfVar10;
    float fVar11;
    int iVar12;
    char *pcVar13;
    short *psVar14;
    unsigned int local_10;
    int meshIndex = DecodeMeshIndex(meshIdx);
    // IDA sub_440D50: a5 (BlendMesh) es un ENTERO -- indice de malla o -1.
    // El port lo habia tipado float y convivian dos codificaciones incompatibles
    // (bit-pattern desde o+100 vs valor numerico en los literales -1.0f), asi que
    // `-1.0f` caia en `a5 <= -2` y disparaba blend aditivo donde el binario
    // texturaba normal.  Ahora es int, como en el binario.
    int blendMeshIndex = blendMesh;
    // v47 del decompile: modo de emision de coordenadas/color del bucle de vertices.
    enum { MODE_NONE = 0, MODE_COLOR = 1, MODE_TEXTURE = 2, MODE_CHROME = 4, MODE_METAL = 64 };
    int renderMode = MODE_NONE;
    uVar6 = flags;

    // [DIAG FORGE] entry-point RenderMesh — world pos del modelo en +0x6c/0x70/0x74
    if (bmd_obj) {
        char* __m = (char*)bmd_obj;
        DbgForge("RenderMesh", -1, -1, -1, -1, meshIndex, flags,
                 *(float *)(__m + 0x6c), *(float *)(__m + 0x70), *(float *)(__m + 0x74),
                 *(float *)(__m + 0x48), *(float *)(__m + 0x4c), *(float *)(__m + 0x50), alpha);
    }

    // ── DIAG: log mesh/flags/tex for first calls ────────────────────────
    static int s_dm_dbg = 0;
    int dm_diag = s_dm_dbg;
    // Raised 24→80 para capturar TODOS los meshes del primer frame de login:
    // sky(4)+ship(10)+wave(1)+shipA(10)+waveA(1)+shipB(10)+waveB(1)+mu(2)+sky4(1) ≈ 40
    bool should_log = (s_dm_dbg < 80);
    if (should_log) s_dm_dbg++;

    // ── DIAG one-shot: dump matrices + viewport + GL error al primer draw ─
    // Si los logos empujan triángulos pero no se ven, el culpable probablemente
    // es la cámara (proyección/modelview) o el viewport. Volcamos TODO una vez.
    static int s_mat_dbg = 0;
    if (s_mat_dbg == 0) {
        s_mat_dbg = 1;
        GLfloat mv[16], pr[16];
        GLint vp[4];
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);
        glGetFloatv(GL_PROJECTION_MATRIX, pr);
        glGetIntegerv(GL_VIEWPORT, vp);
        GLenum err = glGetError();
        GLboolean depthTest, cullFace, depthMask, blend, alphaTest, tex2d, lighting;
        glGetBooleanv(GL_DEPTH_TEST, &depthTest);
        glGetBooleanv(GL_CULL_FACE, &cullFace);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
        glGetBooleanv(GL_BLEND, &blend);
        glGetBooleanv(GL_ALPHA_TEST, &alphaTest);
        glGetBooleanv(GL_TEXTURE_2D, &tex2d);
        glGetBooleanv(GL_LIGHTING, &lighting);
        GLint frontFace, cullMode, depthFunc;
        glGetIntegerv(GL_FRONT_FACE, &frontFace);
        glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        GLfloat clearColor[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        char b[512];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "GL_STATE @first BMD draw: err=0x%x  viewport=(%d,%d,%dx%d)  clearColor=(%.2f,%.2f,%.2f,%.2f)",
            (unsigned)err, vp[0], vp[1], vp[2], vp[3],
            clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "GL_STATE: depthTest=%d depthMask=%d depthFunc=0x%x cull=%d cullMode=0x%x frontFace=0x%x blend=%d alphaTest=%d tex2d=%d light=%d",
            depthTest, depthMask, depthFunc, cullFace, cullMode, frontFace,
            blend, alphaTest, tex2d, lighting);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "PROJ row0: %.3f %.3f %.3f %.3f",
            pr[0], pr[4], pr[8], pr[12]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "PROJ row1: %.3f %.3f %.3f %.3f",
            pr[1], pr[5], pr[9], pr[13]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "PROJ row2: %.3f %.3f %.3f %.3f",
            pr[2], pr[6], pr[10], pr[14]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "PROJ row3: %.3f %.3f %.3f %.3f",
            pr[3], pr[7], pr[11], pr[15]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "MV   row0: %.3f %.3f %.3f %.3f",
            mv[0], mv[4], mv[8], mv[12]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "MV   row1: %.3f %.3f %.3f %.3f",
            mv[1], mv[5], mv[9], mv[13]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "MV   row2: %.3f %.3f %.3f %.3f",
            mv[2], mv[6], mv[10], mv[14]);
        DbgLogPublic(b);
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "MV   row3: %.3f %.3f %.3f %.3f",
            mv[3], mv[7], mv[11], mv[15]);
        DbgLogPublic(b);
    }
    // Get mesh slot pointer: Meshs[meshIdx] (stride 0x28)
    pcVar1 = (char *)(*(int *)((int)bmd_obj + 0x28) + meshIndex * 0x28);

    // Early-out: NumTriangles == 0
    if (*(short *)(*(int *)((int)bmd_obj + 0x28) + 10 + meshIndex * 0x28) == 0) {
        return;
    }

    // Animated variant: delegate to BMD_DrawBoneSlot_Anim
    if ((flags & 0x400) != 0) {
        // El indice de malla llega como BIT-PATTERN (denormal): los callers usan
        // 1.4013e-45f para la malla 1, y FUN_004414d0 lo consume con `(int)frame`
        // -> (int)1.4013e-45f == 0, o sea dibujaba SIEMPRE la malla 0.
        // Se le pasa el indice ya decodificado. Sintoma: Queen Rainer (ModelID
        // 321) oculta su malla 1 en el pase principal y la redibuja por aca; al
        // dibujarse la 0 en su lugar, el vestido no aparecia nunca.
        FUN_004414d0(bmd_obj, '\x01', 0, (float)meshIndex, flags, alpha, blendMesh, uvU, uvV, blendLight, texOverride);
        return;
    }

    // WorldTime wave value (for animated texcoords)
    // ── BUG-FIX 2026-07-15: `__ftol()` leía el tope de la pila x87 (ST0), NO
    // WorldTime — el C no garantiza que WorldTime esté cargado ahí → fase BASURA.
    // IDA sub_440D50: `v42 = (__int64)WorldTime % 10000 * 0.0001`. Con la fase
    // basura, el scroll de texcoords del glow +N (chrome, textura 1170) mapeaba
    // coords random → el glow salía sólido/dorado que NO seguía la forma de la
    // malla (la espada del BK se veía como un recuadro/flama dorada) y con fase
    // distinta por-parte cada frame → las piezas del set +11 brillaban
    // desincronizadas. WorldTime = DAT_05826e08 (la misma que usan las
    // animaciones sin de char-select).
    long long worldTime = (long long)DAT_05826e08;
    float fVar16 = (float)(int)(worldTime % 10000) * _DAT_00552868;

    // Texture lookup: local_10 = IndexTexture[Mesh.Texture]
    local_10 = (unsigned int)*(short *)(*(int *)((int)bmd_obj + 0x38) + *(short *)(pcVar1 + 2) * 2);

    if (local_10 == 300) {  // BITMAP_HIDE
        return;
    }
    if (local_10 == 0x12d) {  // BITMAP_SKIN
        if (*(char *)((int)bmd_obj + 0x99) != '\0') {  // HideSkin
            return;
        }
        local_10 = (int)*(char *)((int)bmd_obj + 0x98) + 0x12d;  // BITMAP_SKIN + Skin
    } else if (local_10 == 0x41) {  // BITMAP_WATER
        local_10 = DAT_0839bc8c + 0x41;  // BITMAP_WATER + WaterTextureNumber
    }

    // Override texture if specified
    if (texOverride != 0xffffffff)
        local_10 = texOverride;

    bVar5 = false;
    // StreamMesh index
    int streamMeshIndex = (int)*(char *)((int)bmd_obj + 0x88);  // StreamMesh
    int waveMeshIndex = streamMeshIndex;

    // Check blend sub-mesh activation
    if ((*(int *)(pcVar1 + 0x24) != 0) && (*(char *)(*(int *)(pcVar1 + 0x24) + 2) != '\0'))
        waveMeshIndex = meshIndex;

    // Enable blend wave if both conditions
    if (((meshIndex == blendMeshIndex) || (meshIndex == waveMeshIndex)) &&
        ((uvU != _DAT_00552580 || (uvV != _DAT_00552580))))
        bVar5 = true;

    // Lighting setup
    unsigned char param_2_b0 = *(char *)((int)bmd_obj + 0x44);  // LightEnable

    // ── BRIGHTNESS BOOST ──────────────────────────────────────────────────────
    // El port se ve oscuro comparado con el armed. Sin GL_LIGHTING activo y
    // con bodyLight bajo (p.ej. 0.20,0.20,0.20) el texel × glColor pinta casi
    // negro. Aplicamos un boost CLAMPEADO a 1.0: los objetos ya brillantes
    // (bodyLight=(1,1,1) — sky, banner, wave) siguen pintando color × 1.0
    // (evita saturación a blanco); los oscuros (ships, chars) se iluminan.
    if (meshIndex == streamMeshIndex) {
        // StreamMesh: flat body color, no per-vertex lighting
        glColor3fv((const GLfloat *)((int)bmd_obj + 0x48));
        param_2_b0 = '\0';
    } else if ((param_2_b0 != '\0') && (iVar12 = 0, 0 < *(short *)(pcVar1 + 6))) {
        // Per-vertex lighting: LightTransform[mesh * 15000 + normal] * BodyLight * boost
        pfVar8  = (float *)(&DAT_077e298c + meshIndex * 15000);
        pfVar10 = (float *)(&DAT_060db65c + 4 + meshIndex * 180000);  // G-component base
        do {
            iVar12++;
            pfVar10[-1] = *pfVar8 * *(float *)((int)bmd_obj + 0x48);  // R
            *pfVar10    = *pfVar8 * *(float *)((int)bmd_obj + 0x4c);  // G
            pfVar10[1]  = *pfVar8 * *(float *)((int)bmd_obj + 0x50);  // B
            pfVar8++;
            pfVar10 += 3;
        } while (iVar12 < *(short *)(pcVar1 + 6));
    }

    bVar4 = (unsigned char)uVar6;

    if ((bVar4 & 1) == 1) {
        // RENDER_COLOR: flat color, disable texture
        renderMode = MODE_COLOR;
        if ((bVar4 & 0x40) == 0x40)
            EnableAlphaBlend();
        else if ((bVar4 & 0x80) == 0x80)
            EnableAlphaBlendMinus();
        else
            DisableAlphaBlend();
        DisableTexture('\0');

        if (alpha < _DAT_00552544) {
            EnableAlphaTest('\x01');
            glColor4f(*(float *)((int)bmd_obj + 0x48),
                      *(float *)((int)bmd_obj + 0x4c),
                      *(float *)((int)bmd_obj + 0x50), alpha);
        } else {
            glColor3fv((const GLfloat *)((int)bmd_obj + 0x48));
        }
    } else {
        uVar7 = uVar6 & 4;
        if (((uVar7 == 4) || ((bVar4 & 8) == 8)) || ((uVar6 & 0x200) == 0x200)) {
            // RENDER_CHROME / OIL / OBJ: compute wavy texcoords
            if ((*(int *)(pcVar1 + 0x24) != 0) && (*(char *)(*(int *)(pcVar1 + 0x24) + 3) != '\0')) {
                return;
            }
            if (*pcVar1 != '\0') {
                return;
            }

            renderMode = MODE_CHROME;

            // Recompute WorldTime-based phase for OIL (misma corrección: usar
            // WorldTime explícito, no ST0 basura). IDA: `(__int64)WorldTime % 5000`.
            iVar12 = 0;
            fVar11 = (float)(int)(worldTime % 5000) * _DAT_005528c0 - _DAT_005528b4;

            if (0 < *(short *)(pcVar1 + 6)) {
                // BUG-FIX CRÍTICO: Ghidra decompiló mal la base.
                // Disasm @ 0x00441194:  MOV ECX,0x5828d60   (NO 0x5828d58)
                // Layout real del array: d5c=U0, d60=V0, d64=U1, d68=V1, ...
                // Con el base mal puesto en d58, pfVar10[-1] escribía en
                // DAT_05828d54 y *pfVar10 en DAT_05828d58.
                // **DAT_05828d58 es el puntero base a la tabla de modelos**
                // (extern DWORD DAT_05828d58 → g_Models). El write lo pisaba con
                // un float aleatorio cada frame que renderizaba chrome → todo
                // acceso posterior a Models[type*0xbc] punteaba memoria basura
                // → heap corruption + render colapsado a wireframe.
                pfVar10 = ((float *)&DAT_05828d5c) + 1;   // = &DAT_05828d60 (V0 slot)
                // BUG-FIX: DAT_06f433c0 está declarado como `float`, así que
                // &DAT_06f433c0 + meshIdx*180000 hace aritmética float* (=
                // +meshIdx*720000 bytes). El stride real per-mesh son 180000
                // BYTES (verificado en disasm @ 0x004411a8: ADD EAX,0x6f433c0
                // tras EBP*180000). Castear a char* para que el +N sea byte arith.
                pfVar8  = (float *)((char*)&DAT_06f433c0 + meshIndex * 180000);  // source normals
                do {
                    if ((uVar6 & 0x200) == 0x200) {
                        // OIL mode
                        pfVar10[-1] = fVar11 + fVar11 + (pfVar8[1] + pfVar8[-1]) * _DAT_00552530;
                        fVar9 = fVar11 * _DAT_00552540 + *pfVar8 + pfVar8[-1];
                    } else if (uVar7 == 4) {
                        // CHROME mode
                        pfVar10[-1] = pfVar8[1] * _DAT_00552504 + fVar16;
                        fVar9 = *pfVar8 * _DAT_00552504 + fVar16 + fVar16;
                    } else {
                        // OBJ mode
                        pfVar10[-1] = pfVar8[1] * _DAT_00552504 + _DAT_005526e4;
                        fVar9 = (*pfVar8 + _DAT_0055256c) * _DAT_00552504;
                    }
                    iVar12++;
                    *pfVar10 = fVar9;
                    pfVar8  += 3;
                    pfVar10 += 2;
                } while (iVar12 < *(short *)(pcVar1 + 6));
            }

            if ((bVar4 & 0x40) == 0x40) {
                if (alpha < _DAT_00552544) {
                    *(float *)((int)bmd_obj + 0x48) = alpha * *(float *)((int)bmd_obj + 0x48);
                    *(float *)((int)bmd_obj + 0x4c) = alpha * *(float *)((int)bmd_obj + 0x4c);
                    *(float *)((int)bmd_obj + 0x50) = alpha * *(float *)((int)bmd_obj + 0x50);
                }
                EnableAlphaBlend();
            } else if ((bVar4 & 0x80) == 0x80) {
                EnableAlphaBlendMinus();
            } else if ((bVar4 & 0x10) == 0x10) {
                EnableLightMap();
            } else if (alpha < _DAT_00552544) {
                EnableAlphaTest('\x01');
            } else {
                DisableAlphaBlend();
            }

            // Select chrome texture
            if (((uVar6 & 0x200) == 0x200) && (texOverride == 0xffffffff))
                local_10 = 0x4f2;
            else if ((uVar7 == 4) && (texOverride == 0xffffffff))
                local_10 = 0x492;
            else if (((bVar4 & 8) == 8) && (texOverride == 0xffffffff))
                local_10 = 0x4ce;
            BindTexture(local_10);

        } else if (
            // BUG-FIX CRÍTICO 2 (banner MU Logo03 backdrop):
            // El binario original compara como INT, no float. IDA sub_440D50:
            //   else if ( a5 <= -2 || *(__int16 *)(v40 + 2) == a5 )
            // `a5` llega como `*(_DWORD *)(obj+100)` → bits raw del entero
            // (MUGAME escribe DWORD 1 en obj+100 vía `v6[25]=1`). Nuestro port
            // pasa eso por un `float` param → 0x00000001 se interpreta como
            // denormal 1.4e-45f, nunca matchea `1.0f`. Reinterpretamos los bits
            // de blendMesh a int para matchear la comparación integer del orig.
            //
            // Sentinel "no blend": caller pasa 0xffffffff (como float = NaN).
            //   - Como int: (int)0xffffffff = -1 → cond1 `<= -2` = FALSE ✓
            //   - Como int: -1 != mesh.Texture (siempre ≥ 0) → cond2 FALSE ✓
            // Sentinel alt: caller pasa float -1.0f → bits 0xBF800000 (int ~ -1.1e9)
            //   - cond1 `<= -2` = TRUE → dispara blend, IDA también (`-1.1e9 <= -2`).
            //   - En IDA real original: a5=-1 integer → cond1 FALSE, cond2 mesh_tex==-1 FALSE.
            //   - Acá divergía pre-fix. Aceptamos la divergencia pues el sentinel
            //     real que los callers usan es 0xffffffff (ver Entity_DrawByType.cpp).
            // MUGAME type 0xa2: obj+100 = DWORD 1 → como int == mesh.Texture=1 (backdrop)
            //   → cond2 TRUE → EnableAlphaBlend (aditivo) sobre backdrop naranja.
            blendMesh <= -2 || (int)*(short *)(pcVar1 + 2) == blendMesh
        ) {
            // Blend mesh / animated UV variant.
            renderMode = MODE_TEXTURE;
            BindTexture(local_10);
            if ((bVar4 & 0x80) == 0x80)
                EnableAlphaBlendMinus();
            else
                EnableAlphaBlend();
            glColor3f(blendLight * *(float *)((int)bmd_obj + 0x48),
                      blendLight * *(float *)((int)bmd_obj + 0x4c),
                      blendLight * *(float *)((int)bmd_obj + 0x50));
            param_2_b0 = '\0';

        } else if ((bVar4 & 2) == 2) {
            // RENDER_TEXTURE: explicit bind with blend
            renderMode = MODE_TEXTURE;
            BindTexture(local_10);
            if ((bVar4 & 0x40) == 0x40)
                EnableAlphaBlend();
            else if ((bVar4 & 0x80) == 0x80)
                EnableAlphaBlendMinus();
            else if ((alpha < _DAT_00552544) || ((&DAT_083a7cc8)[local_10 * 0x38] == '\x04'))
                EnableAlphaTest('\x01');
            else
                DisableAlphaBlend();

        } else if ((bVar4 & 0x40) == 0x40) {
            // Bright-only chrome: skip if texture type is 4
            if ((&DAT_083a7cc8)[local_10 * 0x38] == '\x04') {
                return;
            }
            renderMode = MODE_METAL;
            EnableAlphaBlend();
            DisableTexture('\0');
            DisableDepthMask();

        } else {
            renderMode = MODE_TEXTURE;
        }
    }

    // ── DIAG: dump resolved tex/mode/flags/alpha before triangle loop ───
    // 2026-07-27: furniture_diag (tex 0x346-0x348) disparaba en CADA draw →
    // miles de líneas/frame inundando debug.log y tapando el resto de diags
    // (paquete de compra, HEROLIGHT, etc.). Desactivado.
    bool furniture_diag = false;
    if (should_log || furniture_diag) {
        char b[256];
        float bL0 = *(float*)((char*)bmd_obj + 0x48);
        float bL1 = *(float*)((char*)bmd_obj + 0x4c);
        float bL2 = *(float*)((char*)bmd_obj + 0x50);
        int modelId = -1;
        if ((unsigned int)bmd_obj >= DAT_05828d58) {
            modelId = ((int)bmd_obj - DAT_05828d58) / 0xbc;
        }
        int modeTag = renderMode;
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "BMD_Draw #%d  model=0x%x meshIdx=%d flags=0x%03x tex=0x%03x mode=%d alpha=%.2f bodyLight=(%.2f,%.2f,%.2f) lightEnable=%d texSlot=%d triCount=%d%s",
            dm_diag, modelId, meshIndex, flags, local_10, modeTag, alpha, bL0, bL1, bL2,
            (int)param_2_b0, (int)*(short *)(pcVar1 + 2), (int)*(short *)(pcVar1 + 0x0a),
            furniture_diag ? " [FURNITURE06?]" : "");
        DbgLogPublic(b);
        if (furniture_diag) {
            static int s_furn_face_dbg = 0;
            if (s_furn_face_dbg < 24) {
                char bf[256];
                char *face0 = (char *)(*(int *)(pcVar1 + 0x1c));
                short *light0 = face0 ? (short *)(face0 + 10) : 0;
                short *pos0 = face0 ? (short *)(face0 + 2) : 0;
                short *uv0 = face0 ? (short *)(face0 + 18) : 0;
                _snprintf_s(bf, sizeof(bf), _TRUNCATE,
                    "FURN_FACE model=0x%x mesh=%d face0_nv=%d tex0=%d vIdx=(%d,%d,%d,%d) uvIdx=(%d,%d,%d,%d)",
                    modelId, meshIndex,
                    face0 ? (int)(unsigned char)face0[0] : -1,
                    *(short *)(pcVar1 + 2),
                    pos0 ? (int)pos0[0] : -1, pos0 ? (int)pos0[1] : -1,
                    pos0 ? (int)pos0[2] : -1, pos0 ? (int)pos0[3] : -1,
                    uv0 ? (int)uv0[0] : -1, uv0 ? (int)uv0[1] : -1,
                    uv0 ? (int)uv0[2] : -1, uv0 ? (int)uv0[3] : -1);
                DbgLogPublic(bf);
                s_furn_face_dbg++;
            }
        }
    }

    // Triangle render loop — estructura verbatim del binario original
    // (Ghidra @ 0x00440D50): glBegin(GL_TRIANGLES) UNA sola vez antes del
    // face loop, glEnd() UNA vez después. Las BMD 97k están pre-trianguladas
    // (nv==3 para todas las faces, confirmado via diag _dbgTris/_dbgQuads).
    // El "fix" previo a GL_TRIANGLE_FAN por face divergía del binario real.

    // SAFETY NET para caso lightEnable=0 sin StreamMesh + RENDER_MODE_TEXTURE:
    // si el triangle loop no setea glColor per-vertex, el color previo de
    // GL puede ser (0,0,0) → mesh invisible. Forzamos el bodyLight flat
    // (con boost) justo antes de glBegin como última línea de defensa.
    //
    // NOTA banner Mu (type 0xA2): la rama blend-mesh (línea ~440 arriba) ya
    // setea glColor3f(blendLight * OBJECT.Light[0..2]) correctamente y deja
    // param_2_b0 = '\0'. Si acá volviéramos a pisar glColor con boostedBL
    // (que viene de `bodyLight` combinado de terrain + tint), romperíamos
    // el ramp del MUGAME. Por eso NO se toca glColor en este safety-net:
    // confiamos en el glColor que cada rama del switch ya dejó puesto.
    // El "rectangulo negro" que intentaba prevenir este bloque se resolvió
    // correctamente vía el fix integer del blend-mesh + FUN_004fdc00 ramp.
    // Helper1 (modelo 816) — el "hada"/Guardian Angel. Su BMD tiene 2 meshes:
    //   mesh 0 = fairy.jpg   (el cuerpo, 46 triangulos)
    //   mesh 1 = fairy2.jpg  (el glow, 4 triangulos = 2 quads)
    // Ninguna de las dos lleva el marcador `_R` en el nombre (verificado leyendo
    // Helper01.bmd, que es version 10 y NO esta encriptado), asi que
    // `TextureScriptParsing::parsingTScript` (0x40C190 — reconoce R/H/S/N tras
    // un `_`) no las marca como bright y el mesh del glow queda RENDER_TEXTURE
    // opaco: el fondo negro del JPG se dibuja como un recuadro negro.
    //
    // 2026-08-24: antes esto se gateaba por ESCENA (`g_GameState == 2 || == 4`),
    // dejando in-world afuera a proposito "porque ahi el 816 puede renderizarse
    // como pet-item de inventario (opaco)". Consecuencia: con el Guardian Angel
    // equipado, en el mundo se veia el recuadro negro (reportado sobre el pet de
    // otro jugador). El gate correcto no es la escena sino la MESH: solo el glow
    // (mesh 1) necesita el aditivo; el cuerpo (mesh 0) debe seguir opaco. Asi
    // vale igual en el mundo y en el inventario, donde el glow tambien es glow.
    //
    // DESVIACION documentada: no encontre en IDA el mecanismo por el que el
    // original decide este blend — no sale del asset (el BMD v10 no tiene campo
    // de RenderType; se deriva del nombre de textura) ni de `RenderLinkObject`
    // (0x455430), que no toca BlendMesh. Queda como forzado explicito, igual que
    // el gate por escena que reemplaza.
    if (bmd_obj == (void*)(DAT_05828d58 + 816 * 0xbc) && meshIndex == 1) {
        EnableAlphaBlend();
    }

    glBegin(GL_TRIANGLES);
    texOverride = 0;
    int _diag_vert_push = 0;  // DIAG: vertices realmente empujados a GL
    // DIAG: bounding box of pushed vertices (world-space)
    float _diag_xmin =  1e30f, _diag_ymin =  1e30f, _diag_zmin =  1e30f;
    float _diag_xmax = -1e30f, _diag_ymax = -1e30f, _diag_zmax = -1e30f;
    if (0 < *(short *)(pcVar1 + 0x0a)) {
        float param_5_f = 0.0f;  // face byte offset
        do {
            pcVar13 = (char *)((int)param_5_f + *(int *)(pcVar1 + 0x1c));
            iVar12 = 0;
            if ('\0' < *pcVar13) {
                psVar14 = (short *)(pcVar13 + 10);
                do {
                    sVar3 = psVar14[-4];  // bone/position index

                    if (renderMode == MODE_TEXTURE) {
                        // Standard UV from texcoord table
                        pfVar8 = (float *)(*(int *)(pcVar1 + 0x18) + psVar14[4] * 8);
                        if (bVar5) {
                            // Blend wave offset
                            glTexCoord2f(*pfVar8 + uvU, pfVar8[1] + uvV);
                        } else {
                            glTexCoord2f(*pfVar8, pfVar8[1]);
                        }
                        if (param_2_b0 != '\0') {
                            iVar2 = meshIndex * 15000 + (int)*psVar14;
                            if (alpha < _DAT_00552544) {
                                glColor4f(*(float *)(&DAT_060db65c + iVar2 * 12),
                                          *(float *)(&DAT_060db65c + iVar2 * 12 + 4),
                                          *(float *)(&DAT_060db65c + iVar2 * 12 + 8), alpha);
                            } else {
                                glColor3fv((const GLfloat *)(&DAT_060db65c + iVar2 * 12));
                            }
                        }
                    } else if (renderMode == MODE_CHROME) {
                        // Chrome UV from precomputed chrome table
                        if (alpha < _DAT_00552544) {
                            glColor4f(*(float *)((int)bmd_obj + 0x48),
                                      *(float *)((int)bmd_obj + 0x4c),
                                      *(float *)((int)bmd_obj + 0x50), alpha);
                        } else {
                            glColor3fv((const GLfloat *)((int)bmd_obj + 0x48));
                        }
                        // BUG-FIX: Ghidra emitió `(&DAT_05828d5c + 4)[idx*2]` que
                        // es aritmética float* (+4 = +16 bytes), leyendo
                        // V de d6c+idx*8 en vez del correcto d60+idx*8.
                        // Disasm @ 0x004413bd-c4:
                        //   MOV ECX,[EAX*0x8 + 0x5828d60]   ; V
                        //   MOV EDX,[EAX*0x8 + 0x5828d5c]   ; U
                        // El array es {U,V,U,V,...} contiguo desde d5c → V está
                        // en idx*2+1, no idx*2+4.
                        glTexCoord2f((&DAT_05828d5c)[*psVar14 * 2 + 0],
                                     (&DAT_05828d5c)[*psVar14 * 2 + 1]);
                    }

                    // Emit vertex from BoneVertex buffer
                    {
                        const float *_v = (const float*)(&DAT_0584621c + ((int)sVar3 + meshIndex * 15000) * 3);
                        glVertex3fv((const GLfloat *)_v);
                        if (_v[0] < _diag_xmin) _diag_xmin = _v[0];
                        if (_v[1] < _diag_ymin) _diag_ymin = _v[1];
                        if (_v[2] < _diag_zmin) _diag_zmin = _v[2];
                        if (_v[0] > _diag_xmax) _diag_xmax = _v[0];
                        if (_v[1] > _diag_ymax) _diag_ymax = _v[1];
                        if (_v[2] > _diag_zmax) _diag_zmax = _v[2];
                    }
                    _diag_vert_push++;

                    iVar12++;
                    psVar14++;
                } while (iVar12 < *pcVar13);
            }
            texOverride++;
            param_5_f = (float)((int)param_5_f + 0x24);  // face stride = 36 bytes
        } while ((int)texOverride < (int)*(short *)(pcVar1 + 0x0a));
    }
    glEnd();
    if (should_log) {
        char b2[256];
        short nFaces = *(short *)(pcVar1 + 0x0a);
        if (_diag_vert_push > 0) {
            _snprintf_s(b2, sizeof(b2), _TRUNCATE,
                "BMD_Draw #%d  DRAWN v=%d f=%d  AABB x=[%.1f..%.1f] y=[%.1f..%.1f] z=[%.1f..%.1f] sz=(%.1f,%.1f,%.1f)",
                dm_diag, _diag_vert_push, (int)nFaces,
                _diag_xmin, _diag_xmax, _diag_ymin, _diag_ymax, _diag_zmin, _diag_zmax,
                _diag_xmax-_diag_xmin, _diag_ymax-_diag_ymin, _diag_zmax-_diag_zmin);
        } else {
            _snprintf_s(b2, sizeof(b2), _TRUNCATE,
                "BMD_Draw #%d  DRAWN v=0 f=%d (no verts pushed)",
                dm_diag, (int)nFaces);
        }
        DbgLogPublic(b2);
    }
    // BUG-FIX: restaurar depth-mask al salir. El path chrome (flag&0x40) llama
    // DisableDepthMask() arriba pero nunca lo restauraba dentro de la función,
    // causando que los siguientes meshes del mismo frame dibujaran sin depth-
    // write → efecto "ghost" (se ven las caras traseras a través de las
    // frontales, y cada mesh sucesivo blendea sobre el anterior). El binario
    // original hace EnableDepthMask en los setters de estado entre meshes,
    // pero en nuestra versión el cache DAT_083a42e8 quedaba desincronizado.
    GL_EnableDepthWrites();  // EnableDepthMask
}
