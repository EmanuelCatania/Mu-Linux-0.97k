#include "stdafx.h"
#include "UI/MiniMap.h"
#include "UI/EventTimer.h"
#include "UI/MoveList.h"
#include "UI/UIState.h"
#include "Config/UserSettings.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "Game/MapManager.h"
#include "Local/ClientText.h"
#include <math.h>

CMiniMap gMiniMap;

namespace {
// DLL MiniMap.h / Defines.h.
constexpr int TerrainSize = 256, MapSize = 512, MaxGates = 100;
constexpr float MaxSize = 640.0f;             // lado del mapa en el layout de 640
constexpr float CenterX = 320.0f - 320.0f + MaxSize * 0.5f;
constexpr float CenterY = 192.0f - 240.0f + MaxSize * 0.5f;
constexpr float Rotation = -45.0f;
constexpr int PointTexture = 9;               // flecha (Interface), como el DLL
constexpr int ButtonY = 411, ButtonPanelWidth = 96, ButtonSize = 21;

// Estado de cada frame (DLL RenderFullMap).
struct View {
    float scale, size, charX, charY, offsetX, offsetY;
    int range;
};

View CurrentView(int zoom)
{
    View v;
    v.range = 128 - zoom * 16;
    v.scale = 256.0f / (v.range * 2);
    v.size = 512.0f / v.scale;
    const BYTE* hero = (const BYTE*)(uintptr_t)Hero;
    v.charX = *(const float*)(hero + 0x10) / 100.0f;
    v.charY = *(const float*)(hero + 0x14) / 100.0f;
    v.offsetX = (v.charX - v.range) / 256.0f;
    v.offsetY = ((256.0f - v.charY) - v.range) / 256.0f;
    return v;
}

bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}

float ToX(float x) { return x * gWindow.GetWidth() / 640.0f; }
float ToY(float y) { return y * gWindow.GetHeight() / 480.0f; }

void Rotate(float x, float y, float degrees, float& ox, float& oy)
{
    // AngleMatrix con (0, 0, grados) + VectorRotate: rotación sobre Z.
    const float r = degrees * 3.14159265f / 180.0f, c = cosf(r), s = sinf(r);
    ox = x * c - y * s;
    oy = x * s + y * c;
}

void EmitQuad(const float p[4][2], float u, float v, float uw, float vh)
{
    const float tc[4][2] = { { u, v }, { u, v + vh }, { u + uw, v + vh }, { u + uw, v } };
    const float cx = gWindow.GetWidth() / 2.0f, cy = gWindow.GetHeight() / 2.0f;
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 4; ++i) {
        glTexCoord2f(tc[i][0], tc[i][1]);
        glVertex2f(p[i][0] + cx, p[i][1] + cy);
    }
    glEnd();
}

// DLL OpenglUtil.cpp MyRenderBitRotate.
void BitRotate(float x, float y, float width, float height, float rotate,
               float u, float v, float uw, float vh)
{
    x = ToX(x); y = ToY(y); width = ToX(width); height = ToY(height);
    y = height - y;
    const float cx = width / 2.0f - (width - x), cy = height / 2.0f - (height - y);
    const float ax = -width * 0.5f + cx, bx = width * 0.5f + cx;
    const float ay = -height * 0.5f + cy, by = height * 0.5f + cy;
    const float corners[4][2] = { { ax, by }, { ax, ay }, { bx, ay }, { bx, by } };
    float p[4][2];
    for (int i = 0; i < 4; ++i) Rotate(corners[i][0], corners[i][1], rotate, p[i][0], p[i][1]);
    EmitQuad(p, u, v, uw, vh);
}

// DLL OpenglUtil.cpp MyRenderPointRotate: un icono en (ix, iy) del mapa, que se
// rota con el mapa y además sobre sí mismo; con `tooltip`, el texto al pasar.
void PointRotate(float ix, float iy, float iw, float ih, float rotateLocal,
                 float u, float v, float uw, float vh, const char* tooltip)
{
    const float width = ToX(MaxSize), height = ToY(MaxSize);
    const float x = ToX(CenterX), y = height - ToY(CenterY);
    ix = ToX(ix); iy = height - ToY(iy);
    float px, py;
    Rotate((ix - width * 0.5f) + (width / 2.0f - (width - x)),
           (iy - height * 0.5f) + (height / 2.0f - (height - y)), Rotation, px, py);
    const float corners[4][2] = { { -iw * 0.5f, ih * 0.5f }, { -iw * 0.5f, -ih * 0.5f },
                                  { iw * 0.5f, -ih * 0.5f }, { iw * 0.5f, ih * 0.5f } };
    float p[4][2];
    for (int i = 0; i < 4; ++i) {
        Rotate(corners[i][0], corners[i][1], rotateLocal, p[i][0], p[i][1]);
        p[i][0] += px; p[i][1] += py;
    }
    EmitQuad(p, u, v, uw, vh);
    if (!tooltip) return;

    const float W = (float)gWindow.GetWidth(), H = (float)gWindow.GetHeight();
    const float dx = (px + W / 2.0f) * (640.0f / W) - iw / 4.0f;
    const float dy = (py + H / 2.0f) * (480.0f / H) + ih / 4.0f;
    if (!Inside((int)dx, (int)(480.0f - dy), (int)(iw / 2.0f), (int)(ih / 2.0f))) return;
    SIZE size = {};
    GetTextExtentPointA(gFont.GetTextDC(), tooltip, lstrlenA(tooltip), &size);
    const int textWidth = (int)(size.cx * 640 / gWindow.GetWidth());
    const DWORD text = m_dwTextColor, back = m_dwBackColor;
    glColor3f(1.0f, 1.0f, 1.0f);
    m_dwBackColor = 0xFF000000u;
    m_dwTextColor = 0xFFFFFF00u;   // cian (ABGR)
    RenderTipText((int)(dx + iw / 2.0f) - textWidth / 2, (int)(480.0f - dy - 5.0f), tooltip);
    m_dwTextColor = text;
    m_dwBackColor = back;
}

int CenterTextY(const char* text, int centerY)
{
    SIZE size = {};
    GetTextExtentPointA(gFont.GetTextDC(), text, lstrlenA(text), &size);
    return centerY - ((480 * size.cy / (int)gWindow.GetHeight()) >> 1);
}
}

bool CMiniMap::Available() const
{
    return SceneFlag == 5 && World >= 0 && Hero &&
           gMapManager.GetMiniMap(World) == CMapManager::MiniMapMode::FullMap;
}

void CMiniMap::Toggle()
{
    if (!Available()) { m_Open = false; return; }
    if (UIState::HasTextInput()) return;
    if (UIState::HasRightPanel()) { m_Open = false; return; }
    // DLL MiniMap.cpp Toggle: comparte la pantalla con los paneles H y M.
    gEventTimer.Close();
    gMoveList.Close();
    m_Open = !m_Open;
    PlayBuffer(25, 0, 0);
}

void CMiniMap::ChangeZoom()
{
    m_Zoom = m_Zoom + 1 >= 7 ? 0 : m_Zoom + 1;
    gUserSettings.SetMiniMap(m_Alpha, m_Zoom);
}

void CMiniMap::ChangeAlpha()
{
    m_Alpha = m_Alpha + 1 > 10 ? 3 : m_Alpha + 1;
    gUserSettings.SetMiniMap(m_Alpha, m_Zoom);
}

void CMiniMap::UpdateMouse()
{
    if (!m_Open) return;
    if (!Available() || UIState::HasRightPanel()) { m_Open = false; return; }
    auto button = [&](int panelX, int buttonX, void (CMiniMap::*action)()) {
        if (!Inside(panelX, ButtonY, ButtonPanelWidth, ButtonSize)) return false;
        UIState::CaptureMouseForUI();
        if (Inside(buttonX, ButtonY, ButtonSize, ButtonSize) && MouseLButton && MouseLButtonPush) {
            MouseLButtonPush = 0;
            DAT_07e11d28 = 0;   // MouseUpdateTime
            DAT_00559bec = 6;   // MouseUpdateTimeMax
            PlayBuffer(25, 0, 0);
            (this->*action)();
        }
        return true;
    };
    if (button(0, 75, &CMiniMap::ChangeZoom)) return;
    button(640 - ButtonPanelWidth, 640 - ButtonPanelWidth, &CMiniMap::ChangeAlpha);
}

// DLL LoadMiniMapImage + PaintBorder: cada tile caminable es un bloque de 2x2
// oscuro y semitransparente; el borde contra tiles no caminables, claro.
void CMiniMap::BuildTexture()
{
    static BYTE pixels[MapSize * MapSize * 4];
    memset(pixels, 0, sizeof(pixels));
    auto wall = [](int sx, int sy) { return TerrainWall[(TerrainSize - 1 - sy) * TerrainSize + sx]; };
    auto put = [](int px, int py, BYTE r, BYTE g, BYTE b, BYTE a) {
        BYTE* d = &pixels[(py * MapSize + px) * 4];
        d[0] = r; d[1] = g; d[2] = b; d[3] = a;
    };
    for (int sy = 0; sy < TerrainSize; ++sy) {
        for (int sx = 0; sx < TerrainSize; ++sx) {
            const BYTE w = wall(sx, sy);
            if (w & 0xFC) continue;                 // NOMOVE, NOGROUND, WATER, ACTION, HEIGHT, CAMERA_UP
            // Borde: el vecino no caminable pinta su lado pegado a este tile.
            if (sx - 1 > 0 && (wall(sx - 1, sy) & 0xFC)) {
                put((sx - 1) * 2 + 1, sy * 2, 192, 254, 243, 128);
                put((sx - 1) * 2 + 1, sy * 2 + 1, 192, 254, 243, 128);
            }
            if (sx + 1 < TerrainSize && (wall(sx + 1, sy) & 0xFC)) {
                put((sx + 1) * 2, sy * 2, 192, 254, 243, 128);
                put((sx + 1) * 2, sy * 2 + 1, 192, 254, 243, 128);
            }
            if (sy - 1 > 0 && (wall(sx, sy - 1) & 0xFC)) {
                put(sx * 2, (sy - 1) * 2 + 1, 192, 254, 243, 128);
                put(sx * 2 + 1, (sy - 1) * 2 + 1, 192, 254, 243, 128);
            }
            if (sy + 1 < TerrainSize && (wall(sx, sy + 1) & 0xFC)) {
                put(sx * 2, (sy + 1) * 2, 192, 254, 243, 128);
                put(sx * 2 + 1, (sy + 1) * 2, 192, 254, 243, 128);
            }
            put(sx * 2, sy * 2, 0, 0, 0, 204);
            put(sx * 2 + 1, sy * 2, 0, 0, 0, 189);
            put(sx * 2, sy * 2 + 1, 0, 0, 0, 204);
            put(sx * 2 + 1, sy * 2 + 1, 0, 0, 0, 189);
        }
    }
    if (!m_Texture) glGenTextures(1, (GLuint*)&m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, MapSize, MapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    const GLfloat border[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    DAT_00561574 = 0xFFFFFFFF;   // la caché de BindTexture ya no vale
    m_TextureWorld = World;
}

void CMiniMap::RenderBackground() const
{
    const View v = CurrentView(m_Zoom);
    EnableAlphaTest(true);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    DAT_00561574 = 0xFFFFFFFF;
    glColor4f(1.0f, 1.0f, 1.0f, m_Alpha * 0.1f);
    BitRotate(CenterX, CenterY, MaxSize, MaxSize, Rotation,
              v.offsetX, v.offsetY, v.size / 512.0f, v.size / 512.0f);
}

void CMiniMap::RenderMarkers() const
{
    const View v = CurrentView(m_Zoom);
    const BYTE* base = (const BYTE*)(uintptr_t)CharactersClient;
    if (!base) return;
    for (int i = 0; i < 400; ++i) {
        const BYTE* c = base + i * 916;
        if (!c[0]) continue;
        const float tx = ((*(const float*)(c + 0x10) / 100.0f) / 256.0f - v.offsetX) * (MaxSize * v.scale);
        const float ty = ((256.0f - *(const float*)(c + 0x14) / 100.0f) / 256.0f - v.offsetY) * (MaxSize * v.scale);
        const BYTE kind = c[0x84];
        float r, g, b, size = 7.0f;
        const char* tooltip = nullptr;
        if (kind == 1) {            // jugador: el héroe verde, el resto amarillo
            const bool hero = (uintptr_t)c == (uintptr_t)Hero;
            r = hero ? 0.0f : 1.0f; g = 1.0f; b = 0.0f;
        } else if (kind == 2) {     // monstruo
            r = 0.5f; g = 0.0f; b = 0.0f; size = 6.0f;
        } else if (kind == 4) {     // NPC, con su nombre
            r = 1.0f; g = 0.0f; b = 1.0f;
            tooltip = (const char*)(c + 0x1C1);
        } else {
            continue;
        }
        EnableAlphaTest(true);
        GL_BindTextureSlot(PointTexture);
        glColor4f(r, g, b, m_Alpha * 0.1f);
        PointRotate(tx, ty, v.scale * size, v.scale * size, *(const float*)(c + 0x24) - 45.0f,
                    2.0f / 32.0f, 14.0f / 32.0f, 30.0f / 32.0f, 18.0f / 32.0f, tooltip);
    }
}

void CMiniMap::RenderGates() const
{
    const View v = CurrentView(m_Zoom);
    const GATE_ATTRIBUTE* gates = (const GATE_ATTRIBUTE*)(uintptr_t)GateAttribute;
    if (!gates) return;
    char text[96];
    for (int i = 0; i < MaxGates; ++i) {
        const GATE_ATTRIBUTE& gate = gates[i];
        if (gate.Flag != 1 || gate.Map != World || gate.Target >= MaxGates) continue;
        const float gx = gate.StartX + ceilf((gate.EndX - gate.StartX) / 2.0f);
        const float gy = gate.StartY + ceilf((gate.EndY - gate.StartY) / 2.0f);
        if (gx > v.charX + v.range || gx < v.charX - v.range ||
            gy > v.charY + v.range || gy < v.charY - v.range) continue;
        const float tx = (gx / 256.0f - v.offsetX) * (MaxSize * v.scale);
        const float ty = ((256.0f - gy) / 256.0f - v.offsetY) * (MaxSize * v.scale);
        sprintf_s(text, gClientText.Get(ClientTextId::MiniMapGate),
                  gMapManager.GetName(gates[gate.Target].Map));
        EnableAlphaTest(true);
        GL_BindTextureSlot(PointTexture);
        glColor4f(0.0f, 1.0f, 1.0f, m_Alpha * 0.1f);
        PointRotate(tx, ty, v.scale * 10.0f, v.scale * 10.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, text);
    }
}

// DLL RenderFullMapZoom / RenderFullMapAlpha: placa 0xF5 (espejada a la
// derecha), botón 0x120/0x121 y el valor en rojo.  Opacos con el mouse encima.
void CMiniMap::RenderButton(bool left) const
{
    const float plateX = left ? 0.0f : 640.0f - 75.0f;
    const float buttonX = left ? 75.0f : 640.0f - 75.0f - ButtonSize;
    const int panelX = left ? 0 : 640 - ButtonPanelWidth;
    const float alpha = Inside(panelX, ButtonY, ButtonPanelWidth, ButtonSize) ? 1.0f : 0.3f;
    EnableAlphaTest(true);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    GL_DrawTexture(0xF5, plateX, (float)ButtonY, 75.0f, 21.0f, left ? 0.0f : 75.0f / 128.0f, 0.0f,
                   (left ? 75.0f : -75.0f) / 128.0f, 21.0f / 32.0f, 1, 1);
    const bool pressed = Inside((int)buttonX, ButtonY, ButtonSize, ButtonSize) && MouseLButton;
    GL_DrawTexture(pressed ? 0x121 : 0x120, buttonX, (float)ButtonY, 21.0f, 21.0f,
                   0.0f, 0.0f, 24.0f / 32.0f, 24.0f / 32.0f, 1, 1);
    GL_ResetState();
    glColor3f(1.0f, 1.0f, 1.0f);

    char text[64];
    if (left) sprintf_s(text, gClientText.Get(ClientTextId::MiniMapZoom), m_Zoom);
    else      sprintf_s(text, gClientText.Get(ClientTextId::MiniMapAlpha), m_Alpha * 10);
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
    EnableAlphaTest(true);
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    m_dwBackColor = 0;
    m_dwTextColor = ((DWORD)(alpha * 255) << 24) | 0x000000FFu;   // rojo (ABGR)
    RenderText(left ? 5 : (int)buttonX + 31, CenterTextY(text, 421), text,
               (int)(60 * gWindow.GetWidth() / 640), 0, nullptr);
    m_dwTextColor = color;
    m_dwBackColor = back;
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    GL_ResetState();
}

void CMiniMap::Render()
{
    if (!m_Open || !Available()) return;
    if (!m_Loaded) {
        m_Alpha = gUserSettings.GetMiniMapAlpha();
        m_Zoom = gUserSettings.GetMiniMapZoom();
        m_Loaded = true;
    }
    if (m_TextureWorld != World) BuildTexture();
    RenderBackground();
    RenderMarkers();
    RenderGates();
    GL_ResetState();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderButton(true);
    RenderButton(false);
}
