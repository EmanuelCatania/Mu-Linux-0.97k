// Game_EnterWorldTick.cpp
// Game_EnterWorldTick @ 0x00521D80
//
// Character-select scene tick. Called every frame while g_GameState==4.
// Drives the character list display, selection, and world-entry sequence.
//
// Entry guard: if DAT_05826cb0 < 0x33 → return (not ready yet).
//
// Sub-states (DAT_083a7c14):
//   0x14  — show char list (no free slot)
//   0x15  — show char list (free slot available)
//   0x16  — slide out (char chosen) → transition
//   0x17  — camera slide (equip view) → left
//   0x18  — slide out → send char-select 0xC1/0xF3 packet
//   0x19  — slide in → enter world (DAT_005615c0 = 3)
//   0x1a  — camera slide right
//   0x1b  — slide in to Y=0xd
//   0x1c  — slide out to transition / back to char list
//
// Globals:
//   DAT_083a7c4b   — one-time init guard
//   DAT_083a7c4a   — clear before Loading transition
//   DAT_005616ac   — selected character slot (-1 = none)
//   DAT_005616b0   — selected char confirmation (-1 = pending)
//   DAT_005616a4   — animated dialog Y (lerped each frame)
//   DAT_005616a8   — animated camera X offset (lerped each frame)
//   DAT_07abf050   — character state struct for slot 0
//   DAT_07abf20c   — character page/slot counter
//   DAT_07cf1ffc   — character data pointer
//   DAT_07cf1ff4   — character name buffer
//   DAT_07eaa114..  — anti-tamper counters
//   DAT_05826cb0   — server response code
//     0x33 = char list received
//     0x34 = char-select request sent
//     0x35-0x37 = char create/delete responses
//     0x39 = delete confirm
//     0x3a = enter world confirm
//   DAT_055ca038   — "select" button confirm flag

#include "stdafx.h"
#include "Game/Game_EnterWorldTick.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"

extern "C" void DbgLogPublic(const char*);

// ── CLICK-FLAG WATCHDOG ─────────────────────────────────────────────────────
// DAT_083a4124 (MouseLButtonPush) only toma valores {0,1} en el código fuente
// (WndProc lo settea a 1 en LBUTTONDOWN, todos los handlers lo limpian a 0).
// El runtime mostraba clk=1133983066 (0x4397355A) cada frame en char-select →
// phantom slot-clicks → state oscilando 0x18↔0x17.  La causa es algún stomp
// (probablemente buffer overflow en un struct adyacente).  Esta macro loggea
// cuándo aparece el valor wild para bisectar el corrupter, y la función
// IsClickPushed() filtra valores != 1 para suprimir phantom clicks mientras
// tanto.
static void Clk_Watch(const char* label)
{
    DWORD v = DAT_083a4124;
    if (v != 0 && v != 1) {
        static unsigned s_count = 0;
        if (s_count < 60) {
            s_count++;
            char dbg[160];
            wsprintfA(dbg, "CLK-CORRUPT @ %s: 0x%08X (decimal %u)", label, v, v);
            DbgLogPublic(dbg);
        }
        DAT_083a4124 = 0;  // clamp — only WndProc-set value 1 is trusted
    }
}
#define CLK_WATCH(LABEL) Clk_Watch(LABEL)
// IsClickPushed() ahora vive en globals.h para que todo módulo lo use.

// Same 32-byte XOR key
static const BYTE s_Key[32] = {
    0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
    0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
    0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
    0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
};

// Send a packet (len bytes from pkt) with WSAEWOULDBLOCK fallback
// MuEmu compat: cifrar in-place antes de send().  Pkt_Send recibe pkt
// como const → necesitamos un buffer mutable.  Copiamos a buf_local.
static void Pkt_Send(const BYTE* pkt_in, int len)
{
    if (DAT_055ca168 == 0xffffffff) return;
    BYTE pkt[256];
    if (len > (int)sizeof(pkt)) return;
    memcpy(pkt, pkt_in, len);
    MuEmu::EncryptSend(pkt, len);

    int sent = 0, rem = len;
    do {
        int n = send(DAT_055ca168, (char*)pkt + sent, rem - sent, 0);
        if (n == -1) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK && (int)(DAT_055cc16c + len) < 0x2001) {
                memcpy(DAT_055ca16c + DAT_055cc16c, pkt, len);
                DAT_055cc16c += len;
            } else {
                Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
            }
            return;
        }
        if (n == 0) break;
        if (DAT_055ce174) FUN_0043de60();
        sent += n; rem -= n;
    } while (rem > 0);
}

// Send the character-enter-world packet:
// [0xC1][len][0xF3] + username(10B, XOR) + padding(zeros, XOR) + slot_byte(XOR)
//
// BUG-FIX 2026-04-28: pkt[] era stack-buffer sin inicializar.  El loop XOR
// del slot byte hace `pkt[13] ^= key[13] ^ pkt[14]` y pkt[14] estaba en garbage
// stack-residual → cada call producía un cipher distinto del slot byte → el
// server descifraba un slot inválido y NO respondía con F3/03 (silent drop).
// Forzamos memset a 0 para que pkt[14] sea determinista (0) y el XOR final
// del slot byte sea estable.  Mismo fix aplicado al duplicado en case 0x19.
// 2026-05-05 BUG-FIX server kick post-F3/03 select-char:
// El encoding viejo usaba `pkt[i] ^= key[i] ^ pkt[i+1]` (NEXT byte) — pero
// el server (PacketManager.cpp::XorData) descifra con `pkt[n] ^= pkt[n-1]
// ^ key[n]` (PREVIOUS byte) iterando backward. Eso significa el encoding
// correcto del cliente es FORWARD usando `pkt[i-1]` (previous, que ya
// quedó encoded en el step anterior). Mismo patrón que la duplicate
// version en Game_CharSelectTick.cpp:108 y que Net_SendSmallPacket.
//
// Bug visible: server al recibir F3/03 select-char descifraba garbage,
// CGCharacterInfoRecv leía un nombre corrupto, GDCharacterInfoSend al
// DataServer fallaba (Char no existe), DataServer respondía DGCharacterInfoRecv
// con result=0 → CloseClient (DSProtocol.cpp:630). Eso es el FD_CLOSE.
//
// El flow alternativo es que el server SÍ procese el F3/03 con el nombre
// correcto (caso "mago" que se loguea OK) pero al final algún check
// secundario falla y kickea. La fórmula correcta abajo elimina la
// posibilidad de garbage en el descifrado.
static void Send_CharSelectPacket(void)
{
    BYTE pkt[32];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;
    pkt[1] = 15;        // total length (C1 LL F3 03 + name 10 + slot = 15)
    pkt[2] = 0xF3;
    pkt[3] = 0x03;

    // Username (from DAT_07db8710, max 10 bytes) — payload at pkt[4..13]
    char* uname   = (char*)DAT_07db8710;
    int   unamLen = (int)strlen(uname);
    if (unamLen > 10) unamLen = 10;
    memcpy(pkt + 4, uname, unamLen);
    // Padding bytes 14 - 4 - unamLen remain 0 (memset above).

    // Char slot byte: DAT_07abf20c low nibble * 0x10 + high nibble
    pkt[14] = (BYTE)((char)DAT_07abf20c * 0x10 + ((BYTE*)&DAT_07abf20c)[1]);

    // Forward chain XOR using PREVIOUS byte (matches server's reverse XorData).
    // Iterate i = 3..14 (entire body after pkt[2]). Server reverses n=14..3.
    for (int i = 3; i < 15; i++) {
        pkt[i] ^= pkt[i - 1] ^ s_Key[i & 0x1f];
    }

    // Pkt_Send aplica MuEmu byte-XOR + send (C1 path, no SerialModulus).
    Pkt_Send(pkt, 15);
}

// Send the character-CREATE packet (2026-07-17):
//   [0xC1][15][0xF3][0x01][name(10)][Class] — server PMSG_CHARACTER_CREATE_RECV.
// Antes el OK de crear mandaba Send_CharSelectPacket (F3/03 = seleccionar char
// existente) → el server recibía un select de un char inexistente → FD_CLOSE.
// Class = page*16 (DB_CLASS_DW=0, DK=16, FE=32, MG=48) — la fórmula
// `page*0x10 + byte1` del IDA da ese valor.
static void Send_CharCreatePacket(void)
{
    BYTE pkt[32];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;
    pkt[1] = 15;        // total length (C1 LL F3 01 + name 10 + class = 15)
    pkt[2] = 0xF3;
    pkt[3] = 0x01;      // sub 0x01 = CREATE

    char* name   = (char*)DAT_07db8710;   // typed char name (InputText[0])
    int   namLen = (int)strlen(name);
    if (namLen > 10) namLen = 10;
    memcpy(pkt + 4, name, namLen);

    // Class byte: page*0x10 + byte1 → 0/16/32/48 = DW/DK/FE/MG.
    pkt[14] = (BYTE)((char)DAT_07abf20c * 0x10 + ((BYTE*)&DAT_07abf20c)[1]);

    for (int i = 3; i < 15; i++) {
        pkt[i] ^= pkt[i - 1] ^ s_Key[i & 0x1f];
    }
    Pkt_Send(pkt, 15);
}

void Game_EnterWorldTick(void)
{
    // ── ENTRY GUARD ───────────────────────────────────────────────────────────
    if (DAT_05826cb0 < 0x33) return;

    // Count free entity slots (first 5 entries in entity array)
    bool hasFreeSlot = false;
    {
        char* e = (char*)DAT_07abf5d0;
        for (int i = 0; i < 5; i++, e += 0x394) {
            if (*e == '\0') { hasFreeSlot = true; break; }
        }
    }

    // ── ONE-TIME INIT ─────────────────────────────────────────────────────────
    if (DAT_083a7c4b == '\0') {
        DAT_083a7c4b = 1;

        // ── BUG-FIX 2026-04-25 ──────────────────────────────────────────────
        // Limpiar flags de click "stale" heredadas de la escena anterior.
        // En login/serverselect el usuario hace clicks rápidos para conectar;
        // si dos LBUTTONDOWNs caen <500ms aparte, Windows manda WM_LBUTTONDBLCLK
        // setteando DAT_083a4299=1.  Ese flag persiste hasta char-select y
        // cuando Mouse_Hover marca DAT_00559c50=0 (slot 0 hovered) la rama
        // DBLCLK del slot loop dispara → confirmed=true → state salta de 0x14
        // directo a 0x19 → entrada al mundo sin que el usuario haya clickeado
        // un slot.  Limpiamos también DAT_083a4124 por las dudas.
        DAT_083a4299 = 0;   // double-click flag
        DAT_083a4124 = 0;   // single-click flag
        Clk_Watch("scene-init");

        FUN_0050ff50();
        Clk_Watch("after-FUN_0050ff50");

        // Spawn background object 0xA4 at origin
        float pos3[3] = {0,0,0}, rot3[3] = {0,0,0};
        FUN_004ff5a0(0xa4, pos3, rot3, 1.0f);

        FUN_0045adc0((unsigned char*)&DAT_07abf050, 0xab, 0, 0, 0.0f);
        DAT_07abf20c = (DAT_07abf20c & 0xffffff00) | 1;
        DAT_07abf0d4  = 0;

        DAT_005616ac  = -1;
        DAT_005616b0  = -1;
        DAT_005615e0  = -1;
        DAT_07d78094  = 0;
        DAT_083a7c18  = 0;
        DAT_005616a4  = -100;

        if (!hasFreeSlot) {
            DAT_083a7c14 = 0x14;  // no free slot: char list only
        } else {
            DAT_083a7c14 = 0x15;  // free slot: can create char
            FUN_00404bc0(0x1b, 0, 0);
        }

        FUN_004cbdf0();

        // Anti-tamper: register DAT_07cf1ffc in hash table
        {
            unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, DAT_07cf1ffc);
            if (idx == 0xffffffff) {
                void* node = operator_new(0x585);
                *((BYTE*)node + 0x584) = 1;
                FUN_00403f80(&DAT_055c9bc8, node, DAT_07cf1ffc);
            } else {
                void* node = FUN_00404280(&DAT_055c9bc8, DAT_07cf1ffc);
                ((char*)node)[0x161]--;
                if (((char*)node)[0x161] == '\0')
                    FUN_00404400(node, DAT_07cf1ffc);
            }
        }

        // Clear char data buffers
        {
            char* p = (char*)DAT_07cf1ff4;
            for (int i = 0x56; i <= 0x6a; i++) p[i] = '\0';
            int* q = (int*)((char*)DAT_07cf1ffc + 0x87 * 4);
            for (int i = 7; i >= 0; i--, q += 0x11) *q = 0;
        }

        // Re-register new entry
        {
            unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, DAT_07cf1ffc);
            if (idx == 0xffffffff) {
                void* node = operator_new(0x585);
                *((BYTE*)node + 0x584) = 1;
                FUN_00403f80(&DAT_055c9bc8, node, DAT_07cf1ffc);
            }
        }

        // Clear anti-tamper counters
        DAT_07eaa114 = 0; DAT_07eaa115 = 0;
        DAT_07eaa116 = 0; DAT_07eaa117 = 0;
        DAT_07eaa118 = 0; DAT_07eaa119 = 0;
        DAT_07eaa11b = 0;
        DAT_07eaa124 = 0; DAT_07eaa144 = 0;
        DAT_07eaa14c = 0;

        DAT_07e11d70 = 0; DAT_07e11d71 = 0; DAT_07e11d72 = 0;
        DAT_00559c84 = 1;
        FUN_0047ec60(1);
        DAT_07e11d78 = 0;
        DAT_00559c8c = 0x5a;
        DAT_00559c88 = 1;

        // 120× widget draw
        for (int i = 0x78; i > 0; i--)
            FUN_00480620((const char*)&DAT_083a7c88, (const char*)&DAT_083a7c84, 0);

        // Clear entity local player ptr if null
        if (DAT_07abf5d8 == 0) {
            _DAT_00000010 = 0; _DAT_00000014 = 0; _DAT_00000018 = 0;
            _DAT_0000001c = 0; _DAT_00000020 = 0; _DAT_00000024 = 0;
        }

        // Set camera to char-select world position (slot 5 of CameraWalk).
        // BUG FIX (igual que Game_SceneUpdate): DAT_00561664..0056167b son
        // aliases DWORD& sobre el storage float de CameraWalk_005615ec[30..35].
        // Si los asignamos a float& (DAT_083a7ad0/4/8, _DAT_083a4334) MSVC
        // hace int→float conversion. Forzamos lectura como float reinterpretando
        // el storage (mismo offset, distinto tipo).
        DAT_083a7ad0  = *(float*)&DAT_00561670;
        DAT_083a7ad4  = *(float*)&DAT_00561674;
        DAT_083a7ad8  = *(float*)&DAT_00561678;
        DAT_083a7c38 = 0;
        DAT_083a7c3c = 5;
        DAT_005615e8 = 0;
        DAT_083a432c  = DAT_00561664;     // DWORD&=DWORD ok
        DAT_083a4330  = DAT_00561668;     // DWORD&=DWORD ok
        _DAT_083a4334 = *(float*)&DAT_0056166c;

        FUN_00405540(&DAT_055c9bf0, "> Character scene init success");
    }

    // ── PER-FRAME UPDATES ─────────────────────────────────────────────────────
    CLK_WATCH("frame-start");
    // ── BUG-FIX 2026-07-16: MoveBugs FALTABA en el tick de char-select ──────────
    // Este ES el tick de char-select (Game_MainLoop dispatch state 4 → esta fn,
    // pese al nombre "EnterWorldTick"). IDA 0x521D80 L401 llama MoveBugs() acá,
    // primero. Hace el fade-in del alpha de las entidades "bug" (Alpha()) y
    // posiciona/anima las MONTURAS (Uniria bug=195 / Dinorant bug=267). Sin él,
    // el alpha del mount queda en 0 → Calc_RenderObject cullea → no se dibuja.
    // (La sesión previa lo puso en Game_CharSelectTick, que es el tick de state 5.)
    { extern void __stdcall MoveBugs_stub(void); MoveBugs_stub(); }
    CLK_WATCH("after-MoveBugs");
    Object_MoveUpdate();         CLK_WATCH("after-Object_MoveUpdate");
    // ── BUG-FIX 2026-04-27: MoveParticles_stub (FUN_00477090) decrementa
    // lifetime de cada particle del pool DAT_07abf5f0. Sin esta llamada los
    // particles spawneados (lightning ELS=11, fire/smoke, etc.) se acumulan
    // forever → whiteout. Per IDA/5.2 RenderBlurs_RenderCharacterScene este
    // call se hace per-frame en MoveCharactersClient/MoveCharacterScene path.
    MoveParticles_stub();        CLK_WATCH("after-MoveParticles");
    Character_UpdateAll();       CLK_WATCH("after-Character_UpdateAll");
    FUN_00454fc0((float*)&DAT_07abf050); CLK_WATCH("after-FUN_00454fc0");

    // ── Per-character animation tick ──────────────────────────────────────────
    // En el binario original MoveCharactersClient (0x00455010) itera todas las
    // entidades y por cada slot activo llama MoveCharacterClient (0x00454FC0) →
    //   TestFrustrum2D + MoveMonsterClient + MoveCharacter + MoveCharacterVisual
    // donde MoveCharacter (0x00449900, 2773 lines) llama CharacterAnimation
    // (0x00448600) que avanza entity[+0x108] vía sub_440AA0.
    //
    // En nuestro port FUN_00449900 es un stub vacío y MoveCharacter está sin
    // portar (es enorme, 90% es lógica de combate/mov in-game irrelevante para
    // char-select). Pero los chars en char-select necesitan que su frame de
    // animación avance cada tick para que la idle se vea animada.
    //
    // Solución mínima: aquí, por slot 0..7, llamar MoveCharacterVisual (escribe
    // model.action desde entity[+0x105]) seguido de CharacterAnimation (avanza
    // el frame). Sin MoveCharacterVisual el modelo lee acción 0 y todos los
    // chars correrían la misma anim.
    {
        for (int s = 0; s < 8; ++s) {
            int e = (int)(uintptr_t)DAT_07abf5d0 + s * 0x394;
            if (*(char*)e != '\0') {
                FUN_004520c0(e);            // MoveCharacterVisual: model.action ← entity.action
                CharacterAnimation(e, e);   // advance entity[+0x108] (anim frame)
            }
        }
    } CLK_WATCH("after-CharAnim");
    Effect_UpdateAll();          CLK_WATCH("after-Effect_UpdateAll");
    Login_CameraUpdate();        CLK_WATCH("after-Login_CameraUpdate");
    // MoveCamera @ 0x0051E4E0 — actualiza CameraPosition/CameraAngle desde
    // CurrentCameraPosition/CurrentCameraAngle (lerp hacia wp5 = char-select).
    // Sin esta llamada la cámara queda anclada donde la dejó el login (~0,-1100,500)
    // y char-select se ve sin pitch correcto, los 5 personajes fuera de cuadro.
    // En IDA Game_EnterWorldTick (00521D80) llama MoveCamera() después de MoveParticles().
    MoveCamera_stub();           CLK_WATCH("after-MoveCamera");

    // Delete entity if server confirmed (0x39 = delete ack)
    if (DAT_05826cb0 == 0x39) {
        ((char*)DAT_07abf5d0)[DAT_005615e0 * 0x394] = '\0';
    }

    int mouseX = DAT_083a427c;

    // ── CHARACTER SELECTION (no char selected yet) ────────────────────────────
    if (DAT_005616b0 == -1) {
        FUN_00511600();  // render char list
        CLK_WATCH("after-FUN_00511600");

        int dialogY = DAT_005616a4;

        // Click arrow → select char (0x11d-0x164, Y in [dialogY+0x4b, dialogY+0x64])
        if (hasFreeSlot &&
            mouseX > 0x11d && mouseX < 0x164 &&
            DAT_083a4278 >= dialogY + 0x4b &&
            DAT_083a4278 < dialogY + 0x64 &&
            IsClickPushed())
        {
            DAT_083a4124 = '\0';
            DAT_083a7c24 = DAT_083a7c28;
            DAT_083a7c28 = 0;

            // Find first free entity slot
            int slotFound = DAT_005616b0;
            char* e = (char*)DAT_07abf5d0;
            for (int i = 0; i < 5; i++, e += 0x394) {
                if (*e == '\0') { slotFound = i; break; }
                slotFound = DAT_005616b0;
            }
            DAT_005616b0 = slotFound;
            FUN_00404bc0(0x19, 0, 0);
            DAT_083a7c14 = 0x16;
            DAT_083a7c18 = 0x1b;
            FUN_00404bc0(0x1b, 0, 0);
            FUN_0047ec60(1);
            DAT_00559c84 = 1;
            DAT_00559c88 = 1;
            _DAT_00559c94 = 10;
        }

        // Update free slot flag after possible change
        hasFreeSlot = (DAT_005616ac != -1);

        // Click equip/select button (DAT_005616a8+0x80..0xc6, Y 0x93-0xa7)
        if (hasFreeSlot &&
            mouseX >= DAT_005616a8 + 0x80 && mouseX < DAT_005616a8 + 0xc6 &&
            DAT_083a4278 > 0x93 && DAT_083a4278 < 0xa7 &&
            IsClickPushed())
        {
            DAT_083a4124 = '\0';
            ushort lvl = *(ushort*)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x1be);
            BYTE   flags = ((BYTE*)DAT_07abf5d0)[DAT_005616ac * 0x394 + 0x1c0];

            if (lvl < 40) {
                // Level too low error
                if (flags & 0x12) {
                    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x19; else DAT_083a7c28 = 0x19;
                } else {
                    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x72; else DAT_083a7c28 = 0x72;
                    FUN_0047ec60(1);
                    DAT_00559c84 = 1; DAT_00559c88 = 1;
                    _DAT_00559c94 = DAT_083a7acc;
                    DAT_07e113d8[0] = 1;
                }
            } else {
                if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x84; else DAT_083a7c28 = 0x84;
            }
            FUN_00404bc0(0x19, 0, 0);
        }

        // ── BUG-FIX 2026-07-17: las flechas de cambio de clase se MOVIERON al
        // bloque `else if (DAT_005616b0 != -1)` (create/name view). IDA
        // Game_EnterWorldTick: el input del create-panel (flechas de clase,
        // OK/Cancel, nombre) corre cuando `dword_5616B0 >= 0`, no en la lista.
        // Antes vivían acá (== -1) → tras clickear NEW CHARACTER (que setea
        // 5616B0>=0) este bloque se saltea → las flechas nunca corrían.

        // ── BUG-FIX 2026-04-27 ────────────────────────────────────────────
        // IDA original (00521D80) tiene el bloque "back/delete/Enter" dentro
        // de `if (DAT_005616b0 != -1)` (rama name-input), NO dentro de
        // `if (DAT_005616b0 == -1)` (rama lista). Estaba mal ubicado y eso
        // hacía que Enter en la lista (sin slot abierto para nombre) gatillara
        // el flujo de creación nuevo personaje incluso con slots llenos.
        // El bloque ahora vive en el `else if (DAT_005616b0 != -1)` más abajo.

        // Entity slot click detection (5 slots)
        {
            int slotY = dialogY + 0x32;
            bool confirmed = false;
            int  selSlot   = DAT_005616ac;
            int  pendingB  = DAT_083a7c28;

            for (int i = 0; i < 5; i++, slotY += 0x14) {
                if (i == DAT_00559c50) {
                    if (DAT_083a4299 != '\0') {
                        DAT_083a4299 = '\0';
                        DAT_083a7c28 = 0;
                        confirmed = true;
                        selSlot   = i;
                        pendingB  = 0;
                        DAT_005616ac = i;
                        DAT_083a7c24 = pendingB;
                        {
                            char dbg[160];
                            wsprintfA(dbg,
                                "SLOT-DBLCLK i=%d c50=%d clk=%d mx=%d my=%d s=%X",
                                i, (int)DAT_00559c50, (int)DAT_083a4124,
                                (int)DAT_083a427c, (int)DAT_083a4278,
                                (unsigned)DAT_083a7c14);
                            DbgLogPublic(dbg);
                        }
                    } else if (IsClickPushed()) {
                        // ── DIAGNOSTIC: capture state at slot-click trigger ──
                        {
                            char dbg[160];
                            wsprintfA(dbg,
                                "SLOT-CLICK FIRED i=%d c50=%d clk=%d mx=%d my=%d s=%X t=%u",
                                i, (int)DAT_00559c50, (int)DAT_083a4124,
                                (int)DAT_083a427c, (int)DAT_083a4278,
                                (unsigned)DAT_083a7c14, GetTickCount());
                            DbgLogPublic(dbg);
                        }
                        DAT_083a4124 = '\0';
                        DAT_083a7c28 = 0;
                        DAT_005616ac = i;
                        DAT_083a7c24 = pendingB;
                        FUN_00404bc0(0x19, 0, 0);
                        if (DAT_083a7c14 != 0x17 && DAT_083a7c14 != 0x15)
                            FUN_00404bc0(0x1b, 0, 0);
                        DAT_083a7c14 = 0x18;
                        DAT_083a7c18 = 0x17;
                        selSlot = DAT_005616ac;
                        pendingB = DAT_083a7c28;
                    }
                }
                pendingB = DAT_083a7c28;
            }

            // ── BUG-FIX 2026-04-25 ───────────────────────────────────────
            // IDA original (00521D80) gates the dword_83A7C14 = 25 path with:
            //   if (!v103) goto LABEL_117;     // v103 = (SelectedHero != -1)
            //   if (!ErrorMessage && byte_55CA038) v40 = 1;
            //   if (selectButtonClicked) { ... fall through to flag check }
            //   else { LABEL_117: if (!v40) goto LABEL_124; }
            //   // flag check → state = 25
            //
            // Port previo perdía AMBOS guards (`if (!v103)` y `if (!v40)`):
            // el bloque que setea state=0x19 corría cada frame, y con
            // SelectedHero=-1 leía garbage memory (selSlot*0x394+0x1c0 con
            // selSlot=-1 cae en -0x1d4 desde la base) → triggeraba world-entry
            // sin click del usuario → char-select desaparecía tras 1s.

            bool selectClicked = false;
            if (DAT_005616ac != -1) {
                // v40 = 1 path (line 545 IDA): confirm flag from button trigger
                if (hasFreeSlot && !confirmed) {
                    if (DAT_083a7c24 == 0 && DAT_055ca038 != '\0') {
                        DAT_055ca038 = '\0';
                        DAT_083a7c28 = 0;
                        confirmed = true;
                        DAT_083a7c24 = pendingB;
                        FUN_00404bc0(0x19, 0, 0);
                    }
                }

                // Click center "Select" button (line 556 IDA)
                if (mouseX >= DAT_005616a8 + (int)0xffffff41 + 0x1b9 &&
                    mouseX < DAT_005616a8 + (int)0xffffff01 + 0x1ff &&
                    DAT_083a4278 > 0x94 && DAT_083a4278 < 0xa6 &&
                    IsClickPushed())
                {
                    DAT_083a4124 = '\0';
                    DAT_083a7c28 = 0;
                    DAT_083a7c24 = pendingB;
                    FUN_00404bc0(0x19, 0, 0);
                    selectClicked = true;
                }
            }

            // Flag check ONLY si v103 (char válido) Y (selectClick OR v40):
            // corresponde a IDA `if (!v40) goto LABEL_124` y precondición
            // SelectedHero != -1.  Sin esto, cada frame disparaba state=0x19.
            if (DAT_005616ac != -1 && (selectClicked || confirmed)) {
                BYTE flags = ((BYTE*)DAT_07abf5d0)[DAT_005616ac * 0x394 + 0x1c0];
                // Precedence-fix: IDA `(flags & 1) == 0`, no `!flags & 1`.
                if ((flags & 1) == 0) {
                    DAT_083a7c14 = 0x19;
                    FUN_00404bc0(0x1b, 0, 0);
                } else {
                    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x85; else DAT_083a7c28 = 0x85;
                }
            }
        }
    }
    // ── NAME-INPUT VIEW (slot reservado para nuevo char): b0 != -1 ────────────
    // IDA: `if (-1 < DAT_005616b0)`. Aquí Enter (DAT_055ca038) y los botones
    // back (mouseX 0xea..0x133) / OK (mouseX 0x14f..0x196) operan sobre el
    // diálogo de nombre del nuevo personaje. Antes de este fix vivían dentro
    // de `if (DAT_005616b0 == -1)` y disparaban con Enter en la pantalla de
    // lista, generando el bug "Enter activa New Character con slots llenos".
    else if ((int)DAT_005616b0 != -1) {
        int dialogY = DAT_005616a4;

        // ── Class rotate arrows (create-panel) — IDA: corre con dword_5616B0 >= 0.
        // Fila superior (dialogY+196..215 = 0xc4..0xd7): flecha izq [0x16b,0x17f)=clase--,
        // flecha der [0x180,0x194)=clase++. Re-crea el preview char (tipo = clase+170).
        if (mouseX > 0x16a && mouseX < 0x17f &&
            DAT_083a4278 >= dialogY + 0xc4 && DAT_083a4278 < dialogY + 0xd7 &&
            IsClickPushed())
        {
            DAT_083a4124 = '\0';
            if ((char)DAT_07abf20c == '\0') {
                FUN_00404bc0(0x1a, 0, 0);
            } else {
                BYTE b = (BYTE)((char)DAT_07abf20c - 1);
                DAT_07abf20c = (DAT_07abf20c & 0xffffff00) | b;
                FUN_0045adc0((unsigned char*)&DAT_07abf050, b + 0xaa, 0, 0, 0.0f);
                DAT_07abf0d4 = 0;
                FUN_00404bc0(0x19, 0, 0);
            }
        }
        unsigned maxPage = 2;   // rotate limit: 3 si algún char >= lvl 220
        {
            char* e = (char*)DAT_07abf5d0;
            for (int i = 0; i < 5; i++, e += 0x394)
                if (*e != '\0' && *(ushort*)(e + 0x1be) > 0xdb) { maxPage = 3; break; }
        }
        if (mouseX > 0x17f && mouseX < 0x194 &&
            DAT_083a4278 >= dialogY + 0xc4 && DAT_083a4278 < dialogY + 0xd7 &&
            IsClickPushed())
        {
            DAT_083a4124 = '\0';
            if ((DAT_07abf20c & 0xff) >= maxPage) {
                // Tope de rotacion: sonido de "bloqueado". Faltaba — IDA
                // Game_EnterWorldTick L635-638 tiene las dos ramas.
                FUN_00404bc0(0x1a, 0, 0);   // PlayBuffer(26)
            } else {
                BYTE b = (BYTE)((char)DAT_07abf20c + 1);
                DAT_07abf20c = (DAT_07abf20c & 0xffffff00) | b;
                FUN_0045adc0((unsigned char*)&DAT_07abf050, b + 0xaa, 0, 0, 0.0f);
                DAT_07abf0d4 = 0;
                FUN_00404bc0(0x19, 0, 0);
            }
        }

        bool backClick = (mouseX > 0xea && mouseX < 0x133 &&
                          DAT_083a4278 >= dialogY + 0x15d &&
                          DAT_083a4278 < dialogY + 0x172 &&
                          IsClickPushed());
        bool clearBtn = (DAT_055ca038 != '\0');
        if (clearBtn) DAT_055ca038 = '\0';

        // "Back" path — pure cancel back to slot list (IDA path A)
        if (backClick) {
            DAT_083a4124 = '\0';
            DAT_005616b0 = -1;
            FUN_00404bc0(0x19, 0, 0);
            DAT_083a7c14 = 0x1c;
            DAT_083a7c18 = 0x15;
            FUN_00404bc0(0x1b, 0, 0);
        }
        else {
            // "OK button" hit-test (0x14f..0x196)
            bool okClick = (mouseX >= 0x14f && mouseX <= 0x196 &&
                            DAT_083a4278 >= dialogY + 0x15d &&
                            DAT_083a4278 < dialogY + 0x172 &&
                            IsClickPushed());
            if (okClick || clearBtn) {
                if (okClick) DAT_083a4124 = '\0';
                DAT_005616b0 = -1;
                FUN_00404bc0(0x19, 0, 0);
                DAT_083a7c14 = 0x1c;
                DAT_083a7c18 = 0x15;
                FUN_00404bc0(0x1b, 0, 0);

                // Validate username length
                int ulen = (int)strlen((char*)DAT_07db8710);
                if (ulen < 4) {
                    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x7a; else DAT_083a7c28 = 0x7a;
                } else {
                    if (FUN_00513570() == '\0' && FUN_00406b30((BYTE*)DAT_07db8710) == '\0') {
                        DAT_083a7c18 = 0x1c;
                        DAT_083a7c24 = DAT_083a7c28;
                        DAT_083a7c28 = 0;
                        DAT_05826cb0 = 0x34;
                        Send_CharCreatePacket();   // BUG-FIX 2026-07-17: F3/01 crear (no F3/03 select)
                    } else {
                        if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x73; else DAT_083a7c28 = 0x73;
                    }
                }
            }
        }
    }

    // ── DIAGNOSTIC: log substate transitions (only when state changes) ───────
    {
        static int s_lastSubState = -1;
        static int s_lastDialogY  = -999999;
        static int s_lastCamX     = -999999;
        if ((int)DAT_083a7c14 != s_lastSubState) {
            char dbg[160];
            wsprintfA(dbg,
                "EnterWorld substate %X -> %X  (sel=%d sel2=%d code=%X dY=%d cX=%d hasFree=%d)",
                (unsigned)s_lastSubState, (unsigned)DAT_083a7c14,
                (int)DAT_005616ac, (int)DAT_005616b0,
                (unsigned)DAT_05826cb0,
                (int)DAT_005616a4, (int)DAT_005616a8,
                hasFreeSlot ? 1 : 0);
            DbgLogPublic(dbg);
            s_lastSubState = (int)DAT_083a7c14;
        }
        // Also log dialog Y / camera X every ~32 frames if state is steady so we
        // can see if positions are bouncing without a state change.
        static int s_frameCtr = 0;
        if ((++s_frameCtr & 0x1f) == 0) {
            if ((int)DAT_005616a4 != s_lastDialogY || (int)DAT_005616a8 != s_lastCamX) {
                char dbg[128];
                wsprintfA(dbg,
                    "EnterWorld pos s=%X dY=%d cX=%d",
                    (unsigned)DAT_083a7c14,
                    (int)DAT_005616a4, (int)DAT_005616a8);
                DbgLogPublic(dbg);
                s_lastDialogY = (int)DAT_005616a4;
                s_lastCamX    = (int)DAT_005616a8;
            }
        }
    }

    // ── ANIMATION STATE MACHINE ───────────────────────────────────────────────
    switch (DAT_083a7c14) {
    case 0x15:  // Ease dialog Y to 6
        DAT_005616a4 += (6 - DAT_005616a4) / 3;
        break;

    case 0x16:  // Slide dialog off screen (down)
        DAT_005616a4 -= (DAT_005616a4 + 0x102) / 2;
        if (DAT_005616a4 < -0xff) {
            DAT_083a7c14 = DAT_083a7c18;
            FUN_00404bc0(0x1b, 0, 0);
        }
        break;

    case 0x17:  // Camera slide left
    case 0x1a:  // Camera slide right
        {
            int d = (int)((unsigned long long)((long long)DAT_005616a8 * 0x55555555LL) >> 32) - DAT_005616a8;
            DAT_005616a8 += (d >> 1) - (d >> 31);
        }
        break;

    case 0x18:  // Slide panel off, send char-select
        DAT_005616a8 -= (DAT_005616a8 + 0xca) / 2;
        if (DAT_005616a8 < -199) {
            DAT_083a7c14 = DAT_083a7c18;
            FUN_00404bc0(0x1b, 0, 0);
            if (DAT_05826cb0 == 0x3a) {
                int code = DAT_05826cb0;
                if (DAT_083a7c24 == 0) { DAT_083a7c24 = 0x3a; DAT_05826cb0 = 0x33; }
                else                   { DAT_083a7c28 = 0x3a; DAT_05826cb0 = 0x33; }
            }
        }
        break;

    case 0x19:  // Slide off; on reach: send enter-world packet
        DAT_005616a8 -= (DAT_005616a8 + 0xca) / 2;
        if (DAT_005616a8 < -200) {
            int code = DAT_05826cb0;
            bool validCode = (code == 0x33 || code == 0x35 || code == 0x36 ||
                              code == 0x37 || code == 0x39 || code == 0x3a);
            if (validCode) {
                // Build and send world-enter packet with username + anti-tamper data
                {
                    // Copy char name from selected slot
                    char* charName = (char*)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x1c1);
                    int   clen = (int)strlen(charName);
                    // Copy to char data buffer
                    memcpy((char*)DAT_07cf1ff4, charName, clen + 1);
                    *(ushort*)((char*)DAT_07cf1ff4 + 0xe) = *(ushort*)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x1be);
                    ((char*)DAT_07cf1ff4)[0xb] = ((char*)DAT_07abf5d0)[DAT_005616ac * 0x394 + 0x1bc];
                    ((char*)DAT_07cf1ff4)[0xc] = ((char*)DAT_07abf5d0)[DAT_005616ac * 0x394 + 0x1bd];
                }
                // BUG-FIX 2026-04-28: bug doble.
                //
                // (1) DIRECCIÓN del XOR — server MuEmu (PacketManager.cpp:486
                //     CPacketManager::XorData) descifra con prev-byte chain:
                //         m_buff[n] ^= m_buff[n - 1] ^ key[n % 32]   (n--)
                //     pero nuestro port de Ghidra usaba pkt[i+1] (byte
                //     siguiente).  Server al descifrar producía garbage en
                //     el sub-opcode → switch(lpMsg[3]) no matcheaba 0x03 →
                //     silently drop → no F3/03 response → pantalla negra.
                //     La login (Game_SceneUpdate.cpp:135) ya usa la fórmula
                //     correcta con pkt[i-1].
                //
                // (2) NAME source — el server MuEmu hace lookup por CHAR
                //     name (Protocol.h:122 PMSG_CHARACTER_INFO_RECV.name);
                //     el original 0.97K mandaba InputText[0]=ACCOUNT name
                //     que MuEmu no usa.  Mandamos el char name del slot.
                //
                // (3) Sub-opcode 0x03 explícito — el original lo "creaba"
                //     vía la cadena XOR; aquí lo metemos plano y dejamos
                //     que la cadena XOR forward lo cifre.
                BYTE pkt[32];
                memset(pkt, 0, sizeof(pkt));
                pkt[0] = 0xC1;
                pkt[1] = 0x0E;       // length = 14
                pkt[2] = 0xF3;        // opcode
                pkt[3] = 0x03;        // sub-opcode: CharacterInfoRecv

                char* charName = (char*)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x1c1);
                int   nameLen = (int)strlen(charName);
                if (nameLen > 10) nameLen = 10;
                memcpy(pkt + 4, charName, nameLen);
                int pos = 14;
                // XOR-encode body i=3..size-1 with prev-byte chain (matches
                // server's XorData inverse).
                for (int i = 3; i < pos; i++) {
                    pkt[i] ^= pkt[i - 1] ^ s_Key[i & 0x1f];
                }
                Pkt_Send(pkt, pos);

                // Handle specific response codes
                if (code == 0x36 || code == 0x37) {
                    int r = code;
                    if (DAT_083a7c24 == 0) { DAT_083a7c24 = r; }
                    else { DAT_083a7c28 = r; }
                }

                // ── WORLD ENTRY: copy char data → transition to Loading ──────
                // (BUG-FIX 2026-04-25: en el port original este bloque estaba
                //  FUERA del switch → se ejecutaba cada frame → state 4→3→5
                //  cíclicamente, disparando Game_CharSelectTick que envía un
                //  F3 char-name packet sin MuEmu wrap → server FD_CLOSE.
                //  IDA original 0x00521D80:1150 pone esto DENTRO de case 25
                //  después del Pkt_Send y check de validCode.)
                {
                    char* charName = (char*)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x1c1);
                    char* charData = (char*)DAT_07cf1ff4;
                    int   len      = (int)strlen(charName);
                    memcpy(charData, charName, len + 1);
                    *(ushort*)(charData + 0xe) = *(ushort*)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x1be);
                    charData[0xb] = ((char*)DAT_07abf5d0)[DAT_005616ac * 0x394 + 0x1bc];
                    charData[0xc] = ((char*)DAT_07abf5d0)[DAT_005616ac * 0x394 + 0x1bd];
                }

                DAT_005615c0  = 3;  // g_GameState = Loading
                DAT_083a7c4a  = 0;  // reset Scene_Loading init guard
                FUN_005102c0();     // world loading kickoff
                FUN_00404c60(5);    // BGM_Stop(5)
            }
        }
        break;

    case 0x1b:  // Ease dialog to Y=0xd
        DAT_005616a4 += (0xd - DAT_005616a4) / 3;
        break;

    case 0x1c:  // Slide off; transition to char list or create
        DAT_005616a4 -= (DAT_005616a4 + 0x192) / 2;
        if (DAT_005616a4 < -399) {
            int code = DAT_05826cb0;
            if (code == 0x35 || code == 0x36 || code == 0x37) {
                // Check if any slot is free
                bool anyFree = false;
                char* e = (char*)DAT_07abf5d0;
                for (int i = 0; i < 5; i++, e += 0x394)
                    if (*e == '\0') { anyFree = true; break; }

                if (anyFree) {
                    DAT_083a7c14 = 0x15;
                    FUN_00404bc0(0x1b, 0, 0);
                } else {
                    DAT_083a7c14 = 0x14;
                }

                // Propagate response code
                if (code == 0x36 || code == 0x37) {
                    int r = code;
                    if (DAT_05826cb0 == 0) { DAT_083a7c28 = r; DAT_083a7c24 = DAT_083a7c28; }
                    else if (DAT_083a7c24 == 0) { DAT_083a7c24 = r; }
                    else { DAT_083a7c28 = r; }
                }
            } else if (DAT_083a7c18 == 0x15) {
                DAT_083a7c14 = 0x15;
            }
        }
        break;

    default:
        // Fall through — default animation stop
        break;
    }
  // 2026-07-16 DIAG: bracket la sección UI de char-select
    // (NB: the world-entry transition block — DAT_005615c0=3, FUN_005102c0,
    //  FUN_00404c60(5) — used to live here, OUTSIDE the switch.  Eso era un
    //  port-bug: en el binario original ese código está DENTRO de case 25
    //  (= nuestro case 0x19), tras el Pkt_Send y el check de validCode.
    //  Al estar afuera, se ejecutaba cada frame → state 4→3→5 cíclico →
    //  Game_CharSelectTick disparaba unencrypted F3 → server FD_CLOSE.)
}
