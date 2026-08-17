// Game_MainLoop.cpp
// Game_MainLoop @ 0x00525D40
//
// Main frame loop for g_GameState 2/4/5 (Login/CharSelect/InGame).
// Called every frame from Scene_Dispatch.
// Handles: 25fps frame limiter, game logic tick, render, audio, BGM state machine.
//
// Signature: void __cdecl Game_MainLoop(HDC param_1)
//
// Frame structure:
//   1. Net_Recv() poll
//   2. Busy-wait loop: while (budget > 40ms) { game_logic(); budget -= 40; }
//   3. GL clear + scene render
//   4. SwapBuffers
//   5. Post-render 25fps limiter
//   6. BGM state machine
//   7. Window title updates
//
// Globals:
//   DAT_005615c0  — g_GameState
//   DAT_0055a7ac  — g_GameSubState
//   DAT_005616b8  — frame budget accumulator (adds actual frame ms each frame)
//   DAT_083a7c54  — total frame counter
//   DAT_083a7c00  — anti-tamper tick counter
//   DAT_055ca018  — socket error flag (abort if set)
//   DAT_083a42ec  — screenshot mode toggle (F12)
//   DAT_083a7c48  — connection check enable
//   DAT_083a7c58  — disconnect dialog shown flag
//   DAT_07e118e8  — world/map type
//   DAT_07c74ae4  — BGM track 1 enable flag
//   DAT_07abf5d8  — local player entity pointer
//   DAT_05826d08  — countdown counter A
//   DAT_07e11d7c  — countdown counter B
//   DAT_0839bc8c  — frame index mod 32

#include "stdafx.h"
#include "Game/Game_MainLoop.h"
#include "Game/Game_SceneUpdate.h"
#include "Game/Game_EnterWorldTick.h"
#include "Game/Game_CharSelectTick.h"
#include "Scene/Scene_Login.h"
#include "Scene/Scene_CharSelect.h"
#include "Render/Render.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"
#include <stdlib.h>
// (_rand ya está definido como `_rand() rand()` en stdafx.h)

extern "C" { void DbgLogPublic(const char* msg); void ChkHeapPublic(const char* tag); }
#define CHK(tag) ChkHeapPublic(tag)

void __cdecl Game_MainLoop(HDC param_1)
{
    char   titleBuf[64];
    char   nameBuf[256];
    char   tickBuf[100];
    int    renderFlag = 0;
    DWORD  renderStart;
    int    shiftHeld;

    CHK("ML/enter");

    // 2026-05-05 diag: log entry rate-limited (per state)
    {
        static int s_lastEnterState = -1;
        static DWORD s_lastEnterT = 0;
        DWORD now = GetTickCount();
        if (DAT_005615c0 != s_lastEnterState || now - s_lastEnterT > 2000) {
            char b[80];
            _snprintf_s(b, sizeof(b), _TRUNCATE, "ML enter state=%d tickCnt=%d",
                (int)DAT_005615c0, (int)DAT_005616b8);
            DbgLogPublic(b);
            s_lastEnterState = DAT_005615c0;
            s_lastEnterT = now;
        }
    }

    // ── RECEIVE INCOMING PACKETS ─────────────────────────────────────────────
    FUN_0043fd70();
    CHK("ML/post_recv");

    // ── MULTI-TICK FRAME LIMITER LOOP (25fps / 40ms per tick) ────────────────
    // Runs once per accumulated 40ms budget. Processes game logic each tick.
    while (DAT_005616b8 > 0x27) {

        // Anti-tamper: periodically re-randomize hash table when counter hits
        // a specific residue (0x39 mod 0x139). Not game logic.
        if (DAT_083a7c00 % 0x139 == 0x39) {
            // Hash table rehash: reallocate with random offsets
            if (DAT_055c9be0 != NULL)
                operator_delete((BYTE*)DAT_055c9be0);
            DAT_055c9be0 = (DWORD)(BYTE*)operator_new(_rand() % 0xcc7 + 0x159);

            BYTE* old_bcc = (BYTE*)DAT_055c9bcc;
            BYTE* old_bd0 = (BYTE*)DAT_055c9bd0;
            BYTE* old_be8 = (BYTE*)DAT_055c9be8;
            BYTE* old_bec = (BYTE*)DAT_055c9bec;

            DAT_055c9be8 = (DWORD)(BYTE*)operator_new(DAT_055c9bd4 * 4 + 0x100);
            DAT_055c9bec = (DWORD)(BYTE*)operator_new(DAT_055c9bd4 * 4 + 0x100);

            unsigned rnd1 = (unsigned)(_rand() & 0x3f);
            unsigned rnd2 = (unsigned)(_rand() & 0x3f);
            DAT_055c9bcc = (DWORD)((BYTE*)DAT_055c9be8 + rnd1 * 4);
            DAT_055c9bd0 = (DWORD)((BYTE*)DAT_055c9bec + rnd2 * 4);

            memset((void*)DAT_055c9bcc, 0, DAT_055c9bd4 * 4);
            memset((void*)DAT_055c9bd0, 0, DAT_055c9bd4 * 4);

            _DAT_055c9be4 = ((_rand() % 0xf0) / 2) * 2 + 0x6f;

            // Re-insert all existing entries into new table
            for (unsigned i = 0; i < (unsigned)DAT_055c9bd4; i++) {
                if (((DWORD*)old_bd0)[i] != 0) {
                    DWORD key = ((DWORD*)old_bd0)[i];
                    DWORD val = ((DWORD*)old_bcc)[i];
                    // HashTable_Insert equivalent
                    unsigned h = (**(unsigned(__cdecl**)(DWORD))(DAT_055c9bc8 + 0xc))(key);
                    while (((DWORD*)DAT_055c9bd0)[h] != 0 && ((DWORD*)DAT_055c9bd0)[h] != key)
                        h = (h + 1) % DAT_055c9bd4;
                    ((DWORD*)DAT_055c9bcc)[h] = val;
                    ((DWORD*)DAT_055c9bd0)[h] = key;
                }
            }
            operator_delete(old_be8);
            operator_delete(old_bec);
        }

        FUN_00514310();

        CHK("ML/pre_tick");
        // Dispatch per-state game logic
        if (DAT_005615c0 == 2) { Game_SceneUpdate();     CHK("ML/post_SceneUpdate"); }
        if (DAT_005615c0 == 4) { Game_EnterWorldTick();  CHK("ML/post_EnterWorld"); }
        if (DAT_005615c0 == 5) { Game_CharSelectTick();  CHK("ML/post_CharSelectTick"); }

        // Hash table ticks (anti-tamper, ×5) — comentado: dirección absoluta
        // hardcodeada 0x83a4338 no existe como símbolo en nuestro build.
        // for (int i = 0; i < 5; i++) FUN_00409c40(0x83a4338);

        // Input update
        FUN_0047fcb0();   CHK("ML/post_0047fcb0");
        FUN_00480950();   CHK("ML/post_00480950");

        // F12 → toggle screenshot mode
        if (FUN_0047ec20(0x2c) & 0xff)
            DAT_083a42ec ^= 1;

        // Decrement countdown counters
        if (DAT_05826d08 > 0) DAT_05826d08--;
        if (DAT_07e11d7c > 0) DAT_07e11d7c--;

        // Frame index mod 32
        DAT_0839bc8c = (DAT_0839bc8c + 1) & 0x1f;

        // Anti-tamper: track DAT_083a7c00 ref-count, then increment
        {
            unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, &DAT_083a7c00);
            if (idx == 0xffffffff) {
                void* node = operator_new(5); *((BYTE*)node + 4) = 1;
                FUN_00403f80(&DAT_055c9bc8, node, &DAT_083a7c00);
            } else {
                BYTE* node = *(BYTE**)(DAT_055c9bcc + idx * 4);
                node[4]++;
                if (node[4] < 2) FUN_00409e20(&DAT_083a7c00, node);
            }
        }
        DAT_083a7c00++;
        {
            unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, &DAT_083a7c00);
            if (idx != 0xffffffff) {
                BYTE* node = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_083a7c00);
                node[4]--;
                if (node[4] == 0) FUN_00423760(node, &DAT_083a7c00);
            }
        }

        // Consume 40ms of budget, count frame
        DAT_005616b8  -= 0x28;
        DAT_083a7c54  += 1;
        crt_sprintf(tickBuf, (const char*)&DAT_00561b04);
    }

    // ── ABORT IF SOCKET ERROR ─────────────────────────────────────────────────
    if (DAT_055ca018 != '\0') return;

    // ── PRE-RENDER SETUP ──────────────────────────────────────────────────────
    FUN_00404cd0();  // HashTable maintenance
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        // Formato: "Screen MM DD HH MM - YYYY" (25 chars + '\0')
        crt_sprintf((char*)&DAT_083a4174, "Screen %02d %02d %02d %02d - %04d",
                    (int)st.wMonth, (int)st.wDay, (int)st.wHour,
                    (int)st.wMinute, (int)st.wYear);
    }
    crt_sprintf(nameBuf, (const char*)&DAT_07d4b708);

    // Build window title: serverName + " " + charName
    {
        // DAT_00561694 es signed en el original (Game_SceneUpdate lo setea a -1
        // cuando "no server selected"). Tratarlo como unsigned haría si=0x1e y
        // leer OOB de DAT_083a45d8[0x3600] → strlen de basura → overflow del
        // titleBuf[64] → /GS cookie fail al retornar Game_MainLoop.
        int idx = (int)DAT_00561694;
        int si  = (idx < 0x1e) ? idx : 0x1e;
        if (si < 0) si = 0;
        const char* serverName = (const char*)&DAT_083a45d8 + si * 0x21e;
        const char* charName   = DAT_07abf5d8 ? (DAT_07abf5d8 + 0x1c1) : "";
        // Copia defensiva con tope: %s puede ser largo si DAT_07abf5d8+0x1c1
        // apunta a bytes no-inicializados; wsprintfA no trunca.
        char srvTrim[32];  strncpy_s(srvTrim, sizeof(srvTrim), serverName, 30); srvTrim[31] = 0;
        char chrTrim[32];  strncpy_s(chrTrim, sizeof(chrTrim), charName,   30); chrTrim[31] = 0;
        wsprintfA(titleBuf, "%s %s", srvTrim, chrTrim);
        // Append titleBuf to nameBuf
        int tlen = (int)strlen(titleBuf);
        int nlen = (int)strlen(nameBuf);
        if (nlen + tlen + 1 < (int)sizeof(nameBuf))
            memcpy(nameBuf + nlen, titleBuf, tlen + 1);
    }

    shiftHeld = 1;
    if ((GetAsyncKeyState(VK_SHIFT) >> 8) != 0)
        shiftHeld = 1 - shiftHeld;

    if (DAT_083a42ec != '\0' && shiftHeld == 1)
        FUN_00480620((const char*)&DAT_083a7c94, nameBuf, 1);

    // ── GL CLEAR ──────────────────────────────────────────────────────────────
    if (DAT_0055a7ac == 10) {
        // Sub-state 10: teal clear color
        glClearColor(*(float*)"\x00\x00\x40\x3c",  // 0.047f
                     *(float*)"\x00\x00\xc8\x3d",  // 0.098f
                     *(float*)"\x00\x00\x30\x3e",  // 0.172f
                     1.0f);
    } else {
        // Original: clear color negro (la captura del binario real lo confirma).
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    FUN_005119b0(0, 0, 0x280, 0x1e0);   // BeginOpengl(0,0,640,480) → push PROJECTION+MODELVIEW
    CHK("ML/post_viewport");
    glClear(0x4100);  // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
    // IDA Game_MainLoop (0x525D40): BeginOpengl(0,0,640,480); glClear; EndOpengl();
    // BUG-FIX 2026-06-28: antes era GL_PopMatrixAll() (hack que popea 8×2=16 matrices)
    // sobre un stack con sólo 2 pushes → GL_STACK_UNDERFLOW (0x504) cada frame.
    // El balance correcto es EndOpengl (pop MODELVIEW + PROJECTION), igual a IDA.
    FUN_00511bc0();   // EndOpengl → pop MODELVIEW + PROJECTION (balancea el BeginOpengl)
    CHK("ML/post_PopMatrix");

    // ── HARD GL STATE RESET (post-clear, pre-scene) ──────────────────────────
    // Sincroniza ESTADO REAL + CACHES que usan FUN_00511680/710/790/600 y
    // FUN_005119b0. Los setter de escena hacen `if (cache==X) skip glEnable(...)`,
    // así que si el cache quedó desincronizado por cualquier frame previo la
    // UI se dibuja con state incorrecto (blend off → quads opacos sobre negro,
    // textura off → rectángulos negros, etc). Aquí forzamos ambos en sync.
    //
    // Defaults elegidos para coincidir con lo que FUN_005119b0 deja tras su
    // primer push (Scene_Login llama FUN_005119b0 como primera acción).
    glDisable(GL_BLEND);       DAT_083a412c = 0;    // blend OFF (setters lo activarán)
    glEnable(GL_CULL_FACE);    DAT_083a411c = 1;
    glEnable(GL_TEXTURE_2D);   DAT_083a4125 = 1;
    glDisable(GL_ALPHA_TEST);  DAT_083a411d = 0;
    glEnable(GL_DEPTH_TEST);   DAT_083a411e = 1;
    glDepthMask(GL_TRUE);      DAT_083a42e8 = 1;
    glDisable(GL_FOG);         /* DAT_083a42ea es "fog-is-on" flag global del scene */
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    // Invalidar cache de última textura ligada para forzar rebind
    DAT_00561574 = 0xFFFFFFFFu;

    // NOTA: el workaround `glDisable(GL_CULL_FACE)` aquí era no-op: cada
    // Scene_* llama FUN_005119b0 después y ese re-habilita GL_CULL_FACE
    // (+ cache DAT_083a411c=1). Movido al inicio de cada scene render
    // (ver Scene_Login.cpp / Scene_CharSelect.cpp) donde sí surte efecto.
    // Los blend-setters 2D (FUN_00511680/710/790) ya llaman FUN_00511570
    // (DisableCullFace) antes de dibujar sprites, pero la geometría 3D
    // de fondo (terrain/entidades) necesita el disable explícito hasta que
    // auditemos winding en los emitters.

    renderStart = GetTickCount();
    renderFlag  = 0;

    // ── SCENE RENDER ──────────────────────────────────────────────────────────
    if (DAT_005615c0 == 2) { renderFlag = (char)Scene_Login();      CHK("ML/post_Scene_Login"); }
    if (DAT_005615c0 == 4) { renderFlag = (char)Scene_CharSelect(); CHK("ML/post_Scene_CharSelect"); }
    if (DAT_005615c0 == 5) { renderFlag = (char)Game_RenderTick();  CHK("ML/post_RenderTick"); }

    // (renderFlag=1 forzado para state==2 removido: Scene_Login ya devuelve
    //  bool correctamente ahora que el init-loop no se repite por frame y el
    //  buffer de bones no desborda. Forzarlo causaba SwapBuffers prematuro.)

    // FUN_00409cf0(0x83a4338);  // post-render hash tick — hardcoded abs-addr
    // artifact de Ghidra: 0x83a4338 no es un símbolo real en nuestro build.
    // Es cleanup de lista enlazada de canales de audio (Net_StopAllChannels),
    // no-op sin la estructura construida.

    if (DAT_083a42ec != '\0') {
        if (DAT_083a410c != '\0')
            FUN_0045fa20("Data2/MonsterSetBase2.txt");
        FUN_00511140();
    }

    if (DAT_083a42ec != '\0' && shiftHeld == 0)
        FUN_00480620((const char*)&DAT_083a7c98, nameBuf, 1);

    DAT_083a42ec = '\0';

    CHK("ML/pre_swap");
    // ── SWAP BUFFERS ──────────────────────────────────────────────────────────
    if (renderFlag & 0xff) {
        glFlush();
        SwapBuffers(param_1);
    }
    CHK("ML/post_swap");

    // ── POST-RENDER 25fps FRAME LIMITER ──────────────────────────────────────
    {
        DWORD now = GetTickCount();
        DWORD elapsed = now - renderStart;
        while (elapsed < 0x28) {
            now = GetTickCount();
            elapsed = now - renderStart;
        }
        DAT_005616b8 += elapsed;
    }

    // ── CONNECTION CHECK ──────────────────────────────────────────────────────
    // BUG-FIX 2026-04-28: este check leía *(int*)(DAT_055ca160 + 8) y comparaba
    // con -1. En el original DAT_055ca160 era un Object* con un Type field en
    // +8; en nuestro port DAT_055ca160 es un buffer estático sin esa estructura
    // → el read devolvía garbage que a veces == -1 → disparaba 0x71 ConnLost
    // 100ms después de JoinMapServer, anulando todos los menús.
    // Real disconnect detection: WSA FD_CLOSE event en WinMain; ese path setea
    // DAT_055ca018 que ya bloquea Game_MainLoop al inicio.
    #if 0
    if (DAT_083a7c48 != '\0' && DAT_005615c0 == 5) {
        if (FUN_0043dcc0(((int)(uintptr_t)DAT_055ca160)) == -1) {
            if (DAT_083a7c58 == 0) {
                DAT_083a7c58 = 1;
                FUN_00405540(&DAT_055c9bf0, "> Connection closed...");
                FUN_004055a0(1);
            }
            FUN_005142d0(0x71);
        }
    }
    #endif

    // ── LIVECLIENT KEEPALIVE (opcode 0x0E) ────────────────────────────────────
    // BUG-FIX 2026-04-28: server MuEmu (Protocol.cpp:68 CGLiveClientRecv)
    // espera C1/0E cada ~1-3 seg después de OBJECT_LOGGED. Sin este packet,
    // el server timeout-ea y manda F1/02 sub=0 (Exit). Era la causa del
    // "se cierra sin cartel" después de entrar al mundo.
    //
    // Packet: [C1] [0x0B] [0x0E] [TickCount:DWORD] [PhysiSpeed:WORD] [MagicSpeed:WORD]
    // Total = 11 bytes.
    // 2026-04-29: keepalive 0x0E re-habilitado. Con el send() hook en MuEmu.h/cpp
    // toda send call ahora se auto-encripta si el primer byte es C1/C2/C3/C4 plain.
    // 2026-05-04: gate cambiado de `state==5` a `state>=4 || sub==7`. El
    // server cerraba el socket DURANTE la carga de mapa (state=5 sub=7 →
    // sub=0 transición), 5+ s de silencio del cliente. Ahora keepalive
    // empieza desde char-select (state=4) y sigue mientras carga el mapa.
    #if 1
    if (DAT_055ca168 != 0xffffffff &&
        (DAT_005615c0 == 4 || DAT_005615c0 == 5 || DAT_0055a7ac == 7)) {
        static DWORD s_lastLive = 0;
        DWORD now = GetTickCount();
        if (now - s_lastLive >= 1000) {
            s_lastLive = now;
            BYTE pkt[11];
            pkt[0]  = 0xC1;
            pkt[1]  = 0x0B;
            pkt[2]  = 0x0E;
            *(DWORD*)(pkt + 3) = now;
            *(WORD*)(pkt + 7)  = 0;
            *(WORD*)(pkt + 9)  = 0;
            Net_SendSmallPacket(pkt, 11);
        }
    }
    #endif

    // Login BGM
    if (DAT_005615c0 == 2)
        FUN_00412890(PTR_DAT_005615c8, 0);

    if (DAT_005615c0 != 5) return;

    // ── BGM STATE MACHINE (g_GameSubState) ───────────────────────────────────
    switch (DAT_0055a7ac) {
    case 0:  // Connecting
        if (DAT_07e118e8 == 4) {
            FUN_00404c60(0); FUN_00404c60(1);
        } else {
            FUN_00404bc0(0, 0, 1);
            if (DAT_07c74ae4 > 0)
                FUN_00404bc0(1, 0, 1);
        }
        break;
    case 1:  // Entering world
        FUN_00404bc0(3, 0, 1);
        break;
    case 2:  // In-world
        if (DAT_07e118e8 == 3 || DAT_07e118e8 > 9)
            FUN_00404c60(0);
        else
            FUN_00404bc0(0, 0, 1);
        break;
    case 3:  // Outdoor
        FUN_00404bc0(0, 0, 1);
        if ((_rand() & 0x1ff) == 0)
            FUN_00404bc0(2, 0, 0);  // ambient rare
        break;
    case 4:
        FUN_00404bc0(5, 0, 1); break;
    case 7:
        FUN_00404bc0(6, 0, 1); break;
    case 8:
        FUN_00404bc0(7, 0, 1); break;
    case 10:
        FUN_00404bc0(0x14, 0, 1);
        if (_rand() % 100 != 0) _rand();
        break;
    }

    // Stop tracks not active in this sub-state
    if (DAT_0055a7ac != 0 && DAT_0055a7ac != 2 && DAT_0055a7ac != 3) FUN_00404c60(0);
    if (DAT_0055a7ac != 0 && DAT_0055a7ac != 9)                       FUN_00404c60(1);
    if (DAT_0055a7ac != 1)                                             FUN_00404c60(3);
    if (DAT_0055a7ac != 3)                                             FUN_00404c60(2);
    if (DAT_0055a7ac != 4)                                             FUN_00404c60(5);
    if (DAT_0055a7ac != 7)                                             FUN_00404c60(6);
    if (DAT_0055a7ac != 8)                                             FUN_00404c60(7);
    if (DAT_0055a7ac != 10)                                            FUN_00404c60(0x14);

    // ── WINDOW TITLE (sub-state 0) ────────────────────────────────────────────
    if (DAT_0055a7ac == 0) {
        char dead = *(char*)(DAT_07abf5d8 + 0x34e);
        if (dead != '\0') {
            if (DAT_07e118e8 == 4)
                FUN_00412890(PTR_DAT_005615c4, 0);
            else
                FUN_00412890(PTR_DAT_005615c8, 0);
        }
    } else {
        FUN_004127f0(PTR_DAT_005615c4, 0);
        FUN_004127f0(PTR_DAT_005615c8, 0);
    }

    // ── SUB-STATE 2: IN-WORLD ANTI-TAMPER CHECK ───────────────────────────────
    // Tracks entity position fields (0x388, 0x38c) via hash table.
    // The chain: if val > 0xCC → check again; if < 0xD7 → check 0x38C;
    // if 0x38C value < 0x20 → BGM_Play(track_cc,0) (login BGM?)
    // Otherwise BGM_Play(track_d0,0). This is anti-cheat, not game logic.
    if (DAT_0055a7ac == 2) {
        char dead = *(char*)(DAT_07abf5d8 + 0x34e);
        if (dead != '\0') {
            int val388 = *(int*)(DAT_07abf5d8 + 0x388);
            if (val388 > 0xcc) {
                val388 = *(int*)(DAT_07abf5d8 + 0x388); // re-read after hash tracking
                if (val388 < 0xd7) {
                    int val38c = *(int*)(DAT_07abf5d8 + 0x38c);
                    if (val38c > 0xc) {
                        val38c = *(int*)(DAT_07abf5d8 + 0x38c); // re-read
                        if (val38c < 0x20) {
                            FUN_00412890(PTR_DAT_005615cc, 0);
                            goto LAB_00527402;
                        }
                    }
                }
            }
            FUN_00412890(PTR_DAT_005615d0, 0);
        }
    } else {
        FUN_004127f0(PTR_DAT_005615cc, 0);
        FUN_004127f0(PTR_DAT_005615d0, 0);
    }

LAB_00527402:
    // Sub-state 3 window title
    if (DAT_0055a7ac == 3) {
        if (*(char*)(DAT_07abf5d8 + 0x34e) != '\0')
            FUN_00412890(PTR_DAT_005615d4, 0);
    } else {
        FUN_004127f0(PTR_DAT_005615d4, 0);
    }

    // Sub-states 1/5 window title
    if (DAT_0055a7ac == 1 || DAT_0055a7ac == 5)
        FUN_00412890(PTR_DAT_005615d8, 0);
    else
        FUN_004127f0(PTR_DAT_005615d8, 0);
}
