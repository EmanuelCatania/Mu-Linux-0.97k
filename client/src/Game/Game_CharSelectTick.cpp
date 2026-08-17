// Game_CharSelectTick.cpp
// Game_CharSelectTick @ 0x00524E30
//
// In-game per-frame tick called once the character enters the world.
// Two phases:
//   1) First call (DAT_083a7c4c == 0): send 0xC1/0xF3 char-select packet, init subsystems.
//   2) Every call: wait for server ack (DAT_05826cb0 == 0x3d → DAT_083a7c10=1),
//      then run the full per-frame update pipeline.
//
// Packet sent (first call, if DAT_083a410c == 0):
//   [0xC1][len][0xF3][charName(10B, zero-padded, XOR-enc)][slot_byte]
//
// Globals:
//   DAT_083a7c4c  — one-time init guard
//   DAT_083a7c10  — server ack flag (gates per-frame logic)
//   DAT_05826cb0  — server response: 0x3c=sent, 0x3d=confirmed
//   DAT_083a42e9  — special viewport mode flag
//   DAT_005590ac  — flag enabling anti-tamper block
//   DAT_0055a7ac  — g_GameSubState
//   DAT_07eaa11b  — anti-tamper sequence counter
//   DAT_07eaa116/117 — anti-tamper flags
//   DAT_007eaa118..11c — anti-tamper bytes

#include "stdafx.h"
#include "Game/Game_CharSelectTick.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"

extern "C" void Bisect_ChatMode(const char* tag);

// Same 32-byte XOR key
static const BYTE s_Key[32] = {
    0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
    0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
    0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
    0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
};

void Game_CharSelectTick(void)
{
    // ── ONE-TIME INIT ─────────────────────────────────────────────────────────
    if (DAT_083a7c4c == '\0') {
        FUN_00405540(&DAT_055c9bf0, "> Character selected <%d> %s");
        DAT_083a7c4c = 1;

        // 2026-05-05: BUG-FIX FINAL del FD_CLOSE post-F3/03. Este send es
        // un DUPLICADO del que manda Send_CharSelectPacket en
        // Game_EnterWorldTick.cpp:621 cuando el user clickea OK. Aunque el
        // server CGCharacterInfoRecv early-returns por OBJECT_ONLINE, el
        // packet duplicado dispara el kick. Mensaje confirmado contra DLL
        // companion: el binario original NO tiene este send extra en
        // Game_CharSelectTick init — solo en Send_CharSelectPacket.
        // SKIPEAR COMPLETAMENTE.
        #if 0
        if (DAT_083a410c == '\0' && DAT_005615c0 != 5) {
            // Build and send 0xC1/0xF3 char-name packet
            DAT_05826cb0 = 0x3c;

            // Get character name from entity array.
            // Guardrails: DAT_005616ac == -1 means "no char selected" → would
            // dereference -0x1d3 offset and feed strlen 0xCD heap-fill bytes,
            // returning a huge nameLen → padLen negative → memset wild AV.
            // Treat unset/invalid as slot 0; cap nameLen at 10.
            int slotIdx = (int)DAT_005616ac;
            if (slotIdx < 0 || slotIdx >= 5) slotIdx = 0;
            char  safeName[16] = {0};
            if (DAT_07abf5d0) {
                char* charNameSrc = (char*)(DAT_07abf5d0 + 0x1c1 + slotIdx * 0x394);
                for (int k = 0; k < 10; ++k) safeName[k] = charNameSrc[k];
            }
            safeName[10] = 0;
            char* charName = safeName;
            int   nameLen  = (int)strlen(safeName);
            if (nameLen > 10) nameLen = 10;
            int   padLen   = 10 - nameLen;
            // Use safeName as the source for the encrypted copy below.
            charName = safeName;

            // BUG-FIX 2026-04-28: el server MuEmu (PacketManager.cpp:486
            // CPacketManager::XorData) descifra el body con la fórmula
            //     m_buff[n] ^= m_buff[n - 1] ^ m_XorFilter[n % 32]   (n--)
            // o sea usando el byte PREVIO.  Nuestro port de Ghidra usaba
            // pkt[i+1] (byte siguiente) → encoding inverso → el server al
            // descifrar producía garbage en el sub-opcode → switch(lpMsg[3])
            // no matcheaba 0x03 → silently drop → no F3/03 response.
            //
            // Misma fórmula que la login en Game_SceneUpdate.cpp:135 (que sí
            // funciona).  Encoding: pkt[i] ^= pkt[i-1] ^ key[i % 32].
            //
            // El packet wire format es:
            //   [C1] [0x0E] [encrypted 0xF3] [encrypted 0x03] [encrypted name 10B]
            //   ↑                                                           ↑
            //   header preserved (no XOR)                              charName
            //
            // El server descifra el body desde size-1 hacia atrás (por byte
            // previo), recupera [F3] [03] [name], dispatch a CGCharacterInfoRecv.
            BYTE pkt[32];
            memset(pkt, 0, sizeof(pkt));
            pkt[0] = 0xC1;
            pkt[1] = 0x0E;          // length = 14
            pkt[2] = 0xF3;           // opcode
            pkt[3] = 0x03;           // sub-opcode: CharacterInfoRecv
            memcpy(pkt + 4, charName, nameLen);   // padding bytes already zero
            int pos = 14;
            // XOR-encode body with key[i % 32] using prev-byte chain (i = 3..pos-1).
            // i=2 (header head) is INCLUDED so server's switch(lpMsg[3])
            // post-XorData sees the descrambled sub-opcode.  XorData range is
            // (size-1, end) with end=2 for C1, decoding back from size-1 to 3
            // — so the client must encode forward i=3..size-1 using the SAME
            // chain rule.
            for (int i = 3; i < pos; i++) {
                pkt[i] ^= pkt[i - 1] ^ s_Key[i & 0x1f];
            }

            // MuEmu compat: cifrar el buffer ANTES de send().  El cliente
            // 0.97 original mandaba este F3 plain; MuEmu (HackCheck.cpp)
            // hace `(byte^0x42)-0x42` en recv → necesitamos el inverso aquí.
            MuEmu::EncryptSend(pkt, pos);

            // Send via socket with WSAEWOULDBLOCK fallback
            int len = pos;
            int sent = 0, rem = len;
            if (DAT_055ca168 != 0xffffffff) {
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
                        break;
                    }
                    if (n == 0) break;
                    if (DAT_055ce174) FUN_0043de60();
                    sent += n; rem -= n;
                } while (rem > 0);
            }
        }
        #endif // 0 — DUPLICATE F3/03 send disabled

        // Init in-game subsystems
        FUN_00511060();
        // IDA 0x525384: mov [CameraAngle+8], 0xC2340000 (= float -45.0).
        // BUG-FIX 2026-06-28: DAT_083a42c0 es DWORD& → `= -45.0f` convertía el
        // float a ENTERO -45 (0xFFFFFFD3) que leído como float es NaN. El yaw
        // NaN propagaba a AngleMatrix→VectorIRotate→CameraPosition (escena negra).
        // Escribir el float directamente (bit-pattern 0xC2340000), igual a IDA.
        CameraAngle[2] = -45.0f;  // yaw = -45° (iso rotation around Z)
        FUN_0047ec60(1);
        DAT_00559c84 = 0;
        DAT_07e11d71 = 0;
        DAT_00559c8c = 0x100;
        _DAT_00559c94 = 0x2a;
        _DAT_00559c98 = 10;
        DAT_00559c88 = 2;

        // 120× widget draw
        for (int i = 0x78; i > 0; i--)
            FUN_00480620((const char*)&DAT_083a7c90, (const char*)&DAT_083a7c8c, 0);

        ((BYTE*)&DAT_07e913a8)[0] = 0;
        DAT_07e91428 = 0;
        // DAT_055c9ff0 = ChatListBox engine object.  Vtable slot at +0x28
        // (= slot 10) is ChatLB_clearList — a __fastcall method that
        // expects `this` in ECX.  The previous Ghidra-style stub dispatch
        // here was `__cdecl` with no args, which left ECX = garbage, and
        // the called clearList walked random "list" data → AV reading
        // 0x81EC8B55 (instruction bytes — i.e. node[1] pointed into
        // .text).  Use the proper fastcall dispatch.
        if (DAT_055c9ff0 && *(int*)DAT_055c9ff0) {
            DWORD* obj = (DWORD*)DAT_055c9ff0;
            void** vt  = (void**)*obj;
            typedef void (__fastcall *FnClear)(DWORD*);
            ((FnClear)vt[10])(obj);
            FUN_0040e590((int)DAT_055c9ff0);
        }
        FUN_00405540(&DAT_055c9bf0, "> Main Scene init success");
        FUN_004055a0(1);
    }

    // ── WAIT FOR SERVER ACK ───────────────────────────────────────────────────
    // BUG-FIX 2026-04-28: el latch DAT_083a7c10 se settea cuando vemos
    // DAT_05826cb0==0x3d (CurrentProtocolState=61, set por Recv_JoinMapServer).
    // Pero nuestro Recv puede correr en un frame y luego DAT_05826cb0 cambia
    // antes que Game_CharSelectTick lo vea → latch nunca se settea →
    // Player_InputTick (movement) nunca corre → hero estático.
    // Relajación: si ya estamos en estado 5 con world cargado, asumir latch.
    if (DAT_05826cb0 == 0x3d || (DAT_005615c0 == 5 && (int)DAT_0055a7ac >= 0)) {
        DAT_083a7c10 = 1;
    } else if (DAT_083a7c10 == '\0') {
        return;
    }

    // ── PER-FRAME UPDATE ──────────────────────────────────────────────────────
    Bisect_ChatMode("CST_pre_4f95e0");
    // IDA 00524E30: EarthQuake = EarthQuake * 0.2.  This is a fixed
    // per-frame decay, not g_DeltaTime; using the latter removes the visible
    // camera impulse generated by effects such as Hell Fire.
    _DAT_083a0210 *= 0.2f;

    FUN_004f95e0();  // world tick
    Bisect_ChatMode("CST_post_4f95e0");

    DAT_07eaa160  = 0;
    DAT_0055a3e4  = 0xffffffff;
    DAT_07d78094  = 0;

    if (DAT_083a42e9 == '\0') {
        // Standard viewport mode
        DAT_07d78094 = (DAT_083a4278 > 0x1af) ? 1 : 0;
        DAT_055c9b80 = 0;
        if (DAT_055c9ff0 && *(int*)DAT_055c9ff0) {
            // vtable[+0x14] = slot 5 = ChatLB_tick (__fastcall, takes this+arg).
            DWORD* obj = (DWORD*)DAT_055c9ff0;
            void** vt  = (void**)*obj;
            typedef int (__fastcall *FnTick)(DWORD*, int /*edx*/, int);
            ((FnTick)vt[5])(obj, 0, 0);
        }
        Bisect_ChatMode("CST_post_chatLB");
        FUN_004ecb00();
        Bisect_ChatMode("CST_post_4ecb00");
        FUN_00402fd0((void*)(uintptr_t)DAT_00583d8c);
        Bisect_ChatMode("CST_post_402fd0");
        FUN_004b14f0();
        Bisect_ChatMode("CST_post_4b14f0_ChatInputTick");

        // ── Reposición per-frame del ChatListBox ────────────────────────────
        // FIX 2026-07-20: faltaba el `else` del `DAT_005590ac == 1`, y el caso
        // (-10, 81) estaba metido en la rama equivocada.
        //
        // Las 3 posiciones (verificadas en el binario en 0x5258D8: tres pares
        // de `push` que convergen en el mismo `call sub_40C690`) son:
        //     (186, 420)  sin paneles abiertos
        //     (  0, 420)  con inventario/character/trade/shop/etc. abiertos
        //     (-10,  81)  con el ChatListBox APAGADO → historial arriba-izquierda
        // La estructura es idéntica a CheckFunctionButtons (0x4C04A0), que sí
        // está bien portada en src/Input/Input.cpp:388-433; me guié por esa.
        //
        // Sin el `else`, al apagar el recuadro (botón 2 del popup o F4) el
        // widget se quedaba clavado en (186, 420) y los mensajes seguían
        // dibujándose ahí abajo en vez de volver arriba a la izquierda: las
        // filas se posicionan SIEMPRE en `this[11]+10, this[12]-13*n-16`
        // (IDA sub_40D610), así que mover el widget es lo único que las mueve.
        if (DAT_005590ac == 1) {
            int y, x;
            if (DAT_07eaa117 == '\0') {
                y = 0x1a4; x = 0xba;
            } else {
                if (DAT_07eaa116 == '\0') {
                    // Anti-tamper: track DAT_07eaa11b ref-count
                    unsigned idx = HashTable_GetIndex(&DAT_055c9bc8, &DAT_07eaa11b);
                    if (idx == 0xffffffff) {
                        void* node = operator_new(2); *((BYTE*)node+1)=1;
                        FUN_00403f80(&DAT_055c9bc8, node, &DAT_07eaa11b);
                    } else {
                        BYTE* node = *(BYTE**)(DAT_055c9bcc + idx * 4);
                        node[1]++;
                        if (node[1] < 2) FUN_00404330(&DAT_07eaa11b, node);
                    }
                    // Decrement
                    idx = HashTable_GetIndex(&DAT_055c9bc8, &DAT_07eaa11b);
                    if (idx != 0xffffffff) {
                        BYTE* node = *(BYTE**)(DAT_055c9bcc + idx * 4);
                        node[1]--;
                        if (node[1] == 0) FUN_00423710(node, &DAT_07eaa11b);
                    }
                    // TradeOpened (11b) + Warehouse (119) + ChaosMix (11a) +
                    // EventWindow (11c); después ShopOpened (118).  Per IDA,
                    // el primer grupo NO incluye 118 — acá se leía 118 en lugar
                    // de 11b.  Si TODOS están cerrados, el chat va a (186,420).
                    if (DAT_07eaa11b == '\0' && DAT_07eaa119 == '\0' &&
                        DAT_07eaa11a == '\0' && DAT_07eaa11c == '\0' &&
                        ((BYTE*)&DAT_07eaa118)[0] == '\0')
                    {
                        y = 0x1a4; x = 0xba;
                    } else {
                        y = 0x1a4; x = 0;
                    }
                } else {
                    // CharacterOpened != 0 → LABEL_32 = (0, 420).
                    // Acá había (-10, 81), que es la posición del modo APAGADO.
                    y = 0x1a4; x = 0;
                }
            }
            if (DAT_055c9ff0) {
                FUN_0040c690((void*)(uintptr_t)DAT_055c9ff0, x, y);
            }
        } else if (DAT_055c9ff0) {
            // ChatListBox apagado: historial como overlay arriba-izquierda.
            FUN_0040c690((void*)(uintptr_t)DAT_055c9ff0, (int)0xfffffff6, 0x51);
        }
    }

    if (DAT_083a7c24 != 0)
        DAT_07d78094 = 1;

    if (DAT_083a42e9 == '\0')
        FUN_00503760();
    Bisect_ChatMode("CST_post_503760");

    // Conditional skill effects
    bool doSkillFX = false;
    if (DAT_0055a7ac == 0) {
        if (DAT_07e118e8 == 4) doSkillFX = true;
    } else if (DAT_0055a7ac == 2) {
        if (DAT_07e118e8 == 3 || DAT_07e118e8 > 9) doSkillFX = true;
    } else if (DAT_0055a7ac == 3 || DAT_0055a7ac == 7 ||
               DAT_0055a7ac == 9 || DAT_0055a7ac == 10) {
        doSkillFX = true;
    }
    if (doSkillFX) FUN_0046cc80();
    Bisect_ChatMode("CST_post_skillFX");

    // Full world pipeline
    FUN_00500e80();           Bisect_ChatMode("CST_post_500e80");
    FUN_00502320();           Bisect_ChatMode("CST_post_502320");
    Object_MoveUpdate();      Bisect_ChatMode("CST_post_ObjMove");
    FUN_004821a0();           Bisect_ChatMode("CST_post_4821a0");
    FUN_004acef0();           Bisect_ChatMode("CST_post_PlayerInput");

    // 2026-05-03: per-entity animation tick RE-ENABLED. La concern de stack
    // corruption original venía de NULL-deref en hash table (FUN_00404280
    // returning NULL on key-mismatch). Con el sentinel hash setup ahora hay
    // un buffer válido siempre, y HashTable_GetIndex retorna -1 para que los
    // callers skip el deref.
    //
    // 2026-05-05: Wire MoveCharactersClient_stub (per-frame entity tick que
    // llama FUN_00454fc0 → FUN_00454cd0 path-walker para cada entidad). Sin
    // esto los monsters/NPCs llegaban con packet 0x10 (target_grid set) pero
    // nunca se invocaba el path-walker, así quedaban quietos en su pos
    // inicial. El path-walker SÍ existe y funciona — solo faltaba wirear.
    // 2026-05-05: per-frame entity tick.
    //   FUN_00454cd0 — path tick: pathfind (+0x306/7 target ≠ cached) y
    //                  advance waypoint cuando arrived. NO se llama para el
    //                  hero (Player_InputTick maneja su propio path/motion).
    //   FUN_004520c0 — copia entity.action y world pos al model. SÍ para
    //                  todos los entities (incl hero). Sin esto el model
    //                  queda en posición inicial.
    //   CharacterAnimation — avanza entity[+0x108] (frame counter). Para todos.
    {
        int heroEnt = (int)(uintptr_t)DAT_07abf5d8;
        int base = (int)(uintptr_t)DAT_07abf5d0;
        MoveCharactersClient_stub();
        for (int s = 0; false && s < 400; ++s) {
            int e = base + s * 0x394;
            if (*(char*)e == '\0') continue;   // inactive
            if (e != heroEnt) {
                FUN_00454cd0(e, e);            // path tick (skip hero)
            }
            FUN_004520c0(e);                   // action → model (incl hero)
            CharacterAnimation(e, e);          // advance anim frame (incl hero)
        }
    }

    // ── BUG-FIX 2026-07-16: MoveBugs FALTABA en char-select ─────────────────
    // IDA Game_CharSelectTick (00524E30 L531) llama MoveBugs(). Es el update que
    // hace fade-in del alpha de las entidades "bug" (Alpha() en MoveBugs L72) y
    // posiciona/anima las MONTURAS (Uniria bug=195 / Dinorant bug=267) siguiendo
    // al owner. Sin él, el alpha del mount queda en 0 → Calc_RenderObject lo
    // cullea (alpha < 0.01) → la montura nunca se dibuja. Verificado por diag:
    // el bug 267 existía en el pool pero Calc devolvía 0.
    extern void __stdcall MoveBugs_stub(void);
    MoveBugs_stub();

    Character_UpdateAll();
    FUN_00479380();
    FUN_00475090();
    FUN_0046b790();   // MoveEffects  (0x46b790)
    FUN_004736e0();   // MoveJoints   (0x4736e0)
    // ── BUG-FIX 2026-07-15: MoveParticles (0x477090) FALTABA en char-select ──
    // IDA Game_CharSelectTick (00524E30 L539) llama MoveParticles() acá. Es el
    // update que decrementa el lifetime de las partículas y las despawnea. Sin
    // él, las partículas que spawnean las wings/armas/efectos de los personajes
    // se renderizaban cada frame (RenderParticles) pero NUNCA morían → se
    // acumulaban con blend aditivo → haces dorados saliendo de los bordes de la
    // pantalla, intensificándose progresivamente. El tick in-world
    // (Game_EnterWorldTick L310) sí lo llama; el de char-select no lo tenía.
    extern void __stdcall MoveParticles_stub(void);
    MoveParticles_stub();   // MoveParticles (0x477090)
    Effect_UpdateAll();
    FUN_004794a0();

    // Anti-tamper ftol + frame counter checks
    if (DAT_0839bc86 != '\0' && DAT_07e11d30 == 0) {
        __ftol();
        __ftol();
        FUN_00403a30();
        FUN_00403a30();
    }

    FUN_004c04a0();
}
