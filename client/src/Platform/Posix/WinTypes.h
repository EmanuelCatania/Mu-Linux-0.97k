#pragma once
#include <cstdint>
#include <cstddef>
#include "Platform/CompilerCompat.h"

// Sólo representación de datos heredados. No declara ni emula APIs Windows.
using BYTE = std::uint8_t;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using LONG = std::int32_t;
using ULONG = std::uint32_t;
using BOOL = std::int32_t;
using INT = std::int32_t;
using UINT = std::uint32_t;
using SHORT = std::int16_t;
using USHORT = std::uint16_t;
using CHAR = char;
using WCHAR = std::uint16_t;
using FLOAT = float;
using HRESULT = std::int32_t;
using INT_PTR = std::intptr_t;
using UINT_PTR = std::uintptr_t;
using LONG_PTR = std::intptr_t;
using ULONG_PTR = std::uintptr_t;
using DWORD_PTR = std::uintptr_t;
using WPARAM = UINT_PTR;
using LPARAM = LONG_PTR;
using LRESULT = LONG_PTR;
using SOCKET = UINT_PTR;
using LPVOID = void*;
using LPCVOID = const void*;
using LPSTR = char*;
using HPSTR = char*;
using LPCSTR = const char*;
using LPBYTE = BYTE*;
using LPDWORD = DWORD*;
using LPWORD = WORD*;
using LPBOOL = BOOL*;
using HANDLE = void*;
#define MU_OPAQUE_HANDLE(name) struct name##__; using name = name##__*
MU_OPAQUE_HANDLE(HWND);
MU_OPAQUE_HANDLE(HINSTANCE);
MU_OPAQUE_HANDLE(HDC);
MU_OPAQUE_HANDLE(HGLRC);
MU_OPAQUE_HANDLE(HFONT);
MU_OPAQUE_HANDLE(HBITMAP);
MU_OPAQUE_HANDLE(HMMIO);
MU_OPAQUE_HANDLE(HICON);
MU_OPAQUE_HANDLE(HCURSOR);
MU_OPAQUE_HANDLE(HBRUSH);
MU_OPAQUE_HANDLE(HMENU);
#undef MU_OPAQUE_HANDLE
using HMODULE = HINSTANCE;
using HGDIOBJ = void*;
struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSound3DBuffer;
struct IDirectSound3DListener;
using LPDIRECTSOUND = IDirectSound*;
using LPDIRECTSOUNDBUFFER = IDirectSoundBuffer*;
using LPDIRECTSOUND3DBUFFER = IDirectSound3DBuffer*;
using LPDIRECTSOUND3DLISTENER = IDirectSound3DListener*;
struct POINT { LONG x, y; };
struct SIZE { LONG cx, cy; };
using LPSIZE = SIZE*;
using tagSIZE = SIZE;
struct RECT { LONG left, top, right, bottom; };
union LARGE_INTEGER { std::int64_t QuadPart; };
struct TEXTMETRICA {
    LONG tmHeight, tmAscent, tmDescent, tmInternalLeading, tmExternalLeading;
    LONG tmAveCharWidth, tmMaxCharWidth, tmWeight, tmOverhang;
    LONG tmDigitizedAspectX, tmDigitizedAspectY;
    BYTE tmFirstChar, tmLastChar, tmDefaultChar, tmBreakChar;
    BYTE tmItalic, tmUnderlined, tmStruckOut, tmPitchAndFamily, tmCharSet;
};
#define CALLBACK __stdcall
#define WINAPI __stdcall
#define APIENTRY __stdcall
#define TRUE 1
#define FALSE 0
#define MAX_PATH 260
#define FAR
#define LOWORD(value) (static_cast<WORD>(static_cast<DWORD_PTR>(value) & 0xffffu))
#define HIWORD(value) (static_cast<WORD>((static_cast<DWORD_PTR>(value) >> 16) & 0xffffu))
#define LOBYTE(value) (static_cast<BYTE>(static_cast<DWORD_PTR>(value) & 0xffu))
#define HIBYTE(value) (static_cast<BYTE>((static_cast<DWORD_PTR>(value) >> 8) & 0xffu))
using WNDPROC = LRESULT (CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
static_assert(sizeof(void*) == 4, "El cliente requiere x86");
static_assert(sizeof(BYTE) == 1 && sizeof(WORD) == 2 && sizeof(WCHAR) == 2);
static_assert(sizeof(DWORD) == 4 && sizeof(LONG) == 4 && sizeof(BOOL) == 4);
static_assert(sizeof(UINT) == 4 && sizeof(HRESULT) == 4 && sizeof(FLOAT) == 4);
static_assert(sizeof(HWND) == 4 && sizeof(HDC) == 4 && sizeof(HFONT) == 4);
static_assert(sizeof(WPARAM) == 4 && sizeof(LPARAM) == 4 && sizeof(SOCKET) == 4);
static_assert(sizeof(POINT) == 8 && sizeof(SIZE) == 8 && sizeof(RECT) == 16);
static_assert(sizeof(LARGE_INTEGER) == 8 && sizeof(TEXTMETRICA) == 56);
static_assert(offsetof(TEXTMETRICA, tmFirstChar) == 44);
