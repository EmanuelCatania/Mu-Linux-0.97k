#pragma once
#include <cstdarg>

#include "platform/PlatformHeaders.h"

// OpenGL no se incluye acá: sólo el backend, con Platform/GL/glad.h (3.3 core).

// CRT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "platform/DebugHeaders.h"

// Ghidra intrinsic stubs (not available in MSVC)
#define _rand() rand()
static inline double    fsin(double x) { return sin(x); }
#ifndef SQRT
#define SQRT sqrtf
#endif
#ifndef fcos
#define fcos cosf
#endif

// -----------------------------------------------------------------------
// Game state (SceneFlag)
// State 1 = Intro (Webzen logo)
// State 2 = Login
// State 3 = Loading
// State 4 = CharSelect
// State 5 = InGame
// -----------------------------------------------------------------------
extern int SceneFlag; // IDA: SceneFlag (0x005615C0)

// Ventana, instancia, DC de OpenGL (el que recibe SwapBuffers) y tamaño:
// CWindow (Core/Window.h), a través de gWindow. IDA: g_hWnd 0x055C9FFC,
// g_hInst 0x055CA000, g_hDC 0x055CA004, WindowWidth/Height 0x0056156C/70.
// Fuentes y DC de memoria del texto: CFont (Core/Font.h), a través de gFont.
// El DC de OpenGL NO es el DC de memoria del texto (gFont.GetTextDC()).
#include "app/core/Window.h"
#include "net/Network.h"
#include "app/core/Font.h"
#include "game/core/MapManager.h"

// ── Primitive type aliases (Ghidra pseudo-types) ─────────────────────────────
#include "types.h"
#include "game/entity/EntityTypes.h"
#include "assets/model/ModelTypes.h"
#include "ui/ChatTypes.h"
#include "render/JointTypes.h"

// ── Game type definitions (structs, enums, constants) ────────────────────────
#include "structs.h"
#include "ghidra_compat.h"   // macros del decompile (LODWORD, qmemcpy, ...)

// ── All DAT_ globals and FUN_ forward declarations ──────────────────────────
#include "globals.h"
#include "functions.h"

// ── Enganche de send() ──────────────────────────────────────────────────────
// Todo send() del cliente pasa por MuEmu_send_hook, y de ahí al canal cifrado
// (net/NetChannel.h): el envío tiene un solo camino.  Ver src/net/MuEmu.cpp.
#include "net/MuEmu.h"
#define send(s, buf, len, flags)  MuEmu_send_hook((s), (buf), (len), (flags))

#include "render/Gfx.h"
#include "render/RenderProfile.h"
