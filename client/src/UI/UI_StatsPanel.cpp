// UI_StatsPanel.cpp
// FUN_0051af50 @ 0x0051af50 — UI_StatsPanel_Render
#include "stdafx.h"

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0051af50(void)

{
  // NOTE: quitado el `return 0` auto-skip de Ghidra. Sin él, el menú ESC
  // (state 0x6e) no se dibujaba. Los únicos loops con end-bound absoluto
  // (`while ((int)piVar7 < 0x83a432c)` en estados 0x8b/0x97) son de 2 iters
  // con guards internos — seguros aunque el pool no esté inicializado.
  float fVar1;
  LPCSTR pCVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  LPCSTR pCVar6;
  int *piVar7;
  longlong lVar8;
  undefined8 uVar9;
  undefined4 uVar10;
  char *ppCVar11;
  LPSIZE ptVar12;
  char cVar13;
  char *pcVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  tagSIZE *ptVar17;
  byte *pbVar18;
  tagSIZE local_e4;
  unsigned int local_dc;  // dual-use: integer, float bits, or pointer (Ghidra artifact)
  unsigned int local_d8;
  float local_d4;
  tagSIZE local_d0;
  char   local_c8 [100];  // text format buffer (was LPCSTR[25], corrected)
  CHAR local_64 [100];

  DAT_00559c8c = 0x100;
  GL_ResetState();
  glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
  SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca00c);
  DAT_00559c80 = 0;
  DAT_00559c78 = 0xffffffff;
  if (DAT_083a7c24 != 0) {
    if (DAT_083a7c24 == 0x6e) {
      if (DAT_005615c0 == 5) {
        pCVar6 = (LPCSTR)0x5;
      }
      else {
        pCVar6 = (LPCSTR)((DAT_005615c0 == 4) + 3);
      }
      if (pCVar6 != (LPCSTR)0x0) {
        local_dc = 0x3c;
        local_d8 = (unsigned int)(uintptr_t)pCVar6;
        do {
          fVar1 = (float)(int)local_dc;
          local_e4.cx = (LONG)fVar1;
          GL_DrawTexture(0xf0,260.0,fVar1,120.0,22.0,0.0,0.0,0.83203125,1.0,'\x01','\x01');
          if ((((0x103 < DAT_083a427c) && (DAT_083a427c < 0x17c)) && ((int)local_dc <= DAT_083a4278)
              ) && (DAT_083a4278 < (int)local_dc + 0x16)) {
            glColor3f(0.8f, 0.6f, 0.4f);  // BUG-FIX bits → float
            GL_SetBlendAdditive();
            GL_DrawTexture(0xf0,260.0,fVar1,120.0,22.0,0.0,0.0,0.83203125,1.0,'\x01','\x01');
            glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
            GL_ResetState();
          }
          local_dc = (int)local_dc + 0x1e;
          local_d8 = local_d8 + -1;
        } while (local_d8 != 0);
      }
      GL_SetBlendSrcOver('\x01');
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d45ba0);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45ba0,iVar3,ptVar17);
      UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x41,
                   (LPCSTR)lpString_07d45ba0,(LPSIZE)0x0,'\0',0);
      if (DAT_005615c0 == 5) {
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d45ccc);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45ccc,iVar3,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x5f,
                     (LPCSTR)lpString_07d45ccc,(LPSIZE)0x0,'\0',0);
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d45df8);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45df8,iVar3,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x7d,
                     (LPCSTR)lpString_07d45df8,(LPSIZE)0x0,'\0',0);
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d46050);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46050,iVar3,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x9b,
                     (LPCSTR)lpString_07d46050,(LPSIZE)0x0,'\0',0);
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d45f24);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45f24,iVar3,ptVar17);
        uVar10 = 0xb9;
      }
      else if (DAT_005615c0 == 4) {
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d45ccc);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45ccc,iVar3,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x5f,
                     (LPCSTR)lpString_07d45ccc,(LPSIZE)0x0,'\0',0);
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d46050);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46050,iVar3,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x7d,
                     (LPCSTR)lpString_07d46050,(LPSIZE)0x0,'\0',0);
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d45f24);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45f24,iVar3,ptVar17);
        uVar10 = 0x9b;
      }
      else {
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d46050);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46050,iVar3,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x5f,
                     (LPCSTR)lpString_07d46050,(LPSIZE)0x0,'\0',0);
        ptVar17 = &local_e4;
        iVar3 = lstrlenA((LPCSTR)lpString_07d45f24);
        GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d45f24,iVar3,ptVar17);
        uVar10 = 0x7d;
      }
      UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),uVar10,
                   (LPCSTR)lpString_07d45f24,(LPSIZE)0x0,'\0',0);
    }
    else if (DAT_083a7c24 == 0x96) {
      local_dc = 0x1e;
      do {
        fVar1 = (float)(int)local_dc;
        local_e4.cx = (LONG)fVar1;
        GL_DrawTexture(0xf0,260.0,fVar1,120.0,22.0,0.0,0.0,0.83203125,1.0,'\x01','\x01');
        if (((0x103 < DAT_083a427c) && (DAT_083a427c < 0x17c)) &&
           (((int)local_dc <= DAT_083a4278 && (DAT_083a4278 < (int)local_dc + 0x16)))) {
          glColor3f(0.8f, 0.6f, 0.4f);  // BUG-FIX bits → float
          GL_SetBlendAdditive();
          GL_DrawTexture(0xf0,260.0,fVar1,120.0,22.0,0.0,0.0,0.83203125,1.0,'\x01','\x01');
          glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
          GL_ResetState();
        }
        local_dc = (int)local_dc + 0x1e;
      } while ((int)local_dc < 0x96);
      GL_SetBlendSrcOver('\x01');
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d46050);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46050,iVar3,ptVar17);
      UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x23,
                   (LPCSTR)lpString_07d46050,(LPSIZE)0x0,'\0',0);
      if (DAT_00559c5c == '\0') {
        pcVar14 = s__s_Off_00561854;
      }
      else {
        pcVar14 = s__s_On_0056184c;
      }
      crt_sprintf(local_64,(const char *)pcVar14);
      ptVar17 = &local_e4;
      iVar3 = lstrlenA(local_64);
      GetTextExtentPointA(DAT_055c9fec,local_64,iVar3,ptVar17);
      UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x41,local_64,
                   (LPSIZE)0x0,'\0',0);
      if (DAT_07e11d80 == '\0') {
        pcVar14 = s__s_Off_00561864;
      }
      else {
        pcVar14 = s__s_On_0056185c;
      }
      crt_sprintf(local_64,(const char *)pcVar14);
      ptVar17 = &local_e4;
      iVar3 = lstrlenA(local_64);
      GetTextExtentPointA(DAT_055c9fec,local_64,iVar3,ptVar17);
      UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x5f,local_64,
                   (LPSIZE)0x0,'\0',0);
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d463d4);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d463d4,iVar3,ptVar17);
      UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),0x7d,
                   (LPCSTR)lpString_07d463d4,(LPSIZE)0x0,'\0',0);
    }
    else if ((((DAT_083a7c24 == 0x8b) || (DAT_083a7c24 == 0x8e)) || (DAT_083a7c24 == 0x8d)) ||
            ((DAT_083a7c24 == 0x8c || (DAT_083a7c24 == 0x9a)))) {
      local_d8 = 0x3c;
      GL_DrawTexture(0xfc,213.0,60.0,213.0,5.0,0.0,0.0,0.83203125,0.625,'\x01','\x01');
      local_dc = 0x41;
      iVar3 = 7;
      switch(DAT_083a7c24) {
      case 0x8b:
      case 0x8d:
      case 0x9a:
switchD_0051bea9_caseD_8b:
        iVar3 = 4;
        break;
      case 0x8e:
        iVar3 = ((DAT_083a7c0c + DAT_083a4324) * 0x12 + 0x31) / 0x28;
        if (iVar3 < 9) {
          if (iVar3 < 4) goto switchD_0051bea9_caseD_8b;
          if (iVar3 < 9) break;
        }
        iVar3 = 9;
      }
      if (0 < iVar3) {
        do {
          GL_DrawTexture(0xfb,213.0,(float)(int)local_dc,213.0,40.0,0.0,0.0,0.83203125,0.625,'\x01',
                       '\x01');
          local_dc = (int)local_dc + 0x28;
          iVar3 = iVar3 + -1;
        } while (iVar3 != 0);
      }
      GL_DrawTexture(0xfc,213.0,(float)(int)local_dc,213.0,5.0,0.0,0.0,0.83203125,0.625,'\x01','\x01')
      ;
      // BUG-FIX 2026-05-03: was `(int)piVar7 < 0x83a432c` (literal end addr from
      // source binary). The real iteration count is 2 (button rects: stride 5
      // ints, IDA bound 0x83a432c - 0x83a4304 = 0x28 = 2 × 20 bytes).
      piVar7 = &DAT_083a4304;
      for (int btnIdx = 0; btnIdx < 2; ++btnIdx) {
        if (0 < piVar7[-3]) {
          // 2026-08-26: el ANCHO estaba como `*(float*)piVar7`, o sea
          // reinterpretando los bits, mientras que el ALTO de la linea de al
          // lado convertia con `(float)`. Los dos salen del mismo descriptor de
          // ints (CreateOkMessageBox escribe `v1[3] = 70; v1[4] = 21;`), asi que
          // los dos tienen que convertir. Reinterpretado, el 70 daba 9.8e-44:
          // ancho cero y boton invisible — el cartel de "OK" no se podia cerrar
          // con el mouse. Misma familia que los bugs de `(float)(uintptr_t)`,
          // con la mezcla de estilos dentro de la misma expresion como pista.
          local_d4 = (float)piVar7[1];
          float local_dc_f = (float)*piVar7;
          GL_DrawTexture(piVar7[-3] + 0xf0,(float)piVar7[-2] + _DAT_00552d40,
                       (float)piVar7[-1] + _DAT_0055290c,local_dc_f,local_d4,0.0,0.0,
                       local_dc_f * _DAT_00552d44,local_d4 * _DAT_00552ae4,'\x01','\x01');
        }
        piVar7 = piVar7 + 5;
      }
      if (DAT_083a7c24 == 0x8d) {
        GL_SetBlendSrcOver('\x01');
        glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
        if (DAT_083a7c08 == '\0') {
          uVar16 = 0x3e4ccccd;
          uVar15 = 0x3e4ccccd;
          uVar10 = 0x3e4ccccd;
LAB_0051c08c:
          glColor3f(*(float*)&uVar10, *(float*)&uVar15, *(float*)&uVar16);  // BUG-FIX
        }
        else if (((0xf8 < DAT_083a427c) && (0xc9 < DAT_083a4278)) &&
                ((DAT_083a427c < 0x109 && (DAT_083a4278 < 0xdf)))) {
          if (DAT_083a4124 == '\0') {
            uVar16 = 0x3f333333;
            uVar15 = 0x3f333333;
            uVar10 = 0x3f333333;
            goto LAB_0051c08c;
          }
          glColor3f(0.5f, 0.5f, 0.5f);  // BUG-FIX bits → float
          DAT_083a4124 = '\0';
        }
        FUN_005126e0(9,253.0,210.0,25.0,16.0,0x43870000);
        glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
        if (DAT_083a7c08 == DAT_083a7c09) {
          uVar16 = 0x3e4ccccd;
          uVar15 = 0x3e4ccccd;
          uVar10 = 0x3e4ccccd;
LAB_0051c13d:
          glColor3f(*(float*)&uVar10, *(float*)&uVar15, *(float*)&uVar16);  // BUG-FIX
        }
        else if ((((0x178 < DAT_083a427c) && (0xc9 < DAT_083a4278)) && (DAT_083a427c < 0x189)) &&
                (DAT_083a4278 < 0xdf)) {
          if (DAT_083a4124 == '\0') {
            uVar16 = 0x3f333333;
            uVar15 = 0x3f333333;
            uVar10 = 0x3f333333;
            goto LAB_0051c13d;
          }
          glColor3f(0.5f, 0.5f, 0.5f);  // BUG-FIX bits → float
          DAT_083a4124 = '\0';
        }
        FUN_005126e0(9,383.0,210.0,25.0,16.0,0x42b40000);
        glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
      }
    }
    else if (DAT_083a7c24 == 0x8f) {
      glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
      local_d8 = 0x78;
      GL_DrawTexture(0xfc,213.0,120.0,213.0,5.0,0.0,0.0,0.83203125,0.625,'\x01','\x01');
      local_dc = 0x7d;
      iVar3 = 5;
      do {
        GL_DrawTexture(0xfb,213.0,(float)(int)local_dc,213.0,40.0,0.0,0.0,0.83203125,0.625,'\x01',
                     '\x01');
        local_dc = (int)local_dc + 0x28;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
      GL_DrawTexture(0xfc,213.0,(float)(int)local_dc,213.0,5.0,0.0,0.0,0.83203125,0.625,'\x01','\x01')
      ;
      local_dc = 0x43340000u;
      iVar3 = 0;
      do {
        if ((((float)DAT_083a4278 < *(float*)&local_dc) ||
            (*(float*)&local_dc + _DAT_00552a2c < (float)DAT_083a4278)) ||
           (((float)DAT_083a427c < _DAT_00552cac || (_DAT_0055246c < (float)DAT_083a427c)))) {
          uVar16 = 0x3f333333;
          uVar15 = 0x3f333333;
          uVar10 = 0x3f333333;
        }
        else {
          uVar16 = 0x3f800000;
          uVar15 = 0x3f800000;
          uVar10 = 0x3f800000;
        }
			// IDA stores the hover tint as float bit patterns (0.7f/1.0f).
			// Preserve that representation when forwarding it to OpenGL.
			glColor3f(*(float*)&uVar10,*(float*)&uVar15,*(float*)&uVar16);
        GL_DrawTexture(0xf0,245.0,*(float*)&local_dc,150.0,35.0,0.0,0.0,0.83203125,1.0,'\x01','\x01');
        SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
        DAT_00559c78 = 0xfffff0c8;
        ptVar17 = &local_e4;
        // IDA RenderErrorMessage (0x51AF50) indexes the text table directly:
        // button 0 = GlobalText[735], button 1 = GlobalText[736].  These were
        // mistakenly routed through unpopulated legacy buffers, so the selector
        // frame rendered but its labels were blank.
        pCVar6 = GlobalText[735 + iVar3];
        iVar4 = lstrlenA(pCVar6);
        GetTextExtentPointA(DAT_055c9fec,pCVar6,iVar4,ptVar17);
        uVar10 = 0;
        cVar13 = '\0';
        ptVar12 = (LPSIZE)0x0;
        lVar8 = (longlong)(*(float*)&local_dc + 12.0f);   // IDA RenderErrorMessage case 143: (__int64)(Height + 12.0)
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(int)lVar8,pCVar6,
                     ptVar12,cVar13,uVar10);
        { float _fdc = *(float*)&local_dc + _DAT_00552854; local_dc = *(unsigned int*)&_fdc; }
        SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca00c);
        DAT_00559c78 = 0xffffffff;
        if (iVar3 == 0) {
          // General: the two explanatory lines immediately above button 0.
          UI_RenderText(0xf8,0x96,GlobalText[872],(LPSIZE)0x0,'\0',0);
          pCVar6 = GlobalText[873];
          uVar10 = 0xa0;
        }
        else {
          // Chaos Weapon: the two explanatory lines immediately above button 1.
          UI_RenderText(0xf8,0xf0,GlobalText[870],(LPSIZE)0x0,'\0',0);
          pCVar6 = GlobalText[871];
          uVar10 = 0xfa;
        }
        UI_RenderText(0xf8,uVar10,pCVar6,(LPSIZE)0x0,'\0',0);
        iVar3 = iVar3 + 1;
      } while (iVar3 < 2);
      glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
    }
    else if ((DAT_083a7c24 == 0x97) || (DAT_083a7c24 == 0x99)) {
      local_d8 = 0x64;
      GL_DrawTexture(0xfc,213.0,100.0,213.0,5.0,0.0,0.0,0.83203125,0.625,'\x01','\x01');
      local_dc = 0x69;
      iVar3 = 3;
      do {
        GL_DrawTexture(0xfb,213.0,(float)(int)local_dc,213.0,40.0,0.0,0.0,0.83203125,0.625,'\x01',
                     '\x01');
        local_dc = (int)local_dc + 0x28;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
      GL_DrawTexture(0xfc,213.0,(float)(int)local_dc,213.0,5.0,0.0,0.0,0.83203125,0.625,'\x01','\x01')
      ;
      // BUG-FIX 2026-05-03: same hardcoded address bound as line 213 — 2 button
      // rects (stride 5 ints, total 0x28 bytes / 0x14 stride = 2 entries).
      piVar7 = &DAT_083a4304;
      for (int btnIdx = 0; btnIdx < 2; ++btnIdx) {
        iVar3 = piVar7[-3];
        if ((0 < iVar3) && (iVar3 < 5)) {
          { float _fdc = (float)piVar7[1]; local_dc = *(unsigned int*)&_fdc; }
          local_d4 = (float)*piVar7;
          GL_DrawTexture(iVar3 + 0xf0,(float)piVar7[-2] + _DAT_00552d40,
                       (float)piVar7[-1] + _DAT_005524f0,local_d4,*(float*)&local_dc,0.0,0.0,
                       local_d4 * _DAT_00552d44,*(float*)&local_dc * _DAT_00552ae4,'\x01','\x01');
        }
        piVar7 = piVar7 + 5;
      }
    }
    else {
      local_d8 = 0x3c;
      GL_DrawTexture(0xf0,213.0,60.0,213.0,64.0,0.0,0.0,0.83203125,1.0,'\x01','\x01');
      if (DAT_083a7c24 == 0x74) {
        DAT_00559c8c = 100;
        UI_RenderInputField(0x116,0x62,0);
        DAT_00559c8c = 0x100;
      }
      else if (((DAT_083a7c24 == 0x72) || (DAT_083a7c24 == 0x7e)) || (DAT_083a7c24 == 0x98)) {
        DAT_00559c8c = 100;
        UI_RenderInputField(0xf3,0x62,0);
        DAT_00559c8c = 0x100;
        if (((DAT_083a427c < 0x143) || (0x16a < DAT_083a427c)) ||
           ((DAT_083a4278 < 0x62 || (0x76 < DAT_083a4278)))) {
          iVar3 = 0xf1;
        }
        else {
          iVar3 = 0xf2;
        }
        GL_DrawTexture(iVar3,323.0,98.0,40.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
        if (((DAT_083a427c < 0x175) || (0x19c < DAT_083a427c)) ||
           ((DAT_083a4278 < 0x62 || (0x76 < DAT_083a4278)))) {
          GL_DrawTexture(0xf3,373.0,98.0,40.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
        }
        else {
          GL_DrawTexture(0xf4,373.0,98.0,40.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
        }
      }
      else if ((((DAT_083a7c24 == 0x80) || (DAT_083a7c24 == 0x77)) || (DAT_083a7c24 == 0x78)) ||
              (DAT_083a7c24 == 0x79)) {
        if (((DAT_083a427c < 0xea) || (0x12f < DAT_083a427c)) ||
           ((DAT_083a4278 < 0x62 || (0x76 < DAT_083a4278)))) {
          iVar3 = 0xf1;
        }
        else {
          iVar3 = 0xf2;
        }
        GL_DrawTexture(iVar3,234.0,98.0,70.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
        if (((DAT_083a427c < 0x14e) || (0x193 < DAT_083a427c)) ||
           ((DAT_083a4278 < 0x62 || (0x76 < DAT_083a4278)))) {
          GL_DrawTexture(0xf3,334.0,98.0,70.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
        }
        else {
          GL_DrawTexture(0xf4,334.0,98.0,70.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
        }
      }
      else if ((((DAT_083a427c < 0x11c) || (0x161 < DAT_083a427c)) || (DAT_083a4278 < 0x62)) ||
              (0x76 < DAT_083a4278)) {
        GL_DrawTexture(0xf1,284.0,98.0,70.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
      }
      else {
        GL_DrawTexture(0xf2,284.0,98.0,70.0,21.0,0.0,0.0,0.546875,0.65625,'\x01','\x01');
      }
    }
  }
  GL_SetBlendSrcOver('\x01');
  pCVar2 = (LPCSTR)(uintptr_t)local_d8;
  pCVar6 = (LPCSTR)(uintptr_t)(local_d8 + 0xf);
  switch(DAT_083a7c24) {
  case 0x15:
  case 0x47:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47a18);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47a18,iVar3,ptVar17);
    ppCVar11 = lpString_07d47a18;
    break;
  case 0x16:
  case 0x3f:
  case 0x41:
  case 0x45:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4824c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4824c,iVar3,ptVar17);
    ppCVar11 = lpString_07d4824c;
    break;
  case 0x17:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d48378);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48378,iVar3,ptVar17);
    ppCVar11 = lpString_07d48378;
    break;
  case 0x18:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d484a4);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d484a4,iVar3,ptVar17);
    ppCVar11 = lpString_07d484a4;
    break;
  case 0x19:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d485d0);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d485d0,iVar3,ptVar17);
    ppCVar11 = lpString_07d485d0;
    break;
  case 0x1a:
  case 0x70:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d477c0);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d477c0,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)lpString_07d477c0,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d478ec);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d478ec,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d478ec;
    break;
  case 0x1b:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47b44);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47b44,iVar3,ptVar17);
    ppCVar11 = lpString_07d47b44;
    break;
  case 0x1c:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47c70);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47c70,iVar3,ptVar17);
    ppCVar11 = lpString_07d47c70;
    break;
  case 0x1d:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47d9c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47d9c,iVar3,ptVar17);
    ppCVar11 = lpString_07d47d9c;
    break;
  case 0x1e:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47ec8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47ec8,iVar3,ptVar17);
    ppCVar11 = lpString_07d47ec8;
    break;
  case 0x1f:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47ff4);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47ff4,iVar3,ptVar17);
    ppCVar11 = lpString_07d47ff4;
    break;
  case 0x20:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d48120);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48120,iVar3,ptVar17);
    ppCVar11 = lpString_07d48120;
    break;
  default:
    return DAT_083a7c24 + -0x15;
  case 0x22:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d558c0);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d558c0,iVar3,ptVar17);
    ppCVar11 = lpString_07d558c0;
    break;
  case 0x23:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d559ec);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d559ec,iVar3,ptVar17);
    ppCVar11 = lpString_07d559ec;
    break;
  case 0x24:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d55b18);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d55b18,iVar3,ptVar17);
    ppCVar11 = lpString_07d55b18;
    break;
  case 0x25:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d49890);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d49890,iVar3,ptVar17);
    ppCVar11 = lpString_07d49890;
    break;
  case 0x26:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d49ae8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d49ae8,iVar3,ptVar17);
    ppCVar11 = lpString_07d49ae8;
    break;
  case 0x36:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d46adc);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46adc,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)lpString_07d46adc,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d46c08);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46c08,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d46c08;
    goto LAB_0051ca70;
  case 0x37:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d46d34);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46d34,iVar3,ptVar17);
    ppCVar11 = lpString_07d46d34;
    goto LAB_0051ca70;
  case 0x3a:
    if (DAT_05826d20 == 0) {
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d49e6c);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d49e6c,iVar3,ptVar17);
      ppCVar11 = lpString_07d49e6c;
    }
    else if (DAT_05826d20 == 3) {
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d49f98);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d49f98,iVar3,ptVar17);
      ppCVar11 = lpString_07d49f98;
    }
    else {
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d47310);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47310,iVar3,ptVar17);
      ppCVar11 = lpString_07d47310;
    }
    goto LAB_0051ca70;
  case 100:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47568);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47568,iVar3,ptVar17);
    ppCVar11 = lpString_07d47568;
    goto LAB_0051ca70;
  case 0x65:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47694);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47694,iVar3,ptVar17);
    ppCVar11 = lpString_07d47694;
    goto LAB_0051ca70;
  case 0x71:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4743c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4743c,iVar3,ptVar17);
    ppCVar11 = lpString_07d4743c;
    goto LAB_0051ca70;
  case 0x72:
    DAT_00559c78 = 0xff0000ff;
    crt_sprintf(local_c8,&DAT_07d46e60);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)local_c8,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d46f8c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46f8c,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d46f8c;
    break;
  case 0x73:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d46884);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46884,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)lpString_07d46884,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d469b0);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d469b0,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d469b0;
    break;
  case 0x74:
    if (DAT_07eaa108 == 0) {
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d48954);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48954,iVar3,ptVar17);
      ppCVar11 = lpString_07d48954;
    }
    else if (DAT_07eaa108 == 1) {
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d48a80);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48a80,iVar3,ptVar17);
      ppCVar11 = lpString_07d48a80;
    }
    else {
      ptVar17 = &local_e4;
      iVar3 = lstrlenA((LPCSTR)lpString_07d48bac);
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48bac,iVar3,ptVar17);
      ppCVar11 = lpString_07d48bac;
    }
    goto LAB_0051ca70;
  case 0x75:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d48cd8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48cd8,iVar3,ptVar17);
    ppCVar11 = lpString_07d48cd8;
    goto LAB_0051ca70;
  case 0x76:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d48e04);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d48e04,iVar3,ptVar17);
    ppCVar11 = lpString_07d48e04;
    goto LAB_0051ca70;
  case 0x77:
    FUN_0045ac80(DAT_07eaa0d8);
    crt_sprintf(local_c8,DAT_07d486fc);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)local_c8,(LPSIZE)0x0,'\0',0);
    crt_sprintf(local_c8,&DAT_07d493e0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    ppCVar11 = local_c8;
    pCVar6 = pCVar2 + 0x15;
    goto LAB_0051ca70;
  case 0x78:
  {
    // 0051AF50 obtains the visible character slot from 0045AC80 before
    // expanding the localized invitation format.  The prior void stub lost
    // that return value, so the party prompt had no inviter name.
    const int characterIndex = FUN_0045ac80((int)DAT_07eaa0e4);
    const char* inviterName = (characterIndex < 400 && DAT_07abf5d0)
        ? (const char*)(uintptr_t)(DAT_07abf5d0 + characterIndex * 916 + 449)
        : "";
    _snprintf_s(local_c8, sizeof(local_c8), _TRUNCATE,
                (const char*)&DAT_07d486fc, inviterName);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)local_c8,(LPSIZE)0x0,'\0',0);
    _snprintf_s(local_c8, sizeof(local_c8), _TRUNCATE,
                (const char*)&DAT_07d48f30, inviterName);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    ppCVar11 = local_c8;
    pCVar6 = pCVar2 + 0x15;
    goto LAB_0051ca70;
  }
  case 0x79:
    // IDA: RenderErrorMessage 0x51C940 — solicitud entrante de Trade.
    // ProtocolCore guarda el nombre exacto de diez bytes en DAT_07EA9834;
    // GlobalText[418] aporta el formato localizado y GlobalText[419] la línea
    // de confirmación. ErrorMessage 128 es guerra de guild, no Trade.
    crt_sprintf(local_c8, (const char*)DAT_07d486fc, DAT_07ea9834);
    SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca014);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 4),
                 (LPCSTR)local_c8,(LPSIZE)0x0,'\0',0);
    SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca00c);
    crt_sprintf(local_c8, GlobalText[419]);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    ppCVar11 = local_c8;
    pCVar6 = pCVar2 + 0x1a;
    goto LAB_0051ca70;
  case 0x7a:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4662c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4662c,iVar3,ptVar17);
    ppCVar11 = lpString_07d4662c;
    goto LAB_0051ca70;
  case 0x7b:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d46758);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d46758,iVar3,ptVar17);
    ppCVar11 = lpString_07d46758;
    goto LAB_0051ca70;
  case 0x7e:
    ptVar17 = &local_e4;
    DAT_00559c78 = 0xff0000ff;
    iVar3 = lstrlenA((LPCSTR)lpString_07d49188);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d49188,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)lpString_07d49188,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d492b4);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d492b4,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d492b4;
    break;
  case 0x7f:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4905c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4905c,iVar3,ptVar17);
    ppCVar11 = lpString_07d4905c;
    goto LAB_0051ca70;
  case 0x80:
    // IDA: RenderErrorMessage 0x51CE0D — invitación a guerra de guild.
    crt_sprintf(local_c8, GlobalText[430], GuildWarName);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)local_c8,(LPSIZE)0x0,'\0',0);
    crt_sprintf(local_c8, GlobalText[EnableSoccer ? 432 : 431]);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)local_c8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)local_c8,iVar3,ptVar17);
    ppCVar11 = local_c8;
    pCVar6 = pCVar2 + 0x15;
    break;
  case 0x84:
    ptVar17 = &local_e4;
    DAT_00559c78 = 0xff0000ff;
    iVar3 = lstrlenA((LPCSTR)lpString_07d470b8);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d470b8,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)lpString_07d470b8,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d471e4);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d471e4,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d471e4;
    goto LAB_0051ca70;
  case 0x85:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d499bc);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d499bc,iVar3,ptVar17);
    ppCVar11 = lpString_07d499bc;
    break;
  case 0x86:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4a0c4);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4a0c4,iVar3,ptVar17);
    ppCVar11 = lpString_07d4a0c4;
    break;
  case 0x87:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4a1f0);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4a1f0,iVar3,ptVar17);
    ppCVar11 = lpString_07d4a1f0;
    break;
  case 0x88:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4a31c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4a31c,iVar3,ptVar17);
    ppCVar11 = lpString_07d4a31c;
    break;
  case 0x89:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4a6a0);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4a6a0,iVar3,ptVar17);
    ppCVar11 = lpString_07d4a6a0;
    break;
  case 0x8a:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47310);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47310,iVar3,ptVar17);
    ppCVar11 = lpString_07d47310;
    break;
  case 0x8b:
  case 0x8d:
  case 0x8e:
    iVar3 = (((7 - DAT_083a7c0c) - DAT_083a4324) * 0x12) / 2 + 0x4c;
    if ((DAT_083a7c24 == 0x8e) && (iVar3 < 0x46)) {
      iVar3 = 0x46;
    }
    local_dc = 0;
    if (0 < DAT_083a4324) {
      pCVar6 = (LPCSTR)&DAT_083a44c4;
      do {
        ptVar17 = &local_e4;
        iVar4 = lstrlenA(pCVar6);
        GetTextExtentPointA(DAT_055c9fec,pCVar6,iVar4,ptVar17);
        UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),iVar3,pCVar6,
                     (LPSIZE)0x0,'\0',0);
        iVar3 = iVar3 + 0x12;
        local_dc = local_dc + 1;
        pCVar6 = pCVar6 + 0x26;
      } while ((int)local_dc < DAT_083a4324);
    }
    local_d4 = (float)((DAT_083a4278 - iVar3) / 0x12);
    if (DAT_083a4278 - iVar3 < 0) {
      local_d4 = -NAN;
    }
    if ((DAT_083a7c24 == 0x8e) && (local_dc = 0, 0 < DAT_083a7c0c)) {
      local_d8 = (unsigned int)(uintptr_t)DAT_083a4348;
      do {
        if (((int)local_d4 == (int)local_dc) &&
           (uVar5 = (int)(0x13fU - DAT_083a427c) >> 0x1f,
           (int)((0x13fU - DAT_083a427c ^ uVar5) - uVar5) < 0x6b)) {
          uVar10 = 0;
        }
        else {
          uVar10 = 0x3f800000;
        }
        glColor3f(1.0f, *(float*)&uVar10, 0.0f);  // BUG-FIX
        local_e4.cx = 0;
        pCVar6 = (LPCSTR)(uintptr_t)local_d8;
        do {
          if (*pCVar6 == '\0') break;
          ptVar17 = &local_d0;
          iVar4 = lstrlenA(pCVar6);
          GetTextExtentPointA(DAT_055c9fec,pCVar6,iVar4,ptVar17);
          UI_RenderText(0x140 - ((uint)(local_d0.cx * 0x280) / DAT_0056156c >> 1),iVar3,pCVar6,
                       (LPSIZE)0x0,'\0',0);
          iVar3 = iVar3 + 0x12;
          local_e4.cx = local_e4.cx + 1;
          pCVar6 = pCVar6 + 0x26;
        } while (local_e4.cx < 1);
        local_dc = local_dc + 1;
        local_d8 = local_d8 + 0x26;
      } while ((int)local_dc < DAT_083a7c0c);
    }
    glColor3f(1.0f, 1.0f, 1.0f);  // BUG-FIX: 0x3f800000 son los bits de 1.0f
    return 0;
  case 0x8c:
    iVar3 = FUN_0051ddf0();
    return iVar3;
  case 0x8f:
    DAT_00559c78 = 0xff0080ff;
    SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d5f94c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d5f94c,iVar3,ptVar17);
    ppCVar11 = lpString_07d5f94c;
    break;
  case 0x90:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d47310);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d47310,iVar3,ptVar17);
    ppCVar11 = lpString_07d47310;
    break;
  case 0x91:
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)&lpString_0056186c);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)&lpString_0056186c,iVar3,ptVar17);
    ppCVar11 = lpString_0056186c;
    break;
  case 0x97:
  case 0x99:
    local_dc = local_d8 + 0x1e;
    local_d8 = 0x15e;
    if (DAT_083a7c24 == 0x99) goto LAB_0051d371;
    if (DAT_07eaa13c != 2) {
      if (DAT_07eaa13c != 3) {
        if (DAT_07eaa13c != 4) goto LAB_0051d371;
        pCVar6 = pCVar2 + 0x2d;
      }
      local_d8 = 0x140;
      local_dc = (unsigned int)(uintptr_t)pCVar6;
      goto LAB_0051d371;
    }
    local_d8 = 0x140;
    SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
    glColor3f(1.0f, 1.0f, 0.0f);  // BUG-FIX
    uVar5 = ((int)DAT_07eaa16c < 0) - 1 & DAT_07eaa16c;
    if (DAT_07eaa16c == 0xb) {
      uVar5 = 7;
LAB_0051d2dd:
      iVar3 = uVar5 + 0x25f;
    }
    else {
      iVar3 = uVar5 + 0x259;
      if (4 < (int)uVar5) goto LAB_0051d2dd;
    }
    ptVar17 = &local_d0;
    pCVar6 = &DAT_07d29d24 + iVar3 * 300;
    iVar3 = lstrlenA(pCVar6);
    GetTextExtentPointA(DAT_055c9fec,pCVar6,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_d0.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 0x19),pCVar6,
                 (LPSIZE)0x0,'\0',0);
    local_dc = (unsigned int)(uintptr_t)(pCVar2 + 0x3b);
    SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca00c);
LAB_0051d371:
    iVar3 = 0;
    if (DAT_083a4324 < 1) {
      return DAT_083a4324;
    }
    do {
      glColor3f(1.0f, 0.7f, 0.0f);  // BUG-FIX
      SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
      if (iVar3 == 0) {
        if (DAT_07eaa13c == 1) {
          uVar10 = 0;
LAB_0051d3cf:
          glColor3f(1.0f, *(float*)&uVar10, *(float*)&uVar10);  // BUG-FIX
        }
      }
      else if ((iVar3 == 3) && (DAT_07eaa13c == 3)) {
        uVar10 = 0x3dcccccd;
        goto LAB_0051d3cf;
      }
      if (DAT_07eaa13c == 2) {
        glColor3f(0.9f, 0.9f, 1.0f);  // BUG-FIX
      }
      ptVar17 = &local_d0;
      pCVar6 = (LPCSTR)((int)&DAT_083a44c4 + iVar3 * 0x26);
      iVar4 = lstrlenA(pCVar6);
      GetTextExtentPointA(DAT_055c9fec,pCVar6,iVar4,ptVar17);
      pCVar2 = (LPCSTR)(uintptr_t)local_dc;
      UI_RenderText((int)local_d8 - ((uint)(local_d0.cx * 0x280) / DAT_0056156c >> 1),local_dc,pCVar6
                   ,(LPSIZE)0x0,'\0',0);
      local_dc = (unsigned int)(uintptr_t)(pCVar2 + 0x12);
      iVar3 = iVar3 + 1;
      if (DAT_083a4324 <= iVar3) {
        return DAT_083a4324;
      }
    } while( true );
  case 0x98:
    ptVar17 = &local_e4;
    DAT_00559c78 = 0xff0000ff;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4a448);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4a448,iVar3,ptVar17);
    UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)(pCVar2 + 9),
                 (LPCSTR)lpString_07d4a448,(LPSIZE)0x0,'\0',0);
    ptVar17 = &local_e4;
    iVar3 = lstrlenA((LPCSTR)lpString_07d4a574);
    GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString_07d4a574,iVar3,ptVar17);
    pCVar6 = pCVar2 + 0x15;
    ppCVar11 = lpString_07d4a574;
LAB_0051ca70:
    uVar9 = UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)pCVar6,
                         (LPCSTR)ppCVar11,(LPSIZE)0x0,'\0',0);
    return (int)uVar9;
  case 0x9a:
    uVar9 = FUN_0051db00();
    return (int)uVar9;
  }
  uVar9 = UI_RenderText(0x140 - ((uint)(local_e4.cx * 0x280) / DAT_0056156c >> 1),(unsigned int)(uintptr_t)pCVar6,
                       (LPCSTR)ppCVar11,(LPSIZE)0x0,'\0',0);
  return (int)uVar9;
}
