#pragma once
// Render.h - 3D rendering declarations
//
// Call hierarchy:
//   Game_MainLoop
//     └── Game_RenderTick   @ 0x00525A00
//           └── Render_GameFrame @ 0x004BBFB0
//                 ├── Render_CharInfoPanel  @ 0x004BC220 - char info / name panel
//                 ├── Render_HPBars         @ 0x004BCA20 - world-space HP bars
//                 ├── AntiTamper_HashMaintain_A @ 0x004BF540 - XOR obfuscation block
//                 ├── Render_CharPartyInfo  @ 0x004BEC00 - party/char stat text
//                 ├── Render_CharNameTags   @ 0x004BE710 - names/HP above chars
//                 ├── Render_MacroTimer     @ 0x004BF090 - macro timer display
//                 ├── Render_MapLoadText    @ 0x004BF2D0 - map transition text
//                 ├── Render_FloatingText   @ 0x004BD090 - damage/exp floating numbers
//                 ├── Render_QuickButtons   @ 0x004F5820 - quick-action buttons (hover)
//                 ├── Render_BottomHUD      @ 0x004BD2B0 - life/mana bar area (y=387/432)
//                 ├── Render_ChatBox        @ 0x004BE4F0 - chat input + history
//                 ├── AntiTamper_HashMaintain_B @ 0x004BF990 - XOR obfuscation block
//                 ├── AntiTamper_HashMaintain_C @ 0x004BD650 - XOR obfuscation block (659 lines)
//                 ├── AntiTamper_HashMaintain_D @ 0x004BCD20 - XOR obfuscation block
//                 └── Render_Scene3D        @ 0x004BFDE0 - 3D world geometry

#include "stdafx.h"
#include "Render/Entity_PrepareRender.h"
#include "Render/Entity_Render_3D.h"
#include "Render/Entity_DrawAt.h"
#include "Render/Sprite_Draw.h"
#include "Render/Particle_Draw.h"

// Renders one complete frame (called from Game_MainLoop)
// @ 0x00525A00
unsigned int Game_RenderTick(void);

// Renders the full game frame: HUD, 3D world, UI overlays
// @ 0x004BBFB0
void Render_GameFrame(void);

// 3D scene rendering (main world geometry, characters, effects)
// @ 0x004BFDE0
void Render_Scene3D(void);

// --- HUD sub-renderers ---
void Render_CharInfoPanel(void);      // @ 0x004BC220
void Render_HPBars(void);             // @ 0x004BCA20
void Render_CharPartyInfo(void);      // @ 0x004BEC00
void Render_CharNameTags(void);       // @ 0x004BE710
void Render_MacroTimer(void);         // @ 0x004BF090
void Render_MapLoadText(void);        // @ 0x004BF2D0
void Render_FloatingText(void);       // @ 0x004BD090
void Render_QuickButtons(void);       // @ 0x004F5820
void Render_BottomHUD(void);          // @ 0x004BD2B0
void Render_ChatBox(void);            // @ 0x004BE4F0

// --- Anti-tamper HashTable maintenance blocks (interspersed in render pipeline) ---
// Pattern: HashTable_GetIndex + counter@0x161 + XOR 0x584 bytes with DAT_00559050
void AntiTamper_HashMaintain_A(void); // @ 0x004BF540
void AntiTamper_HashMaintain_B(void); // @ 0x004BF990
void AntiTamper_HashMaintain_C(void); // @ 0x004BD650
void AntiTamper_HashMaintain_D(void); // @ 0x004BCD20
