// Texture.cpp
// OZJ / OZT texture loading + OpenGL upload pipeline
//
// Entry points:
//   Texture_Load      @ 0x00529740  (280 lines) — load OZJ/OZT → GPU
//   Texture_Unload    @ 0x0052a050  ( 29 lines) — ref-counted free
//   GL_BindTexture    @ 0x00511480  ( 15 lines) — cached glBindTexture
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
//   DAT_083bb9d0   int   g_vram_used          running tally of texture memory (bpp*w*h bytes)
//
// ── FILE FORMAT ───────────────────────────────────────────────────────────────
//
//   OZJ = Webzen JPEG variant — libjpeg decode pipeline
//         Decoder objects: FUN_0052acc0, _0052ae70, _0052b540, _0052af50, _0052b2e0, _0052b4a0
//         (jpeg_create_decompress, jpeg_read_header, jpeg_start_decompress,
//          jpeg_read_scanlines, jpeg_finish_decompress / destroy)
//
//   OZT = alternate format, processed with FUN_00543037(file_ptr, 0x18, 0)
//         (likely a Webzen proprietary compressed texture, possibly zlib or RLE)
//
//   Both decode to: width × height × 3 bytes (GL_RGB, GL_UNSIGNED_BYTE)
//   Max supported size: 256 × 256  (sizes > 0x100 silently skipped)
//   Width and height are rounded up to the next power of 2 before GPU upload.
//
// ── TEXTURE PATH MODES ────────────────────────────────────────────────────────
//
//   DAT_0055a7c4 == 0  →  Standard mode:
//       Full path = DAT_0055a7a4 (base dir) + param_1 (filename)
//       FUN_00529130(id, extension, path, NULL, 0) handles the actual I/O.
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
//   param_10 (scale_wh): if set, w and h are scaled via FUN_00511950 / FUN_00511980
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
#include "Texture.h"

extern "C" {
#include "jpeglib.h"
}
#include <setjmp.h>
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

static TexSlot* TexTable = (TexSlot*)g_BitmapsRaw;

extern int  g_bound_texture_id;   // DAT_00561574
extern int  g_screen_height;      // DAT_00561570
extern int  g_vram_used;          // DAT_083bb9d0
// g_tex_ext_mode is declared via #define in globals.h as DAT_0055a7c4 (char)
extern char g_tex_base_dir[];     // DAT_0055a7a4  base directory string
extern char g_tex_ext_hq[];       // DAT_0055a79c  high-quality extension (e.g. ".OZJ")
extern char g_tex_ext_lq[];       // DAT_00561b70  low-quality extension  (e.g. ".OZT")
extern float g_scale_x;           // DAT_0056156c  screen scale X
extern float g_scale_y;           // DAT_0055283c  screen scale Y


// ============================================================
// GL_BindTexture  @ 0x00511480
// Wrapper around glBindTexture that avoids redundant state changes.
//   id >= 0 : look up gl_handle from texture table slot [id]
//   id <  0 : use -id directly as GL texture handle (raw GL object)
// ============================================================
void GL_BindTexture(int id)
{
    // ⚠ ESTE **NO** ES EL BIND QUE USA EL RENDER.
    //
    // El bind real del pipeline es `FUN_00511480` (0x00511480, en GL_State.cpp):
    // ése es el que llaman BMD_DrawMesh, los efectos, los sprites y el HUD.
    // Esta función es un equivalente funcional sin callers vivos — su único
    // caller es `Texture_Draw2D`, que a su vez tampoco tiene callers.
    //
    // Peor: mantiene su PROPIO cache (`g_bound_texture_id`) distinto del de
    // FUN_00511480 (`DAT_00561574`). Si algún día se la vuelve a usar, los dos
    // caches se desincronizan y se omiten binds → textura equivocada.
    //
    // Se conserva porque forma parte del port de este módulo, pero NO
    // instrumentar acá para diagnosticar el render: en la sesión del 2026-08-16
    // se puso un probe en esta función, dio 0 hits, y ese silencio se tomó como
    // evidencia de que las texturas estaban bien — costó una ronda entera.
    if (g_bound_texture_id == id)
        return;
    g_bound_texture_id = id;

    if (id >= 0)
        glBindTexture(GL_TEXTURE_2D, TexTable[id].gl_handle);
    else
        glBindTexture(GL_TEXTURE_2D, (UINT)(-id));
}


// ============================================================
// Texture_Unload  @ 0x0052a050
// Ref-counted texture free. Decrements ref_count; when it reaches 0:
//   glDeleteTextures, free pixel buffer, update g_vram_used.
// ============================================================
void Texture_Unload(int id)
{
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
            glDeleteTextures(1, &slot->gl_handle);
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
// Decode pipeline:
//   g_tex_ext_mode == 0: FUN_00529130(0x18, g_tex_ext_lq, path, NULL, 0)
//                        — inner loader, re-opens file internally
//   g_tex_ext_mode != 0: FUN_00543037(file_ptr, 0x18, 0)
//                        — alternate format decoder (OZT / proprietary)
//   Both eventually call libjpeg-style functions to produce RGB scanlines.
//
// After decode, if width <= 256 and height <= 256:
//   1. Round width and height up to the next power of 2.
//   2. Texture_Unload(id)  — free any existing slot data.
//   3. Set slot: bpp=3, width=pow2_w, height=pow2_h.
//   4. malloc(pow2_w * pow2_h * 3) → slot.pixels.
//   5. Read scanlines into pixel buffer (upward from row 0).
//   6. ref_count++.
//   7. glGenTextures, glBindTexture, glTexImage2D(GL_RGB, GL_UNSIGNED_BYTE).
//   8. glTexParameteri × 4 (min, mag, wrap_s, wrap_t).
//   9. g_vram_used += bpp * pow2_w * pow2_h.
// ============================================================
// libjpeg error handler — longjmp out instead of exit()
struct tex_jerr_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};
static void tex_jpeg_error_exit(j_common_ptr cinfo)
{
    longjmp(((tex_jerr_mgr*)cinfo->err)->setjmp_buffer, 1);
}

// Swap file extension in-place: "Local/Webzenlogo.jpg" -> "Local/Webzenlogo.OZJ"
static void tex_swap_ext(char* path, const char* newExt /*"OZJ"/"OZT"*/)
{
    char* dot = strrchr(path, '.');
    if (!dot) { strcat(path, "."); strcat(path, newExt); return; }
    dot[1] = newExt[0]; dot[2] = newExt[1]; dot[3] = newExt[2]; dot[4] = '\0';
}

static int tex_next_pow2(int v, int maxv)
{
    int i = 1;
    while (i < v && i < maxv) i <<= 1;
    return i;
}

// Internal: decode OZJ (JPEG with 24-byte prefix) into TexSlot + GPU upload
static int tex_load_ozj(const char* fullPath, int id, int min_filt, int wrap)
{
    FILE* f = fopen(fullPath, "rb");
    if (!f) return 0;

    // Skip 24-byte dump header (OZJ prefix)
    fseek(f, 24, SEEK_SET);

    struct jpeg_decompress_struct cinfo;
    tex_jerr_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = tex_jpeg_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(f);
        return 0;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, f);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if (cinfo.output_width > 1024 || cinfo.output_height > 1024) {
        jpeg_destroy_decompress(&cinfo);
        fclose(f);
        return 0;
    }

    int w = tex_next_pow2((int)cinfo.output_width,  1024);
    int h = tex_next_pow2((int)cinfo.output_height, 1024);

    // DIAG: log actual dimensions and decode success
    {
        char dbg[256];
        _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
            "OZJ_Decode id=0x%x file='%s' jpg=%dx%d pow2=%dx%d components=%d",
            id, fullPath, (int)cinfo.output_width, (int)cinfo.output_height,
            w, h, (int)cinfo.output_components);
        DbgLogPublic(dbg);
    }

    TexSlot* slot = &TexTable[id];
    Texture_Unload(id);   // free any previous slot pixels + GL handle

    slot->active    = 1;
    slot->bpp       = 3;
    slot->width     = (float)w;
    slot->height    = (float)h;
    slot->pixels    = (void*)new BYTE[w * h * 3];
    memset(slot->pixels, 0, w * h * 3);
    g_vram_used    += 3 * w * h;

    int row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY rowBuf = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    int bufSize = w * h * 3;
    int offset  = 0;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        if (offset + row_stride > bufSize) break;
        jpeg_read_scanlines(&cinfo, rowBuf, 1);
        memcpy((BYTE*)slot->pixels + (cinfo.output_scanline - 1) * w * 3,
               rowBuf[0], row_stride);
        offset += row_stride;
    }

    slot->ref_count++;
    glGenTextures(1, &slot->gl_handle);
    glBindTexture(GL_TEXTURE_2D, slot->gl_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, slot->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, min_filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     wrap);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(f);
    return 1;
}

// Internal: decode OZT (TGA 32-bit with 4-byte prefix) into TexSlot + GPU upload
static int tex_load_ozt(const char* fullPath, int id, int min_filt, int wrap)
{
    FILE* f = fopen(fullPath, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 20) { fclose(f); return 0; }

    BYTE* pak = new BYTE[size];
    fread(pak, 1, size, f);
    fclose(f);

    // 4-byte OZT prefix + TGA header (12 bytes skipped, then w/h/bpp)
    int idx = 12 + 4;
    short nx  = *(short*)(pak + idx); idx += 2;
    short ny  = *(short*)(pak + idx); idx += 2;
    char  bit = *(char*) (pak + idx); idx += 1;
    idx += 1;

    // BUG-FIX (2026-04-21): mismo issue que en OZJ — 256 era muy bajo.
    if (bit != 32 || nx > 1024 || ny > 1024) {
        delete[] pak;
        return 0;
    }

    int w = tex_next_pow2(nx, 1024);
    int h = tex_next_pow2(ny, 1024);

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

    // BGRA → RGBA + vertical flip
    BYTE* dstBase = (BYTE*)slot->pixels;
    for (int y = 0; y < ny; y++) {
        BYTE* src = pak + idx;   idx += nx * 4;
        BYTE* dst = dstBase + (ny - 1 - y) * w * 4;
        for (int x = 0; x < nx; x++) {
            dst[0] = src[2]; dst[1] = src[1]; dst[2] = src[0]; dst[3] = src[3];
            src += 4; dst += 4;
        }
    }
    delete[] pak;

    slot->ref_count++;
    glGenTextures(1, &slot->gl_handle);
    glBindTexture(GL_TEXTURE_2D, slot->gl_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, slot->pixels);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, min_filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     wrap);
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
        _snprintf_s(full, _TRUNCATE, "Data\\%s", path);
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
        _snprintf_s(msg, _TRUNCATE, "File not found:\n%s", full);
        extern HWND g_hWnd;
        MessageBoxA(g_hWnd, msg, "IError", MB_OK);
        if (g_hWnd) SendMessageA(g_hWnd, WM_CLOSE, 0, 0);
    }
    return ok;
}


// ============================================================
// Texture_Draw2D  @ 0x005125a0
// Draw a textured screen-space quad.
//
//   id         — texture slot (passed to GL_BindTexture)
//   x, y       — screen position of top-left corner (pixels, Y from top)
//   w, h       — width and height (pixels)
//   u0, v0     — top-left UV  (normalized 0.0-1.0)
//   u1, v1     — bottom-right UV delta (added to u0/v0)
//   scale_wh   — if non-0: scale w,h via screen→GL coord transform
//   scale_xy   — if non-0: scale x,y via screen→GL coord transform
//
// Coordinate transform (FUN_00511950 / FUN_00511980):
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
        x = Screen_ToGLx(x);   // FUN_00511950
        y = Screen_ToGLy(y);   // FUN_00511980
    }
    if (scale_wh)
    {
        w = Screen_ToGLx(w);
        h = Screen_ToGLy(h);
    }

    GL_BindTexture(id);

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

    glBegin(GL_QUAD_STRIP);  // 6 = GL_QUAD_STRIP
    for (int i = 0; i < 4; i++)
    {
        glTexCoord2f(uvs[i*2], uvs[i*2+1]);
        glVertex2f(pos[i*2], pos[i*2+1]);
    }
    glEnd();
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

// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 4805-5199 (395 lines)
// FUN_00529740 (Texture_Load OZJ/JPEG raw), FUN_00529bd0 (OpenTGA), FUN_0052a050 (Texture_FreeSlot)
// =============================================================================
// ── FUN_00529740 @ 0x00529740 — Texture_Load (OZJ/JPEG) ─────────────────────
// Loads JPEG or OZJ texture from disk, decompresses with libjpeg, uploads to GL.
// Path mode:
//   DAT_0055a7c4 == 0 → full_path = g_tex_base_dir + filename
//   DAT_0055a7c4 != 0 → strip extension, try g_tex_ext_hq then g_tex_ext_lq
// OZJ files: fseek(f, 24, SEEK_SET) to skip 24-byte Webzen header before JPEG data.
// Limits: 256x256 max, rounds to power-of-2 before GL upload.
int __cdecl FUN_00529740(const char* path, int id, int filter, int wrap, int flags, char show_err)
{
    // --- Path construction ---
    char full_path[256];

    // 2026-05-05: Strip "Data\" / "Data/" prefix si el BMD lo guardó como
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
    FILE* f = fopen(full_path, "rb");
    // DIAG: registrar cada intento. Cap general 60 + cap adicional para Logo/Ship
    // (los archivos que nos interesan para diagnosticar el login scene caen
    // después del cap y antes los perdíamos).
    if (f == NULL) {
        // DIAG: supprimido MessageBox/WM_DESTROY para no matar la ventana
        // durante diagnóstico. Log ya registró el path.
        return 0;
    }

    // --- OZJ header (skip 24 bytes) ---
    // NOTA / BUG PENDIENTE (sistema de texturas): varios .OZJ (Effect\Spark02.OZJ,
    // Local\Webzenlogo.OZJ, ships, logos) son JPEG PLANO de Photoshop (SOI 0xFFD8
    // sin header OZJ) con estructura: THUMBNAIL RGB embebido en APP1/APP13 +
    // imagen PRINCIPAL en CMYK. El skip-24 a ciegas hace que libjpeg lea el SOI del
    // THUMBNAIL (RGB, con color) — por eso los ships/logos se ven bien pero Spark02
    // queda 4x4 (su thumbnail es diminuto). Intentos previos:
    //   - Detectar SOI + leer imagen principal → crash (CMYK, 4 comp, overflow).
    //   - + JCS_RGB → sin crash pero TODO gris (CMYK Adobe invertido mal convertido).
    // Solución correcta pendiente: leer el thumbnail RGB SIEMPRE (que es lo que hace
    // el skip-24 por casualidad), o portar el decode CMYK-Adobe fiel. Por ahora se
    // mantiene skip-24 (colores OK); Spark02 queda chico como efecto secundario.
    fseek(f, 0x18, SEEK_SET);

    // --- libjpeg decompress ---
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, f);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    unsigned int img_w = cinfo.output_width;
    unsigned int img_h = cinfo.output_height;
    int components = cinfo.output_components;

    // --- Size limit: 1024x1024 max ---
    // sky0/sky5 son 512x256, se rechazaban con el límite de 256 y las BMD
    // del sky quedaban con GL handle 0 → default white. Subido a 1024.
    if (img_w > 1024 || img_h > 1024) {
        jpeg_destroy_decompress(&cinfo);
        fclose(f);
        return 0;
    }

    // --- Power-of-2 rounding ---
    int pow2_w = 1;
    while (pow2_w < (int)img_w && pow2_w < 1024) pow2_w *= 2;
    int pow2_h = 1;
    while (pow2_h < (int)img_h && pow2_h < 1024) pow2_h *= 2;

    // --- Setup texture slot (Bitmaps macro from structs.h) ---
    FUN_0052a050(id);  // Unload any existing texture in this slot

    BITMAP_t* slot = &Bitmaps[id];
    slot->Components = 3;          // RGB
    slot->Width  = (float)pow2_w;
    slot->Height = (float)pow2_h;

    // Allocate pixel buffer
    BYTE* pixels = (BYTE*)operator_new(pow2_h * pow2_w * 3);
    slot->Buffer = pixels;

    // Track VRAM usage (DAT_083bb9d0)
    *(int*)&DAT_083bb9d0 += 3 * pow2_w * pow2_h;

    // --- Read scanlines ---
    int row_stride = components * img_w;
    JSAMPARRAY row_buf = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned int scanline = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, row_buf, 1);
        // Copy scanline into pixel buffer (scanline N → row N)
        BYTE* dst = pixels + scanline * pow2_w * 3;
        memcpy(dst, row_buf[0], row_stride);
    }

    // --- Ref count increment ---
    slot->Ref++;

    // --- GL upload ---
    glGenTextures(1, &slot->TextureNumber);
    glBindTexture(GL_TEXTURE_2D, slot->TextureNumber);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, pow2_w, pow2_h, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    // --- Cleanup ---
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(f);
    return 1;
}

// ── FUN_00529bd0 @ 0x00529BD0 — OpenTGA ──────────────────────────────────────
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
int __cdecl FUN_00529bd0(const char* szFileName, int uiTextureIndex,
                         int uiFilter, int uiWrapMode, int bFullPath, char bCheck)
{
    char local_200[256];
    char local_100[256];

    // 2026-05-05: BMDs de NPC almacenan el nombre de textura como path
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
    FILE* Stream = (FILE*)fopen(local_200, DAT_005580ac);
    // DIAG: log TGA loads + specifically log slots 0xc/0x10/0x11 (Account textures)
    {
        static int s_tga = 0;
        bool isAcct = (uiTextureIndex == 0xc || uiTextureIndex == 0x10 ||
                       uiTextureIndex == 0x11);
        if (s_tga < 200 || isAcct) {
            char m[512];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "TGA_Load[%d] id=0x%x '%s' → %s",
                s_tga, uiTextureIndex, local_200, (Stream ? "OK-open" : "FAIL-fopen"));
            DbgLogPublic(m); s_tga++;
        }
    }
    if (!Stream) {
        // DIAG: suprimido MessageBox/SendMessage(WM_DESTROY) para no matar la
        // ventana durante diagnóstico. Log ya registró el path.
        return 0;
    }

    // ── Read entire file into buffer ──────────────────────────────────────────
    FUN_00543037((int*)Stream, 0, 2);            // fseek SEEK_END
    unsigned int uVar5 = (unsigned int)FUN_00542eb4((char*)Stream); // ftell
    FUN_00543037((int*)Stream, 0, 0);            // fseek SEEK_SET
    BYTE* PakBuffer = (BYTE*)operator_new(uVar5);
    fread(PakBuffer, 1, uVar5, Stream);
    fclose(Stream);

    // hdrOff: 0x0C for Data2 (pak with 12-byte header), 0x10 for Data (16-byte)
    int hdrOff = (g_tex_ext_mode == '\0') ? 0x0c : 0x10;

    // ── Validate header ───────────────────────────────────────────────────────
    int width  = (int)(*(short*)(PakBuffer + hdrOff));
    int height = (int)(*(short*)(PakBuffer + hdrOff + 2));
    BYTE depth = PakBuffer[hdrOff + 4];

    // DIAG: log dimensions/depth for account slots to see if they pass validation
    // [DIAG TEMP #4] ungate: ver header real de TODAS las OZT (chair2 etc.). REMOVER al cerrar #4.
    {
        static int s_tgahdr = 0;
        if (s_tgahdr < 250) { s_tgahdr++;
            char m[256];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "TGA_Hdr id=0x%x w=%d h=%d depth=0x%x mode=%d hdrOff=0x%x",
                uiTextureIndex, width, height, depth, (int)g_tex_ext_mode, hdrOff);
            DbgLogPublic(m);
        }
    }
    // [FIX #4 2026-06-30] Límite de tamaño <=256 removido — match companion-DLL
    // Patchs.cpp "Remove TGA size limit" (NOPea el size-check + fuerza los jumps
    // de width/height). Las object textures del mundo (chair2.OZT etc.) son >256,
    // el límite original las rechazaba → renderizaban cyan (textura sin subir).
    // Se mantiene `depth == 0x20` (32bpp) — el patch tampoco lo toca.
    if (depth == 0x20) {
        // Round up to next power-of-2 (max 256)
        int pw = 1; while (pw < width)  pw <<= 1;
        int ph = 1; while (ph < height) ph <<= 1;

        // ── Unload existing slot ──────────────────────────────────────────────
        FUN_0052a050(uiTextureIndex);

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
        texPix[uiTextureIndex * 0xe] = (UINT)(uintptr_t)pixBuf;
        *(int*)&DAT_083bb9d0 += 4 * pw * ph;

        // ── BGR(A) → RGBA copy, TGA row-0 = bottom → flip vertically ─────────
        BYTE* src = PakBuffer + hdrOff + 6;
        for (int row = 0; row < height; row++) {
            int   dstRow = height - 1 - row;            // vertical flip
            BYTE* dst    = pixBuf + 4 * dstRow * pw;
            for (int col = 0; col < width; col++) {
                dst[0] = src[2];   // R ← B
                dst[1] = src[1];   // G ← G
                dst[2] = src[0];   // B ← R
                dst[3] = src[3];   // A ← A
                dst += 4;
                src += 4;
            }
        }

        operator_delete(PakBuffer);

        // ── Increment ref count ───────────────────────────────────────────────
        texRef[byteStride]++;

        // ── Upload to OpenGL ──────────────────────────────────────────────────
        UINT glHandle = 0;
        glGenTextures(1, &glHandle);
        texGL[uiTextureIndex * 0xe] = glHandle;
        glBindTexture(GL_TEXTURE_2D, glHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, pw, ph, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixBuf);
        // BUG-FIX: el valor original era 8192.0f. GL_MODULATE = 0x2100 = 8448.0f.
        // 8192 = 0x2000 = GL_NICEST, que NO es un valor válido para
        // GL_TEXTURE_ENV_MODE → setea glGetError = GL_INVALID_ENUM (0x500),
        // que el driver NVIDIA acumula y eventualmente convierte en AV en una
        // llamada GL siguiente (visto en FUN_005114f0 → glDisable).
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (float)GL_MODULATE); // 0x2100 = 8448.0f
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);
        // [DIAG TEMP #4] ungate: ver qué OZT suben realmente (chair2=0x348 etc.). REMOVER al cerrar #4.
        {
            static int s_tgaup = 0;
            if (s_tgaup < 250) { s_tgaup++;
                char m[256];
                GLenum e = glGetError();
                BYTE* cp = pixBuf + (4 * (ph/2) * pw) + 4*(pw/2);
                _snprintf_s(m, sizeof(m), _TRUNCATE,
                    "TGA_Up id=0x%x gl=0x%x pw=%d ph=%d centerPix=(%d,%d,%d,%d) glErr=0x%x",
                    uiTextureIndex, glHandle, pw, ph, cp[0], cp[1], cp[2], cp[3], e);
                DbgLogPublic(m);
            }
        }
        return 1;
    }

    // [DIAG TEMP #4] ungate: cualquier textura aún rechazada (ej. depth!=0x20). REMOVER al cerrar #4.
    {
        static int s_tgafail = 0;
        if (s_tgafail < 60) { s_tgafail++;
            char m[256];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "TGA_Fail id=0x%x (rejected: w=%d h=%d depth=0x%x)",
                uiTextureIndex, width, height, depth);
            DbgLogPublic(m);
        }
    }
    operator_delete(PakBuffer);
    return 0;
}

// Scene/render helpers
// FUN_0052a050 @ 0x0052A050 — Texture_FreeSlot
// Decrements ref-count at slot (stride 0x38); when reaches 0, calls glDeleteTextures
// and operator_delete on the pixel buffer.
void __cdecl FUN_0052a050(int param_1)
{
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
            GLuint glHandle = texArr_ccc[param_1 * 0xe];
            glDeleteTextures(1, &glHandle);
            void *pixels = (void*)(uintptr_t)texArr_cd4[param_1 * 0xe];
            texArr_ca0[param_1 * 0xe] = 0;  // approximated from (&DAT_083a7ca0)[iVar1]
            operator_delete((unsigned char*)pixels);
            texArr_cd4[param_1 * 0xe] = 0;
            (void)cVar2; // used in original for DAT_083bb9d0 -= cVar2 * w * h
        }
    }
}
