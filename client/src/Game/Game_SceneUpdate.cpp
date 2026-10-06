// Game_SceneUpdate.cpp
// Game_SceneUpdate @ 0x0051F900
//
// Login scene tick — called every frame from Game_MainLoop while in login state.
// Drives the entire login → character-select pipeline as a state machine.
//
// ── LOGIN SUB-STATE (DAT_083a7c14) ───────────────────────────────────────────
//   0   ServerSelect    — Scene_Login_ServerSelect() rendered
//   1   Connecting      — progress bar; keep-alive 0xC1 every 3000 ms
//   2   CredentialInput — username/password dialog
//   3   AwaitLoginResp  — packet sent; waiting for server response
//   4   LoginFailed     — error dialog
//   8   CharSelectInit  — ClearInput(1); char select setup
//  12   Error/Timeout   — retry path (up to 4 retries)
//
// ── SERVER RESPONSE CODE (DAT_05826cb0) ──────────────────────────────────────
//   0x0b  Login OK      → state=2
//   0x0c  Login fail A  → state=4
//   0x0d  Login fail B  → state=4, DAT_07e11d78=6
//   0x14  Char list rdy → sends 0xC1/0xF3; transitions to char-select
//   0x15-0x26 errors    → state=2, various DAT_07e11d78 values
//   0x3e  special       → state=8, CharSelect_Init, DAT_07e113d9=2
//   0x40-0x47 success   → state=2, CharSelect_Init, restore version→username
//
// ── ANIMATION (DAT_005616a4 = dialog Y lerp) ─────────────────────────────────
//   state 0         : Y eases to 0x108 (ServerSelect bar)
//   state 1,3,5,7,9 : Y eases to 0x1c4 (credentials up)
//   state 2,4,6,8   : Y eases to 0x93  (credentials panel)

#include "stdafx.h"
#include "Net/PacketFrame.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" { void DbgLogPublic(const char*); }
#pragma warning(disable: 4554 4244 4018 4146)
#include "Game/Game_SceneUpdate.h"
#include "Net/Net.h"
#include "Net/MuEmu.h"
#include "Net/Network.h"


static void LoginScene_ApplySafeObjectAnim()
{
    if (!(SceneFlag == 2 || SceneFlag == 4))
        return;

    extern float DAT_05826e08;  // WorldTime
    extern void __cdecl MoveObject_Special(int pObj);  // Object_AnimUpdate safe subset: Alpha + login writes
    extern int __cdecl Calc_RenderObject(int param_1, char param_2, int param_3);  // Calc_RenderObject
    extern float* __cdecl Entity_SpawnEffects(int param_1);  // Entity_SpawnEffects / login ship fire
    extern void __stdcall MoveBugs(void);  // butterfly/helper owner-follow update

    for (int i = 0; i < 9; ++i) {
        char* obj = (char*)g_LoginSceneObjects[i];
        if (!obj || *obj == '\0')
            continue;

        // Match the original order more closely: Alpha(o) runs before the
        // animation tick and can early-out nearly invisible objects.
        MoveObject_Special((int)obj);
        if (*(float*)(obj + 0x168) < 0.01f)
            continue;

        short type = *(short*)(obj + 2);
        char* model = (char*)(DAT_05828d58 + type * 0xbc);
        if (model) {
            model[0xa0] = *(char*)(obj + 0x105);
            BMD__PlayAnimation((void*)model,
                (float*)(obj + 0x108),
                (float*)(obj + 0x10c),
                (void*)(obj + 0x106),
                *(float*)(obj + 0xcc));
        }

        if (type == 160 || type == 161) {
            float v = (float)((long long)DAT_05826e08 % 4000);
            *(float*)(obj + 112) = -(v * 0.00025f);
            continue;
        }

        if (type == 162) {
            float ramp = (DAT_083a7af4 != 0) ? 1.5f : ((float)(int)DAT_005615e8 * 0.002f);
            *(float*)(obj + 232) = ramp;
            *(float*)(obj + 236) = ramp;
            *(float*)(obj + 240) = ramp;
            *(float*)(obj + 104) = ramp;
        }

        // No spawnear efectos acá: Entity_SpawnEffects se llama una vez por entidad
        // desde el pase de render (tras Entity_PrepareRender, con los bones frescos),
        // como en el original.
    }

    // Login uses the Butterfles pool for the floating fairy/angel helper
    // created by CreateBug(0x330). Spawning it without the pool tick leaves
    // it frozen at its randomized birth offset; run the safe pool follower
    // update here so it tracks the elf preview like the original scene.
    if (SceneFlag == 2) {
        MoveBugs();
    }
}

static void LoginScene_ClearPreviewEquipmentMeta(unsigned char* c)
{
    if (!c) return;

    const int bodyMetaOff[7] = {
        0x1e2, 0x1fa, 0x212, 0x22a, 0x242, 0x25a, 0x272
    };

    for (int i = 0; i < 7; ++i) {
        c[bodyMetaOff[i] + 0] = 0;
        c[bodyMetaOff[i] + 1] = 0;
        c[bodyMetaOff[i] + 2] = 0;
        c[bodyMetaOff[i] + 3] = 0;
    }

    c[0x28a] = 0;
    c[0x28b] = 0;
    c[0x28c] = 0;
    c[0x28d] = 0;
    c[0x2a2] = 0;
    c[0x2a3] = 0;
    c[0x2a4] = 0;
    c[0x2a5] = 0;
    c[0x2ba] = 0;
    c[0x2bb] = 0;
    c[0x2bc] = 0;
    c[0x2bd] = 0;
}

// Encrypt bytes [start..end) in pkt using the login XOR key.
// Each byte: pkt[i] ^= key[i & 0x1f] ^ pkt[i] (Ghidra pattern)
static void Pkt_XorRange(BYTE* pkt, int start, int end)
{
    for (int i = start; i < end; i++)
        pkt[i] ^= CNetwork::XorKey[i & 0x1f] ^ pkt[i + 1];
}


extern "C" { void DbgLogPublic(const char*); }

int Game_SceneUpdate(void)
{
    // ── ONE-TIME INIT ─────────────────────────────────────────────────────────
    if (DAT_083a7c49 == '\0') {
        DAT_083a7c49 = 1;
        World  = 0xffffffff;
        DAT_05826cb0  = 0;

        // NOTE: the canonical IDA init at 0x0051F900 lines 220-289 does NOT
        // call Net_Connect here. The socket is opened only when the user
        // picks a server in the ServerSelect UI (state 0 → 1 transition),
        // which is how the login scene gets its connecting-progressbar.
        // Calling CreateSocket at scene-init bypasses ServerSelect entirely
        // (the server responds with JoinServer before the user ever sees the
        // list), so we leave it commented out. See IDA line 238+ — only the
        // state/UI globals are initialized here.
        // CreateSocket(PTR_s_connect_muonline_co_kr_005615b8,
        //              (unsigned)(0 | (g_ServerPort)));
        DAT_083a7c48 = 1;
        DAT_083a7c40 = 0;

        // ── Flujo ConnectServer ──────────────────────────────────
        // Si ServerConfig tiene ConnectServer (ConnectServerPort != 0), conectamos YA
        // al ConnectServer para recibir la lista real + el load. A diferencia
        // del GameServer, el ConnectServer NO responde JoinServer al conectar:
        // espera nuestro request C1 04 F4 02 (enviado en FD_CONNECT) y responde
        // con F4/04 (nombres) + F4/02 (load). Al elegir server mandamos F4/03 y
        // el redirect nos lleva al GameServer. Si NO hay línea 2, queda el flujo
        // directo clásico (conectar al elegir server).
        if (gNetwork.HasConnectServer()) {
            gNetwork.BeginConnectServerSession();
            // CRÍTICO: el ConnectServer usa paquetes PLANOS (sin el byte-XOR
            // MuEmu del GameServer). Desactivamos la capa MuEmu mientras hablamos
            // con el CS; Recv_Redirect la reactiva al saltar al GameServer.
            MuEmu::SetActive(false);
            extern void CreateSocket(const char *server, unsigned int port);
            CreateSocket(PTR_s_connect_muonline_co_kr_005615b8, (unsigned)g_ServerPort);
        }

        // Load login-scene resources (sounds, textures)
        Scene_LoadAccountResources(); // OpenLogoSceneData (IDA)

        // Clear 8 entity slots at base entity array + 0x168 (stride 0x394)
        {
            int* p = (int*)(DAT_07abf5d0 + 0x168);
            for (int i = 7; i >= 0; i--, p += 0xe5)
                *p = 0;
        }

        // Spawn background world objects.
        // Valores de Ghidra @ 0x0051F900 (MoveLogInScene) líneas 100-260. `rot` se
        // resetea antes del mu banner (0xA2) y del sky4 (0xA3).
        {
            // ── VALORES CANÓNICOS ─────────────────────────────
            // Restaurados desde Ghidra/IDA @ 0x0051F900 líneas 150-235.
            // CreateObject aplica scale override vía
            // byte_4FFAA4[type-60] cuando SceneFlag==2||4:
            //   type 60  (ship)    → scale 0.8
            //   type 160 (sky)     → scale 0.0438
            //   type 161 (wave)    → scale 0.8
            //   type 162 (banner)  → scale 0.6
            //   type 163 (sun)     → scale 3.0
            // No hacer hacks de scale ni de rot aquí — si algo no renderiza,
            // el bug está upstream en el pipeline (no en los valores).
            float pos[3], rot[3], scale = 1.0f;

            for (int i = 0; i < 9; ++i) g_LoginSceneObjects[i] = 0;

            // Sky 0xA0 @ (0, -520, 0) — rot (0,0,0)
            rot[0] = 0.0f; rot[1] = 0.0f; rot[2] = 0.0f;
            pos[0] = 0.0f; pos[1] = -520.0f; pos[2] = 0.0f;
            g_LoginSceneObjects[0] = CreateObject(0xa0, pos, rot, scale);

            // Ships + Waves: rot (0, 0, 180) canónico.
            float rotShip[3] = { 0.0f, 0.0f, 180.0f };

            // Ship 1 + wave 1 @ (-700, 700, 0)
            pos[0] = -700.0f; pos[1] = 700.0f; pos[2] = 0.0f;
            g_LoginSceneObjects[1] = CreateObject(0x3c, pos, rotShip, scale);
            g_LoginSceneObjects[2] = CreateObject(0xa1, pos, rotShip, scale);

            // Ship 2 + wave 2 @ (400, 400, 0)
            pos[0] = 400.0f; pos[1] = 400.0f; pos[2] = 0.0f;
            g_LoginSceneObjects[3] = CreateObject(0x3c, pos, rotShip, scale);
            g_LoginSceneObjects[4] = CreateObject(0xa1, pos, rotShip, scale);

            // Ship 3 + wave 3 @ (-200, -400, 0)
            pos[0] = -200.0f; pos[1] = -400.0f; pos[2] = 0.0f;
            g_LoginSceneObjects[5] = CreateObject(0x3c, pos, rotShip, scale);
            g_LoginSceneObjects[6] = CreateObject(0xa1, pos, rotShip, scale);

            // Mu banner 0xA2 @ (0, -600, 480)
            rot[0] = 0.0f; rot[1] = 0.0f; rot[2] = 0.0f;
            pos[0] = 0.0f; pos[1] = -600.0f; pos[2] = 480.0f;
            g_LoginSceneObjects[7] = CreateObject(0xa2, pos, rot, scale);

            // Sun 0xA3 @ (-110, 1600, 50)
            pos[0] = -110.0f; pos[1] = 1600.0f; pos[2] = 50.0f;
            g_LoginSceneObjects[8] = CreateObject(0xa3, pos, rot, scale);
        }

        // Spawn 3 background characters (entity id, class, 0, 0, 0)
        unsigned char* loginPreview1 = CreateHero(1, 1, 0, 0.0f, 0.0f, 0.0f);
        LoginScene_ClearPreviewEquipmentMeta(loginPreview1);
        {
            int base = DAT_07abf5d0;
            // Entity 0 (slot 0) — scale + position
            *(float*)(base + 0x3a0) = 0.6f;
            *(int*)  (base + 0x3b0) = 0;
            *(int*)  (base + 0x3b4) = 0;
            *(float*)(base + 0x3b8) = 180.0f;
            *(float*)(base + 0x3a4) = -200.0f;
            *(float*)(base + 0x3a8) = -750.0f;
            *(float*)(base + 0x3ac) = 250.0f;
        }

        unsigned char* loginPreview2 = CreateHero(2, 0, 0, 0.0f, 0.0f, 0.0f);
        LoginScene_ClearPreviewEquipmentMeta(loginPreview2);
        {
            int base = DAT_07abf5d0;
            // Entity 2 (elf).
            // Ghidra @ 0x0051F900 línea 239: CharactersClient[2].Position[1] = -770.0
            *(float*)(base + 0x734) = 0.6f;
            *(int*)  (base + 0x744) = 0;
            *(int*)  (base + 0x748) = 0;
            *(float*)(base + 0x74c) = 180.0f;
            *(float*)(base + 0x738) = -240.0f;
            *(float*)(base + 0x73c) = -770.0f;
            *(float*)(base + 0x740) = 250.0f;
        }

        unsigned char* loginPreview3 = CreateHero(3, 2, 0, 0.0f, 0.0f, 0.0f);
        LoginScene_ClearPreviewEquipmentMeta(loginPreview3);
        {
            int base  = DAT_07abf5d0;
            int slot3 = base + 0xabc;  // entity 3 base
            *(float*)(base + 0xac8) = 0.6f;
            *(int*)  (base + 0xad8) = 0;
            *(int*)  (base + 0xadc) = 0;
            *(float*)(base + 0xae0) = 180.0f;
            *(float*)(base + 0xacc) = 400.0f;
            *(float*)(base + 0xad0) = 50.0f;
            *(float*)(base + 0xad4) = 250.0f;
            CreateBug(0x330, (void*)(base + 0xacc), (void*)slot3, 0);
        }

        // Initialize state variables
        DAT_083a7c14 = 0;       // state = ServerSelect
        DAT_083a7c18 = 0;
        DAT_005616a4 = 0x1e0;   // dialog Y start
        DAT_005616a8 = (int)0xffffff38;
        ServerSelectHi = -1;      // no server selected
        ServerSelectLo = -1;      // no channel selected
        DAT_083a7c44 = 0;
        GuildInputEnable = 0;
        DAT_07e11d71 = 0;
        GoldInputEnable = 0;
        DAT_00559c84 = 1;
        ClearInput(1);        // CharSelect_Init

        // Copy version string → username buffer
        {
            int vlen = (int)strlen((char*)lpData_055c9ba0);
            memcpy(DAT_07db8710, lpData_055c9ba0, vlen + 1);
            // InputTextMax is a per-slot array. IDA canonical init sets BOTH
            // slots so password accepts input too; only setting slot 0 used to
            // silently reject every keystroke in the password field.
            ((int*)&InputTextMax)[0] = 10;  // InputTextMax[0] = username max
            ((int*)&InputTextMax)[1] = 10;  // InputTextMax[1] = password max
            InputNumber = 2;
            DAT_07e113d9 = 1;
            *(DWORD*)DAT_07d780a8 = vlen;
            DAT_07e11d78 = (vlen != 0) ? 1 : 0;
        }

        // Populate server list UI (120 × Widget_Draw)
        for (int i = 0x78; i > 0; i--)
            UIChatLogWindow_AddText((const char*)&DAT_083a7c70, (const char*)&DAT_083a7c6c, 0);

        // En el binario original el check es `if (Hero == NULL)` donde Hero es
        // la base del array de personajes (constante, nunca NULL). Ghidra lo
        // tradujo como DAT_07abf5d8 == 0 (byte en offset 0x8 de entity 0) que
        // SÍ es 0 tras el spawn → escribiría a 0x10/0x14/0x18 (null deref).
        // Branch muerto en el exe real; lo deshabilitamos.
        #if 0
        if (DAT_07abf5d8 == 0) {
            _DAT_00000010 = 0;
            _DAT_00000014 = 0;
            _DAT_00000018 = 0;
        }
        #endif

        // Camera initial position from server-slot table
        // DAT_005615ec/f0/f4/f8/fc/600 son aliases DWORD& sobre el storage float
        // CameraWalk_005615ec[]: se leen vía float* para no convertir int→float.
        _DAT_083a42d4 = 0; _DAT_083a42d8 = 0; _DAT_083a42dc = 0;
        DAT_083a42b8 = 0;
        DAT_083a7ad0  = *((float*)&DAT_005615f8 + DAT_083a7c3c * 6);
        DAT_083a7ad4  = *((float*)&DAT_005615fc + DAT_083a7c3c * 6);
        DAT_083a7ad8  = *((float*)&DAT_00561600 + DAT_083a7c3c * 6);
        DAT_083a432c  = (&DAT_005615ec)[DAT_083a7c3c * 6];   // DWORD&=DWORD ok
        DAT_083a4330  = (&DAT_005615f0)[DAT_083a7c3c * 6];   // DWORD&=DWORD ok
        _DAT_083a4334 = *((float*)&DAT_005615f4 + DAT_083a7c3c * 6);
        DAT_083a42bc = 0;
        DAT_083a42c0 = 0;
        DAT_005615e8 = -1;

        // Los defaults de camara (FOV/Near/Far) salen de globals.cpp (valores
        // del .data del PE: 55.0 / 20.0 / 2000.0) y MoveCamera arma pose+angulo
        // cada frame desde la tabla contigua CameraWalk_005615ec[].


        CErrorReport_Write(&DAT_055c9bf0, "> Login Scene init success");
    }

    // ── PER-FRAME UPDATES ─────────────────────────────────────────────────────
    Object_MoveUpdate();
    // IDA Game_SceneUpdate (0x51F900) llama MoveParticles() cada frame: tickea y
    // expira las partículas. Nuestro Particle_Update() es en realidad
    // Trail_RenderAll (0x46C3E0, mal nombrado) y no decrementa el lifetime.
    MoveParticles();
    Character_UpdateAll();

    // ── Per-character animation tick (login scene) ────────────────────────────
    // En el binario original MoveCharactersClient (0x00455010) corre cada frame
    // y llama MoveCharacter → CharacterAnimation → sub_440AA0 para avanzar
    // entity[+0x108] (frame). Sin esta llamada el ship + el barbarian del login
    // se quedan congelados en frame 0 (cambian de pose abruptamente cuando se
    // setea entity[+0x105], pero no animan).
    {
        for (int s = 0; s < 8; ++s) {
            int e = (int)(uintptr_t)DAT_07abf5d0 + s * 0x394;
            if (*(char*)e != '\0') {
                MoveCharacterVisual(e);            // model.action ← entity.action
                CharacterAnimation(e, e);   // advance entity[+0x108]
            }
        }
    }

    Effect_UpdateAll();
    Login_CameraUpdate();
    // MoveCamera @ 0x0051E4E0 — login scene camera fly-through. Reads
    // CameraWalk_005615ec[] and writes CameraAngle/CameraPosition each frame.
    // Missing this call left the camera at (0,0,0) with angles (0,0,0).
    MoveCamera();
    LoginScene_ApplySafeObjectAnim();

    // Random preview-character action changes (IDA 0x0051F900):
    // the login scene nudges the three background characters into alternate
    // idle/gesture actions with a 1/128 chance each frame.
    {
        int e3 = DAT_07abf5d0 + 0xabc;  // entity 3
        if ((_rand() & 0x7f) == 0) SetAction(e3, 113);
        if ((_rand() & 0x7f) == 0) SetAction(e3, 99);

        int e2 = DAT_07abf5d0 + 0x728;  // entity 2
        if ((_rand() & 0x7f) == 0) SetAction(e2, 115);
        if ((_rand() & 0x7f) == 0) SetAction(e2, 111);

        int e1 = DAT_07abf5d0 + 0x394;  // entity 1
        if ((_rand() & 0x7f) == 0) SetAction(e1, 101);
        if ((_rand() & 0x7f) == 0) SetAction(e1, 111);
    }

    // Optional UI scene load if DAT_083a7af4 == 1 && no server selected
    if (DAT_083a7af4 == 1 && DAT_005615e8 == 0)
        PlayBuffer(4, 0, 0);

    // ── STATE 0: SERVER SELECT ────────────────────────────────────────────────
    if (DAT_083a7c14 == 0) {
        CServerSelWin_UpdateWhileActive();  // Scene_Login_ServerSelect
    }

    // ── STATE 1 / 12: CONNECTING / ERROR ─────────────────────────────────────
    if (DAT_083a7c14 == 1 || DAT_083a7c14 == 0xc) {
        DWORD now = GetTickCount();
        DWORD elapsed = now - DAT_083a7ac8;

        if (elapsed < 0x3a99) {
            // Progress bar
            DWORD clamped = (elapsed < 15000) ? elapsed : 15000;

            // Send keep-alive every 3000 ms
            if (DAT_083a4320 < (int)(clamped / 3000)) {
                DAT_083a4320++;
                if (DAT_083a7c14 == 1) {
                    // Build 6-byte keep-alive: [0xC1][6][0x05][seq][0][0]
                    BYTE pkt[6];
                    pkt[0] = 0xC1; pkt[1] = 6; pkt[2] = 0x05;
                    pkt[3] = (BYTE)DAT_083a4328;
                    pkt[4] = 0; pkt[5] = 0;
                    gNetwork.SendRaw((char*)pkt, 6);
                    // IDA 0x0051F900 L445-446: these are GlobalText[470]/[471],
                    // not independent empty buffers.
                    UIChatLogWindow_AddText((const char*)&DAT_083a7c74, GlobalText[470], 1);
                    UIChatLogWindow_AddText((const char*)&DAT_083a7c78, GlobalText[471], 1);
                }
            }
        } else if (DAT_083a7c14 == 1) {
            // Timeout: reset to ServerSelect
            DAT_083a7c14 = 0;
            DAT_083a7c44 = 0;
        } else {
            // state == 12: restart timestamp
            if (DAT_083a7c14 == 0xc)
                DAT_083a7ac8 = GetTickCount();
        }

        // Click "back" button (0x11d-0x162, Y: 0x106-0x119)
        if (DAT_083a427c >= 0x11d && DAT_083a427c <= 0x162 &&
            DAT_083a4278 >= 0x106 && DAT_083a4278 <= 0x119 &&
            IsClickPushed())
        {
            DAT_083a4124 = '\0';
            if (DAT_083a7c14 == 1) {
                DAT_083a7c14 = 0;
                DAT_083a7c44 = 0;
            } else if (DAT_083a7c14 == 0xc) {
                if (DAT_083a4320 < 4) {
                    // Retry: disconnect and reconnect
                    goto LAB_00520fd8;
                }
                DAT_083a7c14 = 0;
                DAT_083a7c44 = 0;
                Net_Disconnect(((int)(uintptr_t)SocketClient));
                DAT_05826cb0 = 0;
                CreateSocket(PTR_s_connect_muonline_co_kr_005615b8,
                             (unsigned)g_ServerPort);
            }
        }
        goto LAB_00520fd8;
    }

    // ── STATE 2-3: CREDENTIAL INPUT / AWAIT LOGIN RESPONSE ───────────────────
    if (DAT_083a7c14 >= 2 && DAT_083a7c14 <= 3) {
        int dialogY = DAT_005616a4;

        // Account list click: entries at Y = dialogY+0x32, stride 0x14
        {
            int entryY = dialogY + 0x32;
            for (int i = 0; i < InputNumber; i++, entryY += 0x14) {
                if (DAT_083a427c > 0x124 && DAT_083a427c < 0x1a1 &&
                    DAT_083a4278 >= entryY && DAT_083a4278 < entryY + 0x14 &&
                    IsClickPushed())
                {
                    DAT_083a4124 = '\0';
                    DAT_07e11d78 = (unsigned)i;
                    DAT_083a7c24 = DAT_083a7c28;
                    DAT_083a7c28 = 0;
                }
            }
        }

        // Login trigger: Enter key (byte_55CA038) OR click on Login button.
        // Canonical IDA @0x0051F900 gates the entire credential-check +
        // login-send path on this compound trigger — without it the block
        // below fires every frame as soon as JoinServer sets code=2, which
        // spams the server with an empty-creds login and gets the connection
        // dropped.
        bool loginTrigger = (DAT_055ca038 != '\0');
        if (loginTrigger) DAT_055ca038 = '\0';

        // Login button rect: MouseX in [0x143,0x188], Y in [dialogY+0x5f,+0x72].
        // IDA: if login-click THEN consume click + fall-through; else if (!v41)
        //      goto LABEL_194 (skip login logic).
        bool loginClick = (DAT_083a427c >= 0x143 && DAT_083a427c <= 0x188 &&
                           DAT_083a4278 >= dialogY + 0x5f &&
                           DAT_083a4278 <= dialogY + 0x72 &&
                           IsClickPushed());
        if (loginClick) {
            DAT_083a4124 = '\0';
            loginTrigger = true;
        }

        if (loginTrigger && DAT_05826cb0 != 0) {
            // Reset slot selection
            DAT_083a7c24 = DAT_083a7c28;
            DAT_083a7c28 = 0;
            PlayBuffer(0x19, 0, 0);

            // Check username empty
            int ulen = (int)strlen((char*)DAT_07db8710);
            if (ulen == 0) {
                // No username: set slot error
                if (DAT_083a7c24 == 0) { DAT_083a7c24 = 100; }
                else                   { DAT_083a7c28 = 100; }
                DAT_07e11d78 = 0;
                DAT_005616a0 = 1;
            } else {
                int plen = (int)strlen((char*)DAT_07db8810);
                if (plen == 0) {
                    if (DAT_083a7c24 == 0) { DAT_083a7c24 = 0x65; }
                    else                   { DAT_083a7c28 = 0x65; }
                    DAT_07e11d78 = 1;
                    DAT_005616a0 = 1;
                }
            }

        // Login packet send — ONLY when user actually pressed login.
        if (DAT_083a7c24 == 0) {
            int code = DAT_05826cb0;
            bool validCode = (code == 2 || code == 0xb || code == 0xc || code == 0xd ||
                              (code > 0x14 && code < 0x22));
            if (validCode) {
                CErrorReport_Write(&DAT_055c9bf0, "> Login Request");
                DAT_083a7ac8 = GetTickCount();
                DAT_083a4320 = 0;
                CErrorReport_Write(&DAT_055c9bf0, "> Try to Login ... %s",
                             (char*)DAT_07db8710);

                // Copy username to packet staging buffer
                int ulen = (int)strlen((char*)DAT_07db8710) + 1;
                memcpy(LogInID, DAT_07db8710, ulen);
                LogIn = 1;
                DAT_05826cb0 = 0x13;

                // Build login packet [0xC1][0xF1] + user(10) + pass(10) + tick(4) + 5 obf + 16 raw
                BYTE user[10] = {0}, pass[10] = {0};
                memcpy(user, DAT_07db8710, min(ulen - 1, 10));
                memcpy(pass, DAT_07db8810, min((int)strlen((char*)DAT_07db8810), 10));
                // NOTE: companion client (Mu-linux-97K/Source/Client/Main/Reconnect.cpp:222-224)
                // does NOT pre-process account/password — they're fed RAW into
                // PacketArgumentEncrypt (the 3-byte XOR below).  We used to call
                // Buffer_XorKey3 (Buffer_XorKey3) here, which mangled the strings
                // (server saw 'emap...' for input "emanuel...") and the server
                // silently dropped the packet.  Removed.

                // Packet layout matches companion client's PMSG_CONNECT_ACCOUNT_SEND
                // (Mu-linux-97K/Source/Client/Main/Protocol.h:370,
                //  Reconnect.cpp:218 build sequence):
                //   pkt[0]=0xC1 placeholder (send-wrapper replaces with 0xC3),
                //   pkt[1]=size placeholder (stomped with serial in gNetwork.Send),
                //   pkt[2]=0xF1 (head — plaintext),
                //   pkt[3]=0x01 (subh — plaintext, was wrong before → server saw 0x79),
                //   pkt[4..13]=account (XOR'd with {0xFC,0xCF,0xAB} via
                //                       PacketArgumentEncrypt — 3-byte rotating key),
                //   pkt[14..23]=password (same 3-byte XOR),
                //   pkt[24..27]=TickCount (plaintext, little-endian DWORD),
                //   pkt[28..32]=ClientVersion[5] with (v[i] - i - 1) obfuscation,
                //   pkt[33..48]=ClientSerial[16] raw copy.
                // Total plaintext = 49 bytes. CSM expands 48 payload bytes → 66 cipher.
                // Previous implementation XOR-chained every byte starting at pkt[3],
                // which corrupted the subh byte (server saw 0x79 instead of 0x01) and
                // caused LoginResult code=0x05 (blacklist path: empty HardwareID).
                static const BYTE kArgXor[3] = { 0xFC, 0xCF, 0xAB };
                BYTE pkt[64] = {0};
                pkt[0] = 0xC1;
                pkt[1] = 0x01;     // placeholder — serial stomp overwrites
                pkt[2] = 0xF1;     // head (plaintext)
                pkt[3] = 0x01;     // subh (plaintext) — CGConnectAccountRecv dispatch
                int  pos = 4;

                // account[10] — XOR with 3-byte rotating mask (PacketArgumentEncrypt).
                for (int i = 0; i < 10; i++) {
                    pkt[pos + i] = (BYTE)(user[i] ^ kArgXor[i % 3]);
                }
                pos += 10;

                // password[10] — same XOR.
                for (int i = 0; i < 10; i++) {
                    pkt[pos + i] = (BYTE)(pass[i] ^ kArgXor[i % 3]);
                }
                pos += 10;

                // TickCount (4 bytes LE) — plaintext.
                DWORD tick = GetTickCount();
                memcpy(pkt + pos, &tick, 4);
                pos += 4;

                // ClientVersion[5] — obfuscated as (v[i] - i - 1), plaintext after.
                for (int i = 0; i < 5; i++) {
                    pkt[pos + i] = (BYTE)(Version[i] - (char)i - 1);
                }
                pos += 5;

                // ClientSerial[16] — raw copy, plaintext.
                memcpy(pkt + pos, Serial, 16);
                pos += 16;
                // pkt[1] := serial is set later by gNetwork.Send

                // ── LoginKey chain XOR ──────────────────────────────────────
                // Se aplica en gNetwork.Send / gNetwork.SendLarge, para TODOS los
                // paquetes salientes (F1/05 HWID incluido), igual que el companion
                // (Protocol.cpp ExtractPacket).
                int totalLen = pos;

                // Compute CRC and build final send buffer
                int crc = CSimpleModulus_Encode(0, pkt + 1, totalLen - 1);
                if (crc < 0x100) {
                    gNetwork.Send(pkt, totalLen);
                } else {
                    gNetwork.SendLarge(pkt, totalLen);
                }

                // Diagnóstico: log de cada intento de login (longitudes + serial actual,
                // nunca la password), para correlacionar con "F1/01 LOGIN-RESULT code=..."
                // en Net_Process.
                {
                    static int s_loginAttempt = 0;
                    s_loginAttempt++;
                }

                // Post-login status dialog.  In the original binary (IDA
                // 0x0051F900 L1074-1075) these were GlobalText[472] / [473] —
                // loaded from Data\Local\Text.bmd.  Ghidra split them into the
                // independent empty buffers DAT_07d4c644 / DAT_07d4c770, which
                // is why the user saw "cartel sin texto".  Read the real
                // localized strings directly from the GlobalText[] pool.
                UIChatLogWindow_AddText((const char*)&DAT_083a7c7c, GlobalText[472], 1);
                UIChatLogWindow_AddText((const char*)&DAT_083a7c80, GlobalText[473], 1);
                DAT_083a7c14 = 3;
                DAT_083a7c18 = 0xc;
            } else {
                DAT_083a7c18 = 3;
            }
        } else {
            DAT_083a7c14 = 3;
            DAT_083a7c18 = 2;
        }
        PlayBuffer(0x1b, 0, 0);
        } // end if (loginTrigger && DAT_05826cb0 != 0)
    }

    // Login/Start button click (0x11c-0x163, Y in [dialogY+0xb4, dialogY+0xc8])
    {
        int dialogY = DAT_005616a4;
        // Diagnostic: dump every frame where the click flag is set inside the
        // Exit X band, so we can see whether a click is hitting the button or
        // getting eaten by something earlier.
        if (IsClickPushed() &&
            DAT_083a427c > 0x100 && DAT_083a427c < 0x180)
        if (DAT_083a427c > 0x11c && DAT_083a427c < 0x163 &&
            DAT_083a4278 >= dialogY + 0xb4 && DAT_083a4278 < dialogY + 200 &&
            IsClickPushed())
        {
            CErrorReport_Write(&DAT_055c9bf0, "> Login Scene - Exit");
            DAT_083a4124 = '\0';
            DAT_083a7c24 = DAT_083a7c28;
            DAT_083a7c28 = 0;
            PlayBuffer(0x19, 0, 0);
            DAT_083a7c14 = 3;
            DAT_083a7c18 = 0x1d;
            DAT_083a7c1c = 1;
            DAT_083a7c20 = 0x32;
            Net_Disconnect(((int)(uintptr_t)SocketClient));
            PlayBuffer(0x1b, 0, 0);
        }
    }

    // ── STATE ANIMATION + SERVER RESPONSE SWITCH ─────────────────────────────
LAB_00520fd8:
    {
        int  state   = DAT_083a7c14;
        int  respCode = DAT_05826cb0;
        int  slotA   = DAT_083a7c24;
        int  slotB   = DAT_083a7c28;
        int  iVar11  = 0;

        switch (state - 2) {
        case -2: // state 0: ServerSelect bar eases to Y=0x108
            iVar11 = -(0x108 - DAT_005616a4 >> 0x1f);
            DAT_005616a4 += (0x108 - DAT_005616a4) / 3;
            break;

        case 1:  // state 3
        case 3:  // state 5
        case 5:  // state 7
        case 7:  // state 9
        case -1: // state 1
            // Ease dialog up to Y=0x1c4
            iVar11 = (0x1c4 - DAT_005616a4) / 2;
            DAT_005616a4 += iVar11;
            if (DAT_005616a4 < 0x1c3) {
                return iVar11;
            }

            // ── SERVER RESPONSE HANDLER ──────────────────────────────────────
            switch (respCode) {
            case 0x0b: // Login OK
                DAT_083a7c14 = 2;
                DAT_07e11d78 = 0;
                InputNumber = 2;
                if (respCode == 0) {
                    DAT_083a7c28 = 0; DAT_05826cb0 = 2; DAT_083a7c24 = slotB;
                } else if (slotA == 0) {
                    DAT_05826cb0 = 2; DAT_083a7c24 = respCode;
                } else {
                    DAT_05826cb0 = 2; DAT_083a7c28 = respCode;
                }
                break;

            case 0x0c:
            case 0x0d:
                if (respCode == 0x0d) {
                    DAT_07e11d78 = 6;
                } else {
                    DAT_07e11d78 = 0;
                    if (respCode == 0) {
                        DAT_083a7c28 = 0;
                        goto state_fail_common;
                    }
                }
                {
                    int r = respCode;
                    if (slotA != 0) { DAT_083a7c28 = respCode; r = slotA; }
                    DAT_083a7c24 = r;
                }
state_fail_common:
                DAT_05826cb0 = 2;
                DAT_083a7c14 = 4;
                break;

            case 0x14: // Character list ready
                CErrorReport_Write(&DAT_055c9bf0, "> Request Character list");
                SceneFlag = 4;
                DAT_05826cb0 = 0x32;
                {
                    // Send 0xC1/0xF3/0x00 char-list request (4 bytes).
                    //
                    // El chain XOR también se aplica a los C1: el companion
                    // (CPacketManager::ExtractPacket) llama XorData con start=3, end=size sobre el
                    // buffer completo antes de decidir el wrap C3. Fórmula:
                    // `pkt[i] ^= pkt[i-1] ^ key[i]`; con totalLen=4 es una sola iteración,
                    // `pkt[3] ^= pkt[2] ^ key[3]`.
                    BYTE pkt[4];
                    pkt[0] = 0xC1; pkt[1] = 4; pkt[2] = 0xF3;
                    pkt[3] = 0;
                    // Chain XOR (formula universal: pkt[i] ^= pkt[i-1] ^ key[i])
                    pkt[3] ^= pkt[2] ^ CNetwork::XorKey[3 & 0x1f];
                    gNetwork.SendRaw((char*)pkt, 4);
                }
                Resource_LoadOrFatal((CHAR*)LogInID);
                Scene_UnloadAccountResources(); // ReleaseLogoSceneData (IDA)
                return 0;

            // Login error codes 0x13-0x24: show error, back to CredentialInput
            case 0x13: case 0x15: case 0x16: case 0x17: case 0x18:
            case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e:
            case 0x1f: case 0x20: case 0x21: case 0x22: case 0x23: case 0x24:
                if (respCode == 0x15) {
                    DAT_07e11d78 = 1;
                } else {
                    DAT_07e11d78 = 0;
                    if (respCode == 0) {
                        DAT_083a7c14 = 2;
                        DAT_083a7c28 = 0; DAT_05826cb0 = 2; DAT_083a7c24 = slotB;
                        break;
                    }
                }
                if (slotA == 0) {
                    DAT_083a7c14 = 2; DAT_05826cb0 = 2; DAT_083a7c24 = respCode;
                } else {
                    DAT_083a7c14 = 2; DAT_05826cb0 = 2; DAT_083a7c28 = respCode;
                }
                break;

            case 0x3c: // 0x3e
                DAT_05826cb0 = 2;
                DAT_083a7c14 = 8;
                PlayBuffer(0x1b, 0, 0);
                ClearInput(1);
                InputNumber = 2;
                DAT_00559c90 = 1;
                InputTextMax = 0x1e;
                _DAT_00559c98 = 0xe;
                DAT_07e113d9 = 2;
                return 0;

            case 0x3d: // 0x3f
                if (respCode != 0) goto state_alt_error;
                DAT_083a7c14 = 2; DAT_083a7c28 = 0; DAT_05826cb0 = 2; DAT_083a7c24 = slotB;
                break;

            // Success variants 0x40-0x47
            case 0x3e: case 0x3f: case 0x40: case 0x41:
            case 0x42: case 0x43: case 0x44: case 0x45:
                if (respCode == 0) {
                    DAT_083a7c28 = 0;
                } else {
                    int r = respCode;
                    if (slotA != 0) { DAT_083a7c28 = respCode; r = slotA; }
                    DAT_083a7c24 = r;
                }
                DAT_05826cb0 = 2;
                DAT_083a7c14 = 2;
                PlayBuffer(0x1b, 0, 0);
                ClearInput(1);
                {
                    int vlen = (int)strlen((char*)lpData_055c9ba0);
                    memcpy(DAT_07db8710, lpData_055c9ba0, vlen + 1);
                    DAT_07e113d9 = 1;
                    InputNumber = 2;
                    *(DWORD*)DAT_07d780a8 = vlen;
                    InputTextMax = 10;
                    _DAT_00559c98 = 10;
                    DAT_07e11d78 = (vlen != 0) ? 1 : 0;
                }
                return 10;

            default:
state_alt_error:
                if (DAT_005616a0 == '\0' &&
                    DAT_083a7c18 != 4 && DAT_083a7c18 != 10 &&
                    DAT_083a7c18 != 6 && DAT_083a7c18 != 1 &&
                    DAT_083a7c18 != 0xc)
                {
                    return DAT_083a7c18;
                }
                DAT_005616a0 = '\0';
                DAT_083a7c14 = DAT_083a7c18;
                break;
            }

            PlayBuffer(0x1b, 0, 0);
            break;

        case 0:  // state 2
        case 2:  // state 4
        case 4:  // state 6
        case 6:  // state 8
            // Ease dialog down to Y=0x93
            iVar11 = (0x93 - DAT_005616a4) * 0x55555556;
            DAT_005616a4 += (0x93 - DAT_005616a4) / 3;
            break;
        }

        return iVar11;
    }
}
