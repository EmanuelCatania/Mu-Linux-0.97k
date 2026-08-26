// Chat_Log.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

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

