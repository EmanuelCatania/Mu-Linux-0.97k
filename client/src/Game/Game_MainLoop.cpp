// Game_MainLoop.cpp
// Game_MainLoop @ 0x00525D40
//
// Main frame loop for SceneFlag 2/4/5 (Login/CharSelect/InGame).
// Called every frame from Scene_Dispatch.
// Handles: 25fps frame limiter, game logic tick, render, audio, BGM state machine.
//
// Signature: void __cdecl Game_MainLoop(HDC param_1)
//
// Frame structure:
//   1. Net_Recv() poll
//   2. Pasos lógicos acumulados de 40 ms; conservan la recuperación tras demoras.
//   3. GL clear + scene render
//   4. SwapBuffers
//   5. Post-render 25fps limiter
//   6. BGM state machine
//   7. Window title updates
//
// Globals:
//   SceneFlag  — SceneFlag
//   World  — World
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
//   ChatTime  — countdown counter A
//   DAT_07e11d7c  — countdown counter B
//   DAT_0839bc8c  — frame index mod 32

#include "stdafx.h"
#include "Entity/CharacterAttributeView.h"
#include "Net/ServerCharacterStats.h"
#include "Game/FrameLimiter.h"
#include "Game/MapManager.h"
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


// Freno de rendimiento, no es parte del port: ChkHeapPublic() llama a
// _CrtCheckMemory(), que recorre TODO el heap de debug, y abajo se invoca 16
// veces por frame. Es instrumentación nuestra para rastrear corrupción de heap
// (ver ChkHeapPublic en WinMain.cpp). Apagado por defecto; poner en 1 para
// reactivarlo.
#define ML_HEAP_CHECK 0
#if ML_HEAP_CHECK
#define CHK(tag) ChkHeapPublic(tag)
#else
#define CHK(tag) ((void)0)
#endif

void __cdecl Game_MainLoop(HDC param_1)
{
    char   titleBuf[64];
    char   nameBuf[256];
    char   tickBuf[100];
    int    renderFlag = 0;
    DWORD  renderStart;
    int    shiftHeld;

    CHK("ML/enter");

    // Diagnóstico: log de entrada, limitado por estado.
    {
        static int s_lastEnterState = -1;
        static DWORD s_lastEnterT = 0;
        DWORD now = GetTickCount();
        if (SceneFlag != s_lastEnterState || now - s_lastEnterT > 2000) {
            s_lastEnterState = SceneFlag;
            s_lastEnterT = now;
        }
    }

    // ── RECEIVE INCOMING PACKETS ─────────────────────────────────────────────
    Timer_UpdateFrameTiming();
    gWindow.UpdateTitle();
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
                    unsigned h = (**(unsigned(__cdecl**)(DWORD))(MAIN_HASH_CLASS + 0xc))(key);
                    while (((DWORD*)DAT_055c9bd0)[h] != 0 && ((DWORD*)DAT_055c9bd0)[h] != key)
                        h = (h + 1) % DAT_055c9bd4;
                    ((DWORD*)DAT_055c9bcc)[h] = val;
                    ((DWORD*)DAT_055c9bd0)[h] = key;
                }
            }
            operator_delete(old_be8);
            operator_delete(old_bec);
        }

        UI_InGameMenu();

        CHK("ML/pre_tick");
        // Dispatch per-state game logic
        if (SceneFlag == 2) { Game_SceneUpdate();     CHK("ML/post_SceneUpdate"); }
        if (SceneFlag == 4) { Game_EnterWorldTick();  CHK("ML/post_EnterWorld"); }
        if (SceneFlag == 5) { Game_CharSelectTick();  CHK("ML/post_CharSelectTick"); }

        // IDA 0x52626B: cinco pasos de fisica por frame con fTime = 0.005
        // (0x3BA3D70A).  Ademas del recorrido de la lista (vacia en el 0.97k:
        // nadie registra objetos en g_PhysicsManager), cada paso mueve el
        // viento flt_590AF0 que usa la tela (sub_4089B0).
        for (int i = 0; i < 5; i++) CPhysicsManager_Move(g_PhysicsManager, 0.005f);

        // Input update
        Chat_TickNoticeTimer();   CHK("ML/post_0047fcb0");
        Chat_TickMessageTimer();   CHK("ML/post_00480950");

        // F12 → toggle screenshot mode
        if (PressKey(0x2c) & 0xff)
            DAT_083a42ec ^= 1;

        // Decrement countdown counters
        if (ChatTime > 0) ChatTime--;
        if (DAT_07e11d7c > 0) DAT_07e11d7c--;

        // Frame index mod 32
        DAT_0839bc8c = (DAT_0839bc8c + 1) & 0x1f;

        // Anti-tamper: track DAT_083a7c00 ref-count, then increment
        {
            unsigned idx = HashTable_GetIndex(&MAIN_HASH_CLASS, &DAT_083a7c00);
            if (idx == 0xffffffff) {
                void* node = AntiTamper_HashNode(); *((BYTE*)node + 4) = 1;
                HashTable_Insert(&MAIN_HASH_CLASS, node, &DAT_083a7c00);
            }
        }
        DAT_083a7c00++;

        // Consume 40ms of budget, count frame
        DAT_005616b8  -= 0x28;
        DAT_083a7c54  += 1;
        crt_sprintf(tickBuf, (const char*)&DAT_00561b04);
    }

    // ── ABORT IF SOCKET ERROR ─────────────────────────────────────────────────
    if (DAT_055ca018 != '\0') return;

    // ── PRE-RENDER SETUP ──────────────────────────────────────────────────────
    Sound_Update3DPositions();
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        // IDA 0x525D40 L308:
        //   sprintf(GrabFileName, "Screen(%02d_%02d-%02d_%02d)-%04d.jpg",
        //           st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
        // GrabScreen (DAT_083a42f0) lo incrementa SaveScreen modulo 10000.
        //
        // DESVIACION DELIBERADA (pedido del usuario): el binario
        // guarda en la RAIZ del cliente -- GrabFileName no lleva ruta.  Para
        // no ensuciarla, las capturas van a "Screenshots/".  La carpeta se
        // crea una sola vez por sesion y, si no se puede crear, se cae a la
        // raiz, que es el comportamiento original.
        //
        // Se usa barra normal a proposito: fopen la acepta en Windows y es lo
        // que ya usa el resto del archivo (ver Monster_SaveSetBase mas abajo).
        //
        // SCREENSHOT_DIR_DEVIATION en 0 devuelve el comportamiento de IDA.
        #define SCREENSHOT_DIR_DEVIATION 1
        const char* shotDir = "";
#if SCREENSHOT_DIR_DEVIATION
        {
            static int s_dirReady = -1;   // -1 sin probar, 1 lista, 0 fallback
            if (s_dirReady < 0) {
                s_dirReady = (CreateDirectoryA("Screenshots", NULL) != 0 ||
                              GetLastError() == ERROR_ALREADY_EXISTS) ? 1 : 0;
            }
            if (s_dirReady == 1) shotDir = "Screenshots/";
        }
#endif
        crt_sprintf((char*)&GrabFileName, "%sScreen(%02d_%02d-%02d_%02d)-%04d.jpg",
                    shotDir, (int)st.wMonth, (int)st.wDay, (int)st.wHour,
                    (int)st.wMinute, (int)DAT_083a42f0);
    }
    // IDA L309: sprintf(strText, GlobalText[459], GrabFileName).
    // GlobalText[459] es "%s: La captura fue guardada." -- lleva un %s con el
    // nombre del archivo. El port no pasaba el argumento, asi que el %s
    // consumia un valor cualquiera de la pila.
    crt_sprintf(nameBuf, (const char*)&DAT_07d4b708, (const char*)&GrabFileName);

    // Build window title: serverName + " " + charName
    {
        // ServerSelectHi es signed en el original (Game_SceneUpdate lo setea a -1
        // cuando "no server selected"). Tratarlo como unsigned haría si=0x1e y
        // leer OOB de DAT_083a45d8[0x3600] → strlen de basura → overflow del
        // titleBuf[64] → /GS cookie fail al retornar Game_MainLoop.
        int idx = (int)ServerSelectHi;
        int si  = (idx < 0x1e) ? idx : 0x1e;
        if (si < 0) si = 0;
        const char* serverName = (const char*)&DAT_083a45d8 + si * 0x21e;
        const char* charName   = DAT_07abf5d8 ? (DAT_07abf5d8 + 0x1c1) : "";
        // Copia defensiva con tope: %s puede ser largo si DAT_07abf5d8+0x1c1
        // apunta a bytes no-inicializados; wsprintfA no trunca.
        char srvTrim[32];  strncpy_s(srvTrim, sizeof(srvTrim), serverName, 30); srvTrim[31] = 0;
        char chrTrim[32];  strncpy_s(chrTrim, sizeof(chrTrim), charName,   30); chrTrim[31] = 0;
        wsprintfA(titleBuf, " [%s / %s]", srvTrim, chrTrim);   // IDA L334
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
        UIChatLogWindow_AddText((const char*)&DAT_083a7c94, nameBuf, 1);

    // ── GL CLEAR ──────────────────────────────────────────────────────────────
    if (World == 10) {
        // Sub-state 10: teal clear color
        glClearColor(*(float*)"\x00\x00\x40\x3c",  // 0.047f
                     *(float*)"\x00\x00\xc8\x3d",  // 0.098f
                     *(float*)"\x00\x00\x30\x3e",  // 0.172f
                     1.0f);
    } else {
        // Original: clear color negro (la captura del binario real lo confirma).
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    GL_BeginViewport(0, 0, 0x280, 0x1e0);   // BeginOpengl(0,0,640,480) → push PROJECTION+MODELVIEW
    CHK("ML/post_viewport");
    glClear(0x4100);  // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
    // IDA Game_MainLoop (0x525D40): BeginOpengl(0,0,640,480); glClear; EndOpengl();
    // EndOpengl balancea el BeginOpengl (pop MODELVIEW + PROJECTION).
    GL_EndOpenGL();   // EndOpengl → pop MODELVIEW + PROJECTION (balancea el BeginOpengl)
    CHK("ML/post_PopMatrix");

    // ── HARD GL STATE RESET (post-clear, pre-scene) ──────────────────────────
    // Sincroniza ESTADO REAL + CACHES que usan GL_SetBlendSrcOver/710/790/600 y
    // GL_BeginViewport. Los setter de escena hacen `if (cache==X) skip glEnable(...)`,
    // así que si el cache quedó desincronizado por cualquier frame previo la
    // UI se dibuja con state incorrecto (blend off → quads opacos sobre negro,
    // textura off → rectángulos negros, etc). Aquí forzamos ambos en sync.
    //
    // Defaults elegidos para coincidir con lo que GL_BeginViewport deja tras su
    // primer push (Scene_Login llama GL_BeginViewport como primera acción).
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
    // Scene_* llama GL_BeginViewport después y ese re-habilita GL_CULL_FACE
    // (+ cache DAT_083a411c=1). Movido al inicio de cada scene render
    // (ver Scene_Login.cpp / Scene_CharSelect.cpp) donde sí surte efecto.
    // Los blend-setters 2D (GL_SetBlendSrcOver/710/790) ya llaman GL_DisableCullFace
    // (DisableCullFace) antes de dibujar sprites, pero la geometría 3D
    // de fondo (terrain/entidades) necesita el disable explícito hasta que
    // auditemos winding en los emitters.

    renderStart = GetTickCount();
    renderFlag  = 0;

    // ── SCENE RENDER ──────────────────────────────────────────────────────────
    if (SceneFlag == 2) { renderFlag = (char)Scene_Login();      CHK("ML/post_Scene_Login"); }
    if (SceneFlag == 4) { renderFlag = (char)Scene_CharSelect(); CHK("ML/post_Scene_CharSelect"); }
    if (SceneFlag == 5) { renderFlag = (char)Game_RenderTick();  CHK("ML/post_RenderTick"); }

    // (renderFlag=1 forzado para state==2 removido: Scene_Login ya devuelve
    //  bool correctamente ahora que el init-loop no se repite por frame y el
    //  buffer de bones no desborda. Forzarlo causaba SwapBuffers prematuro.)

    CPhysicsManager_Render(g_PhysicsManager);   // IDA 0x5269C3: CPhysicsManager::Render

    if (DAT_083a42ec != '\0') {
        if (DAT_083a410c != '\0')
            Monster_SaveSetBase("Data2/MonsterSetBase2.txt");
        GL_CaptureScreenshot();
    }

    if (DAT_083a42ec != '\0' && shiftHeld == 0)
        UIChatLogWindow_AddText((const char*)&DAT_083a7c98, nameBuf, 1);

    DAT_083a42ec = '\0';

    CHK("ML/pre_swap");
    // ── SWAP BUFFERS ──────────────────────────────────────────────────────────
    if (renderFlag & 0xff) {
        glFlush();
        SwapBuffers(param_1);
    }
    CHK("ML/post_swap");

    // ── POST-RENDER 25fps FRAME LIMITER ──────────────────────────────────────
    DAT_005616b8 += gFrameLimiter.Wait(renderStart);

    // ── CONNECTION CHECK (desactivado) ────────────────────────────────────────
    // En el original SocketClient es un Object* con un Type en +8; en nuestro port
    // es un buffer estático sin esa estructura, así que este chequeo leería basura.
    // La desconexión se detecta por el FD_CLOSE en WinMain, que setea
    // DAT_055ca018 y ya bloquea Game_MainLoop al inicio.
    #if 0
    if (DAT_083a7c48 != '\0' && SceneFlag == 5) {
        if (CWsctlc_GetSocket(((int)(uintptr_t)SocketClient)) == -1) {
            if (DAT_083a7c58 == 0) {
                DAT_083a7c58 = 1;
                CErrorReport_Write(&DAT_055c9bf0, "> Connection closed...");
                CErrorReport_WriteCurrentTime(1); // IDA: FUN_004055A0
            }
            SetErrorMessage(0x71);
        }
    }
    #endif

    // ── LIVECLIENT KEEPALIVE (opcode 0x0E) ────────────────────────────────────
    // El server MuEmu (CGLiveClientRecv) espera C3/0E cada ~1-3 s después de
    // OBJECT_LOGGED; si no llega, manda F1/02 sub=0 (Exit). Se envía también desde
    // char-select y durante la carga del mapa, porque el server cierra el socket
    // si el cliente queda en silencio en esa transición.
    //
    // DESVIACION MuEmu: Protocol.h::PMSG_LIVE_CLIENT_RECV, 12 bytes con padding.
    // HackPacketCheck exige C3; gNetwork recibe el C1 lógico y lo cifra.
    #if 1
    if (SocketClientSocket != 0xffffffff &&
        (SceneFlag == 4 || SceneFlag == 5 || World == 7)) {
        static DWORD s_lastLive = 0;
        DWORD now = GetTickCount();
        if (now - s_lastLive >= 1000) {
            s_lastLive = now;
            Proto::PMSG_LIVE_CLIENT_RECV pkt{};
            pkt.header = { 0xC1, sizeof(pkt), 0x0E };
            pkt.TickCount = now;
            // F3/E1 ya trae la velocidad del server: no descontar bebida dos veces.
            if (!gServerCharacterStats.GetSpeeds(pkt.PhysiSpeed, pkt.MagicSpeed) && CharacterAttribute) {
                const CharacterAttributeView attributes((void*)(uintptr_t)CharacterAttribute);
                const WORD physical = attributes.PhysicalSpeed();
                const WORD magic = attributes.MagicSpeed();
                // Respaldo local como CGLiveClientSend del DLL, sin underflow.
                const WORD drink = (attributes.Effects() & 9) ? 20 : 0;
                pkt.PhysiSpeed = physical >= drink ? physical - drink : 0;
                pkt.MagicSpeed = magic >= drink ? magic - drink : 0;
            }
            gNetwork.Send((const BYTE*)&pkt, sizeof(pkt));
        }
    }
    #endif

    // DESVIACION: IDA 0x525D40 pide g_lpszMp3[1] (MuTheme) en el login, el
    // mismo tema que Lorencia. Acá el login pide su propio archivo,
    // Data\Music\MuTheme.mp3, como el DLL (Patchs.cpp parchea ese nombre en
    // 0x5616D0). El pack no lo trae, así que PlayMp3 no encuentra el archivo y
    // el login queda sin música, como en el 0.97k con la opción de música en su
    // default (apagada). Para tener música en el login alcanza con dejar un mp3
    // con ese nombre. MuTheme es el Lorencia.mp3 del pack (mismo MD5).
    if (SceneFlag == 2)
        Music_PlayTrack((DWORD)(uintptr_t)"Data\\Music\\MuTheme.mp3", 0);

    if (SceneFlag != 5) return;

    // ── Sonidos ambientales por mapa ───────────────────────────────────────
    switch (World) {
    case 0:  // Lorencia
        if (DAT_07e118e8 == 4) {
            Sound_StopBuffer(0); Sound_StopBuffer(1);
        } else {
            PlayBuffer(0, 0, 1);
            if (DAT_07c74ae4 > 0)
                PlayBuffer(1, 0, 1);
        }
        break;
    case 1:  // Dungeon
        PlayBuffer(3, 0, 1);
        break;
    case 2:  // Devias
        if (DAT_07e118e8 == 3 || DAT_07e118e8 > 9)
            Sound_StopBuffer(0);
        else
            PlayBuffer(0, 0, 1);
        break;
    case 3:  // Noria
        PlayBuffer(0, 0, 1);
        if ((_rand() & 0x1ff) == 0)
            PlayBuffer(2, 0, 0);  // ambient rare
        break;
    case 4:
        PlayBuffer(5, 0, 1); break;
    case 7:
        PlayBuffer(6, 0, 1); break;
    case 8:
        PlayBuffer(7, 0, 1); break;
    case 10:
        PlayBuffer(0x14, 0, 1);
        if (_rand() % 100 != 0) _rand();
        break;
    }

    // Corta los ambientales de los otros mapas
    if (World != 0 && World != 2 && World != 3) Sound_StopBuffer(0);
    if (World != 0 && World != 9)                       Sound_StopBuffer(1);
    if (World != 1)                                             Sound_StopBuffer(3);
    if (World != 3)                                             Sound_StopBuffer(2);
    if (World != 4)                                             Sound_StopBuffer(5);
    if (World != 7)                                             Sound_StopBuffer(6);
    if (World != 8)                                             Sound_StopBuffer(7);
    if (World != 10)                                            Sound_StopBuffer(0x14);

    // Música de fondo del mapa (IDA 0x00526D0C..0x00527475): ver CMapManager.
    gMapManager.UpdateMusic();
}
