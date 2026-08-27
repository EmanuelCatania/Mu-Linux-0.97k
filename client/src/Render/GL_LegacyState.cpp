// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// GL helpers — cached OpenGL state wrappers
// CORRECCION: 0xb71 es GL_DEPTH_TEST, NO GL_SCISSOR_TEST (que es 0x0c11).
// FUN_005114d0 @ 0x005114D0 — GL_EnableDepthTest
void __cdecl GL_EnableDepthTest(void) {
    if (DAT_083a411e == '\0') { DAT_083a411e = '\x01'; glEnable(0x0b71); }
}
// FUN_005114f0 @ 0x005114F0 — GL_DisableDepthTest
// DIAG: el AV NVIDIA aparece acá no por el glDisable per se, sino por corrupción previa
// del contexto GL que aflora al primer comando subsiguiente (driver bufferea cmds).
// Loggeamos antes/después con glGetError() para localizar el comando ofensivo.
void __cdecl GL_DisableDepthTest(void) {
    static int s_call = 0;
    s_call++;
    GLenum err_before = glGetError();
    if (err_before != 0) {
        char b[160];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "GL_DisableDepthTest #%d ENTER  glGetError(pre)=0x%x  cache=%d",
            s_call, (int)err_before, (int)DAT_083a411e);
        DbgLogPublic(b);
    }
    if (DAT_083a411e != '\0') { DAT_083a411e = '\0'; glDisable(0x0b71); }
    GLenum err_after = glGetError();
    if (err_after != 0) {
        char b[160];
        _snprintf_s(b, sizeof(b), _TRUNCATE,
            "GL_DisableDepthTest #%d EXIT   glGetError(post)=0x%x",
            s_call, (int)err_after);
        DbgLogPublic(b);
    }
}
// FUN_00511510 @ 0x00511510 — GL_EnableDepthWrites
void __cdecl GL_EnableDepthWrites(void) {
    if (DAT_083a42e8 == '\0') { DAT_083a42e8 = '\x01'; glDepthMask(1); }
}
// FUN_00511530 @ 0x00511530 — GL_DisableDepthWrites
void __cdecl GL_DisableDepthWrites(void) {
    if (DAT_083a42e8 != '\0') { DAT_083a42e8 = '\0'; glDepthMask(0); }
}
// FUN_00511550 @ 0x00511550 — GL_EnableCullFace
// (Called by 3D blend setters GL_ResetState / GL_EnableLightMap per 5.2 source pattern.)
void __cdecl GL_EnableCullFace(void) {
    if (DAT_083a411c == '\0') { DAT_083a411c = '\x01'; glEnable(0xb44); }
}
// FUN_00511570 @ 0x00511570 — GL_DisableCullFace
// (Called by 2D blend setters GL_SetBlendSrcOver / GL_SetBlendAdditive / GL_SetBlendSrcAlpha.)
void __cdecl GL_DisableCullFace(void) {
    if (DAT_083a411c != '\0') { DAT_083a411c = '\0'; glDisable(0xb44); }
}
// FUN_00511590 @ 0x00511590 — GL_SetAlphaTest
void __cdecl GL_SetAlphaTest(char param_1) {
    GL_EnableDepthWrites();
    if (param_1 == '\x01') {
        if (DAT_083a411d == '\0') { DAT_083a411d = '\x01'; glEnable(0xbc0); }
    } else {
        if (DAT_083a411d != '\0') { DAT_083a411d = '\0'; glDisable(0xbc0); }
    }
    if (DAT_083a4125 != '\0') { DAT_083a4125 = '\0'; glDisable(0xde1); }
}
// FUN_00511910 @ 0x00511910 — GL_SetViewport
void __cdecl GL_SetViewport(int param_1, int param_2, int param_3, int param_4) {
    DAT_083a4280 = (DWORD)param_3;
    DAT_083a42c8 = (DWORD)param_1;
    DAT_083a42cc = (DWORD)param_2;
    DAT_083a42b0 = (DWORD)param_4;
    glViewport(param_1, (int)(DAT_00561570 - param_2) - param_4, param_3, param_4);
}
// FUN_00511220 @ 0x00511220 — GL_SetPerspective
void __cdecl GL_SetPerspective(int fov, float aspect, int near_clip, float far_clip) {
    float fovF  = Ff(fov);
    float nearF = Ff(near_clip);
    gluPerspective((double)fovF, (double)aspect, (double)nearF, (double)far_clip);
    double tanHalfFov = tan((double)fovF * (double)_DAT_00552cc4);
    ViewportCenterX = DAT_083a4280 / 2 + DAT_083a42c8;
    ViewportCenterY = DAT_083a42b0 / 2 + DAT_083a42cc;
    _DAT_083a4294 = (float)((int)DAT_0056156c - (int)ViewportCenterY);
    _DAT_083a42a4 = (float)(tanHalfFov / (double)(DAT_083a4280 / 2) * (double)aspect);
    _DAT_083a42a8 = (float)(tanHalfFov / (double)(DAT_083a42b0 / 2)
                            * ((double)(int)DAT_00561570 / (double)(int)DAT_083a42b0));
}
// FUN_00511950 @ 0x00511950 — Screen_ToGLX
// FUN_00511980 @ 0x00511980 — Screen_ToGLY
// Formula: WindowWidth * x * (1/640)   and   WindowHeight * y * (1/480).
// Used by RenderBitmap, RenderColor, CreateFrustrum2D, SetPositionIME_Wnd, etc.
long double __cdecl Screen_ToGLX(float v) {
    return (long double)((float)(int)DAT_0056156c * v * _DAT_0055283c);
}
long double __cdecl Screen_ToGLY(float v) {
    return (long double)((float)(int)DAT_00561570 * v * _DAT_00552838);
}
// FUN_00529000 @ 0x00529000 — WriteJpeg(path, width, height, pixel_buf, quality)
// Writes RGB pixel buffer to JPEG file using libjpeg compression.
// Rows are written bottom-up (flipped) to match OpenGL framebuffer layout.
// Ghidra: local_1d8=3 (components), local_1d4=2 (JCS_RGB), quality from param_5.
unsigned int __cdecl FUN_00529000(const char* path, int width, int height, void* pixelBuf, int quality)
{
    FILE* f = fopen(path, "wb");
    if (f == NULL) return 0;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, f);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    int row_stride = width * 3;
    unsigned char* buf = (unsigned char*)pixelBuf;

    // Write rows bottom-up (flip for OpenGL framebuffer origin)
    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPROW row_pointer = buf + (height - 1 - cinfo.next_scanline) * row_stride;
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(f);
    jpeg_destroy_compress(&cinfo);
    return 1;
}
