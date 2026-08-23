// Render_Frame.cpp
// Game_RenderTick @ 0x00525A00  (152 lines, 43 basic blocks)
// Render_GameFrame @ 0x004BBFB0
//
// ── GAME_RENDERTICK ───────────────────────────────────────────────────────────
//
//   Called from Game_MainLoop once per frame after all game-logic updates.
//   Drives the full OpenGL render pipeline: frustum setup → terrain → entities
//   → particles → skills → 2D HUD overlay → SwapBuffers.
//
// ── GLOBALS ──────────────────────────────────────────────────────────────────
//
//   DAT_083a7c10  — server-ack gate (return immediately if 0)
//   DAT_07e11d1c  — frame render counter (HashTable ref; return if > 0x1e = 30)
//   DAT_083a42ea  — cleared to 0 at frame start
//   DAT_083a42e9  — special/debug view flag (affects viewport + skips some passes)
//   DAT_083a4110  — out: world-space mouse ray direction [3 floats]
//   DAT_083a427c  — mouse X (screen coords)
//   DAT_083a4278  — mouse Y
//   DAT_07abf5d8  — entity array (stride 0x394); slot used for camera
//   DAT_0055283c  — float: viewport→FOV scale factor
//   DAT_0055a7ac  — g_GameSubState (drives clear color)
//   DAT_07e11d30  — depth-test override flag
//   DAT_005590ac  — anti-tamper gate
//   DAT_005615c0  — g_GameState
//   DAT_055c9ff8  — object with flag at +8 (render watchdog?)
//   DAT_083a7c50  — frame counter for render watchdog (reset at 10)
//
// ── ENTRY GUARD ──────────────────────────────────────────────────────────────
//
//   if (DAT_083a7c10 == '\0') return;   // server not yet confirmed
//   HashTable ref-count block on DAT_07e11d1c (anti-tamper / frame limiter)
//   if (iVar5 > 0x1e) return;           // skip frame if counter too high
//
// ── CAMERA POSITION ──────────────────────────────────────────────────────────
//
//   FUN_00524cb0() → Camera_IsSpectator()
//   if '\0' (normal): cam_pos = entity[slot][+0x10..+0x18]  (X,Y,Z floats)
//   else (spectator): cam_pos = entity[slot][+0x170..+0x178]
//
// ── VIEWPORT + FRUSTUM ───────────────────────────────────────────────────────
//
//   iVar5 = FUN_004cb520()              → Screen_GetWidth()
//   FUN_005119b0(0, 0, width, 0x30)     → GL_SetViewport(0, 0, w, 48)
//                                          (top 48-px strip — minimap area?)
//   FUN_004f9050(width * scale, &cam_pos) → Camera_SetupFrustum(fov_w, pos)
//   GL_PopMatrixAll()
//
// ── CLEAR COLOR (g_GameSubState) ─────────────────────────────────────────────
//
//   0  → glClearColor(0.039f, 0.078f, 0.055f, 1) — dark greenish
//   2  → glClearColor(0.000f, 0.000f, 0.039f, 1) — near-black blue
//   10 → glClearColor(0.012f, 0.099f, 0.172f, 1) — dark blue (PvP stage?)
//   *  → glClearColor(0, 0, 0, 1)                — black
//
// ── MAIN VIEWPORT + MOUSE RAY ────────────────────────────────────────────────
//
//   FUN_005119b0(0, 0, width, 0x1b0 or 0x180) → GL_SetViewport (main 3D area)
//     (0x1b0 = 432 full height; 0x180 = 384 when in debug/special mode)
//   FUN_005112f0(mouseX, mouseY, &DAT_083a4110) → Camera_MouseRay(x, y, out)
//
// ── 3D RENDER PASSES (in order) ──────────────────────────────────────────────
//
//   if g_GameSubState != 10: FUN_004f9ac0('\0') → GL_DepthTest(false)
//   FUN_004fd800()    → Terrain_Render()          — tile grid terrain + objects
//   FUN_0046be40()    → Particle_Render()          — particle system draw
//   FUN_00500aa0()    → Entity_UpdatePositions()   — avanza timers/pos de entidades
//                         itera DAT_0839be18, campo -0x5a=active, -0x51=pos float
//                         tipo != 0x10a (cofre): actualiza float pos
//   FUN_0045ab00()    → Entity_RenderAll_3D()      — geometría 3D chars/NPCs
//   if DAT_07e11d30:    FUN_004f9ac0('\x01')       → GL_DepthTest(true)
//   if !debug_view:     FUN_005038e0()             → Entity_Render_Sprites() (billboards)
//   FUN_00502200()    → Entity_PrepareVisibleList() — proyecta world→screen, cull off-screen
//                         itera DAT_083a2e92; llama FUN_004f8ff0 (frustum cull);
//                         si visible: Entity_PrepareRender(); glColor4f(shadow)
//   FUN_00500970()    → NPC_UpdateVisibleList()    — misma lógica para pool DAT_083a1378
//                         tipo 0x186 o sub-state 0x330; rand() para variación
//   FUN_0046cb70()    → SkillEffect_Render()       — proyectiles/efectos de habilidad
//   FUN_00473710()    → ItemDrop_Render()           — items en el suelo
//   FUN_0046bba0()    → MapEffect_RenderAll()      — efectos de mapa (pool DAT_07b11698)
//                         itera stride variable; FUN_004f9590(pos, 400.0) = cull a 400u
//                         tipos < 0x104 con sub-dispatch
//   FUN_00475110()    → Player_Render()             — jugador local
//   FUN_0046c3e0()    → SpellCircle_RenderAll()    — círculos/efectos mágicos en suelo
//                         itera DAT_07c608b4; tipo 0: Frame_UpdateTimer, tipo>2: GL_SetBlend2
//                         tex ID = sub_type + 0x48d
//   FUN_00479790()    → Portal_Render()             — portales / warp pads
//   FUN_00511cf0()    → Render_Flush()              — 1 instrucción, posiblemente glFlush
//   if g_GameSubState == 2 && DAT_07e118e8 not in {3, >=10}:
//     FUN_0046cb70()  → SkillEffect_Render() again  — segunda pasada en in-world state
//   FUN_00479730()    → Sigil_RenderAll()           — círculos de invocación/sigils
//                         itera DAT_07c85894 stride 0x1bc; tipos 0/1/2 → blend distinto
//                         llama FUN_00479670 (Sigil_Draw) + resetea active flag
//   FUN_00478c00()    → Character_Animate()         — actualiza animaciones de personaje
//                         función compleja con muchos floats; ~50 líneas
//   FUN_00479330()    → Sign_RenderAll()            — billboards/letreros en mundo
//                         itera DAT_07c80158 stride 0x70; SetBlendMode(alpha) + EnableAlpha
//                         llama FUN_005120c0(pos, type, pos2, scale, alpha) = Sign_Draw
//   glPopMatrix()
//   FUN_004b0310()    → EntityInfo_Overlay()        — overlay de info de entidades
//                         usa HashTable + SHORT tipo + floats; función ~40 líneas
//   FUN_005123c0()    → GL_SetupOrtho2D()           — establece proyección ortográfica 2D
//                         glPushMatrix + glMatrixMode + glLoadIdentity + glViewport(0,0,w,h)
//   FUN_004cb6f0()    → Target_Render()             — info de entidad seleccionada
//                         DAT_07e11d6e=1; entity lookup via DAT_07abf5d0+idx*0x394
//                         GetTextExtentPointA + texto nombre + HP
//
// ── 2D OVERLAY PASS ──────────────────────────────────────────────────────────
//
//   if !debug_view: Render_GameFrame()  → 2D HUD overlay (HUD_Render.cpp)
//   FUN_0051af50()    → CharInfo_TextRender()       — texto info personaje (stats panel)
//                         múltiples LPCSTR, tagSIZE, texto centrado; ~40+ líneas
//   FUN_004f64d0()    → TeleportUI_Update()         — update/click handler del teleport
//                         FUN_00511600() + DAT_07e11d6e=0; hit-test mouse vs bounds
//                         (_DAT_00552cac, _DAT_00552c24, _DAT_00552c14, _DAT_00552ca8)
//   FUN_0047fce0()    → PlayerName_Render()         — nombres de jugadores con parpadeo
//                         SetBlendMode + SelectObject(bold) + glColor3f(1,1,1)
//                         itera DAT_07db80d8 stride 0x104; parpadeo via DAT_07e11da0 % 10
//   if (DAT_005590ac == 1) || (g_GameState != 5): FUN_00480980()
//                     → SystemText_Render()         — notificaciones del sistema
//                         glColor3f(1,1,1) + SelectObject(normal_font)
//                         CHAR buf[256]; renderiza mensajes del sistema
//   FUN_004c14e0()    → FPS_TimerReset()            — tick por segundo
//                         SelectObject(normal_font); timeGetTime() % 1000:
//                         si > 999ms: resetea DAT_07e016f0 + DAT_07e11dcc=0 (frame counter)
//   FUN_004c3530()    → CondText_Render()           — texto condicional (DAT_07e11d20==1)
//                         FUN_00511600() + DAT_07eaa154=0; operaciones de string
//   FUN_004bffa0()    → AnimUI_Render() × 2        — elemento UI animado (6 frames)
//                         SetBlendMode + glColor3f(1,1,1)
//                         ftol()%6 → UV offset: {1,3,5}→local_4=0.5; {2,3,4}→local_8=0.5
//   FUN_0051e0c0()    → SpecialMap_Render3D()       — pasada 3D para mapas especiales
//                         guard: DAT_083a7c24 ∈ {0x97, 0x99} && DAT_07eaa13c==1
//                         glMatrixMode(GL_PROJECTION) + gluPerspective + LoadCameraMatrix
//   Render watchdog: if (DAT_055c9ff8+8==1) && (++DAT_083a7c50 > 10):
//     DAT_083a7c50 = 0; FUN_0040f670(DAT_055c9ff8) → Watchdog_Reset()
//   FUN_005124b0()    → GL_PopMatrix2()             — glPopMatrix() × 2
//   GL_PopMatrixAll()
//   return 1
//
// ── RENDER_GAMEFRAME @ 0x004BBFB0 ────────────────────────────────────────────
//
//   2D HUD overlay pass — renders in 2D ortho projection over the 3D scene.
//   Has 27 CALL instructions.
//
//   Key calls:
//     004BBFC8: FUN_00511680  → GL_Begin2D / ortho setup
//     004BBfe4: [0x005522C4]  → render callback function pointer
//     004BBFEA: FUN_00511710  → — (counter/timer update)
//     004BC044: FUN_005128C0  → SwapBuffers / Present
//     004BC0A9: FUN_004BC220  → Render_CharInfoPanel()
//     004BC0AE: FUN_004BCA20  → Render_HPBars()
//     004BC0B3: FUN_004BF540  → AntiTamper_A()
//     004BC0C1: FUN_004BEC00  → Render_CharPartyInfo()
//     004BC0C7: FUN_004BE710  → Render_CharNameTags()
//     004BC0CF: FUN_004BF090  → Render_MacroTimer()
//     004BC0D4: FUN_004BF2D0  → Render_MapLoadText()
//     004BC0D9: FUN_004BD090  → Render_FloatingText()
//     004BC0DE: FUN_004F5820  → Render_QuickButtons()
//     004BC0EE: FUN_004BD2B0  → Render_BottomHUD()
//     004BC0F3: FUN_004BE4F0  → Render_ChatBox()
//     004BC0F8: FUN_004BF990  → AntiTamper_B()
//     004BC0FD: FUN_004BD650  → AntiTamper_C() (largest, 659 lines)
//     004BC102: FUN_004BCD20  → AntiTamper_D()
//     004BC107: Render_Scene3D @ 0x004BFDE0
//     004BC10C: FUN_004F6050  → — post-process?
//     004BC111: FUN_004EB070  → — possibly SwapBuffers wrapper
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_00524cb0  → Camera_IsSpectator()          — '\0' si vista normal
//   FUN_004cb520  → Screen_GetWidth()              — ancho viewport en pixels
//   FUN_005119b0  → GL_SetViewport(x,y,w,h)       — glViewport wrapper
//   FUN_004f9050  → Camera_SetupFrustum(fov_w, cam_pos)
//   FUN_004f9ac0  → GL_DepthTest(enable)           — glEnable/Disable(GL_DEPTH_TEST)
//   FUN_004fd800  → Terrain_Render()
//   FUN_0046be40  → Particle_Render()
//   FUN_00500aa0  → Entity_UpdatePositions()       — timer/pos update pool DAT_0839be18
//   FUN_0045ab00  → Entity_RenderAll_3D()
//   FUN_005038e0  → Entity_Render_Sprites()        — billboards 2D-in-3D
//   FUN_00502200  → Entity_PrepareVisibleList()    — frustum cull + PrepareRender
//   FUN_00500970  → NPC_UpdateVisibleList()        — cull NPCs pool DAT_083a1378
//   FUN_0046cb70  → SkillEffect_Render()
//   FUN_00473710  → ItemDrop_Render()
//   FUN_0046bba0  → MapEffect_RenderAll()          — efectos mapa pool DAT_07b11698
//   FUN_00475110  → Player_Render()
//   FUN_0046c3e0  → SpellCircle_RenderAll()        — círculos mágicos pool DAT_07c608b4
//   FUN_00479790  → Portal_Render()
//   FUN_00511cf0  → Render_Flush()                 — 1 instrucción
//   FUN_00479730  → Sigil_RenderAll()              — sigils pool DAT_07c85894 stride 0x1bc
//   FUN_00478c00  → Character_Animate()            — animación de personaje (~50 líneas)
//   FUN_00479330  → Sign_RenderAll()               — letreros pool DAT_07c80158 stride 0x70
//   FUN_004b0310  → EntityInfo_Overlay()           — overlay info entidades (HashTable)
//   FUN_005123c0  → GL_SetupOrtho2D()              — glPushMatrix+glMatrixMode+glViewport
//   FUN_004cb6f0  → Target_Render()               — info entidad seleccionada (entity lookup)
//   FUN_0051af50  → CharInfo_TextRender()          — texto stats panel (~40+ líneas)
//   FUN_004f64d0  → TeleportUI_Update()            — hit-test teleport UI + GL reset
//   FUN_0047fce0  → PlayerName_Render()           — nombres jugadores con parpadeo
//   FUN_00480980  → SystemText_Render()            — notificaciones sistema
//   FUN_004c14e0  → FPS_TimerReset()              — tick/s: timeGetTime %1000 reset
//   FUN_004c3530  → CondText_Render()             — texto condicional DAT_07e11d20
//   FUN_004bffa0  → AnimUI_Render()              — elemento UI 6-frame animado (×2)
//   FUN_0051e0c0  → SpecialMap_Render3D()         — 3D pass mapas 0x97/0x99
//   FUN_0040f670  → Watchdog_Reset(obj)
//   FUN_005124b0  → GL_PopMatrix2()              — glPopMatrix() × 2
//   FUN_005112f0  → Camera_MouseRay(mouseX, mouseY, out_ray[3])
//   FUN_004f8ff0  → Frustum_IsVisible(x, y, z)   — world→screen cull check

#include "stdafx.h"
#include "Render/Render.h"
#include "Render/Camera.h"
#include "Render/Player_Render.h"
extern "C" { void DbgLogPublic(const char* msg); }
// 2026-05-08: backup of DAT_07d78068 — defined here (not in globals.cpp) so
// it lives in a different .obj's BSS, NOT adjacent to DAT_07d78068. The
// unknown writer that sets DAT_07d78068=0x1 also clobbers the next 4 bytes
// to 0 (8-byte write). Putting the backup far away keeps it intact.
extern "C" DWORD g_ItemAttribute_Backup = 0;
// Plus a CANARY before/after to detect if even this gets clobbered.
extern "C" DWORD g_ItemAttr_CanaryBefore = 0xDEADBEEF;
// (g_ItemAttribute_Backup here)
extern "C" DWORD g_ItemAttr_CanaryAfter  = 0xCAFEBABE;

// Forward decls for HUD helpers defined later in this TU.
void Render_CharInfoPanel(void);
void Render_HPBars(void);
void Render_CharPartyInfo(void);
void Render_CharNameTags(void);
void Render_MacroTimer(void);
void Render_MapLoadText(void);
void Render_FloatingText(void);
void Render_QuickButtons(void);
void Render_BottomHUD(void);
void Render_ChatBox(void);
void AntiTamper_HashMaintain_A(void);
void AntiTamper_HashMaintain_B(void);
void AntiTamper_HashMaintain_C(void);
void AntiTamper_HashMaintain_D(void);
void Render_HotbarItems3D(void);
void Render_HudPass_4F6050(void);
void Render_HudPass_4EB070(void);

unsigned int Game_RenderTick(void)
{
    // BUG-FIX 2026-05-04: drain residual GL errors antes del frame para que el
    // diagnostic logging de FUN_005114f0 no spamee con 0x504 stale (de pops
    // sin push del frame previo durante la transición login→in-game).
    while (glGetError() != GL_NO_ERROR) {}

    // BUG-FIX 2026-04-28: anteriormente esto llamaba Render_GameFrame que a su
    // vez llamaba Render_Scene3D al final → recursión infinita cuando porteamos
    // Render_Scene3D para que invocara los UI sub-renderers internamente.
    // Ahora Game_RenderTick → Render_Scene3D directamente (que ES la función
    // RenderMainScene de IDA @ 0x00525A00, hace todo el flujo: BeginOpengl,
    // 3D passes, BeginBitmap, HUD via Render_GameFrame, EndBitmap, EndOpengl).
    Render_Scene3D();
    return 1;
}

// Render_GameFrame — full HUD render pass.  Reconciled 2026-04-29 against
// IDA sub_4BBFB0 of the original mu.exe.  Previously several entries were
// mislabeled "AntiTamper_HashMaintain_*" which they are NOT — IDA shows
// them as plain UI render functions, and several real call sites
// (RenderMainFrameWindow vtable dispatch on dword_55C9FF0, sub_4BFDE0 3D
// hotbar, sub_4F6050 / sub_4EB070) were missing entirely.
//
// IDA verified call order (off + name + size in bytes):
//   if (World==8) { swirling-water bg via RenderBitmapUV }       — TODO
//   glColor3f(1,1,1)
//   sub_4BC220   Render_CharInfoPanel       guild-war/soccer
//   RenderPartyHP                            party HP bars (0x4BCA20, 735 b)
//   RenderNumArrow                           number/arrow overlay (0x4BF540, 1083 b)
//   RenderEquipedHelperLife                  party member helper life (0x4BEC00, 1168 b)
//   RenderBrokenItem                         broken-item warning (0x4BE710, 1255 b)
//   sub_4BF090   Render_MacroTimer
//   sub_4BF2D0   Render_MapLoadText
//   RenderBooleans                           floating-numbers/booleans (0x4BD090, 534 b)
//   sub_4F5820   Render_QuickButtons
//   (*dword_55C9FF0 + 0x10)(self)            chat scroll listbox render
//   RenderMainFrameWindow                    main HUD frame (0x4BD2B0, 919 b)
//   sub_4BE4F0   Render_ChatBox              chat input box
//   RenderExperience                         exp bar/level text (0x4BF990, 1097 b)
//   sub_4BD650                               LARGE HUD pass (3734 b — was misnamed C)
//   sub_4BCD20                               HUD pass D (867 b — was misnamed D)
//   sub_4BFDE0                               3D-projected hotbar items (434 b)
//   sub_4F6050                               unknown HUD pass (973 b)
//   sub_4EB070                               unknown HUD pass (1342 b)
//
// Functions tagged "TODO port" below are scaffold-only; their bodies are
// pending 1:1 IDA ports in dedicated sessions because each pulls in 5-15
// new globals (GuildWarScore[], HeroSoccerTeam, EnableGuildWar, ...) plus
// CRT/Win32 helpers (CreateGuildMark, RenderText_1, RenderBitmap, ...).
void Render_GameFrame(void)
{
    // 2026-05-08: per-frame watchdog. Some unknown writer occasionally
    // clobbers DAT_07d78068 (ItemAttribute table base) to a small value
    // (e.g. 0x00000001), causing tooltip / RenderBrokenItem / RenderObjectScreen
    // to compute attrBase = type*0x40 + 1 → AV when dereferencing.
    // Restore from backup + log to identify the writer pattern.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        bool canaryB_ok = (g_ItemAttr_CanaryBefore == 0xDEADBEEF);
        bool canaryA_ok = (g_ItemAttr_CanaryAfter  == 0xCAFEBABE);
        if (p < 0x100000u || p >= 0x80000000u) {
            // CORRUPTION DETECTED — log the specific bad value first time.
            static DWORD s_lastLog = 0;
            DWORD now = GetTickCount();
            if (now - s_lastLog > 2000) {
                s_lastLog = now;
                char b[256];
                wsprintfA(b, "WD DAT_07d78068 corrupted: was=%p backup=%p canaryB=%X(%s) canaryA=%X(%s)",
                          (void*)(uintptr_t)p,
                          (void*)(uintptr_t)g_ItemAttribute_Backup,
                          g_ItemAttr_CanaryBefore, canaryB_ok ? "ok" : "BAD",
                          g_ItemAttr_CanaryAfter,  canaryA_ok ? "ok" : "BAD");
                DbgLogPublic(b);
            }
            if (g_ItemAttribute_Backup >= 0x100000u &&
                g_ItemAttribute_Backup < 0x80000000u)
            {
                DAT_07d78068 = (int)g_ItemAttribute_Backup;
            }
        }
    }

    if (DAT_0055a7ac == 8) {
        FUN_00511680('\x01');
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        FUN_00511710();
        glColor3f(0.3f, 0.3f, 0.25f);

        const float scrollA = (float)((int)DAT_05826e08 % 100000) * _DAT_00552b88;
        const float scrollB = (float)((int)DAT_05826e08 % 100000) * _DAT_00552500;

        FUN_005125a0(0x494,
                     0.0f, 0.0f, 640.0f, 435.0f,
                     // 2026-08-23 FIX (el "patron" que se ve moverse por toda
                     // la pantalla en Tarkan): esta capa iba con
                     // uWidth/vHeight = 2.0/3.0, o sea la textura de arena se
                     // repetia 2x3 veces y se leia como un mosaico.  IDA
                     // (Render_GameFrame L19) usa 0.3/0.3, con lo que la
                     // textura sale AMPLIADA y da la arena difusa del original.
                     // La capa 2 (0x495) ya estaba bien con 3.0/2.0.
                     // Nota: IDA llama `RenderBitmapUV` (0x5128C0), que toma
                     // (u, v, uWidth, vHeight); aca usamos `RenderBitmap`
                     // (0x5125A0), que toma (u0, v0, u1, v1) — de ahi el
                     // `scroll + ancho` en el tercer par.
                     scrollA, 0.0f, scrollA + 0.30000001f, 0.30000001f,
                     '\x01', '\x01');
        FUN_005125a0(0x495,
                     0.0f, 0.0f, 640.0f, 435.0f,
                     scrollB, 0.0f, scrollB + 3.0f, 2.0f,
                     '\x01', '\x01');
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    Render_CharInfoPanel();         // sub_4BC220 (guild-war/soccer banner)
    Render_HPBars();                // RenderPartyHP @ 0x4BCA20
    AntiTamper_HashMaintain_A();    // RenderNumArrow @ 0x4BF540 (NOT anti-tamper)
    Render_CharPartyInfo();         // RenderEquipedHelperLife @ 0x4BEC00 (was misnamed)
    Render_CharNameTags();          // RenderBrokenItem @ 0x4BE710 (was misnamed)
    Render_MacroTimer();            // sub_4BF090
    Render_MapLoadText();           // sub_4BF2D0
    Render_FloatingText();          // RenderBooleans @ 0x4BD090 (was misnamed)
    Render_QuickButtons();          // sub_4F5820

    // Vtable dispatch — chat-listbox scroll renderer (slot 4 = +0x10).
    // The widget object is constructed in WinMain via ChatListBox_Construct;
    // see UI/ChatListBox.cpp for the per-slot port.  Without this dispatch
    // the chat history scroll UI never paints.
    if (DAT_055c9ff0) {
        DWORD* obj = (DWORD*)DAT_055c9ff0;
        void** vt  = (void**)*obj;
        if (vt) {
            typedef void (__fastcall *FnVoid)(DWORD*);
            ((FnVoid)vt[4])(obj);
        }
    }

    Render_BottomHUD();             // RenderMainFrameWindow @ 0x4BD2B0 (was misnamed)
    Render_ChatBox();               // sub_4BE4F0
    AntiTamper_HashMaintain_B();    // RenderExperience @ 0x4BF990 (NOT anti-tamper)
    AntiTamper_HashMaintain_C();    // sub_4BD650 (1000L HUD pass — NOT anti-tamper)
    AntiTamper_HashMaintain_D();    // sub_4BCD20 (240L HUD pass — NOT anti-tamper)
    Render_HotbarItems3D();         // sub_4BFDE0 (3D hotbar items)
    Render_HudPass_4F6050();        // sub_4F6050 (TODO port)
    Render_HudPass_4EB070();        // sub_4EB070 (TODO port)
}

// Render_CharInfoPanel (sub_4BC220) is implemented in
// src/Render/HUD_Pass2.cpp.  Old comments (claiming "char info panel")
// were misleading — IDA shows it's the guild-war / soccer score banner.

// Render_HPBars (RenderPartyHP @ 0x004BCA20) is implemented in
// src/Render/HUD_Pass1.cpp.  The previous body — kept below in #if 0
// for reference — was a placeholder approximation that wrote a 30×4 red
// quad over every visible entity; the real function only does it for
// party members.
#if 0
void Render_HPBars_OLD(void)
{
    // 2026-04-29 DISABLED: corrompía GL state.
    return;
    if (DAT_005615c0 != 5) return;
    if (DAT_07abf5d8 == nullptr) return;
    if (DAT_07abf5d0 == 0) return;
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int s = 0; s < 400; ++s) {
        BYTE* slot = basePtr + s * 0x394;
        if (slot[0] == 0) continue;                 // inactive
        if (slot == (BYTE*)DAT_07abf5d8) continue;  // skip hero
        if (slot[0x84] == 0) continue;              // not visible

        // World pos at +0x10 (xyz floats). Add height offset so bar floats above head.
        float worldPos[3];
        worldPos[0] = *(float*)(slot + 0x10);
        worldPos[1] = *(float*)(slot + 0x14);
        worldPos[2] = *(float*)(slot + 0x18) + 200.0f;

        int sx = 0, sy = 0;
        FUN_005113f0(worldPos, &sx, &sy);
        if (sx < 0 || sx > 640 || sy < 0 || sy > 480) continue;  // off-screen

        // Draw black shadow + red bar (HP placeholder = 50%)
        // For now: fixed 30×4 bar
        FUN_00511680('\0');     // disable alpha blend (solid)
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        // Black BG quad
        glBegin(GL_QUADS);
        glVertex2f((float)(sx - 16), (float)(sy - 8));
        glVertex2f((float)(sx + 16), (float)(sy - 8));
        glVertex2f((float)(sx + 16), (float)(sy - 4));
        glVertex2f((float)(sx - 16), (float)(sy - 4));
        glEnd();
        // Red foreground (50% wide as placeholder)
        glColor3f(0.9f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f((float)(sx - 15), (float)(sy - 7));
        glVertex2f((float)(sx),      (float)(sy - 7));
        glVertex2f((float)(sx),      (float)(sy - 5));
        glVertex2f((float)(sx - 15), (float)(sy - 5));
        glEnd();
    }
    glColor3f(1.0f, 1.0f, 1.0f);  // restore color
}
#endif

// Render_CharPartyInfo (sub_4BEC00 = RenderEquipedHelperLife) is implemented
// in src/Render/HUD_Pass2.cpp.  IDA name disagrees with the old "party
// stats" guess — it's the equipped pet/helper life bar at top centre.

// Render_CharNameTags (sub_4BE710 = RenderBrokenItem) is now implemented in
// src/Render/HUD_Pass2.cpp.  Old comment ("names text above characters")
// was wrong — IDA shows it's the equipped-item durability warning.

// Render_MacroTimer  (sub_4BF090) is implemented in src/Render/HUD_Pass4.cpp.
// Render_MapLoadText (sub_4BF2D0) is implemented in src/Render/HUD_Pass4.cpp.
// Render_FloatingText (RenderBooleans @ 0x4BD090) iterator is in HUD_Pass1.cpp;
//   the per-entry RenderBoolean (FUN_00480e00) is in HUD_Pass3.cpp.
// Render_QuickButtons (sub_4F5820) is implemented in src/Render/HUD_Pass4.cpp.

// Render_BottomHUD (RenderMainFrameWindow @ 0x004BD2B0) is now implemented
// in src/Render/HUD_Pass1.cpp.  The previous body was a hand-written
// approximation that called RenderBitmap with the wrong UV pattern; the
// real port matches IDA byte-for-byte (mirrored UVs on the right corner,
// the anti-tamper CharacterMachine refresh interleave, etc.).

// Render_ChatBox (sub_4BE4F0) is implemented in src/Render/HUD_Pass2.cpp.

// 2026-04-29: name "AntiTamper_HashMaintain_X" was a misidentification.
// IDA shows these are plain HUD render passes, NOT anti-tamper code.
// Renamed conceptually but symbol kept (callers remain in Render_GameFrame
// only) until the bodies are ported and a final naming pass happens.

// AntiTamper_HashMaintain_A (= RenderNumArrow @ 0x004BF540) is now
// implemented in src/Render/HUD_Pass3.cpp.
//
// AntiTamper_HashMaintain_B (= RenderExperience @ 0x004BF990) is now
// implemented in src/Render/HUD_Pass2.cpp.
//
// AntiTamper_HashMaintain_D (= sub_4BCD20 @ 0x004BCD20) is now
// implemented in src/Render/HUD_Pass3.cpp.
//
// AntiTamper_HashMaintain_C (= sub_4BD650 @ 0x004BD650) is now implemented
// in src/Render/HUD_Pass5.cpp.  All HUD sub-passes are now ported.

// Render_HotbarItems3D (sub_4BFDE0) is implemented in HUD_Pass1.cpp.
// Render_HudPass_4F6050 (sub_4F6050) is implemented in HUD_Pass3.cpp.
// Render_HudPass_4EB070 (sub_4EB070) is implemented in HUD_Pass3.cpp.

// ── Game_RenderTick / RenderMainScene @ 0x00525A00 ──────────────────────────
// Port directo del IDA. Sin esto el InGame state=5 mostraba pantalla negra
// porque Render_Scene3D estaba vacía. Esta función hace TODO el render 3D
// del mundo + UI 2D del HUD.
//
// Helper functions used (ver functions.h):
//   FUN_00524cb0  → MoveMainCamera (returns spectator flag)
//   FUN_004cb520  → GetScreenWidth (returns viewport width)
//   FUN_005119b0  → BeginOpengl (viewport setup + perspective)
//   FUN_004f9050  → Camera_SetupFrustum
//   FUN_005112f0  → CreateScreenVector (mouse ray)
//   FUN_004fd800  → Terrain_Render
//   FUN_00500aa0  → Entity_UpdatePositions / RenderObjects
//   FUN_0045ab00  → Entity_RenderAll_3D / RenderCharactersClient
//   FUN_005038e0  → RenderItems / Entity_Render_Sprites
//   FUN_00500970  → RenderBoids / NPC_UpdateVisibleList
//   FUN_005022f0  → RenderFishs
//   FUN_00500e40  → RenderBugs
//   FUN_004ffeb0  → RenderLeaves (?)
//   FUN_00473710  → RenderJoints
//   FUN_0046bba0  → RenderEffects
//   FUN_0046cb70  → RenderPlanes / SkillEffect_Render
//   FUN_0046c3e0  → RenderBlurs / Particle_Render
//   FUN_00478c00  → RenderParticles (effect pool)
//   FUN_00479730  → RenderSprites
//   FUN_005123c0  → BeginBitmap (ortho2D)
//   FUN_005124b0  → EndBitmap
//   FUN_00511cf0  → BeginSprite (push+identity)
//   FUN_005119b0  → BeginOpengl
//   FUN_00404bc0  → BGM helper
//   FUN_004b0310  → EntityInfo_Overlay / CharPreview
// (Funciones helper ya declaradas en functions.h via stdafx.h)

void Render_Scene3D(void)
{
    // ── DIAG: log every frame entry to track in-game render flow
    {
        static DWORD s_lastR3 = 0;
        DWORD now = GetTickCount();
        if (now - s_lastR3 > 1000) {
            s_lastR3 = now;
            BYTE* hero = (BYTE*)DAT_07abf5d8;
            char b[300];
            float hx = hero ? *(float*)(hero + 0x10) : 0.0f;
            float hy = hero ? *(float*)(hero + 0x14) : 0.0f;
            float hz = hero ? *(float*)(hero + 0x18) : 0.0f;
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "R3D state=%d sub=%d d11d1c=%u hero=%p heroPos=(%.1f,%.1f,%.1f) "
                "CamPos=(%.1f,%.1f,%.1f) CamAng=(%.1f,%.1f,%.1f)",
                (int)DAT_005615c0, (int)DAT_0055a7ac, DAT_07e11d1c,
                hero, hx, hy, hz,
                CameraPosition[0], CameraPosition[1], CameraPosition[2],
                CameraAngle[0], CameraAngle[1], CameraAngle[2]);
            DbgLogPublic(b);
        }
    }

    // ── 1. Anti-tamper LoadingWorld counter (skip frame if > 30) ─────────────
    if (DAT_07e11d1c > 0x1e) return;

    DAT_083a42ea = 0;   // FogEnable = false

    // ── 2. Camera position ────────────────────────────────────────────────────
    FUN_00524cb0();
    float camPos[3];
    if (DAT_07abf5d8) {
        BYTE* hero = (BYTE*)DAT_07abf5d8;
        // Nuestro port actual de MoveMainCamera no reconstruye todavía el
        // bool "spectator/start-position" del binario original y devuelve true
        // siempre en gameplay normal. Usar +0x170 acá rompe el frustum de los
        // scene objects del world (terrain chunks visibles pero object cull=0).
        // Hasta portar ese retorno 1:1, el frustum debe seguir al Hero real.
        camPos[0] = *(float*)(hero + 0x10);
        camPos[1] = *(float*)(hero + 0x14);
        camPos[2] = *(float*)(hero + 0x18);
    } else {
        camPos[0] = camPos[1] = camPos[2] = 0.0f;
    }

    // ── 2b. MoveMainCamera ────────────────────────────────────────────────────
    // Llama al port mínimo de MoveMainCamera (stubs.cpp), que setea
    // CameraAngle/CameraPosition relativos al Hero. Pitch -48.5° (= EarthQuake
    // - 48.5° per IDA), seguimiento 3rd-person.

    // ── 3. Top strip viewport for frustum ─────────────────────────────────────
    int w = FUN_004cb520();
    FUN_005119b0(0, 0, w, 0x30);
    Camera_SetupFrustum((float)w * _DAT_0055283c, camPos);
    FUN_00511bc0();

    // ── 4. Clear color por World ──────────────────────────────────────────────
    int worldId = (int)DAT_0055a7ac;  // g_GameSubState
    float cr = 0, cg = 0, cb = 0;
    if (worldId == 0) {
        cr = 0.039f; cg = 0.078f; cb = 0.055f;
    } else if (worldId == 2) {
        cr = 0.0f;   cg = 0.0f;    cb = 0.039f;
    } else if (worldId == 10) {
        cr = 0.012f; cg = 0.099f;  cb = 0.172f;
    }
    glClearColor(cr, cg, cb, 1.0f);

    // ── 5. Main 3D viewport ──────────────────────────────────────────────────
    bool topView = (DAT_083a42e9 != 0);
    // IDA Game_RenderTick: v10 = CameraTopViewEnable ? 480 : 432  (0x1e0 : 0x1b0).
    // El topview usa el viewport ALTO (480 = pantalla completa). Antes 0x180 (384).
    int vpHeight = topView ? 0x1e0 : 0x1b0;
    FUN_005119b0(0, 0, w, vpHeight);
    FUN_005112f0(DAT_083a427c, DAT_083a4278, (float*)&DAT_083a4110);

    // ── 6. 3D render passes ──────────────────────────────────────────────────
    // BUG-FIX 2026-04-28: faltaba la llamada a RenderTerrain (FUN_004f9ac0) que
    // dibuja la malla de tiles del terreno. Sin ella, el cliente entraba al
    // mundo pero quedaba 100% negro.
    if (worldId != 10) {
        FUN_004f9ac0('\0');                      // RenderTerrain(EditFlag=0) — tile mesh
    }
    FUN_004fd800();                              // Terrain_Render — UNCONDICIONAL en IDA (object walker)
    // 2026-05-07: Particle_Render (FUN_0046BE40) — port FIEL desde IDA
    // Game_RenderTick:113. Itera el effect pool y renderiza partículas
    // (gate sparks, magic glow, etc). ANTES no estaba wireado.
    FUN_0046be40();                              // Particle_Render
    FUN_00500aa0();                              // RenderBoids (decoration animals)
    FUN_0045ab00();                              // Entity_RenderAll_3D
    if (DAT_07e11d30 != 0) {                     // if (EditFlag) RenderTerrain(1) — IDA: aquí, no tras Trail
        FUN_004f9ac0('\x01');
    }
    if (!topView) {                              // if (!CameraTopViewEnable) Entity_Render()
        FUN_005038e0();                          // Entity_Render (sprites)
    }
    // 2026-05-07: RenderFishs + RenderBugs — port FIEL desde IDA
    // Game_RenderTick:124-125. Fauna decorativa (peces, mariposas).
    FUN_00502200(0, 0, 0, 0);                    // RenderFishs
    FUN_00500970();                              // RenderBugs
    FUN_0046cb70();                              // SkillEffect_Render
    FUN_00473710();                              // ItemDrop_Render
    FUN_0046bba0();                              // RenderEffects
    Player_Render();                             // Player_Render
    FUN_0046c3e0();                              // Trail_RenderAll
    FUN_00479790();                              // CheckSprites — faltaba (mark sprites antes de BeginSprite)

    FUN_00511cf0();                              // BeginSprite (push + identity)
    if (worldId == 2 && HeroTile != 3 && HeroTile < 10) {
        FUN_0046cb70();
    }
    FUN_00479730();                              // RenderSprites
    FUN_00478c00();                              // RenderParticles (effect pool)
    // 2026-05-06: damage popup numbers (port FIEL desde IDA Game_RenderTick:139).
    // Llamado entre RenderParticles y glPopMatrix para que los números floten en
    // world-space. CreatePoint (= FUN_004792c0 en stubs.cpp:3407) los populeya
    // desde Net_Process case 0x15 (ReceiveAttackDamage).
    FUN_00479330(0, 0, 0, 0);                    // RenderPoints (damage)
    glPopMatrix();

    FUN_004b0310();                              // EntityInfo / CharPreview overlay

    // ── 7. 2D HUD ─────────────────────────────────────────────────────────────
    FUN_005123c0();                              // BeginBitmap (Ortho2D)

    // 2026-05-07: sub_4CB6F0 (Target_Render) — port FIEL desde IDA
    // Game_RenderTick:143. Renderiza nombre del NPC/mob/player hovered.
    // Sin esto el user no ve qué está hovereando.
    FUN_004cb6f0(0, 0, 0, 0);

    // IDA Render_Scene3D always enters the HUD 2D pass once the ortho layer
    // is active.  `topView` was a Ghidra-era misread of an unrelated dialog
    // Y/global and ended up hiding the whole HUD whenever certain windows or
    // temporary states were open.
    Render_GameFrame();                          // full HUD render
    RenderInformation();                        // HUD/notices/chat/help/cursor/info3D

    if (DAT_055c9ff8 != 0 && *(BYTE*)(DAT_055c9ff8 + 8) == 1) {
        if (++DAT_083a7c50 > 10) {
            DAT_083a7c50 = 0;
        }
    } else {
        DAT_083a7c50 = 0;
    }

    FUN_005124b0();                              // EndBitmap
    FUN_00511bc0();                              // EndOpengl
}
