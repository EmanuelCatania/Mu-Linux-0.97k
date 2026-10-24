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
#include "Render/Gfx.h"
#include "Render/ResourceTables.h"
#include "Render/BufferedDraw.h"
#include "Platform/Text.h"
#include "Platform/IWindow.h"
#include "Platform/IFileSystem.h"
#include "Texture.h"

#include "ImageCodec.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
    FILE* f = Platform::GetFileSystem().Open(fullPath, "rb");
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
    FILE* f = Platform::GetFileSystem().Open(fullPath, "rb");
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
    FILE* f = Platform::GetFileSystem().Open(full_path, "rb");
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
    FILE* Stream = (FILE*)Platform::GetFileSystem().Open(local_200, DAT_005580ac);
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
        UINT*  texPix    = (UINT*)  &DAT_083a7cd4;
        int    byteStride = uiTextureIndex * 0x38;

        texBpp  [byteStride] = 4;                        // 4 components: RGBA
        texWidth [uiTextureIndex * 0xe] = (float)pw;
        texHeight[uiTextureIndex * 0xe] = (float)ph;

        BYTE* pixBuf = (BYTE*)operator_new(pw * ph * 4);
        // DESVIACION CONSCIENTE vs IDA: idem OpenJPG (ver el bloque largo en
        // OpenJPG). El original (OpenTGA 0x529BD0) tampoco inicializa:
        // `operator_new(4 * v21 * v22)` y luego rellena solo width x height.
        memset(pixBuf, 0, (size_t)pw * ph * 4);
        texPix[uiTextureIndex * 0xe] = (UINT)(uintptr_t)pixBuf;
        *(int*)&m_dwUsedTextureMemory += 4 * pw * ph;

        // The codec preserves the legacy vertical flip and BGRA conversion.
        for (int row = 0; row < height; ++row)
            memcpy(pixBuf + row * pw * 4, image.pixels + row * width * 4, width * 4);

        // ── Increment ref count ───────────────────────────────────────────────
        texRef[byteStride]++;

        // ── Upload to OpenGL ──────────────────────────────────────────────────
        UINT glHandle = Gfx::CreateTexture();
        texGL[uiTextureIndex * 0xe] = glHandle;
        Gfx::BindTexture(glHandle);
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
    int iVar1 = param_1 * 0x38;
    DWORD *texArr_cd4 = (DWORD*)&DAT_083a7cd4;
    char  *texArr_cc8 = (char*) &DAT_083a7cc8;
    char  *texArr_cd0 = (char*) &DAT_083a7cd0;
    DWORD *texArr_ccc = (DWORD*)&DAT_083a7ccc;
    DWORD *texArr_ca0 = (DWORD*)&DAT_083a7ca0;
    if (texArr_cd4[param_1 * 0xe] == 0) return;
    char cVar2 = texArr_cc8[iVar1];
    // __ftol calls were computing width/height — skip since we don't have the floats
    char *ref = texArr_cd0 + iVar1;
    if (*ref != '\0') {
        char cVar4 = *ref - 1;
        *ref = cVar4;
        if (cVar4 == '\0') {
            unsigned int glHandle = texArr_ccc[param_1 * 0xe];
            Gfx::DeleteTexture(glHandle);
            void *pixels = (void*)(uintptr_t)texArr_cd4[param_1 * 0xe];
            texArr_ca0[param_1 * 0xe] = 0;  // approximated from (&DAT_083a7ca0)[iVar1]
            operator_delete((unsigned char*)pixels);
            texArr_cd4[param_1 * 0xe] = 0;
            (void)cVar2; // used in original for m_dwUsedTextureMemory -= cVar2 * w * h
        }
    }
}
