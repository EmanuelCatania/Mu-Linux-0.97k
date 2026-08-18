// stubs.cpp
// Stub implementations for FUN_XXXXXXXX functions not yet decompiled,
// plus named-function wrappers and missing global definitions.
//
// Each FUN_ stub corresponds to a known address in the original binary.
// These will be replaced with real decompiled implementations as work progresses.
//
// ── Anti-tamper pattern (B5 review) ──────────────────────────────────────────
// The original binary interleaves ref-count operations on a global hash table
// (DAT_055c9bc8) around every XOR/packet path. The ops are:
//   FUN_00403f80  — HashTable_Insert (__thiscall)
//   FUN_00404280  — HashTable_GetValue
//   FUN_00404330  — HashTable_CloneEntry  (NOTE: CLAUDE.md incorrectly labels
//                    this as Packet_DecryptByte — it is a hash op.)
//   FUN_004041e0  — HashTable_Rehash
//   FUN_00404370  — HashTable_DecRef
//   FUN_00404400  — HashTable_Destroy
// Real implementations live in src/Net/Crypto.cpp with null-guards, so every
// call site in this file resolves to a safe no-op when the table isn't
// constructed. Do NOT remove these call sites blindly — they are wrapped in
// `if (uVar4 != 0xffffffff) {...}` blocks whose *else* arms contain the real
// work. The pattern is tolerated, not loved.

#include "stdafx.h"
#include <stdarg.h>
#include <intrin.h>   // _ReturnAddress
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ==== IDA-Hex-Rays intrinsic shims (auto-generated) ====
#ifndef _IDA_INTRINSICS_SHIMS_
#define _IDA_INTRINSICS_SHIMS_
#include <string.h>
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SHIBYTE(x)           (*(((char*)&(x))+1))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif
#define COERCE_FLOAT(x)      (*(float*)&(x))
#define COERCE_GLFLOAT(x)    (*(float*)&(x))
#define __PAIR64__(hi,lo)    (((unsigned __int64)(DWORD)(hi)<<32)|(DWORD)(lo))
#define MEMORY               ((BYTE*)0)
#ifndef qmemcpy
#define qmemcpy              memcpy
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif
#define delete__(p)          FUN_0054158c((unsigned char*)(p))
#endif // _IDA_INTRINSICS_SHIMS_


// Forward decl a nivel de archivo (extern "C" no puede ir dentro de funciones en MSVC)
extern "C" { void DbgLogPublic(const char* msg); }

// Forward decls for symbols that live in moved sibling files but are still
// referenced from stubs.cpp.
extern void FUN_004fa5a0(void);  // → Scene/Scene_CharSelect_Nav.cpp

// ── Missing globals (extern-declared in various .cpp but never defined) ──────
// Trade.cpp
BYTE  g_trade_active   = 0;   // DAT_05826d30
BYTE  g_duel_mode      = 0;   // DAT_05826d31
BYTE  g_trade_param    = 0;   // DAT_05826d32
BYTE  g_trade_accepted = 0;   // DAT_05826c04
DWORD g_trade_requester= 0;   // DAT_05826c00
char  g_active_shop_name[4]= {};  // lpString_05826bfc
int   g_cached_shop_slot   = 0;   // DAT_00559684
BYTE* g_EntityBase    = nullptr;  // DAT_07abf5d0
DWORD g_LocalEntity   = 0;        // DAT_07abf5d8

// Sound.cpp — real DirectSound8 globals live in globals.cpp:
//   g_EnableSound, g_Enable3DSound, g_lpDS, g_lpDS3DListener, wavefile,
//   g_dwBufferBytes, g_lpDSBuffer[420][4], g_lpDS3DBuffer[420][4],
//   MaxBufferChannel[], BufferChannel[], Enable3DSound[], BufferName[][].

// Texture.cpp
int g_bound_texture_id = 0;   // DAT_00561574
int g_screen_height    = 480; // DAT_00561570
int g_vram_used        = 0;   // DAT_083bb9d0

// Party.cpp
BYTE* g_PartyHPTable   = nullptr; // DAT_07e11e98

// ── CRT wrappers (these are real implementations) ────────────────────────────

// FUN_005416bc — crt_sprintf (MSVC CRT sprintf stub)
int __cdecl FUN_005416bc(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vsprintf(buf, fmt, args);
    va_end(args);
    return r;
}

// FUN_0054158c — operator_delete (free)
void __cdecl FUN_0054158c(void* ptr) {
    free(ptr);
}

// ── Entity_FindById @ 0x0045ac80 ─────────────────────────────────────────────
// Linear scan of entity array (stride 0x394, count 400) for entity_id at +476.
// Returns slot index (0-399), or 400 if not found.  Per IDA FindCharacterIndex
// (50-byte original): also requires the active flag at +0 to be non-zero, AND
// returns 400 (sentinel) on not-found — NOT 0 (slot 0 is the local player).
// 2026-05-07: fixed to match IDA — previously returned 0 on miss + ignored the
// active flag, which caused PacketHandler_0x5c writes for unknown entities to
// land on (potentially NULL or player) slot 0.
int __cdecl Entity_FindById(int entity_id) {
    BYTE* base = (BYTE*)DAT_07abf5d0;
    if (!base) return 400;
    for (int i = 0; i < 400; i++) {
        BYTE* ent = base + i * 0x394;
        if (*(BYTE*)ent != 0 && *(short*)(ent + 476) == (short)entity_id)
            return i;
    }
    return 400;
}

// HashTable_GetIndex is inline in functions.h — no body here.

// ── Net_Disconnect @ 0x0043dc90 ──────────────────────────────────────────────
// Closes the game socket and resets connection state.
int __cdecl Net_Disconnect(int ctx) {
    (void)ctx;
    if (DAT_055ca168 != (DWORD)INVALID_SOCKET) {
        closesocket((SOCKET)DAT_055ca168);
        DAT_055ca168 = (DWORD)INVALID_SOCKET;
    }
    return 0;
}

// ── GL_PopMatrixAll ───────────────────────────────────────────────────────────
// BUG-FIX CRÍTICO: antes hacía un solo glPopMatrix() asumiendo modo actual.
// Pero FUN_005119b0 (GL_SetupView) pushea DOS matrices (PROJECTION + MODELVIEW),
// y Scene_Login sólo hace un glPopMatrix antes de Begin2D. Resultado: cada frame
// quedaba un push acumulado en PROJECTION → stack overflow tras 2 frames →
// matrices corruptas → UI 2D invisible. Aquí forzamos reset completo de ambos
// stacks a identidad. Los glGetError() limpian el GL_STACK_UNDERFLOW que
// generan los pops sobrantes (son inocuos, sólo setean el flag de error).
unsigned int __cdecl GL_PopMatrixAll(void) {
    glMatrixMode(GL_PROJECTION);
    for (int i = 0; i < 8; ++i) glPopMatrix();
    // Drain ALL pending error flags (multiple underflows can stack).
    // Single glGetError() solo limpia uno; el resto persiste y aparece
    // como 0x504 en cada FUN_005114f0 next frame.
    while (glGetError() != GL_NO_ERROR) {}
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    for (int i = 0; i < 8; ++i) glPopMatrix();
    while (glGetError() != GL_NO_ERROR) {}
    glLoadIdentity();
    return 0;
}

// ── Named aliases ────────────────────────────────────────────────────────────

// Object_MoveUpdate — alias for MoveObjects_stub (FUN_004FF260, per-frame
// world-objects animation/render-update dispatcher). The historical naming
// "Object_MoveUpdate" came from a Ghidra mis-id of FUN_0043E050 which is
// actually Movement_Tick (angle math). The CALLERS (Game_SceneUpdate /
// Game_EnterWorldTick / Game_CharSelectTick) want a per-frame objects tick,
// which IS MoveObjects (0x004FF260). Wire them here so the world-objects
// pool actually advances each frame.
//
// 2026-05-07 (revert): MoveObjects_stub iterates the world-objects bucket
// grid (DAT_083a021c..) which is only properly populated when a world is
// loaded (state 5 = in-game). During Login/CharSelect/Loading the bucket
// linked-list pointers are uninitialized garbage → AV in FUN_004fdc00 →
// FUN_0043e5c0 reading param_1 + 0x161. Gate on g_GameState == 5 so this
// is only active in-game where the pool is real.
extern void __stdcall MoveObjects_stub(void);
void __cdecl Object_MoveUpdate(void) {
    if (DAT_005615c0 == 5) {
        MoveObjects_stub();
        return;
    }

    // Login / CharSelect:
    // Keep this path inert for now. Multiple attempts to drive the login scene
    // objects from here ended in second-frame crashes, which strongly suggests
    // the original scene uses a narrower update path than the generic object
    // mover. We'll recover the logo / ship glows from the render side instead
    // of mutating login objects here.
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4) {
        return;
    }
}

// Particle_Update @ 0x0046C3E0 — MISNAMED. Real function = Trail_RenderAll.
// Iterates trail array (DAT_07C608B4, stride 0x2F0); renders GL_QUAD_STRIP segments
// with UV + alpha fade per segment. Implemented in src/Render/Trail_Render.cpp.
// Stub commented out to avoid duplicate symbol.
// void __cdecl Particle_Update(void) {}

// Character_UpdateAll @ 0x00479730 — Particle_RenderAll.
// Iterates effect/particle pool (base DAT_07C85890, stride 0x1BC).
// For each active slot: sets GL blend mode (0=normal,1=additive,2=alpha),
// calls FUN_00479670 to draw it, then clears the active flag.
// Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODOS los efectos (glow +9,
// wing FX, etc.). Ahora itera por índice acotado (1002 slots).
void __cdecl Character_UpdateAll(void) {
    char *pcVar2 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar2 += 0x1bc) {
        if (*pcVar2 != '\0') {
            int blend = *(int*)(pcVar2 + 4);
            if      (blend == 0) FUN_00511710();
            else if (blend == 1) FUN_00511790();
            else if (blend == 2) FUN_00511680('\x01');
            FUN_00479670((int)pcVar2);
            *pcVar2 = 0;
        }
    }
}

// Effect_UpdateAll @ 0x00479790 — marks all active particle entries dirty (+0x160 = 1).
// Pool fix 2026-04-27: ahora itera por índice acotado (1002 slots).
void __cdecl Effect_UpdateAll(void) {
    char *pcVar1 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar1 += 0x1bc) {
        if (*pcVar1 != '\0')
            pcVar1[0x160] = '\x01';
    }
}

// Login_CameraUpdate @ 0x004F8EB0 — SIGNATURE MISMATCH.
// Real: void Login_CameraUpdate(float *entity_pos). Builds login-screen orbit camera:
// timer-driven rotation (FUN_004CB520 * DAT_0055283C * cos/sin constants),
// applies rotation matrix (FUN_004F9DB0 + FUN_004FA0B0) and writes to
// DAT_07EEB228/DAT_07EEB218 (login camera world positions).
// Stub kept void() until Scene_Login.cpp callers are updated.
void __cdecl Login_CameraUpdate(void) {
    // Wrapper: calls FUN_004f8eb0 with login-scene entity[0] world position (+0x10).
    FUN_004f8eb0((float *)(DAT_07abf5d0 + 0x10));
}

// ShowErrorDialog / FUN_005142d0 @ 0x005142D0 — implemented in src/Render/GL_State.cpp

// UIChatLogWindow_AddText @ 0x00480620  — full 1:1 port of IDA sub_480620.
// Two distinct sinks:
//   (1) Engine vtable dispatch on the chat-listbox object at dword_55C9FF0
//       (slot +0x70 = AddText).  This pushes the entry into the doubly-
//       linked history list owned by ChatListBox.  Now active again because
//       ChatListBox_Construct properly installs the vtable in WinMain.
//   (2) Legacy popup ring buffer at DAT_07df9380 (119 slots × 0x118 bytes).
//       The original gates this on `(*strID == 0) || (mode in {0,3,4,5})`.
//       When `mode != 0,3,4,5` AND strID is non-empty, only the listbox sink
//       is used.
//
// Ring buffer layout per slot (stride 0x118):
//   +0x000..+0x00A  label/sender (11 bytes)
//   +0x00B..+0x10B  message text (257 bytes)
//   +0x10C..+0x10F  type DWORD
//
// Called from Trade.cpp (mode=2), Sound_Countdown2 (mode=0), GM messages, etc.
void __cdecl UIChatLogWindow_AddText(const char* label, const char* msg, int mode) {
    // 2026-07-27 DIAG (mensajes "whisper" fantasma con una letra suelta): este es
    // el punto ÚNICO por donde pasan todos los mensajes del chat log. Logueamos
    // label/msg/mode para identificar la fuente del mensaje espurio.
    // 2026-07-27 FIX (mensajes "whisper" fantasma con un carácter suelto):
    // descartar entradas con mensaje vacío/NULL. Game_SceneUpdate mete ~120
    // AddText con msg="" en cada carga de escena (GlobalText[470..473] vienen
    // vacíos); esas entradas SÍ entran al ring buffer de popups (el gate del
    // original sólo mira strID), y un slot del ring con bytes stale se renderiza
    // como un carácter basura en (0,65) — el "whisper" fantasma que se veía cada
    // tanto. Una línea de chat vacía nunca es legítima, así que la ignoramos.
    if (!msg || msg[0] == '\0') return;

    // BUG-FIX 2026-08-17: el dispatch de abajo estaba gateado con `&& label`, y
    // el handler del notice 0x0D type=1 (Net_Process) llama con label = nullptr.
    // Resultado: los mensajes del server (incluido el contador "You will quit
    // game in N second(s)") nunca entraban a la lista del listbox, así que
    // in-game no se veían — sólo aparecían al pasar a char-select, donde los
    // dibuja el otro sink (sub_480980, gateado a g_bUseChatListBox||state!=5).
    // En IDA 0x480620 el dispatch es la PRIMERA sentencia y es incondicional;
    // el original nunca pasa NULL (usa cadena vacía). Normalizamos acá, que
    // además evita el lstrcpynA con origen NULL de más abajo.
    if (!label) label = "";

    // ── 1. Engine vtable dispatch (slot +0x70) ────────────────────────────────
    if (DAT_055c9ff0 && msg) {
        DWORD* obj = (DWORD*)DAT_055c9ff0;
        void** vt = (void**)*obj;
        if (vt) {
            typedef void (__fastcall *FnAddText)(DWORD*, int /*edx*/,
                                                 const char*, const char*,
                                                 int, int);
            ((FnAddText)vt[28])(obj, 0, label, msg, mode, 0);
        }
    }

    // ── 2. Reset the periodic scroll timer ────────────────────────────────────
    DAT_00559ce4 = 0x96;   // Sound_Countdown2 will next fire 150 frames from now

    // ── Gate ring write — 1:1 con IDA 0x480620:
    //   escribe cuando `!label || (mode && mode!=3 && mode!=4 && mode!=5)`.
    //   → SKIP solo cuando label NO-vacío Y mode ∈ {0,3,4,5}.
    // FIX 2026-07-19: estaba INVERTIDO (skipeaba mode ∈ {1,2,6,...}). El ring lo
    // consume sub_480980 (notificaciones login/char-select); con el gate invertido
    // los mensajes de sistema/GM/whisper con sender iban al ring equivocado.
    char firstID = label ? *label : 0;
    if (firstID && (mode == 0 || mode == 3 || mode == 4 || mode == 5)) {
        return;
    }

    // ── 3. Append to player-chat ring buffer ───────────────────────────────────
    int   count = DAT_07e11da4;
    char* base  = DAT_07df9380;
    if (count >= 0x77) {
        // Ring full: shift all entries one slot forward, losing the oldest (slot 0)
        memmove(base, base + 0x118, (0x77 - 1) * 0x118);
        count = 0x77 - 1;
    }
    char* slot = base + count * 0x118;
    memset(slot, 0, 0x118);
    lstrcpynA(slot,        (LPCSTR)label, 0xB);    // sender name: max 11 chars
    lstrcpynA(slot + 0x0B, (LPCSTR)msg,   0x101);  // message:     max 257 chars
    *(int*)(slot + 0x10C) = mode;                  // channel / type dword
    DAT_07e11da4 = count + 1;

    // ── 4. Auto-advance scroll (follow-bottom) ─────────────────────────────────
    // FUN_00480980 (chat renderer) reads slots DAT_00559CE0 + 0..5. In the
    // original binary dword_559CE0 only advances when the user presses Enter
    // to send chat (WndProc LABEL_589, line 2508). But the login scene's
    // 120× FUN_00480620 init loop at Game_SceneUpdate fills the ring counter
    // to 119 BEFORE the user ever sends chat — so scroll stays at 0 and the
    // countdown text (written to slot 118/119 after memmove shift) never
    // appears on screen. Force-follow the tail so newly added messages are
    // always in the visible 6-line window.
    {
        int visible = DAT_07e11da4 - 6;
        if (visible < 0) visible = 0;
        DAT_00559ce0 = visible;
    }
}

// Particle_Spawn (5-arg legacy alias) — delegates to FUN_00475220 (Effect_Spawn).
// Skills.cpp callers pass (type, x, y, z, flags). The real spawner takes
// (type, bone_mat, pos, size, flag, alpha, mode) — we synthesize a position
// vec3 from x/y/z and pass NULL for the optional bone_mat / size with default
// alpha = 1.0 and mode = 0.
void __cdecl Particle_Spawn(int type, float x, float y, float z, int flags) {
    float pos[3]   = { x, y, z };
    float color[3] = { 1.0f, 1.0f, 1.0f };
    FUN_00475220(type, /*bone_mat*/nullptr, pos, color, flags, 1.0f, 0);
}

// ── FUN_ stubs (void returning) ───────────────────────────────────────────────
// FUN_00403a30 @ 0x00403A30 — NOP (empty function in original binary).
void __cdecl FUN_00403a30(void) {}

// ── CRT wrappers (forwarded to real CRT) ─────────────────────────────────────
// FUN_00543037 — CRT fseek wrapper
void __cdecl FUN_00543037(int *fp, int offset, int whence) {
    fseek((FILE*)fp, offset, whence);
}

// FUN_00542eb4 — CRT ftell wrapper
int __cdecl FUN_00542eb4(char *fp) {
    return (int)ftell((FILE*)fp);
}

// FUN_00541597 — CRT fread wrapper
int __cdecl FUN_00541597(void *dst, int size, int count, int *fp) {
    return (int)fread(dst, (size_t)size, (size_t)count, (FILE*)fp);
}

// BMD loaders (FUN_004423e0 BMD::Open, FUN_004422f0/00442260 face tables,
// FUN_00442e60 ComputeBounds, FUN_00442e00 ResetAnimState, FUN_00442a60 SaveToFile)
// moved to src/Render/BMD_Load.cpp (B3 refactor 2026-05-07, 661 lines).

// ── Terrain tile outline / ray-triangle intersection stubs ───────────────────
// FUN_004f7fb0 (RenderTerrainFace) — PORTADO 1:1 en src/Terrain/Terrain_RenderFace.cpp
// (antes era un no-op stub de 4 args; la firma real es 5 args con lodf).
// FUN_00512d40 @ 0x00512D40 — CollisionDetectLineToFace (a.k.a. Ray_TriIntersect)
// BUG-FIX 2026-04-26 (audit #7): activated full IDA port.  Old stub returned 0
// always, so terrain triangle picking *never* registered a hit — click-to-move
// would only land on whatever fallback path remained.  The dormant gated port
// (formerly behind IDA_PORT_00512D40) is now the live implementation.
//   xf → DAT_083a4130  (cursor billboard screen X / picked world X)
//   yf → DAT_083a4134  (cursor billboard screen Y / picked world Y)
//   DAT_083a4120 is the running closest-hit ray parameter (float, not DWORD).
unsigned int __cdecl FUN_00512d40(float *Position, float *Target, int Polygon,
    float *v1, float *v2, float *v3, float *v4, float *Normal, char Collision)
{
    float v24 = Target[0] - Position[0];
    float v25 = Target[1] - Position[1];
    float v26 = Target[2] - Position[2];
    double v12 = v26 * Normal[2] + v25 * Normal[1] + v24 * Normal[0];
    if ( v12 >= 0.0 )
        return 0;
    float v20 = (float)(-((Normal[1] * Position[1]
                          + Normal[0] * Position[0]
                          + Normal[2] * Position[2]
                          - (v1[2] * Normal[2]
                           + v1[1] * Normal[1]
                           + v1[0] * Normal[0]))
                         / v12));
    if ( v20 < 0.0f || (double)v20 > (double)DAT_083a4120 )
        return 0;
    float Targeta   = v20 * v24 + Position[0];
    float Positiona = v20 * v25 + Position[1];
    float Normala   = v20 * v26 + Position[2];

    float v17 = (float)FUN_005129f0(v26);
    float v15 = (float)FUN_005129f0(v25);
    float v14 = (float)FUN_005129f0(v24);
    float v16 = (float)FUN_00512a10(v14, v15);
    float v21 = (float)FUN_00512a10(v16, v17);

    if ( (float)FUN_005129f0(v24) == v21 ) {
        if ( !((float)FUN_00512a10(Position[1], Target[1]) <= Positiona
            && (float)FUN_00512a30(Position[1], Target[1]) >= Positiona
            && (float)FUN_00512a10(Position[2], Target[2]) <= Normala
            && (float)FUN_00512a30(Position[2], Target[2]) >= Normala) )
            return 0;
    } else if ( (float)FUN_005129f0(v25) == v21 ) {
        if ( (float)FUN_00512a10(Position[2], Target[2]) > Normala
          || (float)FUN_00512a30(Position[2], Target[2]) < Normala
          || (float)FUN_00512a10(Position[0], Target[0]) > Targeta
          || (float)FUN_00512a30(Position[0], Target[0]) < Targeta )
            return 0;
    } else if ( (float)FUN_00512a10(Position[0], Target[0]) > Targeta
             || (float)FUN_00512a30(Position[0], Target[0]) < Targeta
             || (float)FUN_00512a10(Position[1], Target[1]) > Positiona
             || (float)FUN_00512a30(Position[1], Target[1]) < Positiona ) {
        return 0;
    }

    float v19;
    int   v18;
    if ( Normal[0] <= -0.5f || Normal[0] >= 0.5f ) {
        v19 = Normal[0];
        v18 = 1;
    } else if ( Normal[1] <= -0.5f || Normal[1] >= 0.5f ) {
        v19 = Normal[1];
        v18 = 2;
    } else {
        v19 = Normal[2];
        v18 = 4;
    }

    if ( !FUN_00512a50(Targeta, Positiona, Normala, Polygon,
                       (int)v1, (int)v2, (int)v3, (int)v4, v18, v19) )
        return 0;

    if ( Collision ) {
        DAT_083a4120 = v20;
        DAT_083a4130 = Targeta;     // xf
        DAT_083a4134 = Positiona;   // yf
        DAT_083a4138 = Normala;
    }
    return 1;
}


// ════════════════════════════════════════════════════════════════════
// IDA HEX-RAYS PORTS reference block (307 IDA-only gated functions, ~29k lines)
// moved to src/stubs_IDA_ports.cpp (B3 refactor 2026-05-07).
// All functions there are gated by IDA_PORT_xxxxxxxx macros; none are
// active in the default build.
// ════════════════════════════════════════════════════════════════════
