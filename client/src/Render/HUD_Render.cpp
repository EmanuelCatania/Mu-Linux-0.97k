// HUD_Render.cpp
// Render_GameFrame   @ 0x004BBFB0  (56 lines, completo)
// Render_CharInfoPanel @ 0x004BC220 (234 lines, completo)
// Render_HPBars      @ ~0x004BC900  (87 lines, completo)
// Render_BottomHUD   @ ~0x004BD350  (200 lines, completo)
// Render_ChatBox     @ ~0x004BD600  (54 lines, completo)
// Render_MacroTimer  @ ~0x004BBD50  (70 lines, completo)
// Render_FloatingText @ ~0x004BC000 (85 lines, completo)
// Render_QuickButtons @ ~0x004BE000 (90 lines, completo)
// Render_Scene3D     @ ~0x004BF000  (54 lines, completo)
// Render_CharPartyInfo @ 0x004BEC00  (278 lines, COMPLETO)
// Render_CharNameTags  @ 0x004BE710  (264 lines, COMPLETO)
// Render_MapLoadText   @ 0x004BF2D0  (142 lines, COMPLETO)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_GameFrame @ 0x004BBFB0  (56 líneas, COMPLETO)
// ══════════════════════════════════════════════════════════════════════════════
//
//   Orden completo de llamadas HUD 2D:
//
//   void Render_GameFrame(void)
//   {
//     // g_GameSubState == 8: map-loading screen scrolling textures
//     if (DAT_0055a7ac == 8) {
//       FUN_00511680('\x01');
//       glColor4f(1.0, 1.0, 1.0, 0.5);
//       FUN_00511710();                 // Frame_UpdateTimer()
//       glColor3f(0.3, 0.3, 0.25);
//       // Scroll layer 1: tex 0x494 fullscreen, UV scrolled by ftol()%100000 * scale
//       FUN_005128c0(0x494, 0.0, 0.0, 640.0, 435.0, (float)(ftol()%100000)*_DAT_00552b88, 0,2,3);
//       // Scroll layer 2: tex 0x495 fullscreen, UV scrolled diferente rate
//       FUN_005128c0(0x495, 0.0, 0.0, 640.0, 435.0, (float)(ftol()%100000)*_DAT_00552500,  0,3,2);
//     }
//     glColor3f(1.0, 1.0, 1.0);
//
//     // HUD principal (orden exacto)
//     Render_CharInfoPanel();      // estado de personaje (hp, nombre, buffs)
//     Render_HPBars();             // barras HP de mobs en pantalla
//     AntiTamper_HashMaintain_A(); // anti-tamper, no game logic
//     iVar1 = Render_CharPartyInfo(); // grupo/party info → devuelve Y offset
//     Render_CharNameTags(iVar1);  // name tags de jugadores en pantalla
//     Render_MacroTimer();         // timer de macro + notificaciones
//     Render_MapLoadText();        // texto de carga de mapa (sub 10..16)
//     Render_FloatingText();       // floating damage/heal numbers
//     Render_QuickButtons();       // botones de acción rápida
//     (**(code**)(*DAT_055c9ff0 + 0x10))(); // vtable call (menu/inventory?)
//     Render_BottomHUD();          // barra inferior (HP/MP/SP + hotbar)
//     Render_ChatBox();            // caja de chat
//     AntiTamper_HashMaintain_B(); // anti-tamper
//     AntiTamper_HashMaintain_C(); // anti-tamper
//     AntiTamper_HashMaintain_D(); // anti-tamper
//     Render_Scene3D();            // 3D pass final (skills desde 2D ortho → back to 3D)
//     FUN_004f6050();              // — post-render 1
//     FUN_004eb070();              // — post-render 2
//   }
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_CharInfoPanel @ 0x004BC220  (234 líneas, COMPLETO)
// ══════════════════════════════════════════════════════════════════════════════
//
//   Renderiza el panel de información del personaje (nombre, buffs, estado).
//
//   void Render_CharInfoPanel(void)
//   {
//     FUN_00511600();   // GL_ResetBlend()
//     DAT_00559c80 = 0x80000000;
//
//     // BLOQUE 1: buff activo (DAT_05826d30 != '\0' && entity[0x1da] != -1)
//     if (DAT_05826d30 != '\0' && *(short*)(DAT_07abf5d8+0x1da) != -1) {
//       FUN_00511680('\x01');   // GL_SetMode(1)
//       iVar3 = FUN_004cb520(); // Screen_GetWidth()
//       fVar2 = (float)iVar3 * 0.5f - _DAT_005524fc;   // X centrado
//       SelectObject(DC, font_small);
//       // Color según DAT_05826d32:
//       DAT_00559c78 = DAT_05826d32 ? 0x005901 : 0xffc301;   // verde o amarillo
//       FUN_004f0100((int)*(short*)(DAT_07abf5d8+0x1da), '\x01'); // SetBuffIcon(id)
//       Texture_Draw2D(0x22, fVar2, 4.0, 8.0, 8.0, 0,0,1,1, '\x01', '\x01');
//       crt_sprintf(local_64, &DAT_00559ef0);   // format buff name
//       DrawText(ftol()+0xd, 4, local_64, 0, 1, 0);
//       // Buscar nombre del buff en tabla DAT_07e919bc (stride 0x50)
//       lpString = &DAT_07e919bc + *(short*)(DAT_07abf5d8+0x1da) * 0x50;
//       DrawText(ftol()+0x1e, 4, lpString, 0, 1, 0);
//
//       // Buscar icono del buff (search en tabla &DAT_07e919bc..0x7ea51ec stride 0x50)
//       // comparando con &lpString_05826bfc
//       // Color hover: 0xff00a6ff o 0x9600
//       FUN_004f0100(found_idx, '\x01');   // SetBuffIcon(found_idx)
//       Texture_Draw2D(0x22, fVar2, 19.0, 8.0, 8.0, ...);
//       crt_sprintf(local_64, &DAT_00559ef4);   // segundo stat
//       DrawText(ftol()+0xd, 0x13, local_64, 0, 1, 0);
//       DrawText(ftol()+0x1e, 0x13, &lpString_05826bfc, 0, 1, 0);
//     }
//
//     // BLOQUE 2: segundo buff (DAT_05826d33 != '\0')
//     if (DAT_05826d33 != '\0') {
//       // color 0xff003cff (rojo)
//       // buscar &lpString_05826cc0 en tabla, mostrar similar a bloque 1
//       // DAT_00559ef8/ef4/efc → formatos para tercer/cuarto stat
//       // &lpString_05826cc9 → nombre del buff 2
//     }
//
//     // BLOQUE 3: kill counter (DAT_05826c08 > 0)
//     if (0 < DAT_05826c08) {
//       crt_sprintf(local_64, "%d/%d");   // s__d__d_00559f00
//       DAT_00559c78 = 0xff968cff;        // lavanda
//       // Dibujar en (x, y+100): &lpString_00559f08
//       // Dibujar en (x, y+0xd): local_64
//     }
//   }
//
//   Entity offsets relevantes:
//   entity + 0x1da  → short: buff slot activo (-1 = ninguno)
//   DAT_07e919bc    → tabla de buffs (stride 0x50 bytes)
//   lpString_05826bfc → nombre buff actual
//   lpString_05826cc0 → nombre buff 2
//   lpString_05826cc9 → nombre buff 2b
//   DAT_05826d30..d33 → flags de buff activo
//   DAT_05826c08    → kill counter
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_HPBars @ 87 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Muestra barras de HP sobre mobs visibles en pantalla.
//
//   void Render_HPBars(void)
//   {
//     if (DAT_07eaa0e0 <= 0) return;
//     // Array: &DAT_07e11e98, stride 0x24, count=DAT_07eaa0e0
//     // Offset +4: entity index (-1 = vacío)
//     // Offset +0: HP percentage (0..10, capped a 10)
//     for each entry (entity_idx != -1):
//       pos = entity[entity_idx][+0x10..+0x18] (world pos)
//       z_offset = entity[entity_idx][+300] + entity[entity_idx][+0x18] + _DAT_005524f0
//       FUN_005113f0(&pos, &screenX, &screenY);  // World_ToScreen
//       screenX -= 0x13;
//       // Tooltip si mouse sobre barra:
//       if (screenX <= mouse_x < screenX+bar_w && screenY-2 <= mouse_y < screenY+6):
//         crt_sprintf(local_64, "HP [  %d0]");
//         DAT_00559c7c = 0xffd2e6ff;
//         DrawText(screenX, screenY-6, local_64, ...)
//       // Dibuja barra:
//       GL_SetMode(1);
//       glColor4f(0,0,0,0.5);      // sombra
//       FUN_005124c0(sx+1, sy+1, 42, 5);    // DrawRect background
//       FUN_00511710();             // Frame_UpdateTimer()
//       glColor3f(0.2,0,0);        // rojo oscuro (vacío)
//       FUN_005124c0(sx, sy, 42, 5);
//       glColor3f(0.196,0.032,0);  // segmento oscuro
//       FUN_005124c0(sx+2, sy+2, 38, 1);
//       // Segmentos HP: up to 10 segments de 3px × 2px
//       hp_segs = min(entry[0], 10);
//       glColor3f(0.992, 0.032, 0); // verde-amarillo
//       for i in range(hp_segs):
//         FUN_005124c0(sx+2+i*4, sy+2, 3, 2);
//       GL_ResetBlend();
//
//     GL_ResetBlend();
//     glColor3f(1,1,1);
//   }
//
//   Globals:
//   DAT_07eaa0e0    — count de barras activas
//   &DAT_07e11e98   — array de barras (stride 0x24)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_BottomHUD @ 200 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Renderiza la barra inferior: esquinas, slots de hotbar.
//
//   void Render_BottomHUD(void)
//   {
//     GL_SetMode(1);
//     DAT_07e11d6e = 0;
//     DAT_00559c78 = 0xffffffff;
//     DAT_00559c80 = 0xff000000;
//     // Esquinas del HUD inferior
//     Texture_Draw2D(0xe9, 0,   387, 108, 45, 0,0,0.844,0.703, '\x01', '\x01');  // izq
//     Texture_Draw2D(0xe9, 532, 387, 108, 45, 0.844,0,-0.844,0.703, '\x01', '\x01'); // der (flip)
//     GL_ResetBlend();
//     // Barras de HP/MP/SP
//     Texture_Draw2D(0xe6, 0,   432, 256, 48, 0,0,1.0,0.75, '\x01', '\x01');  // left panel
//     // [HashTable anti-tamper block 1: ~60 líneas omitidas]
//     crt_sprintf(local_68, "%d/%d");   // s__d__d_00559f1c (HP/MaxHP)
//     // [HashTable anti-tamper block 2: ~40 líneas omitidas]
//     Texture_Draw2D(0xe7, 256, 432, 128, 48, 0,0,1.0,0.75, '\x01', '\x01');  // center panel
//     Texture_Draw2D(0xe8, 384, 432, 256, 48, 0,0,1.0,0.75, '\x01', '\x01');  // right panel
//     GL_ResetBlend();
//   }
//
//   Texturas del panel inferior:
//   0xe6 → left (HP bar area)   256×48 en (0, 432)
//   0xe7 → center               128×48 en (256, 432)
//   0xe8 → right (skills/mini)  256×48 en (384, 432)
//   0xe9 → corner piece         108×45 en (0/532, 387) — mirrored
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_ChatBox @ 54 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Renderiza la caja de chat (si DAT_00559c84 || DAT_07e11d71).
//
//   void Render_ChatBox(void)
//   {
//     glColor3f(1,1,1);
//     SelectObject(DC, font_normal);
//     if (DAT_00559c84 != '\0' || DAT_07e11d71 != '\0') {
//       GL_ResetBlend();
//       Texture_Draw2D(0xf8, 186, 415, 134, 29, 0,0,0.520,0.906, ...); // input bg left
//       Texture_Draw2D(0xf9, 320, 415, 134, 29, 0,0,0.520,0.906, ...); // input bg right
//       GL_SetMode(1);
//       // Font size proporcional al ancho de pantalla
//       DAT_00559c8c = min((DAT_0056156c * 180) / 640, 0x100);
//       DrawInputCaret(0xbf, 0x1a6, 0);   // cursor en el input box de chat
//       DAT_00559c8c = (DAT_0056156c * 50) / 640;
//       DrawInputCaret(0x178, 0x1a6, 1);
//       DAT_00559c8c = 0x100;
//       DAT_00559c78 = 0xffc8c8c8;   // gris claro
//       DAT_00559c80 = 0x64000000;   // transparente
//       // Iterar historial de chat (array en &DAT_07e113e4, stride 0x100)
//       iVar2 = 0;
//       lpString = &DAT_07e113e4;
//       iVar1 = DAT_00559cc4;   // offset del mensaje más reciente
//       do {
//         if (iVar2 != iVar1):
//           DrawText(0x178, (iVar2-iVar1)*15 + 0x1a6, lpString, 0,1,0)
//         lpString += 0x100;
//         iVar2++;
//       } while (lpString < 0x7e118e4);
//       // Ícono de desconexión (if DAT_00559bf0)
//       if (DAT_00559bf0 != '\0'):
//         Texture_Draw2D(0xef, 432, 420, 15, 16, 0,0,0.9375,1.0, ...);
//     }
//   }
//
//   Globals:
//   DAT_00559c84   — chat input visible flag
//   DAT_07e11d71   — chat overlay flag
//   DAT_07e113e4   — chat history buffer (stride 0x100, ~10 entradas hasta 0x7e118e4)
//   DAT_00559cc4   — índice del mensaje más reciente
//   DAT_00559bf0   — disconnected indicator flag
//   0xf8/0xf9     → background del chat input box
//   0xef          → ícono de desconexión
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_MacroTimer @ 70 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Muestra timer de macro activo y notificaciones de skills con cooldown.
//
//   void Render_MacroTimer(void)
//   {
//     // Barra de macro activo (DAT_07e11d7c > 0)
//     if (0 < DAT_07e11d7c) {
//       iVar2 = Screen_GetWidth();
//       iVar5 = DAT_07e11d7c * 50;   // progress en pixels
//       fVar1 = ((float)iVar2 - _DAT_00552598) * 0.5f;
//       GL_SetMode(1);
//       DrawText(ftol(), 0x188, "Macro Time", 0,1,0);   // s_Macro_Time_00559f30
//       FUN_004bbdd0(fVar1, 404.0, 50.0, 2.0, (float)(iVar5/100), '\0', '\x01');
//       // FUN_004bbdd0 = DrawProgressBar(x, y, max_w, h, pct, ?, filled)
//     }
//     glColor3f(1,1,1);
//
//     // Notificación de skill (DAT_07e11d8c > 0)
//     if (0 < DAT_07e11d8c) {
//       DVar3 = GetTickCount();
//       if (DVar3 - DAT_07e11d90 > 30000):   // expirado (30s)
//         DAT_07e11d8c = 0; return;
//       GL_ResetBlend();
//       GL_SetMode('\0');
//       DAT_00559c80 = 0x80000000;
//       DAT_00559c78 = 0xffff8080;   // rojo claro
//       // Skill name por índice (0..3 y 8+: &DAT_07d589f8, 4..7: &DAT_07d65e14, stride 300)
//       if (DAT_07e11d8c < 4 || DAT_07e11d8c > 7):
//         pCVar4 = &DAT_07d589f8 + DAT_07e11d8c * 300;
//       else:
//         pCVar4 = &DAT_07d65e14 + DAT_07e11d8c * 300;
//       // Mostrar "SkillName (%d)" con segundos restantes
//       wsprintfA(local_100, pCVar4, 30 - (DVar3-DAT_07e11d90)/1000);
//       DrawText(10, 0x174, local_100, 0,1,0);
//     }
//   }
//
//   Globals:
//   DAT_07e11d7c   — macro timer restante (decrementado en Game_MainLoop)
//   DAT_07e11d8c   — skill notification type (0=ninguno)
//   DAT_07e11d90   — tick cuando empezó la notificación
//   DAT_07d589f8   — tabla de nombres skill (stride 300, grupos 0-3 y 8+)
//   DAT_07d65e14   — tabla de nombres skill (stride 300, grupos 4-7)
//   FUN_004bbdd0   → DrawProgressBar(x,y,max_w,h,pct,?,filled)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_FloatingText @ 85 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Muestra números flotantes de daño/curación sobre entidades.
//   Dos pasadas: primero calcula posición 2D, luego resuelve colisiones, luego dibuja.
//
//   void Render_FloatingText(void)
//   {
//     SelectObject(DC, font_normal);
//     // Array en &DAT_07e0192c, stride 0x95*4 = 0x254
//     // Pasada 1: project world→screen
//     piVar7 = &DAT_07e0192c;
//     do {
//       if (piVar7[-0x83] > 0 || piVar7[-2] > 0):   // activo
//         iVar2 = *piVar7;   // entity ptr
//         if (iVar2 == 0):
//           pfVar5 = (float*)(piVar7+5);   // posición fija
//         else:
//           pos.x = entity[+0x10]; pos.y = entity[+0x14];
//           pos.z = entity[+300] + entity[+0x18] + _DAT_0055290c;
//         FUN_005113f0(pfVar5, &screenX, &screenY);  // World_ToScreen
//         FUN_00480c60((LPCSTR)(piVar7-0x8d));       // MeasureText(str)
//         piVar7[1] = screenX - (piVar7[3]*640/screen_w)/2;  // X centrado
//         piVar7[2] = screenY - 0x24;                         // Y elevado
//       piVar7 += 0x95;
//     } while (piVar7 < 0x7e101fc);
//
//     // Pasada 2: resolver colisiones entre labels (push-apart)
//     // dos loops anidados: si overlap horizontal y vertical → mover el más nuevo
//
//     // Pasada 3: dibujar
//     piVar7 = &DAT_07e01924;
//     do {
//       if (piVar7[-0x81] > 0 || *piVar7 > 0):
//         FUN_00480e00(piVar7[3], piVar7[4], (byte*)(piVar7-0x8b));  // DrawFloatLabel(x,y,str)
//       piVar7 += 0x95;
//     } while (piVar7 < 0x7e101f4);
//   }
//
//   Pool de labels:
//   &DAT_07e0192c   — array (stride 0x95*4), base index +5 para pos fija
//   Offsets: [-0x83]=timer activo, [-2]=?, [0]=entity ptr, [1]=screenX, [2]=screenY
//            [3]=text width, [4]=display Y (ajustado), [5..7]=pos fija
//            [-0x8d]=string label, [-0x8b]=display string
//   FUN_00480c60   → MeasureText(str) → escribe en piVar7[3]
//   FUN_00480e00   → DrawFloatLabel(x, y, str)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_QuickButtons @ 90 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Botones de acción rápida (2 en esquina, 2 pequeños con tooltip, + skill slots).
//
//   void Render_QuickButtons(void)
//   {
//     glColor3f(1,1,1);
//     GL_ResetBlend();
//     DAT_00559c78 = 0xffff8080;
//
//     // Botón 1 (DAT_07eaa124 != '\0')
//     if (DAT_07eaa124 != '\0') {
//       FUN_004f0480(0x1c2, 0);   // PrepareButton1(state)
//       glColor3f(1,1,1);
//       fVar1 = (float)DAT_07ea5b1c + _DAT_005524fc;
//       fVar2 = (float)DAT_07ea5b20 + _DAT_00552ca4;
//       Texture_Draw2D(0x118, fVar1, fVar2, 70, 21, 0,0,2.1875,0.656, ...); // background
//       // Hit-test mouse → tex 0xf1 (hover) o 0xf2 (normal)
//       iVar4 = mouse_in_rect(fVar1,fVar2,70,21) ? 0xf2 : 0xf1;
//       Texture_Draw2D(iVar4, fVar1, fVar2, 70, 21, 0,0,0.547,0.656, ...);
//       // Botón offset alternativo
//       fVar1 += _DAT_005524f0;
//       Texture_Draw2D(0x118, fVar1, fVar2, 70, 21, ...);
//       iVar4 = mouse_in_rect(...) ? 0xf4 : 0xf3;
//       Texture_Draw2D(iVar4, ...);
//     }
//
//     // Botón ícono pequeño 1 (DAT_07eaa114 != '\0')
//     if (DAT_07eaa114 != '\0') {
//       FUN_004f0810(0x1c2, 0);   // PrepareIconButton1(state)
//       glColor3f(1,1,1);
//       fVar1 = (float)DAT_07e91788 + _DAT_00552464;
//       fVar2 = (float)DAT_07e91784 + _DAT_0055246c;
//       Texture_Draw2D(0x118, fVar1, fVar2, 24, 24, 0,0,0.75,0.75, ...);
//       // Tooltip si hover: DAT_07d39ef4, posición (ftol()-13, ftol())
//       if (mouse_in_rect(fVar1,fVar2,24,24)):
//         DrawText(ftol(), ftol()-13, &DAT_07d39ef4, ...);
//     }
//
//     // Botón ícono pequeño 2 (DAT_07eaa116 != '\0')
//     if (DAT_07eaa116 != '\0') {
//       FUN_004ecc60(0x1c2, 0);   // PrepareIconButton2(state)
//       // similar al anterior pero con DAT_07ea982c/9830 y tooltip DAT_07d3a14c
//     }
//
//     FUN_004ef160(0x1c2, 0);   // RenderSkillSlot_0(state)
//     FUN_004f0a50();           // RenderSkillSlot_1()
//     FUN_004f1270();           // RenderSkillSlot_2()
//     FUN_004f1f50();           // RenderSkillSlot_3()
//     FUN_004f27f0();           // RenderSkillSlot_4()
//     FUN_004f3170();           // RenderSkillSlot_5()
//     FUN_004f3c50();           // RenderSkillSlot_6()
//     FUN_004f46a0();           // RenderSkillSlot_7()
//     FUN_00403f30(DAT_00583d8c); // — (vtable?)
//     FUN_004f5570();           // RenderSkillSlot_8?
//   }
//
//   Globals:
//   DAT_07eaa124   — flag botón 1 visible
//   DAT_07eaa114   — flag icono 1 visible
//   DAT_07eaa116   — flag icono 2 visible
//   DAT_07ea5b1c/20 — posición X/Y botón 1
//   DAT_07e91788/84 — posición X/Y icono 1
//   DAT_07ea982c/30 — posición X/Y icono 2
//   DAT_07d39ef4   — tooltip string icono 1
//   DAT_07d3a14c   — tooltip string icono 2
//   0x118         → textura background botones
//   0xf1/f2/f3/f4 → texturas botones (normal/hover)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_Scene3D @ 54 líneas, COMPLETO
// ══════════════════════════════════════════════════════════════════════════════
//
//   Vuelve al modo 3D para dibujar skill effects sobre el HUD 2D ya renderizado.
//   Configura proyección + view matrix desde los datos de cámara precalculados.
//
//   void Render_Scene3D(void)
//   {
//     FUN_005124b0();        // GL_End2D()
//     glMatrixMode(GL_PROJECTION);
//     glPushMatrix();
//     glLoadIdentity();
//     FUN_00511910(0, 0, DAT_0056156c, DAT_00561570);    // gluPerspective(fov, aspect, ...)
//     FUN_00511220(1.0, DAT_0056156c/DAT_00561570, DAT_0056154c, DAT_00561550); // near/far
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     glLoadIdentity();
//     FUN_005111d0(&DAT_083a4140);   // LoadCameraMatrix(mat_4x4)
//     FUN_005114d0();                // EnableDepthTest()
//     FUN_00511510();                // EnableDepthWrite()
//     FUN_00403150(DAT_00583d8c, '\x01', '\0');  // ObjPool_SetFlag(pool, true, false)
//
//     // Según modo de vista (DAT_07eaa128):
//     if (DAT_07eaa128 == 0 || DAT_07eaa128 == 3):
//       FUN_004f5ce0();    // Skill_RenderEffects()
//       if (DAT_07e91388 > 0 && DAT_07eaa13c == 0):
//         FUN_004f6420();  // TeleportEffect_Render()
//       // Renderizar skill hotbar (items con cooldown):
//       for i in 0..N:
//         iVar1 = FUN_00482be0(i);   // GetHotbarItem(i)
//         if iVar1 != -1:
//           FUN_004e1be0((float)local_14, 454.0, 20.0, 20.0,
//                        (&DAT_07ea8410)[iVar1*0x22], (&DAT_07ea8414)[iVar1*0x11], 0, '\0');
//                        // DrawItemIcon(x, y, w, h, item_id, count, ?, ?)
//         local_14 += 0x1f;   // siguiente slot (X+31)
//       // X inicial = 0xd0 (208)
//     else:
//       FUN_004f54b0();    // PvP_SkillRender() o similar
//
//     // Reset camera para 2D
//     glLoadIdentity();
//     glTranslatef(-_DAT_083a42d4, -_DAT_083a42d8, -_DAT_083a42dc);
//     FUN_005111d0(&DAT_083a4140);
//     FUN_005112f0(100, 100, local_c);   // Camera_SetupHUD(x,y,out)
//     glPopMatrix();
//     glPopMatrix();
//     FUN_005123c0();   // GL_SetupOrtho2D()
//   }
//
//   Globals:
//   DAT_083a4140   — camera matrix (4×4 floats)
//   DAT_0056156c   — screen_width
//   DAT_00561570   — screen_height
//   DAT_0056154c   — near_clip
//   DAT_00561550   — far_clip
//   DAT_083a42d4/d8/dc — camera world position (X/Y/Z)
//   DAT_07eaa128   — view mode (0=normal, 3=?, otras=PvP?)
//   DAT_07e91388   — teleport effect active
//   DAT_07eaa13c   — teleport flag
//   DAT_07ea8410   — hotbar item array (stride 0x22*2 = item_id)
//   DAT_07ea8414   — hotbar count array (stride 0x11*4)
//   FUN_004e1be0   → DrawItemIcon(x,y,w,h,item_id,count,?,shadow)
//   FUN_004f5ce0   → Skill_RenderEffects()
//   FUN_004f6420   → TeleportEffect_Render()
//   FUN_00482be0   → GetHotbarItem(slot_idx) → item_idx
//   FUN_005111d0   → LoadCameraMatrix(mat_ptr)
//   FUN_005114d0   → EnableDepthTest()
//   FUN_00511510   → EnableDepthWrite()
//   FUN_005112f0   → Camera_SetupHUD(x,y,out)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_CharPartyInfo @ 0x004BEC00  (278 líneas, COMPLETO)
// ══════════════════════════════════════════════════════════════════════════════
//
//   Renderiza el panel de party/grupo. Retorna Y offset para Render_CharNameTags.
//
//   int Render_CharPartyInfo(void)
//   {
//     int   retY  = 0xf;
//     float hpOff = (DAT_07eaa0e0 > 0) ? 50.0f : 0.0f;  // margen si hay HP bars
//
//     // Guard: jugador en modo party  (entity[0x2b8] ∈ 0x330..0x333)
//     if (0x32f < *(short*)(DAT_07abf5d8 + 0x2b8) < 0x334) {
//
//       int   sw     = FUN_004cb520();   // Screen_GetWidth()
//       float base_x = ((float)sw - _DAT_00552598) - hpOff - _DAT_00552834;
//
//       // [HashTable anti-tamper ~60 líneas — omitido]
//
//       byte bHP = *(byte*)(DAT_07cf1ffc + 0x452);   // HP% del líder (0..255)
//       FUN_00511680('\x01');   // SetBlendMode(alpha)
//
//       // Icono/nombre del líder según sub-modo de party:
//       int subMode = *(short*)(DAT_07abf5d8 + 0x2b8);
//       char* pcName = (subMode == 0x332) ? &DAT_07d43d28 :
//                      (subMode == 0x333) ? &DAT_07d43bfc :
//                      (char*)((subMode - 400) * 0x40 + DAT_07d78068);  // sprite
//
//       // lstrlenA(name) + GetTextExtentPointA(g_hDC, name, len, &sz)
//       // FUN_0047f6f0(ftol()+0x32, ftol(), name, 0, '\0', 0)
//       //   → DrawText(x+50, y, name, shadow=0, italic=0, flags=0)
//
//       // FUN_004bbdd0(base_x, (bHP*50/255)+_DAT_00552664, 50.0, 2.0, pct, 0, 1)
//       //   → DrawHPBar(x, y_scaled, w=50, h=2, fill%, mirror=0, colored=1)
//
//       retY = (int)lVar18 + 0x17;   // Y = base + 23 (para name tags encima)
//     }
//
//     // Bloque secundario: barra de party global (DAT_05826d24 = nro de miembros)
//     if (DAT_05826d24 != 0) {
//       int sw2     = FUN_004cb520();
//       int bar_w   = DAT_05826d24 * 0x32;   // member_count × 50
//       float base2 = ((float)sw2 - _DAT_00552598) - _DAT_0055297c;
//       FUN_00511680('\x01');
//       // FUN_0047f6f0(ftol()+0x32, 4, &DAT_07d43e54, 0, '\0', 0)  // label "Party"
//       // FUN_004bbdd0(base2, 16.0, 50.0, 2.0, bar_w/100, 0, 1)    // barra HP party
//     }
//     return retY;
//   }
//
//   Globals:
//   DAT_07abf5d8          — player entity ptr (slot 0)
//   DAT_07eaa0e0          — HP bars visibles (>0 → agregar offset)
//   DAT_07cf1ffc + 0x452  — HP byte del líder de party (0..255)
//   DAT_05826d24          — cantidad de miembros de party
//   _DAT_00552598         — ancho del panel
//   _DAT_00552834         — margen extra panel
//   _DAT_00552664         — Y base de la barra HP
//   _DAT_0055297c         — margen X barra secundaria
//   DAT_07d43d28          — nombre para modo guild (subMode 0x332)
//   DAT_07d43bfc          — nombre para modo trade (subMode 0x333)
//   DAT_07d43e54          — label "Party"
//   FUN_004cb520          → Screen_GetWidth()
//   FUN_004bbdd0          → DrawHPBar(x, y, w, h, fill_pct, mirror, color)
//   FUN_0047f6f0          → DrawText(x, y, str, unk, italic, flags)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_CharNameTags @ 0x004BE710  (264 líneas, COMPLETO)
// ══════════════════════════════════════════════════════════════════════════════
//
//   Dibuja etiquetas de nombre flotante sobre jugadores visibles en pantalla.
//   param_1 = Y offset de Render_CharPartyInfo() (inicio Y de los textos).
//
//   void __cdecl Render_CharNameTags(int param_1)
//   {
//     // [Anti-tamper HashTable sobre DAT_07eaa11b — ~25 líneas, omitido]
//     char antiFlag = DAT_07eaa11b;
//
//     if (antiFlag != '\x01') {   // '\x01' = bloqueado por anti-tamper
//
//       float hpOff  = (DAT_07eaa0e0 > 0) ? 50.0f : 0.0f;
//       glColor3f(1.0, 1.0, 1.0);
//       FUN_00511680('\x01');        // SetBlendMode(alpha)
//       int sw      = FUN_004cb520();
//       float rightX = (float)sw - hpOff - _DAT_00552488;  // límite X derecho
//       float idx    = 0.0f;    // offset de byte en lista on-screen (stride 0x44)
//       int   textY  = 0;       // desplazamiento Y acumulado entre textos
//
//       // Loop sobre las 12 ranuras on-screen (0x330 / 0x44 = 12 entidades máx)
//       do {
//         // [Anti-tamper HashTable por ranura — omitido]
//         // array on-screen: DAT_07cf1ffc + 0x218  (stride 0x44)
//         short entType = *(short*)((int)idx + 0x218 + (int)DAT_07cf1ffc);
//
//         if (entType == -1) {
//           // Ranura vacía: HashTable cleanup, continúa
//         } else {
//           // Excluir: 0x87=NPC-A, 0x8f=NPC-B, 0x1a0..0x1a3=map objects
//           if (entType != 0x87 && entType != 0x8f &&
//               !(0x1a0 <= entType && entType <= 0x1a3)) {
//
//             int level = FUN_004c45c0(psVar1, entType*0x40+DAT_07d78068, field>>3&0xf);
//             level &= 0xffff;
//             if (*psVar1 == 0x1aa) level = 200;  // GM: nivel forzado a 200
//
//             // Color del name tag según HP ratio vs nivel:
//             byte hp = entity[+0xd];
//             if (hp == 0) {
//               entity[+0x20] = 8;           // muerto
//               DAT_00559c78  = 0x800a0aff;  // rojo oscuro
//               DAT_00559c80  = 0x800a0aff;
//             } else {
//               float hpF = (float)hp;
//               if      (hpF > (float)level * _DAT_005526e4) { entity[+0x20]=7; DAT_00559c78=0xff0a0aff; }  // rojo
//               else if (hpF > (float)level * _DAT_005528b8) { entity[+0x20]=6; DAT_00559c78=0xff009cff; }  // naranja
//               else if (hpF > (float)level * _DAT_00552504) { entity[+0x20]=5; DAT_00559c78=0xff00e4ff; }  // verde
//               // else hp muy alto → sin cambio de color
//             }
//
//             // Formatear nombre + HP:  crt_sprintf(buf, "%s %d/%d", name, hp, maxhp)
//             // DAT_07e11d6e = 1  (flag texto visible)
//             // FUN_0047f6f0(ftol()+textY, ftol(), buf, 0, '\0', 0)
//             textY += 0x0c;   // siguiente nombre 12px más abajo
//           }
//         }
//         idx = (float)((int)idx + 0x44);   // siguiente ranura
//       } while ((int)idx < 0x330);
//
//       DAT_00559c80 = 0x80000000;   // reset blend
//     }
//   }
//
//   Globals:
//   DAT_07eaa11b          — anti-tamper flag ('\x01' → skip toda la función)
//   DAT_07eaa0e0          — count HP bars (>0 → offset 50px desde derecha)
//   DAT_07cf1ffc + 0x218  — lista entidades on-screen (stride 0x44, 12 slots)
//   entity[+0xd]          — HP byte actual (0 = muerto)
//   entity[+0x20]         — estado visual del name tag (5..8)
//   DAT_07e11d6e          — flag "texto visible"
//   DAT_00559c78          — color texto ABGR
//   DAT_00559c80          — blend color
//   _DAT_00552488         — margen X derecho
//   _DAT_005526e4/_DAT_005528b8/_DAT_00552504 — umbrales HP (alto/medio/bajo)
//   Tipos excluidos de name tags: 0x87=NPC-A, 0x8f=NPC-B, 0x1a0..0x1a3=map objects
//   FUN_004cb520          → Screen_GetWidth()
//   FUN_004c45c0          → GetEntityLevel(ptr, spriteData, field) → int
//   FUN_0047f6f0          → DrawText(x, y, str, unk, italic, flags)
//
// ══════════════════════════════════════════════════════════════════════════════
// Render_MapLoadText @ 0x004BF2D0  (142 líneas, COMPLETO)
// ══════════════════════════════════════════════════════════════════════════════
//
//   Overlay de carga de mapa/dungeon, visible cuando g_GameSubState ∈ [10..16].
//   Retorna DAT_07e11d88 (tipo de texto activo, 0=off).
//
//   uint Render_MapLoadText(void)
//   {
//     uint type = DAT_07e11d88;  // 0=off, 1=dungeon, 2=PvP, 5=instancia especial
//
//     if (DAT_07e11d88 != 0 && 10 < DAT_0055a7ac && DAT_0055a7ac < 0x11) {
//
//       FUN_00511600();            // GL_ResetBlend()
//       FUN_00511680('\0');        // SetBlendMode(none)
//       glColor3f(1.0, 1.0, 1.0);
//       DAT_00559c80 = 0;
//       DAT_00559c78 = 0xff0096ff; // azul inicial
//
//       // Para tipos 1 (dungeon), 2 (PvP) y 5 (especial):
//       if ((DAT_07e11d88 < 3 || DAT_07e11d88 == 5) && DAT_00559ccc > 0) {
//
//         // Línea 1: "jugadores actuales / máximo" (solo si hay límite)
//         if (DAT_00559cd0 != 0xffff) {
//           LPCSTR fmt = (DAT_07e11d88 == 5) ? &DAT_07d693fc : &DAT_07d691a4;
//           wsprintfA(local_100, fmt, DAT_00559cd4, DAT_00559cd0);
//           SelectObject(g_hDC, DAT_055ca010);        // fuente bold
//           FUN_00514270(0x23a, 0x159, local_100);    // DrawTextCenter(570, 345, str)
//         }
//
//         // Línea 2: timer total (vía ftol())
//         SelectObject(g_hDC, DAT_055ca010);
//         wsprintfA(local_100, &param_2_07d692d0);
//         FUN_00514270(0x23a, (int)__ftol(), local_100);
//
//         // Calcular y construir string "MM:SS" mediante strcat inline:
//         int mins = DAT_00559ccc / 60;
//         int secs = DAT_00559ccc - mins * 60;
//         wsprintfA(local_100, s___2d___00559f3c, mins);   // "%2d:"
//         if (secs >= 0) {
//           wsprintfA(local_10c, s___2d_00559f44, secs);   // "%2d"
//           // strcat(local_100, local_10c) — inline strlen+memcpy
//         }
//
//         if (mins < 5)  DAT_00559c78 = 0xff2020ff;    // rojo (urgente, <5 min)
//
//         if (mins < 15) {
//           // Parpadeo de segundos: (int)(ftol() % 60)
//           // wsprintfA(local_10c, ":%2d", blink_sec)
//           // strcat inline → local_100 += local_10c
//         }
//
//         // Línea final: "MM:SS" con fuente grande
//         SelectObject(g_hDC, DAT_055ca014);            // fuente bold2
//         FUN_00514270(0x23a, (int)__ftol(), local_100);
//       }
//     }
//     return type;
//   }
//
//   Globals:
//   DAT_07e11d88   — tipo de overlay (0=off, 1=dungeon, 2=PvP, 5=especial)
//   DAT_0055a7ac   — g_GameSubState (10..16 = loading overlay activo)
//   DAT_00559ccc   — segundos restantes (int)
//   DAT_00559cd0   — límite jugadores (0xffff = sin límite)
//   DAT_00559cd4   — jugadores actuales
//   DAT_07d691a4   — format "current/max" para tipos 1/2
//   DAT_07d693fc   — format para tipo 5
//   DAT_07d692d0   — format timer total
//   DAT_055ca010   — HFONT bold
//   DAT_055ca014   — HFONT bold2 (grande, para MM:SS final)
//   s___2d___00559f3c — "%2d:"  (minutos)
//   s___2d_00559f44   — "%2d"   (segundos)
//   s_____2d_00559f4c — ":%2d"  (segundos parpadeo)
//   FUN_00514270   → DrawTextCenter(x, y, str)
//   FUN_00511600   → GL_ResetBlend()
//   FUN_00511680   → SetBlendMode(mode)

#include "stdafx.h"
#include "Render/HUD_Render.h"
