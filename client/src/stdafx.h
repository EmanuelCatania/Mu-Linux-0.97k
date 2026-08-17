#pragma once

// WinSock2 must come before windows.h
#include <winsock2.h>
#include <ws2tcpip.h>

// Win32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

// OpenGL (legacy fixed-function pipeline, same as original)
#include <gl/gl.h>
#include <gl/glu.h>

// CRT debug heap — enable BEFORE stdlib.h to track all malloc/new sites
// 2026-05-03: investigando crash heap corruption (addr 0x0054B54C, 0x005488CC).
// _CRTDBG_MAP_ALLOC redirige new/malloc al debug heap con file/line tracking.
// _CRTDBG_CHECK_ALWAYS_DF (set en WinMain) hace que CADA alloc valide TODO el
// heap antes/después → captura la corrupción en cuanto pasa.
#ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
#endif

// CRT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <crtdbg.h>

// libjpeg 6b (IJG)
// XMD_H prevents jmorecfg.h from redefining INT32 (already in basetsd.h)
#define XMD_H
#undef FAR
extern "C" {
#include "jpeglib.h"
}
#undef FAR
#define FAR

// Ghidra intrinsic stubs (not available in MSVC)
#define _rand() rand()
static inline long long __ftol() { return (long long)GetTickCount(); }
static inline double    fsin(double x) { return sin(x); }
#ifndef SQRT
#define SQRT sqrtf
#endif
#ifndef fcos
#define fcos cosf
#endif

// -----------------------------------------------------------------------
// Game state (DAT_005615c0)
// State 1 = Intro (Webzen logo)
// State 2 = Login
// State 3 = Loading
// State 4 = CharSelect
// State 5 = InGame
// -----------------------------------------------------------------------
extern int g_GameState;   // 0x005615c0
#define DAT_005615c0 g_GameState

// Global HWND (DAT_055c9ffc)
extern HWND g_hWnd;       // 0x055c9ffc
#define DAT_055c9ffc g_hWnd

// Global HINSTANCE
extern HINSTANCE g_hInst; // 0x055ca000
#define DAT_055ca000 g_hInst

// Window device context (OpenGL double-buffered window DC, target of SwapBuffers)
// Stored at DAT_055ca004 in original binary. NOT to be confused with the font
// memory DC at DAT_055c9fec (used by Font_BuildLayout for GDI text rendering
// into a DIB that later gets uploaded as a GL texture).
extern HDC g_hDC; // 0x055ca004
#define DAT_055ca004 ((DWORD)(uintptr_t)g_hDC)

// Font memory DC (GDI compatible DC with DIB bitmap selected, used to rasterize
// glyphs into a DIB surface). Assigned by Font_BuildLayout. Do NOT pass to
// SwapBuffers — it is an in-memory DC with no backing window surface.
extern HDC DAT_055c9fec;

// ── Primitive type aliases (Ghidra pseudo-types) ─────────────────────────────
#include "types.h"

// ── Game type definitions (structs, enums, constants) ────────────────────────
#include "structs.h"
#include "ghidra_compat.h"   // macros del decompile (LODWORD, qmemcpy, ...)

// ── All DAT_ globals and FUN_ forward declarations ──────────────────────────
#include "globals.h"
#include "functions.h"

// ── MuEmu auto-encrypt send() hook ──────────────────────────────────────────
// Redirect every send() call to our MuEmu_send_hook which auto-applies
// MuEmu::EncryptSend if the buffer starts with a plain MuEmu header byte.
// This is a SAFETY NET — explicit MuEmu::EncryptSend calls in send paths
// must still happen.  See src/Net/MuEmu.cpp for the wrapper body.
#include "Net/MuEmu.h"
#define send(s, buf, len, flags)  MuEmu_send_hook((s), (buf), (len), (flags))
