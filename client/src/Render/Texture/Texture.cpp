// Texture.cpp
// OZJ / OZT texture loading + OpenGL upload pipeline
//
// Entry points:
//   Texture_Load      @ 0x00529740  (280 lines) — load OZJ/OZT → GPU
//   Texture_Unload    @ 0x0052a050  ( 29 lines) — ref-counted free
//   GL_BindTextureSlot    @ 0x00511480  ( 15 lines) — cached glBindTexture
//   Texture_Draw2D    @ 0x005125a0  ( 51 lines) — textured screen quad
//   Texture_LoadFile  @ 0x00529130  (224 lines) — inner file-load worker
//
// ── TEXTURE SLOT TABLE ────────────────────────────────────────────────────────
//
//   Base: DAT_083a7ca0     stride: 0x38 bytes = 14 dwords per slot
//   Max:  ~256 slots (IDs 0 .. 0xFF used; HUD reaches 0xF9)
//
//   Slot layout (byte offsets within each 0x38-byte entry):
//     +0x00  byte   active           set to 0 on Texture_Unload
//     +0x20  float  width            pow-of-2 width uploaded to GL
//     +0x24  float  height           pow-of-2 height uploaded to GL
//     +0x28  byte   bpp              bytes-per-pixel (always 3 = RGB)
//     +0x2C  uint   gl_handle        glGenTextures result
//     +0x30  byte   ref_count        Texture_Load increments, Unload decrements
//     +0x34  ptr    pixels           malloc'd RGB pixel buffer
//
//   Helper addresses for per-slot fields:
//     DAT_083a7ca0  = slot[0].active
//     DAT_083a7cc0  = slot[0].width      (float)
//     DAT_083a7cc4  = slot[0].height     (float)
//     DAT_083a7cc8  = slot[0].bpp        (byte)
//     DAT_083a7ccc  = slot[0].gl_handle  (uint)
//     DAT_083a7cd0  = slot[0].ref_count  (byte)
//     DAT_083a7cd4  = slot[0].pixels     (ptr)
//
//   Accessed as: (&DAT_083a7ccc)[id * 0xE]  (uint* stride = 4 → 0xE*4 = 0x38)
//                (&DAT_083a7cc8)[id * 0x38] (byte* stride = 1)
//
// ── GLOBAL STATE ──────────────────────────────────────────────────────────────
//
//   DAT_00561574   int   g_bound_texture_id   last texture bound (avoids redundant calls)
//   DAT_00561570   int   g_screen_height      used in Texture_Draw2D for Y-flip
//   m_dwUsedTextureMemory   int   g_vram_used          running tally of texture memory (bpp*w*h bytes)
//
// File decoding: stb_image, pinned in lib/stb. OZJ skips 24 bytes, OZT 4.
// JPEG is RGB top-down; legacy TGA is raw 32-bit BGRA, reversed to RGBA.
// Zero-filled power-of-two padding and GL_MAX_TEXTURE_SIZE apply to both.
//
// ── TEXTURE PATH MODES ────────────────────────────────────────────────────────
//
//   DAT_0055a7c4 == 0  →  Standard mode:
//       Full path = DAT_0055a7a4 (base dir) + param_1 (filename)
//       SaveImage(id, extension, path, NULL, 0) handles the actual I/O.
//
//   DAT_0055a7c4 != 0  →  Extension-swap mode (OZJ ↔ OZT):
//       Strips extension from filename up to the last '.'.
//       Appends DAT_0055a79c (e.g. ".OZJ") for high-quality textures,
//       or DAT_00561b70 (e.g. ".OZT") for low-quality / alternate.
//       Used to transparently switch texture quality level at runtime.
//
// ── COORDINATE SYSTEM (Texture_Draw2D) ───────────────────────────────────────
//
//   Screen Y is inverted: OpenGL origin is bottom-left.
//   top_gl_y = screen_height - screen_y
//   bot_gl_y = screen_height - screen_y - height
//
//   param_10 (scale_wh): if set, w and h are scaled via Screen_ToGLX / Screen_ToGLY
//   param_11 (scale_xy): if set, x and y are scaled
//   Scale factor: DAT_0056156c * DAT_0055283c  (set from ChangeDisplaySettings)
//
//   glBegin(6) = GL_QUAD_STRIP → draws 4 vertices (BL, BR, TL, TR) with texcoords
//
// ── GL CONSTANTS ──────────────────────────────────────────────────────────────
//   0xDE1  = GL_TEXTURE_2D
//   0x1907 = GL_RGB
//   0x1401 = GL_UNSIGNED_BYTE
//   0x2800 = GL_TEXTURE_MAG_FILTER    (param_3 in Texture_Load)
//   0x2801 = GL_TEXTURE_MIN_FILTER    (param_3)
//   0x2802 = GL_TEXTURE_WRAP_S        (param_4)
//   0x2803 = GL_TEXTURE_WRAP_T        (param_4)

#include "stdafx.h"
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include "Render/Gfx.h"
#include "Render/ResourceTables.h"
#include "Render/BufferedDraw.h"
#include "Render/RenderState.h"
#include "Platform/Text.h"
#include "Platform/IWindow.h"
#include "Platform/IFileSystem.h"
#include "Texture.h"

#include "ImageCodec.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cstdio>
#include <vector>

extern "C" { void DbgLogPublic(const char* msg); }

// Texture table base
// Bitmaps table is now a contiguous array (g_BitmapsRaw in globals.h)
#define TEX_STRIDE   0x38

struct TexSlot
{
    BYTE     active;         // +0x00
    BYTE     _pad[0x1F];
    float    width;          // +0x20
    float    height;         // +0x24
    BYTE     bpp;            // +0x28
    BYTE     _pad2[3];
    UINT     gl_handle;      // +0x2C
    BYTE     ref_count;      // +0x30
    BYTE     _pad3[3];
    void*    pixels;         // +0x34
};

#define TexTable ((TexSlot*)ResourceTables::TextureBase())

extern int  g_bound_texture_id;   // DAT_00561574
extern int  g_screen_height;      // DAT_00561570
extern int  g_vram_used;          // m_dwUsedTextureMemory
// g_tex_ext_mode is declared via #define in globals.h as DAT_0055a7c4 (char)
extern char g_tex_base_dir[];     // DAT_0055a7a4  base directory string
extern char g_tex_ext_hq[];       // DAT_0055a79c  high-quality extension (e.g. ".OZJ")
extern char g_tex_ext_lq[];       // DAT_00561b70  low-quality extension  (e.g. ".OZT")
extern float g_scale_x;           // DAT_0056156c  screen scale X
extern float g_scale_y;           // DAT_0055283c  screen scale Y


// ============================================================
// Texture_BindLocalCached — module-local compatibility helper.
// Wrapper around glBindTexture that avoids redundant state changes.
//   id >= 0 : look up gl_handle from texture table slot [id]
//   id <  0 : use -id directly as GL texture handle (raw GL object)
// ============================================================
void Texture_BindLocalCached(int id)
{
    // ⚠ ESTE **NO** ES EL BIND QUE USA EL RENDER.
    //
    // El bind real del pipeline es `GL_BindTextureSlot` (0x00511480, en GL_State.cpp):
    // ése es el que llaman BMD_DrawMesh, los efectos, los sprites y el HUD.
    // Esta función es un equivalente funcional sin callers vivos — su único
    // caller es `Texture_Draw2D`, que a su vez tampoco tiene callers.
    //
    // Peor: mantiene su PROPIO cache (`g_bound_texture_id`) distinto del de
    // GL_BindTextureSlot (`DAT_00561574`). Si algún día se la vuelve a usar, los dos
    // caches se desincronizan y se omiten binds → textura equivocada.
    //
    // Se conserva porque forma parte del port de este módulo, pero NO
    // instrumentar acá para diagnosticar el render: un probe acá da 0 hits
    // aunque las texturas estén mal.
    if (g_bound_texture_id == id)
        return;
    g_bound_texture_id = id;

    if (id >= 0)
        Gfx::BindTexture(TexTable[id].gl_handle);
    else
        Gfx::BindTexture((UINT)(-id));
}


// ============================================================
// Texture_Unload  @ 0x0052a050
// Ref-counted texture free. Decrements ref_count; when it reaches 0:
//   glDeleteTextures, free pixel buffer, update g_vram_used.
// ============================================================
void Texture_Unload(int id)
{
    ResourceTables::EnsureTextures(std::size_t(id)+1);
    TexSlot* slot = &TexTable[id];
    if (slot->pixels == nullptr)
        return;

    float w = slot->width;
    float h = slot->height;

    if (slot->ref_count != 0)
    {
        slot->ref_count--;
        if (slot->ref_count == 0)
        {
            Gfx::DeleteTexture(slot->gl_handle);
            slot->active = 0;
            operator_delete(slot->pixels);
            slot->pixels = nullptr;
            g_vram_used -= (int)slot->bpp * (int)w * (int)h;
        }
    }
}


// ============================================================
// Texture_Load  @ 0x00529740
// Load a texture from disk (OZJ or OZT) and upload to GPU.
//
//   path     — relative filename (e.g. "Interface/GFx/main.ozj")
//   id       — texture slot index
//   min_filt — GL_TEXTURE_MIN/MAG_FILTER value (e.g. GL_NEAREST=0x2600, GL_LINEAR=0x2601)
//   wrap     — GL_TEXTURE_WRAP_S/T value      (e.g. GL_REPEAT=0x2901, GL_CLAMP=0x2900)
//   flag     — if non-zero: texture name is printed in error path
//   show_err — if non-zero: show MessageBox + close window when file not found
//
// Returns 1 on success, 0 on failure.
//
// Path construction (before fopen):
//   g_tex_ext_mode == 0:
//     full_path = g_tex_base_dir + path  (used as-is)
//   g_tex_ext_mode != 0:
//     Strip extension from path (up to last '.').
//     Append g_tex_ext_hq or g_tex_ext_lq based on current quality mode.
//
// Decode completes before replacing a slot. Invalid/missing data returns 0.
// Padding stays zero; texture size is bounded by tex_max_size().
// ============================================================
// Swap file extension in-place: "Local/Webzenlogo.jpg" -> "Local/Webzenlogo.OZJ"
static void tex_swap_ext(char* path, const char* newExt /*"OZJ"/"OZT"*/)
{
    char* dot = strrchr(path, '.');
    if (!dot) { strcat(path, "."); strcat(path, newExt); return; }
    dot[1] = newExt[0]; dot[2] = newExt[1]; dot[3] = newExt[2]; dot[4] = '\0';
}

// DESVIACION (DLL Patchs.cpp 0x005299E7..0x00529E8B, "Remove JPG/TGA size
// limit"): el binario rechaza texturas de más de 256 (el port ya las subía a
// 1024).  El techo ahora es el que admite la placa, para assets custom
// grandes.  Sin contexto GL todavía, 4096.
static int tex_max_size(void)
{
    static int s_max = 0;
    if (s_max <= 0) {
        int v = Gfx::MaxTextureSize();
        s_max = (v >= 256) ? (int)v : 0;
        if (s_max == 0) return 4096;
    }
    return s_max;
}

// Internal: decode OZJ (JPEG with 24-byte prefix) into TexSlot + GPU upload
static int tex_load_ozj(const char* fullPath, int id, int min_filt, int wrap)
{
    char hdPath[300];
    FILE* f = Platform::GetFileSystem().Open(Texture_HDAssetPath(fullPath, hdPath, sizeof(hdPath)), "rb");
    if (!f) return 0;

    ImageCodec::Image image;
    const bool decoded = ImageCodec::Decode(f, 24, ImageCodec::Format::Jpeg, tex_max_size(), image);
    fclose(f);
    if (!decoded) return 0;
    int w = ImageCodec::PaddedExtent(image.width, tex_max_size());
    int h = ImageCodec::PaddedExtent(image.height, tex_max_size());

    // DIAG: log actual dimensions and decode success

    ResourceTables::EnsureTextures(std::size_t(id)+1);
    TexSlot* slot = &TexTable[id];
    Texture_Unload(id);   // free any previous slot pixels + GL handle

    slot->active    = 1;
    slot->bpp       = 3;
    slot->width     = (float)w;
    slot->height    = (float)h;
    slot->pixels    = (void*)new BYTE[w * h * 3];
    memset(slot->pixels, 0, w * h * 3);
    g_vram_used    += 3 * w * h;

    for (int y = 0; y < image.height; ++y)
        memcpy((BYTE*)slot->pixels + y * w * 3, image.pixels + y * image.width * 3,
            image.width * 3);

    slot->ref_count++;
    slot->gl_handle = Gfx::CreateTexture();
    Gfx::BindTexture(slot->gl_handle);
    Gfx::TexImage2D(3, w, h, Gfx::Format::RGB, slot->pixels);
    Gfx::TexParameter(Gfx::TexParam::MagFilter, Gfx::TexValueFromLegacy(min_filt));
    Gfx::TexParameter(Gfx::TexParam::MinFilter, Gfx::TexValueFromLegacy(min_filt));
    Gfx::TexParameter(Gfx::TexParam::WrapS, Gfx::TexValueFromLegacy(wrap));
    Gfx::TexParameter(Gfx::TexParam::WrapT, Gfx::TexValueFromLegacy(wrap));

    return 1;
}

// Internal: decode OZT (TGA 32-bit with 4-byte prefix) into TexSlot + GPU upload
static int tex_load_ozt(const char* fullPath, int id, int min_filt, int wrap)
{
    char hdPath[300];
    FILE* f = Platform::GetFileSystem().Open(Texture_HDAssetPath(fullPath, hdPath, sizeof(hdPath)), "rb");
    if (!f) return 0;

    ImageCodec::Image image;
    const bool decoded = ImageCodec::Decode(f, 4, ImageCodec::Format::Tga32, tex_max_size(), image);
    fclose(f);
    if (!decoded) return 0;
    int w = ImageCodec::PaddedExtent(image.width, tex_max_size());
    int h = ImageCodec::PaddedExtent(image.height, tex_max_size());

    ResourceTables::EnsureTextures(std::size_t(id)+1);
    TexSlot* slot = &TexTable[id];
    Texture_Unload(id);

    slot->active = 1;
    slot->bpp    = 4;
    slot->width  = (float)w;
    slot->height = (float)h;
    size_t bufSize = (size_t)w * h * 4;
    slot->pixels = (void*)new BYTE[bufSize];
    memset(slot->pixels, 0, bufSize);
    g_vram_used += 4 * w * h;

    for (int y = 0; y < image.height; ++y)
        memcpy((BYTE*)slot->pixels + y * w * 4, image.pixels + y * image.width * 4,
            image.width * 4);

    slot->ref_count++;
    slot->gl_handle = Gfx::CreateTexture();
    Gfx::BindTexture(slot->gl_handle);
    Gfx::TexImage2D(4, w, h, Gfx::Format::RGBA, slot->pixels);
    Gfx::TexEnvMode(Gfx::TexEnv::Modulate);
    Gfx::TexParameter(Gfx::TexParam::MagFilter, Gfx::TexValueFromLegacy(min_filt));
    Gfx::TexParameter(Gfx::TexParam::MinFilter, Gfx::TexValueFromLegacy(min_filt));
    Gfx::TexParameter(Gfx::TexParam::WrapS, Gfx::TexValueFromLegacy(wrap));
    Gfx::TexParameter(Gfx::TexParam::WrapT, Gfx::TexValueFromLegacy(wrap));
    return 1;
}

int Texture_Load(const char* path, int id,
                 int min_filt, int wrap,
                 int flag, char show_err)
{
    (void)flag;

    // Build full path and swap extension based on source ext.
    //   `Cliente armado\` layout: exe vive al lado de Data\, la mayoría de los
    //   callers pasan paths como "Local/..." / "Interface/..." sin prefijo.
    //   Prependemos "Data\\" salvo que el caller ya lo haya incluido.
    char full[512];
    bool hasDataPrefix =
        (path[0]=='D' || path[0]=='d') && (path[1]=='a' || path[1]=='A') &&
        (path[2]=='t' || path[2]=='T') && (path[3]=='a' || path[3]=='A') &&
        (path[4]=='/' || path[4]=='\\');
    if (hasDataPrefix) {
        strncpy(full, path, sizeof(full) - 8);
    } else {
        Platform::TextFormatTruncated(full, Platform::TruncateText, "Data\\%s", path);
    }
    full[sizeof(full) - 8] = '\0';

    // Detect source extension (case-insensitive)
    char* dot = strrchr(full, '.');
    bool isTga = false;
    if (dot) {
        char extBuf[8] = {0};
        for (int i = 0; i < 4 && dot[1 + i]; i++) extBuf[i] = (char)tolower((unsigned char)dot[1 + i]);
        isTga = (strcmp(extBuf, "tga") == 0 || strcmp(extBuf, "ozt") == 0);
    }
    tex_swap_ext(full, isTga ? "OZT" : "OZJ");

    int ok = isTga ? tex_load_ozt(full, id, min_filt, wrap)
                   : tex_load_ozj(full, id, min_filt, wrap);

    if (!ok && show_err) {
        char msg[600];
        Platform::TextFormatTruncated(msg, Platform::TruncateText, "File not found:\n%s", full);
        Platform::GetWindow().ShowError(msg, "IError", true, false);
        if (Platform::GetWindow().IsCreated()) Platform::GetWindow().CloseWindow();
    }
    return ok;
}


// ============================================================
// Texture_Draw2D  @ 0x005125a0
// Draw a textured screen-space quad.
//
//   id         — texture slot (passed to GL_BindTextureSlot)
//   x, y       — screen position of top-left corner (pixels, Y from top)
//   w, h       — width and height (pixels)
//   u0, v0     — top-left UV  (normalized 0.0-1.0)
//   u1, v1     — bottom-right UV delta (added to u0/v0)
//   scale_wh   — if non-0: scale w,h via screen→GL coord transform
//   scale_xy   — if non-0: scale x,y via screen→GL coord transform
//
// Coordinate transform (Screen_ToGLX / Screen_ToGLY):
//   screen_x → gl_x:  x * g_scale_x * g_scale_y  (DAT_0056156c * DAT_0055283c)
//   screen_y → gl_y:  y * (similar Y scale)
//
// GL Y-flip: OpenGL origin is bottom-left.
//   gl_top    = g_screen_height - y
//   gl_bottom = g_screen_height - y - h
//
// Vertex layout (GL_QUAD_STRIP, glBegin(6), 4 vertices):
//   [0] (u0,    v0+v1) → (x,   top)     top-left
//   [1] (u0,    v0)    → (x,   bottom)  bottom-left
//   [2] (u0+u1, v0+v1) → (x+w, top)     top-right
//   [3] (u0+u1, v0)    → (x+w, bottom)  bottom-right
//   (loop: 4 iterations, iVar1 += 8 each step through local_40 interleaved array)
// ============================================================
void Texture_Draw2D(int id,
                    float x, float y, float w, float h,
                    float u0, float v0, float u1, float v1,
                    char scale_wh, char scale_xy)
{
    // Scale coordinates if requested
    if (scale_xy)
    {
        x = Screen_ToGLx(x);   // Screen_ToGLX
        y = Screen_ToGLy(y);   // Screen_ToGLY
    }
    if (scale_wh)
    {
        w = Screen_ToGLx(w);
        h = Screen_ToGLy(h);
    }

    Texture_BindLocalCached(id);

    // Build vertex + UV array (16 floats: 4 pairs of [u,v] + 4 pairs of [x,y])
    float gl_top    = (float)g_screen_height - y;
    float gl_bottom = gl_top - h;

    // UV quad: top-left, bottom-left, top-right, bottom-right
    float uvs[8] = {
        u0,       v0 + v1,   // vertex 0 texcoord
        u0,       v0,        // vertex 1 texcoord
        u0 + u1,  v0 + v1,   // vertex 2 texcoord
        u0 + u1,  v0,        // vertex 3 texcoord
    };
    float pos[8] = {
        x,     gl_top,       // vertex 0 position
        x,     gl_bottom,    // vertex 1 position
        x + w, gl_top,       // vertex 2 position
        x + w, gl_bottom,    // vertex 3 position
    };

    BufferedDraw::Begin(BufferedDraw::Primitive::QuadStrip);  // 6 = GL_QUAD_STRIP
    for (int i = 0; i < 4; i++)
    {
        BufferedDraw::TexCoord2f(uvs[i*2], uvs[i*2+1]);
        BufferedDraw::Vertex2f(pos[i*2], pos[i*2+1]);
    }
    BufferedDraw::End();
}


// ============================================================
// Texture_LoadFile  @ 0x00529130  (inner worker, 224 lines)
// Called by Texture_Load when g_tex_ext_mode == 0.
// Builds path from g_tex_base_dir + filename, opens file, decodes,
// uploads — same pipeline as Texture_Load but with explicit params:
//   param_1 = texture_id  (0x18 in practice = internal slot)
//   param_2 = extension   (ptr to extension string like ".ozj")
//   param_3 = path        (filename relative to base dir)
//   param_4 = extra_path  (NULL for simple load)
//   param_5 = flag
// Not called directly from game code outside Texture_Load.
// ============================================================
// (Implementation not reproduced — same pipeline as Texture_Load above)

// ── Pack HD ──────────────────────────────────────────────────────────────
// DESVIACION: con la opción de texturas HD, "Data\X\nombre.OZJ" se reemplaza en
// la GPU por "Data\HD\X\nombre.ozj.png" si existe. El PNG es la misma imagen (en
// el orden de filas que deja el decodificador) ampliada un factor entero k; se
// sube sobre un lienzo k veces el de potencia de 2 del original, así los UV no
// cambian. Medidas del slot y copia en CPU (Buffer) quedan las originales.
namespace {
bool HDPath(const char* fullPath, const char* format, char* out, size_t size)
{
    if (!fullPath || tolower((unsigned char)fullPath[0]) != 'd' || tolower((unsigned char)fullPath[1]) != 'a' ||
        tolower((unsigned char)fullPath[2]) != 't' || tolower((unsigned char)fullPath[3]) != 'a' ||
        (fullPath[4] != '\\' && fullPath[4] != '/')) return false;
    const char* dot = strrchr(fullPath, '.');
    if (!dot || size_t(dot - fullPath) + 16 > size) return false;
    // Conserva la extensión original (casa.ozj.png / casa.ozt.png): hay
    // texturas con el mismo nombre en los dos formatos.
    char ext[8]{};
    for (int i = 0; i < 7 && dot[1 + i]; ++i) ext[i] = char(tolower((unsigned char)dot[1 + i]));
    std::snprintf(out, size, "Data\\HD\\%.*s.%s.%s", int(dot - (fullPath + 5)), fullPath + 5, ext, format);
    return true;
}
} // namespace

// Reemplazo de archivos enteros por el pack HD: además de las texturas
// ampliadas (casa.ozj.jpg, arriba), Data\HD puede traer un archivo con el mismo
// nombre que el original, por ejemplo un modelo rehecho (Player\ArmorMale04.bmd)
// con texturas propias (Player\KMUA2_tex_mg004.OZJ). Como las texturas que pide
// un modelo se resuelven con la misma regla, las del pack se encuentran aunque
// el juego original no las tenga. Con la opción apagada no se mira Data\HD.
const char* Texture_HDAssetPath(const char* path, char* out, std::size_t size)
{
    if (!Gfx::HDTextures() || !path || !out || size == 0) return path;
    if (tolower((unsigned char)path[0]) != 'd' || tolower((unsigned char)path[1]) != 'a' ||
        tolower((unsigned char)path[2]) != 't' || tolower((unsigned char)path[3]) != 'a' ||
        (path[4] != '\\' && path[4] != '/')) return path;
    // Ya es una ruta del pack (Data\HD\...): no se anida.
    if (tolower((unsigned char)path[5]) == 'h' && tolower((unsigned char)path[6]) == 'd' &&
        (path[7] == '\\' || path[7] == '/')) return path;
    const int written = std::snprintf(out, size, "Data\\HD\\%s", path + 5);
    if (written <= 0 || size_t(written) >= size) return path;
    if (!Platform::GetFileSystem().Exists(out)) return path;
    static unsigned s_used = 0;
    if (++s_used <= 20 || s_used % 100 == 0) {
        char line[340];
        Platform::TextFormatTruncated(line, sizeof(line), Platform::TruncateText, "Pack HD: %s (%u archivos)", out, s_used);
        DbgLogPublic(line);
    }
    return out;
}

namespace {
// Decodifica la imagen HD y arma el lienzo; corre en un hilo aparte.
// Sin transparencia se acepta JPEG (casa.ozj.jpg), que ocupa varias veces
// menos que PNG; si no hay JPEG se busca el PNG.
struct HDJob {
    int id = 0; unsigned handle = 0; unsigned generation = 0;
    std::vector<unsigned char> bytes; bool jpeg = false;
    int width = 0, height = 0, paddedWidth = 0, paddedHeight = 0, channels = 0, maxSize = 0;
    int k = 0, canvasWidth = 0, canvasHeight = 0;
    std::vector<unsigned char> canvas;
    char path[300]{};
    char pngPath[300]{};
};
// Lee el archivo HD (JPEG si hay, si no PNG). Corre en el hilo de carga: en
// disco mecánico una lectura en frío tarda 10-50 ms y frenaba el juego.
bool ReadHD(HDJob& job)
{
    FILE* file = nullptr;
    if (job.path[0] && (file = Platform::GetFileSystem().Open(job.path, "rb"))) {
        job.jpeg = true;
    } else {
        file = Platform::GetFileSystem().Open(job.pngPath, "rb");
        if (!file) return false;
        memcpy(job.path, job.pngPath, sizeof(job.path));
    }
    long size = -1;
    if (fseek(file, 0, SEEK_END) == 0) size = ftell(file);
    if (size > 0 && fseek(file, 0, SEEK_SET) == 0) {
        job.bytes.resize(size_t(size));
        if (fread(job.bytes.data(), 1, size_t(size), file) != size_t(size)) job.bytes.clear();
    }
    fclose(file);
    return !job.bytes.empty();
}
bool BuildHD(HDJob& job)
{
    if (!ReadHD(job)) return false;
    ImageCodec::Image hd;
    const size_t length = job.bytes.size();
    job.bytes.resize(length + 2);   // espacio para el marcador final del JPEG
    const bool decoded = job.jpeg
        ? ImageCodec::DecodeBuffer(job.bytes.data(), length, ImageCodec::Format::Jpeg, job.maxSize, hd) && hd.channels == 3
        : ImageCodec::DecodePngBuffer(job.bytes.data(), length, job.channels, job.maxSize, hd);
    std::vector<unsigned char>().swap(job.bytes);
    if (!decoded || hd.width % job.width || hd.height % job.height) return false;
    const int k = hd.width / job.width;
    if (k < 2 || k > 8 || hd.height / job.height != k) return false;
    const long long pw = (long long)job.paddedWidth * k, ph = (long long)job.paddedHeight * k;
    if (pw > job.maxSize || ph > job.maxSize) return false;
    const int c = job.channels;
    job.canvas.assign(size_t(pw * ph * c), 0);
    for (int y = 0; y < hd.height; ++y)
        memcpy(job.canvas.data() + size_t(y) * size_t(pw) * c, hd.pixels + size_t(y) * hd.width * c, size_t(hd.width) * c);
    job.k = k; job.canvasWidth = int(pw); job.canvasHeight = int(ph);
    return true;
}

// DESVIACION: decodificar una HD frena entre 30 y 70 ms, y los modelos de
// NPC y monstruos se cargan recién cuando aparecen en pantalla, así que el
// juego se trababa en el medio del mapa. La textura original se sube en el
// momento y la HD la reemplaza unos frames después, cuando el hilo terminó.
class HDLoader {
public:
    ~HDLoader()
    {
        { std::lock_guard<std::mutex> lock(m_mutex); m_stop = true; }
        m_wake.notify_all();
        for (std::thread& t : m_threads) if (t.joinable()) t.join();
    }
    void Push(HDJob&& job)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_threads.empty()) {
                const unsigned cores = std::thread::hardware_concurrency();
                const unsigned count = cores > 4 ? 3 : cores > 2 ? 2 : 1;
                for (unsigned i = 0; i < count; ++i) m_threads.emplace_back([this] { Work(); });
            }
            m_pending.push_back(std::move(job));
        }
        m_wake.notify_all();
    }
    // Pasa a la GPU lo que ya terminó, con un tope de tiempo por frame.
    template <class Apply> void Drain(Apply apply)
    {
        const auto start = std::chrono::steady_clock::now();
        for (;;) {
            HDJob job;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_done.empty()) break;
                job = std::move(m_done.front());
                m_done.pop_front();
            }
            m_wake.notify_one();
            apply(job);
            if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(4)) break;
        }
    }
private:
    void Work()
    {
        for (;;) {
            HDJob job;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Mientras se carga un mapa no hay frames que suban lo
                // terminado: sin este tope los lienzos se apilaban en memoria
                // (cientos de MB) y todo el cliente se volvía lento.
                m_wake.wait(lock, [this] { return m_stop || (!m_pending.empty() && m_done.size() < MaxDone); });
                if (m_stop) return;
                job = std::move(m_pending.front());
                m_pending.pop_front();
            }
            if (!BuildHD(job)) continue;
            std::lock_guard<std::mutex> lock(m_mutex);
            m_done.push_back(std::move(job));
        }
    }
    static constexpr size_t MaxDone = 8;
    std::mutex m_mutex;
    std::condition_variable m_wake;
    std::deque<HDJob> m_pending, m_done;
    std::vector<std::thread> m_threads;
    bool m_stop = false;
};
HDLoader& Loader() { static HDLoader loader; return loader; }
// Una carga nueva en el mismo slot invalida la HD que estaba en camino.
std::vector<unsigned> s_hdGeneration;

// Encola la HD de la textura recién creada. El llamador sube la original
// igual: la HD la reemplaza cuando esté lista (Texture_PumpHD).
void UploadHD(int id, unsigned handle, const char* fullPath, int width, int height, int paddedWidth, int paddedHeight, int channels)
{
    if (id < 0) return;
    if (s_hdGeneration.size() <= size_t(id)) s_hdGeneration.resize(size_t(id) + 1, 0);
    ++s_hdGeneration[size_t(id)];
    if (!Gfx::HDTextures()) return;
    HDJob job;
    // El archivo lo abre y lo lee el hilo de carga (ReadHD), que lo cierra
    // enseguida: los trabajos en cola no tienen archivos abiertos (el CRT tiene
    // un tope de ~512 y al agotarse fallaba la apertura de Item.bmd).
    if (!HDPath(fullPath, "png", job.pngPath, sizeof(job.pngPath))) return;
    if (channels == 3) HDPath(fullPath, "jpg", job.path, sizeof(job.path));
    job.id = id; job.handle = handle; job.generation = s_hdGeneration[size_t(id)];
    job.width = width; job.height = height; job.paddedWidth = paddedWidth; job.paddedHeight = paddedHeight;
    job.channels = channels; job.maxSize = tex_max_size();
    Loader().Push(std::move(job));
}
}

void Texture_PumpHD()
{
    Loader().Drain([](HDJob& job) {
        // Si el slot se liberó o se recargó mientras tanto, la HD ya no corresponde.
        if (size_t(job.id) >= s_hdGeneration.size() || s_hdGeneration[size_t(job.id)] != job.generation) return;
        if (!Bitmaps[job.id].Buffer || Bitmaps[job.id].TextureNumber != job.handle) return;
        const unsigned previous = RenderState::Get().texture;
        Gfx::BindTexture(job.handle);
        Gfx::TexImage2D(job.channels, job.canvasWidth, job.canvasHeight,
                        job.channels == 4 ? Gfx::Format::RGBA : Gfx::Format::RGB, job.canvas.data());
        Gfx::BindTexture(previous);
        static unsigned loaded = 0;
        if (++loaded == 1 || loaded % 100 == 0) {
            char line[340];
            Platform::TextFormatTruncated(line, sizeof(line), Platform::TruncateText, "Texturas HD: %u cargadas (ultima %s, x%d)", loaded, job.path, job.k);
            DbgLogPublic(line);
        }
    });
}

// ── OpenJPG @ 0x00529740 — Texture_Load (OZJ/JPEG) ─────────────────────
// Loads JPEG or OZJ texture from disk, decompresses with stb_image, uploads to GL.
// Path mode:
//   DAT_0055a7c4 == 0 → full_path = g_tex_base_dir + filename
//   DAT_0055a7c4 != 0 → strip extension, try g_tex_ext_hq then g_tex_ext_lq
// OZJ files: fseek(f, 24, SEEK_SET) to skip 24-byte Webzen header before JPEG data.
// Limits: GL_MAX_TEXTURE_SIZE (tex_max_size), rounds to power-of-2 before GL upload.
int __cdecl OpenJPG(const char* path, int id, int filter, int wrap, int flags, char show_err)
{
    // --- Path construction ---
    char full_path[256];

    // Strip "Data\" / "Data/" prefix si el BMD lo guardó como
    // path absoluto desde root. Sin esto el path final queda
    // "Data\Data\Npc\foo.OZJ" y fopen falla.
    const char* nameForPath = path;
    if (path && (path[0] == 'D' || path[0] == 'd') &&
        (path[1] == 'a' || path[1] == 'A') &&
        (path[2] == 't' || path[2] == 'T') &&
        (path[3] == 'a' || path[3] == 'A') &&
        (path[4] == '\\' || path[4] == '/')) {
        nameForPath = path + 5;
    }

    if (DAT_0055a7c4 == '\0') {
        // Standard mode: base dir + filename
        strcpy(full_path, (const char*)DAT_0055a7a4);
        strcat(full_path, nameForPath);
    } else {
        // Extension-swap mode: strip ext from filename, append OZJ/OZT ext
        char basename[256];
        int dotPos = 0;
        int len = (int)strlen(nameForPath);
        for (int i = 0; i < len; i++) {
            basename[i] = nameForPath[i];
            if (nameForPath[i] == '.') { dotPos = i; break; }
        }
        basename[dotPos + 1] = '\0';
        // Build: base_dir + basename + extension
        strcpy(full_path, (const char*)DAT_0055a79c);
        strcat(full_path, basename);
        strcat(full_path, (const char*)DAT_00561b70);
    }

    // --- Open file ---
    // full_path queda con la ruta original: de ahí sale también la textura
    // ampliada del pack HD (UploadHD).
    char hdPath[300];
    FILE* f = Platform::GetFileSystem().Open(Texture_HDAssetPath(full_path, hdPath, sizeof(hdPath)), "rb");
    // DIAG: registrar cada intento. Cap general 60 + cap adicional para Logo/Ship
    // (los archivos que nos interesan para diagnosticar el login scene caen
    // después del cap y antes los perdíamos).
    if (f == NULL) {
        // DIAG: supprimido MessageBox/WM_DESTROY para no matar la ventana
        // durante diagnóstico. Log ya registró el path.
        return 0;
    }

    // Preserve the 24-byte OZJ prefix; decode fully before altering the slot.
    ImageCodec::Image image;
    const bool decoded = ImageCodec::Decode(f, 24, ImageCodec::Format::Jpeg, tex_max_size(), image);
    fclose(f);
    if (!decoded) return 0;
    const int img_w = image.width;
    const int img_h = image.height;
    const int pow2_w = ImageCodec::PaddedExtent(img_w, tex_max_size());
    const int pow2_h = ImageCodec::PaddedExtent(img_h, tex_max_size());

    // --- Setup texture slot (Bitmaps macro from structs.h) ---
    UnloadImage(id);  // Unload any existing texture in this slot

    ResourceTables::EnsureTextures(std::size_t(id)+1);
    BITMAP_t* slot = &Bitmaps[id];
    slot->Components = 3;          // RGB
    slot->Width  = (float)pow2_w;
    slot->Height = (float)pow2_h;

    // Allocate pixel buffer
    BYTE* pixels = (BYTE*)operator_new(pow2_h * pow2_w * 3);
    // DESVIACION CONSCIENTE vs IDA: el original NO inicializa este buffer
    // (OpenJPG 0x529740: `operator_new(3 * v14 * v15)` y despues solo rellena
    // output_width x output_height). Como las dimensiones se redondean a
    // potencia de 2, las columnas/filas de relleno quedan con memoria
    // indeterminada.
    //
    // Eso es visible: los UV de la UI mapean EXACTO al borde de la imagen
    // (p.ej. 0x12 = 0box.ozj, 134x30 -> 256x32, u1 = 0.5234375 -> 134.0), y a
    // escala 1.25 (800x600) o 2.5 (1600x1200) un borde entero cae en N+0.5, o
    // sea justo sobre el punto de muestreo del pixel N. GL_NEAREST resuelve
    // floor(u1 * TexW) = ancho de imagen = PRIMER texel de relleno, y lo dibuja
    // tal cual: una linea de 1 px con lo que hubiera en el heap.
    //
    // El comportamiento original se reprodujo y se confirmo contra el cliente
    // de referencia (mismas lineas en esas dos resoluciones, intermitentes).
    // Se inicializa a 0 para no leer memoria indeterminada y que el resultado
    // sea estable: en Release el heap suele venir a cero y la linea pasa
    // desapercibida, pero no siempre; en Debug el CRT rellena con 0xCD y sale
    // gris casi blanca en todas las texturas con relleno.
    //
    // Solo se sanea el buffer. Dimensiones, redondeo a potencia de 2 y calculo
    // de UV quedan intactos: la geometria no cambia.
    memset(pixels, 0, (size_t)pow2_w * pow2_h * 3);
    slot->Buffer = pixels;

    // Track VRAM usage (m_dwUsedTextureMemory)
    *(int*)&m_dwUsedTextureMemory += 3 * pow2_w * pow2_h;

    // JPEG row N remains row N. Padding stays zero.
    for (int y = 0; y < img_h; ++y)
        memcpy(pixels + y * pow2_w * 3, image.pixels + y * img_w * 3, img_w * 3);

    // --- Ref count increment ---
    slot->Ref++;

    // --- GL upload ---
    slot->TextureNumber = Gfx::CreateTexture();
    Gfx::BindTexture(slot->TextureNumber);
    UploadHD(id, slot->TextureNumber, full_path, img_w, img_h, pow2_w, pow2_h, 3);
    Gfx::TexImage2D(3, pow2_w, pow2_h, Gfx::Format::RGB, pixels);
    Gfx::TexParameter(Gfx::TexParam::MagFilter, Gfx::TexValueFromLegacy(filter));
    Gfx::TexParameter(Gfx::TexParam::MinFilter, Gfx::TexValueFromLegacy(filter));
    Gfx::TexParameter(Gfx::TexParam::WrapS, Gfx::TexValueFromLegacy(wrap));
    Gfx::TexParameter(Gfx::TexParam::WrapT, Gfx::TexValueFromLegacy(wrap));

    return 1;
}

// ── OpenTGA @ 0x00529BD0 — OpenTGA ──────────────────────────────────────
// Loads a custom TGA variant (32-bpp, 6-byte mini-header) from disk,
// performs BGR→RGB swap, uploads to GL, and stores metadata in the texture slot
// table (stride 0x38 bytes / 0xe floats, base DAT_083a7ca0).
//
// Header layout (at offset hdrOff in the raw file buffer):
//   +0  short  width
//   +2  short  height
//   +4  byte   pixel_depth (must be 0x20 = 32 bpp)
//   +5  byte   (unused / flags)
//   +6  bytes  BGRA pixel data, row 0 = bottom of image
//
// hdrOff = 0x0C when DAT_0055a7c4 == 0 (Data2 / pak mode)
//        = 0x10 when DAT_0055a7c4 != 0 (Data  / plain mode)
int __cdecl OpenTGA(const char* szFileName, int uiTextureIndex,
                         int uiFilter, int uiWrapMode, int bFullPath, char bCheck)
{
    char local_200[256];
    char local_100[256];

    // BMDs de NPC almacenan el nombre de textura como path
    // completo "Data\Npc\foo.OZT". Sin strip, la concatenación con base
    // "Data\" produce "Data\Data\Npc\foo.OZT" → fopen FAIL → NPCs blancos.
    // Si szFileName empieza con "Data\" o "Data/", strip ese prefijo.
    const char* nameForPath = szFileName;
    if (szFileName && (szFileName[0] == 'D' || szFileName[0] == 'd') &&
        (szFileName[1] == 'a' || szFileName[1] == 'A') &&
        (szFileName[2] == 't' || szFileName[2] == 'T') &&
        (szFileName[3] == 'a' || szFileName[3] == 'A') &&
        (szFileName[4] == '\\' || szFileName[4] == '/')) {
        nameForPath = szFileName + 5;
    }

    // ── Path construction ──────────────────────────────────────────────────────
    if (g_tex_ext_mode == '\0') {
        // Data2 mode: "Data2\" + szFileName  (extension already in szFileName)
        const char* base = (const char*)DAT_0055a7a4;   // "Data2\"
        int bl = (int)strlen(base);
        memcpy(local_200, base, bl);
        int fl = (int)strlen(nameForPath) + 1;
        memcpy(local_200 + bl, nameForPath, fl);
    } else {
        // Data mode: "Data\" + name-without-ext + DAT_00561ba8 (e.g. ".tga")
        int fl = (int)strlen(nameForPath);
        memcpy(local_100, nameForPath, fl + 1);
        // null-terminate at first '.'
        for (int i = 0; i < fl; i++) {
            if (local_100[i] == '.') { local_100[i + 1] = '\0'; break; }
        }
        const char* base = (const char*)DAT_0055a79c;   // "Data\"
        int bl = (int)strlen(base);
        memcpy(local_200, base, bl);
        int sl = (int)strlen(local_100);
        memcpy(local_200 + bl, local_100, sl);
        const char* ext = (const char*)DAT_00561ba8;      // ".tga"
        int el = (int)strlen(ext) + 1;
        memcpy(local_200 + bl + sl, ext, el);
    }

    // ── Open file ─────────────────────────────────────────────────────────────
    char hdPath[300];
    FILE* Stream = (FILE*)Platform::GetFileSystem().Open(Texture_HDAssetPath(local_200, hdPath, sizeof(hdPath)), DAT_005580ac);
    // DIAG: log TGA loads + specifically log slots 0xc/0x10/0x11 (Account textures)
    {
        static int s_tga = 0;
        bool isAcct = (uiTextureIndex == 0xc || uiTextureIndex == 0x10 ||
                       uiTextureIndex == 0x11);
        if (s_tga < 200 || isAcct) {
        }
    }
    if (!Stream) {
        // DIAG: suprimido MessageBox/SendMessage(WM_DESTROY) para no matar la
        // ventana durante diagnóstico. Log ya registró el path.
        return 0;
    }

    ImageCodec::Image image;
    const int prefix = (g_tex_ext_mode == '\0') ? 0 : 4;
    const bool decoded = ImageCodec::Decode(Stream, prefix, ImageCodec::Format::Tga32,
        tex_max_size(), image);
    fclose(Stream);
    if (!decoded) {
        char message[256];
        Platform::TextFormatTruncated(message, sizeof(message), Platform::TruncateText,
            "TGA_Fail id=0x%x (invalid, truncated, unsupported or oversized): %s",
            uiTextureIndex, local_200);
        DbgLogPublic(message);
        return 0;
    }
    const int width = image.width, height = image.height;
    const int pw = ImageCodec::PaddedExtent(width, tex_max_size());
    const int ph = ImageCodec::PaddedExtent(height, tex_max_size());
    {
        // ── Unload existing slot ──────────────────────────────────────────────
        ResourceTables::EnsureTextures(std::size_t(uiTextureIndex)+1);
        UnloadImage(uiTextureIndex);

        // ── Write slot metadata (stride 0x38 bytes = 0xe floats) ─────────────
        float* texWidth  = (float*) &DAT_083a7cc0;
        float* texHeight = (float*) &DAT_083a7cc4;
        char*  texBpp    = (char*)  &DAT_083a7cc8;
        UINT*  texGL     = (UINT*)  &DAT_083a7ccc;
        char*  texRef    = (char*)  &DAT_083a7cd0;
        int    byteStride = uiTextureIndex * TEXTURE_STRIDE;

        texBpp  [byteStride] = 4;                        // 4 components: RGBA
        texWidth [uiTextureIndex * TEXTURE_STRIDE_DWORDS] = (float)pw;
        texHeight[uiTextureIndex * TEXTURE_STRIDE_DWORDS] = (float)ph;

        BYTE* pixBuf = (BYTE*)operator_new(pw * ph * 4);
        // DESVIACION CONSCIENTE vs IDA: idem OpenJPG (ver el bloque largo en
        // OpenJPG). El original (OpenTGA 0x529BD0) tampoco inicializa:
        // `operator_new(4 * v21 * v22)` y luego rellena solo width x height.
        memset(pixBuf, 0, (size_t)pw * ph * 4);
        Bitmaps[uiTextureIndex].Buffer = pixBuf;
        *(int*)&m_dwUsedTextureMemory += 4 * pw * ph;

        // The codec preserves the legacy vertical flip and BGRA conversion.
        for (int row = 0; row < height; ++row)
            memcpy(pixBuf + row * pw * 4, image.pixels + row * width * 4, width * 4);

        // ── Increment ref count ───────────────────────────────────────────────
        texRef[byteStride]++;

        // ── Upload to OpenGL ──────────────────────────────────────────────────
        UINT glHandle = Gfx::CreateTexture();
        texGL[uiTextureIndex * TEXTURE_STRIDE_DWORDS] = glHandle;
        Gfx::BindTexture(glHandle);
        UploadHD(uiTextureIndex, glHandle, local_200, width, height, pw, ph, 4);
        Gfx::TexImage2D(4, pw, ph, Gfx::Format::RGBA, pixBuf);
        // GL_MODULATE = 0x2100 = 8448.0f. No usar 8192 (0x2000 = GL_NICEST): no es
        // válido para GL_TEXTURE_ENV_MODE → GL_INVALID_ENUM, que el driver NVIDIA
        // acumula y puede convertir en AV en una llamada GL siguiente.
        Gfx::TexEnvMode(Gfx::TexEnv::Modulate); // 0x2100 = 8448.0f
        Gfx::TexParameter(Gfx::TexParam::MinFilter, Gfx::TexValueFromLegacy(uiFilter));
        Gfx::TexParameter(Gfx::TexParam::MagFilter, Gfx::TexValueFromLegacy(uiFilter));
        Gfx::TexParameter(Gfx::TexParam::WrapS, Gfx::TexValueFromLegacy(uiWrapMode));
        Gfx::TexParameter(Gfx::TexParam::WrapT, Gfx::TexValueFromLegacy(uiWrapMode));
        return 1;
    }

}

// Scene/render helpers
// UnloadImage @ 0x0052A050 — Texture_FreeSlot
// Decrements ref-count at slot (stride 0x38); when reaches 0, calls glDeleteTextures
// and operator_delete on the pixel buffer.
// IDA: UnloadImage (0x0052A050)
void __cdecl UnloadImage(int param_1)
{
    ResourceTables::EnsureTextures(std::size_t(param_1)+1);
    int iVar1 = param_1 * TEXTURE_STRIDE;
    char  *texArr_cc8 = (char*) &DAT_083a7cc8;
    char  *texArr_cd0 = (char*) &DAT_083a7cd0;
    DWORD *texArr_ccc = (DWORD*)&DAT_083a7ccc;
    DWORD *texArr_ca0 = (DWORD*)&DAT_083a7ca0;
    if (!Bitmaps[param_1].Buffer) return;
    char cVar2 = texArr_cc8[iVar1];
    // __ftol calls were computing width/height — skip since we don't have the floats
    char *ref = texArr_cd0 + iVar1;
    if (*ref != '\0') {
        char cVar4 = *ref - 1;
        *ref = cVar4;
        if (cVar4 == '\0') {
            unsigned int glHandle = texArr_ccc[param_1 * TEXTURE_STRIDE_DWORDS];
            Gfx::DeleteTexture(glHandle);
            void *pixels = Bitmaps[param_1].Buffer;
            texArr_ca0[param_1 * TEXTURE_STRIDE_DWORDS] = 0;  // approximated from (&DAT_083a7ca0)[iVar1]
            operator_delete((unsigned char*)pixels);
            Bitmaps[param_1].Buffer = nullptr;
            (void)cVar2; // used in original for m_dwUsedTextureMemory -= cVar2 * w * h
        }
    }
}
