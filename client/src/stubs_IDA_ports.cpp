// stubs_IDA_ports.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 28679-57792 (29114 lines).
//
// IDA Hex-Rays ports — reference / inactive code.
//
// 307 functions originally decompiled from IDA Hex-Rays. Each is gated by an
// IDA_PORT_xxxxxxxx preprocessor macro. NONE of these macros are defined in
// the .vcxproj, so this entire file compiles to (almost) nothing.
//
// PURPOSE: keep the IDA decompiles handy as a reference / source of truth for
// port comparison and future bug-fixes. When a function in stubs.cpp or other
// modules needs to be replaced/upgraded with an IDA-faithful version, the
// raw decompile lives here gated.
//
// To activate any single function, add `IDA_PORT_xxxxxxxx` to the
// PreprocessorDefinitions in mu97k.vcxproj.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// Forward decls for symbols that live in stubs.cpp but are referenced by
// IDA-activated (non-gated) functions in this file.
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);

// ════════════════════════════════════════════════════════════════════
// IDA HEX-RAYS PORTS
// Active (in tree above): 44
// Below: absent-in-Ghidra activations + inactive (reference only)
// ════════════════════════════════════════════════════════════════════

// ── FUN_00401010 (IDA-only, gated) ──
#if defined(IDA_PORT_00401010)
// Microsoft VisualC 2-14/net runtime
void *FUN_00401010()
{
  void *result; // eax

  result = &DAT_00567500;
  DAT_00567500 = &DAT_005524b8;
  if ( !g_csQuest )
  {
    g_csQuest = (DWORD)&DAT_00567500;
  }
  *((BYTE *)&DAT_00567500 + 4) = -1;
  *((BYTE *)&DAT_00567500 + 116858) = 0;
  *((BYTE *)&DAT_00567500 + 116859) = 0;
  *((BYTE *)&DAT_00567500 + 116861) = 0;
  *((BYTE *)&DAT_00567500 + 116862) = 0;
  *((BYTE *)&DAT_00567500 + 116863) = 0;
  *((WORD *)&DAT_00567500 + 58432) = 0;
  DAT_00567500 = DAT_005524b4;
  return result;
}
#endif

// ── FUN_00401020 (IDA-only, gated) ──
#if defined(IDA_PORT_00401020)
// Microsoft VisualC 2-14/net runtime
void *FUN_00401020()
{
  void *result; // eax

  result = &DAT_00567500;
  DAT_00567500 = &DAT_005524b8;
  if ( !g_csQuest )
  {
    g_csQuest = (DWORD)&DAT_00567500;
  }
  *((BYTE *)&DAT_00567500 + 4) = -1;
  *((BYTE *)&DAT_00567500 + 116858) = 0;
  *((BYTE *)&DAT_00567500 + 116859) = 0;
  *((BYTE *)&DAT_00567500 + 116861) = 0;
  *((BYTE *)&DAT_00567500 + 116862) = 0;
  *((BYTE *)&DAT_00567500 + 116863) = 0;
  *((WORD *)&DAT_00567500 + 58432) = 0;
  DAT_00567500 = DAT_005524b4;
  return result;
}
#endif

// ── FUN_004011d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004011D0)
short __cdecl CSQuest::FindQuestContext(DWORD This, DWORD pQuest, int index)
{
  int v3; // eax
  int v5; // edx
  BYTE *v6; // ecx

  v3 = 0;
  v5 = *(short *)pQuest;
  if ( v5 <= 0 )
  {
LABEL_5:
    --*(BYTE *)(This + 116858);
    CSQuest::CheckQuestState(This, 0xFFu);
    return *(WORD *)(This + 116864);
  }
  else
  {
    v6 = (BYTE *)(*(unsigned char *)(This + 4) + pQuest + 44);
    while ( !*v6 )
    {
      ++v3;
      v6 += 18;
      if ( v3 >= v5 )
      {
        goto LABEL_5;
      }
    }
    return *(WORD *)(pQuest + 2 * (index + 8 * v3 + v3) + 48);
  }
}
#endif

// ── FUN_00401230 (IDA-only, gated) ──
#if defined(IDA_PORT_00401230)
bool __cdecl CSQuest::CheckRequestCondition(DWORD This, DWORD pQuest, bool bLastCheck)
{
  short *v4; // ecx
  int v5; // ebx
  short *v6; // ebp
  void *v7; // esi
  unsigned int v8; // eax
  BYTE *v9; // eax
  unsigned char v10; // cl
  int v11; // eax
  const void *v12; // esi
  unsigned short v13; // di
  unsigned int v14; // eax
  BYTE *v15; // eax
  char v16; // cl
  void *v17; // esi
  unsigned int v18; // eax
  BYTE *v19; // eax
  unsigned char v20; // cl
  int v21; // eax
  const void *v22; // esi
  unsigned short v23; // di
  unsigned int v24; // eax
  BYTE *v25; // eax
  char v26; // cl
  int v27; // eax
  void *v28; // esi
  unsigned int v29; // eax
  BYTE *v30; // eax
  unsigned char v31; // cl
  int v32; // eax
  const void *v33; // esi
  int v34; // edi
  BYTE *v35; // eax
  char v36; // cl
  int v37; // eax
  short v40; // cx
  short v41; // ax
  DWORD v42; // [esp+10h] [ebp-14h]
  int v43; // [esp+14h] [ebp-10h]
  unsigned char *i; // [esp+18h] [ebp-Ch]
  int v45; // [esp+1Ch] [ebp-8h]
  int v46; // [esp+20h] [ebp-4h]

  v4 = (short *)pQuest;
  v42 = This;
  v45 = 0;
  if ( *(short *)pQuest <= 0 )
  {
    return 1;
  }
  v43 = 0;
  for ( i = (unsigned char *)(pQuest + 43); ; i += 18 )
  {
    if ( *((unsigned char *)&v4[v43 + 22] + *(unsigned char *)(This + 4)) == *(unsigned char *)(This + 5) + 1 )
    {
      v5 = 0;
      v46 = *i;
      if ( v4[1] > 0 )
      {
        break;
      }
    }
LABEL_57:
    v43 += 9;
    if ( ++v45 >= *v4 )
    {
      return 1;
    }
  }
  v6 = v4 + 166;
  while ( 1 )
  {
    if ( *((unsigned char *)v6 - 3) == v46 || *((BYTE *)v6 - 3) == 0xFF )
    {
      if ( *v6 )
      {
        v7 = (void *)CharacterMachine;
        if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
        {
          v11 = operator_new(0x585u);
          *(BYTE *)(v11 + 1412) = 1;
          FUN_00403f80(&DAT_055c9bc8, v11, (int)v7);
        }
        else
        {
          v8 = FUN_004041e0(&DAT_055c9bc8, (int)v7);
          if ( v8 == -1 )
          {
            v9 = 0;
          }
          else
          {
            v9 = *(BYTE **)(DAT_055c9bcc + 4 * v8);
          }
          v10 = v9[1412] + 1;
          v9[1412] = v10;
          if ( v10 < 2u )
          {
            FUN_00404370(v7, v9);
          }
        }
        v12 = (const void *)CharacterMachine;
        v13 = *(WORD *)(CharacterAttribute + 14);
        if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) != -1 )
        {
          v14 = FUN_004041e0(&DAT_055c9bc8, (int)v12);
          v15 = v14 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v14);
          v16 = v15[1412] - 1;
          v15[1412] = v16;
          if ( !v16 )
          {
            FUN_00404400(v15, v12);
          }
        }
        if ( (unsigned short)*v6 > v13 )
        {
          *(WORD *)(v42 + 116864) = *(WORD *)(16 * v5 + pQuest + 340);
          *(BYTE *)(v42 + 116866) = 5;
          return 0;
        }
        v4 = (short *)pQuest;
      }
      if ( v6[1] )
      {
        v17 = (void *)CharacterMachine;
        if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
        {
          v21 = operator_new(0x585u);
          *(BYTE *)(v21 + 1412) = 1;
          FUN_00403f80(&DAT_055c9bc8, v21, (int)v17);
        }
        else
        {
          v18 = FUN_004041e0(&DAT_055c9bc8, (int)v17);
          if ( v18 == -1 )
          {
            v19 = 0;
          }
          else
          {
            v19 = *(BYTE **)(DAT_055c9bcc + 4 * v18);
          }
          v20 = v19[1412] + 1;
          v19[1412] = v20;
          if ( v20 < 2u )
          {
            FUN_00404370(v17, v19);
          }
        }
        v22 = (const void *)CharacterMachine;
        v23 = *(WORD *)(CharacterAttribute + 14);
        if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) != -1 )
        {
          v24 = FUN_004041e0(&DAT_055c9bc8, (int)v22);
          v25 = v24 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v24);
          v26 = v25[1412] - 1;
          v25[1412] = v26;
          if ( !v26 )
          {
            FUN_00404400(v25, v22);
          }
        }
        if ( (unsigned short)v6[1] < v23 )
        {
          v40 = *(WORD *)(16 * v5 + pQuest + 340);
          *(BYTE *)(v42 + 116866) = 5;
          *(WORD *)(v42 + 116864) = v40;
          return 0;
        }
        v4 = (short *)pQuest;
      }
      v27 = *((DWORD *)v6 + 1);
      if ( v27 )
      {
        break;
      }
    }
LABEL_55:
    ++v5;
    v6 += 8;
    if ( v5 >= v4[1] )
    {
      This = v42;
      goto LABEL_57;
    }
  }
  if ( !bLastCheck )
  {
    *(DWORD *)(v42 + 116868) = v27;
    goto LABEL_55;
  }
  v28 = (void *)CharacterMachine;
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
  {
    v32 = operator_new(0x585u);
    *(BYTE *)(v32 + 1412) = 1;
    FUN_00403f80(&DAT_055c9bc8, v32, (int)v28);
  }
  else
  {
    v29 = FUN_004041e0(&DAT_055c9bc8, (int)v28);
    if ( v29 == -1 )
    {
      v30 = 0;
    }
    else
    {
      v30 = *(BYTE **)(DAT_055c9bcc + 4 * v29);
    }
    v31 = v30[1412] + 1;
    v30[1412] = v31;
    if ( v31 < 2u )
    {
      FUN_00404370(v28, v30);
    }
  }
  v33 = (const void *)CharacterMachine;
  v34 = *(DWORD *)(CharacterMachine + 1352);
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) != -1 )
  {
    v35 = (BYTE *)FUN_00404280(&DAT_055c9bc8, v33);
    v36 = v35[1412] - 1;
    v35[1412] = v36;
    if ( !v36 )
    {
      FUN_00404400(v35, v33);
    }
  }
  v37 = *((DWORD *)v6 + 1);
  *(DWORD *)(v42 + 116868) = v37;
  if ( v37 <= v34 )
  {
    v4 = (short *)pQuest;
    goto LABEL_55;
  }
  v41 = *(WORD *)(16 * v5 + pQuest + 340);
  *(BYTE *)(v42 + 116866) = 5;
  *(WORD *)(v42 + 116864) = v41;
  return 0;
}
#endif

// ── FUN_00401650 (IDA-only, gated) ──
#if defined(IDA_PORT_00401650)
// positive sp value has been detected, the output may be wrong!
bool __cdecl CSQuest::CheckActCondition(DWORD This, DWORD pQuest)
{
  int v2; // ebx
  unsigned char *v4; // esi
  DWORD v5; // ecx
  short *v7; // [esp+8h] [ebp-8h]

  v2 = 0;
  if ( *v7 <= 0 )
  {
    return 1;
  }
  v4 = (unsigned char *)(v7 + 20);
  v5 = -40 - (DWORD)v7;
  while ( 1 )
  {
    if ( v4[v5 + 44 + *(unsigned char *)(This + 4) + (DWORD)v7] != 1 || *(v4 - 1) != 1 )
    {
      goto LABEL_7;
    }
    if ( CSQuest::FindQuestItemsInInven(v4[2], 32 * *v4 + v4[1], v4[2], -1) )
    {
      break;
    }
    v5 = pQuest;
LABEL_7:
    ++v2;
    v4 += 18;
    if ( v2 >= *v7 )
    {
      return 1;
    }
  }
  *(WORD *)(This + 116864) = CSQuest::FindQuestContext(This, (DWORD)v7, 1);
  return 0;
}
#endif

// ── FUN_004016e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004016E0)
BYTE __cdecl CSQuest::getQuestState(DWORD This, int questIndex)
{
  int Index; // eax
  int SubIndex; // ecx
  char v5; // al
  BYTE byCurrState; // al

  if ( questIndex == -1 )
  {
    Index = 0;
  }
  else
  {
    Index = *(unsigned char *)(This + 116858) >> 2;
  }
  SubIndex = *(unsigned char *)(This + 116858) - Index;
  if ( SubIndex )
  {
    v5 = *(BYTE *)(Index + This + 116808) >> (2 * SubIndex);
  }
  else
  {
    v5 = *(BYTE *)(Index + This + 116808);
  }
  byCurrState = v5 & 3;
  if ( questIndex == -1 )
  {
    *(BYTE *)(This + 116866) = byCurrState;
  }
  return byCurrState;
}
#endif

// ── FUN_00401730 (IDA-only, gated) ──
#if defined(IDA_PORT_00401730)
BYTE __cdecl CSQuest::CheckQuestState(DWORD This, BYTE state)
{
  DWORD lpQuest; // edi
  short QuestContext; // ax

  lpQuest = This + 584 * *(unsigned char *)(This + 116858) + 8;
  if ( state == 0xFF )
  {
    CSQuest::getQuestState(This, -1);
  }
  else
  {
    *(BYTE *)(This + 116866) = state;
  }
  if ( *(BYTE *)(This + 116866) == 1 )
  {
    (BYTE)(QuestContext) = CSQuest::CheckActCondition(This, lpQuest);
    if ( (BYTE)QuestContext )
    {
      QuestContext = CSQuest::FindQuestContext(This, lpQuest, 2);
      *(WORD *)(This + 116864) = QuestContext;
      *(BYTE *)(This + 116866) = 1;
    }
  }
  else if ( *(BYTE *)(This + 116866) == 2 )
  {
    QuestContext = CSQuest::FindQuestContext(This, lpQuest, 3);
    *(WORD *)(This + 116864) = QuestContext;
  }
  else
  {
    (BYTE)(QuestContext) = *(BYTE *)(This + 116866) - 3;
    if ( *(BYTE *)(This + 116866) == 3 )
    {
      (BYTE)(QuestContext) = CSQuest::CheckRequestCondition(This, lpQuest, 0);
      if ( (BYTE)QuestContext )
      {
        QuestContext = CSQuest::FindQuestContext(This, lpQuest, 0);
        *(WORD *)(This + 116864) = QuestContext;
      }
    }
  }
  return QuestContext;
}
#endif

// ── FUN_004017e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004017E0)
void __cdecl CSQuest::ShowDialogText(DWORD This, int iDialogIndex)
{
  int v2; // eax
  int i; // esi
  char *v4; // ebp
  int v5; // edi
  int iNumLine; // eax
  char lpszAnswer[72]; // [esp+Ch] [ebp-48h] BYREF

  g_iCurrentDialogScript = iDialogIndex;
  g_iNumLineMessageBoxCustom = SeparateTextIntoLines(
                                 g_DialogScript[iDialogIndex].m_lpszText,
                                 g_lpszMessageBoxCustom[0],
                                 7,
                                 38);
  memset(g_lpszDialogAnswer, 0, sizeof(g_lpszDialogAnswer));
  v2 = g_iCurrentDialogScript;
  i = 0;
  g_iNumAnswer = 0;
  if ( g_DialogScript[g_iCurrentDialogScript].m_iNumAnswer > 0 )
  {
    v4 = g_lpszDialogAnswer[0][0];
    do
    {
      v5 = i + 1;
      wsprintfA(lpszAnswer, "%d) %s", i + 1, g_DialogScript[0].m_lpszAnswer[i + 16 * v2]);
      iNumLine = SeparateTextIntoLines(lpszAnswer, v4, 1, 38);
      if ( iNumLine < 0 )
      {
        g_lpszDialogAnswer[i][iNumLine][0] = 0;
      }
      v2 = g_iCurrentDialogScript;
      ++i;
      ++g_iNumAnswer;
      v4 += 38;
    }
    while ( v5 < g_DialogScript[g_iCurrentDialogScript].m_iNumAnswer );
  }
  if ( !g_DialogScript[v2].m_iNumAnswer )
  {
    wsprintfA(lpszAnswer, "%d) %s", i + 1, GlobalText[609]);
    g_iNumAnswer = 1;
    strcpy(g_lpszDialogAnswer[0][0], lpszAnswer);
  }
  SetErrorMessage(0);
}
#endif

// ── FUN_00401960 (IDA-only, gated) ──
#if defined(IDA_PORT_00401960)
void __cdecl CSQuest::clearQuest(DWORD This)
{
  int v1; // edi
  signed int v2; // ebx
  int v3; // eax
  int v4; // esi
  char buf[4]; // [esp+14h] [ebp-410h] BYREF
  char v6; // [esp+18h] [ebp-40Ch]
  int v7; // [esp+420h] [ebp-4h]

  *(BYTE *)(This + 116863) = 0;
  CloseInventoryRelatedWindows();
  v1 = 0;
  buf[2] = -63;
  v7 = 0;
  v6 = 49;
  buf[3] = 3;
  v2 = 3;
  if ( s != -1 )
  {
    while ( 1 )
    {
      v3 = send(s, &buf[v1 + 2], 3 - v1, 0);
      v4 = v3;
      if ( v3 == -1 )
      {
        break;
      }
      if ( v3 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2(&buf[2], v3);
        }
        v2 -= v4;
        v1 += v4;
        if ( v2 > 0 )
        {
          continue;
        }
      }
      return;
    }
    if ( WSAGetLastError() == 10035 && DAT_055cc16c + 3 <= 0x2000 )
    {
      qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf[2], v2);
      DAT_055cc16c += v2;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
}
#endif

// ── FUN_00401af0 (IDA-only, gated) ──
#if defined(IDA_PORT_00401AF0)
void __cdecl FUN_00401af0(DWORD This)
{
  int v1; // esi
  int v2; // eax
  int v3; // ecx
  int v4; // eax
  char v5; // dl
  int v6; // edx
  int i; // esi
  int v8; // eax
  int v9; // edx
  int v10; // ebx
  int v11; // edi
  int v12; // eax
  unsigned char v13; // cl
  int v14; // eax
  BYTE v15; // al
  int v16; // edi
  int v17; // esi
  char *v18; // ebx
  int v19; // eax
  int v20; // ebp
  int v21; // edi
  signed int v22; // ebx
  int v23; // eax
  int v24; // esi
  int v25; // eax
  signed int v26; // edi
  int v27; // ebx
  signed int v28; // ebp
  int v29; // eax
  int v30; // esi
  char v31; // dl
  int v32; // edx
  int j; // esi
  int v34; // eax
  int v35; // edx
  int v36; // ebx
  int v37; // esi
  unsigned int v38; // eax
  int v39; // eax
  unsigned char v40; // cl
  int v41; // eax
  BYTE v42; // al
  unsigned int v43; // eax
  BYTE *v44; // eax
  char v45; // cl
  int v46; // esi
  int v47; // edi
  char *v48; // ebp
  int v49; // eax
  unsigned int v50; // ebx
  int v51; // edi
  unsigned int v52; // ebp
  int v53; // eax
  int v54; // esi
  char *v55; // edi
  char *v56; // edi
  char *v57; // esi
  char v58; // cl
  unsigned int v59; // esi
  int v60; // ebx
  int v61; // eax
  int v62; // edi
  char *v63; // edi
  int v64; // eax
  int v65; // [esp-8h] [ebp-D60h]
  char v66; // [esp+Ch] [ebp-D4Ch]
  char v67; // [esp+Dh] [ebp-D4Bh]
  char v68; // [esp+Eh] [ebp-D4Ah]
  char v69; // [esp+Fh] [ebp-D49h]
  char v70; // [esp+10h] [ebp-D48h]
  char v71; // [esp+11h] [ebp-D47h]
  char v72; // [esp+12h] [ebp-D46h]
  char v73; // [esp+13h] [ebp-D45h]
  char v74; // [esp+14h] [ebp-D44h]
  char v75; // [esp+15h] [ebp-D43h]
  char v76; // [esp+16h] [ebp-D42h]
  char v77; // [esp+17h] [ebp-D41h]
  char v78; // [esp+18h] [ebp-D40h]
  char v79; // [esp+19h] [ebp-D3Fh]
  char v80; // [esp+1Ah] [ebp-D3Eh]
  char v81; // [esp+1Bh] [ebp-D3Dh]
  char v82; // [esp+1Ch] [ebp-D3Ch]
  char v83; // [esp+1Dh] [ebp-D3Bh]
  char v84; // [esp+1Eh] [ebp-D3Ah]
  char v85; // [esp+1Fh] [ebp-D39h]
  char v86; // [esp+20h] [ebp-D38h]
  char v87; // [esp+21h] [ebp-D37h]
  char v88; // [esp+22h] [ebp-D36h]
  char v89; // [esp+23h] [ebp-D35h]
  char v90; // [esp+24h] [ebp-D34h]
  char v91; // [esp+25h] [ebp-D33h]
  char v92; // [esp+26h] [ebp-D32h]
  char v93; // [esp+27h] [ebp-D31h]
  char v94; // [esp+28h] [ebp-D30h]
  char v95; // [esp+29h] [ebp-D2Fh]
  char v96; // [esp+2Ah] [ebp-D2Eh]
  char v97; // [esp+2Bh] [ebp-D2Dh]
  char v98; // [esp+2Fh] [ebp-D29h]
  DWORD v99; // [esp+30h] [ebp-D28h]
  int v100; // [esp+34h] [ebp-D24h]
  int v101; // [esp+38h] [ebp-D20h]
  void *(__cdecl **v102)(std::locale::facet *__hidden, unsigned int); // [esp+3Ch] [ebp-D1Ch]
  BYTE v103[1025]; // [esp+40h] [ebp-D18h] BYREF
  char buf; // [esp+444h] [ebp-914h] BYREF
  char v105; // [esp+445h] [ebp-913h]
  char v106[258]; // [esp+446h] [ebp-912h] BYREF
  char v107; // [esp+548h] [ebp-810h] BYREF
  char v108; // [esp+549h] [ebp-80Fh]
  char v109; // [esp+54Ah] [ebp-80Eh]
  char v110[1025]; // [esp+54Bh] [ebp-80Dh] BYREF
  char v111[1024]; // [esp+94Ch] [ebp-40Ch] BYREF
  int v112; // [esp+D54h] [ebp-4h]

  v1 = This;
  v99 = This;
  v98 = 0;
  v2 = 18 * (7 - (g_iNumAnswer + g_iNumLineMessageBoxCustom)) / 2 + 18 * g_iNumLineMessageBoxCustom + 66;
  if ( *(BYTE *)(This + 116866) != 1 && *(BYTE *)(This + 116863) == 1 )
  {
    v2 = 250;
  }
  if ( MouseY >= 0 )
  {
    v3 = MouseY - v2;
    if ( MouseY - v2 < 18 * g_iNumAnswer && (int)abs32(566 - MouseX) <= 106 && MouseLButtonPush )
    {
      MouseLButtonPush = 0;
      MouseLButton = 0;
      v100 = v3 / 18;
      MouseUpdateTime = 0;
      MouseUpdateTimeMax = 6;
      if ( v3 / 18 >= 0 )
      {
        v4 = g_DialogScript[g_iCurrentDialogScript].m_iReturnForAnswer[v3 / 18];
        if ( v4 == 1 )
        {
          if ( !CSQuest::CheckRequestCondition(v1, v1 + 584 * *(unsigned char *)(v1 + 116858) + 8, 1) )
          {
            v65 = *(short *)(v1 + 116864);
            v98 = 1;
            CSQuest::ShowDialogText(v1, v65);
            goto LABEL_107;
          }
          v102 = &DAT_00552460;
          v112 = 0;
          *(DWORD *)v103 = 29425667;
          v103[4] = -94;
          v5 = *(BYTE *)(v1 + 116858);
          v66 = -25;
          v67 = 109;
          v68 = 58;
          v103[*(unsigned short *)v103 + 2] = v5;
          v69 = -119;
          v70 = -68;
          v71 = -78;
          v6 = *(unsigned short *)v103 + 1;
          v72 = -97;
          v73 = 115;
          v74 = 35;
          v75 = -88;
          v76 = -2;
          v77 = -74;
          v78 = 73;
          v79 = 93;
          v80 = 57;
          v81 = 93;
          v82 = -118;
          v83 = -53;
          v84 = 99;
          v85 = -115;
          v86 = -22;
          v87 = 125;
          v88 = 43;
          v89 = 95;
          v90 = -61;
          v91 = -79;
          v92 = -23;
          v93 = -125;
          v94 = 41;
          v95 = 81;
          v96 = -24;
          v97 = 86;
          for ( i = *(unsigned short *)v103; i != v6; ++i )
          {
            v103[i + 2] ^= v103[i + 1] ^ *(&v66 + i % 32);
          }
          ++*(WORD *)v103;
          if ( *(unsigned short *)v103 + 1 <= 1024 )
          {
            v66 = -25;
            v67 = 109;
            v68 = 58;
            v103[*(unsigned short *)v103 + 2] = 1;
            v8 = *(unsigned short *)v103;
            v69 = -119;
            v70 = -68;
            v71 = -78;
            v9 = *(unsigned short *)v103 + 1;
            v72 = -97;
            v73 = 115;
            v74 = 35;
            v75 = -88;
            v76 = -2;
            v77 = -74;
            v78 = 73;
            v79 = 93;
            v80 = 57;
            v81 = 93;
            v82 = -118;
            v83 = -53;
            v84 = 99;
            v85 = -115;
            v86 = -22;
            v87 = 125;
            v88 = 43;
            v89 = 95;
            v90 = -61;
            v91 = -79;
            v92 = -23;
            v93 = -125;
            v94 = 41;
            v95 = 81;
            v96 = -24;
            v97 = 86;
            if ( *(unsigned short *)v103 != v9 )
            {
              do
              {
                v103[v8 + 2] ^= v103[v8 + 1] ^ *(&v66 + v8 % 32);
                ++v8;
              }
              while ( v8 != v9 );
            }
            ++*(WORD *)v103;
          }
          if ( v103[2] == 193 )
          {
            v103[3] = v103[0];
          }
          else if ( v103[2] == 194 )
          {
            *(WORD *)&v103[3] = *(WORD *)v103;
          }
          v10 = *(unsigned short *)v103;
          qmemcpy(v111, &v103[2], *(unsigned short *)v103);
          v111[v10] = rand();
          v11 = (v111[0] != -63) + 2;
          if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
          {
            v14 = operator_new(2u);
            *(BYTE *)(v14 + 1) = 1;
            FUN_00403f80(&DAT_055c9bc8, v14, (int)&g_byPacketSerialSend);
          }
          else
          {
            v12 = FUN_00404280(&DAT_055c9bc8, &g_byPacketSerialSend);
            v13 = *(BYTE *)(v12 + 1) + 1;
            *(BYTE *)(v12 + 1) = v13;
            if ( v13 < 2u )
            {
              FUN_00404330(&g_byPacketSerialSend, v12);
            }
          }
          v15 = g_byPacketSerialSend;
          v110[v11 + 1024] = g_byPacketSerialSend;
          g_byPacketSerialSend = v15 + 1;
          PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
          v16 = v11 - 1;
          v17 = v10 - v16;
          v18 = &v111[v16];
          v19 = FUN_0053cc30(0, (int)&v111[v16], v17);
          if ( v19 >= 256 )
          {
            v26 = v19 + 3;
            v107 = -60;
            v109 = v19 + 3;
            v108 = (v19 + 3) / 256;
            FUN_0053cc30((int)v110, (int)v18, v17);
            v27 = 0;
            v28 = v26;
            if ( s != -1 )
            {
              while ( 1 )
              {
                v29 = send(s, &v107 + v27, v26 - v27, 0);
                v30 = v29;
                if ( v29 == -1 )
                {
                  break;
                }
                if ( v29 )
                {
                  if ( DAT_055ce174 )
                  {
                    nullsub_2(&v107, v29);
                  }
                  v28 -= v30;
                  v27 += v30;
                  if ( v28 > 0 )
                  {
                    continue;
                  }
                }
                goto LABEL_52;
              }
              if ( WSAGetLastError() == 10035 && DAT_055cc16c + v26 <= 0x2000 )
              {
                qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v107, v28);
                v25 = v28 + DAT_055cc16c;
                goto LABEL_51;
              }
LABEL_49:
              CWsctlc::Close((DWORD)&SocketClient);
              v112 = -1;
LABEL_106:
              v1 = v99;
              goto LABEL_107;
            }
          }
          else
          {
            v101 = v19 + 2;
            buf = -61;
            v105 = v19 + 2;
            FUN_0053cc30((int)v106, (int)&v111[v16], v17);
            v20 = v101;
            v21 = 0;
            v22 = v101;
            if ( s != -1 )
            {
              while ( 1 )
              {
                v23 = send(s, &buf + v21, v20 - v21, 0);
                v24 = v23;
                if ( v23 == -1 )
                {
                  break;
                }
                if ( v23 )
                {
                  if ( DAT_055ce174 )
                  {
                    nullsub_2(&buf, v23);
                  }
                  v22 -= v24;
                  v21 += v24;
                  if ( v22 > 0 )
                  {
                    continue;
                  }
                }
                goto LABEL_52;
              }
              if ( WSAGetLastError() == 10035 && DAT_055cc16c + v20 <= 0x2000 )
              {
                qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf, v22);
                v25 = v22 + DAT_055cc16c;
LABEL_51:
                DAT_055cc16c = v25;
                goto LABEL_52;
              }
              goto LABEL_49;
            }
          }
LABEL_52:
          v112 = -1;
          goto LABEL_106;
        }
        if ( v4 == 2 )
        {
          MouseUpdateTimeMax = 6;
          MouseLButtonPush = 0;
          MouseUpdateTime = 0;
          CSQuest::clearQuest(v1);
          goto LABEL_107;
        }
        if ( v4 != 3 )
        {
LABEL_107:
          PlayBuffer(28, 0, 0);
          v64 = g_DialogScript[g_iCurrentDialogScript].m_iLinkForAnswer[v100];
          if ( v64 > 0 && !v98 )
          {
            CSQuest::ShowDialogText(v1, v64);
          }
          return;
        }
        MouseLButtonPush = 0;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        v102 = &DAT_00552460;
        v112 = 1;
        *(DWORD *)v103 = 29425667;
        v103[4] = -94;
        v31 = *(BYTE *)(v1 + 116858);
        v66 = -25;
        v67 = 109;
        v68 = 58;
        v103[*(unsigned short *)v103 + 2] = v31;
        v69 = -119;
        v70 = -68;
        v71 = -78;
        v32 = *(unsigned short *)v103 + 1;
        v72 = -97;
        v73 = 115;
        v74 = 35;
        v75 = -88;
        v76 = -2;
        v77 = -74;
        v78 = 73;
        v79 = 93;
        v80 = 57;
        v81 = 93;
        v82 = -118;
        v83 = -53;
        v84 = 99;
        v85 = -115;
        v86 = -22;
        v87 = 125;
        v88 = 43;
        v89 = 95;
        v90 = -61;
        v91 = -79;
        v92 = -23;
        v93 = -125;
        v94 = 41;
        v95 = 81;
        v96 = -24;
        v97 = 86;
        for ( j = *(unsigned short *)v103; j != v32; ++j )
        {
          v103[j + 2] ^= v103[j + 1] ^ *(&v66 + j % 32);
        }
        ++*(WORD *)v103;
        if ( *(unsigned short *)v103 + 1 <= 1024 )
        {
          v66 = -25;
          v67 = 109;
          v68 = 58;
          v103[*(unsigned short *)v103 + 2] = 1;
          v34 = *(unsigned short *)v103;
          v69 = -119;
          v70 = -68;
          v71 = -78;
          v35 = *(unsigned short *)v103 + 1;
          v72 = -97;
          v73 = 115;
          v74 = 35;
          v75 = -88;
          v76 = -2;
          v77 = -74;
          v78 = 73;
          v79 = 93;
          v80 = 57;
          v81 = 93;
          v82 = -118;
          v83 = -53;
          v84 = 99;
          v85 = -115;
          v86 = -22;
          v87 = 125;
          v88 = 43;
          v89 = 95;
          v90 = -61;
          v91 = -79;
          v92 = -23;
          v93 = -125;
          v94 = 41;
          v95 = 81;
          v96 = -24;
          v97 = 86;
          if ( *(unsigned short *)v103 != v35 )
          {
            do
            {
              v103[v34 + 2] ^= v103[v34 + 1] ^ *(&v66 + v34 % 32);
              ++v34;
            }
            while ( v34 != v35 );
          }
          ++*(WORD *)v103;
        }
        if ( v103[2] == 193 )
        {
          v103[3] = v103[0];
        }
        else if ( v103[2] == 194 )
        {
          *(WORD *)&v103[3] = *(WORD *)v103;
        }
        v36 = *(unsigned short *)v103;
        qmemcpy(v111, &v103[2], *(unsigned short *)v103);
        v111[v36] = rand();
        v37 = (v111[0] != -63) + 2;
        if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
        {
          v41 = operator_new(2u);
          *(BYTE *)(v41 + 1) = 1;
          FUN_00403f80(&DAT_055c9bc8, v41, (int)&g_byPacketSerialSend);
        }
        else
        {
          v38 = FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
          if ( v38 == -1 )
          {
            v39 = 0;
          }
          else
          {
            v39 = *(DWORD *)(DAT_055c9bcc + 4 * v38);
          }
          v40 = *(BYTE *)(v39 + 1) + 1;
          *(BYTE *)(v39 + 1) = v40;
          if ( v40 < 2u )
          {
            FUN_00404330(&g_byPacketSerialSend, v39);
          }
        }
        v42 = g_byPacketSerialSend;
        v110[v37 + 1024] = g_byPacketSerialSend;
        g_byPacketSerialSend = v42 + 1;
        if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) != -1 )
        {
          v43 = FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
          v44 = v43 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v43);
          v45 = v44[1] - 1;
          v44[1] = v45;
          if ( !v45 )
          {
            FUN_00423710(v44, &g_byPacketSerialSend);
          }
        }
        v46 = v37 - 1;
        v47 = v36 - v46;
        v48 = &v111[v46];
        v49 = FUN_0053cc30(0, (int)&v111[v46], v36 - v46);
        if ( v49 >= 256 )
        {
          v59 = v49 + 3;
          v107 = -60;
          v109 = v49 + 3;
          v108 = (v49 + 3) / 256;
          FUN_0053cc30((int)v110, (int)v48, v47);
          v60 = 0;
          v52 = v59;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v61 = send(s, &v107 + v60, v59 - v60, 0);
              v62 = v61;
              if ( v61 == -1 )
              {
                break;
              }
              if ( v61 )
              {
                if ( DAT_055ce174 )
                {
                  nullsub_2(&v107, v61);
                }
                v52 -= v62;
                v60 += v62;
                if ( (int)v52 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_105;
            }
            if ( WSAGetLastError() == 10035 && (int)(DAT_055cc16c + v59) <= 0x2000 )
            {
              v63 = (char *)&DAT_055ca16c + DAT_055cc16c;
              qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v107, 4 * (v52 >> 2));
              v57 = &v107 + 4 * (v52 >> 2);
              v56 = &v63[4 * (v52 >> 2)];
              v58 = v52;
              goto LABEL_104;
            }
            goto LABEL_102;
          }
        }
        else
        {
          v50 = v49 + 2;
          buf = -61;
          v105 = v49 + 2;
          FUN_0053cc30((int)v106, (int)&v111[v46], v47);
          v51 = 0;
          v52 = v50;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v53 = send(s, &buf + v51, v50 - v51, 0);
              v54 = v53;
              if ( v53 == -1 )
              {
                break;
              }
              if ( v53 )
              {
                if ( DAT_055ce174 )
                {
                  nullsub_2(&buf, v53);
                }
                v52 -= v54;
                v51 += v54;
                if ( (int)v52 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_105;
            }
            if ( WSAGetLastError() == 10035 && (int)(DAT_055cc16c + v50) <= 0x2000 )
            {
              v55 = (char *)&DAT_055ca16c + DAT_055cc16c;
              qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf, 4 * (v52 >> 2));
              v57 = &buf + 4 * (v52 >> 2);
              v56 = &v55[4 * (v52 >> 2)];
              v58 = v52;
LABEL_104:
              qmemcpy(v56, v57, v58 & 3);
              DAT_055cc16c += v52;
              goto LABEL_105;
            }
LABEL_102:
            CWsctlc::Close((DWORD)&SocketClient);
          }
        }
LABEL_105:
        v112 = -1;
        v102 = &DAT_00552460;
        goto LABEL_106;
      }
    }
  }
}
#endif

// ── FUN_00402ff0 (IDA-only, gated) ──
#if defined(IDA_PORT_00402FF0)
void __cdecl FUN_00402ff0(BYTE *_this)
{
  int v1; // edi
  int v3; // esi
  char (*v4)[38]; // ebp
  int v5; // ebx
  const char *v6; // edi
  int i; // ebp
  char (*v8)[1][38]; // [esp+10h] [ebp-8h]
  int v9; // [esp+14h] [ebp-4h]

  m_dwTextColor = -2955521;
  v1 = 0;
  m_dwBackColor = 0;
  EnableAlphaTest(1);
  v3 = 18 * (7 - g_iNumAnswer - g_iNumLineMessageBoxCustom) / 2 + 66;
  if ( g_iNumLineMessageBoxCustom > 0 )
  {
    v4 = g_lpszMessageBoxCustom;
    do
    {
      RenderCenteredText(550, v3, (const char *)v4);
      v3 += 18;
      ++v1;
      ++v4;
    }
    while ( v1 < g_iNumLineMessageBoxCustom );
  }
  if ( _this[116866] != 1 && _this[116863] == 1 )
  {
    v3 = 250;
  }
  v9 = (MouseY - v3) / 18;
  SelectObject(m_hFontDC, g_hFontBold);
  v5 = 0;
  if ( g_iNumAnswer > 0 )
  {
    v6 = g_lpszDialogAnswer[0][0];
    v8 = g_lpszDialogAnswer;
    do
    {
      if ( v9 != v5 || (m_dwTextColor = -16776961, (int)abs32(556 - MouseX) > 106) )
      {
        m_dwTextColor = -9977889;
      }
      for ( i = 0; i < 1; ++i )
      {
        if ( !*v6 )
        {
          break;
        }
        RenderCenteredText(550, v3, v6);
        v3 += 18;
        v6 += 38;
      }
      ++v5;
      v6 = (*v8++)[1];
    }
    while ( v5 < g_iNumAnswer );
  }
  glColor3f(1.0, 1.0, 1.0);
}
#endif

// ── FUN_00403310 (IDA-only, gated) ──
#if defined(IDA_PORT_00403310)
void __cdecl FUN_00403310(DWORD *_this)
{
  *_this = &DAT_00552460;
}
#endif

// ── FUN_00403320 (IDA-only, gated) ──
#if defined(IDA_PORT_00403320)
void __cdecl FUN_00403320(DWORD This)
{
  char v2; // al
  double v3; // st7
  double v4; // st7
  double v5; // st7
  double v6; // st7
  char *MonsterName; // eax
  int v8; // [esp-Ch] [ebp-80h]
  char Buffer[100]; // [esp+10h] [ebp-64h] BYREF

  glColor3f(1.0, 1.0, 1.0);
  EnableAlphaTest(1);
  RenderInventoryInterface(450, 0, 1);
  m_dwTextColor = -2955521;
  m_dwBackColor = 0;
  v2 = *(BYTE *)(This + 116866);
  if ( v2 == 1 )
  {
    RenderBitmap(279, 450.0, 325.0, 190.0, 10.0, 0.0, 0.0, 0.7421875, 0.625, 1, 1);
    if ( FUN_00403150(This, *(BYTE *)(This + 116866), 1) )
    {
      m_dwTextColor = -2955521;
    }
    else
    {
      glColor3f(0.30000001, 0.30000001, 0.30000001);
    }
    v3 = (double)MouseX;
    if ( v3 >= 485.0 && v3 < 605.0 )
    {
      v4 = (double)MouseY;
      if ( v4 >= 355.0 && v4 < 379.0 && MouseLButtonPush )
      {
        glColor3f(0.40000001, 0.40000001, 0.40000001);
        if ( MouseLButtonPop )
        {
          MouseLButtonPush = 0;
          MouseLButton = 0;
        }
      }
    }
    SelectObject(m_hFontDC, g_hFont);
    RenderBitmap(240, 485.0, 355.0, 120.0, 24.0, 0.0, 0.0, 0.83203125, 1.0, 1, 1);
    RenderCenteredText(545, 360, GlobalText[699]);
    glColor3f(1.0, 1.0, 1.0);
  }
  else if ( v2 == 3 )
  {
    RenderBitmap(271, 500.0, 367.70001, 113.0, 18.0, 0.0, 0.0, 0.8828125, 0.5625, 1, 1);
    m_dwBackColor = -14145496;
    m_dwTextColor = -6890241;
    RenderText(470, 370, GlobalText[198], 0, 0, 0);
    m_dwTextColor = FUN_004c3dd0(*(DWORD *)(This + 116868));
    FUN_004c3e10(*(DWORD *)(This + 116868), Buffer);
    RenderText(510, 370, Buffer, 0, 0, 0);
  }
  RenderBitmap(280, 475.0, 395.0, 24.0, 24.0, 0.0, 0.0, 0.75, 0.75, 1, 1);
  v5 = (double)MouseX;
  if ( v5 >= 475.0 && v5 < 499.0 )
  {
    v6 = (double)MouseY;
    if ( v6 >= 395.0 && v6 < 419.0 )
    {
      SelectObject(m_hFontDC, g_hFont);
      m_dwTextColor = -1;
      m_dwBackColor = -16777216;
      RenderTipText(475, 382, GlobalText[225]);
    }
  }
  m_dwBackColor = 0;
  SelectObject(m_hFontDC, g_hFont);
  m_dwTextColor = -983146;
  v8 = 120 * WindowWidth / 0x280;
  MonsterName = getMonsterName(*(unsigned char *)(This + 584 * *(unsigned char *)(This + 116858) + 12));
  RenderText(485, 12, MonsterName, v8, 1, (SIZE *)3);
  m_dwTextColor = -9016;
  RenderText(
    472,
    22,
    (const char *)(This + 584 * *(unsigned char *)(This + 116858) + 13),
    150 * WindowWidth / 0x280,
    1,
    0);
  FUN_00402ff0((BYTE *)This);
  glColor3f(1.0, 1.0, 1.0);
}
#endif

// ── FUN_00403700 (IDA-only, gated) ──
#if defined(IDA_PORT_00403700)
void __cdecl FUN_00403700(int _this, int a2)
{
  int v3; // ebx
  int v4; // edi
  double v5; // st7
  double v6; // st7
  const char *v7; // edi
  double v8; // st7
  double v9; // st7
  double v10; // st6
  int x; // [esp+10h] [ebp-8h]
  float xa; // [esp+10h] [ebp-8h]
  int xb; // [esp+10h] [ebp-8h]
  float y; // [esp+14h] [ebp-4h]

  m_dwTextColor = -1;
  m_dwBackColor = -16777216;
  glColor3f(1.0, 1.0, 1.0);
  EnableAlphaTest(1);
  v3 = 0;
  RenderBitmap(240, 465.0, 204.0, 55.0, 22.0, 0.0, 0.0, 0.83203125, 1.0, 1, 1);
  RenderCenteredText(492, 211, aAiau);
  RenderBitmap(240, 520.0, 204.0, 55.0, 22.0, 0.0, 0.0, 0.83203125, 1.0, 1, 1);
  if ( a2 == 1 )
  {
    RenderCenteredText(547, 211, "Áß´Ü");
  }
  else if ( a2 == 2 )
  {
    RenderCenteredText(547, 211, aAau);
  }
  v4 = 0;
  x = 0;
  do
  {
    v5 = (double)MouseX;
    xa = (double)x + 465.0;
    if ( v5 >= xa && v5 < xa + 55.0 )
    {
      v6 = (double)MouseY;
      if ( v6 >= 204.0 && v6 < 226.0 )
      {
        glColor3f(0.80000001, 0.60000002, 0.40000001);
        EnableAlphaBlend();
        RenderBitmap(240, xa, 204.0, 55.0, 22.0, 0.0, 0.0, 0.83203125, 1.0, 1, 1);
        glColor3f(1.0, 1.0, 1.0);
        DisableAlphaBlend();
      }
    }
    v4 += 55;
    x = v4;
  }
  while ( v4 < 110 );
  EnableAlphaTest(1);
  xb = 0;
  v7 = (const char *)(_this + 13);
  do
  {
    if ( CSQuest::getQuestState(_this, v3) == a2 && strcmp(v7, &strID) )
    {
      v8 = (double)MouseX;
      if ( v8 < 477.0 || v8 >= 572.0 || (v9 = (double)MouseY, v10 = (double)xb + 267.0, v9 < v10) || v9 >= v10 + 10.0 )
      {
        m_dwBackColor = 0;
      }
      else
      {
        m_dwBackColor = -2146825473;
        if ( MouseLButtonPush )
        {
          *(BYTE *)(_this + 116858) = v3;
          CSQuest::CheckQuestState(_this, 1u);
          CSQuest::ShowDialogText(_this, *(short *)(_this + 116864));
        }
      }
      y = (double)xb + 267.0;
      RenderText(477, (__int64)y, v7, 0, 0, 0);
      if ( *(unsigned char *)(_this + 116858) == v3 )
      {
        RenderBitmap(9, 467.0, y, 12.0, 12.0, 0.0, 0.45833334, 1.0, 1.0, 1, 1);
      }
      xb += 10;
    }
    ++v3;
    v7 += 584;
  }
  while ( v3 < 200 );
}
#endif

// ── FUN_00403a40 (IDA-only, gated) ──
#if defined(IDA_PORT_00403A40)
void __cdecl FUN_00403a40(unsigned char *This)
{
  int v2; // ebp
  int v3; // ebx
  double v4; // st7
  double v5; // st7
  double v6; // st7
  double v7; // st7
  int x; // [esp+10h] [ebp-4h]
  float xa; // [esp+10h] [ebp-4h]

  glColor3f(1.0, 1.0, 1.0);
  EnableAlphaTest(1);
  v2 = 0;
  RenderInventoryInterface(450, 0, 1);
  m_dwTextColor = -2955521;
  m_dwBackColor = 0;
  SelectObject(m_hFontDC, g_hFont);
  RenderBitmap(279, 450.0, 250.0, 190.0, 10.0, 0.0, 0.0, 0.7421875, 0.625, 1, 1);
  v3 = 0;
  x = 0;
  do
  {
    xa = (double)x + 465.0;
    RenderBitmap(277, xa, 234.0, 55.0, 17.0, 0.0, 0.0, 1.0, 0.94444442, 1, 1);
    v4 = (double)MouseX;
    if ( v4 >= xa && v4 < xa + 55.0 )
    {
      v5 = (double)MouseY;
      if ( v5 >= 234.0 && v5 < 251.0 && MouseLButtonPush )
      {
        MouseLButtonPop = 0;
        This[116861] = v3;
        RenderBitmap(278, xa, 234.0, 55.0, 17.0, 0.0, 0.0, 1.0, 0.94444442, 1, 1);
      }
    }
    if ( This[116861] == v3 )
    {
      m_dwTextColor = -983146;
      RenderBitmap(278, xa, 234.0, 55.0, 17.0, 0.0, 0.0, 1.0, 0.94444442, 1, 1);
    }
    else
    {
      m_dwTextColor = -2955521;
    }
    RenderCenteredText((__int64)(xa + 27.0), 241, aAu_0);
    v2 += 55;
    ++v3;
    x = v2;
  }
  while ( v2 < 165 );
  RenderBitmap(279, 460.0, 260.0, 170.0, 5.0, 0.0390625, 0.3125, 0.6640625, 0.3125, 1, 1);
  RenderBitmap(279, 460.0, 380.0, 170.0, 6.0, 0.0390625, 0.0, 0.6640625, 0.375, 1, 1);
  RenderBitmap(260, 460.0, 260.0, 1.0, 125.0, 0.00390625, 0.0, 0.00390625, 0.48828125, 1, 1);
  RenderBitmap(260, 630.0, 260.0, 1.0, 125.0, 0.00390625, 0.0, 0.00390625, 0.48828125, 1, 1);
  if ( This[116861] )
  {
    if ( This[116861] == 1 )
    {
      FUN_00403700((int)This, 2);
    }
    else if ( This[116861] == 2 )
    {
      CWsctlc::LogPrintOn((DWORD)This);
    }
  }
  else
  {
    FUN_00403700((int)This, 1);
  }
  RenderBitmap(280, 475.0, 395.0, 24.0, 24.0, 0.0, 0.0, 0.75, 0.75, 1, 1);
  v6 = (double)MouseX;
  if ( v6 >= 475.0 && v6 < 499.0 )
  {
    v7 = (double)MouseY;
    if ( v7 >= 395.0 && v7 < 419.0 )
    {
      SelectObject(m_hFontDC, g_hFont);
      m_dwTextColor = -1;
      m_dwBackColor = -16777216;
      RenderTipText(475, 382, GlobalText[225]);
    }
  }
  m_dwBackColor = -15461356;
  m_dwTextColor = -1644826;
  SelectObject(m_hFontDC, g_hFontBold);
  RenderText(485, 12, "Quest", 120 * WindowWidth / 0x280, 1, (SIZE *)3);
  m_dwTextColor = -9016;
  RenderText(472, 22, (const char *)&This[584 * This[116858] + 13], 150 * WindowWidth / 0x280, 1, 0);
  FUN_00402ff0(This);
  glColor3f(1.0, 1.0, 1.0);
}
#endif

// ── FUN_00403ea0 (IDA-only, gated) ──
#if defined(IDA_PORT_00403EA0)
void __cdecl FUN_00403a40(unsigned char *This)
{
  int v2; // ebp
  int v3; // ebx
  double v4; // st7
  double v5; // st7
  double v6; // st7
  double v7; // st7
  int x; // [esp+10h] [ebp-4h]
  float xa; // [esp+10h] [ebp-4h]

  glColor3f(1.0, 1.0, 1.0);
  EnableAlphaTest(1);
  v2 = 0;
  RenderInventoryInterface(450, 0, 1);
  m_dwTextColor = -2955521;
  m_dwBackColor = 0;
  SelectObject(m_hFontDC, g_hFont);
  RenderBitmap(279, 450.0, 250.0, 190.0, 10.0, 0.0, 0.0, 0.7421875, 0.625, 1, 1);
  v3 = 0;
  x = 0;
  do
  {
    xa = (double)x + 465.0;
    RenderBitmap(277, xa, 234.0, 55.0, 17.0, 0.0, 0.0, 1.0, 0.94444442, 1, 1);
    v4 = (double)MouseX;
    if ( v4 >= xa && v4 < xa + 55.0 )
    {
      v5 = (double)MouseY;
      if ( v5 >= 234.0 && v5 < 251.0 && MouseLButtonPush )
      {
        MouseLButtonPop = 0;
        This[116861] = v3;
        RenderBitmap(278, xa, 234.0, 55.0, 17.0, 0.0, 0.0, 1.0, 0.94444442, 1, 1);
      }
    }
    if ( This[116861] == v3 )
    {
      m_dwTextColor = -983146;
      RenderBitmap(278, xa, 234.0, 55.0, 17.0, 0.0, 0.0, 1.0, 0.94444442, 1, 1);
    }
    else
    {
      m_dwTextColor = -2955521;
    }
    RenderCenteredText((__int64)(xa + 27.0), 241, aAu_0);
    v2 += 55;
    ++v3;
    x = v2;
  }
  while ( v2 < 165 );
  RenderBitmap(279, 460.0, 260.0, 170.0, 5.0, 0.0390625, 0.3125, 0.6640625, 0.3125, 1, 1);
  RenderBitmap(279, 460.0, 380.0, 170.0, 6.0, 0.0390625, 0.0, 0.6640625, 0.375, 1, 1);
  RenderBitmap(260, 460.0, 260.0, 1.0, 125.0, 0.00390625, 0.0, 0.00390625, 0.48828125, 1, 1);
  RenderBitmap(260, 630.0, 260.0, 1.0, 125.0, 0.00390625, 0.0, 0.00390625, 0.48828125, 1, 1);
  if ( This[116861] )
  {
    if ( This[116861] == 1 )
    {
      FUN_00403700((int)This, 2);
    }
    else if ( This[116861] == 2 )
    {
      CWsctlc::LogPrintOn((DWORD)This);
    }
  }
  else
  {
    FUN_00403700((int)This, 1);
  }
  RenderBitmap(280, 475.0, 395.0, 24.0, 24.0, 0.0, 0.0, 0.75, 0.75, 1, 1);
  v6 = (double)MouseX;
  if ( v6 >= 475.0 && v6 < 499.0 )
  {
    v7 = (double)MouseY;
    if ( v7 >= 395.0 && v7 < 419.0 )
    {
      SelectObject(m_hFontDC, g_hFont);
      m_dwTextColor = -1;
      m_dwBackColor = -16777216;
      RenderTipText(475, 382, GlobalText[225]);
    }
  }
  m_dwBackColor = -15461356;
  m_dwTextColor = -1644826;
  SelectObject(m_hFontDC, g_hFontBold);
  RenderText(485, 12, "Quest", 120 * WindowWidth / 0x280, 1, (SIZE *)3);
  m_dwTextColor = -9016;
  RenderText(472, 22, (const char *)&This[584 * This[116858] + 13], 150 * WindowWidth / 0x280, 1, 0);
  FUN_00402ff0(This);
  glColor3f(1.0, 1.0, 1.0);
}
#endif

// ── FUN_00403ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00403EF0)
void __cdecl FUN_00403ef0(DWORD *_this)
{
  *_this = &DAT_005524b8;
  g_csQuest = 0;
}
#endif

// ── FUN_00403f10 (IDA-only, gated) ──
#if defined(IDA_PORT_00403F10)
DWORD *__cdecl FUN_00403f10(DWORD *lpMem, char a2)
{
  FUN_00403ef0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_004041e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004041E0)
unsigned int __cdecl FUN_004041e0(DWORD *_this, int a2)
{
  unsigned int result; // eax
  unsigned int v4; // ebp
  bool v5; // cf
  int v6; // [esp+10h] [ebp-Ch]
  int v7; // [esp+14h] [ebp-8h]
  int v8; // [esp+18h] [ebp-4h] BYREF

  result = (*(int (__cdecl **)(DWORD *, int))(*_this + 12))(_this, a2);
  v4 = _this[3];
  v8 = 0;
  v6 = 0;
  if ( v4 )
  {
    v7 = _this[2];
    while ( memcmp((const char *)&v8, (const char *)(v7 + 4 * result), 4) )
    {
      if ( !memcmp((const char *)&a2, (const char *)(v7 + 4 * result), 4) )
      {
        return result;
      }
      v5 = ++v6 < v4;
      result = (result + 1) % v4;
      if ( !v5 )
      {
        goto LABEL_6;
      }
    }
  }
  else
  {
LABEL_6:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  return -1;
}
#endif

// ── FUN_004042b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004042B0)
int __cdecl FUN_00404280(int *_this, int a2)
{
  int v3; // edx
  int v4; // eax
  int v5; // ecx
  unsigned int v6; // edx
  unsigned int v7; // ecx
  int v9; // [esp+10h] [ebp-8h] BYREF
  int v10; // [esp+14h] [ebp-4h] BYREF
  int v11; // [esp+1Ch] [ebp+4h]

  v3 = *_this;
  v10 = a2;
  v4 = (*(int (__cdecl **)(int *, int))(v3 + 12))(_this, a2);
  v5 = _this[3];
  v6 = v4;
  v9 = 0;
  v11 = 0;
  if ( v5 )
  {
    while ( memcmp((const char *)&v9, (const char *)(_this[2] + 4 * v6), 4) )
    {
      if ( !memcmp((const char *)&v10, (const char *)(_this[2] + 4 * v6), 4) )
      {
        if ( v6 == -1 )
        {
          return 0;
        }
        return *(DWORD *)(_this[1] + 4 * v6);
      }
      v7 = _this[3];
      v6 = (v6 + 1) % v7;
      if ( ++v11 >= v7 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  return 0;
}
#endif

// ── FUN_00404e40 (IDA-only, gated) ──
#if defined(IDA_PORT_00404E40)
DWORD __cdecl waveIO::_waveIO(DWORD This, bool IO)
{
  waveIO::CloseWaveFile(This);
  if ( IO )
  {
    delete__((LPVOID)This);
  }
  return This;
}
#endif

// ── FUN_00404e60 (IDA-only, gated) ──
#if defined(IDA_PORT_00404E60)
bool __cdecl waveIO::CloseWaveFile(DWORD This)
{
  HMMIO v1; // eax

  *(DWORD *)This = DAT_005524c0;
  v1 = *(HMMIO *)(This + 4);
  if ( v1 )
  {
    mmioClose(v1, 0);
  }
  return 1;
}
#endif

// ── FUN_00405240 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_00405240(void *lpBuffer, int iSize, int iKey)
{
  int iConvertSize; // esi
  int v4; // edi
  BYTE *v5; // ebx
  unsigned int v7; // esi

  iConvertSize = (16 - iKey) % 16;
  if ( iConvertSize >= iSize )
  {
    iConvertSize = iSize;
  }
  Xor_ConvertBlock((BYTE *)lpBuffer, iConvertSize, iKey);
  v4 = iSize - iConvertSize;
  v5 = (BYTE *)lpBuffer + iConvertSize;
  if ( iSize - iConvertSize <= 0 )
  {
    return iConvertSize + iKey;
  }
  if ( v4 >= 16 )
  {
    v7 = (unsigned int)v4 >> 4;
    v4 += -16 * ((unsigned int)v4 >> 4);
    do
    {
      Xor_ConvertBlock(v5, 16, 0);
      v5 += 16;
      --v7;
    }
    while ( v7 );
  }
  Xor_ConvertBlock(v5, v4, 0);
  return v4;
}

// ── FUN_00405260 (IDA-only, gated) ──
#if defined(IDA_PORT_00405260)
LPVOID __cdecl FUN_00405260(LPVOID lpMem, char a2)
{
  FUN_00405280();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00405290 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_00405290(int _this)
{
  int result; // eax

  result = 0;
  *(DWORD *)(_this + 4) = -1;
  *(BYTE *)(_this + 8) = 0;
  *(DWORD *)(_this + 268) = 0;
  return result;
}

// ── FUN_004052b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004052B0)
DWORD __cdecl FUN_004052b0(int _this, const char *a2)
{
  strcpy((char *)(_this + 8), a2);
  *(DWORD *)(_this + 268) = 0;
  *(DWORD *)(_this + 4) = CreateFileA((LPCSTR)(_this + 8), 0xC0000000, 1u, 0, 4u, 0x80u, 0);
  FUN_00405340(_this);
  return SetFilePointer(*(HANDLE *)(_this + 4), 0, 0, 2u);
}
#endif

// ── FUN_00405320 (IDA-only, gated) ──
#if defined(IDA_PORT_00405320)
DWORD __cdecl FUN_004052b0(int _this, const char *a2)
{
  strcpy((char *)(_this + 8), a2);
  *(DWORD *)(_this + 268) = 0;
  *(DWORD *)(_this + 4) = CreateFileA((LPCSTR)(_this + 8), 0xC0000000, 1u, 0, 4u, 0x80u, 0);
  FUN_00405340(_this);
  return SetFilePointer(*(HANDLE *)(_this + 4), 0, 0, 2u);
}
#endif

// ── FUN_00405340 (IDA-only, gated) ──
#if defined(IDA_PORT_00405340)
char *__cdecl FUN_00405340(DWORD _this)
{
  char *v2; // edi
  char *result; // eax
  HANDLE FileA; // eax
  DWORD v5; // ecx
  int v6; // [esp-4h] [ebp-20010h]
  DWORD NumberOfBytesRead; // [esp+8h] [ebp-20004h] BYREF
  char Buffer[32767]; // [esp+Ch] [ebp-20000h] BYREF
  char v9; // [esp+800Bh] [ebp-18001h] BYREF

  ReadFile(*(HANDLE *)(_this + 4), Buffer, 0x1FFFFu, &NumberOfBytesRead, 0);
  *(DWORD *)(_this + 268) = Xor_ConvertBuffer(Buffer, NumberOfBytesRead, 0);
  v6 = NumberOfBytesRead;
  Buffer[NumberOfBytesRead] = 0;
  v2 = (char *)FUN_00405420(Buffer, v6);
  if ( NumberOfBytesRead < 0x7FFF )
  {
    result = Buffer;
    if ( v2 == Buffer )
    {
      return result;
    }
  }
  else
  {
    v2 = &v9;
  }
  CloseHandle(*(HANDLE *)(_this + 4));
  DeleteFileA((LPCSTR)(_this + 8));
  FileA = CreateFileA((LPCSTR)(_this + 8), 0xC0000000, 1u, 0, 4u, 0x80u, 0);
  v5 = NumberOfBytesRead - (DWORD)v2;
  *(DWORD *)(_this + 4) = FileA;
  *(DWORD *)(_this + 268) = 0;
  return (char *)CErrorReport::WriteFile(_this, FileA, v2, (DWORD)&Buffer[v5], &NumberOfBytesRead, 0);
}
#endif

// ── FUN_00405420 (IDA-only, gated) ──
#if defined(IDA_PORT_00405420)
char *__stdcall FUN_00405420(char *Str, int a2)
{
  char *result; // eax
  int v3; // ebp
  char *v4; // esi
  size_t v5; // edi
  char *v6; // ebx
  char *v7; // eax
  char *v8; // esi
  int v9[4]; // [esp+0h] [ebp-210h]
  char v10; // [esp+10h] [ebp-200h] BYREF

  result = Str;
  v3 = 0;
  v4 = Str;
  v5 = strlen(Str2);
  if ( Str )
  {
    v6 = &v10;
    do
    {
      if ( !*v4 )
      {
        break;
      }
      v7 = strchr(v4, 35);
      v8 = v7;
      if ( !v7 )
      {
        break;
      }
      if ( !strncmp(v7, Str2, v5) )
      {
        *(DWORD *)v6 = v8;
        ++v3;
        v6 += 4;
        v4 = &v8[v5];
      }
      else
      {
        v4 = v8 + 1;
      }
    }
    while ( v4 );
    if ( v3 < 5 )
    {
      return Str;
    }
    else
    {
      return (char *)v9[v3];
    }
  }
  return result;
}
#endif

// ── FUN_00405500 (IDA-only, gated) ──
#if defined(IDA_PORT_00405500)
void __cdecl CErrorReport::WriteDebugInfoStr(DWORD This, char *lpszToWrite)
{
  HANDLE m_hFile; // esi

  m_hFile = *(HANDLE *)(This + 4);
  if ( m_hFile != (HANDLE)-1 )
  {
    CErrorReport::WriteFile(This, m_hFile, lpszToWrite, strlen(lpszToWrite), (LPDWORD)&lpszToWrite, 0);
  }
}
#endif

// ── FUN_00405590 (IDA-only, gated) ──
#if defined(IDA_PORT_00405590)
void __cdecl CErrorReport::WriteLogBegin(DWORD This)
{
  CErrorReport::Write(This, aLogBegin);
}
#endif

// ── FUN_00405620 (IDA-only, gated) ──
#if defined(IDA_PORT_00405620)
void __cdecl CErrorReport::WriteSystemInfo(DWORD This, DWORD si)
{
  CErrorReport::Write(This, aSystemInformat);
  CErrorReport::Write(This, "OS \t\t\t: %s\r\n", (const char *)(si + 128));
  CErrorReport::Write(This, "CPU \t\t\t: %s\r\n", (const char *)si);
  CErrorReport::Write(This, "RAM \t\t\t: %dMB\r\n", *(DWORD *)(si + 256) / 1024 / 1024 + 1);
  CErrorReport::AddSeparator(This);
  CErrorReport::Write(This, "Direct-X \t\t: %s\r\n", (const char *)(si + 260));
}
#endif

// ── FUN_004056b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004056B0)
void __cdecl CErrorReport::WriteOpenGLInfo(DWORD This)
{
  const char *String; // eax MAPDST
  GLint iResult[2]; // [esp+8h] [ebp-8h] BYREF

  CErrorReport::Write(This, aOpenglInformat);
  String = (const char *)glGetString(0x1F00u);
  CErrorReport::Write(This, "Vendor \t\t\t: %s\r\n", String);
  String = (const char *)glGetString(0x1F01u);
  CErrorReport::Write(This, "Render \t\t\t: %s\r\n", String);
  String = (const char *)glGetString(0x1F02u);
  CErrorReport::Write(This, "OpenGL version \t\t: %s\r\n", String);
  glGetIntegerv(0xD33u, iResult);
  CErrorReport::Write(This, "Max Texture size \t: %d x %d\r\n", iResult[0], iResult[0]);
  glGetIntegerv(0xD3Au, iResult);
  CErrorReport::Write(This, "Max Viewport size \t: %d x %d\r\n", iResult[0], iResult[1]);
}
#endif

// ── FUN_00405760 (IDA-only, gated) ──
#if defined(IDA_PORT_00405760)
void __cdecl CErrorReport::WriteImeInfo(DWORD This, HWND hWnd)
{
  HIMC hImc; // ebx
  HKL hKl; // edi
  char lpszTemp[256]; // [esp+Ch] [ebp-100h] BYREF

  CErrorReport::Write(This, aImeInformation);
  hImc = ImmGetContext(hWnd);
  if ( hImc )
  {
    hKl = GetKeyboardLayout(0);
    ImmGetDescriptionA(hKl, lpszTemp, 0x100u);
    CErrorReport::Write(This, "IME Name \t\t: %s\r\n", lpszTemp);
    ImmGetIMEFileNameA(hKl, lpszTemp, 0x100u);
    CErrorReport::Write(This, "IME File Name \t\t: %s\r\n", lpszTemp);
    ImmReleaseContext(hWnd, hImc);
  }
  GetKeyboardLayoutNameA(lpszTemp);
  CErrorReport::Write(This, "Keyboard type\t\t: %s\r\n", lpszTemp);
}
#endif

// ── FUN_00405e20 (IDA-only, gated) ──
#if defined(IDA_PORT_00405E20)
int __cdecl FUN_00405e20(DWORD dwMilliseconds)
{
  __int64 v6; // rax
  HANDLE CurrentProcess; // esi
  HANDLE CurrentThread; // edi
  unsigned __int64 v15; // [esp+4h] [ebp-38h]
  unsigned __int64 v16; // [esp+Ch] [ebp-30h]
  LARGE_INTEGER Frequency; // [esp+14h] [ebp-28h] BYREF
  LARGE_INTEGER PerformanceCount; // [esp+1Ch] [ebp-20h] BYREF
  LARGE_INTEGER v19; // [esp+24h] [ebp-18h] BYREF
  DWORD dwPriorityClass; // [esp+2Ch] [ebp-10h]
  int nPriority; // [esp+30h] [ebp-Ch]
  ULONG_PTR SystemAffinityMask; // [esp+34h] [ebp-8h] BYREF
  ULONG_PTR ProcessAffinityMask; // [esp+38h] [ebp-4h] BYREF

  _EAX = 1;
  __asm { cpuid }
  nPriority = _EDX;
  if ( (_EDX & 0x10) != 0 )
  {
    LODWORD(v6) = QueryPerformanceFrequency(&Frequency);
    if ( (DWORD)v6 )
    {
      CurrentProcess = GetCurrentProcess();
      CurrentThread = GetCurrentThread();
      dwPriorityClass = GetPriorityClass(CurrentProcess);
      nPriority = GetThreadPriority(CurrentThread);
      GetProcessAffinityMask(CurrentProcess, &ProcessAffinityMask, &SystemAffinityMask);
      SetPriorityClass(CurrentProcess, 0x100u);
      SetThreadPriority(CurrentThread, 15);
      _EAX = SetProcessAffinityMask(CurrentProcess, 1u);
      __asm { cpuid }
      QueryPerformanceCounter(&PerformanceCount);
      v15 = __rdtsc();
      Sleep(dwMilliseconds);
      QueryPerformanceCounter(&v19);
      v16 = __rdtsc();
      SetProcessAffinityMask(CurrentProcess, ProcessAffinityMask);
      SetThreadPriority(CurrentThread, nPriority);
      SetPriorityClass(CurrentProcess, dwPriorityClass);
      return (__int64)((double)(__int64)(v16 - v15)
                     / ((double)(v19.QuadPart - PerformanceCount.QuadPart)
                      / (double)Frequency.QuadPart));
    }
  }
  else
  {
    LODWORD(v6) = 0;
  }
  return v6;
}
#endif

// ── FUN_00406660 (IDA-only, gated) ──
#if defined(IDA_PORT_00406660)
void FUN_00406660()
{
  HANDLE CurrentThread; // eax
  DWORD *Ebp; // esi
  int i; // edi
  CONTEXT Context; // [esp+Ch] [ebp-2CCh] BYREF

  memset(&Context, 0, sizeof(Context));
  Context.ContextFlags = 65543;
  CurrentThread = GetCurrentThread();
  GetThreadContext(CurrentThread, &Context);
  CErrorReport::Write((DWORD)&g_ErrorReport, aRegister);
  CErrorReport::Write(
    (DWORD)&g_ErrorReport,
    "EAX : 0x%08X\tEBX : 0x%08X\tECX : 0x%08X\tEDX : 0x%08X\r\n",
    Context.Eax,
    Context.Ebx,
    Context.Ecx,
    Context.Edx);
  CErrorReport::Write(
    (DWORD)&g_ErrorReport,
    "ESI : 0x%08X\tEDI : 0x%08X\tEBP : 0x%08X\tEIP : 0x%08X\r\n",
    Context.Esi,
    Context.Edi,
    Context.Ebp,
    Context.Eip);
  CErrorReport::Write((DWORD)&g_ErrorReport, aCallStack);
  Ebp = (DWORD *)Context.Ebp;
  for ( i = 0; i < 1024; ++i )
  {
    if ( IsBadReadPtr(Ebp, 4u) )
    {
      break;
    }
    if ( !*Ebp )
    {
      break;
    }
    CErrorReport::Write((DWORD)&g_ErrorReport, "0x%08X\r\n", Ebp[1]);
    Ebp = (DWORD *)*Ebp;
  }
}
#endif

// ── FUN_00406bd0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406BD0)
bool __cdecl FUN_00406bd0(unsigned char *Text)
{
  unsigned char *lpszCheck; // esi
  unsigned char v2; // al
  unsigned char v3; // cl
  unsigned char lpszTrail; // al

  lpszCheck = Text;
  if ( !*Text )
  {
    return 0;
  }
  while ( _mbclen(lpszCheck) == 1 )
  {
    v2 = *lpszCheck;
    if ( *lpszCheck < 0x30u || v2 >= 0x3Au && v2 < 0x41u || v2 >= 0x5Bu && v2 < 0x61u || v2 > 0x7Au )
    {
      return 1;
    }
LABEL_27:
    if ( !*++lpszCheck )
    {
      return 0;
    }
  }
  v3 = *lpszCheck;
  if ( *lpszCheck >= 0x81u && v3 <= 0xC8u )
  {
    if ( (lpszTrail = lpszCheck[1], lpszTrail >= 0x41u) && lpszTrail <= 0x5Au
      || lpszTrail >= 0x61u && lpszTrail <= 0x7Au
      || lpszTrail >= 0x81u && lpszTrail != 0xFF )
    {
      if ( (v3 < 0xA1u || v3 > 0xAFu || lpszTrail < 0xA1u)
        && (v3 != 0xC6 || lpszTrail < 0x53u || lpszTrail > 0xA0u)
        && (v3 < 0xC7u || lpszTrail > 0xA0u) )
      {
        ++lpszCheck;
        goto LABEL_27;
      }
    }
  }
  return 1;
}
#endif

// ── FUN_00406cb0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406CB0)
LPVOID __cdecl FUN_00406cb0(LPVOID lpMem, char a2)
{
  FUN_00406cd0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00406cd0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406CD0)
int __cdecl FUN_00406cd0(DWORD *_this)
{
  int result; // eax

  *_this = &DAT_005524c8;
  result = FUN_00406d40();
  _this[3] = 0;
  _this[1] = 0;
  _this[2] = 0;
  *_this = &DAT_005524d8;
  return result;
}
#endif

// ── FUN_00406d20 (IDA-only, gated) ──
#if defined(IDA_PORT_00406D20)
int __cdecl FUN_00406d20(DWORD *_this)
{
  int result; // eax

  result = 0;
  _this[7] = 131;
  _this[3] = 0;
  _this[1] = 0;
  _this[2] = 0;
  _this[6] = 0;
  return result;
}
#endif

// ── FUN_00406d40 (IDA-only, gated) ──
#if defined(IDA_PORT_00406D40)
void __cdecl FUN_00406d40(int _this)
{
  unsigned int i; // edi
  void *v3; // eax

  for ( i = 0; i < 0x1000; i += 4 )
  {
    if ( *(DWORD *)(*(DWORD *)(_this + 4) + i) )
    {
      delete__(*(LPVOID *)(*(DWORD *)(_this + 4) + i));
      *(DWORD *)(*(DWORD *)(_this + 4) + i) = 0;
    }
  }
  if ( *(DWORD *)(_this + 24) )
  {
    delete__(*(LPVOID *)(_this + 24));
  }
  v3 = *(void **)(_this + 36);
  *(DWORD *)(_this + 4) = *(DWORD *)(_this + 32);
  *(DWORD *)(_this + 8) = v3;
  delete__(v3);
  delete__(*(LPVOID *)(_this + 4));
  *(DWORD *)(_this + 12) = 0;
}
#endif

// ── FUN_00406db0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406DB0)
int __cdecl FUN_00406db0(DWORD *_this, int a2, int a3)
{
  int result; // eax

  _this[4] = a2;
  _this[5] = a3;
  result = rand() % 240 / 2;
  _this[7] = 2 * result + 111;
  return result;
}
#endif

// ── FUN_00406de0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406DE0)
int __cdecl FUN_00406db0(DWORD *_this, int a2, int a3)
{
  int result; // eax

  _this[4] = a2;
  _this[5] = a3;
  result = rand() % 240 / 2;
  _this[7] = 2 * result + 111;
  return result;
}
#endif

// ── FUN_00406e60 (IDA-only, gated) ──
#if defined(IDA_PORT_00406E60)
void __cdecl FUN_00406e60(int _this)
{
  delete__(*(LPVOID *)(_this + 8));
  delete__(*(LPVOID *)(_this + 4));
  *(DWORD *)(_this + 12) = 0;
}
#endif

// ── FUN_00406e90 (IDA-only, gated) ──
#if defined(IDA_PORT_00406E90)
int __cdecl FUN_00406e90(DWORD *_this, char a2)
{
  unsigned int v2; // eax
  char *v3; // edx
  int v4; // esi

  v2 = 0;
  v3 = &a2;
  v4 = 4;
  do
  {
    v2 = (unsigned char)*v3++ + 131 * v2;
    --v4;
  }
  while ( v4 );
  return v2 % _this[3];
}
#endif

// ── FUN_00406ec0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406EC0)
DWORD *__cdecl FUN_00406ec0(DWORD *lpMem, char a2)
{
  lpMem[3] = 0;
  lpMem[1] = 0;
  lpMem[2] = 0;
  *lpMem = &DAT_005524d8;
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00406ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406EF0)
int __cdecl FUN_00406ef0(DWORD *_this, char a2)
{
  unsigned int v2; // eax
  char *v3; // edx
  int v4; // esi

  v2 = 0;
  v3 = &a2;
  v4 = 4;
  do
  {
    v2 = v2 * _this[7] + (unsigned char)*v3++;
    --v4;
  }
  while ( v4 );
  return v2 % _this[3];
}
#endif

// ── FUN_004070d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004070D0)
char __stdcall FUN_004070d0(int a1, int a2)
{
  int v2; // ecx
  int v3; // eax
  int v4; // ecx
  int v5; // eax
  int v6; // ebx
  int v7; // esi
  unsigned int v8; // eax
  char *v9; // eax
  char v10; // cl
  int v11; // eax
  BYTE v12; // al
  BYTE *v13; // eax
  char v14; // cl
  int v15; // esi
  int v16; // edi
  char *v17; // ebp
  int v18; // eax
  unsigned int v19; // ebx
  int v20; // edi
  unsigned int v21; // ebp
  int v22; // eax
  int v23; // esi
  char *v24; // edi
  char *v25; // edi
  char *v26; // esi
  char v27; // cl
  unsigned int v28; // esi
  int v29; // ebx
  int v30; // eax
  int v31; // edi
  char *v33; // edi
  int v34; // [esp-14h] [ebp-DD0h]
  char v35; // [esp+0h] [ebp-DBCh]
  char v36; // [esp+1h] [ebp-DBBh]
  char v37; // [esp+2h] [ebp-DBAh]
  char v38; // [esp+3h] [ebp-DB9h]
  char v39; // [esp+4h] [ebp-DB8h]
  char v40; // [esp+5h] [ebp-DB7h]
  char v41; // [esp+6h] [ebp-DB6h]
  char v42; // [esp+7h] [ebp-DB5h]
  char v43; // [esp+8h] [ebp-DB4h]
  char v44; // [esp+9h] [ebp-DB3h]
  char v45; // [esp+Ah] [ebp-DB2h]
  char v46; // [esp+Bh] [ebp-DB1h]
  char v47; // [esp+Ch] [ebp-DB0h]
  char v48; // [esp+Dh] [ebp-DAFh]
  char v49; // [esp+Eh] [ebp-DAEh]
  char v50; // [esp+Fh] [ebp-DADh]
  char v51; // [esp+10h] [ebp-DACh]
  char v52; // [esp+11h] [ebp-DABh]
  char v53; // [esp+12h] [ebp-DAAh]
  char v54; // [esp+13h] [ebp-DA9h]
  char v55; // [esp+14h] [ebp-DA8h]
  char v56; // [esp+15h] [ebp-DA7h]
  char v57; // [esp+16h] [ebp-DA6h]
  char v58; // [esp+17h] [ebp-DA5h]
  char v59; // [esp+18h] [ebp-DA4h]
  char v60; // [esp+19h] [ebp-DA3h]
  char v61; // [esp+1Ah] [ebp-DA2h]
  char v62; // [esp+1Bh] [ebp-DA1h]
  char v63; // [esp+1Ch] [ebp-DA0h]
  char v64; // [esp+1Dh] [ebp-D9Fh]
  char v65; // [esp+1Eh] [ebp-D9Eh]
  char v66; // [esp+1Fh] [ebp-D9Dh]
  CHAR Text[128]; // [esp+20h] [ebp-D9Ch] BYREF
  void *(__cdecl **v68)(std::locale::facet *__hidden, unsigned int); // [esp+A0h] [ebp-D1Ch]
  BYTE v69[1025]; // [esp+A4h] [ebp-D18h] BYREF
  char buf[2]; // [esp+4A8h] [ebp-914h] BYREF
  char v71[258]; // [esp+4AAh] [ebp-912h] BYREF
  char v72[3]; // [esp+5ACh] [ebp-810h] BYREF
  char v73[1025]; // [esp+5AFh] [ebp-80Dh] BYREF
  char v74[1024]; // [esp+9B0h] [ebp-40Ch] BYREF
  int v75; // [esp+DB8h] [ebp-4h]

  switch ( a1 )
  {
    case 1001:
    case 1002:
      wsprintfA(Text, GlobalText[792], a1);
      goto LABEL_55;
    case 1011:
      MessageBoxA(g_hWnd, GlobalText[793], aError, 0);
      CloseHack(g_hWnd, 1);
      return 0;
    case 1012:
      wsprintfA(Text, GlobalText[794], a1);
      MessageBoxA(g_hWnd, Text, aError, 0);
      CloseHack(g_hWnd, 1);
      return 1;
    case 1013:
      wsprintfA(Text, GlobalText[794], a1);
LABEL_55:
      MessageBoxA(g_hWnd, Text, aError, 0);
      CloseHack(g_hWnd, 1);
      return 0;
    case 1014:
      wsprintfA(Text, GlobalText[792], a2);
      MessageBoxA(g_hWnd, Text, aError, 0);
      CloseHack(g_hWnd, 1);
      return 0;
    case 1016:
      v68 = &DAT_00552460;
      v75 = 0;
      *(DWORD *)v69 = 29425667;
      v69[4] = 115;
      v35 = -25;
      v36 = 109;
      v37 = 58;
      v69[*(unsigned short *)v69 + 2] = 0;
      v2 = *(unsigned short *)v69;
      v38 = -119;
      v3 = *(unsigned short *)v69 + 1;
      v39 = -68;
      v40 = -78;
      v41 = -97;
      v42 = 115;
      v43 = 35;
      v44 = -88;
      v45 = -2;
      v46 = -74;
      v47 = 73;
      v48 = 93;
      v49 = 57;
      v50 = 93;
      v51 = -118;
      v52 = -53;
      v53 = 99;
      v54 = -115;
      v55 = -22;
      v56 = 125;
      v57 = 43;
      v58 = 95;
      v59 = -61;
      v60 = -79;
      v61 = -23;
      v62 = -125;
      v63 = 41;
      v64 = 81;
      v65 = -24;
      v66 = 86;
      if ( *(unsigned short *)v69 != v3 )
      {
        do
        {
          v69[v2 + 2] ^= v69[v2 + 1] ^ *(&v35 + v2 % 32);
          ++v2;
        }
        while ( v2 != v3 );
      }
      ++*(WORD *)v69;
      if ( *(unsigned short *)v69 + 4 <= 1024 )
      {
        v35 = -25;
        v36 = 109;
        v37 = 58;
        *(DWORD *)&v69[*(unsigned short *)v69 + 2] = a2;
        v4 = *(unsigned short *)v69;
        v38 = -119;
        v5 = *(unsigned short *)v69 + 4;
        v39 = -68;
        v40 = -78;
        v41 = -97;
        v42 = 115;
        v43 = 35;
        v44 = -88;
        v45 = -2;
        v46 = -74;
        v47 = 73;
        v48 = 93;
        v49 = 57;
        v50 = 93;
        v51 = -118;
        v52 = -53;
        v53 = 99;
        v54 = -115;
        v55 = -22;
        v56 = 125;
        v57 = 43;
        v58 = 95;
        v59 = -61;
        v60 = -79;
        v61 = -23;
        v62 = -125;
        v63 = 41;
        v64 = 81;
        v65 = -24;
        v66 = 86;
        if ( *(unsigned short *)v69 != v5 )
        {
          do
          {
            v69[v4 + 2] ^= v69[v4 + 1] ^ *(&v35 + v4 % 32);
            ++v4;
          }
          while ( v4 != v5 );
        }
        *(WORD *)v69 += 4;
      }
      if ( v69[2] == 193 )
      {
        v69[3] = v69[0];
      }
      else if ( v69[2] == 194 )
      {
        *(WORD *)&v69[3] = *(WORD *)v69;
      }
      v6 = *(unsigned short *)v69;
      qmemcpy(v74, &v69[2], *(unsigned short *)v69);
      v74[v6] = rand();
      v7 = (v74[0] != -63) + 2;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
      {
        v11 = operator_new(2u);
        *(BYTE *)(v11 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v11, (int)&g_byPacketSerialSend);
      }
      else
      {
        v8 = FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        if ( v8 == -1 )
        {
          v9 = 0;
        }
        else
        {
          v9 = *(char **)(DAT_055c9bcc + 4 * v8);
        }
        v10 = v9[1] + 1;
        v9[1] = v10;
        if ( (unsigned char)v10 < 2u )
        {
          FUN_00404330(&g_byPacketSerialSend, v9);
        }
      }
      v12 = g_byPacketSerialSend;
      v73[v7 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v12 + 1;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) != -1 )
      {
        v13 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        v14 = v13[1] - 1;
        v13[1] = v14;
        if ( !v14 )
        {
          FUN_00423710(v13, &g_byPacketSerialSend);
        }
      }
      v15 = v7 - 1;
      v16 = v6 - v15;
      v17 = &v74[v15];
      v18 = FUN_0053cc30(0, (int)&v74[v15], v6 - v15);
      if ( v18 >= 256 )
      {
        v28 = v18 + 3;
        v72[0] = -60;
        v72[2] = v18 + 3;
        v72[1] = (v18 + 3) / 256;
        FUN_0053cc30((int)v73, (int)v17, v16);
        v29 = 0;
        v21 = v28;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v30 = send(s, &v72[v29], v28 - v29, 0);
            v31 = v30;
            if ( v30 == -1 )
            {
              break;
            }
            if ( v30 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2(v72, v30);
              }
              v21 -= v31;
              v29 += v31;
              if ( (int)v21 > 0 )
              {
                continue;
              }
            }
            return 1;
          }
          if ( WSAGetLastError() == 10035 && (int)(DAT_055cc16c + v28) <= 0x2000 )
          {
            v33 = (char *)&DAT_055ca16c + DAT_055cc16c;
            qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, v72, 4 * (v21 >> 2));
            v26 = &v72[4 * (v21 >> 2)];
            v25 = &v33[4 * (v21 >> 2)];
            v27 = v21;
            goto LABEL_47;
          }
LABEL_45:
          CWsctlc::Close((DWORD)&SocketClient);
          return 1;
        }
      }
      else
      {
        v34 = v6 - v15;
        v19 = v18 + 2;
        buf[0] = -61;
        buf[1] = v18 + 2;
        FUN_0053cc30((int)v71, (int)&v74[v15], v34);
        v20 = 0;
        v21 = v19;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v22 = send(s, &buf[v20], v19 - v20, 0);
            v23 = v22;
            if ( v22 == -1 )
            {
              break;
            }
            if ( v22 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2(buf, v22);
              }
              v21 -= v23;
              v20 += v23;
              if ( (int)v21 > 0 )
              {
                continue;
              }
            }
            return 1;
          }
          if ( WSAGetLastError() == 10035 && (int)(DAT_055cc16c + v19) <= 0x2000 )
          {
            v24 = (char *)&DAT_055ca16c + DAT_055cc16c;
            qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, buf, 4 * (v21 >> 2));
            v26 = &buf[4 * (v21 >> 2)];
            v25 = &v24[4 * (v21 >> 2)];
            v27 = v21;
LABEL_47:
            qmemcpy(v25, v26, v27 & 3);
            DAT_055cc16c += v21;
            return 1;
          }
          goto LABEL_45;
        }
      }
      return 1;
    default:
      return 1;
  }
}
#endif

// ── FUN_00407980 (IDA-only, gated) ──
#if defined(IDA_PORT_00407980)
DWORD *__cdecl FUN_00407980(int _this)
{
  DWORD *result; // eax
  int v2; // esi

  *(DWORD *)(_this + 44) = 0;
  result = (DWORD *)(_this + 28);
  v2 = 3;
  do
  {
    result[5] = 0;
    *result = 0;
    *(result - 3) = 0;
    *(result - 6) = 0;
    ++result;
    --v2;
  }
  while ( v2 );
  *(BYTE *)(_this + 40) = 0;
  return result;
}
#endif

// ── FUN_004079b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004079B0)
int __cdecl FUN_004079b0(int _this, int a2, int a3, int a4, int a5)
{
  int result; // eax

  *(DWORD *)(_this + 28) = a2;
  *(DWORD *)(_this + 36) = a4;
  result = a5;
  *(DWORD *)(_this + 32) = a3;
  if ( a5 )
  {
    *(BYTE *)(_this + 40) |= 1u;
  }
  return result;
}
#endif

// ── FUN_00407ac0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407AC0)
void __cdecl FUN_00407ac0(float *_this, float a2, float a3, float a4)
{
  _this[1] = a2 + _this[1];
  _this[2] = a3 + _this[2];
  _this[3] = a4 + _this[3];
}
#endif

// ── FUN_00407af0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407AF0)
void __cdecl FUN_00407af0(float *_this, float a2)
{
  float *v2; // eax
  int v3; // ecx
  double v4; // st7
  double v5; // st7

  if ( ((BYTE)_this[10] & 1) == 0 )
  {
    v2 = _this + 4;
    v3 = 3;
    do
    {
      v4 = DAT_00559070 * *(v2 - 3);
      ++v2;
      --v3;
      v5 = v4 * a2 + *(v2 - 1);
      *(v2 - 1) = v5;
      v2[2] = v5 * a2 + v2[2];
    }
    while ( v3 );
  }
}
#endif

// ── FUN_00407b30 (IDA-only, gated) ──
#if defined(IDA_PORT_00407B30)
int *__cdecl FUN_00407b30(int *_this, DWORD *a2)
{
  int *result; // eax
  int v4; // ecx
  int v5; // esi

  result = _this + 7;
  v4 = 3;
  do
  {
    v5 = *result++;
    *a2++ = v5;
    --v4;
  }
  while ( v4 );
  return result;
}
#endif

// ── FUN_00407b50 (IDA-only, gated) ──
#if defined(IDA_PORT_00407B50)
void __cdecl FUN_00407b50(char *_this, int a2, int a3)
{
  float *v3; // eax
  char *v4; // ecx
  int v5; // edx
  int v6; // esi
  double v7; // st7

  v3 = (float *)(a2 + 28);
  v4 = &_this[-a2];
  v5 = a3;
  v6 = 3;
  do
  {
    v7 = *(float *)((char *)v3 + (DWORD)v4) - *v3;
    ++v3;
    v5 += 4;
    --v6;
    *(float *)(v5 - 4) = v7;
  }
  while ( v6 );
  FUN_004f9c40(a3);
}
#endif

// ── FUN_00407b90 (IDA-only, gated) ──
#if defined(IDA_PORT_00407B90)
int __cdecl FUN_00407b90(int a1, double a2, int a3, float *a4)
{
  double v6; // st6
  double v7; // st7
  double v8; // st7
  float v9; // [esp+4h] [ebp-Ch] BYREF
  float v10; // [esp+8h] [ebp-8h]
  float v11; // [esp+Ch] [ebp-4h]

  if ( (*(BYTE *)(a1 + 40) & 1) != 0 )
  {
    return 1;
  }
  FUN_00407b50((char *)a1, a3, (int)&v9);
  if ( a2 >= 0.001 )
  {
    FUN_00407b50((char *)a1, a3, (int)&v9);
  }
  else
  {
    a2 = 0.001;
  }
  if ( a2 > a4[1] * 20.0 )
  {
    return 0;
  }
  if ( a2 > a4[1] )
  {
    v6 = a2 - a4[1];
LABEL_11:
    v7 = v6 / a2;
    v9 = v7 * v9;
    v10 = v7 * v10;
    v8 = v7 * v11;
    *(float *)(a1 + 28) = *(float *)(a1 + 28) - v9;
    *(float *)(a1 + 32) = *(float *)(a1 + 32) - v10;
    *(float *)(a1 + 36) = *(float *)(a1 + 36) - v8;
    return 1;
  }
  if ( a2 < *a4 )
  {
    v6 = a2 - *a4;
    goto LABEL_11;
  }
  return 1;
}
#endif

// ── FUN_00407c60 (IDA-only, gated) ──
#if defined(IDA_PORT_00407C60)
int __cdecl FUN_00407c60(int a1, double a2, int a3, float a4)
{
  double v5; // st7
  double v6; // st7
  int result; // eax
  float v8; // [esp+8h] [ebp-Ch] BYREF
  float v9; // [esp+Ch] [ebp-8h]
  float v10; // [esp+10h] [ebp-4h]

  FUN_00407b50((char *)a1, a3, (int)&v8);
  if ( a2 >= 0.001 )
  {
    FUN_00407b50((char *)a1, a3, (int)&v8);
  }
  else
  {
    a2 = 0.001;
  }
  v5 = (a2 - a4) * 0.5 / a2;
  v8 = v5 * v8;
  v9 = v5 * v9;
  v6 = v5 * v10;
  *(float *)(a1 + 48) = *(float *)(a1 + 48) - v8;
  *(float *)(a1 + 52) = *(float *)(a1 + 52) - v9;
  *(float *)(a1 + 56) = *(float *)(a1 + 56) - v6;
  *(float *)(a3 + 48) = v8 + *(float *)(a3 + 48);
  *(float *)(a3 + 52) = v9 + *(float *)(a3 + 52);
  *(float *)(a3 + 56) = v6 + *(float *)(a3 + 56);
  ++*(DWORD *)(a1 + 44);
  result = *(DWORD *)(a3 + 44) + 1;
  *(DWORD *)(a3 + 44) = result;
  return result;
}
#endif

// ── FUN_00407da0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407DA0)
DWORD *__cdecl FUN_00407da0(DWORD *_this)
{
  *_this = (DWORD)&DAT_00552508;
  FUN_00407df0((int)_this);
  return _this;
}
#endif

// ── FUN_00407dc0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407DC0)
LPVOID __cdecl FUN_00407dc0(LPVOID lpMem, char a2)
{
  FUN_00407de0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00407df0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407DF0)
int __cdecl FUN_00407df0(DWORD *_this)
{
  int result; // eax

  result = 0;
  _this[1] = 0;
  _this[2] = 0;
  _this[3] = 0;
  _this[4] = 0;
  _this[5] = 0;
  _this[6] = 0;
  _this[7] = 0;
  return result;
}
#endif

// ── FUN_00407e10 (IDA-only, gated) ──
#if defined(IDA_PORT_00407E10)
int __cdecl FUN_00407e10(DWORD *_this, int a2, int a3, int a4)
{
  int result; // eax

  _this[5] = a2;
  result = a4;
  _this[6] = a3;
  _this[7] = a4;
  return result;
}
#endif

// ── FUN_00407e30 (IDA-only, gated) ──
#if defined(IDA_PORT_00407E30)
DWORD *__cdecl FUN_00407e30(DWORD *_this, DWORD *a2)
{
  DWORD *result; // eax
  DWORD *v3; // ecx

  result = a2;
  v3 = _this + 1;
  *a2 = *v3;
  a2[1] = v3[1];
  a2[2] = v3[2];
  return result;
}
#endif

// ── FUN_00407ea0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407EA0)
LPVOID __cdecl FUN_00407ea0(LPVOID lpMem, char a2)
{
  FUN_00407ec0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00407ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407EF0)
int __cdecl FUN_00407ef0(DWORD *_this, int a2, int a3, int a4, int a5, int a6)
{
  int result; // eax

  _this[1] = a2;
  _this[2] = a3;
  _this[3] = a4;
  result = a6;
  _this[8] = a5;
  _this[4] = a6;
  return result;
}
#endif

// ── FUN_00408070 (IDA-only, gated) ──
#if defined(IDA_PORT_00408070)
void __cdecl FUN_00408070(void *_this)
{
  DWORD *v2; // esi
  void *i; // eax
  void *v4; // eax
  void *v5; // edi

  *(DWORD *)_this = &DAT_00552520;
  *(DWORD *)(*(DWORD *)(*((DWORD *)_this + 20) + 4) + 8) = 0;
  v2 = *(DWORD **)(*((DWORD *)_this + 19) + 8);
  for ( i = v2; v2; i = v2 )
  {
    v2 = (DWORD *)v2[2];
    if ( i )
    {
      delete__(i);
    }
  }
  *(DWORD *)(*((DWORD *)_this + 19) + 8) = *((DWORD *)_this + 20);
  *(DWORD *)(*((DWORD *)_this + 20) + 4) = *((DWORD *)_this + 19);
  v4 = (void *)*((DWORD *)_this + 20);
  *((DWORD *)_this + 18) = 0;
  if ( v4 )
  {
    delete__(v4);
  }
  v5 = (void *)*((DWORD *)_this + 19);
  if ( v5 )
  {
    delete__(v5);
  }
}
#endif

// ── FUN_004080f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004080F0)
int __cdecl FUN_004080f0(DWORD *_this)
{
  int result; // eax

  result = 0;
  _this[1] = 0;
  _this[2] = 0;
  _this[4] = 0;
  _this[3] = 0;
  _this[5] = 0;
  _this[7] = 0;
  _this[6] = 0;
  _this[9] = 1065353216;
  _this[8] = 1065353216;
  _this[17] = 1065353216;
  _this[16] = 1065353216;
  _this[10] = 0;
  _this[11] = 0;
  _this[12] = 0;
  _this[13] = 0;
  _this[14] = 0;
  _this[15] = 0;
  return result;
}
#endif

// ── FUN_004088b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004088B0)
int __cdecl FUN_004088b0(DWORD *_this, int a2, short a3, short a4, float a5, float a6, char a7)
{
  int result; // eax

  result = 16 * a2;
  *(WORD *)(_this[15] + result) = a3;
  *(WORD *)(_this[15] + result + 2) = a4;
  *(float *)(_this[15] + result + 4) = a5;
  *(float *)(_this[15] + result + 8) = a6;
  *(BYTE *)(_this[15] + result + 12) = a7;
  return result;
}
#endif

// ── FUN_00408cb0 (IDA-only, gated) ──
#if defined(IDA_PORT_00408CB0)
int __cdecl FUN_00408cb0(DWORD *a1, double a2, float a3)
{
  int v4; // esi
  float *v5; // esi
  int v6; // eax
  char *v7; // ebp
  float *v8; // ebx
  double v9; // st6
  int v10; // ecx
  int i; // eax
  double v12; // st5
  bool v13; // cc
  int result; // eax
  int v15; // esi
  int v16; // ebx
  float v17; // [esp+0h] [ebp-3Ch]
  float v18; // [esp+4h] [ebp-38h]
  float v19; // [esp+8h] [ebp-34h]
  int v20; // [esp+1Ch] [ebp-20h]
  int v21; // [esp+20h] [ebp-1Ch]
  float v22; // [esp+24h] [ebp-18h]
  float v23; // [esp+28h] [ebp-14h]
  float v24; // [esp+2Ch] [ebp-10h]
  float v25[3]; // [esp+30h] [ebp-Ch] BYREF

  (*(void (__cdecl **)(DWORD *))(*a1 + 8))(a1);
  v4 = 0;
  v20 = 0;
  if ( (int)a1[14] > 0 )
  {
    v21 = 0;
    do
    {
      v5 = (float *)(a1[15] + v4);
      if ( ((BYTE)v5[3] & 2) != 0 )
      {
        v6 = a1[13];
        v7 = (char *)(v6 + 60 * *(short *)v5);
        v8 = (float *)(v6 + 60 * *((short *)v5 + 1));
        FUN_00407b50(v7, (int)v8, (int)v25);
        if ( a2 >= 0.001 )
        {
          FUN_00407b50(v7, (int)v8, (int)v25);
        }
        else
        {
          a2 = 0.001;
        }
        if ( a2 > v5[2] + 0.0099999998 )
        {
          v9 = a2 - v5[2];
          v10 = a1[5] & 0x300;
          for ( i = 0; i < 3; ++i )
          {
            v12 = v9 * v25[i] / a2;
            *(float *)((char *)&v22 + i * 4) = v12;
            if ( v10 == 256 )
            {
              *(float *)((char *)&v22 + i * 4) = v12 * 3.0;
            }
          }
          v19 = -v24;
          v18 = -v23;
          a2 = -v22;
          v17 = a2;
          FUN_00407ac0((float *)v7, v17, v18, v19);
          FUN_00407ac0(v8, v22, v23, v24);
        }
      }
      v4 = v21 + 16;
      v13 = ++v20 < a1[14];
      v21 += 16;
    }
    while ( v13 );
  }
  (*(void (__cdecl **)(DWORD *, int))(*a1 + 4))(a1, 48 * a1[2] + *(DWORD *)(a1[1] + 276));
  result = a1[12];
  v15 = 0;
  if ( result > 0 )
  {
    v16 = 0;
    do
    {
      FUN_00407af0((float *)(v16 + a1[13]), a3);
      result = a1[12];
      ++v15;
      v16 += 60;
    }
    while ( v15 < result );
  }
  return result;
}
#endif

// ── FUN_00408e30 (IDA-only, gated) ──
#if defined(IDA_PORT_00408E30)
int __cdecl FUN_00408e30(DWORD *a1, double a2)
{
  int v3; // ebx
  int i; // ebp
  DWORD *v5; // edi
  float v6; // eax
  int v7; // ebp
  int v8; // edi
  float *v9; // eax
  int j; // ebp
  int v11; // eax
  int k; // edi
  float *v13; // eax
  int v14; // edi
  int v16; // [esp+10h] [ebp-28h]
  float Position[3]; // [esp+14h] [ebp-24h] BYREF
  float v18; // [esp+20h] [ebp-18h]
  float v19; // [esp+24h] [ebp-14h]
  float v20; // [esp+28h] [ebp-10h]
  float WorldPosition[3]; // [esp+2Ch] [ebp-Ch] BYREF

  v3 = 0;
  for ( i = *(DWORD *)(a1[19] + 8); a1[20] != i && i; i = *(DWORD *)(i + 8) )
  {
    v5 = *(DWORD **)i;
    FUN_00407e30(*(DWORD **)i, Position);
    v19 = Position[1];
    v18 = Position[0];
    v6 = Position[0];
    v20 = Position[2];
    a2 = -Position[1];
    Position[1] = a2;
    Position[0] = Position[2];
    Position[2] = v6;
    TransformPosition(
      Models + 188 * *(short *)(a1[1] + 2),
      (float (*)[4])(*(DWORD *)(a1[1] + 276) + 48 * v5[4]),
      Position,
      WorldPosition,
      1);
    FUN_00407e10(v5, SLODWORD(WorldPosition[0]), SLODWORD(WorldPosition[1]), SLODWORD(WorldPosition[2]));
  }
  FUN_00409310(a1);
  v7 = 0;
  if ( (int)a1[14] > 0 )
  {
    v8 = 0;
    do
    {
      v9 = (float *)(a1[15] + v8);
      if ( ((BYTE)v9[3] & 1) != 0 )
      {
        FUN_00407c60(a1[13] + 60 * *(short *)v9, a2, a1[13] + 60 * *((short *)v9 + 1), v9[2]);
      }
      ++v7;
      v8 += 16;
    }
    while ( v7 < a1[14] );
  }
  for ( j = 0; j < a1[11]; ++j )
  {
    v11 = a1[10];
    for ( k = 0; k < v11; ++k )
    {
      FUN_00407d10(a1[13] + 60 * (k + j * v11));
      v11 = a1[10];
    }
  }
  v16 = 0;
  if ( (int)a1[14] <= 0 )
  {
    return 1;
  }
  while ( 1 )
  {
    v13 = (float *)(a1[15] + v3);
    v14 = *((short *)v13 + 1);
    if ( v14 >= a1[10]
      && ((BYTE)v13[3] & 4) != 0
      && !FUN_00407b90(a1[13] + 60 * v14, a2, a1[13] + 60 * *(short *)v13, v13 + 1) )
    {
      break;
    }
    v3 += 16;
    if ( ++v16 >= a1[14] )
    {
      return 1;
    }
  }
  return 0;
}
#endif

// ── FUN_00408ff0 (IDA-only, gated) ──
#if defined(IDA_PORT_00408FF0)
void __cdecl FUN_00408ff0(int _this, int a2)
{
  DWORD *v3; // ebx
  int i; // ebp
  int v5; // eax
  int j; // edi

  v3 = (DWORD *)operator_new(12 * *(DWORD *)(_this + 48));
  for ( i = 0; i < *(DWORD *)(_this + 44); ++i )
  {
    v5 = *(DWORD *)(_this + 40);
    for ( j = 0; j < v5; ++j )
    {
      FUN_00407b30((int *)(*(DWORD *)(_this + 52) + 60 * (j + i * v5)), &v3[3 * j + 3 * i * v5]);
      v5 = *(DWORD *)(_this + 40);
    }
  }
  if ( (*(DWORD *)(_this + 20) & 0x3000) != 0 )
  {
    if ( (*(DWORD *)(_this + 20) & 0x3000) == 4096 )
    {
      EnableAlphaTest(1);
    }
  }
  else
  {
    DisableAlphaBlend();
  }
  glColor3f(1.0, 1.0, 1.0);
  FUN_004090b0((DWORD *)_this, 1, *(GLfloat *)(_this + 12), (int)v3);
  FUN_004090b0((DWORD *)_this, 0, *(GLfloat *)(_this + 16), (int)v3);
  delete__(v3);
}
#endif

// ── FUN_004090b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004090B0)
void __cdecl FUN_004090b0(DWORD *_this, int a2, GLfloat t, int a4)
{
  GLfloat v5; // ebp
  int i; // edi
  int j; // edi
  GLfloat ta; // [esp+18h] [ebp+8h]

  BindTexture(SLODWORD(t));
  glBegin(7u);
  v5 = 0.0;
  if ( a2 )
  {
    ta = 0.0;
    if ( _this[11] - 1 > 0 )
    {
      do
      {
        for ( i = 0; i < _this[10] - 1; ++i )
        {
          FUN_004091d0(a4, i, ta);
          FUN_004091d0(a4, i + 1, ta);
          FUN_004091d0(a4, i + 1, COERCE_GLFLOAT(LODWORD(ta) + 1));
          FUN_004091d0(a4, i, COERCE_GLFLOAT(LODWORD(ta) + 1));
        }
        ++LODWORD(ta);
      }
      while ( SLODWORD(ta) < _this[11] - 1 );
    }
  }
  else if ( _this[11] - 1 > 0 )
  {
    do
    {
      for ( j = 0; j < _this[10] - 1; FUN_004091d0(a4, j, v5) )
      {
        FUN_004091d0(a4, j, v5);
        FUN_004091d0(a4, j++, COERCE_GLFLOAT(LODWORD(v5) + 1));
        FUN_004091d0(a4, j, COERCE_GLFLOAT(LODWORD(v5) + 1));
      }
      ++LODWORD(v5);
    }
    while ( SLODWORD(v5) < _this[11] - 1 );
  }
  glEnd();
}
#endif

// ── FUN_004091d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004091D0)
void __cdecl FUN_004091d0(DWORD *_this, int a2, int a3, GLfloat t)
{
  int v4; // edx
  double v5; // st7
  GLfloat *v6; // esi
  GLfloat s; // [esp+0h] [ebp-Ch]
  float ta; // [esp+18h] [ebp+Ch]

  v4 = _this[10];
  v5 = (double)SLODWORD(t) / (double)(_this[11] - 1);
  v6 = (GLfloat *)(a2 + 12 * (a3 + LODWORD(t) * v4));
  if ( v5 <= 0.99000001 )
  {
    ta = v5;
  }
  else
  {
    ta = 0.99000001;
  }
  s = (double)a3 / (double)(v4 - 1);
  glTexCoord2f(s, ta);
  glVertex3f(*v6, v6[1], v6[2]);
}
#endif

// ── FUN_00409310 (IDA-only, gated) ──
#if defined(IDA_PORT_00409310)
int __cdecl FUN_00409310(DWORD *_this)
{
  int result; // eax
  DWORD *v3; // eax
  int v4; // ebx
  int v5; // edi
  int v6; // ebp
  int v7; // edi
  int v8; // ebx
  DWORD *i; // [esp+Ch] [ebp-4h]

  result = _this[18];
  if ( result > 0 )
  {
    v3 = *(DWORD **)(_this[19] + 8);
    if ( v3 != (DWORD *)_this[20] )
    {
      for ( i = *(DWORD **)(_this[19] + 8); v3; i = v3 )
      {
        v4 = *v3;
        v5 = 0;
        if ( (int)_this[12] > 0 )
        {
          v6 = 0;
          do
          {
            (*(void (__cdecl **)(int, int))(*(DWORD *)v4 + 8))(v4, v6 + _this[13]);
            ++v5;
            v6 += 60;
          }
          while ( v5 < _this[12] );
          v3 = i;
        }
        v3 = (DWORD *)v3[2];
        if ( (DWORD *)_this[20] == v3 )
        {
          break;
        }
      }
    }
    result = _this[12];
    v7 = 0;
    if ( result > 0 )
    {
      v8 = 0;
      do
      {
        FUN_00407d10(v8 + _this[13]);
        result = _this[12];
        ++v7;
        v8 += 60;
      }
      while ( v7 < result );
    }
  }
  return result;
}
#endif

// ── FUN_004093c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004093C0)
void __cdecl FUN_004093c0(void *_this)
{
  *(DWORD *)_this = &DAT_00552548;
  FUN_00408070(_this);
}
#endif

// ── FUN_00409ad0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409AD0)
DWORD *__cdecl FUN_00409ad0(DWORD *This)
{
  int v2; // eax
  int v3; // eax
  int v4; // ecx

  v2 = operator_new(0xCu);
  if ( v2 )
  {
    *(DWORD *)(v2 + 8) = 0;
    *(DWORD *)(v2 + 4) = 0;
  }
  else
  {
    v2 = 0;
  }
  This[2] = v2;
  v3 = operator_new(0xCu);
  if ( v3 )
  {
    *(DWORD *)(v3 + 8) = 0;
    *(DWORD *)(v3 + 4) = 0;
  }
  else
  {
    v3 = 0;
  }
  v4 = This[2];
  This[3] = v3;
  *(DWORD *)(v4 + 8) = v3;
  *(DWORD *)(This[3] + 4) = This[2];
  This[1] = 0;
  *This = &DAT_00552568;
  CWsctlc::LogPrintOn((DWORD)This);
  return This;
}
#endif

// ── FUN_00409b60 (IDA-only, gated) ──
#if defined(IDA_PORT_00409B60)
std::locale::_Locimp *__cdecl std::locale::_Locimp::`scalar deleting destructor'(
        std::locale::_Locimp *_this,
        unsigned int a2)
{
  std::locale::_Locimp::~_Locimp(_this);
  if ( (a2 & 1) != 0 )
  {
    delete__(_this);
  }
  return _this;
}
#endif

// ── FUN_00409b80 (IDA-only, gated) ──
#if defined(IDA_PORT_00409B80)
void __cdecl std::locale::_Locimp::~_Locimp(std::locale::_Locimp *_this)
{
  DWORD *v2; // esi
  void *i; // eax
  void *v4; // eax
  void *v5; // edi

  *(DWORD *)_this = &DAT_00552568;
  FUN_00409d20();
  *(DWORD *)(*(DWORD *)(*((DWORD *)_this + 3) + 4) + 8) = 0;
  v2 = *(DWORD **)(*((DWORD *)_this + 2) + 8);
  for ( i = v2; v2; i = v2 )
  {
    v2 = (DWORD *)v2[2];
    if ( i )
    {
      delete__(i);
    }
  }
  *(DWORD *)(*((DWORD *)_this + 2) + 8) = *((DWORD *)_this + 3);
  *(DWORD *)(*((DWORD *)_this + 3) + 4) = *((DWORD *)_this + 2);
  v4 = (void *)*((DWORD *)_this + 3);
  *((DWORD *)_this + 1) = 0;
  if ( v4 )
  {
    delete__(v4);
  }
  v5 = (void *)*((DWORD *)_this + 2);
  if ( v5 )
  {
    delete__(v5);
  }
}
#endif

// ── FUN_00409d20 (IDA-only, gated) ──
#if defined(IDA_PORT_00409D20)
int __cdecl FUN_00409d20(DWORD *_this)
{
  int *i; // esi
  DWORD *v3; // esi
  void *j; // eax
  int result; // eax

  for ( i = *(int **)(_this[2] + 8); (int *)_this[3] != i && i; i = (int *)i[2] )
  {
    FUN_004086e0(*i);
    if ( *i )
    {
      (**(void (__cdecl ***)(int, int))*i)(*i, 1);
    }
  }
  *(DWORD *)(*(DWORD *)(_this[3] + 4) + 8) = 0;
  v3 = *(DWORD **)(_this[2] + 8);
  for ( j = v3; v3; j = v3 )
  {
    v3 = (DWORD *)v3[2];
    if ( j )
    {
      delete__(j);
    }
  }
  *(DWORD *)(_this[2] + 8) = _this[3];
  result = _this[2];
  *(DWORD *)(_this[3] + 4) = result;
  _this[1] = 0;
  return result;
}
#endif

// ── FUN_00409db0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409DB0)
int __cdecl FUN_00409d20(DWORD *_this)
{
  int *i; // esi
  DWORD *v3; // esi
  void *j; // eax
  int result; // eax

  for ( i = *(int **)(_this[2] + 8); (int *)_this[3] != i && i; i = (int *)i[2] )
  {
    FUN_004086e0(*i);
    if ( *i )
    {
      (**(void (__cdecl ***)(int, int))*i)(*i, 1);
    }
  }
  *(DWORD *)(*(DWORD *)(_this[3] + 4) + 8) = 0;
  v3 = *(DWORD **)(_this[2] + 8);
  for ( j = v3; v3; j = v3 )
  {
    v3 = (DWORD *)v3[2];
    if ( j )
    {
      delete__(j);
    }
  }
  *(DWORD *)(_this[2] + 8) = _this[3];
  result = _this[2];
  *(DWORD *)(_this[3] + 4) = result;
  _this[1] = 0;
  return result;
}
#endif

// ── FUN_00409ea0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409EA0)
// Microsoft VisualC 2-14/net runtime
DWORD *FUN_00409ea0()
{
  int v0; // eax
  int v1; // eax
  int v2; // ecx

  v0 = operator_new(0xCu);
  if ( v0 )
  {
    *(DWORD *)(v0 + 8) = 0;
    *(DWORD *)(v0 + 4) = 0;
  }
  else
  {
    v0 = 0;
  }
  DAT_00590b00[1] = v0;
  v1 = operator_new(0xCu);
  if ( v1 )
  {
    *(DWORD *)(v1 + 8) = 0;
    *(DWORD *)(v1 + 4) = 0;
  }
  else
  {
    v1 = 0;
  }
  v2 = DAT_00590b00[1];
  DAT_00590b00[2] = v1;
  *(DWORD *)(v2 + 8) = v1;
  *(DWORD *)(DAT_00590b00[2] + 4) = DAT_00590b00[1];
  DAT_00590b00[0] = 0;
  return DAT_00590b00;
}
#endif

// ── FUN_00409eb0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409EB0)
// Microsoft VisualC 2-14/net runtime
DWORD *FUN_00409eb0()
{
  int v0; // eax
  int v1; // eax
  int v2; // ecx

  v0 = operator_new(0xCu);
  if ( v0 )
  {
    *(DWORD *)(v0 + 8) = 0;
    *(DWORD *)(v0 + 4) = 0;
  }
  else
  {
    v0 = 0;
  }
  DAT_00590b00[1] = v0;
  v1 = operator_new(0xCu);
  if ( v1 )
  {
    *(DWORD *)(v1 + 8) = 0;
    *(DWORD *)(v1 + 4) = 0;
  }
  else
  {
    v1 = 0;
  }
  v2 = DAT_00590b00[1];
  DAT_00590b00[2] = v1;
  *(DWORD *)(v2 + 8) = v1;
  *(DWORD *)(DAT_00590b00[2] + 4) = DAT_00590b00[1];
  DAT_00590b00[0] = 0;
  return DAT_00590b00;
}
#endif

// ── FUN_00409ed0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409ED0)
DWORD *__cdecl FUN_00409ed0(DWORD *_this)
{
  *_this = &DAT_00552574;
  FUN_00409f20();
  return _this;
}
#endif

// ── FUN_00409ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409EF0)
LPVOID __cdecl FUN_00409ef0(LPVOID lpMem, char a2)
{
  FUN_00409f10();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00409f10 (IDA-only, gated) ──
#if defined(IDA_PORT_00409F10)
void __cdecl FUN_00409f10(DWORD *_this)
{
  *_this = &DAT_00552574;
}
#endif

// ── FUN_00409f20 (IDA-only, gated) ──
#if defined(IDA_PORT_00409F20)
int __cdecl FUN_00409f20(int _this)
{
  int result; // eax

  result = 0;
  *(WORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = 0;
  *(DWORD *)(_this + 24) = 0;
  *(DWORD *)(_this + 28) = 0;
  return result;
}
#endif

// ── FUN_00409f30 (IDA-only, gated) ──
#if defined(IDA_PORT_00409F30)
int __cdecl FUN_00409f30(int a1, int a2, int a3, int a4, int a5, char a6)
{
  short v6; // cx
  int v7; // esi
  int v8; // eax
  int v9; // ebp
  int v10; // edx
  int v11; // ecx
  int v12; // eax
  int v13; // esi
  int v14; // ebp
  short v17; // [esp+8h] [ebp-4h]
  int v18; // [esp+18h] [ebp+Ch]

  if ( *(float *)(a5 + 360) < 0.0099999998 )
  {
    return 0;
  }
  (WORD)(a2) = *(WORD *)(a5 + 88);
  v6 = *(WORD *)(a5 + 100);
  v18 = a2;
  v17 = v6;
  if ( (WORD)a2 == 0xFFFE || v6 == -2 )
  {
    return 0;
  }
  v7 = 0;
  v8 = 0;
  v9 = *(short *)(a4 + 36);
  if ( v9 > 0 )
  {
    v10 = 0;
    do
    {
      if ( (short)a2 != v8
        && v17 != v8
        && (Bitmaps[*(short *)(*(DWORD *)(a4 + 56) + 2 * v8)].Components != 4 || !a6) )
      {
        (WORD)(v11) = *(WORD *)(*(DWORD *)(a4 + 40) + v10 + 10);
        if ( (v11 & 0x8000u) == 0 )
        {
          v11 = (short)v11;
        }
        else
        {
          v11 = 0;
        }
        v7 += v11;
      }
      (WORD)(a2) = v18;
      ++v8;
      v10 += 40;
    }
    while ( v8 < v9 );
  }
  *(DWORD *)(a1 + 24) = 0;
  v12 = operator_new(30 * v7);
  v13 = 0;
  *(DWORD *)(a1 + 28) = v12;
  if ( *(short *)(a4 + 36) > 0 )
  {
    v14 = 0;
    do
    {
      if ( (short)a2 != v13 && v17 != v13 )
      {
        (BYTE)(v18) = 0;
        if ( Bitmaps[*(short *)(*(DWORD *)(a4 + 56) + 2 * v13)].Components != 4 || ((BYTE)(v18) = 1, !a6) )
        {
          FUN_0040a1c0(
            v13,
            a3,
            *(WORD *)(v14 + *(DWORD *)(a4 + 40) + 10),
            *(DWORD *)(v14 + *(DWORD *)(a4 + 40) + 28),
            v18);
        }
      }
      ++v13;
      v14 += 40;
    }
    while ( v13 < *(short *)(a4 + 36) );
  }
  return 1;
}
#endif

// ── FUN_0040a0a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A0A0)
void __cdecl FUN_0040a0a0(int a1, int a2, int a3, int a4, int a5, char a6)
{
  *(DWORD *)(a1 + 16) = 1022739087;
  *(DWORD *)(a1 + 20) = -1082130432;
  *(DWORD *)(a1 + 12) = -1082130432;
  FUN_004f9d60(a1 + 12);
  if ( FUN_00409f30(a1, a2, a3, a4, a5, a6) )
  {
    FUN_0040a300(a3);
    delete__(*(LPVOID *)(a1 + 28));
  }
}
#endif

// ── FUN_0040a110 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A110)
int __cdecl FUN_0040a110(DWORD *_this, short a2, short a3, short a4, int a5, int a6, int a7)
{
  int v7; // esi
  short v8; // ax
  int result; // eax

  v7 = a7 + 36 * a5;
  v8 = *(WORD *)(v7 + 2 * a6 + 26);
  if ( v8 == -1 || (result = 9 * v8, !*(BYTE *)(a7 + 4 * result + 34)) )
  {
    *(WORD *)(_this[7] + 10 * _this[6]) = a2;
    *(WORD *)(_this[7] + 10 * _this[6] + 2) = a3;
    *(WORD *)(_this[7] + 10 * _this[6] + 4) = a4;
    *(WORD *)(_this[7] + 10 * _this[6] + 6) = *(WORD *)(v7 + 2 * a6 + 10);
    *(WORD *)(_this[7] + 10 * _this[6] + 8) = *(WORD *)(v7 + 2 * ((a6 + 1) % 3) + 10);
    result = _this[6] + 1;
    _this[6] = result;
  }
  return result;
}
#endif

// ── FUN_0040a1c0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A1C0)
int __cdecl FUN_0040a1c0(int _this, short a2, int a3, short a4, int a5, int a6)
{
  short *v7; // ebp
  int v8; // esi
  float *v9; // ecx
  float *v10; // edx
  int result; // eax
  int v12; // ebp
  int v13; // esi
  int v14; // [esp+10h] [ebp-1Ch]
  float Normal[3]; // [esp+14h] [ebp-18h] BYREF
  float *__attribute__((__org_arrdim(0,3))) v3; // [esp+28h] [ebp-4h]

  if ( a4 > 0 )
  {
    v14 = a4;
    v7 = (short *)(a5 + 6);
    v8 = 15000 * a2;
    do
    {
      v9 = (float *)(a3 + 12 * (v8 + *(v7 - 2)));
      v10 = (float *)(a3 + 12 * (v8 + *(v7 - 1)));
      v3 = (float *)(a3 + 12 * (v8 + *v7));
      FaceNormalize(v9, v10, v3, Normal);
      *((BYTE *)v7 + 28) = Normal[2] * *(float *)(_this + 20)
                          + Normal[1] * *(float *)(_this + 16)
                          + Normal[0] * *(float *)(_this + 12) <= 0.0;
      v7 += 18;
      --v14;
    }
    while ( v14 );
  }
  result = a4;
  v12 = 0;
  if ( a4 > 0 )
  {
    v13 = a5 + 6;
    do
    {
      if ( *(BYTE *)(v13 + 28) )
      {
        FUN_0040a110((DWORD *)_this, *(WORD *)(v13 - 4), *(WORD *)(v13 - 2), a2, v12, 0, a5);
        FUN_0040a110((DWORD *)_this, *(WORD *)(v13 - 2), *(WORD *)v13, a2, v12, 1, a5);
        FUN_0040a110((DWORD *)_this, *(WORD *)v13, *(WORD *)(v13 - 4), a2, v12, 2, a5);
      }
      result = a4;
      ++v12;
      v13 += 36;
    }
    while ( v12 < a4 );
  }
  return result;
}
#endif

// ── FUN_0040a300 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A300)
int __cdecl FUN_0040a300(int _this, int a2)
{
  int v3; // ebp
  int v4; // eax
  int result; // eax
  int v6; // ebx
  short *v7; // eax
  double v8; // st7
  double v9; // st7
  float scale; // [esp+0h] [ebp-4Ch]
  float scalea; // [esp+0h] [ebp-4Ch]
  float va[3]; // [esp+1Ch] [ebp-30h] BYREF
  float v13[3]; // [esp+28h] [ebp-24h] BYREF
  float vc[3]; // [esp+34h] [ebp-18h] BYREF
  float v15[3]; // [esp+40h] [ebp-Ch] BYREF

  v3 = 0;
  v4 = *(DWORD *)(_this + 24);
  *(WORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = operator_new(72 * v4);
  result = *(DWORD *)(_this + 24);
  if ( result > 0 )
  {
    v6 = 0;
    do
    {
      v7 = (short *)(v6 + *(DWORD *)(_this + 28));
      va[0] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]));
      va[1] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]) + 4);
      va[2] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]) + 8);
      v13[0] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]));
      v13[1] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]) + 4);
      v13[2] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]) + 8);
      if ( va[2] >= 22.5 )
      {
        v8 = va[2];
      }
      else
      {
        v8 = 22.5;
      }
      scale = -(v8 / *(float *)(_this + 20));
      VectorMA(va, scale, (float *)(_this + 12), vc);
      if ( v13[2] >= 22.5 )
      {
        v9 = v13[2];
      }
      else
      {
        v9 = 22.5;
      }
      scalea = -(v9 / *(float *)(_this + 20));
      VectorMA(v13, scalea, (float *)(_this + 12), v15);
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = va[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = va[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = va[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = vc[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = vc[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = vc[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v13[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v13[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v13[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v13[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v13[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v13[2];
      v6 += 10;
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = vc[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = vc[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = vc[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v15[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v15[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v15[2];
      result = *(DWORD *)(_this + 24);
      ++v3;
    }
    while ( v3 < result );
  }
  return result;
}
#endif

// ── FUN_0040a600 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A600)
int __cdecl FUN_0040a300(int _this, int a2)
{
  int v3; // ebp
  int v4; // eax
  int result; // eax
  int v6; // ebx
  short *v7; // eax
  double v8; // st7
  double v9; // st7
  float scale; // [esp+0h] [ebp-4Ch]
  float scalea; // [esp+0h] [ebp-4Ch]
  float va[3]; // [esp+1Ch] [ebp-30h] BYREF
  float v13[3]; // [esp+28h] [ebp-24h] BYREF
  float vc[3]; // [esp+34h] [ebp-18h] BYREF
  float v15[3]; // [esp+40h] [ebp-Ch] BYREF

  v3 = 0;
  v4 = *(DWORD *)(_this + 24);
  *(WORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = operator_new(72 * v4);
  result = *(DWORD *)(_this + 24);
  if ( result > 0 )
  {
    v6 = 0;
    do
    {
      v7 = (short *)(v6 + *(DWORD *)(_this + 28));
      va[0] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]));
      va[1] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]) + 4);
      va[2] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]) + 8);
      v13[0] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]));
      v13[1] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]) + 4);
      v13[2] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]) + 8);
      if ( va[2] >= 22.5 )
      {
        v8 = va[2];
      }
      else
      {
        v8 = 22.5;
      }
      scale = -(v8 / *(float *)(_this + 20));
      VectorMA(va, scale, (float *)(_this + 12), vc);
      if ( v13[2] >= 22.5 )
      {
        v9 = v13[2];
      }
      else
      {
        v9 = 22.5;
      }
      scalea = -(v9 / *(float *)(_this + 20));
      VectorMA(v13, scalea, (float *)(_this + 12), v15);
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = va[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = va[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = va[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = vc[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = vc[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = vc[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v13[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v13[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v13[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v13[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v13[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v13[2];
      v6 += 10;
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = vc[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = vc[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = vc[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v15[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v15[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v15[2];
      result = *(DWORD *)(_this + 24);
      ++v3;
    }
    while ( v3 < result );
  }
  return result;
}
#endif

// ── FUN_0040a6c0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A6C0)
LPVOID __cdecl FUN_0040a6c0(LPVOID lpMem, char a2)
{
  FUN_0040a6e0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0040a6e0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A6E0)
void __cdecl FUN_0040a6e0(DWORD *_this)
{
  *_this = &DAT_00552588;
  FUN_00409f10(_this);
}
#endif

// ── FUN_0040a830 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A830)
void __cdecl FUN_0040a830(int _this)
{
  void *v2; // esi

  if ( *(DWORD *)(_this + 28) )
  {
    delete__(*(LPVOID *)(_this + 28));
  }
  v2 = *(void **)(_this + 8);
  if ( v2 )
  {
    delete__(v2);
  }
}
#endif

// ── FUN_0040a8f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A8F0)
void __cdecl FUN_0040a8f0(char *_this, int a2, GLfloat a3, int a4, int a5)
{
  double v6; // st7
  float v7; // ecx
  float v8; // edx
  float v9; // edx
  double v10; // st7
  GLfloat x; // [esp+0h] [ebp-50h]
  GLfloat xa; // [esp+0h] [ebp-50h]
  GLfloat xb; // [esp+0h] [ebp-50h]
  GLfloat xc; // [esp+0h] [ebp-50h]
  GLfloat y; // [esp+4h] [ebp-4Ch]
  GLfloat ya; // [esp+4h] [ebp-4Ch]
  GLfloat yb; // [esp+4h] [ebp-4Ch]
  GLfloat yc; // [esp+4h] [ebp-4Ch]
  GLfloat z; // [esp+8h] [ebp-48h]
  GLfloat za; // [esp+8h] [ebp-48h]
  GLfloat zb; // [esp+8h] [ebp-48h]
  GLfloat zc; // [esp+8h] [ebp-48h]
  GLfloat t; // [esp+1Ch] [ebp-34h]
  float v24; // [esp+20h] [ebp-30h] BYREF
  float v25; // [esp+24h] [ebp-2Ch]
  float v26; // [esp+28h] [ebp-28h]
  float v27; // [esp+2Ch] [ebp-24h] BYREF
  float v28; // [esp+30h] [ebp-20h]
  float v29; // [esp+34h] [ebp-1Ch]
  float v30; // [esp+38h] [ebp-18h]
  float v31; // [esp+3Ch] [ebp-14h]
  float v32; // [esp+40h] [ebp-10h]
  float v33; // [esp+44h] [ebp-Ch]
  float v34; // [esp+48h] [ebp-8h]
  float v35; // [esp+4Ch] [ebp-4h]
  float v36; // [esp+54h] [ebp+4h]
  GLfloat v37; // [esp+58h] [ebp+8h]
  GLfloat v38; // [esp+58h] [ebp+8h]

  glColor3f(1.0, 1.0, 1.0);
  v27 = *(float *)LODWORD(a3) - *(float *)a2;
  v28 = *(float *)(LODWORD(a3) + 4) - *(float *)(a2 + 4);
  v29 = *(float *)(LODWORD(a3) + 8) - *(float *)(a2 + 8);
  v6 = FUN_004f9c40(&v27);
  v7 = *(float *)(a2 + 4);
  v8 = *(float *)(a2 + 8);
  v30 = *(float *)LODWORD(a3);
  v34 = v7;
  v31 = *(float *)(LODWORD(a3) + 4);
  v35 = v8;
  v9 = *(float *)(LODWORD(a3) + 8);
  v37 = (50.0 - v6) * 0.0099999998;
  v10 = *(float *)a2;
  v28 = v31 - v7;
  v29 = v9 - v35;
  v27 = (v30 - v10) * 0.1;
  v28 = v28 * 0.1;
  v29 = v29 * 0.1;
  v33 = v10 - v27;
  v34 = v7 - v28;
  v35 = v35 - v29;
  v30 = v30 + v27;
  v31 = v31 + v28;
  v32 = v9 + v29;
  v36 = (double)(rand() % 100) * 0.0099999998;
  glColor3f(1.0, 1.0, 1.0);
  BindTexture(494);
  EnableAlphaBlendMinus();
  FUN_004f9d20(_this + 12, &v27, &v24);
  FUN_004f9d60(&v24);
  v24 = v24 * 10.0;
  v25 = v25 * 10.0;
  v26 = v26 * 10.0;
  glBegin(7u);
  t = v36 + v37;
  glTexCoord2f(0.0, t);
  z = v35 - v26;
  y = v34 - v25;
  x = v33 - v24;
  glVertex3f(x, y, z);
  v38 = 1.0 - v37 + v36;
  glTexCoord2f(0.0, v38);
  za = v32 - v26;
  ya = v31 - v25;
  xa = v30 - v24;
  glVertex3f(xa, ya, za);
  glTexCoord2f(1.0, v38);
  zb = v26 + v32;
  yb = v25 + v31;
  xb = v24 + v30;
  glVertex3f(xb, yb, zb);
  glTexCoord2f(1.0, t);
  zc = v26 + v35;
  yc = v25 + v34;
  xc = v24 + v33;
  glVertex3f(xc, yc, zc);
  glEnd();
}
#endif

// ── FUN_0040b350 (IDA-only, gated) ──
#if defined(IDA_PORT_0040B350)
int FUN_0040b350()
{
  char i; // bl
  int result; // eax
  char *v2; // esi
  char k; // bl
  FILE *v4; // edx
  char *v5; // esi
  char j; // al
  FILE *v7; // edx
  char m; // bl
  FILE *v9; // edx
  char String[100]; // [esp+8h] [ebp-64h] BYREF

  DAT_00590b10 = 0;
  i = fgetc(fp);
  if ( i == -1 )
  {
    return 2;
  }
  while ( 1 )
  {
    if ( i == 47 )
    {
      i = fgetc(fp);
      if ( i == 47 )
      {
        for ( i = fgetc(fp); i != 10; i = fgetc(fp) )
        {
          ;
        }
      }
    }
    if ( !isspace(i) )
    {
      break;
    }
    i = fgetc(fp);
    if ( i == -1 )
    {
      return 2;
    }
  }
  switch ( i )
  {
    case '"':
      v5 = &DAT_00590b10;
      for ( j = getc(fp); j != -1; j = getc(v7) )
      {
        if ( j == 34 )
        {
          goto LABEL_25;
        }
        v7 = SMDFile;
        *v5++ = j;
      }
      ungetc(j, SMDFile);
      goto LABEL_25;
    case '#':
      result = 35;
      DAT_00809794 = 35;
      break;
    case ',':
      result = 44;
      DAT_00809794 = 44;
      break;
    case '-':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      ungetc(i, SMDFile);
      v2 = String;
      for ( k = getc(fp); k != -1; k = getc(v4) )
      {
        if ( k != 46 && !isdigit(k) && k != 45 )
        {
          break;
        }
        v4 = SMDFile;
        *v2++ = k;
      }
      *v2 = 0;
      DAT_00809798 = atof(String);
      result = 1;
      DAT_00809794 = 1;
      break;
    case ';':
      result = 59;
      DAT_00809794 = 59;
      break;
    case '{':
      result = 123;
      DAT_00809794 = 123;
      break;
    case '}':
      result = 125;
      DAT_00809794 = 125;
      break;
    default:
      if ( isalpha(i) )
      {
        DAT_00590b10 = i;
        v5 = (char *)&DAT_00590b11;
        for ( m = getc(fp); m != -1; m = getc(v9) )
        {
          if ( m != 46 && m != 95 && !isalnum(m) )
          {
            break;
          }
          v9 = SMDFile;
          *v5++ = m;
        }
        ungetc(m, SMDFile);
LABEL_25:
        *v5 = 0;
        DAT_00809794 = 0;
        result = 0;
      }
      else
      {
        result = 60;
        DAT_00809794 = 60;
      }
      break;
  }
  return result;
}
#endif

// ── FUN_0040c170 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C170)
void __cdecl TextureScript::setScript(DWORD This, DWORD That)
{
  *(BYTE *)This = *(BYTE *)That;
  *(BYTE *)(This + 1) = *(BYTE *)(That + 1);
  *(BYTE *)(This + 2) = *(BYTE *)(That + 2);
  *(BYTE *)(This + 3) = *(BYTE *)(That + 3);
}
#endif

// ── FUN_0040c190 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C190)
bool __cdecl TextureScriptParsing::parsingTScript(DWORD This, char *filename)
{
  char *v3; // eax
  char *v4; // ebp
  char *v5; // edx
  signed int v6; // edi
  int v7; // esi
  char Control[8]; // [esp+10h] [ebp-28h] BYREF
  char Str[32]; // [esp+18h] [ebp-20h] BYREF

  qmemcpy(Str, filename, sizeof(Str));
  strcpy(Control, "RHSN");
  v3 = strchr(Str, 95);
  v4 = v3;
  if ( v3 )
  {
    v5 = strtok(v3, Delimiter);
    v6 = strlen(v5) <= 5 ? strlen(v5) : 5;
    if ( strcspn(v5, Control) )
    {
      v7 = 1;
      if ( v6 > 1 )
      {
        while ( 2 )
        {
          switch ( v4[v7] )
          {
            case 'H':
              *(BYTE *)(This + 1) = 1;
              goto LABEL_12;
            case 'N':
              *(BYTE *)(This + 3) = 1;
              goto LABEL_12;
            case 'R':
              *(BYTE *)This = 1;
              goto LABEL_12;
            case 'S':
              *(BYTE *)(This + 2) = 1;
LABEL_12:
              ++v7;
              *(BYTE *)(This + 4) = 1;
              if ( v7 >= v6 )
              {
                return *(BYTE *)(This + 4);
              }
              continue;
            default:
              *(BYTE *)(This + 4) = 0;
              return 0;
          }
        }
      }
    }
  }
  return *(BYTE *)(This + 4);
}
#endif

// ── FUN_0040c2a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C2A0)
int __cdecl FUN_0040c2a0(LPCSTR lpString, int a2, int a3, int a4, size_t Count, int a6, int a7)
{
  const CHAR *v7; // edi
  int v8; // ebx
  size_t v10; // eax
  int v11; // eax
  int v12; // esi
  int v13; // ebp
  int v14; // ebx
  int v15; // esi
  double v16; // st7
  signed int j; // esi
  size_t i; // [esp+10h] [ebp-20h]
  int v19; // [esp+14h] [ebp-1Ch]
  char *Destination; // [esp+18h] [ebp-18h]
  int v21; // [esp+20h] [ebp-10h]
  struct tagSIZE sz; // [esp+28h] [ebp-8h] BYREF
  LPCSTR lpStringa; // [esp+34h] [ebp+4h]

  v7 = lpString;
  v8 = 0;
  if ( !lpString )
  {
    return 0;
  }
  lpStringa = 0;
  v10 = Count * (a4 - 1);
  for ( i = v10; ; i -= Count )
  {
    v21 = v8 + 1;
    if ( v8 + 1 > a4 )
    {
      break;
    }
    if ( a7 != 1 )
    {
      v10 = (size_t)lpStringa;
    }
    Destination = (char *)(a2 + v10);
    v11 = lstrlenA(v7);
    GetTextExtentPointA(m_hFontDC, v7, v11, &sz);
    v12 = (__int64)((double)sz.cx / g_fScreenRate_x);
    if ( !sz.cx )
    {
      return v8;
    }
    v13 = a3 - (v8 == 0 ? a6 : 0);
    if ( v12 <= v13 )
    {
      strncpy(Destination, v7, Count);
      return v8 + 1;
    }
    v14 = v13 / (v12 / lstrlenA(v7));
    v15 = (__int64)(((double)v14 + 1.0) * 0.5);
    v19 = v15;
    while ( v15 )
    {
      v15 = (__int64)(((double)v19 + 1.0) * 0.5);
      v19 = v15;
      GetTextExtentPointA(m_hFontDC, v7, v14, &sz);
      v16 = (double)sz.cx / g_fScreenRate_x;
      if ( v16 <= (double)(v13 + 4) )
      {
        if ( v16 >= (double)(v13 - 4) )
        {
          break;
        }
        v14 += v15;
      }
      else
      {
        v14 -= v15;
        if ( v15 == 1 )
        {
          break;
        }
      }
    }
    for ( j = 0; j < v14; j += _mbclen((const unsigned char *)&v7[j]) )
    {
      ;
    }
    strncpy(Destination, v7, j);
    lpStringa += Count;
    v7 += j;
    v10 = i - Count;
    Destination[j] = 0;
    v8 = v21;
  }
  return v8 + 1;
}
#endif

// ── FUN_0040c500 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C500)
int __cdecl FUN_0040c500(DWORD *_this, int a2, int a3, int a4)
{
  DWORD *v5; // esi
  DWORD *v6; // edi
  DWORD *v7; // eax
  DWORD *v8; // ecx
  DWORD *v9; // ecx
  DWORD *v10; // eax
  int result; // eax

  DAT_055c9b50 = a2;
  DAT_055c9b54 = a3;
  DAT_055c9b58 = a4;
  v5 = (DWORD *)_this[2];
  v6 = (DWORD *)v5[1];
  v7 = (DWORD *)operator_new(0x14u);
  v8 = v5;
  if ( !v5 )
  {
    v8 = v7;
  }
  *v7 = v8;
  v9 = v6;
  if ( !v6 )
  {
    v9 = v7;
  }
  v7[1] = v9;
  v5[1] = v7;
  *(DWORD *)v7[1] = v7;
  v10 = v7 + 2;
  if ( v10 )
  {
    *v10 = DAT_055c9b50;
    v10[1] = DAT_055c9b54;
    v10[2] = DAT_055c9b58;
  }
  result = _this[3] + 1;
  _this[3] = result;
  return result;
}
#endif

// ── FUN_0040c580 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C580)
int __cdecl FUN_0040c580(int _this)
{
  int result; // eax
  DWORD *v3; // eax
  DWORD **v4; // [esp-4h] [ebp-8h]

  result = *(DWORD *)(_this + 12);
  if ( result )
  {
    v3 = **(DWORD ***)(_this + 8);
    *(DWORD *)(_this + 16) = v3[2];
    *(DWORD *)(_this + 20) = v3[3];
    *(DWORD *)(_this + 24) = v3[4];
    v4 = **(DWORD ****)(_this + 8);
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
    result = *(DWORD *)(_this + 12) - 1;
    *(DWORD *)(_this + 12) = result;
  }
  return result;
}
#endif

// ── FUN_0040c5d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C5D0)
int __cdecl FUN_0040c5d0(int _this)
{
  DWORD *v2; // eax
  char v4; // [esp+Bh] [ebp-11h]

  *(BYTE *)(_this + 4) = v4;
  v2 = (DWORD *)operator_new(0x14u);
  *v2 = v2;
  v2[1] = v2;
  *(DWORD *)(_this + 8) = v2;
  *(DWORD *)(_this + 12) = 0;
  *(DWORD *)_this = &DAT_005525a0;
  *(DWORD *)(_this + 28) = FUN_0040c480();
  *(DWORD *)(_this + 32) = 0;
  FUN_0040c670(0);
  *(DWORD *)(_this + 40) = 0;
  FUN_0040c690((DWORD *)_this, 0, 0);
  FUN_0040c6b0(100, 100);
  FUN_0040c6d0(0, 0, 0);
  FUN_0040c6f0(0, 0, 0);
  *(DWORD *)(_this + 84) = 1;
  return _this;
}
#endif

// ── FUN_0040c670 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C670)
int __cdecl FUN_0040c670(DWORD *_this, int a2)
{
  int result; // eax

  result = a2;
  _this[9] = a2;
  return result;
}
#endif

// ── FUN_0040c6b0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C6B0)
int __cdecl FUN_0040c6b0(DWORD *_this, int a2, int a3)
{
  int result; // eax

  result = a2;
  _this[13] = a2;
  _this[14] = a3;
  return result;
}
#endif

// ── FUN_0040c6d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C6D0)
int __cdecl FUN_0040c6d0(DWORD *_this, int a2, int a3, int a4)
{
  int result; // eax

  _this[15] = a2;
  result = a4;
  _this[17] = a3;
  _this[18] = a4;
  return result;
}
#endif

// ── FUN_0040c6f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C6F0)
int __cdecl FUN_0040c6f0(DWORD *_this, int a2, int a3, int a4)
{
  int result; // eax

  _this[16] = a2;
  result = a4;
  _this[19] = a3;
  _this[20] = a4;
  return result;
}
#endif

// ── FUN_0040c710 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C710)
int __cdecl FUN_0040c710(DWORD *_this, int a2)
{
  int v4; // eax

  while ( _this[3] )
  {
    FUN_0040c580((int)_this);
    if ( !(*(int (__cdecl **)(DWORD *))(*_this + 36))(_this) )
    {
      (*(void (__cdecl **)(DWORD *))(*_this + 32))(_this);
    }
  }
  (*(void (__cdecl **)(DWORD *, int))(*_this + 24))(_this, a2);
  if ( a2 == 1 )
  {
    return 0;
  }
  if ( !FUN_0040c490(_this[11], _this[12], _this[13], _this[14], _this[21]) )
  {
    return (*(int (__cdecl **)(DWORD *))(*_this + 28))(_this);
  }
  if ( !DAT_055c9b80 )
  {
    if ( DAT_055c9b7c && DAT_055c9b7c != _this[7] )
    {
      return 0;
    }
    DAT_055c9b80 = _this[7];
  }
  if ( FUN_0040c680(_this) )
  {
    return (*(int (__cdecl **)(DWORD *))(*_this + 28))(_this);
  }
  v4 = _this[7];
  if ( DAT_055c9b80 == v4 || DAT_055c9b7c == v4 )
  {
    return (*(int (__cdecl **)(DWORD *))(*_this + 28))(_this);
  }
  else
  {
    return 0;
  }
}
#endif

// ── FUN_0040d550 (IDA-only, gated) ──
#if defined(IDA_PORT_0040D550)
void __cdecl FUN_0040d550(void *_this)
{
  DWORD **v2; // ebx
  DWORD *i; // edi
  DWORD **v4; // eax
  DWORD **v5; // ebx
  DWORD *j; // edi
  DWORD **v7; // [esp-4h] [ebp-18h]
  char v8[4]; // [esp+10h] [ebp-4h] BYREF

  *(DWORD *)_this = DAT_005525c8;
  v2 = (DWORD **)*((DWORD *)_this + 2);
  for ( i = *v2; i != v2; --*((DWORD *)_this + 3) )
  {
    v4 = (DWORD **)i;
    i = (DWORD *)*i;
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
  }
  v5 = (DWORD **)*((DWORD *)_this + 2);
  for ( j = *v5; j != v5; --*((DWORD *)_this + 3) )
  {
    j = (DWORD *)*j;
    v7 = *(DWORD ***)FUN_00410e30(v8, 0);
    *v7[1] = *v7;
    (*v7)[1] = v7[1];
    delete__(v7);
  }
  delete__(*((LPVOID *)_this + 2));
  *((DWORD *)_this + 2) = 0;
  *((DWORD *)_this + 3) = 0;
}
#endif

// ── FUN_0040db60 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DB60)
LPVOID __cdecl FUN_0040db60(LPVOID lpMem, char a2)
{
  FUN_0040d550(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0040db80 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DB80)
char *__cdecl FUN_0040db80(char *lpMem, char a2)
{
  FUN_0040dba0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0040dba0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DBA0)
int __cdecl FUN_0040dba0(char *_this)
{
  DWORD **v2; // ebx
  char *v3; // esi
  DWORD *v4; // edi
  void *v5; // eax
  int *v6; // ebx
  LPVOID *v7; // eax
  int *v8; // edi
  LPVOID *v9; // eax
  LPVOID *v11; // [esp-4h] [ebp-30h]
  int v12[2]; // [esp+10h] [ebp-1Ch] BYREF
  char v13[4]; // [esp+18h] [ebp-14h] BYREF
  char v14[4]; // [esp+1Ch] [ebp-10h] BYREF
  int v15; // [esp+28h] [ebp-4h]

  v12[1] = (int)_this;
  *(DWORD *)_this = &DAT_00552668;
  v2 = (DWORD **)*((DWORD *)_this + 23);
  v3 = _this + 88;
  v15 = 2;
  v4 = *v2;
  while ( v4 != v2 )
  {
    v5 = v4;
    v4 = (DWORD *)*v4;
    FUN_00411360(v12, v5);
  }
  *((DWORD *)_this + 34) = 0;
  (BYTE)(v15) = 1;
  v6 = (int *)*((DWORD *)_this + 31);
  v12[0] = *v6;
  while ( (int *)v12[0] != v6 )
  {
    v7 = (LPVOID *)FUN_00410e30(v13, 0);
    FUN_00411360(v14, *v7);
  }
  delete__(*((LPVOID *)_this + 31));
  *((DWORD *)_this + 31) = 0;
  *((DWORD *)_this + 32) = 0;
  v8 = (int *)*((DWORD *)v3 + 1);
  (BYTE)(v15) = 0;
  v12[0] = *v8;
  while ( (int *)v12[0] != v8 )
  {
    v9 = (LPVOID *)FUN_00410e30(v14, 0);
    FUN_00411360(v13, *v9);
  }
  delete__(*((LPVOID *)v3 + 1));
  *((DWORD *)v3 + 1) = 0;
  *((DWORD *)v3 + 2) = 0;
  *(DWORD *)_this = DAT_005525c8;
  v11 = (LPVOID *)*((DWORD *)_this + 2);
  v15 = 3;
  FUN_00410de0(v14, *v11, v11);
  v15 = -1;
  return FUN_00410d90(_this + 4);
}
#endif

// ── FUN_0040dce0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DCE0)
void __stdcall FUN_0040dce0(int Texture, int a2, float x, float y, float Width, float a6, GLfloat alpha, float a8)
{
  int v8; // eax
  float v9; // [esp+0h] [ebp-30h]
  float v10; // [esp+4h] [ebp-2Ch]
  float Heighta; // [esp+Ch] [ebp-24h]
  float Height; // [esp+Ch] [ebp-24h]
  float v13; // [esp+10h] [ebp-20h]
  int v14; // [esp+14h] [ebp-1Ch]
  float v15; // [esp+14h] [ebp-1Ch]
  float uWidth; // [esp+18h] [ebp-18h]
  float uWidtha; // [esp+18h] [ebp-18h]
  float vHeight; // [esp+1Ch] [ebp-14h]
  float vHeighta; // [esp+1Ch] [ebp-14h]
  float v20; // [esp+50h] [ebp+20h]

  if ( a2 == 1 )
  {
    if ( MouseLButton )
    {
      glColor4f(0.80000001, 0.80000001, 0.80000001, alpha);
      if ( a8 == 0.0 )
      {
        v14 = 0;
        Height = a6;
      }
      else
      {
        v14 = 1031798784;
        Height = a6 * -1.0;
      }
      v10 = y + 1.0;
      v9 = x + 1.0;
      uWidtha = (Width - 1.0) * 0.0625;
      vHeighta = (a6 - 1.0) * 0.0625;
      RenderBitmap(Texture, v9, v10, Width, Height, 0.0, *(float *)&v14, uWidtha, vHeighta, 1, 1);
      glColor4f(1.0, 1.0, 1.0, alpha);
    }
    else
    {
      v20 = (double)(LODWORD(a8) != 0 ? -1 : 1) * a6;
      v15 = a6 * 0.0625;
      v13 = Width * 0.0625;
      RenderBitmap(Texture, x, y, Width, v20, 0.0, 0.0, v13, v15, 1, 1);
      glColor4f(1.0, 1.0, 1.0, 0.1);
      RenderColor(x, y, Width, v20);
      glEnable(0xDE1u);
      glColor4f(1.0, 1.0, 1.0, alpha);
    }
  }
  else
  {
    vHeight = a6 * 0.0625;
    v8 = -(LODWORD(a8) != 0);
    (BYTE)(v8) = v8 & 0xFE;
    uWidth = Width * 0.0625;
    Heighta = (double)(v8 + 1) * a6;
    RenderBitmap(Texture, x, y, Width, Heighta, 0.0, 0.0, uWidth, vHeight, 1, 1);
  }
}
#endif

// ── FUN_0040def0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DEF0)
void __cdecl FUN_0040def0(int _this)
{
  int v2; // ecx
  double v3; // st7
  int v4; // edi
  double v5; // st7
  double v6; // st7
  float x; // [esp+0h] [ebp-34h]
  float y; // [esp+4h] [ebp-30h]
  float v9; // [esp+10h] [ebp-24h]
  float v10; // [esp+10h] [ebp-24h]
  int v11; // [esp+20h] [ebp-14h]
  char *v12; // [esp+24h] [ebp-10h]

  v2 = *(DWORD *)(_this + 192);
  if ( v2 == 1 && *(float *)(_this + 196) < 1.0 )
  {
    v3 = *(float *)(_this + 196) + 0.2;
LABEL_7:
    *(float *)(_this + 196) = v3;
    goto LABEL_8;
  }
  if ( !v2 && *(float *)(_this + 196) > 0.0 )
  {
    v3 = *(float *)(_this + 196) - 0.2;
    goto LABEL_7;
  }
LABEL_8:
  if ( *(float *)(_this + 196) > 0.0 && !InputEnable )
  {
    EnableAlphaTest(1);
    glColor4f(1.0, 1.0, 1.0, *(GLfloat *)(_this + 196));
    m_dwTextColor = -1;
    m_dwBackColor = -16777216;
    v4 = 0;
    if ( FUN_0040c490((__int64)ChatListBox_TabButtonsX, (__int64)ChatListBox_TabButtonsY, 16, 16, 1) )
    {
      v4 = 1;
    }
    else if ( FUN_0040c490((__int64)(ChatListBox_TabButtonSpacing + ChatListBox_TabButtonsX), (__int64)ChatListBox_TabButtonsY, 16, 16, 1) )
    {
      v4 = 2;
    }
    else if ( FUN_0040c490((__int64)(ChatListBox_TabButtonSpacing + ChatListBox_TabButtonSpacing + ChatListBox_TabButtonsX), (__int64)ChatListBox_TabButtonsY, 16, 16, 1) )
    {
      v4 = 3;
    }
    if ( DAT_00559bf1 )
    {
      FUN_0040dce0(1285, v4 == 1, ChatListBox_TabButtonsX, ChatListBox_TabButtonsY, 16.0, 16.0, *(GLfloat *)(_this + 196), 0.0);
    }
    else
    {
      glColor4f(0.69999999, 0.69999999, 0.69999999, *(GLfloat *)(_this + 196));
      y = ChatListBox_TabButtonsY + 1.0;
      x = ChatListBox_TabButtonsX + 1.0;
      RenderBitmap(1285, x, y, 16.0, 16.0, 0.0, 0.0, 0.9375, 0.9375, 1, 1);
      glColor4f(1.0, 1.0, 1.0, *(GLfloat *)(_this + 196));
    }
    v9 = ChatListBox_TabButtonSpacing + ChatListBox_TabButtonsX;
    FUN_0040dce0(1286, v4 == 2, v9, ChatListBox_TabButtonsY, 16.0, 16.0, *(GLfloat *)(_this + 196), 0.0);
    v10 = ChatListBox_TabButtonSpacing + ChatListBox_TabButtonSpacing + ChatListBox_TabButtonsX;
    FUN_0040dce0(1287, v4 == 3, v10, ChatListBox_TabButtonsY, 16.0, 16.0, *(GLfloat *)(_this + 196), 0.0);
    if ( v4 == 1 )
    {
      v12 = GlobalText[750];
      v5 = ChatListBox_TabButtonsX;
      v11 = (__int64)(ChatListBox_TabButtonsY - 10.0);
    }
    else
    {
      if ( v4 == 2 )
      {
        v12 = GlobalText[751];
        v6 = ChatListBox_TabButtonSpacing;
        v11 = (__int64)(ChatListBox_TabButtonsY - 10.0);
      }
      else
      {
        if ( v4 != 3 )
        {
LABEL_27:
          glColor4f(1.0, 1.0, 1.0, 1.0);
          DisableAlphaBlend();
          return;
        }
        v12 = GlobalText[752];
        v11 = (__int64)(ChatListBox_TabButtonsY - 10.0);
        v6 = ChatListBox_TabButtonSpacing + ChatListBox_TabButtonSpacing;
      }
      v5 = v6 + ChatListBox_TabButtonsX;
    }
    RenderTipText((__int64)(v5 - 16.0), v11, v12);
    goto LABEL_27;
  }
}
#endif

// ── FUN_0040e230 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E230)
BOOL __cdecl FUN_0040e230(DWORD *_this)
{
  int v2; // eax

  v2 = _this[28];
  if ( v2 != _this[23] && MouseRButton )
  {
    strncpy(InputText[1], (const char *)(v2 + 8), 0x100u);
    InputLength[1] = strlen(InputText[1]);
  }
  _this[28] = _this[23];
  if ( FUN_0040c490(160, 436, 320, 50, 1) )
  {
    _this[48] = 1;
  }
  else if ( MouseY < 416 )
  {
    _this[48] = 0;
  }
  return FUN_0040c490(_this[13] + _this[11] - 30, _this[12] - 6, 33, _this[14] - 2, 2)
      || _this[48] == 1 && FUN_0040c490((__int64)ChatListBox_TabButtonsX, (__int64)ChatListBox_TabButtonsY, (__int64)(ChatListBox_TabButtonSpacing * 3.0), 16, 1);
}
#endif

// ── FUN_0040e400 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E400)
int __cdecl FUN_0040e400(float *_this)
{
  double v2; // st7

  if ( FUN_0040c490(*((DWORD *)_this + 11), *((DWORD *)_this + 12) - *((DWORD *)_this + 14), *((DWORD *)_this + 13), 8, 1)
    && !FUN_0040c680(_this)
    && !DAT_055c9b7c )
  {
    DAT_055c9b7c = *((DWORD *)_this + 7);
    (*(void (__cdecl **)(float *, int))(*(DWORD *)_this + 4))(_this, 1);
    PlayBuffer(25, 0, 0);
  }
  if ( *((DWORD *)_this + 48) && !InputEnable )
  {
    if ( FUN_0040c490((__int64)ChatListBox_TabButtonsX, (__int64)ChatListBox_TabButtonsY, 16, 16, 1) )
    {
      DAT_00559bf1 = DAT_00559bf1 == 0;
      PlayBuffer(25, 0, 0);
      (*(void (__cdecl **)(float *, DWORD))(*(DWORD *)_this + 48))(_this, 0);
      MouseLButtonPush = 0;
    }
    if ( FUN_0040c490((__int64)(ChatListBox_TabButtonSpacing + ChatListBox_TabButtonsX), (__int64)ChatListBox_TabButtonsY, 16, 16, 1) )
    {
      FUN_0040e330(_this);
      PlayBuffer(25, 0, 0);
      MouseLButtonPush = 0;
    }
    if ( FUN_0040c490((__int64)(ChatListBox_TabButtonSpacing + ChatListBox_TabButtonSpacing + ChatListBox_TabButtonsX), (__int64)ChatListBox_TabButtonsY, 16, 16, 1) )
    {
      v2 = _this[47] + 0.2;
      _this[47] = v2;
      if ( v2 > 0.89999998 )
      {
        _this[47] = 0.2;
      }
      PlayBuffer(25, 0, 0);
      MouseLButtonPush = 0;
    }
  }
  return 1;
}
#endif

// ── FUN_0040e5b0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E5B0)
void __cdecl FUN_0040e5b0(const char *_this, char *Source)
{
  int v3; // eax
  char *v4; // edi
  char *v5; // edx
  char *Destination; // [esp+10h] [ebp-10Ch]
  int v7; // [esp+14h] [ebp-108h]
  int v8; // [esp+18h] [ebp-104h]
  char String[253]; // [esp+1Ch] [ebp-100h] BYREF
  short v10; // [esp+119h] [ebp-3h]
  char v11; // [esp+11Bh] [ebp-1h]

  if ( Source )
  {
    v3 = 0;
    if ( strlen(Source) <= 0xFF )
    {
      (BYTE)(v3) = _this[200] != 0;
      v8 = v3;
      memset(String, 0, sizeof(String));
      v10 = 0;
      v11 = 0;
      strncpy(String, Source, 0x100u);
      strtok(String, DAT_005590ec);
      v4 = (char *)(_this + 200);
      v7 = 0;
      Destination = (char *)(_this + 200);
      do
      {
        v5 = strtok(0, DAT_005590ec);
        if ( v5 )
        {
          if ( strlen(v5)
             + strlen(_this + 200)
             + strlen(_this + 456)
             + strlen(_this + 712)
             + strlen(_this + 968)
             + strlen(_this + 1224) <= 0x1E )
          {
            v4 = Destination;
            strncpy(Destination, v5, 0x100u);
          }
          else
          {
            if ( !v7 )
            {
              return;
            }
            v4 = Destination;
            *Destination = 0;
          }
        }
        else
        {
          *v4 = 0;
        }
        v4 += 256;
        Destination = v4;
        ++v7;
      }
      while ( v7 < 5 );
      if ( _this[200] )
      {
        UIChatLogWindow_AddText(&strID, GlobalText[755], 1);
      }
      else if ( v8 == 1 )
      {
        UIChatLogWindow_AddText(&strID, GlobalText[756], 1);
      }
    }
  }
}
#endif

// ── FUN_0040e730 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E730)
int __cdecl FUN_0040e730(const char *_this, char *Text)
{
  const char *v2; // esi
  int i; // edi

  v2 = _this + 200;
  if ( !_this[200] )
  {
    return 1;
  }
  for ( i = 0; i < 5; ++i )
  {
    if ( !*v2 )
    {
      break;
    }
    if ( FindText(Text, v2, 0) )
    {
      return 1;
    }
    v2 += 256;
  }
  return 0;
}
#endif

// ── FUN_0040e780 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E780)
char __cdecl FUN_0040e780(BYTE *_this, char *a2)
{
  char result; // al
  int v3; // ebx
  BYTE *v4; // edx

  *a2 = 0;
  result = _this[200];
  if ( result )
  {
    v3 = 0;
    v4 = _this + 456;
    do
    {
      result = 0;
      strcat(a2, v4 - 256);
      if ( !*v4 )
      {
        break;
      }
      result = 0;
      v4 += 256;
      strcat(a2, DAT_005590f0);
      ++v3;
    }
    while ( v3 < 5 );
  }
  return result;
}
#endif

// ── FUN_0040eac0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040EAC0)
char *__cdecl FUN_0040eac0(char *lpMem, char a2)
{
  FUN_0040eae0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0040eae0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040EAE0)
int __cdecl FUN_0040eae0(char *_this)
{
  DWORD **v2; // ebx
  char *v3; // esi
  DWORD *v4; // edi
  void *v5; // eax
  int *v6; // ebx
  LPVOID *v7; // eax
  int *v8; // edi
  LPVOID *v9; // eax
  LPVOID *v11; // [esp-4h] [ebp-30h]
  int v12[2]; // [esp+10h] [ebp-1Ch] BYREF
  char v13[4]; // [esp+18h] [ebp-14h] BYREF
  char v14[4]; // [esp+1Ch] [ebp-10h] BYREF
  int v15; // [esp+28h] [ebp-4h]

  v12[1] = (int)_this;
  *(DWORD *)_this = &DAT_00552760;
  v2 = (DWORD **)*((DWORD *)_this + 23);
  v3 = _this + 88;
  v15 = 2;
  v4 = *v2;
  while ( v4 != v2 )
  {
    v5 = v4;
    v4 = (DWORD *)*v4;
    FUN_00411360(v12, v5);
  }
  *((DWORD *)_this + 34) = 0;
  (BYTE)(v15) = 1;
  v6 = (int *)*((DWORD *)_this + 31);
  v12[0] = *v6;
  while ( (int *)v12[0] != v6 )
  {
    v7 = (LPVOID *)FUN_00410e30(v13, 0);
    FUN_00411360(v14, *v7);
  }
  delete__(*((LPVOID *)_this + 31));
  *((DWORD *)_this + 31) = 0;
  *((DWORD *)_this + 32) = 0;
  v8 = (int *)*((DWORD *)v3 + 1);
  (BYTE)(v15) = 0;
  v12[0] = *v8;
  while ( (int *)v12[0] != v8 )
  {
    v9 = (LPVOID *)FUN_00410e30(v14, 0);
    FUN_00411360(v13, *v9);
  }
  delete__(*((LPVOID *)v3 + 1));
  *((DWORD *)v3 + 1) = 0;
  *((DWORD *)v3 + 2) = 0;
  *(DWORD *)_this = DAT_005525c8;
  v11 = (LPVOID *)*((DWORD *)_this + 2);
  v15 = 3;
  FUN_00410de0(v14, *v11, v11);
  v15 = -1;
  return FUN_00410d90(_this + 4);
}
#endif

// ── FUN_0040f520 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F520)
DWORD *__cdecl FUN_0040f520(DWORD *lpMem, char a2)
{
  FUN_0040f540(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0040f540 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F540)
DWORD *__cdecl FUN_0040f540(DWORD *_this)
{
  DWORD *result; // eax
  int v2; // ecx

  result = _this;
  v2 = _this[1];
  *result = &DAT_005527e0;
  if ( v2 )
  {
    return (DWORD *)(*(int (__cdecl **)(int, int))(*(DWORD *)v2 + 20))(v2, 1);
  }
  return result;
}
#endif

// ── FUN_0040f650 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F650)
int __cdecl CUIRenderText::SetFont(DWORD **_this, int a2)
{
  int result; // eax

  result = a2;
  if ( a2 )
  {
    return (*(int (__cdecl **)(DWORD *, int))(*_this[1] + 8))(_this[1], a2);
  }
  return result;
}
#endif

// ── FUN_0040f680 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F680)
DWORD *__cdecl FUN_0040f680(DWORD *_this)
{
  DWORD *result; // eax

  result = _this;
  *_this = DAT_005527e4;
  return result;
}
#endif

// ── FUN_0040f690 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F690)
void __cdecl FUN_0040f690(DWORD *_this)
{
  *_this = DAT_005527e4;
}
#endif

// ── FUN_0040f730 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F730)
int __cdecl FUN_0040f730(int _this)
{
  DWORD *v2; // ebp
  DWORD *v3; // eax
  DWORD *v4; // eax
  char v6; // [esp+13h] [ebp-19h]
  void *lpMem; // [esp+14h] [ebp-18h]
  char v8[4]; // [esp+18h] [ebp-14h] BYREF
  int v9; // [esp+1Ch] [ebp-10h]
  int v10; // [esp+28h] [ebp-4h]

  v9 = _this;
  FUN_0040f680((DWORD *)_this);
  v10 = 0;
  *(BYTE *)(_this + 4) = v6;
  *(BYTE *)(_this + 5) = v6;
  *(BYTE *)(_this + 12) = 1;
  v2 = (DWORD *)operator_new(0x138u);
  lpMem = v2;
  v2[1] = 0;
  v2[77] = 1;
  std::_Lockit::_Lockit((std::_Lockit *)v8);
  if ( !DAT_055c9b98 )
  {
    DAT_055c9b98 = (int)v2;
    *v2 = 0;
    lpMem = 0;
    *(DWORD *)(DAT_055c9b98 + 8) = 0;
  }
  ++DAT_055c9b94;
  std::_Lockit::~_Lockit((std::_Lockit *)v8);
  if ( lpMem )
  {
    delete__(lpMem);
  }
  v3 = (DWORD *)FUN_00411840(DAT_055c9b98, 0);
  *(DWORD *)(_this + 8) = v3;
  *(DWORD *)(_this + 16) = 0;
  *v3 = v3;
  *(DWORD *)(*(DWORD *)(_this + 8) + 8) = *(DWORD *)(_this + 8);
  (BYTE)(v10) = 1;
  *(BYTE *)(_this + 24) = v6;
  v4 = (DWORD *)operator_new(0xCu);
  *v4 = v4;
  v4[1] = v4;
  *(DWORD *)(_this + 28) = v4;
  *(DWORD *)(_this + 32) = 0;
  *(DWORD *)_this = &DAT_005527f8;
  *(DWORD *)(_this + 204) = 0;
  return _this;
}
#endif

// ── FUN_0040f840 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F840)
LPVOID __cdecl FUN_0040f840(LPVOID lpMem, char a2)
{
  FUN_0040f950();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0040f860 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F860)
void __cdecl FUN_0040f860(int _this)
{
  int *v2; // edi
  int v3; // ecx
  int v4; // eax
  LPVOID *i; // ebx
  int v6; // eax
  LPVOID *v7; // edi
  DWORD *v8; // eax
  int v9; // ebx
  void *v10; // esi
  int *v11; // [esp+10h] [ebp-Ch] BYREF
  char v12[4]; // [esp+14h] [ebp-8h] BYREF
  char v13[4]; // [esp+18h] [ebp-4h] BYREF

  v2 = *(int **)(_this + 4);
  v3 = *(DWORD *)(_this + 12);
  v4 = *v2;
  v11 = (int *)*v2;
  if ( v3 )
  {
    v6 = DAT_055c9b98;
    v7 = (LPVOID *)v2[1];
    for ( i = v7; v7 != (LPVOID *)DAT_055c9b98; i = v7 )
    {
      FUN_00411420(v7[2]);
      v7 = (LPVOID *)*v7;
      delete__(i);
      v6 = DAT_055c9b98;
    }
    *(DWORD *)(*(DWORD *)(_this + 4) + 4) = v6;
    v8 = *(DWORD **)(_this + 4);
    *(DWORD *)(_this + 12) = 0;
    *v8 = v8;
    *(DWORD *)(*(DWORD *)(_this + 4) + 8) = *(DWORD *)(_this + 4);
    FUN_00410e40(v12);
  }
  else if ( (int *)v4 != v2 )
  {
    do
    {
      v9 = v4;
      FUN_004112b0(&v11);
      FUN_00410e50(v13, v9);
      v4 = (int)v11;
    }
    while ( v11 != v2 );
  }
  delete__(*(LPVOID *)(_this + 4));
  *(DWORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 12) = 0;
  v10 = 0;
  std::_Lockit::_Lockit((std::_Lockit *)&v11);
  if ( !--DAT_055c9b94 )
  {
    v10 = (void *)DAT_055c9b98;
    DAT_055c9b98 = 0;
  }
  std::_Lockit::~_Lockit((std::_Lockit *)&v11);
  if ( v10 )
  {
    delete__(v10);
  }
}
#endif

// ── FUN_0040f950 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F950)
void __cdecl FUN_0040f950(DWORD *_this)
{
  int *v2; // eax
  int v3; // ecx
  DWORD *v4; // esi
  int *v5; // edi
  int v6; // eax
  int **v7; // ecx
  int v8; // ecx
  DWORD *v9; // eax
  DWORD *v10; // eax
  DWORD **v11; // ebx
  DWORD *v12; // ebp
  DWORD *v13; // edi
  void *v14; // eax
  int v15; // ecx
  int *v16; // edi
  int v17; // eax
  int **v18; // edx
  int v19; // ecx
  DWORD *v20; // eax
  DWORD *v21; // eax
  void *v22; // esi
  int *v23; // [esp+10h] [ebp-20h] BYREF
  DWORD *v24; // [esp+14h] [ebp-1Ch]
  char v25[4]; // [esp+18h] [ebp-18h] BYREF
  char v26[4]; // [esp+1Ch] [ebp-14h] BYREF
  char v27[4]; // [esp+20h] [ebp-10h] BYREF
  int v28; // [esp+2Ch] [ebp-4h]

  v24 = _this;
  *_this = &DAT_005527f8;
  v2 = (int *)_this[2];
  v3 = _this[4];
  v4 = _this + 1;
  v5 = v2;
  v6 = *v2;
  v28 = 2;
  v23 = (int *)v6;
  if ( v3 && (v7 = (int **)FUN_00410e40(v25), v6 = (int)v23, v23 == *v7) && (v8 = _this[2], v5 == (int *)v8) )
  {
    FUN_00411420(*(LPVOID *)(v8 + 4));
    *(DWORD *)(_this[2] + 4) = DAT_055c9b98;
    v9 = (DWORD *)_this[2];
    _this[4] = 0;
    *v9 = v9;
    *(DWORD *)(_this[2] + 8) = _this[2];
    FUN_00410e40(v26);
  }
  else if ( (int *)v6 != v5 )
  {
    do
    {
      v10 = (DWORD *)FUN_004117c0(v25, 0);
      FUN_00410e50(v27, *v10);
    }
    while ( v23 != v5 );
  }
  v11 = (DWORD **)_this[7];
  v12 = _this + 6;
  (BYTE)(v28) = 1;
  v13 = *v11;
  while ( v13 != v11 )
  {
    v14 = v13;
    v13 = (DWORD *)*v13;
    FUN_00411360(v27, v14);
  }
  delete__((LPVOID)v12[1]);
  v12[1] = 0;
  v12[2] = 0;
  v15 = v4[3];
  v16 = (int *)v4[1];
  v17 = *v16;
  (BYTE)(v28) = 0;
  v23 = (int *)v17;
  if ( v15 && (v18 = (int **)FUN_00410e40(v27), v17 = (int)v23, v23 == *v18) && (v19 = v4[1], v16 == (int *)v19) )
  {
    FUN_00411420(*(LPVOID *)(v19 + 4));
    *(DWORD *)(v4[1] + 4) = DAT_055c9b98;
    v20 = (DWORD *)v4[1];
    v4[3] = 0;
    *v20 = v20;
    *(DWORD *)(v4[1] + 8) = v4[1];
    FUN_00410e40(v26);
  }
  else if ( (int *)v17 != v16 )
  {
    do
    {
      v21 = (DWORD *)FUN_004117c0(v27, 0);
      FUN_00410e50(v25, *v21);
    }
    while ( v23 != v16 );
  }
  delete__((LPVOID)v4[1]);
  v4[1] = 0;
  v4[3] = 0;
  v22 = 0;
  std::_Lockit::_Lockit((std::_Lockit *)&v23);
  if ( !--DAT_055c9b94 )
  {
    v22 = (void *)DAT_055c9b98;
    DAT_055c9b98 = 0;
  }
  std::_Lockit::~_Lockit((std::_Lockit *)&v23);
  if ( v22 )
  {
    delete__(v22);
  }
  FUN_0040f690(v24);
}
#endif

// ── FUN_0040fb70 (IDA-only, gated) ──
#if defined(IDA_PORT_0040FB70)
double __cdecl FUN_0040fb70(char *_this, int a2, int a3, LPCSTR lpString, int a5, int a6, int a7, int a8, int a9)
{
  int v10; // eax
  int v11; // esi
  double v12; // st7
  double v13; // st6
  int v14; // edi
  struct tagSIZE sz; // [esp+Ch] [ebp-8h] BYREF
  LPCSTR lpStringa; // [esp+20h] [ebp+Ch]

  if ( !lpString || !*lpString || !strlen(lpString) && !a5 )
  {
    return 0.0;
  }
  v10 = lstrlenA(lpString);
  GetTextExtentPointA(m_hFontDC, lpString, v10, &sz);
  v11 = 0;
  lpStringa = (LPCSTR)sz.cx;
  switch ( a7 )
  {
    case 1:
      if ( a5 > 0 )
      {
        lpStringa = (LPCSTR)a5;
      }
      break;
    case 2:
      v11 = (a5 - sz.cx) / 2;
      lpStringa = (LPCSTR)(sz.cx + 2 * v11);
      break;
    case 3:
      v11 = a5 - sz.cx;
      lpStringa = (LPCSTR)a5;
      break;
  }
  v12 = (double)(int)lpStringa / g_fScreenRate_x;
  v13 = (double)a9;
  if ( (double)a2 + v12 <= v13 )
  {
    v14 = a2;
  }
  else
  {
    v14 = (__int64)(v13 - v12);
  }
  if ( !FUN_004106d0(lpString, v14, a3, v11) )
  {
    FUN_0040fcd0(_this, (char *)lpString, a5, a6, v11, a7, a8);
    FUN_004106d0(lpString, v14, a3, v11);
  }
  if ( *lpString == 10 )
  {
    return (double)sz.cy / g_fScreenRate_y / 2.0;
  }
  else
  {
    return (double)sz.cy / g_fScreenRate_y / 1.0;
  }
}
#endif

// ── FUN_0040fcd0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040FCD0)
void __cdecl FUN_0040fcd0(char *_this, char *Source, int a3, int a4, int x, int a6, int a7)
{
  int v8; // eax
  int cx; // eax
  int cy; // ecx
  int v11; // ebp
  signed int i; // ebp
  int v13; // eax
  GLuint v14; // edi
  int v15; // edx
  int j; // ecx
  char *v17; // ebx
  bool v18; // al
  DWORD *v19; // ecx
  DWORD *v20; // edi
  DWORD *v21; // ebp
  DWORD *v22; // esi
  DWORD *v23; // eax
  int v24; // eax
  DWORD *v25; // eax
  DWORD *v26; // eax
  DWORD *v27; // edx
  DWORD *v28; // ecx
  int v29; // ecx
  int v30; // [esp-Ch] [ebp-284h]
  int v31; // [esp-Ch] [ebp-284h]
  unsigned int v32; // [esp+Ch] [ebp-26Ch] BYREF
  char v33; // [esp+11h] [ebp-267h] BYREF
  char v34; // [esp+12h] [ebp-266h] BYREF
  char v35; // [esp+13h] [ebp-265h] BYREF
  int v36; // [esp+14h] [ebp-264h] BYREF
  int v37; // [esp+18h] [ebp-260h] BYREF
  struct tagSIZE sz; // [esp+24h] [ebp-254h] BYREF
  char Destination[292]; // [esp+2Ch] [ebp-24Ch] BYREF
  int v40; // [esp+150h] [ebp-128h] BYREF
  char v41[292]; // [esp+154h] [ebp-124h] BYREF

  if ( Source && strlen(Source) <= 0xFF )
  {
    memset(Destination, 0, sizeof(Destination));
    strncpy(Destination, Source, 0x100u);
    FUN_004104b0((LONG)_this, Source);
    *(DWORD *)&Destination[272] = m_dwTextColor;
    *(DWORD *)&Destination[276] = m_dwBackColor;
    *(DWORD *)&Destination[264] = x;
    v8 = lstrlenA(Source);
    GetTextExtentPointA(m_hFontDC, Source, v8, &sz);
    cx = a3;
    if ( a3 )
    {
      v37 = a3;
    }
    else
    {
      cx = sz.cx;
      v37 = sz.cx;
    }
    cy = a4;
    if ( !a4 )
    {
      cy = sz.cy;
    }
    v36 = cy;
    if ( *Source != 10 )
    {
      v11 = 0;
      if ( v36 > 0 )
      {
        v32 = 3 * cx;
        do
        {
          memset((char *)ppvBits + 768 * v11 * DAT_005590bc, 0, v32);
          ++v11;
        }
        while ( v11 < v36 );
      }
      SetTextColor(m_hFontDC, (COLORREF)&DAT_00ffffff);
      TextOutA(m_hFontDC, x, 0, Source, strlen(Source));
      cx = v37;
    }
    *(DWORD *)&Destination[256] = cx;
    *(DWORD *)&Destination[260] = v36;
    if ( cx <= 256 )
    {
      v32 = 1;
    }
    else
    {
      v32 = 2;
      *(DWORD *)&Destination[288] = 1;
    }
    for ( i = 0; i < (int)v32; ++i )
    {
      if ( v32 == 1 )
      {
        v13 = v37;
      }
      else if ( i )
      {
        v13 = v37 - 256;
      }
      else
      {
        v13 = 256;
      }
      FUN_004105f0(Bitmaps[0].Buffer, i << 8, v13, v36);
      v14 = FUN_00410270(_this);
      glBindTexture(0xDE1u, v14);
      glPixelStorei(0xCF5u, 1);
      glTexEnvf(0x2300u, 0x2200u, 8448.0);
      glTexParameteri(0xDE1u, 0x2800u, 9728);
      glTexParameteri(0xDE1u, 0x2801u, 9728);
      glTexParameteri(0xDE1u, 0x2802u, 10496);
      glTexParameteri(0xDE1u, 0x2803u, 10496);
      glTexImage2D(
        0xDE1u,
        0,
        Bitmaps[0].Components,
        (__int64)Bitmaps[0].Width,
        (__int64)Bitmaps[0].Height,
        0,
        0x1908u,
        0x1401u,
        Bitmaps[0].Buffer);
      if ( i )
      {
        *(DWORD *)&Destination[284] = v14;
      }
      else
      {
        *(DWORD *)&Destination[280] = v14;
      }
    }
    v15 = 0;
    for ( j = 0; j < 4; ++j )
    {
      if ( !Destination[j] )
      {
        break;
      }
      v15 += (unsigned char)Destination[j];
    }
    v40 = v15;
    v17 = _this + 4;
    v18 = 1;
    qmemcpy(v41, Destination, sizeof(v41));
    v19 = (DWORD *)*((DWORD *)v17 + 1);
    v20 = v19;
    v21 = (DWORD *)v19[1];
    while ( v21 != (DWORD *)DAT_055c9b98 )
    {
      v20 = v21;
      v18 = v15 < v21[3];
      if ( v15 >= v21[3] )
      {
        v21 = (DWORD *)v21[2];
      }
      else
      {
        v21 = (DWORD *)*v21;
      }
    }
    if ( v17[8] )
    {
      v22 = (DWORD *)FUN_00411840(v20, 0);
      *v22 = DAT_055c9b98;
      v22[2] = DAT_055c9b98;
      FUN_004124d0(v22 + 3, &v40);
      v23 = (DWORD *)*((DWORD *)v17 + 1);
      ++*((DWORD *)v17 + 3);
      if ( v20 == v23 || v21 != (DWORD *)DAT_055c9b98 || v40 < v20[3] )
      {
        *v20 = v22;
        v25 = (DWORD *)*((DWORD *)v17 + 1);
        if ( v20 == v25 )
        {
          v25[1] = v22;
          *(DWORD *)(*((DWORD *)v17 + 1) + 8) = v22;
        }
        else if ( v20 == (DWORD *)*v25 )
        {
          *v25 = v22;
        }
      }
      else
      {
        v20[2] = v22;
        v24 = *((DWORD *)v17 + 1);
        if ( v20 == *(DWORD **)(v24 + 8) )
        {
          *(DWORD *)(v24 + 8) = v22;
        }
      }
      while ( v22 != *(DWORD **)(*((DWORD *)v17 + 1) + 4) )
      {
        v26 = (DWORD *)v22[1];
        if ( v26[77] )
        {
          break;
        }
        v27 = (DWORD *)v26[1];
        v28 = (DWORD *)*v27;
        if ( v26 == (DWORD *)*v27 )
        {
          v29 = v27[2];
          if ( *(DWORD *)(v29 + 308) )
          {
            if ( v22 == (DWORD *)v26[2] )
            {
              v22 = (DWORD *)v22[1];
              FUN_00411700(v26);
            }
            *(DWORD *)(v22[1] + 308) = 1;
            *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
            FUN_00411760(*(DWORD *)(v22[1] + 4));
          }
          else
          {
            v26[77] = 1;
            *(DWORD *)(v29 + 308) = 1;
            *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
            v22 = *(DWORD **)(v22[1] + 4);
          }
        }
        else if ( v28[77] )
        {
          if ( v22 == (DWORD *)*v26 )
          {
            v22 = (DWORD *)v22[1];
            FUN_00411760(v26);
          }
          *(DWORD *)(v22[1] + 308) = 1;
          *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
          FUN_00411700(*(DWORD *)(v22[1] + 4));
        }
        else
        {
          v26[77] = 1;
          v28[77] = 1;
          *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
          v22 = *(DWORD **)(v22[1] + 4);
        }
      }
      *(DWORD *)(*(DWORD *)(*((DWORD *)v17 + 1) + 4) + 308) = 1;
      goto LABEL_66;
    }
    v32 = (unsigned int)v20;
    if ( v18 )
    {
      if ( v20 == (DWORD *)*v19 )
      {
        v35 = 1;
        v30 = FUN_00411460(&v36, v21, v20, &v40);
        FUN_00411820(v30, &v35);
LABEL_66:
        if ( strcmp(Source, Destination) )
        {
          strncpy(Source, Destination, 0x100u);
        }
        return;
      }
      FUN_00411870(&v32);
    }
    if ( *(DWORD *)(v32 + 12) >= v40 )
    {
      v34 = 0;
      FUN_00411820(&v32, &v34);
    }
    else
    {
      v33 = 1;
      v31 = FUN_00411460(&v37, v21, v20, &v40);
      FUN_00411820(v31, &v33);
    }
    goto LABEL_66;
  }
}
#endif

// ── FUN_004102e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004102E0)
DWORD __cdecl FUN_004102e0(DWORD *_this, int a2, char a3)
{
  DWORD result; // eax
  DWORD v4; // edx

  result = 0;
  switch ( a3 )
  {
    case 1:
    case -12:
      v4 = -16776961;
      goto LABEL_13;
    case -16:
      v4 = m_dwTextColor;
      result = m_dwBackColor;
      goto LABEL_14;
    case -15:
      v4 = m_dwBackColor;
      result = m_dwTextColor;
      goto LABEL_14;
    case -14:
      v4 = -14116;
      goto LABEL_13;
    case -13:
      v4 = -16711736;
      goto LABEL_13;
    case -11:
      result = 0;
      v4 = -11521516;
      goto LABEL_14;
    case -10:
      result = 0;
      v4 = -16751556;
      goto LABEL_14;
    case -9:
      result = 0;
      v4 = -16777116;
      goto LABEL_14;
    case -8:
      v4 = -3613466;
      if ( !g_bUseChatListBox )
      {
        result = -1778384896;
      }
      goto LABEL_14;
    default:
      v4 = -1;
LABEL_13:
      result = 0;
LABEL_14:
      _this[4 * a2 + 11] = v4;
      _this[4 * a2 + 12] = result;
      return result;
  }
}
#endif

// ── FUN_004105f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004105F0)
int __cdecl FUN_004105f0(DWORD *_this, DWORD *a2, int a3, int a4, int a5)
{
  int result; // eax
  int v6; // edx
  DWORD *v7; // ebp
  int v8; // edi
  BYTE *v9; // eax
  int v10; // edx
  DWORD *v11; // esi
  DWORD v12; // edx
  int v13; // [esp+0h] [ebp-4h]

  result = a5;
  v6 = 0;
  v13 = 0;
  if ( a5 > 0 )
  {
    v7 = a2;
    do
    {
      v8 = 0;
      v9 = (char *)ppvBits + 512 * v6 * DAT_005590bc + 256 * v6 * DAT_005590bc + 2 * a3 + a3;
      if ( a4 > 0 )
      {
        do
        {
          v10 = _this[49] - 1;
          if ( v10 >= 0 )
          {
            v11 = &_this[4 * v10 + 10];
            do
            {
              if ( v8 + a3 > *v11 )
              {
                break;
              }
              --v10;
              v11 -= 4;
            }
            while ( v10 >= 0 );
          }
          if ( *v9 == 0xFF )
          {
            if ( v10 == -1 )
            {
              v12 = m_dwTextColor;
            }
            else
            {
              v12 = _this[4 * v10 + 11];
            }
          }
          else if ( v10 == -1 )
          {
            v12 = m_dwBackColor;
          }
          else
          {
            v12 = _this[4 * v10 + 12];
          }
          *v7 = v12;
          v9 += 3;
          ++v7;
          ++v8;
        }
        while ( v8 < a4 );
        v6 = v13;
      }
      result = a5;
      ++v6;
      v7 = a2 + 256;
      v13 = v6;
      a2 += 256;
    }
    while ( v6 < a5 );
  }
  return result;
}
#endif

// ── FUN_004106d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004106D0)
int __cdecl FUN_004106d0(DWORD *_this, float a2, int a3, int a4, float a5)
{
  const char *v5; // ebx
  int v6; // edx
  int i; // ecx
  float **v8; // edi
  float *v9; // eax
  float *v10; // ebp
  float **v12; // edi
  float *v13; // edi
  __int64 Height; // rax
  int v15; // eax
  int v16; // esi
  int v17; // eax
  int v18; // edx
  float v19; // [esp-4h] [ebp-20h]
  int v20; // [esp+10h] [ebp-Ch] BYREF
  DWORD *v21; // [esp+14h] [ebp-8h]
  char v22[4]; // [esp+18h] [ebp-4h] BYREF

  v5 = (const char *)LODWORD(a2);
  v6 = 0;
  v21 = _this;
  v20 = 0;
  for ( i = 0; i < 4; ++i )
  {
    if ( !*(BYTE *)(i + LODWORD(a2)) )
    {
      break;
    }
    v6 += *(unsigned char *)(i + LODWORD(a2));
  }
  v20 = v6;
  v8 = (float **)FUN_004113e0(&a2, &v20);
  v9 = *(float **)FUN_004113a0(v22, &v20);
  v10 = *v8;
  if ( v9 == *v8 )
  {
    return 0;
  }
  v12 = (float **)(v21 + 5);
  v21[5] = v9;
  do
  {
    if ( *((DWORD *)v9 + 72) == m_dwTextColor
      && *((DWORD *)v9 + 73) == m_dwBackColor
      && *((DWORD *)v9 + 70) == LODWORD(a5)
      && !strcmp((const char *)v9 + 16, v5) )
    {
      break;
    }
    FUN_004112b0(v12);
    v9 = *v12;
  }
  while ( *v12 != v10 );
  v13 = *v12;
  if ( v13 == v10 )
  {
    return 0;
  }
  Height = (__int64)Bitmaps[0].Height;
  if ( *((DWORD *)v13 + 69) > (int)Height )
  {
    *((DWORD *)v13 + 69) = Height;
  }
  v15 = *((DWORD *)v13 + 76);
  v16 = 0;
  if ( (v15 != 0) + 1 > 0 )
  {
    a2 = (float)a4;
    a5 = (float)a3;
    do
    {
      if ( v15 )
      {
        if ( v16 )
        {
          v17 = *((DWORD *)v13 + 68) - 256;
        }
        else
        {
          v17 = 256;
        }
      }
      else
      {
        v17 = *((DWORD *)v13 + 68);
      }
      if ( v16 )
      {
        v18 = *((DWORD *)v13 + 75);
      }
      else
      {
        v18 = *((DWORD *)v13 + 74);
      }
      v19 = v13[69];
      DAT_055c9b90 = v18;
      a4 = v16 << 8;
      FUN_004108b0(
        (__int64)(a5 * g_fScreenRate_x + (double)(v16 << 8)),
        (__int64)(a2 * g_fScreenRate_y),
        *(float *)&v17,
        v19);
      v15 = *((DWORD *)v13 + 76);
      ++v16;
    }
    while ( v16 < (v15 != 0) + 1 );
  }
  ++*((DWORD *)v13 + 71);
  return 1;
}
#endif

// ── FUN_004108b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004108B0)
void __stdcall FUN_004108b0(int a1, int a2, float Width, float Height)
{
  int v4; // eax
  float x; // [esp+0h] [ebp-28h]
  float y; // [esp+4h] [ebp-24h]
  float uWidth; // [esp+18h] [ebp-10h]
  float vHeight; // [esp+1Ch] [ebp-Ch]
  float Widtha; // [esp+34h] [ebp+Ch]
  float Heighta; // [esp+38h] [ebp+10h]

  v4 = a1;
  if ( a1 < 0 )
  {
    v4 = 0;
    a1 = 0;
  }
  if ( v4 + LODWORD(Width) > (int)WindowWidth )
  {
    a1 = WindowWidth - LODWORD(Width);
  }
  Heighta = (float)SLODWORD(Height);
  Widtha = (float)SLODWORD(Width);
  vHeight = (Heighta + 0.0099999998) / Bitmaps[0].Height;
  uWidth = (Widtha + 0.0099999998) / Bitmaps[0].Width;
  y = (float)a2;
  x = (float)a1;
  RenderBitmap(-DAT_055c9b90, x, y, Widtha, Heighta, 0.0, 0.0, uWidth, vHeight, 0, 0);
}
#endif

// ── FUN_00410a90 (IDA-only, gated) ──
#if defined(IDA_PORT_00410A90)
DWORD *__cdecl FUN_00410a90(DWORD *_this)
{
  FUN_0040f680(_this);
  *_this = &DAT_00552810;
  return _this;
}
#endif

// ── FUN_00410ab0 (IDA-only, gated) ──
#if defined(IDA_PORT_00410AB0)
LPVOID __cdecl FUN_00410ab0(LPVOID lpMem, char a2)
{
  FUN_00410ad0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00410ad0 (IDA-only, gated) ──
#if defined(IDA_PORT_00410AD0)
void __cdecl FUN_00410ad0(DWORD *_this)
{
  *_this = &DAT_00552810;
  FUN_0040f690(_this);
}
#endif

// ── FUN_00410d90 (IDA-only, gated) ──
#if defined(IDA_PORT_00410D90)
int __cdecl FUN_00410d90(int _this)
{
  DWORD **v2; // ebx
  DWORD *i; // esi
  DWORD **v4; // eax
  int result; // eax

  v2 = *(DWORD ***)(_this + 4);
  for ( i = *v2; i != v2; --*(DWORD *)(_this + 8) )
  {
    v4 = (DWORD **)i;
    i = (DWORD *)*i;
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
  }
  delete__(*(LPVOID *)(_this + 4));
  result = 0;
  *(DWORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = 0;
  return result;
}
#endif

// ── FUN_00410de0 (IDA-only, gated) ──
#if defined(IDA_PORT_00410DE0)
DWORD *__cdecl FUN_00410de0(DWORD *_this, DWORD *a2, DWORD *lpMem, DWORD *a4)
{
  DWORD *i; // esi
  DWORD **v6; // eax
  DWORD *result; // eax

  for ( i = lpMem; i != a4; --_this[2] )
  {
    v6 = (DWORD **)i;
    i = (DWORD *)*i;
    *v6[1] = *v6;
    (*v6)[1] = v6[1];
    delete__(v6);
  }
  result = a2;
  *a2 = i;
  return result;
}
#endif

// ── FUN_00410e30 (IDA-only, gated) ──
#if defined(IDA_PORT_00410E30)
DWORD *__cdecl FUN_00410e30(DWORD **_this, DWORD *a2, int a3)
{
  DWORD *v3; // edx
  DWORD *result; // eax

  v3 = *_this;
  *_this = (DWORD *)**_this;
  result = a2;
  *a2 = v3;
  return result;
}
#endif

// ── FUN_00410e40 (IDA-only, gated) ──
#if defined(IDA_PORT_00410E40)
DWORD *__cdecl FUN_00410e40(DWORD **_this, DWORD *a2)
{
  DWORD *result; // eax

  result = a2;
  *a2 = *_this[1];
  return result;
}
#endif

// ── FUN_00410e50 (IDA-only, gated) ──
#if defined(IDA_PORT_00410E50)
DWORD *__cdecl FUN_00410e50(DWORD *_this, DWORD *a2, DWORD *a3)
{
  DWORD *v3; // edi
  DWORD *v4; // esi
  DWORD *v5; // ebx
  DWORD *v6; // ebp
  DWORD *v7; // eax
  DWORD *i; // ecx
  DWORD *v9; // edx
  DWORD *v10; // ebp
  int v11; // eax
  DWORD *v12; // eax
  int v13; // eax
  DWORD *v14; // ecx
  int v15; // eax
  DWORD *v16; // eax
  DWORD *v17; // eax
  DWORD *v18; // ecx
  DWORD *v19; // eax
  int v20; // ebp
  DWORD *j; // ecx
  DWORD *v22; // eax
  int v23; // ecx
  DWORD *v24; // eax
  int v25; // eax
  DWORD *v26; // ecx
  int v27; // edx
  DWORD *v28; // edx
  DWORD *v29; // eax
  int v30; // ecx
  int v31; // edx
  int v32; // edx
  DWORD *v33; // edx
  int v34; // ecx
  int v35; // edx
  int v36; // edx
  DWORD *v37; // edx
  DWORD *v38; // eax
  DWORD *v39; // ecx
  int v40; // edx
  DWORD *v41; // edx
  DWORD *v42; // ecx
  int v43; // edx
  DWORD *v44; // edx
  int v45; // edx
  int v46; // edx
  DWORD *v47; // edx
  DWORD *v48; // ecx
  DWORD *result; // eax
  DWORD *lpMem; // [esp+14h] [ebp-Ch]
  DWORD *v52; // [esp+18h] [ebp-8h]
  char v53[4]; // [esp+1Ch] [ebp-4h] BYREF

  v3 = a3;
  FUN_004112b0(&a3);
  v4 = (DWORD *)*v3;
  v5 = v3 + 2;
  lpMem = v3;
  v6 = v3 + 2;
  if ( *v3 == DAT_055c9b98 )
  {
    v4 = (DWORD *)*v5;
  }
  else
  {
    v7 = (DWORD *)*v5;
    if ( *v5 != DAT_055c9b98 )
    {
      for ( i = (DWORD *)*v7; i != (DWORD *)DAT_055c9b98; i = (DWORD *)*i )
      {
        v7 = i;
      }
      v4 = (DWORD *)v7[2];
      v6 = v7 + 2;
      lpMem = v7;
    }
  }
  std::_Lockit::_Lockit((std::_Lockit *)v53);
  v9 = lpMem;
  if ( lpMem == v3 )
  {
    v14 = _this;
    v4[1] = lpMem[1];
    v15 = _this[1];
    if ( *(DWORD **)(v15 + 4) == v3 )
    {
      *(DWORD *)(v15 + 4) = v4;
    }
    else
    {
      v16 = (DWORD *)v3[1];
      if ( (DWORD *)*v16 == v3 )
      {
        *v16 = v4;
      }
      else
      {
        v16[2] = v4;
      }
    }
    v17 = (DWORD *)_this[1];
    v52 = v17;
    if ( (DWORD *)*v17 == v3 )
    {
      if ( *v5 == DAT_055c9b98 )
      {
        *v17 = v3[1];
      }
      else
      {
        v18 = v4;
        if ( *v4 != DAT_055c9b98 )
        {
          v19 = (DWORD *)*v4;
          do
          {
            v18 = v19;
            v19 = (DWORD *)*v19;
          }
          while ( v19 != (DWORD *)DAT_055c9b98 );
          v17 = v52;
        }
        *v17 = v18;
        v14 = _this;
      }
    }
    v20 = v14[1];
    if ( *(DWORD **)(v20 + 8) == v3 )
    {
      if ( *v3 == DAT_055c9b98 )
      {
        j = (DWORD *)v3[1];
      }
      else
      {
        v22 = (DWORD *)v4[2];
        for ( j = v4; v22 != (DWORD *)DAT_055c9b98; v22 = (DWORD *)v22[2] )
        {
          j = v22;
        }
      }
      *(DWORD *)(v20 + 8) = j;
    }
    v10 = _this;
  }
  else
  {
    *(DWORD *)(*v3 + 4) = lpMem;
    *lpMem = *v3;
    if ( lpMem == (DWORD *)*v5 )
    {
      v4[1] = lpMem;
    }
    else
    {
      v4[1] = lpMem[1];
      *(DWORD *)lpMem[1] = v4;
      *v6 = *v5;
      *(DWORD *)(*v5 + 4) = lpMem;
    }
    v10 = _this;
    v11 = _this[1];
    if ( *(DWORD **)(v11 + 4) == v3 )
    {
      *(DWORD *)(v11 + 4) = lpMem;
    }
    else
    {
      v12 = (DWORD *)v3[1];
      if ( (DWORD *)*v12 == v3 )
      {
        *v12 = lpMem;
      }
      else
      {
        v12[2] = lpMem;
      }
    }
    lpMem = v3;
    v9[1] = v3[1];
    v13 = v9[77];
    v9[77] = v3[77];
    v3[77] = v13;
    v9 = v3;
  }
  if ( v9[77] == 1 )
  {
    for ( ; v4 != *(DWORD **)(v10[1] + 4); v4 = (DWORD *)v4[1] )
    {
      if ( v4[77] != 1 )
      {
        break;
      }
      v23 = v4[1];
      v24 = *(DWORD **)v23;
      if ( v4 == *(DWORD **)v23 )
      {
        v24 = *(DWORD **)(v23 + 8);
        if ( !v24[77] )
        {
          v24[77] = 1;
          *(DWORD *)(v4[1] + 308) = 0;
          v25 = v4[1];
          v26 = *(DWORD **)(v25 + 8);
          *(DWORD *)(v25 + 8) = *v26;
          if ( *v26 != DAT_055c9b98 )
          {
            *(DWORD *)(*v26 + 4) = v25;
          }
          v26[1] = *(DWORD *)(v25 + 4);
          v27 = v10[1];
          if ( v25 == *(DWORD *)(v27 + 4) )
          {
            *(DWORD *)(v27 + 4) = v26;
          }
          else
          {
            v28 = *(DWORD **)(v25 + 4);
            if ( v25 == *v28 )
            {
              *v28 = v26;
            }
            else
            {
              v28[2] = v26;
            }
          }
          *v26 = v25;
          *(DWORD *)(v25 + 4) = v26;
          v24 = *(DWORD **)(v4[1] + 8);
        }
        if ( *(DWORD *)(*v24 + 308) != 1 || *(DWORD *)(v24[2] + 308) != 1 )
        {
          if ( *(DWORD *)(v24[2] + 308) == 1 )
          {
            *(DWORD *)(*v24 + 308) = 1;
            v34 = *v24;
            v24[77] = 0;
            *v24 = *(DWORD *)(v34 + 8);
            v35 = *(DWORD *)(v34 + 8);
            if ( v35 != DAT_055c9b98 )
            {
              *(DWORD *)(v35 + 4) = v24;
            }
            *(DWORD *)(v34 + 4) = v24[1];
            v36 = v10[1];
            if ( v24 == *(DWORD **)(v36 + 4) )
            {
              *(DWORD *)(v36 + 4) = v34;
            }
            else
            {
              v37 = (DWORD *)v24[1];
              if ( v24 == (DWORD *)v37[2] )
              {
                v37[2] = v34;
              }
              else
              {
                *v37 = v34;
              }
            }
            *(DWORD *)(v34 + 8) = v24;
            v24[1] = v34;
            v24 = *(DWORD **)(v4[1] + 8);
          }
          v24[77] = *(DWORD *)(v4[1] + 308);
          *(DWORD *)(v4[1] + 308) = 1;
          *(DWORD *)(v24[2] + 308) = 1;
          v38 = (DWORD *)v4[1];
          v39 = (DWORD *)v38[2];
          v38[2] = *v39;
          if ( *v39 != DAT_055c9b98 )
          {
            *(DWORD *)(*v39 + 4) = v38;
          }
          v39[1] = v38[1];
          v40 = v10[1];
          if ( v38 == *(DWORD **)(v40 + 4) )
          {
            *(DWORD *)(v40 + 4) = v39;
            *v39 = v38;
          }
          else
          {
            v41 = (DWORD *)v38[1];
            if ( v38 == (DWORD *)*v41 )
            {
              *v41 = v39;
            }
            else
            {
              v41[2] = v39;
            }
            *v39 = v38;
          }
LABEL_100:
          v38[1] = v39;
          break;
        }
      }
      else
      {
        if ( !v24[77] )
        {
          v24[77] = 1;
          *(DWORD *)(v4[1] + 308) = 0;
          v29 = (DWORD *)v4[1];
          v30 = *v29;
          *v29 = *(DWORD *)(*v29 + 8);
          v31 = *(DWORD *)(v30 + 8);
          if ( v31 != DAT_055c9b98 )
          {
            *(DWORD *)(v31 + 4) = v29;
          }
          *(DWORD *)(v30 + 4) = v29[1];
          v32 = v10[1];
          if ( v29 == *(DWORD **)(v32 + 4) )
          {
            *(DWORD *)(v32 + 4) = v30;
          }
          else
          {
            v33 = (DWORD *)v29[1];
            if ( v29 == (DWORD *)v33[2] )
            {
              v33[2] = v30;
            }
            else
            {
              *v33 = v30;
            }
          }
          *(DWORD *)(v30 + 8) = v29;
          v29[1] = v30;
          v24 = *(DWORD **)v4[1];
        }
        if ( *(DWORD *)(v24[2] + 308) != 1 || *(DWORD *)(*v24 + 308) != 1 )
        {
          if ( *(DWORD *)(*v24 + 308) == 1 )
          {
            *(DWORD *)(v24[2] + 308) = 1;
            v42 = (DWORD *)v24[2];
            v24[77] = 0;
            v24[2] = *v42;
            if ( *v42 != DAT_055c9b98 )
            {
              *(DWORD *)(*v42 + 4) = v24;
            }
            v42[1] = v24[1];
            v43 = v10[1];
            if ( v24 == *(DWORD **)(v43 + 4) )
            {
              *(DWORD *)(v43 + 4) = v42;
            }
            else
            {
              v44 = (DWORD *)v24[1];
              if ( v24 == (DWORD *)*v44 )
              {
                *v44 = v42;
              }
              else
              {
                v44[2] = v42;
              }
            }
            *v42 = v24;
            v24[1] = v42;
            v24 = *(DWORD **)v4[1];
          }
          v24[77] = *(DWORD *)(v4[1] + 308);
          *(DWORD *)(v4[1] + 308) = 1;
          *(DWORD *)(*v24 + 308) = 1;
          v38 = (DWORD *)v4[1];
          v39 = (DWORD *)*v38;
          *v38 = *(DWORD *)(*v38 + 8);
          v45 = v39[2];
          if ( v45 != DAT_055c9b98 )
          {
            *(DWORD *)(v45 + 4) = v38;
          }
          v39[1] = v38[1];
          v46 = v10[1];
          if ( v38 == *(DWORD **)(v46 + 4) )
          {
            *(DWORD *)(v46 + 4) = v39;
          }
          else
          {
            v47 = (DWORD *)v38[1];
            if ( v38 == (DWORD *)v47[2] )
            {
              v47[2] = v39;
            }
            else
            {
              *v47 = v39;
            }
          }
          v39[2] = v38;
          goto LABEL_100;
        }
      }
      v24[77] = 0;
    }
    v4[77] = 1;
  }
  std::_Lockit::~_Lockit((std::_Lockit *)v53);
  delete__(lpMem);
  v48 = a3;
  --v10[3];
  result = a2;
  *a2 = v48;
  return result;
}
#endif

// ── FUN_004112b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004112B0)
DWORD *__cdecl FUN_004112b0(void *_this)
{
  DWORD *v1; // edx
  DWORD *result; // eax

  v1 = *(DWORD **)(*(DWORD *)_this + 8);
  if ( v1 == (DWORD *)DAT_055c9b98 )
  {
    for ( result = *(DWORD **)(*(DWORD *)_this + 4); *(DWORD *)_this == result[2]; result = (DWORD *)result[1] )
    {
      *(DWORD *)_this = result;
    }
    if ( *(DWORD **)(*(DWORD *)_this + 8) != result )
    {
      *(DWORD *)_this = result;
    }
  }
  else
  {
    for ( result = (DWORD *)*v1; result != (DWORD *)DAT_055c9b98; result = (DWORD *)*result )
    {
      v1 = result;
    }
    *(DWORD *)_this = v1;
  }
  return result;
}
#endif

// ── FUN_00411300 (IDA-only, gated) ──
#if defined(IDA_PORT_00411300)
DWORD *__cdecl FUN_00411300(DWORD *_this, DWORD *a2, DWORD *a3, DWORD *a4)
{
  DWORD *v5; // edi
  DWORD *v6; // eax
  DWORD *v7; // ecx
  DWORD *v8; // ecx

  v5 = (DWORD *)a3[1];
  v6 = (DWORD *)operator_new(0xCu);
  v7 = a3;
  if ( !a3 )
  {
    v7 = v6;
  }
  *v6 = v7;
  v8 = v5;
  if ( !v5 )
  {
    v8 = v6;
  }
  v6[1] = v8;
  a3[1] = v6;
  *(DWORD *)v6[1] = v6;
  if ( v6 != (DWORD *)-8 )
  {
    v6[2] = *a4;
  }
  ++_this[2];
  *a2 = v6;
  return a2;
}
#endif

// ── FUN_00411360 (IDA-only, gated) ──
#if defined(IDA_PORT_00411360)
DWORD *__cdecl FUN_00411360(DWORD *_this, DWORD *a2, DWORD **lpMem)
{
  DWORD *v4; // edi
  DWORD *result; // eax

  v4 = *lpMem;
  *lpMem[1] = *lpMem;
  (*lpMem)[1] = lpMem[1];
  delete__(lpMem);
  --_this[2];
  result = a2;
  *a2 = v4;
  return result;
}
#endif

// ── FUN_004113a0 (IDA-only, gated) ──
#if defined(IDA_PORT_004113A0)
DWORD *__cdecl FUN_004113a0(DWORD *_this, DWORD *a2, DWORD *a3)
{
  DWORD *v3; // ecx
  DWORD *v4; // eax
  DWORD *result; // eax

  v3 = (DWORD *)_this[1];
  v4 = (DWORD *)v3[1];
  if ( v4 == (DWORD *)DAT_055c9b98 )
  {
    result = a2;
    *a2 = v3;
  }
  else
  {
    do
    {
      if ( v4[3] >= *a3 )
      {
        v3 = v4;
        v4 = (DWORD *)*v4;
      }
      else
      {
        v4 = (DWORD *)v4[2];
      }
    }
    while ( v4 != (DWORD *)DAT_055c9b98 );
    result = a2;
    *a2 = v3;
  }
  return result;
}
#endif

// ── FUN_004113e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004113E0)
DWORD *__cdecl FUN_004113e0(DWORD *_this, DWORD *a2, DWORD *a3)
{
  DWORD *v3; // ecx
  DWORD *v4; // eax
  DWORD *result; // eax

  v3 = (DWORD *)_this[1];
  v4 = (DWORD *)v3[1];
  if ( v4 == (DWORD *)DAT_055c9b98 )
  {
    result = a2;
    *a2 = v3;
  }
  else
  {
    do
    {
      if ( *a3 >= v4[3] )
      {
        v4 = (DWORD *)v4[2];
      }
      else
      {
        v3 = v4;
        v4 = (DWORD *)*v4;
      }
    }
    while ( v4 != (DWORD *)DAT_055c9b98 );
    result = a2;
    *a2 = v3;
  }
  return result;
}
#endif

// ── FUN_00411420 (IDA-only, gated) ──
#if defined(IDA_PORT_00411420)
int __stdcall FUN_00411420(LPVOID *lpMem)
{
  int result; // eax
  LPVOID *v2; // edi
  LPVOID *i; // esi

  result = DAT_055c9b98;
  v2 = lpMem;
  for ( i = lpMem; i != (LPVOID *)DAT_055c9b98; v2 = i )
  {
    FUN_00411420(i[2]);
    i = (LPVOID *)*i;
    delete__(v2);
    result = DAT_055c9b98;
  }
  return result;
}
#endif

// ── FUN_00411460 (IDA-only, gated) ──
#if defined(IDA_PORT_00411460)
DWORD *__cdecl FUN_00411460(DWORD *_this, DWORD *a2, int a3, DWORD *a4, DWORD *a5)
{
  DWORD *v6; // ebp
  DWORD *v7; // eax
  int v8; // eax
  DWORD *v9; // eax
  DWORD *v10; // eax
  DWORD *v11; // ecx
  DWORD *v12; // esi
  DWORD *v13; // edx
  int v14; // edx
  DWORD *v15; // ecx
  int v16; // edx
  DWORD *v17; // edx
  DWORD *v18; // ecx
  DWORD *v19; // edx
  int v20; // esi
  int v21; // esi
  DWORD *v22; // esi
  DWORD *v23; // ecx
  int v24; // edx
  int v25; // edx
  DWORD *v26; // edx
  int v27; // esi
  DWORD *v28; // esi
  DWORD *result; // eax

  v6 = (DWORD *)operator_new(0x138u);
  v6[1] = a4;
  v6[77] = 0;
  *v6 = DAT_055c9b98;
  v6[2] = DAT_055c9b98;
  FUN_004124d0(v6 + 3, a5);
  v7 = (DWORD *)_this[1];
  ++_this[3];
  if ( a4 == v7 || a3 != DAT_055c9b98 || *a5 < a4[3] )
  {
    *a4 = v6;
    v9 = (DWORD *)_this[1];
    if ( a4 == v9 )
    {
      v9[1] = v6;
      *(DWORD *)(_this[1] + 8) = v6;
    }
    else if ( a4 == (DWORD *)*v9 )
    {
      *v9 = v6;
    }
  }
  else
  {
    a4[2] = v6;
    v8 = _this[1];
    if ( a4 == *(DWORD **)(v8 + 8) )
    {
      *(DWORD *)(v8 + 8) = v6;
    }
  }
  v10 = v6;
  while ( v10 != *(DWORD **)(_this[1] + 4) )
  {
    v11 = (DWORD *)v10[1];
    if ( v11[77] )
    {
      break;
    }
    v12 = (DWORD *)v11[1];
    v13 = (DWORD *)*v12;
    if ( v11 == (DWORD *)*v12 )
    {
      v14 = v12[2];
      if ( *(DWORD *)(v14 + 308) )
      {
        if ( v10 == (DWORD *)v11[2] )
        {
          v10 = (DWORD *)v10[1];
          v15 = (DWORD *)v11[2];
          v10[2] = *v15;
          if ( *v15 != DAT_055c9b98 )
          {
            *(DWORD *)(*v15 + 4) = v10;
          }
          v15[1] = v10[1];
          v16 = _this[1];
          if ( v10 == *(DWORD **)(v16 + 4) )
          {
            *(DWORD *)(v16 + 4) = v15;
          }
          else
          {
            v17 = (DWORD *)v10[1];
            if ( v10 == (DWORD *)*v17 )
            {
              *v17 = v15;
            }
            else
            {
              v17[2] = v15;
            }
          }
          *v15 = v10;
          v10[1] = v15;
        }
        *(DWORD *)(v10[1] + 308) = 1;
        *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
        v18 = *(DWORD **)(v10[1] + 4);
        v19 = (DWORD *)*v18;
        *v18 = *(DWORD *)(*v18 + 8);
        v20 = v19[2];
        if ( v20 != DAT_055c9b98 )
        {
          *(DWORD *)(v20 + 4) = v18;
        }
        v19[1] = v18[1];
        v21 = _this[1];
        if ( v18 == *(DWORD **)(v21 + 4) )
        {
          *(DWORD *)(v21 + 4) = v19;
          v19[2] = v18;
        }
        else
        {
          v22 = (DWORD *)v18[1];
          if ( v18 == (DWORD *)v22[2] )
          {
            v22[2] = v19;
          }
          else
          {
            *v22 = v19;
          }
          v19[2] = v18;
        }
LABEL_51:
        v18[1] = v19;
        continue;
      }
      v11[77] = 1;
      *(DWORD *)(v14 + 308) = 1;
      *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
      v10 = *(DWORD **)(v10[1] + 4);
    }
    else
    {
      if ( v13[77] )
      {
        if ( v10 == (DWORD *)*v11 )
        {
          v10 = (DWORD *)v10[1];
          v23 = (DWORD *)*v11;
          *v10 = v23[2];
          v24 = v23[2];
          if ( v24 != DAT_055c9b98 )
          {
            *(DWORD *)(v24 + 4) = v10;
          }
          v23[1] = v10[1];
          v25 = _this[1];
          if ( v10 == *(DWORD **)(v25 + 4) )
          {
            *(DWORD *)(v25 + 4) = v23;
          }
          else
          {
            v26 = (DWORD *)v10[1];
            if ( v10 == (DWORD *)v26[2] )
            {
              v26[2] = v23;
            }
            else
            {
              *v26 = v23;
            }
          }
          v23[2] = v10;
          v10[1] = v23;
        }
        *(DWORD *)(v10[1] + 308) = 1;
        *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
        v18 = *(DWORD **)(v10[1] + 4);
        v19 = (DWORD *)v18[2];
        v18[2] = *v19;
        if ( *v19 != DAT_055c9b98 )
        {
          *(DWORD *)(*v19 + 4) = v18;
        }
        v19[1] = v18[1];
        v27 = _this[1];
        if ( v18 == *(DWORD **)(v27 + 4) )
        {
          *(DWORD *)(v27 + 4) = v19;
        }
        else
        {
          v28 = (DWORD *)v18[1];
          if ( v18 == (DWORD *)*v28 )
          {
            *v28 = v19;
          }
          else
          {
            v28[2] = v19;
          }
        }
        *v19 = v18;
        goto LABEL_51;
      }
      v11[77] = 1;
      v13[77] = 1;
      *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
      v10 = *(DWORD **)(v10[1] + 4);
    }
  }
  *(DWORD *)(*(DWORD *)(_this[1] + 4) + 308) = 1;
  result = a2;
  *a2 = v6;
  return result;
}
#endif

// ── FUN_00411700 (IDA-only, gated) ──
#if defined(IDA_PORT_00411700)
DWORD *__cdecl FUN_00411700(DWORD *_this, int a2)
{
  DWORD *result; // eax
  int v3; // ecx
  DWORD *v4; // ecx

  result = *(DWORD **)(a2 + 8);
  *(DWORD *)(a2 + 8) = *result;
  if ( *result != DAT_055c9b98 )
  {
    *(DWORD *)(*result + 4) = a2;
  }
  result[1] = *(DWORD *)(a2 + 4);
  v3 = _this[1];
  if ( a2 == *(DWORD *)(v3 + 4) )
  {
    *(DWORD *)(v3 + 4) = result;
    *result = a2;
    *(DWORD *)(a2 + 4) = result;
  }
  else
  {
    v4 = *(DWORD **)(a2 + 4);
    if ( a2 == *v4 )
    {
      *v4 = result;
    }
    else
    {
      v4[2] = result;
    }
    *result = a2;
    *(DWORD *)(a2 + 4) = result;
  }
  return result;
}
#endif

// ── FUN_00411760 (IDA-only, gated) ──
#if defined(IDA_PORT_00411760)
int __cdecl FUN_00411760(DWORD *_this, DWORD *a2)
{
  int result; // eax
  int v3; // esi
  int v4; // ecx
  DWORD *v5; // ecx

  result = *a2;
  *a2 = *(DWORD *)(*a2 + 8);
  v3 = *(DWORD *)(result + 8);
  if ( v3 != DAT_055c9b98 )
  {
    *(DWORD *)(v3 + 4) = a2;
  }
  *(DWORD *)(result + 4) = a2[1];
  v4 = _this[1];
  if ( a2 == *(DWORD **)(v4 + 4) )
  {
    *(DWORD *)(v4 + 4) = result;
    *(DWORD *)(result + 8) = a2;
    a2[1] = result;
  }
  else
  {
    v5 = (DWORD *)a2[1];
    if ( a2 == (DWORD *)v5[2] )
    {
      v5[2] = result;
    }
    else
    {
      *v5 = result;
    }
    *(DWORD *)(result + 8) = a2;
    a2[1] = result;
  }
  return result;
}
#endif

// ── FUN_00411820 (IDA-only, gated) ──
#if defined(IDA_PORT_00411820)
BYTE *__cdecl FUN_00411820(BYTE *_this, DWORD *a2, BYTE *a3)
{
  BYTE *result; // eax

  result = _this;
  *(DWORD *)_this = *a2;
  _this[4] = *a3;
  return result;
}
#endif

// ── FUN_00411840 (IDA-only, gated) ──
#if defined(IDA_PORT_00411840)
int __stdcall FUN_00411840(int a1, int a2)
{
  int result; // eax

  result = operator_new(0x138u);
  *(DWORD *)(result + 4) = a1;
  *(DWORD *)(result + 308) = a2;
  return result;
}
#endif

// ── FUN_00411870 (IDA-only, gated) ──
#if defined(IDA_PORT_00411870)
DWORD *__cdecl FUN_00411870(void *_this)
{
  DWORD *v1; // eax
  DWORD *result; // eax
  DWORD *v3; // edx

  v1 = *(DWORD **)_this;
  if ( *(DWORD *)(*(DWORD *)_this + 308) || *(DWORD **)(v1[1] + 4) != v1 )
  {
    v3 = (DWORD *)*v1;
    if ( *v1 == DAT_055c9b98 )
    {
      for ( result = (DWORD *)v1[1]; *(DWORD *)_this == *result; result = (DWORD *)result[1] )
      {
        *(DWORD *)_this = result;
      }
      *(DWORD *)_this = result;
    }
    else
    {
      for ( result = (DWORD *)v3[2]; result != (DWORD *)DAT_055c9b98; result = (DWORD *)result[2] )
      {
        v3 = result;
      }
      *(DWORD *)_this = v3;
    }
  }
  else
  {
    result = (DWORD *)v1[2];
    *(DWORD *)_this = result;
  }
  return result;
}
#endif

// ── FUN_004118d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004118D0)
void __cdecl FUN_004118d0(DWORD *_this)
{
  DWORD **v2; // ebx
  DWORD *i; // esi
  DWORD **v4; // eax

  v2 = (DWORD **)_this[23];
  for ( i = *v2; i != v2; --_this[24] )
  {
    v4 = (DWORD **)i;
    i = (DWORD *)*i;
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
  }
  _this[34] = 0;
}
#endif

// ── FUN_00411920 (IDA-only, gated) ──
#if defined(IDA_PORT_00411920)
int __cdecl FUN_00411920(DWORD *_this)
{
  int i; // edi
  int v3; // eax

  (*(void (__cdecl **)(DWORD *))(*_this + 88))(_this);
  (*(void (__cdecl **)(DWORD *))(*_this + 80))(_this);
  glColor3f(1.0, 1.0, 1.0);
  SelectObject(m_hFontDC, g_hFont);
  for ( i = 0; i < _this[35]; _this[25] = *(DWORD *)_this[25] )
  {
    if ( _this[25] == _this[23] )
    {
      break;
    }
    v3 = (*(int (__cdecl **)(DWORD *, int))(*_this + 92))(_this, i);
    if ( v3 >= 0 )
    {
      if ( !v3 )
      {
        --i;
      }
    }
    else
    {
      i -= v3;
    }
    ++i;
  }
  return (*(int (__cdecl **)(DWORD *))(*_this + 96))(_this);
}
#endif

// ── FUN_00412180 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_00412180(DWORD *_this)
{
  int v2; // ecx
  int result; // eax
  int v4; // ecx
  int *v5; // eax
  int v6; // edx
  int v7; // ecx
  int v8; // eax
  int v9; // ecx
  int v10; // edx
  int v11; // edx
  int v12; // ecx

  v2 = _this[4];
  switch ( v2 )
  {
    case 7:
      (*(void (__cdecl **)(DWORD *))(*_this + 108))(_this);
      result = 0;
      break;
    case 12:
      (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, -100);
      if ( _this[26] != 1 || !_this[24] )
      {
        goto LABEL_23;
      }
      v4 = _this[23];
      _this[28] = v4;
      result = 0;
      _this[28] = *(DWORD *)(v4 + 4);
      break;
    case 13:
    case 14:
      if ( _this[26] != 1 || !_this[24] )
      {
        goto LABEL_23;
      }
      if ( v2 == 13 )
      {
        v5 = (int *)_this[28];
        if ( v5 == (int *)_this[23] )
        {
          goto LABEL_23;
        }
        v6 = *v5;
        _this[28] = *v5;
        if ( v6 == _this[23] )
        {
          _this[28] = *(DWORD *)(v6 + 4);
        }
      }
      else if ( v2 == 14 )
      {
        v7 = _this[28];
        if ( v7 == *(DWORD *)_this[23] )
        {
          goto LABEL_23;
        }
        _this[28] = *(DWORD *)(v7 + 4);
      }
      if ( _this[24] <= _this[35] )
      {
        goto LABEL_23;
      }
      v8 = 0;
      v9 = *(DWORD *)_this[23];
      _this[25] = v9;
      if ( v9 != _this[23] )
      {
        do
        {
          if ( _this[28] == v9 )
          {
            break;
          }
          ++v8;
          v10 = *(DWORD *)_this[25];
          _this[25] = v10;
          v9 = v10;
        }
        while ( v10 != _this[23] );
      }
      v11 = _this[35];
      v12 = _this[34];
      if ( v8 < v12 + v11 )
      {
        if ( v8 < v12 )
        {
          (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, v12 - v8);
        }
        goto LABEL_23;
      }
      (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, v12 - (v8 - v11 + 1));
      result = 0;
      break;
    default:
LABEL_23:
      result = 0;
      break;
  }
  return result;
}

// ── FUN_004122c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004122C0)
int __cdecl FUN_004122c0(DWORD *_this)
{
  int v2; // ebx
  int result; // eax
  int v4; // edi
  DWORD **v5; // [esp-4h] [ebp-10h]

  v2 = _this[24];
  result = _this[33];
  if ( v2 >= result )
  {
    v4 = 0;
    if ( v2 - result > 0 )
    {
      do
      {
        v5 = *(DWORD ***)(_this[23] + 4);
        *v5[1] = *v5;
        (*v5)[1] = v5[1];
        delete__(v5);
        --_this[24];
        ++v4;
        result = v2 - _this[33];
      }
      while ( v4 < result );
    }
  }
  return result;
}
#endif

// ── FUN_004124f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004124F0)
DWORD *__cdecl FUN_004124f0(DWORD *lpMem, char a2)
{
  FUN_00412510(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00412510 (IDA-only, gated) ──
#if defined(IDA_PORT_00412510)
int __fastcall FUN_00412510(DWORD *a1)
{
  DWORD *v1; // edi
  DWORD **v2; // ebx
  DWORD *v3; // esi
  DWORD **v4; // eax
  DWORD *v5; // ebx
  DWORD **v6; // ebp
  DWORD *v7; // esi
  DWORD **v8; // eax
  DWORD **v9; // ebx
  DWORD *v10; // esi
  DWORD **v11; // eax
  DWORD *v13; // [esp+10h] [ebp-8h]
  char v14[4]; // [esp+14h] [ebp-4h] BYREF

  v1 = a1 + 22;
  *a1 = &DAT_00552668;
  v13 = a1;
  v2 = (DWORD **)a1[23];
  v3 = *v2;
  if ( *v2 != v2 )
  {
    do
    {
      v4 = (DWORD **)v3;
      v3 = (DWORD *)*v3;
      FUN_00411360(v1, v14, v4);
    }
    while ( v3 != v2 );
    a1 = v13;
  }
  v5 = a1 + 30;
  a1[34] = 0;
  v6 = (DWORD **)a1[31];
  v7 = *v6;
  while ( v7 != v6 )
  {
    v8 = (DWORD **)v7;
    v7 = (DWORD *)*v7;
    FUN_00411360(v5, v14, v8);
  }
  delete__((LPVOID)v5[1]);
  v5[1] = 0;
  v5[2] = 0;
  v9 = (DWORD **)v1[1];
  v10 = *v9;
  while ( v10 != v9 )
  {
    v11 = (DWORD **)v10;
    v10 = (DWORD *)*v10;
    FUN_00411360(v1, v14, v11);
  }
  delete__((LPVOID)v1[1]);
  v1[1] = 0;
  v1[2] = 0;
  *v13 = DAT_005525c8;
  FUN_00410de0(v13 + 1, v14, *(DWORD **)v13[2], (DWORD *)v13[2]);
  return FUN_00410d90((int)(v13 + 1));
}
#endif

// ── FUN_004125f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004125F0)
LPVOID __cdecl FUN_004125f0(LPVOID lpMem, char a2)
{
  FUN_00412610();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_00412610 (IDA-only, gated) ──
#if defined(IDA_PORT_00412610)
int __fastcall FUN_00412610(DWORD *a1)
{
  DWORD *v1; // edi
  DWORD **v2; // ebx
  DWORD *v3; // esi
  DWORD **v4; // eax
  DWORD *v5; // ebx
  DWORD **v6; // ebp
  DWORD *v7; // esi
  DWORD **v8; // eax
  DWORD **v9; // ebx
  DWORD *v10; // esi
  DWORD **v11; // eax
  DWORD *v13; // [esp+10h] [ebp-8h]
  int v14; // [esp+14h] [ebp-4h] BYREF

  v1 = a1 + 22;
  *a1 = &DAT_00552760;
  v13 = a1;
  v2 = (DWORD **)a1[23];
  v3 = *v2;
  if ( *v2 != v2 )
  {
    do
    {
      v4 = (DWORD **)v3;
      v3 = (DWORD *)*v3;
      FUN_00411360(v1, &v14, v4);
    }
    while ( v3 != v2 );
    a1 = v13;
  }
  v5 = a1 + 30;
  a1[34] = 0;
  v6 = (DWORD **)a1[31];
  v7 = *v6;
  while ( v7 != v6 )
  {
    v8 = (DWORD **)v7;
    v7 = (DWORD *)*v7;
    FUN_00411360(v5, &v14, v8);
  }
  delete__((LPVOID)v5[1]);
  v5[1] = 0;
  v5[2] = 0;
  v9 = (DWORD **)v1[1];
  v10 = *v9;
  while ( v10 != v9 )
  {
    v11 = (DWORD **)v10;
    v10 = (DWORD *)*v10;
    FUN_00411360(v1, &v14, v11);
  }
  delete__((LPVOID)v1[1]);
  v1[1] = 0;
  v1[2] = 0;
  *v13 = DAT_005525c8;
  FUN_00410de0(v13 + 1, &v14, *(DWORD **)v13[2], (DWORD *)v13[2]);
  return FUN_00410d90((int)(v13 + 1));
}
#endif

// ── FUN_00412700 (IDA-only, gated) ──
#if defined(IDA_PORT_00412700)
int FUN_00412700()
{
  return FUN_0053d430(aMu);
}
#endif

// ── FUN_00412710 (IDA-only, gated) ──
#if defined(IDA_PORT_00412710)
int FUN_00412700()
{
  return FUN_0053d430(aMu);
}
#endif

// ── FUN_00412780 (IDA-only, gated) ──
#if defined(IDA_PORT_00412780)
// Microsoft VisualC 2-14/net runtime
int *FUN_00412780()
{
  int v0; // eax
  int v1; // eax
  int v2; // edx
  int v3; // eax
  void *v4; // edi
  unsigned int v5; // edx
  int v6; // eax

  DAT_055c9bc8 = (int)&DAT_005524c8;
  FUN_00406d20(&DAT_055c9bc8);
  if ( *(&DAT_055c9bc8 + 6) )
  {
    delete__((LPVOID)*(&DAT_055c9bc8 + 6));
  }
  v0 = rand();
  *(&DAT_055c9bc8 + 6) = operator_new(v0 % 3271 + 345);
  *(&DAT_055c9bc8 + 3) = 1024;
  v1 = operator_new(0x1000u);
  v2 = *(&DAT_055c9bc8 + 3);
  *(&DAT_055c9bc8 + 1) = v1;
  v3 = operator_new(4 * v2);
  v4 = (void *)*(&DAT_055c9bc8 + 1);
  v5 = 4 * *(&DAT_055c9bc8 + 3);
  *(&DAT_055c9bc8 + 2) = v3;
  memset(v4, 0, v5);
  memset((void *)*(&DAT_055c9bc8 + 2), 0, 4 * *(&DAT_055c9bc8 + 3));
  v6 = *(&DAT_055c9bc8 + 1);
  *(&DAT_055c9bc8 + 9) = *(&DAT_055c9bc8 + 2);
  *(&DAT_055c9bc8 + 8) = v6;
  return &DAT_055c9bc8;
}
#endif

// ── FUN_00412790 (IDA-only, gated) ──
#if defined(IDA_PORT_00412790)
// Microsoft VisualC 2-14/net runtime
int *FUN_00412790()
{
  int v0; // eax
  int v1; // eax
  int v2; // edx
  int v3; // eax
  void *v4; // edi
  unsigned int v5; // edx
  int v6; // eax

  DAT_055c9bc8 = (int)&DAT_005524c8;
  FUN_00406d20(&DAT_055c9bc8);
  if ( *(&DAT_055c9bc8 + 6) )
  {
    delete__((LPVOID)*(&DAT_055c9bc8 + 6));
  }
  v0 = rand();
  *(&DAT_055c9bc8 + 6) = operator_new(v0 % 3271 + 345);
  *(&DAT_055c9bc8 + 3) = 1024;
  v1 = operator_new(0x1000u);
  v2 = *(&DAT_055c9bc8 + 3);
  *(&DAT_055c9bc8 + 1) = v1;
  v3 = operator_new(4 * v2);
  v4 = (void *)*(&DAT_055c9bc8 + 1);
  v5 = 4 * *(&DAT_055c9bc8 + 3);
  *(&DAT_055c9bc8 + 2) = v3;
  memset(v4, 0, v5);
  memset((void *)*(&DAT_055c9bc8 + 2), 0, 4 * *(&DAT_055c9bc8 + 3));
  v6 = *(&DAT_055c9bc8 + 1);
  *(&DAT_055c9bc8 + 9) = *(&DAT_055c9bc8 + 2);
  *(&DAT_055c9bc8 + 8) = v6;
  return &DAT_055c9bc8;
}
#endif

// ── FUN_004127c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004127C0)
// Microsoft VisualC 2-14/net runtime
DWORD *FUN_004127c0()
{
  g_ErrorReport = (DWORD)DAT_005524c4;
  FUN_00405290((int)&g_ErrorReport);
  FUN_004052b0((int)&g_ErrorReport, aMuerrorLog);
  return &g_ErrorReport;
}
#endif

// ── FUN_004127d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004127D0)
// Microsoft VisualC 2-14/net runtime
DWORD *FUN_004127d0()
{
  g_ErrorReport = (DWORD)DAT_005524c4;
  FUN_00405290((int)&g_ErrorReport);
  FUN_004052b0((int)&g_ErrorReport, aMuerrorLog);
  return &g_ErrorReport;
}
#endif

// ── FUN_00422074 (IDA-only, gated) ──
#if defined(IDA_PORT_00422074)
int __stdcall FUN_00422074(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
  void *v4; // esp
  unsigned int v6; // eax
  int v7; // eax
  int v8; // [esp-Ch] [ebp-1778h] BYREF
  int v9; // [esp+0h] [ebp-176Ch]
  int v10; // [esp+4h] [ebp-1768h]
  unsigned int v11; // [esp+8h] [ebp-1764h]
  int v12; // [esp+Ch] [ebp-1760h]
  unsigned int v13; // [esp+10h] [ebp-175Ch]
  int v14; // [esp+14h] [ebp-1758h]
  unsigned int v15; // [esp+18h] [ebp-1754h]
  unsigned int v16; // [esp+1Ch] [ebp-1750h]
  unsigned int v17; // [esp+20h] [ebp-174Ch]
  unsigned int v18; // [esp+24h] [ebp-1748h]
  unsigned int v19; // [esp+28h] [ebp-1744h]
  unsigned int v20; // [esp+2Ch] [ebp-1740h]
  int v21; // [esp+30h] [ebp-173Ch]
  unsigned int v22; // [esp+34h] [ebp-1738h]
  unsigned int v23; // [esp+38h] [ebp-1734h]
  int v24; // [esp+3Ch] [ebp-1730h]
  int v25; // [esp+40h] [ebp-172Ch]
  unsigned int v26; // [esp+44h] [ebp-1728h]
  unsigned int v27; // [esp+48h] [ebp-1724h]
  unsigned int v28; // [esp+4Ch] [ebp-1720h]
  int v29; // [esp+50h] [ebp-171Ch]
  unsigned int v30; // [esp+54h] [ebp-1718h]
  unsigned int v31; // [esp+58h] [ebp-1714h]
  unsigned int v32; // [esp+5Ch] [ebp-1710h]
  unsigned int v33; // [esp+60h] [ebp-170Ch]
  int v34; // [esp+64h] [ebp-1708h]
  unsigned int v35; // [esp+68h] [ebp-1704h]
  unsigned int v36; // [esp+6Ch] [ebp-1700h]
  unsigned int v37; // [esp+70h] [ebp-16FCh]
  unsigned int v38; // [esp+74h] [ebp-16F8h]
  int v39; // [esp+78h] [ebp-16F4h]
  unsigned int v40; // [esp+7Ch] [ebp-16F0h]
  unsigned int v41; // [esp+80h] [ebp-16ECh]
  unsigned int v42; // [esp+84h] [ebp-16E8h]
  unsigned int v43; // [esp+88h] [ebp-16E4h]
  unsigned int v44; // [esp+8Ch] [ebp-16E0h]
  unsigned int v45; // [esp+90h] [ebp-16DCh]
  int v46; // [esp+94h] [ebp-16D8h]
  unsigned int v47; // [esp+98h] [ebp-16D4h]
  unsigned int v48; // [esp+9Ch] [ebp-16D0h]
  int v49; // [esp+A0h] [ebp-16CCh]
  int v50; // [esp+A4h] [ebp-16C8h]
  int v51; // [esp+A8h] [ebp-16C4h]
  unsigned int v52; // [esp+ACh] [ebp-16C0h]
  char v53; // [esp+B0h] [ebp-16BCh]
  char v54; // [esp+B4h] [ebp-16B8h]
  unsigned int v55; // [esp+B8h] [ebp-16B4h]
  unsigned int v56; // [esp+BCh] [ebp-16B0h]
  int v57; // [esp+C0h] [ebp-16ACh]
  unsigned int v58; // [esp+C4h] [ebp-16A8h]
  unsigned int v59; // [esp+C8h] [ebp-16A4h]
  unsigned int v60; // [esp+CCh] [ebp-16A0h]
  unsigned int v61; // [esp+D0h] [ebp-169Ch]
  int v62; // [esp+D4h] [ebp-1698h]
  unsigned int v63; // [esp+D8h] [ebp-1694h]
  unsigned int v64; // [esp+DCh] [ebp-1690h]
  unsigned int v65; // [esp+E0h] [ebp-168Ch]
  unsigned int v66; // [esp+E4h] [ebp-1688h]
  int v67; // [esp+E8h] [ebp-1684h]
  unsigned int v68; // [esp+ECh] [ebp-1680h]
  unsigned int v69; // [esp+F0h] [ebp-167Ch]
  unsigned int v70; // [esp+F4h] [ebp-1678h]
  unsigned int v71; // [esp+F8h] [ebp-1674h]
  int v72; // [esp+FCh] [ebp-1670h]
  unsigned int v73; // [esp+100h] [ebp-166Ch]
  unsigned int v74; // [esp+104h] [ebp-1668h]
  DWORD v75; // [esp+108h] [ebp-1664h]
  exception *v76; // [esp+10Ch] [ebp-1660h]
  exception *v77; // [esp+110h] [ebp-165Ch]
  int v78; // [esp+114h] [ebp-1658h]
  int v79; // [esp+118h] [ebp-1654h]
  void *v80; // [esp+11Ch] [ebp-1650h]
  int v81; // [esp+120h] [ebp-164Ch]
  int v82; // [esp+124h] [ebp-1648h]
  void *v83; // [esp+128h] [ebp-1644h]
  int v84; // [esp+12Ch] [ebp-1640h]
  BYTE *v85; // [esp+130h] [ebp-163Ch]
  BYTE *v86; // [esp+134h] [ebp-1638h]
  BYTE *v87; // [esp+138h] [ebp-1634h]
  BYTE *v88; // [esp+13Ch] [ebp-1630h]
  BYTE *v89; // [esp+140h] [ebp-162Ch]
  int v90; // [esp+144h] [ebp-1628h]
  BYTE *v91; // [esp+148h] [ebp-1624h]
  DWORD Width; // [esp+14Ch] [ebp-1620h]
  int v93; // [esp+150h] [ebp-161Ch]
  int v94; // [esp+154h] [ebp-1618h]
  int v95; // [esp+158h] [ebp-1614h]
  unsigned int v99; // [esp+168h] [ebp-1604h]
  unsigned int v100; // [esp+16Ch] [ebp-1600h]
  int v101; // [esp+170h] [ebp-15FCh]
  unsigned int v102; // [esp+174h] [ebp-15F8h]
  unsigned int v103; // [esp+178h] [ebp-15F4h]
  int v104; // [esp+17Ch] [ebp-15F0h]
  int v105; // [esp+180h] [ebp-15ECh]
  int v106; // [esp+184h] [ebp-15E8h]
  int v107; // [esp+188h] [ebp-15E4h]
  unsigned int v108; // [esp+18Ch] [ebp-15E0h]
  int v109; // [esp+190h] [ebp-15DCh]
  int v110; // [esp+194h] [ebp-15D8h]
  int v111; // [esp+198h] [ebp-15D4h]
  int v112; // [esp+19Ch] [ebp-15D0h]
  unsigned int v113; // [esp+1A0h] [ebp-15CCh]
  int v114; // [esp+1A4h] [ebp-15C8h]
  int v115; // [esp+1A8h] [ebp-15C4h]
  unsigned int v116; // [esp+1ACh] [ebp-15C0h]
  int v117; // [esp+1B0h] [ebp-15BCh]
  int v118; // [esp+1B4h] [ebp-15B8h]
  unsigned int v119; // [esp+1B8h] [ebp-15B4h]
  int v120; // [esp+1BCh] [ebp-15B0h]
  int v121; // [esp+1C0h] [ebp-15ACh]
  int v122; // [esp+1C4h] [ebp-15A8h]
  int v123; // [esp+1C8h] [ebp-15A4h]
  int v124; // [esp+1CCh] [ebp-15A0h]
  int v125; // [esp+1D0h] [ebp-159Ch]
  int v126; // [esp+1D4h] [ebp-1598h]
  int v127; // [esp+1D8h] [ebp-1594h]
  unsigned int v128; // [esp+1DCh] [ebp-1590h]
  int v129; // [esp+1E0h] [ebp-158Ch]
  int v130; // [esp+1E4h] [ebp-1588h]
  unsigned int v131; // [esp+1E8h] [ebp-1584h]
  int v132; // [esp+1ECh] [ebp-1580h]
  int v133; // [esp+1F0h] [ebp-157Ch]
  int v134; // [esp+1F4h] [ebp-1578h]
  int v135; // [esp+1F8h] [ebp-1574h]
  unsigned int v136; // [esp+1FCh] [ebp-1570h]
  int v137; // [esp+200h] [ebp-156Ch]
  int v138; // [esp+204h] [ebp-1568h]
  unsigned int v139; // [esp+208h] [ebp-1564h]
  int v140; // [esp+20Ch] [ebp-1560h]
  int v141; // [esp+210h] [ebp-155Ch]
  int v142; // [esp+214h] [ebp-1558h]
  int v143; // [esp+218h] [ebp-1554h]
  unsigned int v144; // [esp+21Ch] [ebp-1550h]
  int v145; // [esp+220h] [ebp-154Ch]
  int v146; // [esp+224h] [ebp-1548h]
  unsigned int v147; // [esp+228h] [ebp-1544h]
  int v148; // [esp+22Ch] [ebp-1540h]
  int v149; // [esp+230h] [ebp-153Ch]
  int v150; // [esp+234h] [ebp-1538h]
  int v151; // [esp+238h] [ebp-1534h]
  unsigned int v152; // [esp+23Ch] [ebp-1530h]
  int v153; // [esp+240h] [ebp-152Ch]
  int v154; // [esp+244h] [ebp-1528h]
  unsigned int v155; // [esp+248h] [ebp-1524h]
  int v156; // [esp+24Ch] [ebp-1520h]
  int v157; // [esp+250h] [ebp-151Ch]
  unsigned int v158; // [esp+254h] [ebp-1518h]
  int v159; // [esp+258h] [ebp-1514h]
  int v160; // [esp+25Ch] [ebp-1510h]
  int v161; // [esp+260h] [ebp-150Ch]
  int v162; // [esp+264h] [ebp-1508h]
  int v163; // [esp+268h] [ebp-1504h]
  WORD *v164; // [esp+26Ch] [ebp-1500h]
  int v165; // [esp+270h] [ebp-14FCh]
  int v166; // [esp+274h] [ebp-14F8h]
  signed int v167; // [esp+278h] [ebp-14F4h]
  int v168; // [esp+27Ch] [ebp-14F0h]
  int v169; // [esp+280h] [ebp-14ECh]
  int v170; // [esp+284h] [ebp-14E8h]
  signed int v171; // [esp+288h] [ebp-14E4h]
  int v172; // [esp+28Ch] [ebp-14E0h]
  int v173; // [esp+290h] [ebp-14DCh]
  int v174; // [esp+294h] [ebp-14D8h]
  int v175; // [esp+298h] [ebp-14D4h]
  int v176; // [esp+2ACh] [ebp-14C0h]
  int v177; // [esp+2B0h] [ebp-14BCh]
  int v178; // [esp+2B4h] [ebp-14B8h]
  unsigned char v179[1024]; // [esp+2B8h] [ebp-14B4h] BYREF
  int v180; // [esp+6B8h] [ebp-10B4h]
  char v181[3]; // [esp+6BCh] [ebp-10B0h] BYREF
  char v182[1025]; // [esp+6BFh] [ebp-10ADh] BYREF
  char buf[2]; // [esp+AC0h] [ebp-CACh] BYREF
  char v184[258]; // [esp+AC2h] [ebp-CAAh] BYREF
  char v185[4]; // [esp+BC4h] [ebp-BA8h] BYREF
  int v186; // [esp+BC8h] [ebp-BA4h]
  int v187; // [esp+BCCh] [ebp-BA0h]
  int n; // [esp+BD0h] [ebp-B9Ch]
  char v189[32]; // [esp+BD4h] [ebp-B98h] BYREF
  char v190[4]; // [esp+BF4h] [ebp-B78h] BYREF
  int v191; // [esp+BF8h] [ebp-B74h]
  int v192; // [esp+BFCh] [ebp-B70h]
  int m; // [esp+C00h] [ebp-B6Ch]
  char v194[32]; // [esp+C04h] [ebp-B68h] BYREF
  char v195[4]; // [esp+C24h] [ebp-B48h] BYREF
  int v196; // [esp+C28h] [ebp-B44h]
  int v197; // [esp+C2Ch] [ebp-B40h]
  int k; // [esp+C30h] [ebp-B3Ch]
  char v199[32]; // [esp+C34h] [ebp-B38h] BYREF
  char v200[4]; // [esp+C54h] [ebp-B18h] BYREF
  char v201[4]; // [esp+C58h] [ebp-B14h] BYREF
  unsigned int v202; // [esp+D0Ch] [ebp-A60h]
  int v203; // [esp+D10h] [ebp-A5Ch]
  int v204; // [esp+D14h] [ebp-A58h]
  unsigned int v205; // [esp+D18h] [ebp-A54h]
  int v206; // [esp+D1Ch] [ebp-A50h]
  int v207; // [esp+D20h] [ebp-A4Ch]
  int v208; // [esp+D24h] [ebp-A48h]
  int v209; // [esp+D28h] [ebp-A44h]
  unsigned int v210; // [esp+D2Ch] [ebp-A40h]
  int v211; // [esp+D30h] [ebp-A3Ch]
  int v212; // [esp+D34h] [ebp-A38h]
  unsigned int v213; // [esp+D38h] [ebp-A34h]
  int v214; // [esp+D3Ch] [ebp-A30h]
  int v215; // [esp+D40h] [ebp-A2Ch]
  int v216; // [esp+D44h] [ebp-A28h]
  int v217; // [esp+D48h] [ebp-A24h]
  unsigned int v218; // [esp+D4Ch] [ebp-A20h]
  int v219; // [esp+D50h] [ebp-A1Ch]
  int v220; // [esp+D54h] [ebp-A18h]
  unsigned int v221; // [esp+D58h] [ebp-A14h]
  int v222; // [esp+D5Ch] [ebp-A10h]
  int v223; // [esp+D60h] [ebp-A0Ch]
  int v224; // [esp+D64h] [ebp-A08h]
  int v225; // [esp+D68h] [ebp-A04h]
  unsigned int v226; // [esp+D6Ch] [ebp-A00h]
  int v227; // [esp+D70h] [ebp-9FCh]
  int v228; // [esp+D74h] [ebp-9F8h]
  unsigned int v229; // [esp+D78h] [ebp-9F4h]
  int v230; // [esp+D7Ch] [ebp-9F0h]
  int v231; // [esp+D80h] [ebp-9ECh]
  int v232; // [esp+D84h] [ebp-9E8h]
  int v233; // [esp+D88h] [ebp-9E4h]
  unsigned int v234; // [esp+D8Ch] [ebp-9E0h]
  int v235; // [esp+D90h] [ebp-9DCh]
  int v236; // [esp+D94h] [ebp-9D8h]
  unsigned int v237; // [esp+D98h] [ebp-9D4h]
  int v238; // [esp+D9Ch] [ebp-9D0h]
  int v239; // [esp+DA0h] [ebp-9CCh]
  int v240; // [esp+DA4h] [ebp-9C8h]
  int v241; // [esp+DA8h] [ebp-9C4h]
  exception *v242; // [esp+DACh] [ebp-9C0h]
  DWORD v243; // [esp+DB0h] [ebp-9BCh]
  LPVOID v244; // [esp+DB4h] [ebp-9B8h]
  int v245; // [esp+DB8h] [ebp-9B4h]
  LPVOID lpMem; // [esp+DBCh] [ebp-9B0h]
  int v247; // [esp+DC0h] [ebp-9ACh]
  int v248; // [esp+DC4h] [ebp-9A8h]
  BYTE *v249; // [esp+DC8h] [ebp-9A4h]
  BYTE *v250; // [esp+DCCh] [ebp-9A0h]
  BYTE *v251; // [esp+DD0h] [ebp-99Ch]
  BYTE *v252; // [esp+DD4h] [ebp-998h]
  BYTE *v253; // [esp+DD8h] [ebp-994h]
  int v254; // [esp+DDCh] [ebp-990h]
  BYTE *v255; // [esp+DE0h] [ebp-98Ch]
  int v256; // [esp+DE4h] [ebp-988h]
  void *(__cdecl **v257)(std::locale::facet *__hidden, unsigned int); // [esp+DE8h] [ebp-984h]
  WORD v258[514]; // [esp+DECh] [ebp-980h] BYREF
  int v259; // [esp+11F0h] [ebp-57Ch]
  int i; // [esp+11F4h] [ebp-578h]
  DEVMODEA DevMode[2]; // [esp+11F8h] [ebp-574h] BYREF
  char v262[4]; // [esp+1394h] [ebp-3D8h] BYREF
  char *Str; // [esp+1398h] [ebp-3D4h]
  char ER_SystemInfo[388]; // [esp+139Ch] [ebp-3D0h] BYREF
  CHAR tstrFilename[4]; // [esp+1520h] [ebp-24Ch] BYREF
  char Buffer[256]; // [esp+1624h] [ebp-148h] BYREF
  WORD pwVersion[3]; // [esp+1724h] [ebp-48h] BYREF
  WORD hWnd[3]; // [esp+172Ah] [ebp-42h]
  WORD wPortNumber; // [esp+1730h] [ebp-3Ch] BYREF
  int j; // [esp+1734h] [ebp-38h]
  char pvParam[4]; // [esp+1738h] [ebp-34h] BYREF
  int iFontSize; // [esp+173Ch] [ebp-30h]
  struct tagMSG msg; // [esp+1740h] [ebp-2Ch] BYREF
  int *v274; // [esp+175Ch] [ebp-10h]
  int v275; // [esp+1768h] [ebp-4h]

  v4 = alloca(5980);
  v274 = &v8;
  v275 = 0;
  FUN_00406af0();
  v103 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
  if ( v103 == -1 )
  {
    v101 = operator_new(5u);
    v239 = v101;
    v240 = v101;
    *(BYTE *)(v101 + 4) = 1;
    FUN_00403f80(&DAT_055c9bc8, v240, (int)&DAT_055ca01c);
  }
  else
  {
    v102 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
    v237 = v102;
    if ( v102 == -1 )
    {
      v238 = 0;
      v241 = 0;
    }
    else
    {
      v241 = *(DWORD *)(DAT_055c9bcc + 4 * v237);
    }
    if ( (unsigned char)++*(BYTE *)(v241 + 4) < 2u )
    {
      FUN_00409e20(&DAT_055ca01c, v241);
    }
  }
  v100 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
  if ( v100 != -1 )
  {
    v99 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
    v234 = v99;
    if ( v99 == -1 )
    {
      v235 = 0;
      v236 = 0;
    }
    else
    {
      v236 = *(DWORD *)(DAT_055c9bcc + 4 * v234);
    }
    if ( !--*(BYTE *)(v236 + 4) )
    {
      FUN_00423760(v236, &DAT_055ca01c);
    }
  }
  OpenTextData();
  if ( !UpdateMuExe() )
  {
    return 0;
  }
  strcpy(Buffer, "unknown");
  memset(&Buffer[8], 0, 0xF8u);
  Str = GetCommandLineA();
  memset(pwVersion, 0, sizeof(pwVersion));
  hWnd[0] = 0;
  if ( GetFileNameOfFilePath(tstrFilename, Str) )
  {
    if ( GetFileVersion(tstrFilename, pwVersion) )
    {
      sprintf(Buffer, "%d.%02.d", pwVersion[0], pwVersion[1]);
      if ( pwVersion[2] )
      {
        v262[1] = (BYTE)((DAT_00559470) >> 8);
        v262[0] = (BYTE)(pwVersion[2]) - 1 + DAT_00559470;
        strcat(Buffer, v262);
      }
    }
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, "\r\n");
  CErrorReport::WriteLogBegin((DWORD)&g_ErrorReport);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  CErrorReport::Write(
    (DWORD)&g_ErrorReport,
    "Mu online %s (%s) executed. (%d.%d.%d.%d)\r\n",
    Buffer,
    (const char *)DAT_055c9e50,
    pwVersion[0],
    pwVersion[1],
    pwVersion[2],
    hWnd[0]);
  CErrorReport::WriteCurrentTime((DWORD)&g_ErrorReport, 1);
  memset(ER_SystemInfo, 0, sizeof(ER_SystemInfo));
  GetSystemInfo((DWORD)ER_SystemInfo);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  CErrorReport::WriteSystemInfo((DWORD)&g_ErrorReport, (DWORD)ER_SystemInfo);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  if ( GetConnectServerInfo(szCmdLine, g_lpszCmdURL, &wPortNumber) )
  {
    szServerIpAddress = g_lpszCmdURL;
    g_ServerPort = wPortNumber;
  }
  if ( !strlen(szCmdLine) )
  {
    strcpy((char *)&DevMode[0].dmPanningHeight, TextMu);
    WinExec((LPCSTR)&DevMode[0].dmPanningHeight, 5u);
    return 0;
  }
  *(DWORD *)&hWnd[1] = FindWindowA(aDialog, aMu);
  if ( *(DWORD *)&hWnd[1] )
  {
    SendMessageA(*(HWND *)&hWnd[1], 0x10u, 0, 0);
  }
  if ( !OpenMainExe() )
  {
    return 0;
  }
  CSimpleModulus::LoadEncryptionKey((DWORD)&g_SimpleModulusCS, aDataEnc1Dat);
  CSimpleModulus::LoadDecryptionKey((DWORD)&g_SimpleModulusSC, szFileName);
  if ( !Config_Load() )
  {
    CErrorReport::Write((DWORD)&g_ErrorReport, aConfigIniReadE);
    return 0;
  }
  for ( DevMode[0].dmPanningWidth = 0;
        EnumDisplaySettingsA(0, DevMode[0].dmPanningWidth, DevMode);
        ++DevMode[0].dmPanningWidth )
  {
    ;
  }
  v95 = operator_new(148 * DevMode[0].dmPanningWidth);
  v256 = v95;
  v259 = v95;
  for ( DevMode[0].dmPanningWidth = 0;
        EnumDisplaySettingsA(0, DevMode[0].dmPanningWidth, (DEVMODEA *)(148 * DevMode[0].dmPanningWidth + v259));
        ++DevMode[0].dmPanningWidth )
  {
    ;
  }
  for ( i = 0; i < (int)DevMode[0].dmPanningWidth; ++i )
  {
    if ( *(DWORD *)(v259 + 148 * i + 108) == WindowWidth
      && *(DWORD *)(v259 + 148 * i + 112) == WindowHeight
      && *(DWORD *)(v259 + 148 * i + 104) == 16 )
    {
      ChangeDisplaySettingsA((DEVMODEA *)(148 * i + v259), 0);
      break;
    }
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, "> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);
  g_hInst = hInstance;
  g_hWnd = StartWindow(hInstance, nCmdShow);
  CErrorReport::Write((DWORD)&g_ErrorReport, aStartWindowSuc);
  (BYTE)(v94) = CreateOpenglWindow();
  if ( !(BYTE)v94 )
  {
    return 0;
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, aOpenglInitSucc);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  CErrorReport::WriteOpenGLInfo((DWORD)&g_ErrorReport);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  FUN_004058b0(&g_ErrorReport);
  ShowWindow(g_hWnd, nCmdShow);
  UpdateWindow(g_hWnd);
  (BYTE)(v93) = npGameGuard::init((int)g_hWnd);
  if ( !(BYTE)v93 )
  {
    CErrorReport::Write((DWORD)&g_ErrorReport, aGgInitError);
    return 0;
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, aGgInitSuccess);
  CErrorReport::WriteImeInfo((DWORD)&g_ErrorReport, g_hWnd);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  FUN_00406db0(&DAT_055c9bc8, (int)g_hWnd, 1025);// Esta función no aparece en el S5
  Width = WindowWidth;
  if ( WindowWidth > 1024 )
  {
    if ( Width == 1280 )
    {
      FontHeight = 15;
    }
  }
  else
  {
    switch ( Width )
    {
      case 1024u:
        FontHeight = 14;
        break;
      case 640u:
        FontHeight = 12;
        break;
      case 800u:
        FontHeight = 13;
        break;
    }
  }
  iFontSize = FontHeight - 1;
  g_hFont = CreateFontA(
              FontHeight - 1,
              0,
              0,
              0,
              400,
              0,
              0,
              0,
              g_dwCharSet[0],
              0,
              0,
              NONANTIALIASED_QUALITY,   /* FIX 2026-07-25: sin esto la fuente sale
                 con AA/ClearType; el compose de burbujas (FUN_0047f360) hace
                 threshold (todo pixel != 0 -> color de texto), asi que el fringe
                 AA se ve como el nombre dibujado 2 veces. 1-bit = crisp como el
                 MU original. */
              0,
              GlobalText[0][0] != 0 ? GlobalText[0] : 0);
  g_hFontBold = CreateFontA(
                  iFontSize,
                  0,
                  0,
                  0,
                  700,
                  0,
                  0,
                  0,
                  g_dwCharSet[0],
                  0,
                  0,
                  0,
                  0,
                  GlobalText[0][0] != 0 ? GlobalText[0] : 0);
  g_hFontBig = CreateFontA(
                 2 * iFontSize,
                 0,
                 0,
                 0,
                 700,
                 0,
                 0,
                 0,
                 g_dwCharSet[0],
                 0,
                 0,
                 NONANTIALIASED_QUALITY,   /* ver nota en g_hFont */
                 0,
                 GlobalText[0][0] != 0 ? GlobalText[0] : 0);
  setlocale(0, lpszLocale);
  if ( m_SoundOnOff )
  {
    InitDirectSound(g_hWnd);
  }
  SetTimer(g_hWnd, 0x3E8u, 20000u, 0);
  v6 = time(0);
  srand(v6);
  for ( j = 0; j < 100; ++j )
  {
    RandomTable[j] = rand() % 360;
  }
  v7 = rand();
  v91 = (BYTE *)operator_new(v7 % 100 + 1);
  v255 = v91;
  RendomMemoryDump = v91;
  v90 = operator_new(0x384u);
  v254 = v90;
  GateAttribute = v90;
  v89 = (BYTE *)operator_new(0xA00u);
  v253 = v89;
  SkillAttribute = v89;
  v88 = (BYTE *)operator_new(0xA00u);
  v252 = v88;
  SkillAttribute2 = v88;
  v87 = (BYTE *)operator_new(98304u);
  v251 = v87;
  ItemAttRibuteMemoryDump = v87;
  ItemAttribute = (ITEM_ATTRIBUTE *)&ItemAttRibuteMemoryDump[64 * (rand() % 1024)];
  v86 = (BYTE *)operator_new(0x8000u);
  v250 = v86;
  ItemAttribute2 = v86;
  v85 = (BYTE *)operator_new(0x764D4u);
  v249 = v85;
  CharacterMemoryDump = v85;
  CharactersClient = (DWORD)&CharacterMemoryDump[916 * (rand() % 128)];
  v84 = operator_new(1412u);
  v248 = v84;
  CharacterMachine = v84;
  memset((void *)GateAttribute, 0, 0x384u);
  memset(SkillAttribute, 0, 0xA00u);
  memset(SkillAttribute2, 0, 0xA00u);
  memset(ItemAttribute, 0, 0x8000u);
  memset(ItemAttribute2, 0, 0x8000u);
  memset((void *)CharactersClient, 0, 0x59AD4u);
  memset((void *)CharacterMachine, 0, 0x584u);
  CharacterAttribute = CharacterMachine;
  CHARACTER_MACHINE::Init(CharacterMachine);
  Hero = CharactersClient;
  v83 = (void *)operator_new(0x5C8u);
  lpMem = v83;
  (BYTE)(v275) = 1;
  if ( v83 )
  {
    v82 = FUN_0040c7d0((int)lpMem);
    v81 = v82;
  }
  else
  {
    v81 = 0;
  }
  v247 = v81;
  (BYTE)(v275) = 0;
  DAT_055c9ff0 = v81;
  v80 = (void *)operator_new(0xBCu);
  v244 = v80;
  (BYTE)(v275) = 2;
  if ( v80 )
  {
    v79 = FUN_0040e990((int)v244);
    v78 = v79;
  }
  else
  {
    v78 = 0;
  }
  v245 = v78;
  (BYTE)(v275) = 0;
  DAT_055c9ff4 = v78;
  v77 = (exception *)operator_new(0xCu);
  v242 = v77;
  (BYTE)(v275) = 3;
  if ( v77 )
  {
    v76 = exception::exception(v242);
    v75 = (DWORD)v76;
  }
  else
  {
    v75 = 0;
  }
  v243 = v75;
  (BYTE)(v275) = 0;
  g_pRenderText = v75;
  SystemParametersInfoA(0x61u, 1u, pvParam, 0);
  SystemParametersInfoA(0xEu, 0, &g_iScreenSaverOldValue, 0);
  SystemParametersInfoA(0xFu, 18000u, 0, 0);
  RegisterHotKey(g_hWnd, 0, 1u, 9u);
  while ( 1 )
  {
    while ( !PeekMessageA(&msg, 0, 0, 0, 0) )
    {
      v74 = FUN_004041e0(&DAT_055c9bc8, (int)&g_iNoMouseTime);
      if...
      if ( ++g_iNoMouseTime > 30 )
      {
        v71 = FUN_004041e0(&DAT_055c9bc8, (int)&g_iNoMouseTime);
        if...
        v69 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca024);
        if...
        if...
        if...
        v31 = FUN_004041e0(&DAT_055c9bc8, (int)&g_iNoMouseTime);
        if...
      }
      v28 = FUN_004041e0(&DAT_055c9bc8, (int)&g_iNoMouseTime);
      if...
      v26 = FUN_004041e0(&DAT_055c9bc8, (int)&g_bWndActive);
      if...
      (BYTE)(v127) = g_bWndActive;
      PACKET_ENCRYPT(&DAT_055c9bc8, &g_bWndActive);
      if ( (BYTE)v127 )
      {
        Scene_Dispatch(g_hDC);
      }
      else
      {
        SetForegroundWindow(g_hWnd);
        SetFocus(g_hWnd);
        v23 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca02c);
        if ( v23 == -1 )
        {
          v21 = operator_new(5u);
          v121 = v21;
          v122 = v21;
          *(BYTE *)(v21 + 4) = 1;
          FUN_00403f80(&DAT_055c9bc8, v122, (int)&DAT_055ca02c);
        }
        else
        {
          v22 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca02c);
          v119 = v22;
          if ( v22 == -1 )
          {
            v120 = 0;
            v123 = 0;
          }
          else
          {
            v123 = *(DWORD *)(DAT_055c9bcc + 4 * v119);
          }
          if ( (unsigned char)++*(BYTE *)(v123 + 4) < 2u )
          {
            FUN_00409e20(&DAT_055ca02c, v123);
          }
        }
        if ( DAT_055ca02c <= 1 )
        {
          ++DAT_055ca02c;
          v18 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca02c);
          if ( v18 != -1 )
          {
            v17 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca02c);
            v113 = v17;
            if ( v17 == -1 )
            {
              v114 = 0;
              v115 = 0;
            }
            else
            {
              v115 = *(DWORD *)(DAT_055c9bcc + 4 * v113);
            }
            if ( !--*(BYTE *)(v115 + 4) )
            {
              FUN_00423760(v115, &DAT_055ca02c);
            }
          }
          v16 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
          if ( v16 == -1 )
          {
            v14 = operator_new(5u);
            v110 = v14;
            v111 = v14;
            *(BYTE *)(v14 + 4) = 1;
            FUN_00403f80(&DAT_055c9bc8, v111, (int)&DAT_055ca01c);
          }
          else
          {
            v15 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
            v108 = v15;
            if ( v15 == -1 )
            {
              v109 = 0;
              v112 = 0;
            }
            else
            {
              v112 = *(DWORD *)(DAT_055c9bcc + 4 * v108);
            }
            if ( (unsigned char)++*(BYTE *)(v112 + 4) < 2u )
            {
              FUN_00409e20(&DAT_055ca01c, v112);
            }
          }
          DAT_055ca01c = 1;
          v13 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
          if ( v13 != -1 )
          {
            v12 = FUN_00404280(&DAT_055c9bc8, &DAT_055ca01c);
            v107 = v12;
            if ( !--*(BYTE *)(v107 + 4) )
            {
              FUN_00423760(v107, &DAT_055ca01c);
            }
          }
          ShowWindow(g_hWnd, 6);
          v11 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca01c);
          if ( v11 == -1 )
          {
            v9 = operator_new(5u);
            v104 = v9;
            v105 = v9;
            *(BYTE *)(v9 + 4) = 1;
            FUN_00403f80(&DAT_055c9bc8, v105, (int)&DAT_055ca01c);
          }
          else
          {
            v10 = FUN_00404280(&DAT_055c9bc8, &DAT_055ca01c);
            v106 = v10;
            ++*(BYTE *)(v10 + 4);
            if ( *(unsigned char *)(v106 + 4) < 2u )
            {
              FUN_00409e20(&DAT_055ca01c, v106);
            }
          }
          DAT_055ca01c = 0;
          FUN_004233e0(&DAT_055c9bc8, &DAT_055ca01c);
          ShowWindow(g_hWnd, 3);
        }
        else
        {
          v20 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca02c);
          if ( v20 != -1 )
          {
            v19 = FUN_004041e0(&DAT_055c9bc8, (int)&DAT_055ca02c);
            v116 = v19;
            if ( v19 == -1 )
            {
              v117 = 0;
              v118 = 0;
            }
            else
            {
              v118 = *(DWORD *)(DAT_055c9bcc + 4 * v116);
            }
            if ( !--*(BYTE *)(v118 + 4) )
            {
              FUN_00423760(v118, &DAT_055ca02c);
            }
          }
          SetTimer(g_hWnd, 0x3E9u, 0x3E8u, 0);
          PostMessageA(g_hWnd, 0x10u, 0, 0);
        }
      }
LABEL_306:
      ProtocolCore();
    }
    if ( !GetMessageA(&msg, 0, 0, 0) )
    {
      break;
    }
    TranslateMessage(&msg);
    if ( msg.message != 260 && msg.message != 261 )
    {
      DispatchMessageA(&msg);
      goto LABEL_306;
    }
  }
  v275 = -1;
  DestroyWindow();
  return msg.wParam;
}
#endif

// ── FUN_00422c50 (IDA-only, gated) ──
#if defined(IDA_PORT_00422C50)
void __cdecl FUN_00422c50(int _this)
{
  LPVOID *v2; // ecx
  DWORD *v3; // esi
  int v4; // eax
  int *v5; // ecx
  int v6; // edi
  int v7; // ebx
  DWORD *v8; // edx
  int v9; // eax
  LPVOID lpMem; // [esp+8h] [ebp-8h] BYREF
  int v11; // [esp+Ch] [ebp-4h] BYREF

  if ( *(DWORD *)(_this + 1020) )
  {
    delete__(*(LPVOID *)(_this + 1020));
    *(DWORD *)(_this + 1020) = 0;
  }
  if ( *(DWORD *)(_this + 1032) )
  {
    delete__(*(LPVOID *)(_this + 1032));
    *(DWORD *)(_this + 1032) = 0;
  }
  if ( *(DWORD *)(_this + 1036) )
  {
    delete__(*(LPVOID *)(_this + 1036));
    *(DWORD *)(_this + 1036) = 0;
  }
  if ( *(DWORD *)(_this + 1040) )
  {
    delete__(*(LPVOID *)(_this + 1040));
    *(DWORD *)(_this + 1040) = 0;
  }
  *(DWORD *)(_this + 1024) = 1950000000;
  *(DWORD *)(_this + 1028) = -1;
  v2 = *(LPVOID **)(_this + 1052);
  v3 = (DWORD *)(_this + 1044);
  *v3 = &DAT_00552840;
  if ( !v2 )
  {
    v3[1] = 0;
    return;
  }
  lpMem = v2;
  if ( v2[2] )
  {
    FUN_004235d0(v2[2]);
    v2 = (LPVOID *)lpMem;
  }
  if ( v2[3] )
  {
    FUN_004235d0(v2[3]);
    v2 = (LPVOID *)lpMem;
  }
  v11 = 0;
  v4 = (int)v2[2];
  if ( !v4 )
  {
    v4 = (int)v2[3];
    if ( !v4 )
    {
      FUN_004236c0(&lpMem);
      v3[2] = 0;
      v3[1] = 0;
      return;
    }
    if ( v2 == (LPVOID *)v3[2] )
    {
      v3[2] = v4;
      *(DWORD *)(v4 + 16) = 0;
      goto LABEL_27;
    }
LABEL_23:
    v8 = v2[4];
    if ( v2 == (LPVOID *)v8[3] )
    {
      v8[3] = v4;
    }
    else
    {
      v8[2] = v4;
    }
    *(DWORD *)(v4 + 16) = v8;
    goto LABEL_27;
  }
  if ( !v2[3] )
  {
    if ( v2 == (LPVOID *)v3[2] )
    {
      v3[2] = v4;
      *(DWORD *)(v4 + 16) = 0;
LABEL_27:
      if ( lpMem )
      {
        delete__(lpMem);
      }
      v9 = v3[1];
      v3[2] = 0;
      v3[1] = v9 - 1;
      v3[1] = 0;
      return;
    }
    goto LABEL_23;
  }
  do
  {
    v5 = (int *)v4;
    v11 = v4;
    v4 = *(DWORD *)(v4 + 12);
  }
  while ( v4 );
  v6 = *v5;
  v7 = v5[1];
  FUN_004232f0(&v11);
  *(DWORD *)lpMem = v6;
  *((DWORD *)lpMem + 1) = v7;
  v3[2] = 0;
  v3[1] = 0;
}
#endif

// ── FUN_004233e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004233E0)
void __cdecl FUN_004233e0(int *_this, char *a2)
{
  int v3; // edx
  int v4; // eax
  int v5; // ecx
  unsigned int v6; // edx
  unsigned int v7; // ecx
  int v8; // edx
  unsigned int v9; // edx
  int v10; // eax
  unsigned int v11; // ecx
  int v12; // edx
  char v13; // al
  DWORD *v14; // ebx
  unsigned int v15; // edi
  BYTE *v16; // esi
  char v17; // al
  char v18; // al
  int v19; // [esp+10h] [ebp-Ch]
  int v20; // [esp+10h] [ebp-Ch]
  DWORD *v21; // [esp+10h] [ebp-Ch]
  char *v22; // [esp+14h] [ebp-8h] BYREF
  char *v23; // [esp+18h] [ebp-4h] BYREF

  v3 = *_this;
  v23 = a2;
  v4 = (*(int (__cdecl **)(int *, char *))(v3 + 12))(_this, a2);
  v5 = _this[3];
  v6 = v4;
  v22 = 0;
  v19 = 0;
  if ( v5 )
  {
    while ( memcmp((const char *)&v22, (const char *)(_this[2] + 4 * v6), 4) )
    {
      if ( !memcmp((const char *)&v23, (const char *)(_this[2] + 4 * v6), 4) )
      {
        if ( v6 != -1 )
        {
          v8 = *_this;
          v22 = a2;
          v9 = (*(int (__cdecl **)(int *, char *))(v8 + 12))(_this, a2);
          v10 = _this[3];
          v23 = 0;
          v20 = 0;
          if ( v10 )
          {
            while ( memcmp((const char *)&v23, (const char *)(_this[2] + 4 * v9), 4) )
            {
              if ( !memcmp((const char *)&v22, (const char *)(_this[2] + 4 * v9), 4) )
              {
                if ( v9 == -1 )
                {
                  break;
                }
                v12 = *(DWORD *)(_this[1] + 4 * v9);
                v21 = (DWORD *)v12;
                goto LABEL_13;
              }
              v11 = _this[3];
              v9 = (v9 + 1) % v11;
              if ( ++v20 >= v11 )
              {
                goto LABEL_11;
              }
            }
          }
          else
          {
LABEL_11:
            CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
          }
          v21 = 0;
          v12 = 0;
LABEL_13:
          v13 = *(BYTE *)(v12 + 4) - 1;
          *(BYTE *)(v12 + 4) = v13;
          if ( !v13 )
          {
            v14 = (DWORD *)operator_new(4u);
            v15 = 0;
            *v14 = *(DWORD *)a2;
            v16 = v14;
            do
            {
              v17 = *v16 + 71;
              *v16 = v17;
              v18 = (v17 ^ DAT_00559050[v15 & 0x8000000F]) + 35;
              *v16 = v18;
              if ( v15 < 3 )
              {
                *v16 = v18 ^ *((BYTE *)v14 + v15 + 1);
              }
              v16[a2 - (char *)v14] = rand();
              ++v15;
              ++v16;
            }
            while ( v15 < 4 );
            *v21 = *v14;
            delete__(v14);
          }
        }
        return;
      }
      v7 = _this[3];
      v6 = (v6 + 1) % v7;
      if ( ++v19 >= v7 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
}
#endif

// ── FUN_004236c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004236C0)
void __cdecl FUN_004236c0(DWORD *_this, LPVOID *a2)
{
  int v2; // eax

  v2 = *((DWORD *)*a2 + 4);
  if ( v2 )
  {
    if ( *(LPVOID *)(v2 + 8) == *a2 )
    {
      *(DWORD *)(v2 + 8) = 0;
    }
    else
    {
      *(DWORD *)(v2 + 12) = 0;
    }
  }
  else
  {
    _this[2] = 0;
  }
  --_this[1];
  if ( *a2 )
  {
    delete__(*a2);
  }
  *a2 = 0;
}
#endif

// ── FUN_00423c30 (IDA-only, gated) ──
#if defined(IDA_PORT_00423C30)
// Microsoft VisualC 2-14/net runtime
BOOL FUN_00423c30()
{
  return CWsctlc::Close((DWORD)&SocketClient);
}
#endif

// ── FUN_00433830 (IDA-only, gated) ──
#if defined(IDA_PORT_00433830)
void __cdecl FUN_00433830(BYTE *ReceiveBuffer)
{
  BYTE *v1; // esi
  int v2; // edi
  DWORD *p_Key; // esi

  if ( EquipmentItem )
  {
    if ( !g_bPacketAfter_EquipmentItem )
    {
      g_bPacketAfter_EquipmentItem = 1;
      *(DWORD *)g_byPacketAfter_EquipmentItem = *(DWORD *)ReceiveBuffer;
    }
  }
  else
  {
    v1 = ReceiveBuffer;
    if ( !ReceiveBuffer[3] )
    {
      UIChatLogWindow_AddText(DAT_05826d54, GlobalText[494], 2);
      DAT_07eaa0e8 = 0;
      v2 = 0;
      p_Key = &Inventory[0].Key;
      do
      {
        if ( *((WORD *)p_Key - 28) != 0xFFFF && *p_Key )
        {
          FUN_004cc530(v2, (int)Inventory);
        }
        p_Key += 17;
        ++v2;
      }
      while ( (int)p_Key < (int)&Inventory[32].Key );
      v1 = ReceiveBuffer;
    }
    if ( v1[3] == 2 )
    {
      UIChatLogWindow_AddText(DAT_05826d58, GlobalText[495], 2);
    }
    if ( v1[3] == 3 )
    {
      UIChatLogWindow_AddText(DAT_05826d5c, GlobalText[496], 2);
      SetErrorMessage(0);
    }
    InventoryOpened = 0;
    CloseInventoryRelatedWindows();
    DAT_07e91388 = 0;
    if ( ErrorMessage == 116 )
    {
      SetErrorMessage(0);
      ClearInput(0);
      InputTextMax[0] = 42;
      InputNumber = 2;
      GoldInputEnable = 0;
      InputEnable = 0;
    }
  }
}
#endif

// ── FUN_0043d670 (IDA-only, gated) ──
#if defined(IDA_PORT_0043D670)
int __cdecl FUN_0043d670(int *_this, char *a2)
{
  char *v3; // esi
  int v4; // eax
  int v5; // eax
  int v6; // ecx
  unsigned int v7; // edx
  unsigned int v8; // ecx
  unsigned int v9; // eax
  int v10; // edi
  unsigned char v11; // al
  DWORD *v12; // esi
  unsigned int v13; // ecx
  int v14; // edi
  int v15; // edx
  int v16; // eax
  int v17; // ecx
  unsigned int v18; // ebx
  unsigned int v19; // ecx
  bool v20; // cf
  int v22; // eax
  unsigned int v23; // eax
  int v24; // eax
  char v25; // cl
  int v26; // [esp+10h] [ebp-10h]
  int v27; // [esp+10h] [ebp-10h]
  int v28; // [esp+14h] [ebp-Ch] BYREF
  char *v29; // [esp+18h] [ebp-8h] BYREF
  char *v30; // [esp+1Ch] [ebp-4h] BYREF

  v3 = a2;
  v4 = *_this;
  v29 = a2;
  v5 = (*(int (__cdecl **)(int *, char *))(v4 + 12))(_this, a2);
  v6 = _this[3];
  v7 = v5;
  v28 = 0;
  v26 = 0;
  if ( v6 )
  {
    while ( 1 )
    {
      if ( !memcmp((const char *)&v28, (const char *)(_this[2] + 4 * v7), 4) )
      {
        goto LABEL_23;
      }
      if ( !memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v7), 4) )
      {
        break;
      }
      v8 = _this[3];
      v7 = (v7 + 1) % v8;
      if ( ++v26 >= v8 )
      {
        v3 = a2;
        goto LABEL_6;
      }
    }
    if ( v7 == -1 )
    {
LABEL_23:
      v3 = a2;
      goto LABEL_24;
    }
    v9 = FUN_004041e0(_this, (int)a2);
    if ( v9 == -1 )
    {
      v10 = 0;
    }
    else
    {
      v10 = *(DWORD *)(_this[1] + 4 * v9);
    }
    v11 = *(BYTE *)(v10 + 4) + 1;
    *(BYTE *)(v10 + 4) = v11;
    if ( v11 < 2u )
    {
      v12 = (DWORD *)operator_new(4u);
      v13 = 3;
      *v12 = *(DWORD *)v10;
      v14 = 4;
      do
      {
        if ( v13 < 3 )
        {
          *((BYTE *)v12 + v13) ^= *((BYTE *)v12 + v13 + 1);
        }
        *((BYTE *)v12 + v13) = ((*((BYTE *)v12 + v13) - 35) ^ DAT_00559050[(int)v13 % 16]) - 71;
        --v13;
        --v14;
      }
      while ( v14 );
      *(DWORD *)a2 = *v12;
      delete__(v12);
    }
    v3 = a2;
  }
  else
  {
LABEL_6:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
LABEL_24:
    v22 = operator_new(5u);
    *(BYTE *)(v22 + 4) = 1;
    FUN_00403f80(_this, v22, (int)v3);
  }
  v15 = *_this;
  v27 = *(DWORD *)v3;
  v30 = v3;
  v16 = (*(int (__cdecl **)(int *, char *))(v15 + 12))(_this, v3);
  v17 = _this[3];
  v18 = v16;
  v29 = 0;
  v28 = 0;
  if ( v17 )
  {
    while ( memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v18), 4) )
    {
      if ( !memcmp((const char *)&v30, (const char *)(_this[2] + 4 * v18), 4) )
      {
        if ( v18 != -1 )
        {
          v23 = FUN_004041e0(_this, (int)a2);
          if ( v23 == -1 )
          {
            v24 = 0;
          }
          else
          {
            v24 = *(DWORD *)(_this[1] + 4 * v23);
          }
          v25 = *(BYTE *)(v24 + 4) - 1;
          *(BYTE *)(v24 + 4) = v25;
          if ( !v25 )
          {
            FUN_00423760((DWORD *)v24, a2);
          }
        }
        return v27;
      }
      v19 = _this[3];
      v20 = ++v28 < v19;
      v18 = (v18 + 1) % v19;
      if ( !v20 )
      {
        goto LABEL_22;
      }
    }
    return v27;
  }
  else
  {
LABEL_22:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    return v27;
  }
}
#endif

// ── FUN_0043daf0 (IDA-only, gated) ──
#if defined(IDA_PORT_0043DAF0)
DWORD *__cdecl FUN_0043daf0(DWORD *_this)
{
  memset(_this + 4103, 0, 0x258960u);
  _this[4101] = 0;
  _this[4102] = 0;
  return _this;
}
#endif

// ── FUN_0043dc90 (IDA-only, gated) ──
#if defined(IDA_PORT_0043DC90)
BOOL __cdecl CWsctlc::Close(DWORD This)
{
  g_bGameServerConnected = 0;
  closesocket(*(DWORD *)(This + 8));
  *(DWORD *)(This + 8) = -1;
  return 1;
}
#endif

// ── FUN_0043ddd0 (IDA-only, gated) ──
#if defined(IDA_PORT_0043DDD0)
int __cdecl FUN_0043ddd0(const char *This)
{
  int v2; // ebx
  const char *v3; // ebp
  int v4; // eax
  int v5; // edi
  int v6; // eax

  v2 = 0;
  if ( *((int *)This + 2051) <= 0 )
  {
    return 1;
  }
  v3 = This + 12;
  while ( 1 )
  {
    v4 = send(*((DWORD *)This + 2), &v3[v2], *((DWORD *)This + 2051) - v2, 0);
    v5 = v4;
    if ( v4 == -1 )
    {
      break;
    }
    if ( v4 <= 0 )
    {
      goto LABEL_10;
    }
    if ( *((DWORD *)This + 4101) )
    {
      nullsub_2(This + 12, v4);
    }
    v2 += v5;
    v6 = *((DWORD *)This + 2051) - v5;
    *((DWORD *)This + 2051) = v6;
    if ( v6 <= 0 )
    {
      return 1;
    }
  }
  if ( WSAGetLastError() == 10035 )
  {
    return 1;
  }
LABEL_10:
  CWsctlc::Close((DWORD)This);
  return 0;
}
#endif

// ── FUN_0043de70 (IDA-only, gated) ──
#if defined(IDA_PORT_0043DE70)
int __cdecl FUN_0043de70(SOCKET *_this)
{
  int v2; // eax
  int v4; // edx
  int v5; // edi
  char v6; // al
  int v7; // ebx
  int v8; // eax
  signed int v9; // ecx

  v2 = recv(_this[2], (char *)_this + _this[4100] + 8208, 0x2000 - _this[4100], 0);
  if ( !v2 )
  {
    return 1;
  }
  if ( v2 == -1 )
  {
    WSAGetLastError();
    return 1;
  }
  else
  {
    v4 = v2 + _this[4100];
    _this[4100] = v4;
    if ( v4 >= 3 )
    {
      v5 = 0;
      do
      {
        v6 = *((BYTE *)_this + v5 + 8208);
        if ( v6 == -63 || v6 == -61 )
        {
          v7 = *((unsigned char *)_this + v5 + 8209);
        }
        else
        {
          if ( v6 != -62 && v6 != -60 )
          {
            _this[4100] = 0;
            return 0;
          }
          v7 = *((unsigned char *)_this + v5 + 8210) + (*((unsigned char *)_this + v5 + 8209) << 8);
        }
        if ( v7 <= 0 )
        {
          break;
        }
        if ( v7 > (int)_this[4100] )
        {
          if ( v5 > 0 )
          {
            v9 = _this[4100];
            if ( v9 >= 1 )
            {
              qmemcpy(_this + 2052, (char *)_this + v5 + 8208, v9);
            }
          }
          return 0;
        }
        FUN_0043df90((char *)_this + v5 + 8208, v7);
        if ( _this[4101] )
        {
          nullsub_2((int)_this + v5 + 8208, v7);
        }
        v5 += v7;
        v8 = _this[4100] - v7;
        _this[4100] = v8;
      }
      while ( v8 > 0 );
      return 0;
    }
    else
    {
      return 3;
    }
  }
}
#endif

// ── FUN_00442260 (IDA-only, gated) ──
#if defined(IDA_PORT_00442260)
void __cdecl BMD::FindNearTriangle(DWORD This)
{
  int v2; // ebx
  int v3; // eax
  int v4; // ebp
  int v5; // eax
  int v6; // ecx
  int v7; // esi
  int i; // esi
  int v9; // [esp+8h] [ebp-4h]

  v2 = 0;
  if ( *(short *)(This + 36) > 0 )
  {
    v9 = 0;
    do
    {
      v3 = v9 + *(DWORD *)(This + 40);
      v4 = *(short *)(v3 + 10);
      if ( v4 > 0 )
      {
        v5 = *(DWORD *)(v3 + 28) + 26;
        v6 = v4;
        do
        {
          v7 = v5;
          v5 += 36;
          --v6;
          *(DWORD *)v7 = -1;
          *(WORD *)(v7 + 4) = -1;
        }
        while ( v6 );
      }
      for ( i = 0; i < v4; ++i )
      {
        BMD::FindTriangleForEdge(This, v2, i, 0);
        BMD::FindTriangleForEdge(This, v2, i, 1);
        BMD::FindTriangleForEdge(This, v2, i, 2);
      }
      ++v2;
      v9 += 40;
    }
    while ( v2 < *(short *)(This + 36) );
  }
}
#endif

// ── FUN_004422f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004422F0)
void __cdecl BMD::FindTriangleForEdge(DWORD This, int iMesh, int iTri1, int iIndex11)
{
  int v4; // ecx
  int v5; // edx
  int v6; // ebx
  int v7; // ecx
  int v8; // eax
  WORD *v9; // esi
  int v10; // ecx
  int v11; // [esp+10h] [ebp-Ch]
  int v12; // [esp+18h] [ebp-4h]
  int iMesha; // [esp+20h] [ebp+4h]

  v4 = *(DWORD *)(This + 40);
  v5 = iTri1;
  v6 = *(DWORD *)(v4 + 40 * iMesh + 28);
  v11 = v6 + 36 * iTri1;
  if ( *(WORD *)(v11 + 2 * iIndex11 + 26) == 0xFFFF )
  {
    v7 = *(short *)(v4 + 40 * iMesh + 10);
    v8 = 0;
    v12 = v7;
    iMesha = 0;
    if ( v7 > 0 )
    {
      while ( v5 == v8 )
      {
LABEL_10:
        ++v8;
        v6 += 36;
        iMesha = v8;
        if ( v8 >= v7 )
        {
          return;
        }
      }
      v9 = (WORD *)(v6 + 2);
      v10 = 0;
      while ( v9[12] != 0xFFFF
           || *(WORD *)(v11 + 2 * (iIndex11 + 1)) != *(WORD *)(v6 + 2 * ((v10 + 1) % 3) + 2)
           || *(WORD *)(v11 + 2 * ((iIndex11 + 1) % 3) + 2) != *v9 )
      {
        ++v10;
        ++v9;
        if ( v10 >= 3 )
        {
          v8 = iMesha;
          v7 = v12;
          v5 = iTri1;
          goto LABEL_10;
        }
      }
      *(WORD *)(v11 + 2 * iIndex11 + 26) = iMesha;
      *(WORD *)(v6 + 2 * v10 + 26) = iTri1;
    }
  }
}
#endif

// ── FUN_00442e00 (IDA-only, gated) ──
#if defined(IDA_PORT_00442E00)
void __cdecl BMD::Init(DWORD This, bool Dummy)
{
  int v2; // esi
  int v3; // edx
  BYTE *v4; // eax

  if ( Dummy )
  {
    v2 = 0;
    if ( *(short *)(This + 34) > 0 )
    {
      v3 = 0;
      do
      {
        v4 = (BYTE *)(v3 + *(DWORD *)(This + 44));
        v4[34] = *v4 == 68 && v4[1] == 117;
        ++v2;
        v3 += 140;
      }
      while ( v2 < *(short *)(This + 34) );
    }
  }
  *(DWORD *)(This + 84) = -1;
  *(BYTE *)(This + 136) = -1;
  BMD::CreateBoundingBox(This);
}
#endif

// ── FUN_00444b30 (IDA-only, gated) ──
#if defined(IDA_PORT_00444B30)
void __cdecl FUN_00444b30(DWORD o)
{
  if ( *(WORD *)(o + 2) == 390 )
  {
    SetAction(o, 87);
  }
  else
  {
    SetAction(o, 5);
  }
}
#endif

// ── FUN_00445230 (IDA-only, gated) ──
#if defined(IDA_PORT_00445230)
extern "C" void DbgLogPublic(const char* msg);   // probe AEDBG (temporal)

// ─────────────────────────────────────────────────────────────────────────────
// Shims de compatibilidad para el port crudo de AttackEffect (2026-08-16).
//
// El decompile de IDA trae el ruido anti-tamper tal cual (hash-table +
// CErrorReport + operator_new/delete alrededor de CADA lectura del byte de
// skill en Owner+770). Por policy del proyecto ese ruido no se ejecuta.
//
// Camino real en nuestro build: g_HashTableCtx tiene capacity=1 con el slot 0
// como centinela (key=0), asi que el primer `memcmp(&v245 /*0*/, keys[0] /*0*/)`
// da match, rompe el while y cae directo al `LABEL_*: Ownerx = *vN;` que lee el
// byte EN CLARO — que es exactamente lo que queremos (todo el resto del port
// escribe/lee Owner+770 sin encriptar). El segundo bloque (re-encrypt) no entra
// al while por la misma razon y no toca nada.
//
// Estos shims neutralizan las llamadas para que compile sin editar los ~60
// bloques a mano (menos riesgo de romper la logica de gameplay al reescribir).
// Se hace #undef de todo justo despues del #endif de esta funcion.
// ─────────────────────────────────────────────────────────────────────────────
namespace CErrorReport { inline void Write(DWORD, const char*) {} }
static const char aHashTableFullG[] = "hash table full";
static DWORD      g_ErrorReport = 0;

// Scratch fijo: el "nodo" que crea el ruido anti-tamper nunca se lee (el insert
// es no-op), asi que devolver un buffer estatico evita el leak de operator_new.
static BYTE g_AE_HashScratch[16];
static inline DWORD AE_new(size_t)            { return (DWORD)(uintptr_t)g_AE_HashScratch; }
static inline void  AE_delete(DWORD)          {}
// Reemplaza el dispatch inline `(*(int(**)(int*,DWORD))(DAT_055c9bc8+12))(...)`.
// En nuestro build ese slot es HashFn_Sentinel (globals.cpp) y devuelve 0; el
// shim evita el deref de vtable sin cambiar el comportamiento.
static inline int AE_ht_hash(DWORD) { return 0; }
static inline unsigned int AE_ht_index(const void*, DWORD) { return 0xFFFFFFFFu; }
static inline void  AE_ht_noop2(const void*, DWORD)        {}
static inline void  AE_ht_noop3(const void*, DWORD, DWORD) {}
static inline unsigned char AE_SkillRead(const void* ctx, const void* p)
{ return FUN_0045fae0((DWORD)(uintptr_t)ctx, (unsigned char*)p); }

// Wrappers de tipo: Hex-Rays pasa enteros/DWORD donde functions.h declara
// float*/void*. Se castea aca en vez de tocar los ~50 call sites.
static inline float* AE_CreateEffect(int type, float* p1, float* p2, float* p3,
                                     int a4, int a5, int a6, int a7, int flag)
{ return FUN_00460dc0(type, p1, p2, p3, (float*)(intptr_t)a4, (float*)(intptr_t)a5,
                      (float*)(intptr_t)a6, (float*)(intptr_t)a7, (byte)flag); }
static inline float* AE_CreateEffect(int type, float* p1, float* p2, float* p3,
                                     int a4, int a5, float* a6, int a7, int flag)
{ return FUN_00460dc0(type, p1, p2, p3, (float*)(intptr_t)a4, (float*)(intptr_t)a5,
                      a6, (float*)(intptr_t)a7, (byte)flag); }
static inline void* AE_CreateJoint(int type, float* p1, float* p2, float* p3,
                                   int sub, int owner, double scale, int a, int b)
{ return FUN_0046d840(type, p1, p2, p3, (unsigned int)sub, owner, (float)scale,
                      (short)a, (unsigned char)b); }
static inline void AE_TransformPosition(DWORD model, const void* mat,
                                        float* pos, float* out, int flag)
{ FUN_004409a0((void*)(uintptr_t)model, (float*)mat, pos, out, (char)flag); }

#define Models                        DAT_05828d58
#define Matrix                        DAT_07abf444   // IDA `Matrix` @ 0x07ABF444
#define operator_new(n)               AE_new((size_t)(n))
#define delete__(p)                   AE_delete((DWORD)(uintptr_t)(p))
#define FUN_004041e0(ctx, key)        AE_ht_index((const void*)(ctx), (DWORD)(key))
#define FUN_00403f80(ctx, node, key)  AE_ht_noop3((const void*)(ctx), (DWORD)(uintptr_t)(node), (DWORD)(key))
#define FUN_00423710(node, key)       AE_ht_noop2((const void*)(uintptr_t)(node), (DWORD)(uintptr_t)(key))
#define PACKET_ENCRYPT(ctx, key)      AE_ht_noop2((const void*)(ctx), (DWORD)(uintptr_t)(key))
#define FUN_0045fae0(ctx, p)          AE_SkillRead((const void*)(ctx), (const void*)(p))
#define FUN_00466300(pos)             FUN_00466300((float*)(uintptr_t)(pos))
#define CreateJoint                   AE_CreateJoint
#define TransformPosition(a,b,c,d,e)  AE_TransformPosition((DWORD)(uintptr_t)(a), (const void*)(b), (c), (d), (e))
#define PlayBuffer(a,b,c)             FUN_00404bc0((a), (DWORD)(b), (BOOL)(c))
#undef  CreateEffect
#define CreateEffect                  AE_CreateEffect

void __cdecl FUN_00445230(int Owner)
{
  // Guard de port (no esta en IDA): si la tabla de modelos todavia no esta
  // cargada, `Models + 188*type` seria un puntero basura que TransformPosition
  // deferenciaria. La version parcial de stubs_misc2.cpp tenia el mismo guard.
  if (!Owner || !DAT_05828d58) return;

  DWORD v2; // edi
  char v3; // al
  char *v4; // ebx
  unsigned int v5; // eax
  bool v6; // cf
  int v7; // eax
  unsigned int v8; // eax
  char *v9; // esi
  unsigned char v10; // al
  char *v11; // eax
  char v12; // cl
  char v13; // cl
  unsigned int v14; // eax
  BYTE *v15; // edi
  char v16; // al
  BYTE *v17; // esi
  char v18; // al
  bool v19; // zf
  signed int v20; // eax
  double v21; // st7
  int v22; // edi
  DWORD v23; // eax
  float *v24; // ebx
  float *v25; // esi
  char v26; // al
  char *v27; // ebx
  unsigned int v28; // edx
  const char *v29; // edi
  int v30; // eax
  unsigned int v31; // eax
  char *v32; // esi
  unsigned char v33; // al
  char *v34; // eax
  char v35; // cl
  char v36; // cl
  unsigned int v37; // eax
  BYTE *v38; // edi
  char v39; // al
  BYTE *v40; // esi
  char v41; // al
  int v42; // esi
  int v43; // eax
  char v44; // al
  int v45; // esi
  float v46; // eax
  double v47; // st7
  float v48; // ecx
  float v49; // edx
  float v50; // eax
  int v51; // eax
  float v52; // eax
  float v53; // ecx
  double v54; // st7
  int v55; // edx
  float v56; // ecx
  float v57; // edx
  float v58; // eax
  double v59; // st7
  int v60; // ecx
  char *v61; // ebx
  unsigned int v62; // edx
  const char *v63; // edi
  int v64; // eax
  unsigned int v65; // eax
  char *v66; // esi
  unsigned char v67; // al
  char *v68; // eax
  char v69; // cl
  char v70; // cl
  unsigned int v71; // eax
  BYTE *v72; // edi
  char v73; // al
  BYTE *v74; // esi
  char v75; // al
  char v76; // al
  int v77; // esi
  int v78; // eax
  char v79; // al
  char *v80; // ebx
  unsigned int v81; // edx
  const char *v82; // edi
  int v83; // eax
  unsigned int v84; // eax
  char *v85; // esi
  unsigned char v86; // al
  char *v87; // eax
  char v88; // cl
  char v89; // cl
  unsigned int v90; // eax
  BYTE *v91; // edi
  char v92; // al
  BYTE *v93; // esi
  char v94; // al
  int v95; // esi
  float v96; // ecx
  double v97; // st7
  char *v98; // ebx
  unsigned int v99; // edx
  const char *v100; // edi
  int v101; // eax
  unsigned int v102; // eax
  char *v103; // esi
  unsigned char v104; // al
  char *v105; // eax
  char v106; // cl
  char v107; // cl
  unsigned int v108; // eax
  BYTE *v109; // edi
  char v110; // al
  BYTE *v111; // esi
  char v112; // al
  int v113; // ebx
  int v114; // eax
  float v115; // eax
  float v116; // ecx
  char *v117; // ebx
  int v118; // eax
  unsigned int v119; // eax
  char *v120; // esi
  unsigned char v121; // al
  char *v122; // eax
  char v123; // cl
  char v124; // cl
  unsigned int v125; // eax
  BYTE *v126; // eax
  char v127; // cl
  int v128; // esi
  float v129; // eax
  double v130; // st7
  char *v131; // ebx
  unsigned int v132; // edx
  const char *v133; // edi
  int v134; // eax
  unsigned int v135; // eax
  char *v136; // esi
  unsigned char v137; // al
  char *v138; // eax
  char v139; // cl
  char v140; // cl
  unsigned int v141; // eax
  BYTE *v142; // edi
  char v143; // al
  BYTE *v144; // esi
  char v145; // al
  double v146; // st7
  float v147; // ecx
  float v148; // edx
  float v149; // eax
  double v150; // st7
  int v151; // esi
  char *v152; // ebx
  unsigned int v153; // edx
  const char *v154; // edi
  int v155; // eax
  unsigned int v156; // eax
  char *v157; // esi
  unsigned char v158; // al
  char *v159; // eax
  char v160; // cl
  char v161; // cl
  unsigned int v162; // eax
  BYTE *v163; // edi
  char v164; // al
  BYTE *v165; // esi
  char v166; // al
  float v167; // eax
  float v168; // ecx
  double v169; // st7
  float v170; // ecx
  char *v171; // ebx
  unsigned int v172; // edx
  const char *v173; // edi
  int v174; // eax
  unsigned int v175; // eax
  char *v176; // esi
  unsigned char v177; // al
  char *v178; // eax
  char v179; // cl
  char v180; // cl
  unsigned int v181; // eax
  BYTE *v182; // edi
  char v183; // al
  BYTE *v184; // esi
  char v185; // al
  int v186; // esi
  unsigned int v187; // ebx
  int v188; // eax
  char *v189; // edi
  char v190; // bl
  double v191; // st7
  unsigned int v192; // eax
  char *v193; // esi
  unsigned char v194; // al
  char *v195; // eax
  char v196; // cl
  char v197; // cl
  double v198; // st7
  int v199; // ebx
  int v200; // eax
  int v201; // ecx
  short v202; // ax
  DWORD v203; // esi
  float v204; // ecx
  float v205; // edx
  float v206; // ecx
  float v207; // edx
  float v208; // edx
  float v209; // eax
  float v210; // ecx
  double v211; // st7
  int v212; // edx
  float *v213; // ebp
  int v214; // edi
  int k; // ebx
  int m; // ebx
  int j; // edi
  int i; // edi
  int v219; // edi
  int v220; // edi
  int v221; // edi
  int ii; // edi
  int n; // edi
  int jj; // edi
  int v225; // edi
  int v226; // eax
  float v227; // eax
  float (*v228)[4]; // eax
  float v229; // ecx
  float (*v230)[4]; // [esp-10h] [ebp-7Ch]
  float (*v231)[4]; // [esp-10h] [ebp-7Ch]
  float (*v232)[4]; // [esp-10h] [ebp-7Ch]
  DWORD This; // [esp+10h] [ebp-5Ch]
  unsigned int v234; // [esp+14h] [ebp-58h]
  unsigned int v235; // [esp+14h] [ebp-58h]
  unsigned int v236; // [esp+14h] [ebp-58h]
  unsigned int v237; // [esp+14h] [ebp-58h]
  unsigned int v238; // [esp+14h] [ebp-58h]
  unsigned int v239; // [esp+14h] [ebp-58h]
  unsigned int v240; // [esp+14h] [ebp-58h]
  unsigned int v241; // [esp+14h] [ebp-58h]
  unsigned int v242; // [esp+14h] [ebp-58h]
  DWORD v243; // [esp+18h] [ebp-54h] BYREF
  const char *v244; // [esp+1Ch] [ebp-50h] BYREF
  DWORD v245; // [esp+20h] [ebp-4Ch] BYREF
  float v246[3]; // [esp+24h] [ebp-48h] BYREF
  float TargetPosition[3]; // [esp+30h] [ebp-3Ch] BYREF
  float v248[3]; // [esp+3Ch] [ebp-30h] BYREF
  float Light[3]; // [esp+48h] [ebp-24h] BYREF
  float Angle[3]; // [esp+54h] [ebp-18h] BYREF
  float Position[3]; // [esp+60h] [ebp-Ch] BYREF
  DWORD Ownera; // [esp+70h] [ebp+4h]
  char Ownerb; // [esp+70h] [ebp+4h]
  DWORD Ownerc; // [esp+70h] [ebp+4h]
  char Ownerd; // [esp+70h] [ebp+4h]
  DWORD Ownere; // [esp+70h] [ebp+4h]
  char Ownerf; // [esp+70h] [ebp+4h]
  DWORD Ownerg; // [esp+70h] [ebp+4h]
  char Ownerh; // [esp+70h] [ebp+4h]
  int Ownery; // [esp+70h] [ebp+4h]
  signed int Owneri; // [esp+70h] [ebp+4h]
  DWORD Ownerj; // [esp+70h] [ebp+4h]
  char Ownerk; // [esp+70h] [ebp+4h]
  unsigned int Ownerl; // [esp+70h] [ebp+4h]
  char Ownerm; // [esp+70h] [ebp+4h]
  signed int Ownern; // [esp+70h] [ebp+4h]
  DWORD Ownero; // [esp+70h] [ebp+4h]
  char Ownerp; // [esp+70h] [ebp+4h]
  DWORD Ownerq; // [esp+70h] [ebp+4h]
  char Ownerr; // [esp+70h] [ebp+4h]
  DWORD Owners; // [esp+70h] [ebp+4h]
  char Ownert; // [esp+70h] [ebp+4h]
  signed int Owneru; // [esp+70h] [ebp+4h]
  DWORD Ownerv; // [esp+70h] [ebp+4h]
  float Ownerw; // [esp+70h] [ebp+4h]
  float Ownerx; // [esp+70h] [ebp+4h]

  v2 = Models + 188 * *(short *)(Owner + 2);
  This = v2;
  rand();
  v3 = *(BYTE *)(Owner + 747);
  memset(v248, 0, sizeof(v248));
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  switch ( v3 )
  {
    case 35:
      v171 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v172 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Owners = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v173 = (const char *)(DAT_055c9bd0 + 4 * v172);
          v244 = v173;
          if ( !memcmp((const char *)&v245, v173, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v172 == -1 )
            {
              break;
            }
            v175 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v175 == -1 )
            {
              v176 = 0;
            }
            else
            {
              v176 = *(char **)(DAT_055c9bcc + 4 * v175);
            }
            v177 = v176[1] + 1;
            v176[1] = v177;
            if ( v177 < 2u )
            {
              v178 = (char *)operator_new(1u);
              v179 = *v176;
              *v178 = *v176;
              v179 -= 35;
              *v178 = v179;
              v180 = (DAT_00559050[0] ^ v179) - 71;
              *v178 = v180;
              *v171 = v180;
              delete__(v178);
            }
            goto LABEL_265;
          }
          v172 = (v172 + 1) % DAT_055c9bd4;
          if ( ++Owners >= DAT_055c9bd4 )
          {
            goto LABEL_263;
          }
        }
      }
      else
      {
LABEL_263:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v174 = operator_new(2u);
      *(BYTE *)(v174 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v174, Owner + 770);
LABEL_265:
      Ownert = *v171;
      v244 = (const char *)(Owner + 770);
      v242 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v242), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v242), 4) )
          {
            if ( v242 != -1 )
            {
              v181 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v182 = v181 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v181);
              v183 = v182[1] - 1;
              v182[1] = v183;
              if ( !v183 )
              {
                v184 = (BYTE *)operator_new(1u);
                v185 = *v171 + 71;
                *v184 = v185;
                *v184 = (DAT_00559050[0] ^ v185) + 35;
                *v171 = rand();
                *v182 = *v184;
                delete__(v184);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v242 = (v242 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_269;
          }
        }
      }
      else
      {
LABEL_269:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownert == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        v186 = 0;
        Owneru = 0;
        do
        {
          v246[2] = (double)Owneru * 20.0;
          v246[0] = 0.0;
          v246[1] = 0.0;
          CreateEffect(191, (float *)(Owner + 16), v246, (float *)(Owner + 232), 1, Owner, -1, 0, 0);
          Owneru = ++v186;
        }
        while ( v186 < 18 );
        PlayBuffer(46, 0, 0);
      }
      break;
    case 38:
    case 67:
      v244 = (const char *)(Owner + 770);
      v187 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerv = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v187), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v187), 4) )
          {
            if ( v187 == -1 )
            {
              break;
            }
            v189 = (char *)(Owner + 770);
            v192 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v192 == -1 )
            {
              v193 = 0;
            }
            else
            {
              v193 = *(char **)(DAT_055c9bcc + 4 * v192);
            }
            v194 = v193[1] + 1;
            v193[1] = v194;
            if ( v194 < 2u )
            {
              v195 = (char *)operator_new(1u);
              v196 = *v193;
              *v195 = *v193;
              v196 -= 35;
              *v195 = v196;
              v197 = (DAT_00559050[0] ^ v196) - 71;
              *v195 = v197;
              *v189 = v197;
              delete__(v195);
            }
            goto LABEL_293;
          }
          v6 = ++Ownerv < DAT_055c9bd4;
          v187 = (v187 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_291;
          }
        }
      }
      else
      {
LABEL_291:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v188 = operator_new(2u);
      v189 = (char *)(Owner + 770);
      *(BYTE *)(v188 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v188, Owner + 770);
LABEL_293:
      v190 = *v189;
      PACKET_ENCRYPT(&DAT_055c9bc8, v189);
      if ( v190 == 50 )
      {
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
          CreateEffect(200, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          CreateEffect(201, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          PlayBuffer(89, 0, 0);
        }
        TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
        v191 = (double)(rand() % 1024) + *(float *)(Owner + 20);
        TargetPosition[2] = *(float *)(Owner + 24);
        TargetPosition[1] = v191 - 512.0;
        CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
        goto LABEL_75;
      }
      break;
    case 42:
      v152 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v153 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerq = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v154 = (const char *)(DAT_055c9bd0 + 4 * v153);
          v244 = v154;
          if ( !memcmp((const char *)&v245, v154, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v153 == -1 )
            {
              break;
            }
            v156 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v156 == -1 )
            {
              v157 = 0;
            }
            else
            {
              v157 = *(char **)(DAT_055c9bcc + 4 * v156);
            }
            v158 = v157[1] + 1;
            v157[1] = v158;
            if ( v158 < 2u )
            {
              v159 = (char *)operator_new(1u);
              v160 = *v157;
              *v159 = *v157;
              v160 -= 35;
              *v159 = v160;
              v161 = (DAT_00559050[0] ^ v160) - 71;
              *v159 = v161;
              *v152 = v161;
              delete__(v159);
            }
            goto LABEL_238;
          }
          v153 = (v153 + 1) % DAT_055c9bd4;
          if ( ++Ownerq >= DAT_055c9bd4 )
          {
            goto LABEL_236;
          }
        }
      }
      else
      {
LABEL_236:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v155 = operator_new(2u);
      *(BYTE *)(v155 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v155, Owner + 770);
LABEL_238:
      Ownerr = *v152;
      v244 = (const char *)(Owner + 770);
      v241 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v241), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v241), 4) )
          {
            if ( v241 != -1 )
            {
              v162 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v163 = v162 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v162);
              v164 = v163[1] - 1;
              v163[1] = v164;
              if ( !v164 )
              {
                v165 = (BYTE *)operator_new(1u);
                v166 = *v152 + 71;
                *v165 = v166;
                *v165 = (DAT_00559050[0] ^ v166) + 35;
                *v152 = rand();
                *v163 = *v165;
                delete__(v165);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v241 = (v241 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_242;
          }
        }
      }
      else
      {
LABEL_242:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerr == 50 )
      {
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
          v230 = (float (*)[4])(*(DWORD *)(Owner + 276) + 528);
          memset(v248, 0, sizeof(v248));
          TransformPosition(This, v230, v248, TargetPosition, 1);
          v167 = *(float *)(Owner + 32);
          v246[0] = *(float *)(Owner + 28) - 20.0;
          v246[2] = *(float *)(Owner + 36) - 30.0;
          v246[1] = v167;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          v168 = *(float *)(Owner + 36);
          v169 = *(float *)(Owner + 28) - 30.0;
          v246[1] = *(float *)(Owner + 32);
          v246[0] = v169;
          v246[2] = v168;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          v170 = *(float *)(Owner + 32);
          v246[0] = *(float *)(Owner + 28) - 20.0;
          v246[2] = *(float *)(Owner + 36) + 30.0;
          v246[1] = v170;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          PlayBuffer(46, 0, 0);
        }
        goto LABEL_31;
      }
      break;
    case 45:
      v199 = 4;
      do
      {
        v248[0] = (float)(rand() % 32 - 16);
        v248[1] = (float)(rand() % 32 - 16);
        v200 = rand() % 32;
        v201 = *(DWORD *)(Owner + 276);
        v248[2] = (float)(v200 - 16);
        TransformPosition(v2, (float (*)[4])(v201 + 96), v248, TargetPosition, 1);
        Particle_Spawn(1241, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
        Particle_Spawn(1206, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
        --v199;
      }
      while ( v199 );
      break;
    case 49:
      if ( *(unsigned char *)(Owner + 757) % 5 == 1 )
      {
        TransformPosition(v2, (float (*)[4])(*(DWORD *)(Owner + 276) + 3024), v248, TargetPosition, 1);
        CreateEffect(1211, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      v131 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v132 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownero = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v133 = (const char *)(DAT_055c9bd0 + 4 * v132);
          v244 = v133;
          if ( !memcmp((const char *)&v245, v133, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v132 == -1 )
            {
              break;
            }
            v135 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v135 == -1 )
            {
              v136 = 0;
            }
            else
            {
              v136 = *(char **)(DAT_055c9bcc + 4 * v135);
            }
            v137 = v136[1] + 1;
            v136[1] = v137;
            if ( v137 < 2u )
            {
              v138 = (char *)operator_new(1u);
              v139 = *v136;
              *v138 = *v136;
              v139 -= 35;
              *v138 = v139;
              v140 = (DAT_00559050[0] ^ v139) - 71;
              *v138 = v140;
              *v131 = v140;
              delete__(v138);
            }
            goto LABEL_210;
          }
          v132 = (v132 + 1) % DAT_055c9bd4;
          if ( ++Ownero >= DAT_055c9bd4 )
          {
            goto LABEL_208;
          }
        }
      }
      else
      {
LABEL_208:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v134 = operator_new(2u);
      *(BYTE *)(v134 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v134, Owner + 770);
LABEL_210:
      Ownerp = *v131;
      v244 = (const char *)(Owner + 770);
      v240 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v240), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v240), 4) )
          {
            if ( v240 != -1 )
            {
              v141 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v142 = v141 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v141);
              v143 = v142[1] - 1;
              v142[1] = v143;
              if ( !v143 )
              {
                v144 = (BYTE *)operator_new(1u);
                v145 = *v131 + 71;
                *v144 = v145;
                *v144 = (DAT_00559050[0] ^ v145) + 35;
                *v131 = rand();
                *v142 = *v144;
                delete__(v144);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v240 = (v240 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_214;
          }
        }
      }
      else
      {
LABEL_214:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerp == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        v146 = *(float *)(Owner + 36) + 20.0;
        v147 = *(float *)(Owner + 32);
        v148 = *(float *)(Owner + 16);
        v246[0] = *(float *)(Owner + 28);
        v149 = *(float *)(Owner + 20);
        v246[1] = v147;
        v246[2] = v146;
        v150 = *(float *)(Owner + 24) + 50.0;
        v248[0] = v148;
        v248[1] = v149;
        Light[0] = 0.42000002;
        Light[1] = 0.84000003;
        Light[2] = 1.4;
        v151 = 9;
        v248[2] = v150;
        do
        {
          v246[2] = v246[2] + 40.0;
          CreateEffect(1210, v248, v246, Light, 0, 0, (float *)-1, 0, 0);
          --v151;
        }
        while ( v151 );
      }
      break;
    case 53:
    case 58:
    case 59:
      if ( *(BYTE *)(Owner + 757) == 1 )
      {
        FUN_00466300(Owner + 16);
      }
      if ( *(BYTE *)(Owner + 757) == 14 && *(BYTE *)(Owner + 747) == 59 )
      {
        v117 = (char *)(Owner + 770);
        v244 = (const char *)(Owner + 770);
        Ownerl = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * Ownerl), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * Ownerl), 4) )
            {
              if ( Ownerl == -1 )
              {
                break;
              }
              v119 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              if ( v119 == -1 )
              {
                v120 = 0;
              }
              else
              {
                v120 = *(char **)(DAT_055c9bcc + 4 * v119);
              }
              v121 = v120[1] + 1;
              v120[1] = v121;
              if ( v121 < 2u )
              {
                v122 = (char *)operator_new(1u);
                v123 = *v120;
                *v122 = *v120;
                v123 -= 35;
                *v122 = v123;
                v124 = (DAT_00559050[0] ^ v123) - 71;
                *v122 = v124;
                *v117 = v124;
                delete__(v122);
              }
              goto LABEL_181;
            }
            v6 = ++v243 < DAT_055c9bd4;
            Ownerl = (Ownerl + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_179;
            }
          }
        }
        else
        {
LABEL_179:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        v118 = operator_new(2u);
        *(BYTE *)(v118 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v118, Owner + 770);
LABEL_181:
        Ownerm = *v117;
        v244 = (const char *)(Owner + 770);
        v239 = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v239), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v239), 4) )
            {
              if ( v239 != -1 )
              {
                v125 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
                v126 = v125 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v125);
                v127 = v126[1] - 1;
                v126[1] = v127;
                if ( !v127 )
                {
                  FUN_00423710(v126, (BYTE *)(Owner + 770));
                }
              }
              break;
            }
            v6 = ++v243 < DAT_055c9bd4;
            v239 = (v239 + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_185;
            }
          }
        }
        else
        {
LABEL_185:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        if ( Ownerm == 50 )
        {
          v128 = 0;
          Ownern = 0;
          do
          {
            v129 = *(float *)(Owner + 32);
            v130 = (double)Ownern * 20.0 + *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v129;
            v246[2] = v130;
            CreateEffect(568, (float *)(Owner + 16), v246, (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
            Ownern = ++v128;
          }
          while ( v128 < 18 );
        }
      }
      break;
    case 54:
    case 57:
    case -105:
      if ( *(BYTE *)(Owner + 757) == 1 )
      {
        v113 = *(DWORD *)(Owner + 276);
        v114 = *(unsigned char *)(Owner + 628);
        v248[0] = 60.0;
        v248[1] = -110.0;
        v248[2] = 0.0;
        TransformPosition(v2, (float (*)[4])(v113 + 48 * v114), v248, TargetPosition, 1);
        CreateEffect(223, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, Owner, -1, 0, 0);
        if ( *(BYTE *)(Owner + 747) == 57 )
        {
          v115 = *(float *)(Owner + 28);
          v116 = *(float *)(Owner + 32);
          Angle[2] = *(float *)(Owner + 36) + 20.0;
          Angle[0] = v115;
          Angle[1] = v116;
          CreateEffect(223, (float *)(Owner + 16), Angle, (float *)(Owner + 232), 0, Owner, -1, 0, 0);
          Angle[2] = Angle[2] - 40.0;
          CreateEffect(223, (float *)(Owner + 16), Angle, (float *)(Owner + 232), 0, Owner, -1, 0, 0);
        }
      }
      break;
    case 61:
    case 63:
      v19 = v3 == 63;
      v79 = *(BYTE *)(Owner + 757);
      if ( v19 )
      {
        if ( v79 == 1 )
        {
          FUN_00466300(Owner + 16);
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
        }
        v80 = (char *)(Owner + 770);
        v243 = Owner + 770;
        v81 = AE_ht_hash(Owner + 770);
        v245 = 0;
        Ownerg = 0;
        if ( DAT_055c9bd4 )
        {
          while ( 1 )
          {
            v82 = (const char *)(DAT_055c9bd0 + 4 * v81);
            v244 = v82;
            if ( !memcmp((const char *)&v245, v82, 4) )
            {
              break;
            }
            if ( !memcmp((const char *)&v243, v244, 4) )
            {
              if ( v81 == -1 )
              {
                break;
              }
              v84 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              if ( v84 == -1 )
              {
                v85 = 0;
              }
              else
              {
                v85 = *(char **)(DAT_055c9bcc + 4 * v84);
              }
              v86 = v85[1] + 1;
              v85[1] = v86;
              if ( v86 < 2u )
              {
                v87 = (char *)operator_new(1u);
                v88 = *v85;
                *v87 = *v85;
                v88 -= 35;
                *v87 = v88;
                v89 = (DAT_00559050[0] ^ v88) - 71;
                *v87 = v89;
                *v80 = v89;
                delete__(v87);
              }
              goto LABEL_116;
            }
            v81 = (v81 + 1) % DAT_055c9bd4;
            if ( ++Ownerg >= DAT_055c9bd4 )
            {
              goto LABEL_114;
            }
          }
        }
        else
        {
LABEL_114:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        v83 = operator_new(2u);
        *(BYTE *)(v83 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v83, Owner + 770);
LABEL_116:
        Ownerh = *v80;
        v244 = (const char *)(Owner + 770);
        v237 = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v237), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v237), 4) )
            {
              if ( v237 != -1 )
              {
                v90 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
                v91 = v90 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v90);
                v92 = v91[1] - 1;
                v91[1] = v92;
                if ( !v92 )
                {
                  v93 = (BYTE *)operator_new(1u);
                  v94 = *v80 + 71;
                  *v93 = v94;
                  *v93 = (DAT_00559050[0] ^ v94) + 35;
                  *v80 = rand();
                  *v91 = *v93;
                  delete__(v93);
                }
              }
              break;
            }
            v6 = ++v243 < DAT_055c9bd4;
            v237 = (v237 + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_120;
            }
          }
        }
        else
        {
LABEL_120:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        if ( Ownerh == 50 )
        {
          if ( *(BYTE *)(Owner + 747) == 63 )
          {
            TargetPosition[0] = (double)(rand() % 800) + *(float *)(Owner + 16) - 400.0;
            Ownery = rand() % 800;
            TargetPosition[2] = *(float *)(Owner + 24);
            TargetPosition[1] = (double)Ownery + *(float *)(Owner + 20) - 400.0;
            CreateEffect(240, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          }
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            v95 = 0;
            Owneri = 0;
            do
            {
              v96 = *(float *)(Owner + 32);
              v97 = (double)Owneri * 20.0 + *(float *)(Owner + 36);
              v246[0] = *(float *)(Owner + 28);
              v246[1] = v96;
              v246[2] = v97;
              CreateEffect(568, (float *)(Owner + 16), v246, (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
              Owneri = ++v95;
            }
            while ( v95 < 18 );
          }
        }
      }
      else if ( v79 == 1 )
      {
        CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      break;
    case 66:
      v98 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v99 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerj = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v100 = (const char *)(DAT_055c9bd0 + 4 * v99);
          v244 = v100;
          if ( !memcmp((const char *)&v245, v100, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v99 == -1 )
            {
              break;
            }
            v102 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v102 == -1 )
            {
              v103 = 0;
            }
            else
            {
              v103 = *(char **)(DAT_055c9bcc + 4 * v102);
            }
            v104 = v103[1] + 1;
            v103[1] = v104;
            if ( v104 < 2u )
            {
              v105 = (char *)operator_new(1u);
              v106 = *v103;
              *v105 = *v103;
              v106 -= 35;
              *v105 = v106;
              v107 = (DAT_00559050[0] ^ v106) - 71;
              *v105 = v107;
              *v98 = v107;
              delete__(v105);
            }
            goto LABEL_148;
          }
          v99 = (v99 + 1) % DAT_055c9bd4;
          if ( ++Ownerj >= DAT_055c9bd4 )
          {
            goto LABEL_146;
          }
        }
      }
      else
      {
LABEL_146:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v101 = operator_new(2u);
      *(BYTE *)(v101 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v101, Owner + 770);
LABEL_148:
      Ownerk = *v98;
      v244 = (const char *)(Owner + 770);
      v238 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v238), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v238), 4) )
          {
            if ( v238 != -1 )
            {
              v108 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v109 = v108 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v108);
              v110 = v109[1] - 1;
              v109[1] = v110;
              if ( !v110 )
              {
                v111 = (BYTE *)operator_new(1u);
                v112 = *v98 + 71;
                *v111 = v112;
                *v111 = (DAT_00559050[0] ^ v112) + 35;
                *v98 = rand();
                *v109 = *v111;
                delete__(v111);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v238 = (v238 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_152;
          }
        }
      }
      else
      {
LABEL_152:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerk == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        goto LABEL_30;
      }
      break;
    case 70:
      if ( *(BYTE *)(Owner + 757) == 5 && CharactersClient )  // guard de port
      {
        v22 = 20;
        v23 = CharactersClient + 916 * *(short *)(Owner + 784);
        v24 = (float *)(v23 + 28);
        v25 = (float *)(v23 + 16);
        do
        {
          CreateEffect(1271, v25, v24, Light, 0, 0, (float *)-1, 0, 0);
          --v22;
        }
        while ( v22 );
      }
      break;
    case 71:
    case 74:
      v26 = *(BYTE *)(Owner + 261);
      if ( (v26 == 3 || v26 == 4) && *(BYTE *)(Owner + 757) == 5 )
      {
        FUN_00466300(Owner + 16);
        CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      break;
    case 72:
      v27 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v28 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerc = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v29 = (const char *)(DAT_055c9bd0 + 4 * v28);
          v244 = v29;
          if ( !memcmp((const char *)&v245, v29, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v28 == -1 )
            {
              break;
            }
            v31 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v31 == -1 )
            {
              v32 = 0;
            }
            else
            {
              v32 = *(char **)(DAT_055c9bcc + 4 * v31);
            }
            v33 = v32[1] + 1;
            v32[1] = v33;
            if ( v33 < 2u )
            {
              v34 = (char *)operator_new(1u);
              v35 = *v32;
              *v34 = *v32;
              v35 -= 35;
              *v34 = v35;
              v36 = (DAT_00559050[0] ^ v35) - 71;
              *v34 = v36;
              *v27 = v36;
              delete__(v34);
            }
            goto LABEL_46;
          }
          v28 = (v28 + 1) % DAT_055c9bd4;
          if ( ++Ownerc >= DAT_055c9bd4 )
          {
            goto LABEL_44;
          }
        }
      }
      else
      {
LABEL_44:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v30 = operator_new(2u);
      *(BYTE *)(v30 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v30, Owner + 770);
LABEL_46:
      Ownerd = *v27;
      v244 = (const char *)(Owner + 770);
      v235 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v235), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v235), 4) )
          {
            if ( v235 != -1 )
            {
              v37 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v38 = v37 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v37);
              v39 = v38[1] - 1;
              v38[1] = v39;
              if ( !v39 )
              {
                v40 = (BYTE *)operator_new(1u);
                v41 = *v27 + 71;
                *v40 = v41;
                *v40 = (DAT_00559050[0] ^ v41) + 35;
                *v27 = rand();
                *v38 = *v40;
                delete__(v40);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v235 = (v235 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_50;
          }
        }
      }
      else
      {
LABEL_50:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerd == 50 && *(BYTE *)(Owner + 757) == 14 )
      {
        memset(Angle, 0, sizeof(Angle));
        v42 = 36;
        do
        {
          Angle[0] = (float)(rand() % 360);
          Angle[1] = (float)(rand() % 360);
          v43 = rand();
          Position[1] = *(float *)(Owner + 20);
          Position[0] = *(float *)(Owner + 16);
          Angle[2] = (float)(v43 % 360);
          Position[2] = *(float *)(Owner + 24) + 100.0;
          CreateJoint(1253, Position, Position, Angle, 1, 0, 60.0, 0, 0);
          --v42;
        }
        while ( v42 );
      }
      break;
    case 73:
    case 75:
      v44 = *(BYTE *)(Owner + 757);
      if ( *(BYTE *)(Owner + 261) == 3 )
      {
        if ( v44 == 11 )
        {
          FUN_00466300(Owner + 16);
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          v45 = 5;
          do
          {
            v46 = *(float *)(Owner + 36);
            v47 = *(float *)(Owner + 28) + 45.0;
            v48 = *(float *)(Owner + 16);
            v246[1] = *(float *)(Owner + 32);
            v49 = *(float *)(Owner + 20);
            v246[2] = v46;
            v50 = *(float *)(Owner + 24);
            Light[0] = 1.0;
            v246[0] = v47;
            Light[1] = 0.5;
            Light[2] = 0.0;
            TargetPosition[0] = v48;
            TargetPosition[1] = v49;
            TargetPosition[2] = v50;
            TargetPosition[0] = (double)(rand() % 1001 - 500) + v48;
            v51 = rand();
            *(float *)(Owner + 368) = TargetPosition[0];
            TargetPosition[1] = (double)(v51 % 1001 - 500) + TargetPosition[1];
            TargetPosition[2] = TargetPosition[2] + 500.0;
            v52 = TargetPosition[2];
            *(float *)(Owner + 372) = TargetPosition[1];
            *(float *)(Owner + 376) = v52;
            CreateEffect(256, TargetPosition, v246, Light, 1, Owner, -1, 0, 0);
            --v45;
          }
          while ( v45 );
        }
      }
      else
      {
        if ( v44 == 13 )
        {
          v53 = *(float *)(Owner + 32);
          v54 = *(float *)(Owner + 28) + 45.0;
          v246[2] = *(float *)(Owner + 36);
          v55 = *(DWORD *)(Owner + 276);
          v246[1] = v53;
          v246[0] = v54;
          Light[0] = 1.0;
          Light[1] = 0.5;
          Light[2] = 0.0;
          v248[0] = -50.0;
          v248[1] = 100.0;
          v248[2] = 0.0;
          TransformPosition(v2, (float (*)[4])(v55 + 528), v248, TargetPosition, 1);
          v56 = TargetPosition[1];
          v57 = TargetPosition[2];
          *(float *)(Owner + 368) = TargetPosition[0];
          *(float *)(Owner + 372) = v56;
          *(float *)(Owner + 376) = v57;
          CreateEffect(256, TargetPosition, v246, Light, 1, Owner, -1, 0, 0);
          goto LABEL_75;
        }
        if ( v44 == 9 )
        {
          v58 = *(float *)(Owner + 32);
          v59 = *(float *)(Owner + 28) + 45.0;
          v246[2] = *(float *)(Owner + 36);
          v60 = *(DWORD *)(Owner + 276);
          v246[1] = v58;
          v246[0] = v59;
          Light[0] = 1.0;
          Light[1] = 0.5;
          Light[2] = 0.0;
          memset(v248, 0, sizeof(v248));
          TransformPosition(v2, (float (*)[4])(v60 + 528), v248, TargetPosition, 1);
        }
      }
      break;
    case 77:
      v61 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v62 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownere = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v63 = (const char *)(DAT_055c9bd0 + 4 * v62);
          v244 = v63;
          if ( !memcmp((const char *)&v245, v63, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v62 == -1 )
            {
              break;
            }
            v65 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v65 == -1 )
            {
              v66 = 0;
            }
            else
            {
              v66 = *(char **)(DAT_055c9bcc + 4 * v65);
            }
            v67 = v66[1] + 1;
            v66[1] = v67;
            if ( v67 < 2u )
            {
              v68 = (char *)operator_new(1u);
              v69 = *v66;
              *v68 = *v66;
              v69 -= 35;
              *v68 = v69;
              v70 = (DAT_00559050[0] ^ v69) - 71;
              *v68 = v70;
              *v61 = v70;
              delete__(v68);
            }
            goto LABEL_84;
          }
          v62 = (v62 + 1) % DAT_055c9bd4;
          if ( ++Ownere >= DAT_055c9bd4 )
          {
            goto LABEL_82;
          }
        }
      }
      else
      {
LABEL_82:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v64 = operator_new(2u);
      *(BYTE *)(v64 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v64, Owner + 770);
LABEL_84:
      Ownerf = *v61;
      v244 = (const char *)(Owner + 770);
      v236 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v236), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v236), 4) )
          {
            if ( v236 != -1 )
            {
              v71 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v72 = v71 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v71);
              v73 = v72[1] - 1;
              v72[1] = v73;
              if ( !v73 )
              {
                v74 = (BYTE *)operator_new(1u);
                v75 = *v61 + 71;
                *v74 = v75;
                *v74 = (DAT_00559050[0] ^ v75) + 35;
                *v61 = rand();
                *v72 = *v74;
                delete__(v74);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v236 = (v236 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_88;
          }
        }
      }
      else
      {
LABEL_88:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerf == 50 )
      {
        v76 = *(BYTE *)(Owner + 757);
        if ( v76 == 2 || v76 == 6 )
        {
          memset(Angle, 0, sizeof(Angle));
          v77 = 40;
          do
          {
            Angle[0] = (float)(rand() % 360);
            Angle[1] = (float)(rand() % 360);
            v78 = rand();
            Position[1] = *(float *)(Owner + 20);
            Position[0] = *(float *)(Owner + 16);
            Angle[2] = (float)(v78 % 360);
            Position[2] = *(float *)(Owner + 24) + 100.0;
            CreateJoint(1253, Position, Position, Angle, 3, 0, 50.0, 0, 0);
            --v77;
          }
          while ( v77 );
        }
      }
      break;
    case 89:
    case 95:
    case 112:
    case 118:
    case 124:
    case -126:
    case -120:
      v4 = (char *)(Owner + 770);
      v244 = (const char *)(Owner + 770);
      v5 = AE_ht_hash(Owner + 770);
      v243 = 0;
      Ownera = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v243, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
          {
            if ( v5 == -1 )
            {
              break;
            }
            v8 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v8 == -1 )
            {
              v9 = 0;
            }
            else
            {
              v9 = *(char **)(DAT_055c9bcc + 4 * v8);
            }
            v10 = v9[1] + 1;
            v9[1] = v10;
            if ( v10 < 2u )
            {
              v11 = (char *)operator_new(1u);
              v12 = *v9;
              *v11 = *v9;
              v12 -= 35;
              *v11 = v12;
              v13 = (DAT_00559050[0] ^ v12) - 71;
              *v11 = v13;
              *v4 = v13;
              delete__(v11);
            }
            goto LABEL_8;
          }
          v6 = ++Ownera < DAT_055c9bd4;
          v5 = (v5 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_6;
          }
        }
      }
      else
      {
LABEL_6:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v7 = operator_new(2u);
      *(BYTE *)(v7 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v7, Owner + 770);
LABEL_8:
      Ownerb = *v4;
      v245 = Owner + 770;
      v234 = AE_ht_hash(Owner + 770);
      v244 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v234), 4) )
        {
          if ( !memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v234), 4) )
          {
            if ( v234 != -1 )
            {
              v14 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v15 = v14 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v14);
              v16 = v15[1] - 1;
              v15[1] = v16;
              if ( !v16 )
              {
                v17 = (BYTE *)operator_new(1u);
                v18 = *v4 + 71;
                *v17 = v18;
                *v17 = (DAT_00559050[0] ^ v18) + 35;
                *v4 = rand();
                *v15 = *v17;
                delete__(v17);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v234 = (v234 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_12;
          }
        }
      }
      else
      {
LABEL_12:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerb == 50 )
      {
        v20 = rand() & 0x80000001;
        v19 = v20 == 0;
        if ( v20 < 0 )
        {
          v19 = (((BYTE)v20 - 1) | 0xFFFFFFFE) == -1;
        }
        if ( !v19 )
        {
LABEL_31:
          TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
          v21 = (double)(rand() % 1024) + *(float *)(Owner + 20);
          TargetPosition[2] = *(float *)(Owner + 24);
          TargetPosition[1] = v21 - 512.0;
          CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          goto LABEL_75;
        }
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
LABEL_30:
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 1, 0, -1, 0, 0);
        }
      }
      break;
    case 103:
      if ( FUN_0045fae0(&DAT_055c9bc8, (BYTE *)(Owner + 770)) == 50 )
      {
        TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
        v198 = (double)(rand() % 1024) + *(float *)(Owner + 20);
        TargetPosition[2] = *(float *)(Owner + 24);
        TargetPosition[1] = v198 - 512.0;
        CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
LABEL_75:
        PlayBuffer(46, 0, 0);
      }
      break;
    default:
      break;
  }
  v202 = *(WORD *)(Owner + 784);
  // `&& CharactersClient` no esta en IDA: en nuestro build el pool arranca en 0
  // hasta que se aloca, y `0 + 916*idx` seria un puntero basura. Mismo guard que
  // Render_Frame.cpp:395.
  if ( v202 >= 0 && v202 < 400 && CharactersClient )
  {
    v203 = CharactersClient + 916 * v202;
    if ( FUN_0045fae0(&DAT_055c9bc8, (BYTE *)(Owner + 770)) == 17 )
    {
      switch ( *(BYTE *)(Owner + 747) )
      {
        case 0x25:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( i = 0; i < 4; ++i )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (i >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x2E:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( j = 0; j < 4; ++j )
          {
            memset(v246, 0, sizeof(v246));
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (j >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            CreateJoint(1166, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            CreateJoint(1166, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 10.0, -1, 0);
          }
          break;
        case 0x3D:
          for ( k = 0; k < 6; ++k )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (k >= 3) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 10.0, -1, 0);
          }
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( m = 0; m < 4; ++m )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (m >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x42:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(60, 0, 0);
          }
          v219 = 0;
          Ownerw = 45.0 - (double)((int)((__int64)WorldTime / 10 + 3 * *(unsigned char *)(Owner + 757)) % 90) + 180.0;
          do
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (v219 % 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = Ownerw;
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
            ++v219;
            Ownerw = Ownerw + 270.0;
          }
          while ( v219 < 4 );
          break;
        case 0x45:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            v213 = (float *)(Owner + 16);
            v214 = 4;
            do
            {
              CreateJoint(1249, v213, v213, v246, 7, v203, 50.0, -1, 0);
              CreateJoint(1249, TargetPosition, TargetPosition, v246, 7, v203, 50.0, -1, 0);
              --v214;
            }
            while ( v214 );
          }
          break;
        case 0x49:
        case 0x4B:
          if ( *(BYTE *)(Owner + 261) == 4 && *(BYTE *)(Owner + 757) == 13 )
          {
            v210 = *(float *)(Owner + 32);
            v211 = *(float *)(Owner + 28) + 45.0;
            v246[2] = *(float *)(Owner + 36);
            v212 = *(DWORD *)(Owner + 276);
            v246[1] = v210;
            v246[0] = v211;
            Light[0] = 1.0;
            Light[1] = 0.5;
            Light[2] = 0.0;
            v248[0] = -50.0;
            v248[1] = 100.0;
            v248[2] = 0.0;
            TransformPosition(This, (float (*)[4])(v212 + 528), v248, TargetPosition, 1);
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
          }
          break;
        case 0x4D:
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, (float (*)[4])Matrix, v248, TargetPosition, 1);
            v208 = *(float *)(Owner + 32);
            v209 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v208;
            v246[2] = v209;
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, TargetPosition, v246, 2, v203, 50.0, -1, 0);
          }
          break;
        case 0x57:
        case 0x5D:
        case 0x63:
        case 0x74:
        case 0x7A:
        case 0x80:
        case 0x86:
          if ( *(BYTE *)(Owner + 757) == 13 )
          {
            v232 = (float (*)[4])(*(DWORD *)(Owner + 276) + 288);
            Light[0] = 1.0;
            Light[1] = 1.0;
            Light[2] = 1.0;
            v248[0] = 60.0;
            v248[1] = 30.0;
            v248[2] = 0.0;
            TransformPosition(This, v232, v248, TargetPosition, 1);
            v206 = *(float *)(Owner + 32);
            v207 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v206;
            v246[2] = v207;
            CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 5, 0, -1, 0, 0);
          }
          break;
        case 0x59:
        case 0x5F:
        case 0x70:
        case 0x76:
        case 0x7C:
        case 0x82:
        case 0x88:
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            v231 = (float (*)[4])(*(DWORD *)(Owner + 276) + 1584);
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, v231, v248, TargetPosition, 1);
            v204 = *(float *)(Owner + 32);
            v205 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v204;
            v246[2] = v205;
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, TargetPosition, v246, 2, v203, 50.0, -1, 0);
          }
          break;
        default:
          return;
      }
    }
    else if ( FUN_0045fae0(&DAT_055c9bc8, (BYTE *)(Owner + 770)) == 3 )
    {
      switch ( *(BYTE *)(Owner + 747) )
      {
        case 0x22:
          for ( n = 0; n < 4; ++n )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (n >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
            Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          }
          break;
        case 0x25:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( ii = 0; ii < 4; ++ii )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (ii >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x27:
          v19 = *(WORD *)(Owner + 2) == 390;
          v248[0] = 0.0;
          v248[1] = 0.0;
          if ( !v19 )
          {
            v248[1] = -130.0;
          }
          v225 = *(DWORD *)(Owner + 276);
          v226 = *(unsigned char *)(Owner + 628);
          v248[2] = 0.0;
          TransformPosition(This, (float (*)[4])(v225 + 48 * v226), v248, TargetPosition, 1);
          v227 = *(float *)(Owner + 36);
          v246[0] = -60.0;
          v246[1] = 0.0;
          v246[2] = v227;
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
          Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          break;
        case 0x30:
          for ( jj = 0; jj < 6; ++jj )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (jj >= 3) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 10.0, -1, 0);
          }
          break;
        case 0x4D:
          if ( *(BYTE *)(Owner + 757) >= 8u )
          {
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, (float (*)[4])DAT_07abf3e4, v248, TargetPosition, 1);
            v221 = 4;
            do
            {
              v246[0] = 0.0;
              v246[1] = 0.0;
              v246[2] = (float)(rand() % 360);
              CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 80.0, -1, 0);
              --v221;
            }
            while ( v221 );
          }
          break;
        case 0x59:
        case 0x5F:
        case 0x70:
        case 0x76:
        case 0x7C:
        case 0x82:
        case 0x88:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(60, 0, 0);
          }
          v220 = 0;
          Ownerx = 45.0 - (double)((int)((__int64)WorldTime / 10 + 3 * *(unsigned char *)(Owner + 757)) % 90) + 180.0;
          do
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (v220 % 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = Ownerx;
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
            ++v220;
            Ownerx = Ownerx + 270.0;
          }
          while ( v220 < 4 );
          break;
        default:
          v19 = *(WORD *)(Owner + 2) == 390;
          v248[0] = 0.0;
          v248[1] = 0.0;
          if ( !v19 )
          {
            v248[1] = -130.0;
          }
          v228 = (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 628));
          v248[2] = 0.0;
          TransformPosition(This, v228, v248, TargetPosition, 1);
          v229 = *(float *)(Owner + 36);
          v246[0] = -60.0;
          v246[1] = 0.0;
          v246[2] = v229;
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
          Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          break;
      }
    }
  }
}

// Fin de los shims de AttackEffect — restaurar el estado global de macros.
#undef Models
#undef Matrix
#undef operator_new
#undef delete__
#undef FUN_004041e0
#undef FUN_00403f80
#undef FUN_00423710
#undef PACKET_ENCRYPT
#undef FUN_0045fae0
#undef FUN_00466300
#undef CreateJoint
#undef TransformPosition
#undef PlayBuffer
#undef CreateEffect
#define CreateEffect FUN_00460dc0
#endif

// ── FUN_004454fc (IDA-only, gated) ──
#if defined(IDA_PORT_004454FC)
void __cdecl FUN_004454fc(DWORD Owner)
{
  DWORD v2; // edi
  char v3; // al
  char *v4; // ebx
  unsigned int v5; // eax
  bool v6; // cf
  int v7; // eax
  unsigned int v8; // eax
  char *v9; // esi
  unsigned char v10; // al
  char *v11; // eax
  char v12; // cl
  char v13; // cl
  unsigned int v14; // eax
  BYTE *v15; // edi
  char v16; // al
  BYTE *v17; // esi
  char v18; // al
  bool v19; // zf
  signed int v20; // eax
  double v21; // st7
  int v22; // edi
  DWORD v23; // eax
  float *v24; // ebx
  float *v25; // esi
  char v26; // al
  char *v27; // ebx
  unsigned int v28; // edx
  const char *v29; // edi
  int v30; // eax
  unsigned int v31; // eax
  char *v32; // esi
  unsigned char v33; // al
  char *v34; // eax
  char v35; // cl
  char v36; // cl
  unsigned int v37; // eax
  BYTE *v38; // edi
  char v39; // al
  BYTE *v40; // esi
  char v41; // al
  int v42; // esi
  int v43; // eax
  char v44; // al
  int v45; // esi
  float v46; // eax
  double v47; // st7
  float v48; // ecx
  float v49; // edx
  float v50; // eax
  int v51; // eax
  float v52; // eax
  float v53; // ecx
  double v54; // st7
  int v55; // edx
  float v56; // ecx
  float v57; // edx
  float v58; // eax
  double v59; // st7
  int v60; // ecx
  char *v61; // ebx
  unsigned int v62; // edx
  const char *v63; // edi
  int v64; // eax
  unsigned int v65; // eax
  char *v66; // esi
  unsigned char v67; // al
  char *v68; // eax
  char v69; // cl
  char v70; // cl
  unsigned int v71; // eax
  BYTE *v72; // edi
  char v73; // al
  BYTE *v74; // esi
  char v75; // al
  char v76; // al
  int v77; // esi
  int v78; // eax
  char v79; // al
  char *v80; // ebx
  unsigned int v81; // edx
  const char *v82; // edi
  int v83; // eax
  unsigned int v84; // eax
  char *v85; // esi
  unsigned char v86; // al
  char *v87; // eax
  char v88; // cl
  char v89; // cl
  unsigned int v90; // eax
  BYTE *v91; // edi
  char v92; // al
  BYTE *v93; // esi
  char v94; // al
  int v95; // esi
  float v96; // ecx
  double v97; // st7
  char *v98; // ebx
  unsigned int v99; // edx
  const char *v100; // edi
  int v101; // eax
  unsigned int v102; // eax
  char *v103; // esi
  unsigned char v104; // al
  char *v105; // eax
  char v106; // cl
  char v107; // cl
  unsigned int v108; // eax
  BYTE *v109; // edi
  char v110; // al
  BYTE *v111; // esi
  char v112; // al
  int v113; // ebx
  int v114; // eax
  float v115; // eax
  float v116; // ecx
  char *v117; // ebx
  int v118; // eax
  unsigned int v119; // eax
  char *v120; // esi
  unsigned char v121; // al
  char *v122; // eax
  char v123; // cl
  char v124; // cl
  unsigned int v125; // eax
  BYTE *v126; // eax
  char v127; // cl
  int v128; // esi
  float v129; // eax
  double v130; // st7
  char *v131; // ebx
  unsigned int v132; // edx
  const char *v133; // edi
  int v134; // eax
  unsigned int v135; // eax
  char *v136; // esi
  unsigned char v137; // al
  char *v138; // eax
  char v139; // cl
  char v140; // cl
  unsigned int v141; // eax
  BYTE *v142; // edi
  char v143; // al
  BYTE *v144; // esi
  char v145; // al
  double v146; // st7
  float v147; // ecx
  float v148; // edx
  float v149; // eax
  double v150; // st7
  int v151; // esi
  char *v152; // ebx
  unsigned int v153; // edx
  const char *v154; // edi
  int v155; // eax
  unsigned int v156; // eax
  char *v157; // esi
  unsigned char v158; // al
  char *v159; // eax
  char v160; // cl
  char v161; // cl
  unsigned int v162; // eax
  BYTE *v163; // edi
  char v164; // al
  BYTE *v165; // esi
  char v166; // al
  float v167; // eax
  float v168; // ecx
  double v169; // st7
  float v170; // ecx
  char *v171; // ebx
  unsigned int v172; // edx
  const char *v173; // edi
  int v174; // eax
  unsigned int v175; // eax
  char *v176; // esi
  unsigned char v177; // al
  char *v178; // eax
  char v179; // cl
  char v180; // cl
  unsigned int v181; // eax
  BYTE *v182; // edi
  char v183; // al
  BYTE *v184; // esi
  char v185; // al
  int v186; // esi
  unsigned int v187; // ebx
  int v188; // eax
  char *v189; // edi
  char v190; // bl
  double v191; // st7
  unsigned int v192; // eax
  char *v193; // esi
  unsigned char v194; // al
  char *v195; // eax
  char v196; // cl
  char v197; // cl
  double v198; // st7
  int v199; // ebx
  int v200; // eax
  int v201; // ecx
  short v202; // ax
  DWORD v203; // esi
  float v204; // ecx
  float v205; // edx
  float v206; // ecx
  float v207; // edx
  float v208; // edx
  float v209; // eax
  float v210; // ecx
  double v211; // st7
  int v212; // edx
  float *v213; // ebp
  int v214; // edi
  int k; // ebx
  int m; // ebx
  int j; // edi
  int i; // edi
  int v219; // edi
  int v220; // edi
  int v221; // edi
  int ii; // edi
  int n; // edi
  int jj; // edi
  int v225; // edi
  int v226; // eax
  float v227; // eax
  float (*v228)[4]; // eax
  float v229; // ecx
  float (*v230)[4]; // [esp-10h] [ebp-7Ch]
  float (*v231)[4]; // [esp-10h] [ebp-7Ch]
  float (*v232)[4]; // [esp-10h] [ebp-7Ch]
  DWORD This; // [esp+10h] [ebp-5Ch]
  unsigned int v234; // [esp+14h] [ebp-58h]
  unsigned int v235; // [esp+14h] [ebp-58h]
  unsigned int v236; // [esp+14h] [ebp-58h]
  unsigned int v237; // [esp+14h] [ebp-58h]
  unsigned int v238; // [esp+14h] [ebp-58h]
  unsigned int v239; // [esp+14h] [ebp-58h]
  unsigned int v240; // [esp+14h] [ebp-58h]
  unsigned int v241; // [esp+14h] [ebp-58h]
  unsigned int v242; // [esp+14h] [ebp-58h]
  DWORD v243; // [esp+18h] [ebp-54h] BYREF
  const char *v244; // [esp+1Ch] [ebp-50h] BYREF
  DWORD v245; // [esp+20h] [ebp-4Ch] BYREF
  float v246[3]; // [esp+24h] [ebp-48h] BYREF
  float TargetPosition[3]; // [esp+30h] [ebp-3Ch] BYREF
  float v248[3]; // [esp+3Ch] [ebp-30h] BYREF
  float Light[3]; // [esp+48h] [ebp-24h] BYREF
  float Angle[3]; // [esp+54h] [ebp-18h] BYREF
  float Position[3]; // [esp+60h] [ebp-Ch] BYREF
  DWORD Ownera; // [esp+70h] [ebp+4h]
  char Ownerb; // [esp+70h] [ebp+4h]
  DWORD Ownerc; // [esp+70h] [ebp+4h]
  char Ownerd; // [esp+70h] [ebp+4h]
  DWORD Ownere; // [esp+70h] [ebp+4h]
  char Ownerf; // [esp+70h] [ebp+4h]
  DWORD Ownerg; // [esp+70h] [ebp+4h]
  char Ownerh; // [esp+70h] [ebp+4h]
  int Ownery; // [esp+70h] [ebp+4h]
  signed int Owneri; // [esp+70h] [ebp+4h]
  DWORD Ownerj; // [esp+70h] [ebp+4h]
  char Ownerk; // [esp+70h] [ebp+4h]
  unsigned int Ownerl; // [esp+70h] [ebp+4h]
  char Ownerm; // [esp+70h] [ebp+4h]
  signed int Ownern; // [esp+70h] [ebp+4h]
  DWORD Ownero; // [esp+70h] [ebp+4h]
  char Ownerp; // [esp+70h] [ebp+4h]
  DWORD Ownerq; // [esp+70h] [ebp+4h]
  char Ownerr; // [esp+70h] [ebp+4h]
  DWORD Owners; // [esp+70h] [ebp+4h]
  char Ownert; // [esp+70h] [ebp+4h]
  signed int Owneru; // [esp+70h] [ebp+4h]
  DWORD Ownerv; // [esp+70h] [ebp+4h]
  float Ownerw; // [esp+70h] [ebp+4h]
  float Ownerx; // [esp+70h] [ebp+4h]

  v2 = Models + 188 * *(short *)(Owner + 2);
  This = v2;
  rand();
  v3 = *(BYTE *)(Owner + 747);
  memset(v248, 0, sizeof(v248));
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  switch ( v3 )
  {
    case 35:
      v171 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v172 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Owners = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v173 = (const char *)(DAT_055c9bd0 + 4 * v172);
          v244 = v173;
          if ( !memcmp((const char *)&v245, v173, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v172 == -1 )
            {
              break;
            }
            v175 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v175 == -1 )
            {
              v176 = 0;
            }
            else
            {
              v176 = *(char **)(DAT_055c9bcc + 4 * v175);
            }
            v177 = v176[1] + 1;
            v176[1] = v177;
            if ( v177 < 2u )
            {
              v178 = (char *)operator_new(1u);
              v179 = *v176;
              *v178 = *v176;
              v179 -= 35;
              *v178 = v179;
              v180 = (DAT_00559050[0] ^ v179) - 71;
              *v178 = v180;
              *v171 = v180;
              delete__(v178);
            }
            goto LABEL_265;
          }
          v172 = (v172 + 1) % DAT_055c9bd4;
          if ( ++Owners >= DAT_055c9bd4 )
          {
            goto LABEL_263;
          }
        }
      }
      else
      {
LABEL_263:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v174 = operator_new(2u);
      *(BYTE *)(v174 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v174, Owner + 770);
LABEL_265:
      Ownert = *v171;
      v244 = (const char *)(Owner + 770);
      v242 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v242), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v242), 4) )
          {
            if ( v242 != -1 )
            {
              v181 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v182 = v181 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v181);
              v183 = v182[1] - 1;
              v182[1] = v183;
              if ( !v183 )
              {
                v184 = (BYTE *)operator_new(1u);
                v185 = *v171 + 71;
                *v184 = v185;
                *v184 = (DAT_00559050[0] ^ v185) + 35;
                *v171 = rand();
                *v182 = *v184;
                delete__(v184);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v242 = (v242 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_269;
          }
        }
      }
      else
      {
LABEL_269:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownert == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        v186 = 0;
        Owneru = 0;
        do
        {
          v246[2] = (double)Owneru * 20.0;
          v246[0] = 0.0;
          v246[1] = 0.0;
          CreateEffect(191, (float *)(Owner + 16), v246, (float *)(Owner + 232), 1, Owner, -1, 0, 0);
          Owneru = ++v186;
        }
        while ( v186 < 18 );
        PlayBuffer(46, 0, 0);
      }
      break;
    case 38:
    case 67:
      v244 = (const char *)(Owner + 770);
      v187 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerv = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v187), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v187), 4) )
          {
            if ( v187 == -1 )
            {
              break;
            }
            v189 = (char *)(Owner + 770);
            v192 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v192 == -1 )
            {
              v193 = 0;
            }
            else
            {
              v193 = *(char **)(DAT_055c9bcc + 4 * v192);
            }
            v194 = v193[1] + 1;
            v193[1] = v194;
            if ( v194 < 2u )
            {
              v195 = (char *)operator_new(1u);
              v196 = *v193;
              *v195 = *v193;
              v196 -= 35;
              *v195 = v196;
              v197 = (DAT_00559050[0] ^ v196) - 71;
              *v195 = v197;
              *v189 = v197;
              delete__(v195);
            }
            goto LABEL_293;
          }
          v6 = ++Ownerv < DAT_055c9bd4;
          v187 = (v187 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_291;
          }
        }
      }
      else
      {
LABEL_291:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v188 = operator_new(2u);
      v189 = (char *)(Owner + 770);
      *(BYTE *)(v188 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v188, Owner + 770);
LABEL_293:
      v190 = *v189;
      PACKET_ENCRYPT(&DAT_055c9bc8, v189);
      if ( v190 == 50 )
      {
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
          CreateEffect(200, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          CreateEffect(201, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          PlayBuffer(89, 0, 0);
        }
        TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
        v191 = (double)(rand() % 1024) + *(float *)(Owner + 20);
        TargetPosition[2] = *(float *)(Owner + 24);
        TargetPosition[1] = v191 - 512.0;
        CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
        goto LABEL_75;
      }
      break;
    case 42:
      v152 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v153 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerq = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v154 = (const char *)(DAT_055c9bd0 + 4 * v153);
          v244 = v154;
          if ( !memcmp((const char *)&v245, v154, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v153 == -1 )
            {
              break;
            }
            v156 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v156 == -1 )
            {
              v157 = 0;
            }
            else
            {
              v157 = *(char **)(DAT_055c9bcc + 4 * v156);
            }
            v158 = v157[1] + 1;
            v157[1] = v158;
            if ( v158 < 2u )
            {
              v159 = (char *)operator_new(1u);
              v160 = *v157;
              *v159 = *v157;
              v160 -= 35;
              *v159 = v160;
              v161 = (DAT_00559050[0] ^ v160) - 71;
              *v159 = v161;
              *v152 = v161;
              delete__(v159);
            }
            goto LABEL_238;
          }
          v153 = (v153 + 1) % DAT_055c9bd4;
          if ( ++Ownerq >= DAT_055c9bd4 )
          {
            goto LABEL_236;
          }
        }
      }
      else
      {
LABEL_236:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v155 = operator_new(2u);
      *(BYTE *)(v155 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v155, Owner + 770);
LABEL_238:
      Ownerr = *v152;
      v244 = (const char *)(Owner + 770);
      v241 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v241), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v241), 4) )
          {
            if ( v241 != -1 )
            {
              v162 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v163 = v162 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v162);
              v164 = v163[1] - 1;
              v163[1] = v164;
              if ( !v164 )
              {
                v165 = (BYTE *)operator_new(1u);
                v166 = *v152 + 71;
                *v165 = v166;
                *v165 = (DAT_00559050[0] ^ v166) + 35;
                *v152 = rand();
                *v163 = *v165;
                delete__(v165);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v241 = (v241 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_242;
          }
        }
      }
      else
      {
LABEL_242:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerr == 50 )
      {
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
          v230 = (float (*)[4])(*(DWORD *)(Owner + 276) + 528);
          memset(v248, 0, sizeof(v248));
          TransformPosition(This, v230, v248, TargetPosition, 1);
          v167 = *(float *)(Owner + 32);
          v246[0] = *(float *)(Owner + 28) - 20.0;
          v246[2] = *(float *)(Owner + 36) - 30.0;
          v246[1] = v167;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          v168 = *(float *)(Owner + 36);
          v169 = *(float *)(Owner + 28) - 30.0;
          v246[1] = *(float *)(Owner + 32);
          v246[0] = v169;
          v246[2] = v168;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          v170 = *(float *)(Owner + 32);
          v246[0] = *(float *)(Owner + 28) - 20.0;
          v246[2] = *(float *)(Owner + 36) + 30.0;
          v246[1] = v170;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          PlayBuffer(46, 0, 0);
        }
        goto LABEL_31;
      }
      break;
    case 45:
      v199 = 4;
      do
      {
        v248[0] = (float)(rand() % 32 - 16);
        v248[1] = (float)(rand() % 32 - 16);
        v200 = rand() % 32;
        v201 = *(DWORD *)(Owner + 276);
        v248[2] = (float)(v200 - 16);
        TransformPosition(v2, (float (*)[4])(v201 + 96), v248, TargetPosition, 1);
        Particle_Spawn(1241, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
        Particle_Spawn(1206, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
        --v199;
      }
      while ( v199 );
      break;
    case 49:
      if ( *(unsigned char *)(Owner + 757) % 5 == 1 )
      {
        TransformPosition(v2, (float (*)[4])(*(DWORD *)(Owner + 276) + 3024), v248, TargetPosition, 1);
        CreateEffect(1211, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      v131 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v132 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownero = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v133 = (const char *)(DAT_055c9bd0 + 4 * v132);
          v244 = v133;
          if ( !memcmp((const char *)&v245, v133, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v132 == -1 )
            {
              break;
            }
            v135 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v135 == -1 )
            {
              v136 = 0;
            }
            else
            {
              v136 = *(char **)(DAT_055c9bcc + 4 * v135);
            }
            v137 = v136[1] + 1;
            v136[1] = v137;
            if ( v137 < 2u )
            {
              v138 = (char *)operator_new(1u);
              v139 = *v136;
              *v138 = *v136;
              v139 -= 35;
              *v138 = v139;
              v140 = (DAT_00559050[0] ^ v139) - 71;
              *v138 = v140;
              *v131 = v140;
              delete__(v138);
            }
            goto LABEL_210;
          }
          v132 = (v132 + 1) % DAT_055c9bd4;
          if ( ++Ownero >= DAT_055c9bd4 )
          {
            goto LABEL_208;
          }
        }
      }
      else
      {
LABEL_208:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v134 = operator_new(2u);
      *(BYTE *)(v134 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v134, Owner + 770);
LABEL_210:
      Ownerp = *v131;
      v244 = (const char *)(Owner + 770);
      v240 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v240), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v240), 4) )
          {
            if ( v240 != -1 )
            {
              v141 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v142 = v141 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v141);
              v143 = v142[1] - 1;
              v142[1] = v143;
              if ( !v143 )
              {
                v144 = (BYTE *)operator_new(1u);
                v145 = *v131 + 71;
                *v144 = v145;
                *v144 = (DAT_00559050[0] ^ v145) + 35;
                *v131 = rand();
                *v142 = *v144;
                delete__(v144);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v240 = (v240 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_214;
          }
        }
      }
      else
      {
LABEL_214:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerp == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        v146 = *(float *)(Owner + 36) + 20.0;
        v147 = *(float *)(Owner + 32);
        v148 = *(float *)(Owner + 16);
        v246[0] = *(float *)(Owner + 28);
        v149 = *(float *)(Owner + 20);
        v246[1] = v147;
        v246[2] = v146;
        v150 = *(float *)(Owner + 24) + 50.0;
        v248[0] = v148;
        v248[1] = v149;
        Light[0] = 0.42000002;
        Light[1] = 0.84000003;
        Light[2] = 1.4;
        v151 = 9;
        v248[2] = v150;
        do
        {
          v246[2] = v246[2] + 40.0;
          CreateEffect(1210, v248, v246, Light, 0, 0, (float *)-1, 0, 0);
          --v151;
        }
        while ( v151 );
      }
      break;
    case 53:
    case 58:
    case 59:
      if ( *(BYTE *)(Owner + 757) == 1 )
      {
        FUN_00466300(Owner + 16);
      }
      if ( *(BYTE *)(Owner + 757) == 14 && *(BYTE *)(Owner + 747) == 59 )
      {
        v117 = (char *)(Owner + 770);
        v244 = (const char *)(Owner + 770);
        Ownerl = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * Ownerl), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * Ownerl), 4) )
            {
              if ( Ownerl == -1 )
              {
                break;
              }
              v119 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              if ( v119 == -1 )
              {
                v120 = 0;
              }
              else
              {
                v120 = *(char **)(DAT_055c9bcc + 4 * v119);
              }
              v121 = v120[1] + 1;
              v120[1] = v121;
              if ( v121 < 2u )
              {
                v122 = (char *)operator_new(1u);
                v123 = *v120;
                *v122 = *v120;
                v123 -= 35;
                *v122 = v123;
                v124 = (DAT_00559050[0] ^ v123) - 71;
                *v122 = v124;
                *v117 = v124;
                delete__(v122);
              }
              goto LABEL_181;
            }
            v6 = ++v243 < DAT_055c9bd4;
            Ownerl = (Ownerl + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_179;
            }
          }
        }
        else
        {
LABEL_179:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        v118 = operator_new(2u);
        *(BYTE *)(v118 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v118, Owner + 770);
LABEL_181:
        Ownerm = *v117;
        v244 = (const char *)(Owner + 770);
        v239 = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v239), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v239), 4) )
            {
              if ( v239 != -1 )
              {
                v125 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
                v126 = v125 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v125);
                v127 = v126[1] - 1;
                v126[1] = v127;
                if ( !v127 )
                {
                  FUN_00423710(v126, (BYTE *)(Owner + 770));
                }
              }
              break;
            }
            v6 = ++v243 < DAT_055c9bd4;
            v239 = (v239 + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_185;
            }
          }
        }
        else
        {
LABEL_185:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        if ( Ownerm == 50 )
        {
          v128 = 0;
          Ownern = 0;
          do
          {
            v129 = *(float *)(Owner + 32);
            v130 = (double)Ownern * 20.0 + *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v129;
            v246[2] = v130;
            CreateEffect(568, (float *)(Owner + 16), v246, (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
            Ownern = ++v128;
          }
          while ( v128 < 18 );
        }
      }
      break;
    case 54:
    case 57:
    case -105:
      if ( *(BYTE *)(Owner + 757) == 1 )
      {
        v113 = *(DWORD *)(Owner + 276);
        v114 = *(unsigned char *)(Owner + 628);
        v248[0] = 60.0;
        v248[1] = -110.0;
        v248[2] = 0.0;
        TransformPosition(v2, (float (*)[4])(v113 + 48 * v114), v248, TargetPosition, 1);
        CreateEffect(223, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, Owner, -1, 0, 0);
        if ( *(BYTE *)(Owner + 747) == 57 )
        {
          v115 = *(float *)(Owner + 28);
          v116 = *(float *)(Owner + 32);
          Angle[2] = *(float *)(Owner + 36) + 20.0;
          Angle[0] = v115;
          Angle[1] = v116;
          CreateEffect(223, (float *)(Owner + 16), Angle, (float *)(Owner + 232), 0, Owner, -1, 0, 0);
          Angle[2] = Angle[2] - 40.0;
          CreateEffect(223, (float *)(Owner + 16), Angle, (float *)(Owner + 232), 0, Owner, -1, 0, 0);
        }
      }
      break;
    case 61:
    case 63:
      v19 = v3 == 63;
      v79 = *(BYTE *)(Owner + 757);
      if ( v19 )
      {
        if ( v79 == 1 )
        {
          FUN_00466300(Owner + 16);
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
        }
        v80 = (char *)(Owner + 770);
        v243 = Owner + 770;
        v81 = AE_ht_hash(Owner + 770);
        v245 = 0;
        Ownerg = 0;
        if ( DAT_055c9bd4 )
        {
          while ( 1 )
          {
            v82 = (const char *)(DAT_055c9bd0 + 4 * v81);
            v244 = v82;
            if ( !memcmp((const char *)&v245, v82, 4) )
            {
              break;
            }
            if ( !memcmp((const char *)&v243, v244, 4) )
            {
              if ( v81 == -1 )
              {
                break;
              }
              v84 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              if ( v84 == -1 )
              {
                v85 = 0;
              }
              else
              {
                v85 = *(char **)(DAT_055c9bcc + 4 * v84);
              }
              v86 = v85[1] + 1;
              v85[1] = v86;
              if ( v86 < 2u )
              {
                v87 = (char *)operator_new(1u);
                v88 = *v85;
                *v87 = *v85;
                v88 -= 35;
                *v87 = v88;
                v89 = (DAT_00559050[0] ^ v88) - 71;
                *v87 = v89;
                *v80 = v89;
                delete__(v87);
              }
              goto LABEL_116;
            }
            v81 = (v81 + 1) % DAT_055c9bd4;
            if ( ++Ownerg >= DAT_055c9bd4 )
            {
              goto LABEL_114;
            }
          }
        }
        else
        {
LABEL_114:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        v83 = operator_new(2u);
        *(BYTE *)(v83 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v83, Owner + 770);
LABEL_116:
        Ownerh = *v80;
        v244 = (const char *)(Owner + 770);
        v237 = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v237), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v237), 4) )
            {
              if ( v237 != -1 )
              {
                v90 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
                v91 = v90 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v90);
                v92 = v91[1] - 1;
                v91[1] = v92;
                if ( !v92 )
                {
                  v93 = (BYTE *)operator_new(1u);
                  v94 = *v80 + 71;
                  *v93 = v94;
                  *v93 = (DAT_00559050[0] ^ v94) + 35;
                  *v80 = rand();
                  *v91 = *v93;
                  delete__(v93);
                }
              }
              break;
            }
            v6 = ++v243 < DAT_055c9bd4;
            v237 = (v237 + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_120;
            }
          }
        }
        else
        {
LABEL_120:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        if ( Ownerh == 50 )
        {
          if ( *(BYTE *)(Owner + 747) == 63 )
          {
            TargetPosition[0] = (double)(rand() % 800) + *(float *)(Owner + 16) - 400.0;
            Ownery = rand() % 800;
            TargetPosition[2] = *(float *)(Owner + 24);
            TargetPosition[1] = (double)Ownery + *(float *)(Owner + 20) - 400.0;
            CreateEffect(240, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          }
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            v95 = 0;
            Owneri = 0;
            do
            {
              v96 = *(float *)(Owner + 32);
              v97 = (double)Owneri * 20.0 + *(float *)(Owner + 36);
              v246[0] = *(float *)(Owner + 28);
              v246[1] = v96;
              v246[2] = v97;
              CreateEffect(568, (float *)(Owner + 16), v246, (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
              Owneri = ++v95;
            }
            while ( v95 < 18 );
          }
        }
      }
      else if ( v79 == 1 )
      {
        CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      break;
    case 66:
      v98 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v99 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerj = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v100 = (const char *)(DAT_055c9bd0 + 4 * v99);
          v244 = v100;
          if ( !memcmp((const char *)&v245, v100, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v99 == -1 )
            {
              break;
            }
            v102 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v102 == -1 )
            {
              v103 = 0;
            }
            else
            {
              v103 = *(char **)(DAT_055c9bcc + 4 * v102);
            }
            v104 = v103[1] + 1;
            v103[1] = v104;
            if ( v104 < 2u )
            {
              v105 = (char *)operator_new(1u);
              v106 = *v103;
              *v105 = *v103;
              v106 -= 35;
              *v105 = v106;
              v107 = (DAT_00559050[0] ^ v106) - 71;
              *v105 = v107;
              *v98 = v107;
              delete__(v105);
            }
            goto LABEL_148;
          }
          v99 = (v99 + 1) % DAT_055c9bd4;
          if ( ++Ownerj >= DAT_055c9bd4 )
          {
            goto LABEL_146;
          }
        }
      }
      else
      {
LABEL_146:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v101 = operator_new(2u);
      *(BYTE *)(v101 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v101, Owner + 770);
LABEL_148:
      Ownerk = *v98;
      v244 = (const char *)(Owner + 770);
      v238 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v238), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v238), 4) )
          {
            if ( v238 != -1 )
            {
              v108 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v109 = v108 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v108);
              v110 = v109[1] - 1;
              v109[1] = v110;
              if ( !v110 )
              {
                v111 = (BYTE *)operator_new(1u);
                v112 = *v98 + 71;
                *v111 = v112;
                *v111 = (DAT_00559050[0] ^ v112) + 35;
                *v98 = rand();
                *v109 = *v111;
                delete__(v111);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v238 = (v238 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_152;
          }
        }
      }
      else
      {
LABEL_152:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerk == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        goto LABEL_30;
      }
      break;
    case 70:
      if ( *(BYTE *)(Owner + 757) == 5 && CharactersClient )  // guard de port
      {
        v22 = 20;
        v23 = CharactersClient + 916 * *(short *)(Owner + 784);
        v24 = (float *)(v23 + 28);
        v25 = (float *)(v23 + 16);
        do
        {
          CreateEffect(1271, v25, v24, Light, 0, 0, (float *)-1, 0, 0);
          --v22;
        }
        while ( v22 );
      }
      break;
    case 71:
    case 74:
      v26 = *(BYTE *)(Owner + 261);
      if ( (v26 == 3 || v26 == 4) && *(BYTE *)(Owner + 757) == 5 )
      {
        FUN_00466300(Owner + 16);
        CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      break;
    case 72:
      v27 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v28 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerc = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v29 = (const char *)(DAT_055c9bd0 + 4 * v28);
          v244 = v29;
          if ( !memcmp((const char *)&v245, v29, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v28 == -1 )
            {
              break;
            }
            v31 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v31 == -1 )
            {
              v32 = 0;
            }
            else
            {
              v32 = *(char **)(DAT_055c9bcc + 4 * v31);
            }
            v33 = v32[1] + 1;
            v32[1] = v33;
            if ( v33 < 2u )
            {
              v34 = (char *)operator_new(1u);
              v35 = *v32;
              *v34 = *v32;
              v35 -= 35;
              *v34 = v35;
              v36 = (DAT_00559050[0] ^ v35) - 71;
              *v34 = v36;
              *v27 = v36;
              delete__(v34);
            }
            goto LABEL_46;
          }
          v28 = (v28 + 1) % DAT_055c9bd4;
          if ( ++Ownerc >= DAT_055c9bd4 )
          {
            goto LABEL_44;
          }
        }
      }
      else
      {
LABEL_44:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v30 = operator_new(2u);
      *(BYTE *)(v30 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v30, Owner + 770);
LABEL_46:
      Ownerd = *v27;
      v244 = (const char *)(Owner + 770);
      v235 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v235), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v235), 4) )
          {
            if ( v235 != -1 )
            {
              v37 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v38 = v37 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v37);
              v39 = v38[1] - 1;
              v38[1] = v39;
              if ( !v39 )
              {
                v40 = (BYTE *)operator_new(1u);
                v41 = *v27 + 71;
                *v40 = v41;
                *v40 = (DAT_00559050[0] ^ v41) + 35;
                *v27 = rand();
                *v38 = *v40;
                delete__(v40);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v235 = (v235 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_50;
          }
        }
      }
      else
      {
LABEL_50:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerd == 50 && *(BYTE *)(Owner + 757) == 14 )
      {
        memset(Angle, 0, sizeof(Angle));
        v42 = 36;
        do
        {
          Angle[0] = (float)(rand() % 360);
          Angle[1] = (float)(rand() % 360);
          v43 = rand();
          Position[1] = *(float *)(Owner + 20);
          Position[0] = *(float *)(Owner + 16);
          Angle[2] = (float)(v43 % 360);
          Position[2] = *(float *)(Owner + 24) + 100.0;
          CreateJoint(1253, Position, Position, Angle, 1, 0, 60.0, 0, 0);
          --v42;
        }
        while ( v42 );
      }
      break;
    case 73:
    case 75:
      v44 = *(BYTE *)(Owner + 757);
      if ( *(BYTE *)(Owner + 261) == 3 )
      {
        if ( v44 == 11 )
        {
          FUN_00466300(Owner + 16);
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          v45 = 5;
          do
          {
            v46 = *(float *)(Owner + 36);
            v47 = *(float *)(Owner + 28) + 45.0;
            v48 = *(float *)(Owner + 16);
            v246[1] = *(float *)(Owner + 32);
            v49 = *(float *)(Owner + 20);
            v246[2] = v46;
            v50 = *(float *)(Owner + 24);
            Light[0] = 1.0;
            v246[0] = v47;
            Light[1] = 0.5;
            Light[2] = 0.0;
            TargetPosition[0] = v48;
            TargetPosition[1] = v49;
            TargetPosition[2] = v50;
            TargetPosition[0] = (double)(rand() % 1001 - 500) + v48;
            v51 = rand();
            *(float *)(Owner + 368) = TargetPosition[0];
            TargetPosition[1] = (double)(v51 % 1001 - 500) + TargetPosition[1];
            TargetPosition[2] = TargetPosition[2] + 500.0;
            v52 = TargetPosition[2];
            *(float *)(Owner + 372) = TargetPosition[1];
            *(float *)(Owner + 376) = v52;
            CreateEffect(256, TargetPosition, v246, Light, 1, Owner, -1, 0, 0);
            --v45;
          }
          while ( v45 );
        }
      }
      else
      {
        if ( v44 == 13 )
        {
          v53 = *(float *)(Owner + 32);
          v54 = *(float *)(Owner + 28) + 45.0;
          v246[2] = *(float *)(Owner + 36);
          v55 = *(DWORD *)(Owner + 276);
          v246[1] = v53;
          v246[0] = v54;
          Light[0] = 1.0;
          Light[1] = 0.5;
          Light[2] = 0.0;
          v248[0] = -50.0;
          v248[1] = 100.0;
          v248[2] = 0.0;
          TransformPosition(v2, (float (*)[4])(v55 + 528), v248, TargetPosition, 1);
          v56 = TargetPosition[1];
          v57 = TargetPosition[2];
          *(float *)(Owner + 368) = TargetPosition[0];
          *(float *)(Owner + 372) = v56;
          *(float *)(Owner + 376) = v57;
          CreateEffect(256, TargetPosition, v246, Light, 1, Owner, -1, 0, 0);
          goto LABEL_75;
        }
        if ( v44 == 9 )
        {
          v58 = *(float *)(Owner + 32);
          v59 = *(float *)(Owner + 28) + 45.0;
          v246[2] = *(float *)(Owner + 36);
          v60 = *(DWORD *)(Owner + 276);
          v246[1] = v58;
          v246[0] = v59;
          Light[0] = 1.0;
          Light[1] = 0.5;
          Light[2] = 0.0;
          memset(v248, 0, sizeof(v248));
          TransformPosition(v2, (float (*)[4])(v60 + 528), v248, TargetPosition, 1);
        }
      }
      break;
    case 77:
      v61 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v62 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownere = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v63 = (const char *)(DAT_055c9bd0 + 4 * v62);
          v244 = v63;
          if ( !memcmp((const char *)&v245, v63, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v62 == -1 )
            {
              break;
            }
            v65 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v65 == -1 )
            {
              v66 = 0;
            }
            else
            {
              v66 = *(char **)(DAT_055c9bcc + 4 * v65);
            }
            v67 = v66[1] + 1;
            v66[1] = v67;
            if ( v67 < 2u )
            {
              v68 = (char *)operator_new(1u);
              v69 = *v66;
              *v68 = *v66;
              v69 -= 35;
              *v68 = v69;
              v70 = (DAT_00559050[0] ^ v69) - 71;
              *v68 = v70;
              *v61 = v70;
              delete__(v68);
            }
            goto LABEL_84;
          }
          v62 = (v62 + 1) % DAT_055c9bd4;
          if ( ++Ownere >= DAT_055c9bd4 )
          {
            goto LABEL_82;
          }
        }
      }
      else
      {
LABEL_82:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v64 = operator_new(2u);
      *(BYTE *)(v64 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v64, Owner + 770);
LABEL_84:
      Ownerf = *v61;
      v244 = (const char *)(Owner + 770);
      v236 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v236), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v236), 4) )
          {
            if ( v236 != -1 )
            {
              v71 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v72 = v71 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v71);
              v73 = v72[1] - 1;
              v72[1] = v73;
              if ( !v73 )
              {
                v74 = (BYTE *)operator_new(1u);
                v75 = *v61 + 71;
                *v74 = v75;
                *v74 = (DAT_00559050[0] ^ v75) + 35;
                *v61 = rand();
                *v72 = *v74;
                delete__(v74);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v236 = (v236 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_88;
          }
        }
      }
      else
      {
LABEL_88:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerf == 50 )
      {
        v76 = *(BYTE *)(Owner + 757);
        if ( v76 == 2 || v76 == 6 )
        {
          memset(Angle, 0, sizeof(Angle));
          v77 = 40;
          do
          {
            Angle[0] = (float)(rand() % 360);
            Angle[1] = (float)(rand() % 360);
            v78 = rand();
            Position[1] = *(float *)(Owner + 20);
            Position[0] = *(float *)(Owner + 16);
            Angle[2] = (float)(v78 % 360);
            Position[2] = *(float *)(Owner + 24) + 100.0;
            CreateJoint(1253, Position, Position, Angle, 3, 0, 50.0, 0, 0);
            --v77;
          }
          while ( v77 );
        }
      }
      break;
    case 89:
    case 95:
    case 112:
    case 118:
    case 124:
    case -126:
    case -120:
      v4 = (char *)(Owner + 770);
      v244 = (const char *)(Owner + 770);
      v5 = AE_ht_hash(Owner + 770);
      v243 = 0;
      Ownera = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v243, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
          {
            if ( v5 == -1 )
            {
              break;
            }
            v8 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
            if ( v8 == -1 )
            {
              v9 = 0;
            }
            else
            {
              v9 = *(char **)(DAT_055c9bcc + 4 * v8);
            }
            v10 = v9[1] + 1;
            v9[1] = v10;
            if ( v10 < 2u )
            {
              v11 = (char *)operator_new(1u);
              v12 = *v9;
              *v11 = *v9;
              v12 -= 35;
              *v11 = v12;
              v13 = (DAT_00559050[0] ^ v12) - 71;
              *v11 = v13;
              *v4 = v13;
              delete__(v11);
            }
            goto LABEL_8;
          }
          v6 = ++Ownera < DAT_055c9bd4;
          v5 = (v5 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_6;
          }
        }
      }
      else
      {
LABEL_6:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v7 = operator_new(2u);
      *(BYTE *)(v7 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v7, Owner + 770);
LABEL_8:
      Ownerb = *v4;
      v245 = Owner + 770;
      v234 = AE_ht_hash(Owner + 770);
      v244 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v234), 4) )
        {
          if ( !memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v234), 4) )
          {
            if ( v234 != -1 )
            {
              v14 = FUN_004041e0(&DAT_055c9bc8, Owner + 770);
              v15 = v14 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v14);
              v16 = v15[1] - 1;
              v15[1] = v16;
              if ( !v16 )
              {
                v17 = (BYTE *)operator_new(1u);
                v18 = *v4 + 71;
                *v17 = v18;
                *v17 = (DAT_00559050[0] ^ v18) + 35;
                *v4 = rand();
                *v15 = *v17;
                delete__(v17);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v234 = (v234 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_12;
          }
        }
      }
      else
      {
LABEL_12:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerb == 50 )
      {
        v20 = rand() & 0x80000001;
        v19 = v20 == 0;
        if ( v20 < 0 )
        {
          v19 = (((BYTE)v20 - 1) | 0xFFFFFFFE) == -1;
        }
        if ( !v19 )
        {
LABEL_31:
          TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
          v21 = (double)(rand() % 1024) + *(float *)(Owner + 20);
          TargetPosition[2] = *(float *)(Owner + 24);
          TargetPosition[1] = v21 - 512.0;
          CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          goto LABEL_75;
        }
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
LABEL_30:
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 1, 0, -1, 0, 0);
        }
      }
      break;
    case 103:
      if ( FUN_0045fae0(&DAT_055c9bc8, (BYTE *)(Owner + 770)) == 50 )
      {
        TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
        v198 = (double)(rand() % 1024) + *(float *)(Owner + 20);
        TargetPosition[2] = *(float *)(Owner + 24);
        TargetPosition[1] = v198 - 512.0;
        CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
LABEL_75:
        PlayBuffer(46, 0, 0);
      }
      break;
    default:
      break;
  }
  v202 = *(WORD *)(Owner + 784);
  // `&& CharactersClient` no esta en IDA: en nuestro build el pool arranca en 0
  // hasta que se aloca, y `0 + 916*idx` seria un puntero basura. Mismo guard que
  // Render_Frame.cpp:395.
  if ( v202 >= 0 && v202 < 400 && CharactersClient )
  {
    v203 = CharactersClient + 916 * v202;
    if ( FUN_0045fae0(&DAT_055c9bc8, (BYTE *)(Owner + 770)) == 17 )
    {
      switch ( *(BYTE *)(Owner + 747) )
      {
        case 0x25:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( i = 0; i < 4; ++i )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (i >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x2E:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( j = 0; j < 4; ++j )
          {
            memset(v246, 0, sizeof(v246));
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (j >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            CreateJoint(1166, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            CreateJoint(1166, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 10.0, -1, 0);
          }
          break;
        case 0x3D:
          for ( k = 0; k < 6; ++k )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (k >= 3) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 10.0, -1, 0);
          }
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( m = 0; m < 4; ++m )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (m >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x42:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(60, 0, 0);
          }
          v219 = 0;
          Ownerw = 45.0 - (double)((int)((__int64)WorldTime / 10 + 3 * *(unsigned char *)(Owner + 757)) % 90) + 180.0;
          do
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (v219 % 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = Ownerw;
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
            ++v219;
            Ownerw = Ownerw + 270.0;
          }
          while ( v219 < 4 );
          break;
        case 0x45:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            v213 = (float *)(Owner + 16);
            v214 = 4;
            do
            {
              CreateJoint(1249, v213, v213, v246, 7, v203, 50.0, -1, 0);
              CreateJoint(1249, TargetPosition, TargetPosition, v246, 7, v203, 50.0, -1, 0);
              --v214;
            }
            while ( v214 );
          }
          break;
        case 0x49:
        case 0x4B:
          if ( *(BYTE *)(Owner + 261) == 4 && *(BYTE *)(Owner + 757) == 13 )
          {
            v210 = *(float *)(Owner + 32);
            v211 = *(float *)(Owner + 28) + 45.0;
            v246[2] = *(float *)(Owner + 36);
            v212 = *(DWORD *)(Owner + 276);
            v246[1] = v210;
            v246[0] = v211;
            Light[0] = 1.0;
            Light[1] = 0.5;
            Light[2] = 0.0;
            v248[0] = -50.0;
            v248[1] = 100.0;
            v248[2] = 0.0;
            TransformPosition(This, (float (*)[4])(v212 + 528), v248, TargetPosition, 1);
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
          }
          break;
        case 0x4D:
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, (float (*)[4])Matrix, v248, TargetPosition, 1);
            v208 = *(float *)(Owner + 32);
            v209 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v208;
            v246[2] = v209;
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, TargetPosition, v246, 2, v203, 50.0, -1, 0);
          }
          break;
        case 0x57:
        case 0x5D:
        case 0x63:
        case 0x74:
        case 0x7A:
        case 0x80:
        case 0x86:
          if ( *(BYTE *)(Owner + 757) == 13 )
          {
            v232 = (float (*)[4])(*(DWORD *)(Owner + 276) + 288);
            Light[0] = 1.0;
            Light[1] = 1.0;
            Light[2] = 1.0;
            v248[0] = 60.0;
            v248[1] = 30.0;
            v248[2] = 0.0;
            TransformPosition(This, v232, v248, TargetPosition, 1);
            v206 = *(float *)(Owner + 32);
            v207 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v206;
            v246[2] = v207;
            CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 5, 0, -1, 0, 0);
          }
          break;
        case 0x59:
        case 0x5F:
        case 0x70:
        case 0x76:
        case 0x7C:
        case 0x82:
        case 0x88:
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            v231 = (float (*)[4])(*(DWORD *)(Owner + 276) + 1584);
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, v231, v248, TargetPosition, 1);
            v204 = *(float *)(Owner + 32);
            v205 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v204;
            v246[2] = v205;
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, TargetPosition, v246, 2, v203, 50.0, -1, 0);
          }
          break;
        default:
          return;
      }
    }
    else if ( FUN_0045fae0(&DAT_055c9bc8, (BYTE *)(Owner + 770)) == 3 )
    {
      switch ( *(BYTE *)(Owner + 747) )
      {
        case 0x22:
          for ( n = 0; n < 4; ++n )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (n >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
            Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          }
          break;
        case 0x25:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( ii = 0; ii < 4; ++ii )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (ii >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x27:
          v19 = *(WORD *)(Owner + 2) == 390;
          v248[0] = 0.0;
          v248[1] = 0.0;
          if ( !v19 )
          {
            v248[1] = -130.0;
          }
          v225 = *(DWORD *)(Owner + 276);
          v226 = *(unsigned char *)(Owner + 628);
          v248[2] = 0.0;
          TransformPosition(This, (float (*)[4])(v225 + 48 * v226), v248, TargetPosition, 1);
          v227 = *(float *)(Owner + 36);
          v246[0] = -60.0;
          v246[1] = 0.0;
          v246[2] = v227;
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
          Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          break;
        case 0x30:
          for ( jj = 0; jj < 6; ++jj )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (jj >= 3) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 10.0, -1, 0);
          }
          break;
        case 0x4D:
          if ( *(BYTE *)(Owner + 757) >= 8u )
          {
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, (float (*)[4])DAT_07abf3e4, v248, TargetPosition, 1);
            v221 = 4;
            do
            {
              v246[0] = 0.0;
              v246[1] = 0.0;
              v246[2] = (float)(rand() % 360);
              CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 80.0, -1, 0);
              --v221;
            }
            while ( v221 );
          }
          break;
        case 0x59:
        case 0x5F:
        case 0x70:
        case 0x76:
        case 0x7C:
        case 0x82:
        case 0x88:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(60, 0, 0);
          }
          v220 = 0;
          Ownerx = 45.0 - (double)((int)((__int64)WorldTime / 10 + 3 * *(unsigned char *)(Owner + 757)) % 90) + 180.0;
          do
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (v220 % 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = Ownerx;
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
            ++v220;
            Ownerx = Ownerx + 270.0;
          }
          while ( v220 < 4 );
          break;
        default:
          v19 = *(WORD *)(Owner + 2) == 390;
          v248[0] = 0.0;
          v248[1] = 0.0;
          if ( !v19 )
          {
            v248[1] = -130.0;
          }
          v228 = (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 628));
          v248[2] = 0.0;
          TransformPosition(This, v228, v248, TargetPosition, 1);
          v229 = *(float *)(Owner + 36);
          v246[0] = -60.0;
          v246[1] = 0.0;
          v246[2] = v229;
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
          Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          break;
      }
    }
  }
}
#endif

// ── FUN_00451ea0 (IDA-only, gated) ──
#if defined(IDA_PORT_00451EA0)
void __cdecl FUN_00451ea0(int a1, DWORD This, int a3, int a4)
{
  float Position[3]; // [esp+8h] [ebp-Ch] BYREF

  memset(Position, 0, sizeof(Position));
  TransformPosition(This, (float (*)[4])(*(DWORD *)(a1 + 276) + 48 * a3), Position, (float *)(a1 + 76), 1);
  memset(Position, 0, sizeof(Position));
  TransformPosition(This, (float (*)[4])(*(DWORD *)(a1 + 276) + 48 * a4), Position, (float *)(a1 + 64), 1);
}
#endif

// ── FUN_00452030 (IDA-only, gated) ──
#if defined(IDA_PORT_00452030)
void __cdecl FUN_00452030(int a1)
{
  float Position[3]; // [esp+4h] [ebp-Ch] BYREF
  int v3; // [esp+14h] [ebp+4h]

  if ( *(BYTE *)(a1 + 261) == 2 )
  {
    Position[0] = (double)(rand() % 200) + *(float *)(a1 + 16) - 100.0;
    v3 = rand() % 200;
    Position[2] = *(float *)(a1 + 24);
    Position[1] = (double)v3 + *(float *)(a1 + 20) - 100.0;
    Particle_Spawn(1221, Position, (float *)(a1 + 28), (float *)(a1 + 232), 0, 1.0, 0);
  }
}
#endif

// ── FUN_0045c720 (IDA-only, gated) ──
#if defined(IDA_PORT_0045C720)
void __cdecl FUN_0045c720(DWORD c)
{
  unsigned char v1; // al
  char v2; // cl
  int v3; // eax
  int v4; // ecx

  if ( *(WORD *)(c + 2) == 390 )
  {
    v1 = *(BYTE *)(c + 261);
    v2 = 1;
    if ( v1 >= 0x85u && v1 <= 0x8Cu )
    {
      v2 = 0;
    }
    if ( (v1 < 0x22u || v1 > 0x5Bu) && v2 )
    {
      SetPlayerStop(c);
    }
    v3 = *(BYTE *)(c + 444) & 7;
    v4 = *(unsigned char *)(c + 444) >> 3;
    if ( *(short *)(c + 504) == v3 + 912 )
    {
      *(BYTE *)(c + 506) = 0;
      *(WORD *)(c + 504) = v3 + 4 * v4 + 912;
      *(BYTE *)(c + 507) = 0;
    }
    if ( *(short *)(c + 552) == v3 + 919 )
    {
      *(BYTE *)(c + 530) = 0;
      *(WORD *)(c + 528) = v3 + 4 * v4 + 919;
      *(BYTE *)(c + 531) = 0;
    }
    if ( *(short *)(c + 576) == v3 + 926 )
    {
      *(BYTE *)(c + 554) = 0;
      *(WORD *)(c + 552) = v3 + 4 * v4 + 926;
      *(BYTE *)(c + 555) = 0;
    }
    if ( *(short *)(c + 600) == v3 + 933 )
    {
      *(BYTE *)(c + 578) = 0;
      *(WORD *)(c + 576) = v3 + 4 * v4 + 933;
      *(BYTE *)(c + 579) = 0;
    }
    if ( *(short *)(c + 624) == v3 + 940 )
    {
      *(BYTE *)(c + 602) = 0;
      *(WORD *)(c + 600) = v3 + 4 * v4 + 940;
      *(BYTE *)(c + 603) = 0;
    }
    SetCharacterScale(c);
  }
}
#endif

// ── FUN_0045fae0 (IDA-only, gated) ──
#if defined(IDA_PORT_0045FAE0)
char __cdecl FUN_0045fae0(int *_this, BYTE *a2)
{
  BYTE *v3; // esi
  int v4; // eax
  int v5; // eax
  int v6; // ecx
  unsigned int v7; // edx
  unsigned int v8; // ecx
  unsigned int v9; // eax
  char *v10; // esi
  unsigned char v11; // al
  char *v12; // eax
  char v13; // cl
  char v14; // cl
  int v15; // edx
  int v16; // eax
  int v17; // ecx
  unsigned int v18; // ebx
  unsigned int v19; // ecx
  bool v20; // cf
  int v22; // eax
  unsigned int v23; // eax
  BYTE *v24; // eax
  char v25; // cl
  char v26; // [esp+13h] [ebp-Dh]
  int v27; // [esp+14h] [ebp-Ch]
  int v28; // [esp+14h] [ebp-Ch]
  BYTE *v29; // [esp+18h] [ebp-8h] BYREF
  BYTE *v30; // [esp+1Ch] [ebp-4h] BYREF

  v3 = a2;
  v4 = *_this;
  v30 = a2;
  v5 = (*(int (__cdecl **)(int *, BYTE *))(v4 + 12))(_this, a2);
  v6 = _this[3];
  v7 = v5;
  v29 = 0;
  v27 = 0;
  if ( v6 )
  {
    while ( 1 )
    {
      if ( !memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v7), 4) )
      {
        goto LABEL_19;
      }
      if ( !memcmp((const char *)&v30, (const char *)(_this[2] + 4 * v7), 4) )
      {
        break;
      }
      v8 = _this[3];
      v7 = (v7 + 1) % v8;
      if ( ++v27 >= v8 )
      {
        v3 = a2;
        goto LABEL_6;
      }
    }
    if ( v7 == -1 )
    {
LABEL_19:
      v3 = a2;
      goto LABEL_20;
    }
    v9 = FUN_004041e0(_this, (int)a2);
    if ( v9 == -1 )
    {
      v10 = 0;
    }
    else
    {
      v10 = *(char **)(_this[1] + 4 * v9);
    }
    v11 = v10[1] + 1;
    v10[1] = v11;
    if ( v11 < 2u )
    {
      v12 = (char *)operator_new(1u);
      v13 = *v10;
      *v12 = *v10;
      v13 -= 35;
      *v12 = v13;
      v14 = (DAT_00559050[0] ^ v13) - 71;
      *v12 = v14;
      *a2 = v14;
      delete__(v12);
    }
    v3 = a2;
  }
  else
  {
LABEL_6:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
LABEL_20:
    v22 = operator_new(2u);
    *(BYTE *)(v22 + 1) = 1;
    FUN_00403f80(_this, v22, (int)v3);
  }
  v15 = *_this;
  v26 = *v3;
  v29 = v3;
  v16 = (*(int (__cdecl **)(int *, BYTE *))(v15 + 12))(_this, v3);
  v17 = _this[3];
  v18 = v16;
  v30 = 0;
  v28 = 0;
  if ( v17 )
  {
    while ( memcmp((const char *)&v30, (const char *)(_this[2] + 4 * v18), 4) )
    {
      if ( !memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v18), 4) )
      {
        if ( v18 != -1 )
        {
          v23 = FUN_004041e0(_this, (int)a2);
          if ( v23 == -1 )
          {
            v24 = 0;
          }
          else
          {
            v24 = *(BYTE **)(_this[1] + 4 * v23);
          }
          v25 = v24[1] - 1;
          v24[1] = v25;
          if ( !v25 )
          {
            FUN_00423710(v24, a2);
          }
        }
        return v26;
      }
      v19 = _this[3];
      v20 = ++v28 < v19;
      v18 = (v18 + 1) % v19;
      if ( !v20 )
      {
        goto LABEL_18;
      }
    }
    return v26;
  }
  else
  {
LABEL_18:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    return v26;
  }
}
#endif

// ── FUN_0046b980 (IDA-only, gated) ──
#if defined(IDA_PORT_0046B980)
void __cdecl FUN_0046b980(DWORD o)
{
  int v2; // edi
  DWORD v3; // ebx
  char v4; // al
  char v5; // cl
  int v6; // edx
  int v7; // eax
  int v8; // ecx
  DWORD v9; // eax
  float v10; // eax
  double v11; // st7
  float v12; // eax
  float v13; // ecx
  double v14; // st7
  float v15; // eax
  float v16; // [esp-20h] [ebp-44h]
  float alpha; // [esp+4h] [ebp-20h]
  float v19; // [esp+8h] [ebp-1Ch]
  float Light[3]; // [esp+Ch] [ebp-18h] BYREF
  float Angle[3]; // [esp+18h] [ebp-Ch] BYREF
  DWORD oa; // [esp+28h] [ebp+4h]

  if ( (double)*(int *)(o + 96) > 10.0 )
  {
    v2 = *(unsigned char *)(*(DWORD *)(o + 252) + 136) + 400;
    alpha = *(float *)(o + 360);
    v3 = Models + 188 * v2;
    v4 = *(BYTE *)(Hero + 444);
    v5 = *(BYTE *)(o + 261);
    *(DWORD *)(v3 + 108) = *(DWORD *)(o + 16);
    v6 = *(short *)(o + 2);
    *(BYTE *)(v3 + 152) = v4 & 7;
    v7 = *(DWORD *)(o + 20);
    *(BYTE *)(v3 + 160) = v5;
    v8 = *(DWORD *)(o + 24);
    *(DWORD *)(v3 + 112) = v7;
    v9 = *(DWORD *)(o + 216);
    *(DWORD *)(v3 + 116) = v8;
    v19 = (float)v6;
    oa = v9;
    *(WORD *)(o + 2) = v2;
    ItemObjectAttribute(o);
    v10 = *(float *)(o + 20);
    *(DWORD *)(o + 216) = oa;
    RequestTerrainLight(*(float *)(o + 16), v10, Light);
    v11 = Light[0] + *(float *)(o + 232);
    v12 = *(float *)(o + 32);
    v13 = *(float *)(o + 36);
    Angle[0] = *(float *)(o + 28);
    Angle[1] = v12;
    Light[0] = v11;
    v14 = Light[1] + *(float *)(o + 236);
    Angle[2] = v13;
    (BYTE)(v13) = *(BYTE *)(o + 262);
    Light[1] = v14;
    v15 = *(float *)(o + 264);
    v16 = *(float *)(o + 268);
    Light[2] = Light[2] + *(float *)(o + 240);
    BMD_Animation(v3, (float (*)[3][4])BoneMatrix, v15, v16, (BYTE)(v13), Angle, (float *)(o + 40), 0, 0);
    RenderPartObject(o, v2, 0, Light, alpha, 8 * *(unsigned char *)(*(DWORD *)(o + 252) + 137), 0, 1, 1, 1, 0, 2);
    *(WORD *)(o + 2) = (__int64)v19;
  }
}
#endif

// ── FUN_0046c5a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0046C5A0)
int __cdecl FUN_0046c5a0(int a1, int a2, float Position[3], float angles[3])
{
  int v4; // ebx
  int result; // eax
  float out[3]; // [esp+Ch] [ebp-54h] BYREF
  float Light[3]; // [esp+18h] [ebp-48h] BYREF
  float in1[3]; // [esp+24h] [ebp-3Ch] BYREF
  float in2[3][4]; // [esp+30h] [ebp-30h] BYREF

  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  Particle_Spawn(1176, Position, (float *)(a2 + 28), Light, 0, 1.0, 0);
  in1[0] = 0.0;
  in1[1] = 50.0;
  in1[2] = 0.0;
  AngleMatrix(angles, in2);
  VectorRotate(in1, in2, out);
  v4 = 20;
  out[0] = out[0] + *Position;
  out[1] = out[1] + Position[1];
  out[2] = out[2] + Position[2];
  do
  {
    rand();
    rand();
    result = Particle_Spawn(1175, Position, (float *)(a2 + 28), Light, 0, 1.0, 0);
    --v4;
  }
  while ( v4 );
  return result;
}
#endif

// ── FUN_0046c7f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0046C7F0)
void __cdecl FUN_0046c7f0(int a1, float *a2, float a3, float a4, float a5)
{
  float *v5; // edi
  bool v6; // zf
  signed int v7; // eax
  bool v8; // zf
  signed int v9; // eax
  double v10; // st7
  bool v11; // zf
  signed int v12; // eax
  int v13; // eax
  float out[3]; // [esp+8h] [ebp-54h] BYREF
  float Light[3]; // [esp+14h] [ebp-48h] BYREF
  float in1[3]; // [esp+20h] [ebp-3Ch] BYREF
  float in2[3][4]; // [esp+2Ch] [ebp-30h] BYREF

  v5 = a2 + 7;
  AngleMatrix(a2 + 7, in2);
  in1[0] = a3;
  in1[1] = a4;
  in1[2] = a5;
  VectorRotate(in1, in2, out);
  out[0] = out[0] + a2[4];
  out[1] = out[1] + a2[5];
  out[2] = out[2] + a2[6];
  out[0] = (double)(rand() % 16 - 8) + out[0];
  out[1] = (double)(rand() % 16 - 8) + out[1];
  out[2] = (double)(rand() % 16 - 8) + out[2];
  if ( a1 )
  {
    if ( a1 == 1 )
    {
      v9 = rand() & 0x80000001;
      v8 = v9 == 0;
      if ( v9 < 0 )
      {
        v8 = (((BYTE)v9 - 1) | 0xFFFFFFFE) == -1;
      }
      if ( v8 )
      {
        Particle_Spawn(1220, out, v5, a2 + 58, 0, 1.0, 0);
      }
    }
    else if ( a1 == 2 )
    {
      v7 = rand() & 0x80000001;
      v6 = v7 == 0;
      if ( v7 < 0 )
      {
        v6 = (((BYTE)v7 - 1) | 0xFFFFFFFE) == -1;
      }
      if ( v6 )
      {
        Particle_Spawn(1220, out, v5, a2 + 58, 2, 1.0, 0);
      }
    }
  }
  else
  {
    v10 = (double)(rand() % 6 + 6) * 0.1;
    Light[0] = v10;
    Light[1] = v10 * 0.60000002;
    Light[2] = v10 * 0.40000001;
    v12 = rand() & 0x80000001;
    v11 = v12 == 0;
    if ( v12 < 0 )
    {
      v11 = (((BYTE)v12 - 1) | 0xFFFFFFFE) == -1;
    }
    if ( v11 )
    {
      v13 = rand() % 4;
      Particle_Spawn(1195, out, v5, Light, v13, 1.0, 0);
    }
    AddTerrainLight(out[0], out[1], Light, 4, PrimaryTerrainLight[0]);
  }
}
#endif

// ── FUN_004797b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004797B0)
FILE *__cdecl FUN_004797b0(char *FileName)
{
  FILE *result; // eax
  int v2; // esi

  result = fopen(FileName, "rb");
  SMDFile_0 = result;
  if ( result )
  {
    while ( 1 )
    {
      result = (FILE *)GetToken();
      if ( result == (FILE *)2 )
      {
        break;
      }
      if ( result == (FILE *)1 )
      {
        v2 = (__int64)TokenNumber;
        GetToken();
        strcpy(GlobalText[v2], TokenString);
      }
    }
  }
  return result;
}
#endif

// ── FUN_00479830 (IDA-only, gated) ──
#if defined(IDA_PORT_00479830)
int __cdecl FUN_00479830(char *FileName)
{
  FILE *v1; // ebp
  void *v3; // ebx
  char (*v4)[300]; // eax
  char *v5; // esi
  char (*v6)[300]; // edi
  CHAR Text[256]; // [esp+8h] [ebp-100h] BYREF

  v1 = fopen(FileName, "rb");
  if ( v1 )
  {
    v3 = (void *)operator_new(0x493E0u);
    fread(v3, 0x493E0u, 1u, v1);
    BuxConvert_0((BYTE *)v3, 300000);
    v4 = GlobalText;
    do
    {
      v5 = &(*v4)[(BYTE *)v3 - (BYTE *)GlobalText];
      v6 = v4++;
      qmemcpy(v6, v5, sizeof(char[300]));
    }
    while ( (int)v4 < (int)DAT_07d73104 );
    delete__(v3);
    return fclose(v1);
  }
  else
  {
    sprintf(Text, "%s file not found.\r\n", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    MessageBoxA(g_hWnd, Text, 0, 0);
    return SendMessageA(g_hWnd, 2u, 0, 0);
  }
}
#endif

// ── FUN_00479950 (IDA-only, gated) ──
#if defined(IDA_PORT_00479950)
int __cdecl FUN_00479950(FILE *Stream)
{
  char (*v1)[300]; // ebx
  char (*v2)[300]; // ebp
  FILE *Streama; // [esp+14h] [ebp+4h]

  Streama = fopen((const char *)Stream, aWb);
  v1 = (char (*)[300])operator_new(0x12Cu);
  v2 = GlobalText;
  do
  {
    qmemcpy(v1, v2, sizeof(char[300]));
    BuxConvert_0((BYTE *)v1, 300);
    FUN_005430f0(v1, 0x12Cu, 1u, Streama);
    ++v2;
  }
  while ( (int)v2 < (int)DAT_07d73104 );
  delete__(v1);
  return fclose(Streama);
}
#endif

// ── FUN_00479a50 (IDA-only, gated) ──
#if defined(IDA_PORT_00479A50)
int __cdecl FUN_00479a50(char *FileName)
{
  char *v1; // ebx
  char *v2; // ebp
  int v3; // ebp
  unsigned int i; // edx
  int v5; // esi
  FILE *Stream; // [esp+10h] [ebp-8h]
  int Buffer; // [esp+14h] [ebp-4h] BYREF
  char *FileNamea; // [esp+1Ch] [ebp+4h]

  Stream = fopen(FileName, aWb);
  FileNamea = (char *)operator_new(0x4E20u);
  v1 = FileNamea;
  v2 = DAT_07d73104;
  do
  {
    qmemcpy(v1, v2, 0x14u);
    BuxConvert_0((BYTE *)v1, 20);
    v2 += 20;
    v1 += 20;
  }
  while ( (int)v2 < (int)&DAT_07d77f24 );
  v3 = (int)&DAT_007cfa00;
  for ( i = 0; i <= 0x4E1C; i += 4 )
  {
    v5 = *(DWORD *)&FileNamea[i];
    if ( (((unsigned char)(i >> 2) - 1) & 1) != 0 )
    {
      if ( (((unsigned char)(i >> 2) - 1) & 1) == 1 )
      {
        v3 += v5;
      }
    }
    else
    {
      v3 ^= v5;
    }
    if ( (i & 0xF) == 0 )
    {
      v3 ^= (unsigned int)(v3 + 15997) >> (((i >> 2) & 7) + 1);
    }
  }
  Buffer = v3;
  FUN_005430f0(FileNamea, 0x4E20u, 1u, Stream);
  FUN_005430f0(&Buffer, 4u, 1u, Stream);
  delete__(FileNamea);
  return fclose(Stream);
}
#endif

// ── FUN_00479b30 (IDA-only, gated) ──
#if defined(IDA_PORT_00479B30)
void __cdecl FUN_00479b30(char *FileName)
{
  FILE *v1; // esi
  char *v2; // ebp
  unsigned int v3; // edi
  unsigned int i; // edx
  int v5; // esi
  char *v6; // ebx
  int v7; // [esp+8h] [ebp-10Ch]
  void *lpMem; // [esp+Ch] [ebp-108h]
  void *Buffer; // [esp+10h] [ebp-104h] BYREF
  CHAR Text[256]; // [esp+14h] [ebp-100h] BYREF

  v1 = fopen(FileName, "rb");
  if ( v1 )
  {
    v2 = (char *)operator_new(20000u);
    lpMem = v2;
    fread(v2, 20000u, 1u, v1);
    fread(&Buffer, 4u, 1u, v1);
    fclose(v1);
    v3 = (unsigned int)&DAT_007cfa00;
    for ( i = 0; i <= 0x4E1C; i += 4 )
    {
      v5 = *(DWORD *)&v2[i];
      if ( (((unsigned char)(i >> 2) - 1) & 1) != 0 )
      {
        if ( (((unsigned char)(i >> 2) - 1) & 1) == 1 )
        {
          v3 += v5;
        }
      }
      else
      {
        v3 ^= v5;
      }
      if ( (i & 0xF) == 0 )
      {
        v3 ^= (v3 + 15997) >> (((i >> 2) & 7) + 1);
      }
    }
    if ( Buffer == (void *)v3 )
    {
      v7 = 0;
      v6 = DAT_07d73104;
      while ( 1 )
      {
        BuxConvert_0((BYTE *)v2, 20);
        qmemcpy(v6, v2, 0x14u);
        if ( !*v6 )
        {
          break;
        }
        v2 += 20;
        v6 += 20;
        ++v7;
        if ( (int)v6 >= (int)&DAT_07d77f24 )
        {
          goto LABEL_18;
        }
      }
      DAT_07d78070 = v7;
LABEL_18:
      delete__(lpMem);
    }
    else
    {
      sprintf(Text, "%s - File corrupted.", FileName);
      CErrorReport::Write((DWORD)&g_ErrorReport, Text);
      MessageBoxA(g_hWnd, Text, 0, 0);
      SendMessageA(g_hWnd, 2u, 0, 0);
      delete__(v2);
    }
  }
  else
  {
    sprintf(Text, "%s - File not exist.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    MessageBoxA(g_hWnd, Text, 0, 0);
    SendMessageA(g_hWnd, 2u, 0, 0);
  }
}
#endif

// ── FUN_00479cf0 (IDA-only, gated) ──
#if defined(IDA_PORT_00479CF0)
FILE *__cdecl FUN_00479cf0(char *FileName)
{
  FILE *result; // eax
  int v2; // edx

  result = fopen(FileName, "rb");
  SMDFile_0 = result;
  if ( result )
  {
    while ( GetToken() != 2 )
    {
      v2 = DAT_07d78074;
      strcpy((char *)(20 * DAT_07d78074 + 131233296), TokenString);
      DAT_07d78074 = v2 + 1;
    }
    return (FILE *)fclose(SMDFile_0);
  }
  return result;
}
#endif

// ── FUN_00479d70 (IDA-only, gated) ──
#if defined(IDA_PORT_00479D70)
int __cdecl FUN_00479d70(char *FileName)
{
  char *v1; // ebx
  char *v2; // ebp
  int v3; // ebp
  unsigned int i; // edx
  int v5; // esi
  FILE *Stream; // [esp+10h] [ebp-8h]
  int Buffer; // [esp+14h] [ebp-4h] BYREF
  char *FileNamea; // [esp+1Ch] [ebp+4h]

  Stream = fopen(FileName, aWb);
  FileNamea = (char *)operator_new(0x4E20u);
  v1 = FileNamea;
  v2 = DAT_07d27610;
  do
  {
    qmemcpy(v1, v2, 0x14u);
    BuxConvert_0((BYTE *)v1, 20);
    v2 += 20;
    v1 += 20;
  }
  while ( (int)v2 < (int)&GlobalText[33][96] );
  v3 = (int)&DAT_00578200;
  for ( i = 0; i <= 0x4E1C; i += 4 )
  {
    v5 = *(DWORD *)&FileNamea[i];
    if ( (((unsigned char)(i >> 2) - 1) & 1) != 0 )
    {
      if ( (((unsigned char)(i >> 2) - 1) & 1) == 1 )
      {
        v3 += v5;
      }
    }
    else
    {
      v3 ^= v5;
    }
    if ( (i & 0xF) == 0 )
    {
      v3 ^= (unsigned int)(v3 + 11201) >> (((i >> 2) & 7) + 1);
    }
  }
  Buffer = v3;
  FUN_005430f0(FileNamea, 0x4E20u, 1u, Stream);
  FUN_005430f0(&Buffer, 4u, 1u, Stream);
  delete__(FileNamea);
  return fclose(Stream);
}
#endif

// ── FUN_00479e50 (IDA-only, gated) ──
#if defined(IDA_PORT_00479E50)
void __cdecl FUN_00479e50(char *FileName)
{
  FILE *v1; // esi
  char *v2; // ebp
  unsigned int v3; // edi
  unsigned int i; // edx
  int v5; // esi
  char *v6; // ebx
  int v7; // [esp+8h] [ebp-10Ch]
  void *lpMem; // [esp+Ch] [ebp-108h]
  void *Buffer; // [esp+10h] [ebp-104h] BYREF
  CHAR Text[256]; // [esp+14h] [ebp-100h] BYREF

  v1 = fopen(FileName, "rb");
  if ( v1 )
  {
    v2 = (char *)operator_new(0x4E20u);
    lpMem = v2;
    fread(v2, 0x4E20u, 1u, v1);
    fread(&Buffer, 4u, 1u, v1);
    fclose(v1);
    v3 = (unsigned int)&DAT_00578200;
    for ( i = 0; i <= 0x4E1C; i += 4 )
    {
      v5 = *(DWORD *)&v2[i];
      if ( (((unsigned char)(i >> 2) - 1) & 1) != 0 )
      {
        if ( (((unsigned char)(i >> 2) - 1) & 1) == 1 )
        {
          v3 += v5;
        }
      }
      else
      {
        v3 ^= v5;
      }
      if ( (i & 0xF) == 0 )
      {
        v3 ^= (v3 + 11201) >> (((i >> 2) & 7) + 1);
      }
    }
    if ( Buffer == (void *)v3 )
    {
      v7 = 0;
      v6 = DAT_07d27610;
      while ( 1 )
      {
        BuxConvert_0((BYTE *)v2, 20);
        qmemcpy(v6, v2, 0x14u);
        if ( !*v6 )
        {
          break;
        }
        v2 += 20;
        v6 += 20;
        ++v7;
        if ( (int)v6 >= (int)&GlobalText[33][96] )
        {
          goto LABEL_18;
        }
      }
      DAT_07d78074 = v7;
LABEL_18:
      delete__(lpMem);
    }
    else
    {
      sprintf(Text, "%s - File corrupted.", FileName);
      CErrorReport::Write((DWORD)&g_ErrorReport, Text);
      MessageBoxA(g_hWnd, Text, 0, 0);
      SendMessageA(g_hWnd, 2u, 0, 0);
      delete__(v2);
    }
  }
  else
  {
    sprintf(Text, "%s - File not exist.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    MessageBoxA(g_hWnd, Text, 0, 0);
    SendMessageA(g_hWnd, 2u, 0, 0);
  }
}
#endif

// ── FUN_0047cfb0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047CFB0)
short __cdecl FUN_0047cfb0(WORD *Value)
{
  WORD *v1; // ecx
  DWORD v3; // [esp-4h] [ebp-4h]

  if ( *Value == 0xFFFF )
  {
    return 0;
  }
  (WORD)(v1) = Value[9];
  v3 = (DWORD)Value;
  Value = v1;
  PlusSpecial((WORD *)&Value, 63, v3);
  return (short)Value;
}
#endif

// ── FUN_0047cfe0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047CFE0)
short __cdecl FUN_0047cfe0(WORD *a1)
{
  if ( *a1 == 0xFFFF )
  {
    return 0;
  }
  else
  {
    return a1[10];
  }
}
#endif

// ── FUN_0047d000 (IDA-only, gated) ──
#if defined(IDA_PORT_0047D000)
short __cdecl FUN_0047d000(WORD *a1)
{
  if ( *a1 == 0xFFFF )
  {
    return 0;
  }
  else
  {
    return a1[12];
  }
}
#endif

// ── FUN_0047e350 (IDA-only, gated) ──
#if defined(IDA_PORT_0047E350)
int __cdecl FUN_0047e350(int _this)
{
  unsigned short v1; // di
  int result; // eax
  int v3; // esi

  v1 = *(WORD *)(_this + 14);
  result = v1 * v1 * (v1 + 9);
  v3 = 10 * result;
  *(DWORD *)(_this + 52) = 10 * result;
  if ( v1 > 0xFFu )
  {
    result = 125 * (v1 - 255) * (v1 - 255) * (v1 - 255 + 9);
    *(DWORD *)(_this + 52) = v3 + 1000 * (v1 - 255) * (v1 - 255) * (v1 - 255 + 9);
  }
  return result;
}
#endif

// ── FUN_0047e4f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047E4F0)
void __cdecl CHARACTER_MACHINE::GetMagicSkillDamage(DWORD This, int iType, int *piMinDamage, int *piMaxDamage)
{
  int v4; // ebx
  int v5; // esi
  unsigned int v6; // eax
  bool v7; // cf
  unsigned int v8; // eax
  BYTE *v9; // esi
  unsigned char v10; // al
  void *v11; // ebp
  unsigned int v12; // ecx
  int v13; // esi
  char v14; // al
  int v15; // eax
  DWORD v16; // esi
  BYTE *v17; // eax
  int *v18; // ebx
  unsigned int v19; // eax
  unsigned int v20; // eax
  int v21; // eax
  char v22; // cl
  int v23; // [esp+10h] [ebp-10h]
  int v24; // [esp+14h] [ebp-Ch] BYREF
  BYTE *v25; // [esp+18h] [ebp-8h] BYREF
  DWORD v26; // [esp+1Ch] [ebp-4h]
  BYTE *iTypea; // [esp+24h] [ebp+4h]
  int iTypeb; // [esp+24h] [ebp+4h]

  v4 = 40 * iType;
  v26 = This;
  iTypea = &SkillAttribute[40 * iType];
  v5 = (int)iTypea;
  v25 = iTypea;
  v6 = (*(int (__cdecl **)(int *, BYTE *))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, iTypea);
  v24 = 0;
  v23 = 0;
  if...
  v16 = v26;
  v17 = &SkillAttribute[v4 + 33];
  *piMinDamage = *v17 + *(unsigned short *)(v26 + 70);
  *piMaxDamage = (*v17 >> 1) + *v17 + *(unsigned short *)(v16 + 72);
  v18 = (int *)&SkillAttribute[v4];
  piMaxDamage = v18;
  v19 = (*(int (__cdecl **)(int *, int *))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, v18);
  piMinDamage = 0;
  iTypeb = 0;
  if...
}
#endif

// ── FUN_0047f360 (IDA-only, gated) ──
#if defined(IDA_PORT_0047F360)
int __cdecl FUN_0047f360(int a1, int a2, LPCSTR a3, int a4, int a5, int x, int a7, int a8, LPCSTR lpString)
{
  int v9; // esi
  int v10; // edi
  bool v11; // zf
  int v12; // eax
  __int64 Height; // rax
  BYTE *Buffer; // ecx
  char *v15; // ebp
  LONG v16; // eax
  BYTE *i; // esi
  DWORD v18; // edx
  struct tagSIZE sz; // [esp+10h] [ebp-8h] BYREF
  int v21; // [esp+1Ch] [ebp+4h]
  BYTE *v22; // [esp+20h] [ebp+8h]

  v9 = a2;
  v10 = a1;
  v11 = *a3 == 10;
  sz.cx = a1;
  sz.cy = a2;
  if ( !v11 )
  {
    if ( lpString )
    {
      v12 = lstrlenA(lpString);
      GetTextExtentPointA(m_hFontDC, lpString, v12, &sz);
      TextOutA(m_hFontDC, x, 0, lpString, strlen(lpString));
    }
    else
    {
      sz.cx = 0;
    }
    SetTextColor(m_hFontDC, (COLORREF)&DAT_00ffffff);
    TextOutA(m_hFontDC, x + sz.cx, 0, a3, strlen(a3));
    v9 = a2;
    v10 = a1;
  }
  if ( !a8 )
  {
    a8 = v10;
  }
  Height = (__int64)Bitmaps[0].Height;
  if ( v9 > (int)Height )
  {
    v9 = (__int64)Bitmaps[0].Height;
  }
  if ( v9 > 0 )
  {
    Buffer = Bitmaps[0].Buffer;
    v15 = (char *)ppvBits;
    v22 = Bitmaps[0].Buffer;
    v21 = v9;
    do
    {
      v16 = 0;
      for ( i = v15; v16 < v10; ++v16 )
      {
        if ( *i )
        {
          if ( v16 >= sz.cx )
          {
            v18 = m_dwTextColor;
          }
          else
          {
            v18 = SetTextColor_0;
          }
        }
        else
        {
          v18 = v16 >= a8 ? 0 : m_dwBackColor;
        }
        *(DWORD *)Buffer = v18;
        i += 3;
        Buffer += 4;
      }
      v15 += 1536;
      Buffer = v22 + 1024;
      LODWORD(Height) = v21 - 1;
      v11 = v21 == 1;
      v22 += 1024;
      --v21;
    }
    while ( !v11 );
  }
  return Height;
}
#endif

// ── FUN_0047fed0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047FED0)
char __cdecl FUN_0047fed0(int a1, const char *a2)
{
  void *v2; // ebx
  unsigned int v3; // eax
  bool v4; // cf
  int v5; // eax
  const void *v6; // ebx
  unsigned int v7; // edx
  unsigned int v8; // eax
  BYTE *v9; // esi
  unsigned char v10; // al
  void *v11; // ebp
  unsigned int v12; // ecx
  int v13; // esi
  unsigned int v14; // eax
  BYTE *v15; // eax
  char v16; // cl
  char (*v18)[10]; // edi
  int v19; // [esp+10h] [ebp-10h]
  int v20; // [esp+10h] [ebp-10h]
  DWORD v21; // [esp+14h] [ebp-Ch] BYREF
  DWORD v22; // [esp+18h] [ebp-8h] BYREF
  int v23; // [esp+1Ch] [ebp-4h]

  v2 = (void *)CharacterMachine;
  v22 = CharacterMachine;
  v3 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v21 = 0;
  v19 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v3), 4) )
    {
      if ( !memcmp((const char *)&v22, (const char *)(DAT_055c9bd0 + 4 * v3), 4) )
      {
        if ( v3 == -1 )
        {
          break;
        }
        v8 = FUN_004041e0(&DAT_055c9bc8, (int)v2);
        if ( v8 == -1 )
        {
          v9 = 0;
        }
        else
        {
          v9 = *(BYTE **)(DAT_055c9bcc + 4 * v8);
        }
        v10 = v9[1412] + 1;
        v9[1412] = v10;
        if ( v10 < 2u )
        {
          v11 = (void *)operator_new(0x584u);
          qmemcpy(v11, v9, 0x584u);
          v12 = 1411;
          v13 = 1412;
          do
          {
            if ( v12 < 0x583 )
            {
              *((BYTE *)v11 + v12) ^= *((BYTE *)v11 + v12 + 1);
            }
            *((BYTE *)v11 + v12) = ((*((BYTE *)v11 + v12) - 35) ^ DAT_00559050[(int)v12 % 16]) - 71;
            --v12;
            --v13;
          }
          while ( v13 );
          qmemcpy(v2, v11, 0x584u);
          delete__(v11);
        }
        goto LABEL_7;
      }
      v4 = ++v19 < (unsigned int)DAT_055c9bd4;
      v3 = (v3 + 1) % DAT_055c9bd4;
      if ( !v4 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v5 = operator_new(0x585u);
  *(BYTE *)(v5 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v5, (int)v2);
LABEL_7:
  v6 = (const void *)CharacterMachine;
  v23 = *(unsigned short *)(CharacterAttribute + 14);
  v21 = CharacterMachine;
  v7 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v22 = 0;
  v20 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v22, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
    {
      if ( !memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
      {
        if ( v7 != -1 )
        {
          v14 = FUN_004041e0(&DAT_055c9bc8, (int)v6);
          if ( v14 == -1 )
          {
            v15 = 0;
          }
          else
          {
            v15 = *(BYTE **)(DAT_055c9bcc + 4 * v14);
          }
          v16 = v15[1412] - 1;
          v15[1412] = v16;
          if ( !v16 )
          {
            FUN_00404400(v15, v6);
          }
        }
        break;
      }
      v7 = (v7 + 1) % DAT_055c9bd4;
      if ( ++v20 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_11;
      }
    }
  }
  else
  {
LABEL_11:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  if ( v23 < a1 )
  {
    v18 = WhisperRegistID;
    while ( strcmp(a2, (const char *)v18) )
    {
      if ( (int)++v18 >= (int)WhisperRegistID[10] )
      {
        UIChatLogWindow_AddText(DAT_07e11dd4, GlobalText[479], 1);
        return 0;
      }
    }
  }
  return 1;
}
#endif

// ── FUN_00481a40 (IDA-only, gated) ──
#if defined(IDA_PORT_00481A40)
void __cdecl FUN_00481a40(int x, int y, DWORD c)
{
  double v3; // st7
  int v4; // eax
  LONG v5; // edi
  LONG v6; // ecx
  int j; // eax
  int v9; // esi
  char *v10; // edx
  char *v11; // edi
  char v12; // al
  int v13; // ecx
  char v14; // bl
  int v15; // ecx
  DWORD v16; // eax
  float v17; // edi
  char v18; // al
  unsigned char v19; // bl
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // eax
  int v24; // eax
  int v25; // ebx
  int cx; // esi
  LONG v27; // ecx
  int i; // eax
  int v29; // eax
  char *v30; // edx
  unsigned int v31; // esi
  char *v32; // edi
  char v33; // al
  DWORD v34; // edi
  int v35; // ecx
  int v36; // ecx
  char v37; // al
  int v38; // eax
  int v39; // eax
  int v40; // esi
  char *v41; // edx
  unsigned int v42; // ecx
  char *v43; // edi
  int v44; // eax
  int v45; // ecx
  int v46; // eax
  char *v47; // edx
  char *v48; // edi
  int v49; // eax
  int v50; // esi
  int v51; // eax
  char *v52; // edx
  char *v53; // edi
  const char *v54; // [esp-8h] [ebp-140h]
  GLfloat red; // [esp+0h] [ebp-138h]
  GLfloat reda; // [esp+0h] [ebp-138h]
  GLfloat green; // [esp+4h] [ebp-134h]
  GLfloat greena; // [esp+4h] [ebp-134h]
  float greenb; // [esp+4h] [ebp-134h]
  GLfloat blue; // [esp+8h] [ebp-130h]
  float bluea; // [esp+8h] [ebp-130h]
  GLfloat blueb; // [esp+8h] [ebp-130h]
  float bluec; // [esp+8h] [ebp-130h]
  GLfloat alpha; // [esp+Ch] [ebp-12Ch]
  float alphaa; // [esp+Ch] [ebp-12Ch]
  float alphab; // [esp+Ch] [ebp-12Ch]
  int ya; // [esp+20h] [ebp-118h]
  int yd; // [esp+20h] [ebp-118h]
  int ye; // [esp+20h] [ebp-118h]
  int yf; // [esp+20h] [ebp-118h]
  float yb; // [esp+20h] [ebp-118h]
  int yc; // [esp+20h] [ebp-118h]
  unsigned int v73; // [esp+24h] [ebp-114h]
  unsigned char v74; // [esp+24h] [ebp-114h]
  unsigned char v75; // [esp+24h] [ebp-114h]
  int v76; // [esp+24h] [ebp-114h]
  int v77; // [esp+24h] [ebp-114h]
  unsigned int v78; // [esp+24h] [ebp-114h]
  unsigned int v79; // [esp+24h] [ebp-114h]
  int a2; // [esp+28h] [ebp-110h]
  int a2b; // [esp+28h] [ebp-110h]
  int a2a; // [esp+28h] [ebp-110h]
  unsigned char v83; // [esp+2Ch] [ebp-10Ch] BYREF
  short v84; // [esp+2Dh] [ebp-10Bh]
  int v85; // [esp+30h] [ebp-108h]
  float xa; // [esp+34h] [ebp-104h]
  char a4[253]; // [esp+38h] [ebp-100h] BYREF
  short v88; // [esp+135h] [ebp-3h]
  char v89; // [esp+137h] [ebp-1h]
  int v90; // [esp+13Ch] [ebp+4h]
  int a3; // [esp+140h] [ebp+8h]

  if ( *(DWORD *)(g_pRenderText + 8) != 1 )
  {
    EnableAlphaTest(1);
    glColor3f(1.0, 1.0, 1.0);
    v25 = 1;
    cx = *(DWORD *)(c + 576);
    v27 = *(DWORD *)(c + 580);
    TextSize.cx = cx;
    TextSize.cy = v27;
    do
    {
      if ( v25 >= cx )
      {
        break;
      }
      v25 *= 2;
    }
    while ( v25 < 256 );
    for ( i = 1; i < 256; i *= 2 )
    {
      if ( i >= v27 )
      {
        break;
      }
    }
    yc = i;
    switch ( *(BYTE *)(c + 36) )
    {
      case 0:
        m_dwTextColor = -983146;
        break;
      case 1:
        m_dwTextColor = -34716;
        break;
      case 2:
        m_dwTextColor = -19316;
        break;
      case 3:
        m_dwTextColor = -9016;
        break;
      case 4:
        m_dwTextColor = -12806401;
        break;
      case 5:
        m_dwTextColor = -14790401;
        break;
      default:
        m_dwTextColor = -16776961;
        break;
    }
    DAT_07e11d6e = 1;
    v90 = (int)(x * WindowWidth) / 640;
    v29 = FontHeight;
    a3 = (int)(y * WindowHeight) / 480;
    if ( FontHeight > 32 )
    {
      v29 = 32;
      FontHeight = 32;
    }
    if ( v29 > 0 )
    {
      v30 = (char *)ppvBits;
      v76 = v29;
      while ( 1 )
      {
        if ( cx > 512 )
        {
          cx = 512;
          TextSize.cx = 512;
        }
        v31 = 3 * cx;
        memset(v30, 0, 4 * (v31 >> 2));
        v32 = &v30[4 * (v31 >> 2)];
        v30 += 1536;
        memset(v32, 0, v31 & 3);
        if ( !--v76 )
        {
          break;
        }
        cx = TextSize.cx;
      }
    }
    v33 = *(BYTE *)(c + 37);
    if ( v33 )
    {
      if ( v33 == 1 )
      {
        v34 = -1778359236;
        SetTextColor_0 = -16711736;
        m_dwBackColor = -1778359236;
      }
      else
      {
        v34 = -1778384796;
        SetTextColor_0 = -16776961;
        m_dwBackColor = -1778384796;
      }
    }
    else
    {
      v34 = -1773129196;
      SetTextColor_0 = -14116;
      m_dwBackColor = -1773129196;
    }
    v35 = *(DWORD *)(c + 568);
    if ( v35 <= MouseX && MouseX < (int)(v35 + 640 * *(DWORD *)(c + 576) / WindowWidth) )
    {
      v36 = *(DWORD *)(c + 572);
      if ( v36 <= MouseY
        && MouseY < (int)(v36 + 480 * *(DWORD *)(c + 580) / WindowHeight)
        && InputEnable
        && *(BYTE *)(Hero + 846)
        && strcmp((const char *)c, (const char *)(Hero + 449))
        && DAT_07e11da8 % 6 < 3 )
      {
        m_dwBackColor = m_dwTextColor;
        m_dwTextColor = v34;
      }
    }
    FUN_0047f360(TextSize.cx, FontHeight, (LPCSTR)c, v25, 0, 0, 0, 0, (LPCSTR)(c + 24));
    FUN_0047f4c0(v90, a3, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
    v37 = *(BYTE *)(c + 37);
    if ( v37 )
    {
      m_dwBackColor = v37 != 1 ? -1778384846 : -1778372066;
    }
    else
    {
      m_dwBackColor = -1775100406;
    }
    v38 = *(DWORD *)(c + 560);
    if ( v38 <= 0 )
    {
      v49 = *(DWORD *)(c + 556);
      if ( v49 > 0 )
      {
        m_dwTextColor = -3613466;
        if ( v49 < 10 )
        {
          m_dwTextColor = -2134319898;
        }
        v50 = FontHeight;
        v51 = TextSize.cx;
        if ( FontHeight > 0 )
        {
          v79 = 3 * TextSize.cx;
          v52 = (char *)ppvBits;
          do
          {
            memset(v52, 0, 4 * (v79 >> 2));
            v53 = &v52[4 * (v79 >> 2)];
            v52 += 1536;
            --v50;
            memset(v53, 0, v79 & 3);
          }
          while ( v50 );
          v50 = FontHeight;
          v51 = TextSize.cx;
        }
        FUN_0047f360(v51, v50, (LPCSTR)(c + 44), v25, 0, 0, 0, 0, 0);
        FUN_0047f4c0(v90, a3 + FontHeight, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
      }
    }
    else
    {
      m_dwTextColor = -3613466;
      if ( v38 < 10 )
      {
        m_dwTextColor = -2134319898;
      }
      v39 = FontHeight;
      v40 = TextSize.cx;
      if ( FontHeight > 0 )
      {
        v41 = (char *)ppvBits;
        v77 = FontHeight;
        do
        {
          v42 = (unsigned int)(3 * v40) >> 2;
          memset(v41, 0, 4 * v42);
          v43 = &v41[4 * v42];
          v41 += 1536;
          memset(v43, 0, (3 * (BYTE)v40) & 3);
          --v77;
        }
        while ( v77 );
        v39 = FontHeight;
      }
      FUN_0047f360(v40, v39, (LPCSTR)(c + 300), v25, 0, 0, 0, 0, 0);
      FUN_0047f4c0(v90, a3 + FontHeight, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
      v44 = *(DWORD *)(c + 556);
      m_dwTextColor = -3613466;
      if ( v44 < 10 )
      {
        m_dwTextColor = -2134319898;
      }
      v45 = FontHeight;
      v46 = TextSize.cx;
      if ( FontHeight > 0 )
      {
        a2a = FontHeight;
        v78 = 3 * TextSize.cx;
        v47 = (char *)ppvBits;
        do
        {
          memset(v47, 0, 4 * (v78 >> 2));
          v48 = &v47[4 * (v78 >> 2)];
          v47 += 1536;
          memset(v48, 0, v78 & 3);
          --a2a;
        }
        while ( a2a );
        v46 = TextSize.cx;
        v45 = FontHeight;
      }
      FUN_0047f360(v46, v45, (LPCSTR)(c + 44), v25, 0, 0, 0, 0, 0);
      FUN_0047f4c0(v90, a3 + 2 * FontHeight, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
    }
    return;
  }
  EnableAlphaTest(1);
  glColor3f(1.0, 1.0, 1.0);
  v4 = 1;
  v5 = *(DWORD *)(c + 576);
  v6 = *(DWORD *)(c + 580);
  TextSize.cx = v5;
  TextSize.cy = v6;
  do
  {
    if ( v4 >= v5 )
    {
      break;
    }
    v4 *= 2;
  }
  while ( v4 < 256 );
  for ( j = 1; j < 256; j *= 2 )
  {
    if ( j >= v6 )
    {
      break;
    }
  }
  switch ( *(BYTE *)(c + 36) )
  {
    case 0:
      m_dwTextColor = -983146;
      break;
    case 1:
      m_dwTextColor = -34716;
      break;
    case 2:
      m_dwTextColor = -19316;
      break;
    case 3:
      m_dwTextColor = -9016;
      break;
    case 4:
      m_dwTextColor = -12806401;
      break;
    case 5:
      m_dwTextColor = -14790401;
      break;
    default:
      m_dwTextColor = -16776961;
      break;
  }
  a2 = x;
  if ( x < 0 )
  {
    a2 = 0;
  }
  __asm
  {
    fild    [esp+128h+a2]
    fmul    g_fScreenRate_x
    fiadd   TextSize._cx
    fild    WindowWidth
    fcompp
    fnstsw  ax
  }
  if ( (_AX & 0x100) != 0 )
  {
    a2b = WindowWidth - v5;
    __asm
    {
      fild    [esp+128h+a2]
      fdiv    g_fScreenRate_x
    }
    a2 = _ftol(v3);
  }
  v9 = FontHeight;
  if ( FontHeight > 0 )
  {
    v73 = 3 * v5;
    v10 = (char *)ppvBits;
    do
    {
      memset(v10, 0, 4 * (v73 >> 2));
      v11 = &v10[4 * (v73 >> 2)];
      v10 += 768;
      --v9;
      memset(v11, 0, v73 & 3);
    }
    while ( v9 );
  }
  v12 = *(BYTE *)(c + 37);
  (BYTE)(ya) = -106;
  if ( v12 )
  {
    if ( v12 == 1 )
    {
      v74 = 60;
      (BYTE)(v85) = 100;
    }
    else
    {
      v74 = 100;
      (BYTE)(v85) = 0;
    }
    v83 = 0;
  }
  else
  {
    v74 = 20;
    (BYTE)(v85) = 50;
    v83 = 80;
  }
  v13 = *(DWORD *)(c + 568);
  v14 = 0;
  m_dwBackColor = 0;
  if ( v13 <= MouseX && MouseX < (int)(v13 + 640 * *(DWORD *)(c + 576) / WindowWidth) )
  {
    v15 = *(DWORD *)(c + 572);
    if ( v15 <= MouseY
      && MouseY < (int)(v15 + 480 * *(DWORD *)(c + 580) / WindowHeight)
      && InputEnable
      && *(BYTE *)(Hero + 846)
      && strcmp((const char *)c, (const char *)(Hero + 449))
      && DAT_07e11da8 % 6 < 3 )
    {
      v16 = m_dwTextColor;
      m_dwTextColor = -16777216;
      v74 = v16;
      v14 = 3;
      (BYTE)(v85) = ((BYTE)((v16) >> 8));
      v83 = ((BYTE)((v16) >> 16));
      (BYTE)(ya) = (BYTE)((v16) >> 8);
    }
  }
  ya = (unsigned char)ya;
  __asm { fild    [esp+12Ch+y] }
  yd = v83;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+12Ch+alpha]; alpha
    fild    [esp+12Ch+y]
    fmul    ds:DAT_00552b70
  }
  ye = (unsigned char)v85;
  __asm
  {
    fstp    [esp+130h+blue]; blue
    fild    [esp+130h+y]
  }
  yf = v74;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+134h+green]; green
    fild    [esp+134h+y]
    fmul    ds:DAT_00552b70
    fstp    [esp+138h+red]; red
  }
  glColor4f(red, green, blue, alpha);
  __asm
  {
    fild    [esp+128h+a3]
    fstp    [esp+12Ch+y]
    fild    [esp+12Ch+a2]
    fstp    [esp+12Ch+x]
    fild    FontHeight
  }
  v17 = xa;
  __asm
  {
    fdiv    g_fScreenRate_y
    fstp    [esp+12Ch+alpha]; Height
    fild    TextSize._cx
    fdiv    g_fScreenRate_x
    fstp    [esp+130h+blue]; Width
  }
  RenderColor(xa, yb, bluea, alphaa);
  v18 = *(BYTE *)(c + 37);
  if ( v18 )
  {
    if ( v18 == 1 )
    {
      v75 = 30;
      (BYTE)(v85) = 50;
    }
    else
    {
      v75 = 50;
      (BYTE)(v85) = 0;
    }
    v83 = 0;
  }
  else
  {
    v75 = 10;
    (BYTE)(v85) = 30;
    v83 = 50;
  }
  LODWORD(xa) = v83;
  __asm { fild    [esp+130h+x] }
  LODWORD(xa) = (unsigned char)v85;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+130h+blue]; blue
    fild    [esp+130h+x]
  }
  LODWORD(xa) = v75;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+134h+green]; green
    fild    [esp+134h+x]
    fmul    ds:DAT_00552b70
    fstp    [esp+138h+red]; red
  }
  glColor4f(reda, greena, blueb, 0.58823532);
  if ( *(int *)(c + 560) > 0 )
  {
    __asm
    {
      fild    FontHeight
      fld     st
      fadd    st, st
    }
LABEL_48:
    __asm
    {
      fdiv    g_fScreenRate_y
      fstp    [esp+12Ch+alpha]; Height
      fild    TextSize._cx
      fdiv    g_fScreenRate_x
      fstp    [esp+130h+blue]; Width
      fdiv    g_fScreenRate_x
      fadd    [esp+134h+y]
      fstp    [esp+134h+green]; y
    }
    RenderColor(v17, greenb, bluec, alphab);
    goto LABEL_49;
  }
  if ( *(int *)(c + 556) > 0 )
  {
    __asm
    {
      fild    FontHeight
      fld     st
    }
    goto LABEL_48;
  }
LABEL_49:
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glEnable(0xDE1u);
  a4[0] = DAT_07e11de0;
  memset(&a4[1], 0, 0xFCu);
  v88 = 0;
  v89 = 0;
  if ( *(BYTE *)(c + 24) )
  {
    v19 = *(BYTE *)(c + 37) + v14 - 14;
    v83 = 2;
    v84 = v19;
    strcat(a4, (const char *)&v83);
    (BYTE)(v84) = -16;
    strcat(a4, (const char *)(c + 24));
    strcat(a4, (const char *)&v83);
  }
  strcat(a4, (const char *)c);
  CUIRenderText::RenderText(g_pRenderText, a2, y, a4, 0, 0, 1, 0, 640);
  v20 = *(DWORD *)(c + 560);
  __asm { fstp    st }
  if ( v20 > 0 )
  {
    m_dwTextColor = -3613466;
    if ( v20 < 10 )
    {
      m_dwTextColor = -2134319898;
    }
    __asm
    {
      fild    FontHeight
      fdiv    g_fScreenRate_y
      fadd    [esp+140h+y]
      fadd    ds:DAT_00552504
    }
    v21 = _ftol(v3);
    CUIRenderText::RenderText(g_pRenderText, a2, v21, (const char *)(c + 300), 0, 0, 1, 0, 640);
    v22 = *(DWORD *)(c + 556);
    m_dwTextColor = -3613466;
    __asm { fstp    st }
    if ( v22 < 10 )
    {
      m_dwTextColor = -2134319898;
    }
    __asm
    {
      fild    FontHeight
      fdiv    g_fScreenRate_y
    }
    v54 = (const char *)(c + 44);
    __asm { fadd    st, st }
LABEL_57:
    __asm
    {
      fadd    [esp+140h+y]
      fadd    ds:DAT_00552504
    }
    v23 = _ftol(v3);
    CUIRenderText::RenderText(g_pRenderText, a2, v23, v54, 0, 0, 1, 0, 640);
    __asm { fstp    st }
    return;
  }
  v24 = *(DWORD *)(c + 556);
  if ( v24 > 0 )
  {
    m_dwTextColor = -3613466;
    if ( v24 < 10 )
    {
      m_dwTextColor = -2134319898;
    }
    __asm
    {
      fild    FontHeight
      fdiv    g_fScreenRate_y
    }
    v54 = (const char *)(c + 44);
    goto LABEL_57;
  }
}
#endif

// ── FUN_00482350 (IDA-only, gated) ──
#if defined(IDA_PORT_00482350)
void __cdecl FUN_00482350()
{
  const char *v0; // ebp
  int v1; // eax
  int v2; // eax
  BYTE *v3; // eax
  int v4; // ecx
  int v5; // ecx

  v0 = (const char *)DAT_07e01720;
  do
  {
    if ( *(int *)v0 > 0 )
    {
      --*(DWORD *)v0;
    }
    v1 = *((DWORD *)v0 + 129);
    if ( v1 > 0 )
    {
      *((DWORD *)v0 + 129) = v1 - 1;
    }
    v2 = *((DWORD *)v0 + 130);
    if ( v2 > 0 )
    {
      *((DWORD *)v0 + 130) = v2 - 1;
    }
    v3 = (BYTE *)*((DWORD *)v0 + 131);
    if ( v3 && (!*v3 || !v3[352]) )
    {
      *(DWORD *)v0 = 0;
      *((DWORD *)v0 + 129) = 0;
      *((DWORD *)v0 + 130) = 0;
    }
    v4 = *((DWORD *)v0 + 132);
    if ( v4 <= MouseX && MouseX < (int)(v4 + 640 * *((DWORD *)v0 + 134) / WindowWidth) )
    {
      v5 = *((DWORD *)v0 + 133);
      if ( v5 <= MouseY
        && MouseY < (int)(v5 + 480 * *((DWORD *)v0 + 135) / WindowHeight)
        && InputEnable
        && *(BYTE *)(Hero + 846)
        && strcmp(v0 - 40, (const char *)(Hero + 449))
        && MouseRButtonPush )
      {
        strcpy(InputText[1], v0 - 40);
        MouseRButtonPush = 0;
        InputLength[1] = strlen(InputText[1]);
        PlayBuffer(25, 0, 0);
      }
    }
    v0 += 596;
  }
  while ( (int)v0 < (int)&DAT_07e0fff0 );
}
#endif

// ── FUN_004824c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004824C0)
int FUN_004824c0()
{
  void *v0; // ebp
  unsigned int v1; // eax
  bool v2; // cf
  BYTE *v3; // eax
  unsigned char v4; // cl
  const void *v5; // ebp
  int v6; // esi
  unsigned int v7; // eax
  int v8; // eax
  unsigned int v9; // eax
  BYTE *v10; // eax
  char v11; // cl
  int v12; // ebx
  int v13; // edi
  int *v14; // esi
  int v15; // ecx
  int result; // eax
  int *v17; // edx
  const void *v18; // ebp
  unsigned int v19; // eax
  unsigned int v20; // eax
  BYTE *v21; // eax
  char v22; // cl
  int v23; // [esp+10h] [ebp-14h]
  int v24; // [esp+10h] [ebp-14h]
  int v25; // [esp+14h] [ebp-10h] BYREF
  DWORD v26; // [esp+18h] [ebp-Ch] BYREF
  DWORD v27; // [esp+1Ch] [ebp-8h] BYREF
  int v28; // [esp+20h] [ebp-4h] BYREF

  v0 = (void *)CharacterMachine;
  v26 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v25 = 0;
  v23 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v25, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v3 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v0);
        v4 = v3[1412] + 1;
        v3[1412] = v4;
        if ( v4 < 2u )
        {
          FUN_00404370(v0, v3);
        }
        goto LABEL_9;
      }
      v2 = ++v23 < (unsigned int)DAT_055c9bd4;
      v1 = (v1 + 1) % DAT_055c9bd4;
      if ( !v2 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v8 = operator_new(0x585u);
  *(BYTE *)(v8 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v8, (int)v0);
LABEL_9:
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 )
  {
    v5 = (const void *)CharacterMachine;
    v6 = *(short *)(CharacterMachine + 536);
    v28 = *(short *)(CharacterMachine + 604);
    v24 = v6;
    v27 = CharacterMachine;
    v7 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
        {
          if ( v7 != -1 )
          {
            v9 = FUN_004041e0(&DAT_055c9bc8, (int)v5);
            v10 = v9 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v9);
            v11 = v10[1412] - 1;
            v10[1412] = v11;
            if ( !v11 )
            {
              FUN_00404400(v10, v5);
            }
          }
          break;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v7 = (v7 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          v6 = v24;
          goto LABEL_15;
        }
      }
      v6 = v24;
    }
    else
    {
LABEL_15:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v28 >= 128 && v28 < 135 || v28 == 145 )
    {
      v12 = 143;
    }
    else if ( (v6 < 136 || v6 >= 143) && (v6 < 144 || v6 >= 160) )
    {
      v12 = v28;
    }
    else
    {
      v12 = 135;
    }
    v13 = 7;
    v14 = (int *)&DAT_07ea9504;
LABEL_35:
    v15 = 7;
    result = v13 + 56;
    v17 = v14;
    while ( *((short *)v17 - 28) != v12 || *v17 <= 0 )
    {
      --v15;
      v17 -= 136;
      result -= 8;
      if ( v15 < 0 )
      {
        v14 -= 17;
        --v13;
        if ( (int)v14 >= (int)&DAT_07ea9328 )
        {
          goto LABEL_35;
        }
        return -1;
      }
    }
  }
  else
  {
    v18 = (const void *)CharacterMachine;
    v27 = CharacterMachine;
    v19 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v28 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
        {
          if ( v19 != -1 )
          {
            v20 = FUN_004041e0(&DAT_055c9bc8, (int)v18);
            v21 = v20 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v20);
            v22 = v21[1412] - 1;
            v21[1412] = v22;
            if ( !v22 )
            {
              FUN_00404400(v21, v18);
            }
          }
          return -1;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v19 = (v19 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          goto LABEL_45;
        }
      }
      return -1;
    }
    else
    {
LABEL_45:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      return -1;
    }
  }
  return result;
}
#endif

// ── FUN_004827a0 (IDA-only, gated) ──
#if defined(IDA_PORT_004827A0)
int FUN_004824c0()
{
  void *v0; // ebp
  unsigned int v1; // eax
  bool v2; // cf
  BYTE *v3; // eax
  unsigned char v4; // cl
  const void *v5; // ebp
  int v6; // esi
  unsigned int v7; // eax
  int v8; // eax
  unsigned int v9; // eax
  BYTE *v10; // eax
  char v11; // cl
  int v12; // ebx
  int v13; // edi
  int *v14; // esi
  int v15; // ecx
  int result; // eax
  int *v17; // edx
  const void *v18; // ebp
  unsigned int v19; // eax
  unsigned int v20; // eax
  BYTE *v21; // eax
  char v22; // cl
  int v23; // [esp+10h] [ebp-14h]
  int v24; // [esp+10h] [ebp-14h]
  int v25; // [esp+14h] [ebp-10h] BYREF
  DWORD v26; // [esp+18h] [ebp-Ch] BYREF
  DWORD v27; // [esp+1Ch] [ebp-8h] BYREF
  int v28; // [esp+20h] [ebp-4h] BYREF

  v0 = (void *)CharacterMachine;
  v26 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v25 = 0;
  v23 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v25, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v3 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v0);
        v4 = v3[1412] + 1;
        v3[1412] = v4;
        if ( v4 < 2u )
        {
          FUN_00404370(v0, v3);
        }
        goto LABEL_9;
      }
      v2 = ++v23 < (unsigned int)DAT_055c9bd4;
      v1 = (v1 + 1) % DAT_055c9bd4;
      if ( !v2 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v8 = operator_new(0x585u);
  *(BYTE *)(v8 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v8, (int)v0);
LABEL_9:
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 )
  {
    v5 = (const void *)CharacterMachine;
    v6 = *(short *)(CharacterMachine + 536);
    v28 = *(short *)(CharacterMachine + 604);
    v24 = v6;
    v27 = CharacterMachine;
    v7 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
        {
          if ( v7 != -1 )
          {
            v9 = FUN_004041e0(&DAT_055c9bc8, (int)v5);
            v10 = v9 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v9);
            v11 = v10[1412] - 1;
            v10[1412] = v11;
            if ( !v11 )
            {
              FUN_00404400(v10, v5);
            }
          }
          break;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v7 = (v7 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          v6 = v24;
          goto LABEL_15;
        }
      }
      v6 = v24;
    }
    else
    {
LABEL_15:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v28 >= 128 && v28 < 135 || v28 == 145 )
    {
      v12 = 143;
    }
    else if ( (v6 < 136 || v6 >= 143) && (v6 < 144 || v6 >= 160) )
    {
      v12 = v28;
    }
    else
    {
      v12 = 135;
    }
    v13 = 7;
    v14 = (int *)&DAT_07ea9504;
LABEL_35:
    v15 = 7;
    result = v13 + 56;
    v17 = v14;
    while ( *((short *)v17 - 28) != v12 || *v17 <= 0 )
    {
      --v15;
      v17 -= 136;
      result -= 8;
      if ( v15 < 0 )
      {
        v14 -= 17;
        --v13;
        if ( (int)v14 >= (int)&DAT_07ea9328 )
        {
          goto LABEL_35;
        }
        return -1;
      }
    }
  }
  else
  {
    v18 = (const void *)CharacterMachine;
    v27 = CharacterMachine;
    v19 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v28 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
        {
          if ( v19 != -1 )
          {
            v20 = FUN_004041e0(&DAT_055c9bc8, (int)v18);
            v21 = v20 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v20);
            v22 = v21[1412] - 1;
            v21[1412] = v22;
            if ( !v22 )
            {
              FUN_00404400(v21, v18);
            }
          }
          return -1;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v19 = (v19 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          goto LABEL_45;
        }
      }
      return -1;
    }
    else
    {
LABEL_45:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      return -1;
    }
  }
  return result;
}
#endif

// ── FUN_00482850 (IDA-only, gated) ──
#if defined(IDA_PORT_00482850)
int FUN_00482850()
{
  void *v0; // ebp
  unsigned int v1; // eax
  bool v2; // cf
  int v3; // eax
  const void *v4; // ebp
  int v5; // esi
  unsigned int v6; // eax
  BYTE *v7; // eax
  unsigned char v8; // cl
  unsigned int v9; // eax
  BYTE *v10; // eax
  char v11; // cl
  int v12; // esi
  int v13; // edi
  int *v14; // edx
  int *v15; // eax
  int v16; // ecx
  const void *v18; // ebp
  unsigned int v19; // eax
  unsigned int v20; // eax
  BYTE *v21; // eax
  char v22; // cl
  int v23; // [esp+14h] [ebp-14h]
  int v24; // [esp+14h] [ebp-14h]
  int v25; // [esp+18h] [ebp-10h] BYREF
  DWORD v26; // [esp+1Ch] [ebp-Ch] BYREF
  DWORD v27; // [esp+20h] [ebp-8h] BYREF
  int v28; // [esp+24h] [ebp-4h] BYREF

  v0 = (void *)CharacterMachine;
  v26 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v25 = 0;
  v23 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v25, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v7 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v0);
        v8 = v7[1412] + 1;
        v7[1412] = v8;
        if ( v8 < 2u )
        {
          FUN_00404370(v0, v7);
        }
        goto LABEL_7;
      }
      v2 = ++v23 < (unsigned int)DAT_055c9bd4;
      v1 = (v1 + 1) % DAT_055c9bd4;
      if ( !v2 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v3 = operator_new(0x585u);
  *(BYTE *)(v3 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v3, (int)v0);
LABEL_7:
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 )
  {
    v4 = (const void *)CharacterMachine;
    v5 = *(short *)(CharacterMachine + 536);
    v28 = *(short *)(CharacterMachine + 604);
    v24 = v5;
    v27 = CharacterMachine;
    v6 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
        {
          if ( v6 != -1 )
          {
            v9 = FUN_004041e0(&DAT_055c9bc8, (int)v4);
            v10 = v9 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v9);
            v11 = v10[1412] - 1;
            v10[1412] = v11;
            if ( !v11 )
            {
              FUN_00404400(v10, v4);
            }
          }
          break;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v6 = (v6 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          v5 = v24;
          goto LABEL_13;
        }
      }
      v5 = v24;
    }
    else
    {
LABEL_13:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v28 >= 128 && v28 < 135 || v28 == 145 )
    {
      v12 = 143;
    }
    else if ( (v5 < 136 || v5 >= 143) && (v5 < 144 || v5 >= 160) )
    {
      v12 = v28;
    }
    else
    {
      v12 = 135;
    }
    v13 = 0;
    v14 = (int *)&DAT_07ea9504;
    do
    {
      v15 = v14;
      v16 = 8;
      do
      {
        if ( *((short *)v15 - 28) == v12 && *v15 > 0 )
        {
          ++v13;
        }
        v15 -= 136;
        --v16;
      }
      while ( v16 );
      v14 -= 17;
    }
    while ( (int)v14 >= (int)&DAT_07ea9328 );
    return v13;
  }
  else
  {
    v18 = (const void *)CharacterMachine;
    v27 = CharacterMachine;
    v19 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v28 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
        {
          if ( v19 != -1 )
          {
            v20 = FUN_004041e0(&DAT_055c9bc8, (int)v18);
            v21 = v20 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v20);
            v22 = v21[1412] - 1;
            v21[1412] = v22;
            if ( !v22 )
            {
              FUN_00404400(v21, v18);
            }
          }
          return 0;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v19 = (v19 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          goto LABEL_46;
        }
      }
      return 0;
    }
    else
    {
LABEL_46:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      return 0;
    }
  }
}
#endif

// ── FUN_00482d70 (IDA-only, gated) ──
#if defined(IDA_PORT_00482D70)
int __cdecl FUN_00482d70(int iType, int iLevel)
{
  int v2; // ebx
  int *v3; // esi
  int v4; // edx
  int result; // eax
  int *v6; // ecx

  v2 = 7;
  v3 = (int *)&DAT_07ea9504;
LABEL_2:
  v4 = 7;
  result = v2 + 56;
  v6 = v3;
  while ( *((short *)v6 - 28) != iType || *v6 <= 0 || iLevel != -1 && ((*(v6 - 13) >> 3) & 0xF) != iLevel )
  {
    --v4;
    v6 -= 136;
    result -= 8;
    if ( v4 < 0 )
    {
      v3 -= 17;
      --v2;
      if ( (int)v3 >= (int)&DAT_07ea9328 )
      {
        goto LABEL_2;
      }
      return -1;
    }
  }
  return result;
}
#endif

// ── FUN_00482dd0 (IDA-only, gated) ──
#if defined(IDA_PORT_00482DD0)
int __cdecl CSQuest::FindQuestItemsInInven(DWORD This, int nType, int nCount, int nLevel)
{
  int v4; // ebx
  int *v5; // edi
  int v6; // edx
  int *v7; // ecx

  v4 = 0;
  v5 = (int *)&DAT_07ea9504;
  while ( 2 )
  {
    v6 = 7;
    v7 = v5;
    do
    {
      if ( *((short *)v7 - 28) == nType
        && *v7 > 0
        && (nLevel == -1 || ((*(v7 - 13) >> 3) & 0xF) == nLevel)
        && ++v4 >= nCount )
      {
        return 0;
      }
      --v6;
      v7 -= 136;
    }
    while ( v6 >= 0 );
    v5 -= 17;
    if ( (int)v5 >= (int)&DAT_07ea9328 )
    {
      continue;
    }
    break;
  }
  return nCount - v4;
}
#endif

// ── FUN_00482ff0 (IDA-only, gated) ──
#if defined(IDA_PORT_00482FF0)
int __cdecl FUN_00482ff0(int iType, int iLevel)
{
  int result; // eax
  int *v3; // edi
  int *v4; // ecx
  int v5; // edx

  result = 0;
  v3 = (int *)&DAT_07ea9504;
  do
  {
    v4 = v3;
    v5 = 8;
    do
    {
      if ( *((short *)v4 - 28) == iType
        && (iType == -1 || *v4 > 0)
        && (iLevel == -1 || ((*(v4 - 13) >> 3) & 0xF) == iLevel) )
      {
        ++result;
      }
      v4 -= 136;
      --v5;
    }
    while ( v5 );
    v3 -= 17;
  }
  while ( (int)v3 >= (int)&DAT_07ea9328 );
  return result;
}
#endif

// ── FUN_00485780 (IDA-only, gated) ──
#if defined(IDA_PORT_00485780)
void __cdecl FUN_00485780(DWORD c, DWORD o)
{
  void *v2; // esi
  unsigned int v3; // eax
  BYTE *v4; // eax
  unsigned char v5; // cl
  int v6; // eax
  const void *v7; // esi
  unsigned int v8; // eax
  BYTE *v9; // eax
  char v10; // cl
  char *v11; // esi
  char v12; // al
  double v13; // st7
  int v14; // edx
  int i; // esi
  int v16; // ecx
  int j; // esi
  int v18; // ecx
  int k; // esi
  int v20; // ebp
  int v21; // edi
  int v22; // eax
  int v23; // esi
  bool v24; // zf
  int v25; // eax
  DWORD v26; // ecx
  int v27; // eax
  float v28; // edx
  double v29; // st7
  float v30; // ecx
  float v31; // eax
  DWORD TickCount; // eax
  double v33; // st7
  float v34; // edx
  float v35; // eax
  double v36; // st7
  double v37; // st7
  char *v38; // ebp
  char *v39; // esi
  int v40; // eax
  unsigned int v41; // kr14_4
  int v42; // edx
  unsigned int v43; // kr18_4
  int v44; // ecx
  int m; // esi
  int v46; // ecx
  int n; // esi
  int v48; // ecx
  int ii; // esi
  __int64 v50; // rax
  int v51; // ecx
  int jj; // esi
  int v53; // ecx
  int kk; // esi
  int v55; // ecx
  int mm; // esi
  int v57; // ebp
  int v58; // esi
  BYTE v59; // al
  int v60; // edi
  char *v61; // esi
  int v62; // eax
  signed int v63; // ebp
  int v64; // edi
  int v65; // eax
  int v66; // esi
  int v67; // eax
  float *v68; // ebp
  int v69; // edi
  int v70; // eax
  int v71; // esi
  int v72; // esi
  short v73; // dx
  void *v74; // esi
  unsigned int v75; // eax
  BYTE *v76; // eax
  unsigned char v77; // cl
  int v78; // eax
  BYTE *v79; // esi
  int v80; // eax
  unsigned int v81; // eax
  BYTE *v82; // eax
  char v83; // cl
  DWORD v84; // edx
  unsigned int v85; // kr1C_4
  int v86; // esi
  unsigned int v87; // kr20_4
  int v88; // ecx
  int v89; // eax
  int v90; // ecx
  int nn; // esi
  int v92; // ecx
  int i1; // esi
  int v94; // ecx
  int i2; // esi
  char v96; // cl
  int v97; // ecx
  int i3; // esi
  int v99; // edx
  int v100; // ecx
  int i4; // esi
  int *v102; // edi
  int v103; // ebp
  int v104; // eax
  int v105; // ecx
  int i5; // esi
  char v107; // cl
  int v108; // ecx
  int i6; // esi
  int v110; // ebp
  int v111; // edi
  char *v112; // eax
  char v113; // cl
  int v114; // eax
  BYTE v115; // al
  int v116; // edi
  int v117; // esi
  char *v118; // ebp
  int v119; // eax
  signed int v120; // ebp
  int v121; // edi
  int v122; // eax
  int v123; // esi
  int v124; // eax
  float *v125; // edi
  int v126; // ebp
  int v127; // eax
  int v128; // esi
  int v129; // eax
  float *__attribute__((__org_arrdim(0,3))) v130; // edx
  unsigned int v131; // ecx
  void *v132; // edi
  float v133; // edx
  double v134; // st7
  BYTE *v135; // eax
  unsigned char v136; // cl
  int v137; // eax
  float v138; // eax
  int v139; // edx
  void *v140; // edi
  BYTE *v141; // eax
  unsigned char v142; // cl
  int v143; // eax
  int v144; // eax
  int v145; // edi
  int v146; // eax
  int v147; // edi
  int v148; // eax
  BYTE *v149; // eax
  char v150; // cl
  int v151; // edi
  int v152; // eax
  BYTE *v153; // eax
  char v154; // cl
  float v155; // eax
  const void *v156; // edi
  int v157; // edx
  BYTE *v158; // eax
  char v159; // cl
  DWORD v160; // edx
  unsigned int v161; // kr24_4
  int v162; // esi
  unsigned int v163; // kr28_4
  int v164; // ecx
  int v165; // eax
  int v166; // ecx
  int i7; // esi
  int v168; // eax
  WORD *v169; // edi
  int v170; // eax
  int v171; // ecx
  int i8; // esi
  char v173; // cl
  int v174; // ecx
  int i9; // esi
  int v176; // ebp
  int v177; // edi
  char *v178; // eax
  char v179; // cl
  int v180; // eax
  BYTE v181; // al
  int v182; // edi
  int v183; // esi
  char *v184; // ebp
  int v185; // eax
  int v186; // edi
  int v187; // eax
  int v188; // esi
  float *v189; // edi
  int v190; // ebp
  int v191; // eax
  int v192; // esi
  int v193; // ecx
  double v194; // st7
  char v195; // cl
  unsigned char v196; // al
  BYTE v197; // al
  int v198; // esi
  int v199; // eax
  int v200; // ecx
  int v201; // eax
  int v202; // ebx
  int v203; // edi
  signed int v204; // ebp
  int v205; // eax
  int v206; // esi
  int v207; // [esp-4h] [ebp-DD4h]
  signed int Position; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positiona; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionj; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionb; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionc; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positiond; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positione; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionf; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positiong; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionh; // [esp+10h] [ebp-DC0h]
  unsigned char Positioni; // [esp+10h] [ebp-DC0h]
  char v219; // [esp+14h] [ebp-DBCh]
  char v220; // [esp+15h] [ebp-DBBh]
  char v221; // [esp+16h] [ebp-DBAh]
  char v222; // [esp+17h] [ebp-DB9h]
  char v223; // [esp+18h] [ebp-DB8h]
  char v224; // [esp+19h] [ebp-DB7h]
  char v225; // [esp+1Ah] [ebp-DB6h]
  char v226; // [esp+1Bh] [ebp-DB5h]
  char v227; // [esp+1Ch] [ebp-DB4h]
  char v228; // [esp+1Dh] [ebp-DB3h]
  char v229; // [esp+1Eh] [ebp-DB2h]
  char v230; // [esp+1Fh] [ebp-DB1h]
  char v231; // [esp+20h] [ebp-DB0h]
  char v232; // [esp+21h] [ebp-DAFh]
  char v233; // [esp+22h] [ebp-DAEh]
  char v234; // [esp+23h] [ebp-DADh]
  char v235; // [esp+24h] [ebp-DACh]
  char v236; // [esp+25h] [ebp-DABh]
  char v237; // [esp+26h] [ebp-DAAh]
  char v238; // [esp+27h] [ebp-DA9h]
  char v239; // [esp+28h] [ebp-DA8h]
  char v240; // [esp+29h] [ebp-DA7h]
  char v241; // [esp+2Ah] [ebp-DA6h]
  char v242; // [esp+2Bh] [ebp-DA5h]
  char v243; // [esp+2Ch] [ebp-DA4h]
  char v244; // [esp+2Dh] [ebp-DA3h]
  char v245; // [esp+2Eh] [ebp-DA2h]
  char v246; // [esp+2Fh] [ebp-DA1h]
  char v247; // [esp+30h] [ebp-DA0h]
  char v248; // [esp+31h] [ebp-D9Fh]
  char v249; // [esp+32h] [ebp-D9Eh]
  char v250; // [esp+33h] [ebp-D9Dh]
  char v251; // [esp+34h] [ebp-D9Ch]
  char v252; // [esp+35h] [ebp-D9Bh]
  char v253; // [esp+36h] [ebp-D9Ah]
  char v254; // [esp+37h] [ebp-D99h]
  char v255; // [esp+38h] [ebp-D98h]
  char v256; // [esp+39h] [ebp-D97h]
  char v257; // [esp+3Ah] [ebp-D96h]
  char v258; // [esp+3Bh] [ebp-D95h]
  char v259; // [esp+3Ch] [ebp-D94h]
  char v260; // [esp+3Dh] [ebp-D93h]
  char v261; // [esp+3Eh] [ebp-D92h]
  char v262; // [esp+3Fh] [ebp-D91h]
  char v263; // [esp+40h] [ebp-D90h]
  char v264; // [esp+41h] [ebp-D8Fh]
  char v265; // [esp+42h] [ebp-D8Eh]
  char v266; // [esp+43h] [ebp-D8Dh]
  char v267; // [esp+44h] [ebp-D8Ch]
  char v268; // [esp+45h] [ebp-D8Bh]
  char v269; // [esp+46h] [ebp-D8Ah]
  char v270; // [esp+47h] [ebp-D89h]
  char v271; // [esp+48h] [ebp-D88h]
  char v272; // [esp+49h] [ebp-D87h]
  char v273; // [esp+4Ah] [ebp-D86h]
  char v274; // [esp+4Bh] [ebp-D85h]
  char v275; // [esp+4Ch] [ebp-D84h]
  char v276; // [esp+4Dh] [ebp-D83h]
  char v277; // [esp+4Eh] [ebp-D82h]
  char v278; // [esp+4Fh] [ebp-D81h]
  char v279; // [esp+50h] [ebp-D80h]
  char v280; // [esp+51h] [ebp-D7Fh]
  char v281; // [esp+52h] [ebp-D7Eh]
  char v282; // [esp+53h] [ebp-D7Dh]
  WORD SkillIndex[2]; // [esp+54h] [ebp-D7Ch] BYREF
  DWORD v284; // [esp+58h] [ebp-D78h]
  int v285; // [esp+5Ch] [ebp-D74h] BYREF
  char v286; // [esp+60h] [ebp-D70h]
  char v287; // [esp+61h] [ebp-D6Fh]
  char v288; // [esp+62h] [ebp-D6Eh]
  char v289; // [esp+63h] [ebp-D6Dh]
  char v290; // [esp+64h] [ebp-D6Ch]
  char v291; // [esp+65h] [ebp-D6Bh]
  char v292; // [esp+66h] [ebp-D6Ah]
  char v293; // [esp+67h] [ebp-D69h]
  char v294; // [esp+68h] [ebp-D68h]
  char v295; // [esp+69h] [ebp-D67h]
  char v296; // [esp+6Ah] [ebp-D66h]
  char v297; // [esp+6Bh] [ebp-D65h]
  char v298; // [esp+6Ch] [ebp-D64h]
  char v299; // [esp+6Dh] [ebp-D63h]
  char v300; // [esp+6Eh] [ebp-D62h]
  char v301; // [esp+6Fh] [ebp-D61h]
  char v302; // [esp+70h] [ebp-D60h]
  char v303; // [esp+71h] [ebp-D5Fh]
  char v304; // [esp+72h] [ebp-D5Eh]
  char v305; // [esp+73h] [ebp-D5Dh]
  char v306; // [esp+74h] [ebp-D5Ch]
  char v307; // [esp+75h] [ebp-D5Bh]
  char v308; // [esp+76h] [ebp-D5Ah]
  char v309; // [esp+77h] [ebp-D59h]
  char v310; // [esp+78h] [ebp-D58h]
  char v311; // [esp+79h] [ebp-D57h]
  char v312; // [esp+7Ah] [ebp-D56h]
  char v313; // [esp+7Bh] [ebp-D55h]
  float x2; // [esp+7Ch] [ebp-D54h]
  float Angle[3]; // [esp+80h] [ebp-D50h] BYREF
  char v316; // [esp+8Fh] [ebp-D41h]
  int v317; // [esp+90h] [ebp-D40h]
  WORD v318[2]; // [esp+94h] [ebp-D3Ch]
  int v319; // [esp+98h] [ebp-D38h]
  float v320; // [esp+9Ch] [ebp-D34h] BYREF
  float v321; // [esp+A0h] [ebp-D30h]
  float v322; // [esp+A4h] [ebp-D2Ch]
  float Light[3]; // [esp+A8h] [ebp-D28h] BYREF
  void *(__cdecl **v324)(std::locale::facet *__hidden, unsigned int); // [esp+B4h] [ebp-D1Ch]
  WORD buf[514]; // [esp+B8h] [ebp-D18h] BYREF
  char v326; // [esp+4BCh] [ebp-914h] BYREF
  char v327; // [esp+4BDh] [ebp-913h]
  char v328[258]; // [esp+4BEh] [ebp-912h] BYREF
  char v329; // [esp+5C0h] [ebp-810h] BYREF
  char v330; // [esp+5C1h] [ebp-80Fh]
  char v331; // [esp+5C2h] [ebp-80Eh]
  char v332[1025]; // [esp+5C3h] [ebp-80Dh] BYREF
  char v333[1024]; // [esp+9C4h] [ebp-40Ch] BYREF
  int v334; // [esp+DCCh] [ebp-4h]

  v2 = (void *)CharacterMachine;
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
  {
    v6 = operator_new(0x585u);
    *(BYTE *)(v6 + 1412) = 1;
    FUN_00403f80(&DAT_055c9bc8, v6, (int)v2);
  }
  else
  {
    v3 = FUN_004041e0(&DAT_055c9bc8, (int)v2);
    if ( v3 == -1 )
    {
      v4 = 0;
    }
    else
    {
      v4 = *(BYTE **)(DAT_055c9bcc + 4 * v3);
    }
    v5 = v4[1412] + 1;
    v4[1412] = v5;
    if ( v5 < 2u )
    {
      FUN_00404370(v2, v4);
    }
  }
  if ( (BYTE)DAT_07d78098 )
  {
    v317 = *(unsigned char *)(DAT_07d7809c + CharacterAttribute + 87);
  }
  else
  {
    v317 = DAT_07d7809c;
  }
  v7 = (const void *)CharacterMachine;
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) != -1 )
  {
    v8 = FUN_004041e0(&DAT_055c9bc8, (int)v7);
    v9 = v8 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v8);
    v10 = v9[1412] - 1;
    v9[1412] = v10;
    if ( !v10 )
    {
      FUN_00404400(v9, v7);
    }
  }
  v11 = (char *)(Hero + 904);
  FUN_0043d3e0(&DAT_055c9bc8, (DWORD *)(Hero + 904));
  *(float *)SkillIndex = *(float *)v11;
  FUN_004233e0(&DAT_055c9bc8, v11);
  v12 = FUN_0043d670(&DAT_055c9bc8, (char *)(Hero + 908));
  v324 = &DAT_00552460;
  v13 = *(float *)(Hero + 36);
  buf[1] = 0x1C1;
  v334 = 0;
  (BYTE)(buf[2]) = 0x10;
  buf[0] = 3;
  (WORD)(v285) = 28135;
  *((BYTE *)&buf[1] + buf[0]) = SkillIndex[0];
  (WORD)((v285) >> 16) = 0x893A;
  v286 = -68;
  v14 = buf[0] + 1;
  v287 = -78;
  v288 = -97;
  v289 = 115;
  v290 = 35;
  v291 = -88;
  v292 = -2;
  v293 = -74;
  v294 = 73;
  v295 = 93;
  v296 = 57;
  v297 = 93;
  v298 = -118;
  v299 = -53;
  v300 = 99;
  v301 = -115;
  v302 = -22;
  v303 = 125;
  v304 = 43;
  v305 = 95;
  v306 = -61;
  v307 = -79;
  v308 = -23;
  v309 = -125;
  v310 = 41;
  v311 = 81;
  v312 = -24;
  v313 = 86;
  for ( i = buf[0]; i != v14; ++i )
  {
    *((BYTE *)&buf[1] + i) ^= *((BYTE *)buf + i + 1) ^ *((BYTE *)&v285 + i % 32);
  }
  if ( ++buf[0] + 1 <= 1024 )
  {
    (WORD)(v285) = 28135;
    ((BYTE)((v285) >> 16)) = 58;
    *((BYTE *)&buf[1] + buf[0]) = v12;
    (BYTE)((v285) >> 8) = -119;
    v286 = -68;
    v287 = -78;
    v16 = buf[0] + 1;
    v288 = -97;
    v289 = 115;
    v290 = 35;
    v291 = -88;
    v292 = -2;
    v293 = -74;
    v294 = 73;
    v295 = 93;
    v296 = 57;
    v297 = 93;
    v298 = -118;
    v299 = -53;
    v300 = 99;
    v301 = -115;
    v302 = -22;
    v303 = 125;
    v304 = 43;
    v305 = 95;
    v306 = -61;
    v307 = -79;
    v308 = -23;
    v309 = -125;
    v310 = 41;
    v311 = 81;
    v312 = -24;
    v313 = 86;
    for ( j = buf[0]; j != v16; ++j )
    {
      *((BYTE *)&buf[1] + j) ^= *((BYTE *)buf + j + 1) ^ *((BYTE *)&v285 + j % 32);
    }
    ++buf[0];
  }
  *(DWORD *)v318 = 0;
  v319 = 0;
  if ( buf[0] + 1 <= 1024 )
  {
    (WORD)(v285) = 28135;
    ((BYTE)((v285) >> 16)) = 58;
    *((BYTE *)&buf[1] + buf[0]) = 16 * ((__int64)((v13 + 22.5) * 0.022222223 + 1.0) & 7);
    (BYTE)((v285) >> 8) = -119;
    v286 = -68;
    v287 = -78;
    v18 = buf[0] + 1;
    v288 = -97;
    v289 = 115;
    v290 = 35;
    v291 = -88;
    v292 = -2;
    v293 = -74;
    v294 = 73;
    v295 = 93;
    v296 = 57;
    v297 = 93;
    v298 = -118;
    v299 = -53;
    v300 = 99;
    v301 = -115;
    v302 = -22;
    v303 = 125;
    v304 = 43;
    v305 = 95;
    v306 = -61;
    v307 = -79;
    v308 = -23;
    v309 = -125;
    v310 = 41;
    v311 = 81;
    v312 = -24;
    v313 = 86;
    for ( k = buf[0]; k != v18; ++k )
    {
      *((BYTE *)&buf[1] + k) ^= *((BYTE *)buf + k + 1) ^ *((BYTE *)&v285 + k % 32);
    }
    ++buf[0];
  }
  if ( (BYTE)(buf[1]) == 193 )
  {
    (BYTE)((buf[1]) >> 8) = buf[0];
  }
  else if ( (BYTE)(buf[1]) == 194 )
  {
    *(WORD *)((char *)&buf[1] + 1) = buf[0];
  }
  v20 = buf[0];
  v21 = 0;
  Position = buf[0];
  if ( s != -1 )
  {
    while ( 1 )
    {
      v22 = send(s, (const char *)&buf[1] + v21, v20 - v21, 0);
      v23 = v22;
      if ( v22 == -1 )
      {
        break;
      }
      if ( v22 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2((int)&buf[1], v22);
        }
        v21 += v23;
        Position -= v23;
        if ( Position > 0 )
        {
          continue;
        }
      }
      goto LABEL_44;
    }
    if ( WSAGetLastError() == 10035 && DAT_055cc16c + v20 <= 0x2000 )
    {
      qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf[1], Position);
      DAT_055cc16c += Position;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
LABEL_44:
  v334 = -1;
  v24 = *(WORD *)(o + 2) == 390;
  *(BYTE *)(c + 748) = 0;
  if ( v24 )
  {
    SetAttackSpeed();
    switch ( v317 )
    {
      case 43:
        SetAction(o, 67);
        break;
      case 47:
        SetAction(o, 66);
        break;
      case 49:
        if ( World == 8 || World == 10 )
        {
          SetAction(o, 65);
        }
        else
        {
          SetAction(o, 64);
        }
        break;
      default:
        SetAction(o, DAT_07d7809c + 37);
        break;
    }
  }
  else
  {
    SetPlayerAttack(c);
  }
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  Positiona = (float *)(o + 16);
  Particle_Spawn(1232, (float *)(o + 16), (float *)(o + 28), Light, 0, 0.0, o);
  v25 = rand() % 2;
  PlayBuffer(v25 + 40, 0, 0);
  v26 = CharactersClient;
  v27 = 229 * MovementSkillTarget;
  *(DWORD *)(c + 796) = *(DWORD *)(CharactersClient + 916 * MovementSkillTarget + 24);
  v28 = *(float *)(o + 20);
  x2 = *(float *)(v26 + 4 * v27 + 16);
  *(float *)(c + 788) = x2;
  v29 = *(float *)(v26 + 4 * v27 + 20);
  v30 = x2;
  *(float *)SkillIndex = v29;
  v31 = *(float *)SkillIndex;
  *(float *)(c + 792) = v29;
  *(float *)SkillIndex = Movement_Tick(*(float *)(o + 16), v28, v30, v31);
  v24 = v317 == 43;
  *(float *)(o + 36) = *(float *)SkillIndex;
  if ( v24 )
  {
    TickCount = GetTickCount();
    v33 = *(float *)(c + 788);
    DAT_07e11d84 = TickCount;
    v34 = *(float *)(o + 20);
    Angle[0] = *Positiona;
    v35 = *(float *)(o + 24);
    Angle[1] = v34;
    Angle[2] = v35;
    v320 = v33 - Angle[0];
    v321 = *(float *)(c + 792) - v34;
    v322 = *(float *)(c + 796) - v35;
    if ( FUN_004f9c40(&v320) >= 1.0 )
    {
      v36 = FUN_004f9c40(&v320);
    }
    else
    {
      v36 = 1.0;
    }
    v37 = 120.0 / v36;
    v320 = v37 * v320;
    v321 = v37 * v321;
    v322 = v37 * v322;
    v38 = (char *)(c + 908);
    LODWORD(x2) = (__int64)(*(float *)(c + 788) * 0.0099999998);
    FUN_0043d3e0(&DAT_055c9bc8, (DWORD *)(c + 908));
    FUN_004233e0(&DAT_055c9bc8, (char *)(c + 908));
    v39 = (char *)(c + 904);
    FUN_0043d3e0(&DAT_055c9bc8, (DWORD *)(c + 904));
    Positionj = *(float **)(c + 904);
    FUN_004233e0(&DAT_055c9bc8, (char *)(c + 904));
    v316 = (16 * ((BYTE)(x2) - (BYTE)Positionj + 8)) | ((BYTE)(x2) - (BYTE)Positionj - 8) & 0xF;
    v284 = Hero + 449;
    v40 = 0;
    v41 = strlen(aWebzen_8) + 1;
    v42 = v41 - 1;
    v43 = strlen((const char *)(Hero + 449)) + 1;
    if ( (int)(v43 - v41) >= 0 )
    {
      x2 = 0.0;
      do
      {
        Positionb = 0;
        if ( v42 <= 0 )
        {
          goto LABEL_121;
        }
        *(DWORD *)v318 = v284 + v40;
        while ( *((BYTE *)Positionb + *(DWORD *)v318) == aWebzen_8[(DWORD)Positionb] )
        {
          Positionb = (float *)((char *)Positionb + 1);
          if ( (int)Positionb >= v42 )
          {
            goto LABEL_121;
          }
        }
        v40 = ++LODWORD(x2);
      }
      while ( SLODWORD(x2) <= (int)(v43 - v41) );
    }
    DAT_05826d10 = 0x2B;
    v324 = &DAT_00552460;
    buf[1] = 0x1C1;
    v334 = 1;
    (BYTE)(buf[2]) = 0x1E;
    buf[0] = 3;
    FUN_0043d3e0(&DAT_055c9bc8, v38);
    x2 = *(float *)v38;
    FUN_004233e0(&DAT_055c9bc8, v38);
    FUN_0043d3e0(&DAT_055c9bc8, v39);
    *(DWORD *)v318 = *(DWORD *)v39;
    FUN_004233e0(&DAT_055c9bc8, v39);
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = 43;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v44 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( m = buf[0]; m != v44; ++m )
      {
        *((BYTE *)&buf[1] + m) ^= *((BYTE *)buf + m + 1) ^ *(&v219 + m % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v318[0];
      v221 = 58;
      v222 = -119;
      v223 = -68;
      v46 = buf[0] + 1;
      v224 = -78;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( n = buf[0]; n != v46; ++n )
      {
        *((BYTE *)&buf[1] + n) ^= *((BYTE *)buf + n + 1) ^ *(&v219 + n % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      *((BYTE *)&buf[1] + buf[0]) = (BYTE)(x2);
      v221 = 58;
      v222 = -119;
      v223 = -68;
      v48 = buf[0] + 1;
      v224 = -78;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( ii = buf[0]; ii != v48; ++ii )
      {
        *((BYTE *)&buf[1] + ii) ^= *((BYTE *)buf + ii + 1) ^ *(&v219 + ii % 32);
      }
      ++buf[0];
    }
    v50 = (__int64)(*(float *)(o + 36) * 0.71111113);
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = v50;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v51 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( jj = buf[0]; jj != v51; ++jj )
      {
        *((BYTE *)&buf[1] + jj) ^= *((BYTE *)buf + jj + 1) ^ *(&v219 + jj % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v316;
      v221 = 58;
      v222 = -119;
      v223 = -68;
      v53 = buf[0] + 1;
      v224 = -78;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( kk = buf[0]; kk != v53; ++kk )
      {
        *((BYTE *)&buf[1] + kk) ^= *((BYTE *)buf + kk + 1) ^ *(&v219 + kk % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = 0;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v55 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( mm = buf[0]; mm != v55; ++mm )
      {
        *((BYTE *)&buf[1] + mm) ^= *((BYTE *)buf + mm + 1) ^ *(&v219 + mm % 32);
      }
      ++buf[0];
    }
    if ( (BYTE)(buf[1]) == 193 )
    {
      (BYTE)((buf[1]) >> 8) = buf[0];
    }
    else if ( (BYTE)(buf[1]) == 194 )
    {
      *(WORD *)((char *)&buf[1] + 1) = buf[0];
    }
    v57 = buf[0];
    qmemcpy(v333, &buf[1], buf[0]);
    v333[v57] = rand();
    v58 = (v333[0] != -63) + 2;
    PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    v59 = g_byPacketSerialSend;
    v332[v58 + 1024] = g_byPacketSerialSend;
    g_byPacketSerialSend = v59 + 1;
    PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    --v58;
    v60 = v57 - v58;
    v61 = &v333[v58];
    v62 = FUN_0053cc30(0, (int)v61, v60);
    if ( v62 >= 256 )
    {
      v68 = (float *)(v62 + 3);
      v329 = -60;
      v331 = v62 + 3;
      v330 = (v62 + 3) / 256;
      FUN_0053cc30((int)v332, (int)v61, v60);
      v69 = 0;
      Positiond = v68;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v70 = send(s, &v329 + v69, (int)v68 - v69, 0);
          v71 = v70;
          if ( v70 == -1 )
          {
            break;
          }
          if ( v70 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&v329, v70);
            }
            v69 += v71;
            Positiond = (float *)((char *)Positiond - v71);
            if ( (int)Positiond > 0 )
            {
              continue;
            }
          }
          goto LABEL_120;
        }
        if ( WSAGetLastError() == 10035 && (int)v68 + DAT_055cc16c <= 0x2000 )
        {
          qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v329, (unsigned int)Positiond);
          v67 = (int)Positiond + DAT_055cc16c;
          goto LABEL_119;
        }
        goto LABEL_117;
      }
    }
    else
    {
      Positionc = (float *)(v62 + 2);
      v326 = -61;
      v327 = v62 + 2;
      FUN_0053cc30((int)v328, (int)v61, v60);
      v63 = (signed int)Positionc;
      v64 = 0;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v65 = send(s, &v326 + v64, (int)Positionc - v64, 0);
          v66 = v65;
          if ( v65 == -1 )
          {
            break;
          }
          if ( v65 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&v326, v65);
            }
            v63 -= v66;
            v64 += v66;
            if ( v63 > 0 )
            {
              continue;
            }
          }
          goto LABEL_120;
        }
        if ( WSAGetLastError() == 10035 && (int)Positionc + DAT_055cc16c <= 0x2000 )
        {
          qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v326, v63);
          v67 = v63 + DAT_055cc16c;
LABEL_119:
          DAT_055cc16c = v67;
          goto LABEL_120;
        }
LABEL_117:
        CWsctlc::Close((DWORD)&SocketClient);
      }
    }
LABEL_120:
    v334 = -1;
LABEL_121:
    v72 = 2;
    v285 = *(short *)(CharactersClient + 916 * MovementSkillTarget + 476);
    *(DWORD *)SkillIndex = 1;
    do
    {
      v73 = *(WORD *)(o + 134);
      Angle[0] = v320 + Angle[0];
      Angle[1] = v321 + Angle[1];
      Angle[2] = v322 + Angle[2];
      FUN_0045fdb0((int)Angle, 100.0, v73, (int)&v285, (int)SkillIndex, 6);
      --v72;
    }
    while ( v72 );
    v74 = (void *)CharacterMachine;
    if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
    {
      v78 = operator_new(0x585u);
      *(BYTE *)(v78 + 1412) = 1;
      FUN_00403f80(&DAT_055c9bc8, v78, (int)v74);
    }
    else
    {
      v75 = FUN_004041e0(&DAT_055c9bc8, (int)v74);
      if ( v75 == -1 )
      {
        v76 = 0;
      }
      else
      {
        v76 = *(BYTE **)(DAT_055c9bcc + 4 * v75);
      }
      v77 = v76[1412] + 1;
      v76[1412] = v77;
      if ( v77 < 2u )
      {
        FUN_00404370(v74, v76);
      }
    }
    LODWORD(x2) = *(unsigned char *)(*(unsigned char *)(Hero + 913) + CharacterAttribute + 87);
    v79 = (BYTE *)CharacterMachine;
    v80 = *(unsigned char *)(CharacterMachine + 1408);
    v207 = CharacterMachine;
    *(BYTE *)(o + 136) = v80;
    v79[1408] = v80 + 1;
    if ( FUN_004041e0(&DAT_055c9bc8, v207) != -1 )
    {
      v81 = FUN_004041e0(&DAT_055c9bc8, (int)v79);
      v82 = v81 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v81);
      v83 = v82[1412] - 1;
      v82[1412] = v83;
      if ( !v83 )
      {
        FUN_00404400(v82, v79);
      }
    }
    v84 = Hero + 449;
    v85 = strlen(aWebzen_9) + 1;
    v86 = v85 - 1;
    v87 = strlen((const char *)(Hero + 449)) + 1;
    *(DWORD *)v318 = Hero + 449;
    if ( (int)(v87 - v85) >= 0 )
    {
      v88 = 0;
      while ( 1 )
      {
        v89 = 0;
        if ( v86 <= 0 )
        {
          goto LABEL_305;
        }
        while ( *(BYTE *)(v88 + v84 + v89) == aWebzen_9[v89] )
        {
          if ( ++v89 >= v86 )
          {
            goto LABEL_305;
          }
        }
        if ( ++v88 > (int)(v87 - v85) )
        {
          break;
        }
        v84 = *(DWORD *)v318;
      }
    }
    v324 = &DAT_00552460;
    buf[1] = 0x1C1;
    v334 = 2;
    (BYTE)(buf[2]) = 0x1D;
    buf[0] = 3;
    v219 = -25;
    v220 = 109;
    *((BYTE *)&buf[1] + buf[0]) = (BYTE)(x2);
    v221 = 58;
    v222 = -119;
    v223 = -68;
    v90 = buf[0] + 1;
    v224 = -78;
    v225 = -97;
    v226 = 115;
    v227 = 35;
    v228 = -88;
    v229 = -2;
    v230 = -74;
    v231 = 73;
    v232 = 93;
    v233 = 57;
    v234 = 93;
    v235 = -118;
    v236 = -53;
    v237 = 99;
    v238 = -115;
    v239 = -22;
    v240 = 125;
    v241 = 43;
    v242 = 95;
    v243 = -61;
    v244 = -79;
    v245 = -23;
    v246 = -125;
    v247 = 41;
    v248 = 81;
    v249 = -24;
    v250 = 86;
    for ( nn = buf[0]; nn != v90; ++nn )
    {
      *((BYTE *)&buf[1] + nn) ^= *((BYTE *)buf + nn + 1) ^ *(&v219 + nn % 32);
    }
    if ( ++buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = (__int64)(Angle[0] * 0.0099999998);
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v92 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i1 = buf[0]; i1 != v92; ++i1 )
      {
        *((BYTE *)&buf[1] + i1) ^= *((BYTE *)buf + i1 + 1) ^ *(&v219 + i1 % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = (__int64)(Angle[1] * 0.0099999998);
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v94 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i2 = buf[0]; i2 != v94; ++i2 )
      {
        *((BYTE *)&buf[1] + i2) ^= *((BYTE *)buf + i2 + 1) ^ *(&v219 + i2 % 32);
      }
      ++buf[0];
    }
    v96 = *(BYTE *)(o + 136);
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = v96;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v97 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i3 = buf[0]; i3 != v97; ++i3 )
      {
        *((BYTE *)&buf[1] + i3) ^= *((BYTE *)buf + i3 + 1) ^ *(&v219 + i3 % 32);
      }
      ++buf[0];
    }
    v99 = *(DWORD *)SkillIndex;
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = SkillIndex[0];
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v100 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i4 = buf[0]; i4 != v100; ++i4 )
      {
        *((BYTE *)&buf[1] + i4) ^= *((BYTE *)buf + i4 + 1) ^ *(&v219 + i4 % 32);
      }
      ++buf[0];
    }
    if ( v99 > 0 )
    {
      v102 = &v285;
      v103 = v99;
      do
      {
        v104 = *v102 >> 8;
        if ( buf[0] + 1 <= 1024 )
        {
          v219 = -25;
          v220 = 109;
          v221 = 58;
          *((BYTE *)&buf[1] + buf[0]) = v104;
          v222 = -119;
          v223 = -68;
          v224 = -78;
          v105 = buf[0] + 1;
          v225 = -97;
          v226 = 115;
          v227 = 35;
          v228 = -88;
          v229 = -2;
          v230 = -74;
          v231 = 73;
          v232 = 93;
          v233 = 57;
          v234 = 93;
          v235 = -118;
          v236 = -53;
          v237 = 99;
          v238 = -115;
          v239 = -22;
          v240 = 125;
          v241 = 43;
          v242 = 95;
          v243 = -61;
          v244 = -79;
          v245 = -23;
          v246 = -125;
          v247 = 41;
          v248 = 81;
          v249 = -24;
          v250 = 86;
          for ( i5 = buf[0]; i5 != v105; ++i5 )
          {
            *((BYTE *)&buf[1] + i5) ^= *((BYTE *)buf + i5 + 1) ^ *(&v219 + i5 % 32);
          }
          ++buf[0];
        }
        v107 = *(BYTE *)v102;
        if ( buf[0] + 1 <= 1024 )
        {
          v251 = -25;
          v252 = 109;
          v253 = 58;
          *((BYTE *)&buf[1] + buf[0]) = v107;
          v254 = -119;
          v255 = -68;
          v256 = -78;
          v108 = buf[0] + 1;
          v257 = -97;
          v258 = 115;
          v259 = 35;
          v260 = -88;
          v261 = -2;
          v262 = -74;
          v263 = 73;
          v264 = 93;
          v265 = 57;
          v266 = 93;
          v267 = -118;
          v268 = -53;
          v269 = 99;
          v270 = -115;
          v271 = -22;
          v272 = 125;
          v273 = 43;
          v274 = 95;
          v275 = -61;
          v276 = -79;
          v277 = -23;
          v278 = -125;
          v279 = 41;
          v280 = 81;
          v281 = -24;
          v282 = 86;
          for ( i6 = buf[0]; i6 != v108; ++i6 )
          {
            *((BYTE *)&buf[1] + i6) ^= *((BYTE *)buf + i6 + 1) ^ *(&v251 + i6 % 32);
          }
          ++buf[0];
        }
        ++v102;
        --v103;
      }
      while ( v103 );
    }
    if ( (BYTE)(buf[1]) == 193 )
    {
      (BYTE)((buf[1]) >> 8) = buf[0];
    }
    else if ( (BYTE)(buf[1]) == 194 )
    {
      *(WORD *)((char *)&buf[1] + 1) = buf[0];
    }
    v110 = buf[0];
    qmemcpy(v333, &buf[1], buf[0]);
    v333[v110] = rand();
    v111 = (v333[0] != -63) + 2;
    if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
    {
      v114 = operator_new(2u);
      *(BYTE *)(v114 + 1) = 1;
      FUN_00403f80(&DAT_055c9bc8, v114, (int)&g_byPacketSerialSend);
    }
    else
    {
      v112 = (char *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
      v113 = v112[1] + 1;
      v112[1] = v113;
      if ( (unsigned char)v113 < 2u )
      {
        FUN_00404330(&g_byPacketSerialSend, v112);
      }
    }
    v115 = g_byPacketSerialSend;
    v332[v111 + 1024] = g_byPacketSerialSend;
    g_byPacketSerialSend = v115 + 1;
    PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    v116 = v111 - 1;
    v117 = v110 - v116;
    v118 = &v333[v116];
    v119 = FUN_0053cc30(0, (int)&v333[v116], v117);
    if ( v119 < 256 )
    {
      Positione = (float *)(v119 + 2);
      v326 = -61;
      v327 = v119 + 2;
      FUN_0053cc30((int)v328, (int)&v333[v116], v117);
      v120 = (signed int)Positione;
      v121 = 0;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v122 = send(s, &v326 + v121, (int)Positione - v121, 0);
          v123 = v122;
          if ( v122 == -1 )
          {
            break;
          }
          if ( v122 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&v326, v122);
            }
            v120 -= v123;
            v121 += v123;
            if ( v120 > 0 )
            {
              continue;
            }
          }
          goto LABEL_304;
        }
        if ( WSAGetLastError() == 10035 && (int)Positione + DAT_055cc16c <= 0x2000 )
        {
LABEL_194:
          qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v326, v120);
          v124 = v120 + DAT_055cc16c;
LABEL_303:
          DAT_055cc16c = v124;
          goto LABEL_304;
        }
        goto LABEL_300;
      }
LABEL_304:
      v334 = -1;
      goto LABEL_305;
    }
    v125 = (float *)(v119 + 3);
    v329 = -60;
    v331 = v119 + 3;
    v330 = (v119 + 3) / 256;
    FUN_0053cc30((int)v332, (int)v118, v117);
    v126 = 0;
    Positionf = v125;
    if ( s == -1 )
    {
      goto LABEL_304;
    }
    while ( 1 )
    {
      v127 = send(s, &v329 + v126, (int)v125 - v126, 0);
      v128 = v127;
      if ( v127 == -1 )
      {
        break;
      }
      if ( v127 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2((int)&v329, v127);
        }
        v126 += v128;
        Positionf = (float *)((char *)Positionf - v128);
        if ( (int)Positionf > 0 )
        {
          continue;
        }
      }
      goto LABEL_304;
    }
    if ( WSAGetLastError() != 10035 || (v129 = DAT_055cc16c, (int)v125 + DAT_055cc16c > 0x2000) )
    {
LABEL_300:
      CWsctlc::Close((DWORD)&SocketClient);
      goto LABEL_304;
    }
    v130 = Positionf;
    v131 = (unsigned int)Positionf;
    goto LABEL_302;
  }
  if ( v317 == 56 )
  {
    v132 = (void *)CharacterMachine;
    v133 = *(float *)(o + 32);
    Angle[0] = *(float *)(o + 28);
    v134 = *(float *)SkillIndex - 40.0;
    Angle[1] = v133;
    *(float *)SkillIndex = 0.0;
    Angle[2] = v134;
    if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
    {
      v137 = operator_new(0x585u);
      *(BYTE *)(v137 + 1412) = 1;
      FUN_00403f80(&DAT_055c9bc8, v137, (int)v132);
    }
    else
    {
      v135 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v132);
      v136 = v135[1412] + 1;
      v135[1412] = v136;
      if ( v136 < 2u )
      {
        FUN_00404370(v132, v135);
      }
    }
    v138 = 0.0;
    while ( *(BYTE *)(CharacterAttribute + LODWORD(v138) + 87) != 56 )
    {
      ++LODWORD(v138);
      if ( SLODWORD(v138) >= 20 )
      {
        goto LABEL_216;
      }
    }
    *(float *)SkillIndex = v138;
LABEL_216:
    STRUCT_ENCRYPT(&DAT_055c9bc8, (const void *)CharacterMachine);
    (WORD)(v139) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, v139, *(int *)SkillIndex, 0);
    v140 = (void *)CharacterMachine;
    *(DWORD *)v318 = 0;
    Angle[2] = Angle[2] + 20.0;
    if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
    {
      v143 = operator_new(0x585u);
      *(BYTE *)(v143 + 1412) = 1;
      FUN_00403f80(&DAT_055c9bc8, v143, (int)v140);
    }
    else
    {
      v141 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v140);
      v142 = v141[1412] + 1;
      v141[1412] = v142;
      if ( v142 < 2u )
      {
        FUN_00404370(v140, v141);
      }
    }
    v144 = 0;
    while ( *(BYTE *)(CharacterAttribute + v144 + 87) != 56 )
    {
      if ( ++v144 >= 20 )
      {
        v145 = *(DWORD *)v318;
        goto LABEL_225;
      }
    }
    v145 = v144;
LABEL_225:
    STRUCT_ENCRYPT(&DAT_055c9bc8, (const void *)CharacterMachine);
    (WORD)(v146) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, v146, v145, 0);
    v147 = 0;
    Angle[2] = Angle[2] + 20.0;
    STRUCT_DECRYPT(&DAT_055c9bc8, (void *)CharacterMachine);
    v148 = 0;
    while ( *(BYTE *)(CharacterAttribute + v148 + 87) != 56 )
    {
      if ( ++v148 >= 20 )
      {
        goto LABEL_230;
      }
    }
    v147 = v148;
LABEL_230:
    *(float *)SkillIndex = *(float *)&CharacterMachine;
    v149 = (BYTE *)FUN_004041e0(&DAT_055c9bc8, CharacterMachine);
    if ( v149 != (BYTE *)-1 )
    {
      v149 = (BYTE *)FUN_00404280(&DAT_055c9bc8, *(int *)SkillIndex);
      v150 = v149[1412] - 1;
      v149[1412] = v150;
      if ( !v150 )
      {
        FUN_00404400(v149, *(const void **)SkillIndex);
      }
    }
    (WORD)(v149) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, (int)v149, v147, 0);
    v151 = 0;
    Angle[2] = Angle[2] + 20.0;
    STRUCT_DECRYPT(&DAT_055c9bc8, (void *)CharacterMachine);
    v152 = 0;
    while ( *(BYTE *)(CharacterAttribute + v152 + 87) != 56 )
    {
      if ( ++v152 >= 20 )
      {
        goto LABEL_238;
      }
    }
    v151 = v152;
LABEL_238:
    *(float *)SkillIndex = *(float *)&CharacterMachine;
    v153 = (BYTE *)FUN_004041e0(&DAT_055c9bc8, CharacterMachine);
    if ( v153 != (BYTE *)-1 )
    {
      v153 = (BYTE *)FUN_00404280(&DAT_055c9bc8, *(int *)SkillIndex);
      v154 = v153[1412] - 1;
      v153[1412] = v154;
      if ( !v154 )
      {
        FUN_00404400(v153, *(const void **)SkillIndex);
      }
    }
    (WORD)(v153) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, (int)v153, v151, 0);
    *(float *)SkillIndex = 0.0;
    Angle[2] = Angle[2] + 20.0;
    STRUCT_DECRYPT(&DAT_055c9bc8, (void *)CharacterMachine);
    v155 = 0.0;
    while ( *(BYTE *)(CharacterAttribute + LODWORD(v155) + 87) != 56 )
    {
      ++LODWORD(v155);
      if ( SLODWORD(v155) >= 20 )
      {
        goto LABEL_246;
      }
    }
    *(float *)SkillIndex = v155;
LABEL_246:
    v156 = (const void *)CharacterMachine;
    if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) != -1 )
    {
      v158 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v156);
      v159 = v158[1412] - 1;
      v158[1412] = v159;
      if ( !v159 )
      {
        FUN_00404400(v158, v156);
      }
    }
    (WORD)(v157) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, v157, *(int *)SkillIndex, 0);
  }
  else
  {
    v160 = Hero + 449;
    v161 = strlen(aWebzen_10) + 1;
    v162 = v161 - 1;
    v163 = strlen((const char *)(Hero + 449)) + 1;
    *(DWORD *)v318 = Hero + 449;
    if ( (int)(v163 - v161) < 0 )
    {
LABEL_258:
      if ( (int)abs32(GetTickCount() - g_dwLatestMagicTick) <= 300 )
      {
        goto LABEL_305;
      }
      g_dwLatestMagicTick = GetTickCount();
      v324 = &DAT_00552460;
      buf[1] = 449;
      v334 = 3;
      (BYTE)(buf[2]) = 25;
      buf[0] = 3;
      v251 = -25;
      v252 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v317;
      v253 = 58;
      v254 = -119;
      v255 = -68;
      v166 = buf[0] + 1;
      v256 = -78;
      v257 = -97;
      v258 = 115;
      v259 = 35;
      v260 = -88;
      v261 = -2;
      v262 = -74;
      v263 = 73;
      v264 = 93;
      v265 = 57;
      v266 = 93;
      v267 = -118;
      v268 = -53;
      v269 = 99;
      v270 = -115;
      v271 = -22;
      v272 = 125;
      v273 = 43;
      v274 = 95;
      v275 = -61;
      v276 = -79;
      v277 = -23;
      v278 = -125;
      v279 = 41;
      v280 = 81;
      v281 = -24;
      v282 = 86;
      for ( i7 = buf[0]; i7 != v166; ++i7 )
      {
        *((BYTE *)&buf[1] + i7) ^= *((BYTE *)buf + i7 + 1) ^ *(&v251 + i7 % 32);
      }
      ++buf[0];
      v168 = 229 * MovementSkillTarget;
      v169 = (WORD *)(CharactersClient + 916 * MovementSkillTarget + 476);
      (WORD)(v168) = *v169;
      v170 = v168 >> 8;
      if ( buf[0] + 1 <= 1024 )
      {
        v251 = -25;
        v252 = 109;
        v253 = 58;
        *((BYTE *)&buf[1] + buf[0]) = v170;
        v254 = -119;
        v255 = -68;
        v256 = -78;
        v171 = buf[0] + 1;
        v257 = -97;
        v258 = 115;
        v259 = 35;
        v260 = -88;
        v261 = -2;
        v262 = -74;
        v263 = 73;
        v264 = 93;
        v265 = 57;
        v266 = 93;
        v267 = -118;
        v268 = -53;
        v269 = 99;
        v270 = -115;
        v271 = -22;
        v272 = 125;
        v273 = 43;
        v274 = 95;
        v275 = -61;
        v276 = -79;
        v277 = -23;
        v278 = -125;
        v279 = 41;
        v280 = 81;
        v281 = -24;
        v282 = 86;
        for ( i8 = buf[0]; i8 != v171; ++i8 )
        {
          *((BYTE *)&buf[1] + i8) ^= *((BYTE *)buf + i8 + 1) ^ *(&v251 + i8 % 32);
        }
        ++buf[0];
      }
      v173 = *(BYTE *)v169;
      if ( buf[0] + 1 <= 1024 )
      {
        v251 = -25;
        v252 = 109;
        v253 = 58;
        *((BYTE *)&buf[1] + buf[0]) = v173;
        v254 = -119;
        v255 = -68;
        v256 = -78;
        v174 = buf[0] + 1;
        v257 = -97;
        v258 = 115;
        v259 = 35;
        v260 = -88;
        v261 = -2;
        v262 = -74;
        v263 = 73;
        v264 = 93;
        v265 = 57;
        v266 = 93;
        v267 = -118;
        v268 = -53;
        v269 = 99;
        v270 = -115;
        v271 = -22;
        v272 = 125;
        v273 = 43;
        v274 = 95;
        v275 = -61;
        v276 = -79;
        v277 = -23;
        v278 = -125;
        v279 = 41;
        v280 = 81;
        v281 = -24;
        v282 = 86;
        for ( i9 = buf[0]; i9 != v174; ++i9 )
        {
          *((BYTE *)&buf[1] + i9) ^= *((BYTE *)buf + i9 + 1) ^ *(&v251 + i9 % 32);
        }
        ++buf[0];
      }
      if ( (BYTE)(buf[1]) == 193 )
      {
        (BYTE)((buf[1]) >> 8) = buf[0];
      }
      else if ( (BYTE)(buf[1]) == 194 )
      {
        *(WORD *)((char *)&buf[1] + 1) = buf[0];
      }
      v176 = buf[0];
      qmemcpy(v333, &buf[1], buf[0]);
      v333[v176] = rand();
      v177 = (v333[0] != -63) + 2;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
      {
        v180 = operator_new(2u);
        *(BYTE *)(v180 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v180, (int)&g_byPacketSerialSend);
      }
      else
      {
        v178 = (char *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        v179 = v178[1] + 1;
        v178[1] = v179;
        if ( (unsigned char)v179 < 2u )
        {
          FUN_00404330(&g_byPacketSerialSend, v178);
        }
      }
      v181 = g_byPacketSerialSend;
      v332[v177 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v181 + 1;
      PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
      v182 = v177 - 1;
      v183 = v176 - v182;
      v184 = &v333[v182];
      v185 = FUN_0053cc30(0, (int)&v333[v182], v183);
      if ( v185 < 256 )
      {
        Positiong = (float *)(v185 + 2);
        v326 = -61;
        v327 = v185 + 2;
        FUN_0053cc30((int)v328, (int)&v333[v182], v183);
        v120 = (signed int)Positiong;
        v186 = 0;
        if ( s == -1 )
        {
          goto LABEL_304;
        }
        while ( 1 )
        {
          v187 = send(s, &v326 + v186, (int)Positiong - v186, 0);
          v188 = v187;
          if ( v187 == -1 )
          {
            break;
          }
          if ( v187 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&v326, v187);
            }
            v120 -= v188;
            v186 += v188;
            if ( v120 > 0 )
            {
              continue;
            }
          }
          goto LABEL_304;
        }
        if ( WSAGetLastError() == 10035 && (int)Positiong + DAT_055cc16c <= 0x2000 )
        {
          goto LABEL_194;
        }
        goto LABEL_300;
      }
      v189 = (float *)(v185 + 3);
      v329 = -60;
      v331 = v185 + 3;
      v330 = (v185 + 3) / 256;
      FUN_0053cc30((int)v332, (int)v184, v183);
      v190 = 0;
      Positionh = v189;
      if ( s == -1 )
      {
        goto LABEL_304;
      }
      while ( 1 )
      {
        v191 = send(s, &v329 + v190, (int)v189 - v190, 0);
        v192 = v191;
        if ( v191 == -1 )
        {
          break;
        }
        if ( v191 )
        {
          if ( DAT_055ce174 )
          {
            nullsub_2((int)&v329, v191);
          }
          v190 += v192;
          Positionh = (float *)((char *)Positionh - v192);
          if ( (int)Positionh > 0 )
          {
            continue;
          }
        }
        goto LABEL_304;
      }
      if ( WSAGetLastError() != 10035 )
      {
        goto LABEL_300;
      }
      v129 = DAT_055cc16c;
      if ( (int)v189 + DAT_055cc16c > 0x2000 )
      {
        goto LABEL_300;
      }
      v130 = Positionh;
      v131 = (unsigned int)Positionh;
LABEL_302:
      qmemcpy((char *)&DAT_055ca16c + v129, &v329, v131);
      v124 = (int)v130 + DAT_055cc16c;
      goto LABEL_303;
    }
    v164 = 0;
    while ( 1 )
    {
      v165 = 0;
      if ( v162 <= 0 )
      {
        break;
      }
      while ( *(BYTE *)(v165 + v164 + v160) == aWebzen_10[v165] )
      {
        if ( ++v165 >= v162 )
        {
          goto LABEL_305;
        }
      }
      if ( ++v164 > (int)(v163 - v161) )
      {
        goto LABEL_258;
      }
      v160 = *(DWORD *)v318;
    }
  }
LABEL_305:
  v193 = *(DWORD *)(o + 120);
  *(BYTE *)(c + 757) = 1;
  if ( (v193 & 0x20) != 32 )
  {
    v194 = *(float *)(c + 792) * 0.0099999998;
    (BYTE)(v284) = (__int64)(*(float *)(c + 788) * 0.0099999998);
    Positioni = (__int64)v194;
    if ( World >= 11 && World <= 16 )
    {
      switch ( abs32((__int64)(*(float *)(o + 36) * 0.022222223)) )
      {
        case 0u:
          goto LABEL_317;
        case 1u:
          v195 = v284 - 1;
          goto LABEL_316;
        case 2u:
          (BYTE)(v284) = v284 - 1;
          break;
        case 3u:
          v196 = Positioni - 1;
          (BYTE)(v284) = v284 - 1;
          goto LABEL_318;
        case 4u:
          goto LABEL_313;
        case 5u:
          (BYTE)(v284) = v284 + 1;
LABEL_313:
          v196 = Positioni - 1;
          goto LABEL_318;
        case 6u:
          (BYTE)(v284) = v284 + 1;
          break;
        case 7u:
          v195 = v284 + 1;
LABEL_316:
          (BYTE)(v284) = v195;
LABEL_317:
          v196 = Positioni + 1;
LABEL_318:
          Positioni = v196;
          break;
        default:
          break;
      }
    }
    v197 = TerrainWall[TERRAIN_INDEX((unsigned char)v284, Positioni)];
    if ( (v197 & 4) != 4 && (v197 & 8) != 8 && v317 != 47 && v317 != 43 && v317 != 49 )
    {
      v324 = &DAT_00552460;
      buf[1] = 449;
      v334 = 4;
      (BYTE)(buf[2]) = 17;
      buf[0] = 3;
      v251 = -25;
      v252 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v284;
      v198 = buf[0];
      v199 = buf[0] + 1;
      v253 = 58;
      v254 = -119;
      v255 = -68;
      v256 = -78;
      v257 = -97;
      v258 = 115;
      v259 = 35;
      v260 = -88;
      v261 = -2;
      v262 = -74;
      v263 = 73;
      v264 = 93;
      v265 = 57;
      v266 = 93;
      v267 = -118;
      v268 = -53;
      v269 = 99;
      v270 = -115;
      v271 = -22;
      v272 = 125;
      v273 = 43;
      v274 = 95;
      v275 = -61;
      v276 = -79;
      v277 = -23;
      v278 = -125;
      v279 = 41;
      v280 = 81;
      v281 = -24;
      v282 = 86;
      if ( buf[0] != v199 )
      {
        do
        {
          *((BYTE *)&buf[1] + v198) ^= *((BYTE *)buf + v198 + 1) ^ *(&v251 + v198 % 32);
          ++v198;
        }
        while ( v198 != v199 );
      }
      if ( ++buf[0] + 1 <= 1024 )
      {
        v251 = -25;
        v252 = 109;
        *((BYTE *)&buf[1] + buf[0]) = Positioni;
        v200 = buf[0];
        v253 = 58;
        v201 = buf[0] + 1;
        v254 = -119;
        v255 = -68;
        v256 = -78;
        v257 = -97;
        v258 = 115;
        v259 = 35;
        v260 = -88;
        v261 = -2;
        v262 = -74;
        v263 = 73;
        v264 = 93;
        v265 = 57;
        v266 = 93;
        v267 = -118;
        v268 = -53;
        v269 = 99;
        v270 = -115;
        v271 = -22;
        v272 = 125;
        v273 = 43;
        v274 = 95;
        v275 = -61;
        v276 = -79;
        v277 = -23;
        v278 = -125;
        v279 = 41;
        v280 = 81;
        v281 = -24;
        v282 = 86;
        if ( buf[0] != v201 )
        {
          do
          {
            *((BYTE *)&buf[1] + v200) ^= *((BYTE *)buf + v200 + 1) ^ *(&v251 + v200 % 32);
            ++v200;
          }
          while ( v200 != v201 );
        }
        ++buf[0];
      }
      if ( (BYTE)(buf[1]) == 193 )
      {
        (BYTE)((buf[1]) >> 8) = buf[0];
      }
      else if ( (BYTE)(buf[1]) == 194 )
      {
        *(WORD *)((char *)&buf[1] + 1) = buf[0];
      }
      v202 = buf[0];
      v203 = 0;
      v204 = buf[0];
      if ( s != -1 )
      {
        while ( 1 )
        {
          v205 = send(s, (const char *)&buf[1] + v203, v202 - v203, 0);
          v206 = v205;
          if ( v205 == -1 )
          {
            break;
          }
          if ( v205 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&buf[1], v205);
            }
            v204 -= v206;
            v203 += v206;
            if ( v204 > 0 )
            {
              continue;
            }
          }
          return;
        }
        if ( WSAGetLastError() == 10035 && DAT_055cc16c + v202 <= 0x2000 )
        {
          qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf[1], v204);
          DAT_055cc16c += v204;
        }
        else
        {
          CWsctlc::Close((DWORD)&SocketClient);
        }
      }
    }
  }
}
#endif

// ── FUN_0048b680 (IDA-only, gated) ──
#if defined(IDA_PORT_0048B680)
void __cdecl FUN_0048b680(int a1)
{
  void *v1; // ebx
  unsigned int v2; // eax
  bool v3; // cf
  int v4; // eax
  const void *v5; // ebx
  unsigned int v6; // edx
  unsigned int v7; // eax
  BYTE *v8; // eax
  unsigned char v9; // cl
  unsigned int v10; // eax
  char *v11; // eax
  char v12; // cl
  char *v13; // ebp
  unsigned int v14; // esi
  int v15; // ebx
  char *v16; // edi
  char v17; // al
  short v18; // cx
  char v19; // si
  short v20; // ax
  char v21; // si
  char v22; // [esp+Bh] [ebp-19h]
  int v23; // [esp+Ch] [ebp-18h]
  char *v24; // [esp+Ch] [ebp-18h]
  short *v25; // [esp+10h] [ebp-14h]
  int v26; // [esp+14h] [ebp-10h] BYREF
  DWORD v27; // [esp+18h] [ebp-Ch] BYREF
  int v28; // [esp+1Ch] [ebp-8h] BYREF
  short *v29; // [esp+20h] [ebp-4h]
  void *retaddr; // [esp+24h] [ebp+0h]

  v26 = FUN_004824c0();
  if ( v26 != -1 )
  {
    v1 = (void *)CharacterMachine;
    v27 = CharacterMachine;
    v2 = (*(int (__cdecl **)(int *, DWORD, int))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine, a1);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v2), 4) )
      {
        if ( !memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v2), 4) )
        {
          if ( v2 == -1 )
          {
            break;
          }
          v7 = FUN_004041e0(&DAT_055c9bc8, (int)v1);
          if ( v7 == -1 )
          {
            v8 = 0;
          }
          else
          {
            v8 = *(BYTE **)(DAT_055c9bcc + 4 * v7);
          }
          v9 = v8[1412] + 1;
          v8[1412] = v9;
          if ( v9 < 2u )
          {
            FUN_00404370(v1, v8);
          }
          goto LABEL_8;
        }
        v3 = (unsigned int)v25 + 1 < DAT_055c9bd4;
        v25 = (short *)((char *)v25 + 1);
        v2 = (v2 + 1) % DAT_055c9bd4;
        if ( !v3 )
        {
          goto LABEL_6;
        }
      }
    }
    else
    {
LABEL_6:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    v4 = operator_new(0x585u);
    *(BYTE *)(v4 + 1412) = 1;
    FUN_00403f80(&DAT_055c9bc8, v4, (int)v1);
LABEL_8:
    v5 = (const void *)CharacterMachine;
    if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 && !DAT_07e91388 )
    {
      v26 = CharacterMachine + 536;
      retaddr = (void *)(CharacterMachine + 604);
    }
    v29 = (short *)CharacterMachine;
    v6 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v27 = 0;
    v23 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
      {
        if ( !memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
        {
          if ( v6 != -1 )
          {
            v10 = FUN_004041e0(&DAT_055c9bc8, (int)v5);
            if ( v10 == -1 )
            {
              v24 = 0;
              v11 = 0;
            }
            else
            {
              v11 = *(char **)(DAT_055c9bcc + 4 * v10);
              v24 = v11;
            }
            v12 = v11[1412] - 1;
            v11[1412] = v12;
            if ( !v12 )
            {
              v13 = (char *)operator_new(0x584u);
              qmemcpy(v13, v5, 0x584u);
              v14 = 0;
              v15 = (BYTE *)v5 - v13;
              do
              {
                v16 = &v13[v14];
                v17 = ((v13[v14] + 71) ^ DAT_00559050[v14 & 0x8000000F]) + 35;
                v13[v14] = v17;
                if ( v14 < 0x583 )
                {
                  *v16 = v17 ^ v13[v14 + 1];
                }
                ++v14;
                v16[v15] = rand();
              }
              while ( v14 < 0x584 );
              qmemcpy(v24, v13, 0x584u);
              delete__(v13);
            }
          }
          break;
        }
        v6 = (v6 + 1) % DAT_055c9bd4;
        if ( ++v23 >= (unsigned int)DAT_055c9bd4 )
        {
          goto LABEL_15;
        }
      }
    }
    else
    {
LABEL_15:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v22 )
    {
      v18 = *v29;
      if ( (*v29 >= 128 && v18 < 135 || v18 == 145) && *v25 == -1 )
      {
        qmemcpy(&pPickedItem, &OffsetInventoryItems + v26, 0x44u);
        v19 = v26 + 12;
        *(DWORD *)&Inventory[32].Type = v26 + 12;
        UI_Main(v26 + 12, &OffsetInventoryItems.Type, 8u);
        DAT_07e11e78 = 0;
        SendRequestEquipmentItem(0, v19, 0, 0);
        UIChatLogWindow_AddText(DAT_07e11dec, GlobalText[250], 1);
      }
      else
      {
        v20 = *v25;
        if ( *v25 >= 136 && v20 < 143 || v20 >= 144 && v20 < 160 && v18 == -1 )
        {
          qmemcpy(&pPickedItem, &OffsetInventoryItems + v26, 0x44u);
          v21 = v26 + 12;
          *(DWORD *)&Inventory[32].Type = v26 + 12;
          UI_Main(v26 + 12, &OffsetInventoryItems.Type, 8u);
          DAT_07e11e78 = 1;
          SendRequestEquipmentItem(0, v21, 0, 1);
          UIChatLogWindow_AddText(DAT_07e11df0, GlobalText[250], 1);
        }
      }
    }
  }
}
#endif

// ── FUN_004c3dd0 (IDA-only, gated) ──
#if defined(IDA_PORT_004C3DD0)
int __cdecl FUN_004c3dd0(int a1)
{
  if ( a1 >= (int)&DAT_00989680 )
  {
    return -16776961;
  }
  if ( a1 < 1000000 )
  {
    return a1 < 100000 ? -6890241 : -15152896;
  }
  return -16738561;
}
#endif

// ── FUN_004c4080 (IDA-only, gated) ──
#if defined(IDA_PORT_004C4080)
void FUN_004c4080()
{
  void *v0; // ebx
  unsigned int v1; // eax
  bool v2; // cf
  int v3; // eax
  int v4; // ebx
  int i; // edi
  short *v6; // esi
  WORD v7; // ax
  short v8; // cx
  int v9; // eax
  ITEM *v10; // esi
  int v11; // edi
  WORD v12; // ax
  short Type; // cx
  int v14; // eax
  unsigned int v15; // edx
  unsigned int v16; // eax
  BYTE *v17; // esi
  unsigned char v18; // al
  void *v19; // ebp
  unsigned int v20; // ecx
  int v21; // esi
  unsigned int v22; // edx
  int v23; // ebx
  char *v24; // edx
  char v25; // al
  BYTE *v26; // eax
  char *v27; // ebx
  BYTE *v28; // ebp
  unsigned int v29; // esi
  int v30; // ebx
  BYTE *v31; // edi
  char v32; // al
  int v33; // [esp-10h] [ebp-90h]
  int Durability; // [esp-10h] [ebp-90h]
  int v35; // [esp-Ch] [ebp-8Ch]
  int v36; // [esp-Ch] [ebp-8Ch]
  short v37; // [esp-8h] [ebp-88h]
  short v38; // [esp-8h] [ebp-88h]
  char *v39; // [esp+10h] [ebp-70h] BYREF
  char *v40; // [esp+14h] [ebp-6Ch] BYREF
  DWORD v41; // [esp+18h] [ebp-68h] BYREF
  char Buffer[100]; // [esp+1Ch] [ebp-64h] BYREF

  v0 = (void *)CharacterMachine;
  DAT_07eaa0f8 = 0;
  v41 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v39 = 0;
  v40 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v39, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v41, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v16 = FUN_004041e0(&DAT_055c9bc8, (int)v0);
        if ( v16 == -1 )
        {
          v17 = 0;
        }
        else
        {
          v17 = *(BYTE **)(DAT_055c9bcc + 4 * v16);
        }
        v18 = v17[1412] + 1;
        v17[1412] = v18;
        if ( v18 < 2u )
        {
          v19 = (void *)operator_new(0x584u);
          qmemcpy(v19, v17, 0x584u);
          v20 = 1411;
          v21 = 1412;
          do
          {
            if ( v20 < 0x583 )
            {
              *((BYTE *)v19 + v20) ^= *((BYTE *)v19 + v20 + 1);
            }
            *((BYTE *)v19 + v20) = ((*((BYTE *)v19 + v20) - 35) ^ DAT_00559050[(int)v20 % 16]) - 71;
            --v20;
            --v21;
          }
          while ( v21 );
          qmemcpy(v0, v19, 0x584u);
          delete__(v19);
        }
        goto LABEL_7;
      }
      v2 = (unsigned int)++v40 < DAT_055c9bd4;
      v1 = (v1 + 1) % DAT_055c9bd4;
      if ( !v2 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v3 = operator_new(0x585u);
  *(BYTE *)(v3 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v3, (int)v0);
LABEL_7:
  v4 = 0;
  for ( i = 0; i < 816; i += 68 )
  {
    v6 = (short *)(i + CharacterMachine + 536);
    if ( *v6 != -1 )
    {
      if ( *(DWORD *)(i + CharacterMachine + 592) )
      {
        v7 = CalcMaxDurability(
               (ITEM *)(i + CharacterMachine + 536),
               &ItemAttribute[*v6],
               (*(int *)(i + CharacterMachine + 540) >> 3) & 0xF);
        v8 = *v6;
        if ( (*v6 < 416 || v8 > 419)
          && v8 != 426
          && v8 != 135
          && v8 != 143
          && v8 < 448
          && (v8 < 391 || v8 > 403)
          && (v8 < 430 || v8 > 435)
          && *((unsigned char *)v6 + 26) < (int)v7 )
        {
          v37 = *v6;
          v35 = v7;
          v33 = *((unsigned char *)v6 + 26);
          v9 = ItemValue((int)v6, 2);
          DAT_07eaa0f8 += ConvertRepairGold(v9, v33, v35, v37, Buffer);
        }
      }
    }
  }
  v10 = &OffsetInventoryItems;
  do
  {
    v11 = 8;
    do
    {
      if ( v10->Key )
      {
        v12 = CalcMaxDurability(v10, &ItemAttribute[v10->Type], (v10->Level >> 3) & 0xF);
        Type = v10->Type;
        if ( (v10->Type < 416 || Type > 419)
          && Type != 426
          && Type != 135
          && Type != 143
          && Type < 448
          && (Type < 391 || Type > 403)
          && (Type < 430 || Type > 435)
          && v10->Durability < (int)v12 )
        {
          v38 = v10->Type;
          v36 = v12;
          Durability = v10->Durability;
          v14 = ItemValue((int)v10, 2);
          DAT_07eaa0f8 += ConvertRepairGold(v14, Durability, v36, v38, Buffer);
        }
      }
      ++v10;
      --v11;
    }
    while ( v11 );
  }
  while ( (int)v10 < (int)&DAT_07ea9510 );
  v39 = (char *)CharacterMachine;
  v40 = (char *)CharacterMachine;
  v15 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v41 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v41, (const char *)(DAT_055c9bd0 + 4 * v15), 4) )
    {
      if ( !memcmp((const char *)&v40, (const char *)(DAT_055c9bd0 + 4 * v15), 4) )
      {
        if ( v15 != -1 )
        {
          v40 = v39;
          v22 = (*(int (__cdecl **)(int *, char *))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, v39);
          v23 = 0;
          v41 = 0;
          if ( DAT_055c9bd4 )
          {
            while ( memcmp((const char *)&v41, (const char *)(DAT_055c9bd0 + 4 * v22), 4) )
            {
              if ( !memcmp((const char *)&v40, (const char *)(DAT_055c9bd0 + 4 * v22), 4) )
              {
                if ( v22 == -1 )
                {
                  break;
                }
                v24 = *(char **)(DAT_055c9bcc + 4 * v22);
                v40 = v24;
                goto LABEL_62;
              }
              v22 = (v22 + 1) % DAT_055c9bd4;
              if ( ++v23 >= (unsigned int)DAT_055c9bd4 )
              {
                goto LABEL_60;
              }
            }
          }
          else
          {
LABEL_60:
            CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
          }
          v40 = 0;
          v24 = 0;
LABEL_62:
          v25 = v24[1412] - 1;
          v24[1412] = v25;
          if ( !v25 )
          {
            v26 = (BYTE *)operator_new(0x584u);
            v27 = v39;
            v28 = v26;
            qmemcpy(v26, v39, 0x584u);
            v29 = 0;
            v30 = v27 - v26;
            do
            {
              v31 = &v28[v29];
              v32 = ((v28[v29] + 71) ^ DAT_00559050[v29 & 0x8000000F]) + 35;
              v28[v29] = v32;
              if ( v29 < 0x583 )
              {
                *v31 = v32 ^ v28[v29 + 1];
              }
              ++v29;
              v31[v30] = rand();
            }
            while ( v29 < 0x584 );
            qmemcpy(v40, v28, 0x584u);
            delete__(v28);
          }
        }
        return;
      }
      v15 = (v15 + 1) % DAT_055c9bd4;
      if ( ++v4 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_44;
      }
    }
  }
  else
  {
LABEL_44:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
}
#endif

// ── FUN_004cbdd0 (IDA-only, gated) ──
#if defined(IDA_PORT_004CBDD0)
int *FUN_004cbdd0()
{
  int *result; // eax

  result = DAT_07e11fb0;
  do
  {
    *((WORD *)result - 28) = -1;
    *result = 0;
    result += 17;
  }
  while ( (int)result < (int)&Items[0][56] );
  return result;
}
#endif

// ── FUN_004cdc70 (IDA-only, gated) ──
#if defined(IDA_PORT_004CDC70)
void __cdecl FUN_004cdc70(float a1, float a2, float a3, float a4, int a5)
{
  int v5; // edi
  char *v6; // ebp
  void *v7; // esi
  unsigned int v8; // eax
  BYTE *v9; // eax
  unsigned char v10; // cl
  int v11; // eax
  WORD *v12; // edx
  void *v13; // esi
  unsigned int v14; // eax
  BYTE *v15; // eax
  unsigned char v16; // cl
  int v17; // eax
  int v18; // esi
  int v19; // edi
  int v20; // ebp
  unsigned int v21; // eax
  BYTE *v22; // eax
  char v23; // cl
  ITEM_ATTRIBUTE *v24; // edx
  int v25; // eax
  void *v26; // ebx
  unsigned int v27; // edx
  int v28; // ecx
  bool v29; // cf
  bool v30; // zf
  BYTE *v31; // edi
  int v32; // ecx
  BYTE *v33; // edi
  bool v34; // cf
  bool v35; // zf
  int v36; // eax
  BYTE *v37; // eax
  unsigned char v38; // cl
  const void *v39; // ebx
  unsigned int v40; // edx
  int v41; // ecx
  bool v42; // cf
  bool v43; // zf
  BYTE *v44; // edi
  int v45; // ecx
  BYTE *v46; // edi
  bool v47; // cf
  bool v48; // zf
  BYTE *v49; // eax
  char v50; // cl
  void *v51; // ebx
  unsigned int v52; // edx
  int v53; // ecx
  bool v54; // cf
  bool v55; // zf
  BYTE *v56; // edi
  int v57; // ecx
  BYTE *v58; // edi
  bool v59; // cf
  bool v60; // zf
  int v61; // eax
  unsigned int v62; // edx
  bool v63; // cf
  bool v64; // zf
  int v65; // ecx
  BYTE *v66; // edi
  int v67; // ecx
  BYTE *v68; // edi
  bool v69; // cf
  bool v70; // zf
  BYTE *v71; // eax
  unsigned char v72; // cl
  char v73; // cl
  void *v74; // ebx
  unsigned int v75; // edx
  int v76; // ecx
  bool v77; // cf
  bool v78; // zf
  BYTE *v79; // edi
  int v80; // ecx
  BYTE *v81; // edi
  bool v82; // cf
  bool v83; // zf
  int v84; // eax
  const void *v85; // ebx
  unsigned int v86; // edx
  int v87; // ecx
  bool v88; // cf
  bool v89; // zf
  BYTE *v90; // edi
  int v91; // ecx
  BYTE *v92; // edi
  bool v93; // cf
  bool v94; // zf
  BYTE *v95; // eax
  unsigned char v96; // cl
  BYTE *v97; // eax
  char v98; // cl
  short v99; // dx
  int v100; // eax
  int v101; // esi
  int v102; // esi
  int v103; // edi
  int v104; // esi
  int v105; // edi
  int v106; // esi
  int v107; // edi
  int v108; // esi
  int v109; // edi
  int v110; // esi
  int v111; // edi
  int v112; // esi
  int v113; // edi
  int v114; // edi
  int j; // eax
  int v116; // ebx
  int v117; // esi
  BYTE v118; // al
  int v119; // edi
  char *v120; // esi
  int v121; // eax
  signed int v122; // ebx
  int v123; // edi
  signed int v124; // ebp
  int v125; // eax
  int v126; // esi
  signed int v127; // ebp
  int v128; // edi
  signed int v129; // ebx
  int v130; // eax
  int v131; // esi
  int v132; // ecx
  short *v133; // eax
  int v134; // edi
  int v135; // esi
  int v136; // edx
  int v137; // esi
  int v138; // esi
  int v139; // edi
  int v140; // esi
  int v141; // edi
  int v142; // esi
  int v143; // edi
  int v144; // esi
  int v145; // edi
  int v146; // esi
  int v147; // edi
  int v148; // esi
  int v149; // edi
  int v150; // edi
  int n; // eax
  int v152; // ebx
  int v153; // esi
  BYTE v154; // al
  int v155; // edi
  char *v156; // esi
  int v157; // eax
  signed int v158; // ebx
  int v159; // edi
  int v160; // eax
  int v161; // esi
  signed int v162; // ebp
  int v163; // edi
  int v164; // eax
  int v165; // esi
  int v166; // esi
  int v167; // esi
  int v168; // edi
  int v169; // esi
  int v170; // edi
  int v171; // esi
  int v172; // edi
  int v173; // esi
  int v174; // edi
  int v175; // esi
  int v176; // edi
  int v177; // esi
  int v178; // edi
  int v179; // edi
  int k; // eax
  int v181; // ebx
  int v182; // esi
  BYTE v183; // al
  BYTE *v184; // eax
  char v185; // cl
  int v186; // esi
  int v187; // edi
  char *v188; // ebp
  int v189; // eax
  signed int v190; // ebx
  int v191; // edi
  int v192; // eax
  int v193; // esi
  signed int v194; // esi
  int v195; // ebx
  signed int v196; // ebp
  int v197; // eax
  int v198; // edi
  int v199; // eax
  int v200; // esi
  int v201; // esi
  int v202; // edi
  int v203; // esi
  int v204; // edi
  int v205; // esi
  int v206; // edi
  int v207; // esi
  int v208; // edi
  int v209; // esi
  int v210; // edi
  int v211; // esi
  int v212; // edi
  int v213; // edi
  int m; // eax
  int v215; // ebx
  int v216; // esi
  BYTE v217; // al
  BYTE *v218; // eax
  char v219; // cl
  int v220; // esi
  int v221; // edi
  char *v222; // ebp
  int v223; // eax
  signed int v224; // ebx
  int v225; // edi
  int v226; // eax
  int v227; // esi
  signed int v228; // esi
  int v229; // ebx
  int v230; // eax
  int v231; // edi
  short v232; // cx
  int v233; // esi
  short v234; // ax
  int v235; // esi
  int v236; // edi
  int i; // eax
  int v238; // ebx
  int v239; // esi
  unsigned int v240; // eax
  char *v241; // eax
  char v242; // cl
  int v243; // eax
  BYTE v244; // al
  unsigned int v245; // eax
  BYTE *v246; // eax
  char v247; // cl
  int v248; // esi
  int v249; // edi
  char *v250; // ebp
  int v251; // eax
  signed int v252; // ebx
  int v253; // edi
  int v254; // eax
  int v255; // esi
  signed int v256; // esi
  int v257; // ebp
  int v258; // eax
  int v259; // edi
  unsigned int v260; // edx
  int v261; // ebx
  unsigned int v262; // eax
  unsigned int v263; // ebx
  char *v264; // eax
  char v265; // cl
  BYTE *v266; // eax
  BYTE *v267; // ebx
  BYTE *v268; // ebp
  unsigned int v269; // esi
  int v270; // ebx
  BYTE *v271; // edi
  char v272; // al
  int v273; // [esp-4h] [ebp-D6Ch]
  int v274; // [esp-4h] [ebp-D6Ch]
  DWORD v275; // [esp-4h] [ebp-D6Ch]
  char v276; // [esp+10h] [ebp-D58h]
  char v277; // [esp+11h] [ebp-D57h]
  char v278; // [esp+12h] [ebp-D56h]
  char v279; // [esp+13h] [ebp-D55h]
  char v280; // [esp+14h] [ebp-D54h]
  char v281; // [esp+15h] [ebp-D53h]
  char v282; // [esp+16h] [ebp-D52h]
  char v283; // [esp+17h] [ebp-D51h]
  char v284; // [esp+18h] [ebp-D50h]
  char v285; // [esp+19h] [ebp-D4Fh]
  char v286; // [esp+1Ah] [ebp-D4Eh]
  char v287; // [esp+1Bh] [ebp-D4Dh]
  char v288; // [esp+1Ch] [ebp-D4Ch]
  char v289; // [esp+1Dh] [ebp-D4Bh]
  char v290; // [esp+1Eh] [ebp-D4Ah]
  char v291; // [esp+1Fh] [ebp-D49h]
  char v292; // [esp+20h] [ebp-D48h]
  char v293; // [esp+21h] [ebp-D47h]
  char v294; // [esp+22h] [ebp-D46h]
  char v295; // [esp+23h] [ebp-D45h]
  char v296; // [esp+24h] [ebp-D44h]
  char v297; // [esp+25h] [ebp-D43h]
  char v298; // [esp+26h] [ebp-D42h]
  char v299; // [esp+27h] [ebp-D41h]
  char v300; // [esp+28h] [ebp-D40h]
  char v301; // [esp+29h] [ebp-D3Fh]
  char v302; // [esp+2Ah] [ebp-D3Eh]
  char v303; // [esp+2Bh] [ebp-D3Dh]
  int v304; // [esp+2Ch] [ebp-D3Ch]
  int v305; // [esp+30h] [ebp-D38h]
  char *v306; // [esp+34h] [ebp-D34h]
  BYTE *v307; // [esp+38h] [ebp-D30h] BYREF
  int v308; // [esp+3Ch] [ebp-D2Ch] BYREF
  BYTE *v309; // [esp+40h] [ebp-D28h]
  DWORD v310; // [esp+44h] [ebp-D24h] BYREF
  int v311; // [esp+48h] [ebp-D20h]
  void *(__cdecl **v312)(std::locale::facet *__hidden, unsigned int); // [esp+4Ch] [ebp-D1Ch]
  unsigned short v313; // [esp+50h] [ebp-D18h]
  unsigned char v314; // [esp+52h] [ebp-D16h] BYREF
  WORD v315[512]; // [esp+53h] [ebp-D15h] BYREF
  char buf; // [esp+454h] [ebp-914h] BYREF
  char v317; // [esp+455h] [ebp-913h]
  char v318[258]; // [esp+456h] [ebp-912h] BYREF
  char v319; // [esp+558h] [ebp-810h] BYREF
  char v320; // [esp+559h] [ebp-80Fh]
  char v321; // [esp+55Ah] [ebp-80Eh]
  char v322[1025]; // [esp+55Bh] [ebp-80Dh] BYREF
  char v323[1024]; // [esp+95Ch] [ebp-40Ch] BYREF
  int v324; // [esp+D64h] [ebp-4h]

  if ( EnableUse > 0 || EquipmentItem )
  {
    return;
  }
  v5 = InventoryStartX + (__int64)a1;
  v311 = v5;
  v6 = (char *)(InventoryStartY + (__int64)a2);
  v7 = (void *)CharacterMachine;
  v306 = v6;
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
  {
    v11 = operator_new(0x585u);
    *(BYTE *)(v11 + 1412) = 1;
    FUN_00403f80(&DAT_055c9bc8, v11, (int)v7);
  }
  else
  {
    v8 = FUN_004041e0(&DAT_055c9bc8, (int)v7);
    if ( v8 == -1 )
    {
      v9 = 0;
    }
    else
    {
      v9 = *(BYTE **)(DAT_055c9bcc + 4 * v8);
    }
    v10 = v9[1412] + 1;
    v9[1412] = v10;
    if ( v10 < 2u )
    {
      FUN_00404370(v7, v9);
    }
  }
  v12 = (WORD *)(CharacterMachine + 68 * a5 + 536);
  v309 = v12;
  if ( DAT_07e91388 <= 0 )
  {
    v232 = *v12;
    if ( *v12 == 0xFFFF
      || MouseX < v5
      || (double)v311 + a3 <= (double)MouseX
      || MouseY < (int)v6
      || (double)(int)v306 + a4 <= (double)MouseY )
    {
      goto LABEL_576;
    }
    DAT_07eaa164 = 1;
    if ( !MouseLButtonPush )
    {
      *(BYTE *)(CharacterMachine + 68 * a5 + 600) = 2;
      DAT_07ea9844 = 0;
      CheckInventory = (DWORD)v12;
      sy = (int)v6;
      sx = v5 + (__int64)a3 / 2;
      goto LABEL_576;
    }
    if ( Teleport )
    {
      goto LABEL_576;
    }
    if ( *(DWORD *)&RepairEnable_0 )
    {
      if ( v232 >= 416 && v232 <= 419
        || v232 == 426
        || v232 == 135
        || v232 == 143
        || v232 >= 448
        || v232 >= 391 && v232 <= 403
        || v232 >= 430 && v232 <= 435 )
      {
        goto LABEL_576;
      }
      MouseLButtonPush = 0;
      v312 = &DAT_00552460;
      v314 = -63;
      v324 = 4;
      v315[0] = 13313;
      v313 = 3;
      v276 = -25;
      v277 = 109;
      v278 = 58;
      (BYTE)(v315[1]) = a5;
      v235 = 3;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      do
      {
        *(&v314 + v235) ^= *((BYTE *)&v313 + v235 + 1) ^ *(&v276 + v235 % 32);
        ++v235;
      }
      while ( v235 != 4 );
      if ( ++v313 + 1 <= 1024 )
      {
        v289 = 93;
        v291 = 93;
        v276 = -25;
        *(&v314 + v313) = RepairEnable;
        v277 = 109;
        v278 = 58;
        v279 = -119;
        v236 = v313 + 1;
        v280 = -68;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v290 = 57;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        for ( i = v313; i != v236; ++i )
        {
          *(&v314 + i) ^= *((BYTE *)&v313 + i + 1) ^ *(&v276 + i % 32);
        }
        ++v313;
      }
      if ( v314 == 193 )
      {
        (BYTE)(v315[0]) = v313;
      }
      else if ( v314 == 194 )
      {
        v315[0] = v313;
      }
      v238 = v313;
      qmemcpy(v323, &v314, v313);
      v323[v238] = rand();
      v239 = (v323[0] != -63) + 2;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
      {
        v243 = operator_new(2u);
        *(BYTE *)(v243 + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, v243, (int)&g_byPacketSerialSend);
      }
      else
      {
        v240 = FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        if ( v240 == -1 )
        {
          v241 = 0;
        }
        else
        {
          v241 = *(char **)(DAT_055c9bcc + 4 * v240);
        }
        v242 = v241[1] + 1;
        v241[1] = v242;
        if ( (unsigned char)v242 < 2u )
        {
          FUN_00404330(&g_byPacketSerialSend, v241);
        }
      }
      v244 = g_byPacketSerialSend;
      v322[v239 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v244 + 1;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) != -1 )
      {
        v245 = FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        v246 = v245 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v245);
        v247 = v246[1] - 1;
        v246[1] = v247;
        if ( !v247 )
        {
          FUN_00423710(v246, &g_byPacketSerialSend);
        }
      }
      v248 = v239 - 1;
      v249 = v238 - v248;
      v250 = &v323[v248];
      v251 = FUN_0053cc30(0, (int)&v323[v248], v238 - v248);
      if ( v251 < 256 )
      {
        v252 = v251 + 2;
        buf = -61;
        v317 = v251 + 2;
        FUN_0053cc30((int)v318, (int)&v323[v248], v249);
        v253 = 0;
        v124 = v252;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v254 = send(s, &buf + v253, v252 - v253, 0);
            v255 = v254;
            if ( v254 == -1 )
            {
              break;
            }
            if ( v254 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2((int)&buf, v254);
              }
              v124 -= v255;
              v253 += v255;
              if ( v124 > 0 )
              {
                continue;
              }
            }
            goto LABEL_574;
          }
          if ( WSAGetLastError() == 10035 && DAT_055cc16c + v252 <= 0x2000 )
          {
            goto LABEL_561;
          }
          goto LABEL_571;
        }
        goto LABEL_574;
      }
      v256 = v251 + 3;
      v319 = -60;
      v321 = v251 + 3;
      v320 = (v251 + 3) / 256;
      FUN_0053cc30((int)v322, (int)v250, v249);
      v257 = 0;
      v129 = v256;
      if ( s == -1 )
      {
        goto LABEL_574;
      }
      while ( 1 )
      {
        v258 = send(s, &v319 + v257, v256 - v257, 0);
        v259 = v258;
        if ( v258 == -1 )
        {
          break;
        }
        if ( v258 )
        {
          if ( DAT_055ce174 )
          {
            nullsub_2((int)&v319, v258);
          }
          v129 -= v259;
          v257 += v259;
          if ( v129 > 0 )
          {
            continue;
          }
        }
        goto LABEL_574;
      }
      if ( WSAGetLastError() != 10035 || DAT_055cc16c + v256 > 0x2000 )
      {
        goto LABEL_571;
      }
      goto LABEL_572;
    }
    MouseLButtonPush = 0;
    if ( World != 10 )
    {
      goto LABEL_515;
    }
    v233 = 0;
    v234 = *(WORD *)(CharacterMachine + 1012);
    if ( v234 >= 384 && v234 <= 390 )
    {
      v233 = 1;
    }
    if ( *(short *)(CharacterMachine + 1080) >= 419 )
    {
      ++v233;
    }
    if ( v233 > 1 || (v232 < 384 || v232 > 390) && v232 != 419 )
    {
LABEL_515:
      DAT_07ea9800 = (int)&OffsetInventoryItems;
      qmemcpy(&pPickedItem, v12, 0x44u);
      v275 = Hero;
      *v12 = -1;
      *((DWORD *)v12 + 1) = 0;
      *((BYTE *)v12 + 27) = 0;
      *(DWORD *)&Inventory[32].Type = a5;
      SetCharacterClass(v275);
      CheckInventory = 0;
      PlayBuffer(29, 0, 0);
      if ( a5 == 8 )
      {
        DeleteBug(Hero);
      }
    }
    goto LABEL_576;
  }
  if ( MouseX < v5
    || (double)v311 + a3 <= (double)MouseX
    || MouseY < (int)v6
    || (double)(int)v306 + a4 <= (double)MouseY )
  {
    goto LABEL_576;
  }
  v13 = (void *)CharacterMachine;
  (BYTE)((v305) >> 8) = 1;
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) == -1 )
  {
    v17 = operator_new(0x585u);
    *(BYTE *)(v17 + 1412) = 1;
    FUN_00403f80(&DAT_055c9bc8, v17, (int)v13);
  }
  else
  {
    v14 = FUN_004041e0(&DAT_055c9bc8, (int)v13);
    if ( v14 == -1 )
    {
      v15 = 0;
    }
    else
    {
      v15 = *(BYTE **)(DAT_055c9bcc + 4 * v14);
    }
    v16 = v15[1412] + 1;
    v15[1412] = v16;
    if ( v16 < 2u )
    {
      FUN_00404370(v13, v15);
    }
  }
  v18 = CharacterMachine;
  v19 = *(short *)(CharacterMachine + 536);
  v20 = *(short *)(CharacterMachine + 604);
  v311 = v19;
  v308 = v20;
  if ( FUN_004041e0(&DAT_055c9bc8, CharacterMachine) != -1 )
  {
    v21 = FUN_004041e0(&DAT_055c9bc8, v18);
    v22 = v21 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v21);
    v23 = v22[1412] - 1;
    v22[1412] = v23;
    if ( !v23 )
    {
      FUN_00404400(v22, (const void *)v18);
    }
  }
  (WORD)(v18) = (WORD)pPickedItem;
  if ( (*(BYTE *)(Hero + 0x1BC) & 7) == 3 )
  {
    v24 = ItemAttribute;
    v25 = (short)pPickedItem << 6;
    if ( ItemAttribute->RequireClass[v25 + 3] )
    {
      (BYTE)((v305) >> 8) = 1;
      goto LABEL_36;
    }
    if ( ItemAttribute->RequireClass[v25] || ItemAttribute->RequireClass[v25 + 1] )
    {
      goto LABEL_36;
    }
  }
  else
  {
    v24 = ItemAttribute;
    v25 = (short)pPickedItem << 6;
    if ( ItemAttribute->RequireClass[v25 + (*(BYTE *)(Hero + 0x1BC) & 7)] )
    {
      goto LABEL_36;
    }
  }
  (BYTE)((v305) >> 8) = 0;
LABEL_36:
  if ( (WORD)pPickedItem != 135 && (WORD)pPickedItem != 143 )
  {
    if ( v19 != -1 && v19 != 135 && v19 != 143 && a5 == 1 )
    {
      if ( *(&v24->Width + v25) >= 2u && ((short)pPickedItem < 192 || (short)pPickedItem >= 224) )
      {
        (BYTE)((v305) >> 8) = 0;
      }
      if ( v24[v19].Width >= 2u )
      {
        (BYTE)((v305) >> 8) = 0;
      }
    }
    if ( v20 != -1 && v20 != 135 && v20 != 143 && !a5 )
    {
      if ( *(&v24->Width + v25) >= 2u )
      {
        (BYTE)((v305) >> 8) = 0;
      }
      if ( v24[v20].Width >= 2u && (v20 < 192 || v20 >= 224) )
      {
        (BYTE)((v305) >> 8) = 0;
      }
LABEL_118:
      if ( (WORD)pPickedItem == 135 )
      {
        (BYTE)((v305) >> 8) = 0;
      }
      v51 = (void *)CharacterMachine;
      v310 = CharacterMachine;
      v52 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
      v307 = 0;
      v306 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v53 = 4;
          v18 = (int)&v307;
          v20 = 0;
          v54 = 0;
          v55 = 1;
          v56 = (BYTE *)(DAT_055c9bd0 + 4 * v52);
          do
          {
            if ( !v53 )
            {
              break;
            }
            v54 = *(BYTE *)v18 < *v56;
            v55 = *(BYTE *)v18++ == *v56++;
            --v53;
          }
          while ( v55 );
          if ( !v55 )
          {
            v20 = -v54 - (v54 - 1);
          }
          if ( !v20 )
          {
            break;
          }
          v57 = 4;
          v58 = (BYTE *)(DAT_055c9bd0 + 4 * v52);
          v18 = (int)&v310;
          v20 = 0;
          v59 = 0;
          v60 = 1;
          do
          {
            if ( !v57 )
            {
              break;
            }
            v59 = *(BYTE *)v18 < *v58;
            v60 = *(BYTE *)v18++ == *v58++;
            --v57;
          }
          while ( v60 );
          if ( !v60 )
          {
            v20 = -v59 - (v59 - 1);
          }
          if ( !v20 )
          {
            if ( v52 == -1 )
            {
              break;
            }
            v71 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v51);
            v72 = v71[1412] + 1;
            v71[1412] = v72;
            if ( v72 < 2u )
            {
              FUN_00404370(v51, v71);
            }
            goto LABEL_136;
          }
          v52 = (v52 + 1) % DAT_055c9bd4;
          if ( (unsigned int)++v306 >= DAT_055c9bd4 )
          {
            goto LABEL_134;
          }
        }
      }
      else
      {
LABEL_134:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v61 = operator_new(0x585u);
      *(BYTE *)(v61 + 1412) = 1;
      FUN_00403f80(&DAT_055c9bc8, v61, (int)v51);
LABEL_136:
      if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2
        && v308 == 135
        && ((short)pPickedItem < 128 || (short)pPickedItem >= 160 || (WORD)pPickedItem == 143) )
      {
        (BYTE)((v305) >> 8) = 0;
      }
      v39 = (const void *)CharacterMachine;
      v307 = (BYTE *)CharacterMachine;
      v62 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
      v308 = 0;
      v306 = 0;
      if ( DAT_055c9bd4 )
      {
        do
        {
          v18 = (int)&v308;
          v20 = 0;
          v63 = 0;
          v64 = 1;
          v65 = 4;
          v66 = (BYTE *)(DAT_055c9bd0 + 4 * v62);
          do
          {
            if ( !v65 )
            {
              break;
            }
            v63 = *(BYTE *)v18 < *v66;
            v64 = *(BYTE *)v18++ == *v66++;
            --v65;
          }
          while ( v64 );
          if ( !v64 )
          {
            v20 = -v63 - (v63 - 1);
          }
          if ( !v20 )
          {
            goto LABEL_163;
          }
          v67 = 4;
          v68 = (BYTE *)(DAT_055c9bd0 + 4 * v62);
          v18 = (int)&v307;
          v20 = 0;
          v69 = 0;
          v70 = 1;
          do
          {
            if ( !v67 )
            {
              break;
            }
            v69 = *(BYTE *)v18 < *v68;
            v70 = *(BYTE *)v18++ == *v68++;
            --v67;
          }
          while ( v70 );
          if ( !v70 )
          {
            v20 = -v69 - (v69 - 1);
          }
          if ( !v20 )
          {
            if ( v62 == -1 )
            {
              goto LABEL_163;
            }
            v49 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v39);
            v73 = v49[1412] - 1;
            v49[1412] = v73;
            if ( v73 )
            {
              goto LABEL_163;
            }
            goto LABEL_162;
          }
          v62 = (v62 + 1) % DAT_055c9bd4;
        }
        while ( (unsigned int)++v306 < DAT_055c9bd4 );
      }
LABEL_156:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
LABEL_163:
      (WORD)(v18) = (WORD)pPickedItem;
      goto LABEL_164;
    }
  }
  if ( a5 == 1 )
  {
    if ( (WORD)pPickedItem == 143 )
    {
      (BYTE)((v305) >> 8) = 0;
    }
    v26 = (void *)CharacterMachine;
    v307 = (BYTE *)CharacterMachine;
    v27 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v310 = 0;
    v306 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( 1 )
      {
        v28 = 4;
        v18 = (int)&v310;
        v20 = 0;
        v29 = 0;
        v30 = 1;
        v31 = (BYTE *)(DAT_055c9bd0 + 4 * v27);
        do
        {
          if ( !v28 )
          {
            break;
          }
          v29 = *(BYTE *)v18 < *v31;
          v30 = *(BYTE *)v18++ == *v31++;
          --v28;
        }
        while ( v30 );
        if ( !v30 )
        {
          v20 = -v29 - (v29 - 1);
        }
        if ( !v20 )
        {
          break;
        }
        v32 = 4;
        v33 = (BYTE *)(DAT_055c9bd0 + 4 * v27);
        v18 = (int)&v307;
        v20 = 0;
        v34 = 0;
        v35 = 1;
        do
        {
          if ( !v32 )
          {
            break;
          }
          v34 = *(BYTE *)v18 < *v33;
          v35 = *(BYTE *)v18++ == *v33++;
          --v32;
        }
        while ( v35 );
        if ( !v35 )
        {
          v20 = -v34 - (v34 - 1);
        }
        if ( !v20 )
        {
          if ( v27 == -1 )
          {
            break;
          }
          v37 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v26);
          v38 = v37[1412] + 1;
          v37[1412] = v38;
          if ( v38 < 2u )
          {
            FUN_00404370(v26, v37);
          }
          goto LABEL_77;
        }
        v27 = (v27 + 1) % DAT_055c9bd4;
        if ( (unsigned int)++v306 >= DAT_055c9bd4 )
        {
          goto LABEL_75;
        }
      }
    }
    else
    {
LABEL_75:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    v36 = operator_new(0x585u);
    *(BYTE *)(v36 + 1412) = 1;
    FUN_00403f80(&DAT_055c9bc8, v36, (int)v26);
LABEL_77:
    if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 0 || (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 )
    {
      if ( (short)pPickedItem >= 0 && (short)pPickedItem < 128 )
      {
        (BYTE)((v305) >> 8) = 0;
      }
      if ( (short)pPickedItem >= 160 && (short)pPickedItem < 192 )
      {
        (BYTE)((v305) >> 8) = 0;
      }
      if ( v311 == 143 )
      {
        if ( (WORD)pPickedItem == 135 || (short)pPickedItem < 128 || (short)pPickedItem > 160 )
        {
LABEL_98:
          (BYTE)((v305) >> 8) = 0;
        }
      }
      else if ( (WORD)pPickedItem == 135 && v311 != -1 && (v311 < 128 || v311 > 160) )
      {
        goto LABEL_98;
      }
    }
    v39 = (const void *)CharacterMachine;
    v310 = CharacterMachine;
    v40 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
    v307 = 0;
    v306 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( 1 )
      {
        v41 = 4;
        v18 = (int)&v307;
        v20 = 0;
        v42 = 0;
        v43 = 1;
        v44 = (BYTE *)(DAT_055c9bd0 + 4 * v40);
        do
        {
          if ( !v41 )
          {
            break;
          }
          v42 = *(BYTE *)v18 < *v44;
          v43 = *(BYTE *)v18++ == *v44++;
          --v41;
        }
        while ( v43 );
        if ( !v43 )
        {
          v20 = -v42 - (v42 - 1);
        }
        if ( !v20 )
        {
          goto LABEL_163;
        }
        v45 = 4;
        v46 = (BYTE *)(DAT_055c9bd0 + 4 * v40);
        v18 = (int)&v310;
        v20 = 0;
        v47 = 0;
        v48 = 1;
        do
        {
          if ( !v45 )
          {
            break;
          }
          v47 = *(BYTE *)v18 < *v46;
          v48 = *(BYTE *)v18++ == *v46++;
          --v45;
        }
        while ( v48 );
        if ( !v48 )
        {
          v20 = -v47 - (v47 - 1);
        }
        if ( !v20 )
        {
          if ( v40 != -1 )
          {
            v49 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v39);
            v50 = v49[1412] - 1;
            v49[1412] = v50;
            if ( !v50 )
            {
LABEL_162:
              FUN_00404400(v49, v39);
            }
          }
          goto LABEL_163;
        }
        v40 = (v40 + 1) % DAT_055c9bd4;
        if ( (unsigned int)++v306 >= DAT_055c9bd4 )
        {
          goto LABEL_156;
        }
      }
    }
    goto LABEL_156;
  }
  if ( !a5 )
  {
    goto LABEL_118;
  }
LABEL_164:
  if ( (short)v18 >= 430 && (short)v18 <= 435 )
  {
    (BYTE)((v305) >> 8) = 0;
  }
  v74 = (void *)CharacterMachine;
  v307 = (BYTE *)CharacterMachine;
  v75 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v308 = 0;
  v306 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( 1 )
    {
      v76 = 4;
      v18 = (int)&v308;
      v20 = 0;
      v77 = 0;
      v78 = 1;
      v79 = (BYTE *)(DAT_055c9bd0 + 4 * v75);
      do
      {
        if ( !v76 )
        {
          break;
        }
        v77 = *(BYTE *)v18 < *v79;
        v78 = *(BYTE *)v18++ == *v79++;
        --v76;
      }
      while ( v78 );
      if ( !v78 )
      {
        v20 = -v77 - (v77 - 1);
      }
      if ( !v20 )
      {
        break;
      }
      v80 = 4;
      v81 = (BYTE *)(DAT_055c9bd0 + 4 * v75);
      v18 = (int)&v307;
      v20 = 0;
      v82 = 0;
      v83 = 1;
      do
      {
        if ( !v80 )
        {
          break;
        }
        v82 = *(BYTE *)v18 < *v81;
        v83 = *(BYTE *)v18++ == *v81++;
        --v80;
      }
      while ( v83 );
      if ( !v83 )
      {
        v20 = -v82 - (v82 - 1);
      }
      if ( !v20 )
      {
        if ( v75 == -1 )
        {
          break;
        }
        v95 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v74);
        v96 = v95[1412] + 1;
        v95[1412] = v96;
        if ( v96 < 2u )
        {
          FUN_00404370(v74, v95);
        }
        goto LABEL_183;
      }
      v75 = (v75 + 1) % DAT_055c9bd4;
      if ( (unsigned int)++v306 >= DAT_055c9bd4 )
      {
        goto LABEL_181;
      }
    }
  }
  else
  {
LABEL_181:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v84 = operator_new(0x585u);
  *(BYTE *)(v84 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v84, (int)v74);
LABEL_183:
  if ( (unsigned short)DAT_07e9136c > *(WORD *)(CharacterAttribute + 20) )
  {
    (BYTE)((v305) >> 8) = 0;
  }
  if ( (unsigned short)DAT_07e9136e > *(WORD *)(CharacterAttribute + 22) )
  {
    (BYTE)((v305) >> 8) = 0;
  }
  if ( (unsigned short)DAT_07e91370 > *(WORD *)(CharacterAttribute + 26) )
  {
    (BYTE)((v305) >> 8) = 0;
  }
  if ( ((BYTE)DAT_07e91358 == 10
     || (BYTE)DAT_07e91358 == 11
     || (BYTE)DAT_07e91358 == 9
     || (BYTE)DAT_07e91358 == 8
     || (BYTE)DAT_07e91358 == 7)
    && (unsigned short)DAT_07e91372 > *(WORD *)(CharacterAttribute + 14) )
  {
    (BYTE)((v305) >> 8) = 0;
  }
  v85 = (const void *)CharacterMachine;
  v307 = (BYTE *)CharacterMachine;
  v86 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v308 = 0;
  v306 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( 1 )
    {
      v87 = 4;
      v18 = (int)&v308;
      v20 = 0;
      v88 = 0;
      v89 = 1;
      v90 = (BYTE *)(DAT_055c9bd0 + 4 * v86);
      do
      {
        if ( !v87 )
        {
          break;
        }
        v88 = *(BYTE *)v18 < *v90;
        v89 = *(BYTE *)v18++ == *v90++;
        --v87;
      }
      while ( v89 );
      if ( !v89 )
      {
        v20 = -v88 - (v88 - 1);
      }
      if ( !v20 )
      {
        break;
      }
      v91 = 4;
      v92 = (BYTE *)(DAT_055c9bd0 + 4 * v86);
      v18 = (int)&v307;
      v20 = 0;
      v93 = 0;
      v94 = 1;
      do
      {
        if ( !v91 )
        {
          break;
        }
        v93 = *(BYTE *)v18 < *v92;
        v94 = *(BYTE *)v18++ == *v92++;
        --v91;
      }
      while ( v94 );
      if ( !v94 )
      {
        v20 = -v93 - (v93 - 1);
      }
      if ( !v20 )
      {
        if ( v86 != -1 )
        {
          v97 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)v85);
          v98 = v97[1412] - 1;
          v97[1412] = v98;
          if ( !v98 )
          {
            FUN_00404400(v97, v85);
          }
        }
        break;
      }
      v86 = (v86 + 1) % DAT_055c9bd4;
      if ( (unsigned int)++v306 >= DAT_055c9bd4 )
      {
        goto LABEL_210;
      }
    }
  }
  else
  {
LABEL_210:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  if ( (BYTE)DAT_07e91358 == 7 )
  {
    v99 = (short)pPickedItem;
    if ( (short)pPickedItem >= 392 && (short)pPickedItem < 416 )
    {
      goto LABEL_220;
    }
    goto LABEL_231;
  }
  if ( (BYTE)DAT_07e91358 == 8 )
  {
    if ( World == 7 )
    {
      if ( (WORD)pPickedItem == 418 || (WORD)pPickedItem == 419 )
      {
        goto LABEL_220;
      }
    }
    else if ( World == 10 && (WORD)pPickedItem == 418 )
    {
LABEL_220:
      v309[64] = 3;
      if ( MouseLButtonPush )
      {
        MouseLButtonPush = 0;
        if ( !EquipmentItem )
        {
          FUN_004cd3b0(v20, v18);
        }
      }
      goto LABEL_576;
    }
  }
  v99 = (short)pPickedItem;
LABEL_231:
  if ( !(BYTE)((v305) >> 8) )
  {
    goto LABEL_220;
  }
  v100 = a5;
  v18 = (unsigned char)DAT_07e91358;
  if ( (unsigned char)DAT_07e91358 != a5 )
  {
    if ( (BYTE)DAT_07e91358 == 10 )
    {
      if ( a5 != 11 )
      {
        goto LABEL_220;
      }
    }
    else
    {
      if ( (BYTE)DAT_07e91358 )
      {
        goto LABEL_220;
      }
      if ( a5 != 1 )
      {
        goto LABEL_220;
      }
      v18 = (int)ItemAttribute;
      if ( ItemAttribute[v99].Width >= 2u )
      {
        goto LABEL_220;
      }
    }
  }
  v309[64] = 2;
  if ( !MouseLButtonPush )
  {
    goto LABEL_576;
  }
  DAT_07eaa164 = 1;
  MouseLButtonPush = 0;
  DAT_07e11e78 = a5;
  if ( a5 == 1 && v99 >= 0 && v99 < 96 && v311 == -1 )
  {
    v100 = 0;
    DAT_07e11e78 = 0;
  }
  if ( (ITEM *)DAT_07ea9800 == &OffsetInventoryItems )
  {
    if ( EquipmentItem )
    {
      goto LABEL_576;
    }
    EquipmentItem = 1;
    v312 = &DAT_00552460;
    v324 = 0;
    v314 = -63;
    (BYTE)(v315[0]) = 1;
    strcpy((char *)v315 + 1, "$");
    v313 = 3;
    v276 = -25;
    v277 = 109;
    v278 = 58;
    v101 = 3;
    v279 = -119;
    v280 = -68;
    v281 = -78;
    v282 = -97;
    v283 = 115;
    v284 = 35;
    v285 = -88;
    v286 = -2;
    v287 = -74;
    v288 = 73;
    v289 = 93;
    v290 = 57;
    v291 = 93;
    v292 = -118;
    v293 = -53;
    v294 = 99;
    v295 = -115;
    v296 = -22;
    v297 = 125;
    v298 = 43;
    v299 = 95;
    v300 = -61;
    v301 = -79;
    v302 = -23;
    v303 = -125;
    v304 = 1458065705;
    do
    {
      *(&v314 + v101) ^= *((BYTE *)&v313 + v101 + 1) ^ *(&v276 + v101 % 32);
      ++v101;
    }
    while ( v101 != 4 );
    if ( ++v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      *(&v314 + v313) = Inventory[32].Type;
      v102 = v313;
      v278 = 58;
      v279 = -119;
      v280 = -68;
      v103 = v313 + 1;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v103 )
      {
        do
        {
          *(&v314 + v102) ^= *((BYTE *)&v313 + v102 + 1) ^ *(&v276 + v102 % 32);
          ++v102;
        }
        while ( v102 != v103 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      *(&v314 + v313) = (unsigned char)pPickedItem;
      v104 = v313;
      v278 = 58;
      v279 = -119;
      v280 = -68;
      v105 = v313 + 1;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v105 )
      {
        do
        {
          *(&v314 + v104) ^= *((BYTE *)&v313 + v104 + 1) ^ *(&v276 + v104 % 32);
          ++v104;
        }
        while ( v104 != v105 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = Level;
      v106 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v107 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v107 )
      {
        do
        {
          *(&v314 + v106) ^= *((BYTE *)&v313 + v106 + 1) ^ *(&v276 + v106 % 32);
          ++v106;
        }
        while ( v106 != v107 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = DAT_07e9136a;
      v108 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v109 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v109 )
      {
        do
        {
          *(&v314 + v108) ^= *((BYTE *)&v313 + v108 + 1) ^ *(&v276 + v108 % 32);
          ++v108;
        }
        while ( v108 != v109 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = DAT_07e9136b;
      v110 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v111 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v111 )
      {
        do
        {
          *(&v314 + v110) ^= *((BYTE *)&v313 + v110 + 1) ^ *(&v276 + v110 % 32);
          ++v110;
        }
        while ( v110 != v111 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = 0;
      v112 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v113 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v113 )
      {
        do
        {
          *(&v314 + v112) ^= *((BYTE *)&v313 + v112 + 1) ^ *(&v276 + v112 % 32);
          ++v112;
        }
        while ( v112 != v113 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v289 = 93;
      v291 = 93;
      *(&v314 + v313) = DAT_07e11e78;
      v276 = -25;
      v277 = 109;
      v278 = 58;
      v114 = v313 + 1;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v290 = 57;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      for ( j = v313; j != v114; ++j )
      {
        *(&v314 + j) ^= *((BYTE *)&v313 + j + 1) ^ *(&v276 + j % 32);
      }
      ++v313;
    }
    if ( v314 == 193 )
    {
      (BYTE)(v315[0]) = v313;
    }
    else if ( v314 == 194 )
    {
      v315[0] = v313;
    }
    v116 = v313;
    qmemcpy(v323, &v314, v313);
    v323[v116] = rand();
    v117 = (v323[0] != -63) + 2;
    PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    v118 = g_byPacketSerialSend;
    v322[v117 + 1024] = g_byPacketSerialSend;
    g_byPacketSerialSend = v118 + 1;
    PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    --v117;
    v119 = v116 - v117;
    v120 = &v323[v117];
    v121 = FUN_0053cc30(0, (int)v120, v119);
    if ( v121 < 256 )
    {
      v122 = v121 + 2;
      buf = -61;
      v317 = v121 + 2;
      FUN_0053cc30((int)v318, (int)v120, v119);
      v123 = 0;
      v124 = v122;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v125 = send(s, &buf + v123, v122 - v123, 0);
          v126 = v125;
          if ( v125 == -1 )
          {
            break;
          }
          if ( v125 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&buf, v125);
            }
            v124 -= v126;
            v123 += v126;
            if ( v124 > 0 )
            {
              continue;
            }
          }
          goto LABEL_574;
        }
        if ( WSAGetLastError() == 10035 && DAT_055cc16c + v122 <= 0x2000 )
        {
LABEL_561:
          qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf, v124);
          v199 = v124 + DAT_055cc16c;
          goto LABEL_573;
        }
LABEL_571:
        CWsctlc::Close((DWORD)&SocketClient);
        v324 = -1;
        goto LABEL_576;
      }
      goto LABEL_574;
    }
    v127 = v121 + 3;
    v319 = -60;
    v321 = v121 + 3;
    v320 = (v121 + 3) / 256;
    FUN_0053cc30((int)v322, (int)v120, v119);
    v128 = 0;
    v129 = v127;
    if ( s == -1 )
    {
      goto LABEL_574;
    }
    while ( 1 )
    {
      v130 = send(s, &v319 + v128, v127 - v128, 0);
      v131 = v130;
      if ( v130 == -1 )
      {
        break;
      }
      if ( v130 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2((int)&v319, v130);
        }
        v129 -= v131;
        v128 += v131;
        if ( v129 > 0 )
        {
          continue;
        }
      }
      goto LABEL_574;
    }
    if ( WSAGetLastError() != 10035 || DAT_055cc16c + v127 > 0x2000 )
    {
      goto LABEL_571;
    }
    goto LABEL_572;
  }
  if ( (WORD *)DAT_07ea9800 != &Inventory[32].WalkSpeed )
  {
    if ( (ITEM *)DAT_07ea9800 == &OffsetMixItems )
    {
      if ( EquipmentItem )
      {
        goto LABEL_576;
      }
      EquipmentItem = 1;
      v312 = &DAT_00552460;
      v314 = -63;
      v324 = 2;
      v315[0] = 9217;
      v313 = 3;
      v276 = -25;
      v277 = 109;
      v278 = 58;
      (BYTE)(v315[1]) = 3;
      v166 = 3;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      do
      {
        *(&v314 + v166) ^= *((BYTE *)&v313 + v166 + 1) ^ *(&v276 + v166 % 32);
        ++v166;
      }
      while ( v166 != 4 );
      if ( ++v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        *(&v314 + v313) = Inventory[32].Type;
        v167 = v313;
        v278 = 58;
        v279 = -119;
        v280 = -68;
        v168 = v313 + 1;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v168 )
        {
          do
          {
            *(&v314 + v167) ^= *((BYTE *)&v313 + v167 + 1) ^ *(&v276 + v167 % 32);
            ++v167;
          }
          while ( v167 != v168 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        *(&v314 + v313) = (unsigned char)pPickedItem;
        v169 = v313;
        v278 = 58;
        v279 = -119;
        v280 = -68;
        v170 = v313 + 1;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v170 )
        {
          do
          {
            *(&v314 + v169) ^= *((BYTE *)&v313 + v169 + 1) ^ *(&v276 + v169 % 32);
            ++v169;
          }
          while ( v169 != v170 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = Level;
        v171 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v172 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v172 )
        {
          do
          {
            *(&v314 + v171) ^= *((BYTE *)&v313 + v171 + 1) ^ *(&v276 + v171 % 32);
            ++v171;
          }
          while ( v171 != v172 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = DAT_07e9136a;
        v173 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v174 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v174 )
        {
          do
          {
            *(&v314 + v173) ^= *((BYTE *)&v313 + v173 + 1) ^ *(&v276 + v173 % 32);
            ++v173;
          }
          while ( v173 != v174 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = DAT_07e9136b;
        v175 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v176 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v176 )
        {
          do
          {
            *(&v314 + v175) ^= *((BYTE *)&v313 + v175 + 1) ^ *(&v276 + v175 % 32);
            ++v175;
          }
          while ( v175 != v176 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = 0;
        v177 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v178 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v178 )
        {
          do
          {
            *(&v314 + v177) ^= *((BYTE *)&v313 + v177 + 1) ^ *(&v276 + v177 % 32);
            ++v177;
          }
          while ( v177 != v178 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v289 = 93;
        v291 = 93;
        *(&v314 + v313) = DAT_07e11e78;
        v276 = -25;
        v277 = 109;
        v278 = 58;
        v179 = v313 + 1;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v290 = 57;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        for ( k = v313; k != v179; ++k )
        {
          *(&v314 + k) ^= *((BYTE *)&v313 + k + 1) ^ *(&v276 + k % 32);
        }
        ++v313;
      }
      if ( v314 == 193 )
      {
        (BYTE)(v315[0]) = v313;
      }
      else if ( v314 == 194 )
      {
        v315[0] = v313;
      }
      v181 = v313;
      qmemcpy(v323, &v314, v313);
      v323[v181] = rand();
      v182 = (v323[0] != -63) + 2;
      PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
      v183 = g_byPacketSerialSend;
      v322[v182 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v183 + 1;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) != -1 )
      {
        v184 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        v185 = v184[1] - 1;
        v184[1] = v185;
        if ( !v185 )
        {
          FUN_00423710(v184, &g_byPacketSerialSend);
        }
      }
      v186 = v182 - 1;
      v187 = v181 - v186;
      v188 = &v323[v186];
      v189 = FUN_0053cc30(0, (int)&v323[v186], v181 - v186);
      if ( v189 < 256 )
      {
        v273 = v181 - v186;
        v190 = v189 + 2;
        buf = -61;
        v317 = v189 + 2;
        FUN_0053cc30((int)v318, (int)&v323[v186], v273);
        v191 = 0;
        v124 = v190;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v192 = send(s, &buf + v191, v190 - v191, 0);
            v193 = v192;
            if ( v192 == -1 )
            {
              break;
            }
            if ( v192 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2((int)&buf, v192);
              }
              v124 -= v193;
              v191 += v193;
              if ( v124 > 0 )
              {
                continue;
              }
            }
            goto LABEL_574;
          }
          if ( WSAGetLastError() == 10035 && DAT_055cc16c + v190 <= 0x2000 )
          {
            goto LABEL_561;
          }
          goto LABEL_571;
        }
        goto LABEL_574;
      }
      v194 = v189 + 3;
      v319 = -60;
      v321 = v189 + 3;
      v320 = (v189 + 3) / 256;
      FUN_0053cc30((int)v322, (int)v188, v187);
      v195 = 0;
      v196 = v194;
      if ( s == -1 )
      {
        goto LABEL_574;
      }
      while ( 1 )
      {
        v197 = send(s, &v319 + v195, v194 - v195, 0);
        v198 = v197;
        if ( v197 == -1 )
        {
          break;
        }
        if ( v197 )
        {
          if ( DAT_055ce174 )
          {
            nullsub_2((int)&v319, v197);
          }
          v196 -= v198;
          v195 += v198;
          if ( v196 > 0 )
          {
            continue;
          }
        }
        goto LABEL_574;
      }
      if ( WSAGetLastError() != 10035 || DAT_055cc16c + v194 > 0x2000 )
      {
        goto LABEL_571;
      }
    }
    else
    {
      if ( !m_bMyConfirm )
      {
        m_nMyTradeWait = 150;
      }
      if ( EquipmentItem )
      {
        goto LABEL_576;
      }
      EquipmentItem = 1;
      v312 = &DAT_00552460;
      v314 = -63;
      v324 = 3;
      v315[0] = 9217;
      v313 = 3;
      v276 = -25;
      v277 = 109;
      v278 = 58;
      (BYTE)(v315[1]) = 1;
      v200 = 3;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      do
      {
        *(&v314 + v200) ^= *((BYTE *)&v313 + v200 + 1) ^ *(&v276 + v200 % 32);
        ++v200;
      }
      while ( v200 != 4 );
      if ( ++v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        *(&v314 + v313) = Inventory[32].Type;
        v201 = v313;
        v278 = 58;
        v279 = -119;
        v280 = -68;
        v202 = v313 + 1;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v202 )
        {
          do
          {
            *(&v314 + v201) ^= *((BYTE *)&v313 + v201 + 1) ^ *(&v276 + v201 % 32);
            ++v201;
          }
          while ( v201 != v202 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        *(&v314 + v313) = (unsigned char)pPickedItem;
        v203 = v313;
        v278 = 58;
        v279 = -119;
        v280 = -68;
        v204 = v313 + 1;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v204 )
        {
          do
          {
            *(&v314 + v203) ^= *((BYTE *)&v313 + v203 + 1) ^ *(&v276 + v203 % 32);
            ++v203;
          }
          while ( v203 != v204 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = Level;
        v205 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v206 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v206 )
        {
          do
          {
            *(&v314 + v205) ^= *((BYTE *)&v313 + v205 + 1) ^ *(&v276 + v205 % 32);
            ++v205;
          }
          while ( v205 != v206 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = DAT_07e9136a;
        v207 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v208 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v208 )
        {
          do
          {
            *(&v314 + v207) ^= *((BYTE *)&v313 + v207 + 1) ^ *(&v276 + v207 % 32);
            ++v207;
          }
          while ( v207 != v208 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = DAT_07e9136b;
        v209 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v210 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v210 )
        {
          do
          {
            *(&v314 + v209) ^= *((BYTE *)&v313 + v209 + 1) ^ *(&v276 + v209 % 32);
            ++v209;
          }
          while ( v209 != v210 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v276 = -25;
        v277 = 109;
        v278 = 58;
        *(&v314 + v313) = 0;
        v211 = v313;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v212 = v313 + 1;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v289 = 93;
        v290 = 57;
        v291 = 93;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        if ( v313 != v212 )
        {
          do
          {
            *(&v314 + v211) ^= *((BYTE *)&v313 + v211 + 1) ^ *(&v276 + v211 % 32);
            ++v211;
          }
          while ( v211 != v212 );
        }
        ++v313;
      }
      if ( v313 + 1 <= 1024 )
      {
        v289 = 93;
        v291 = 93;
        *(&v314 + v313) = DAT_07e11e78;
        v276 = -25;
        v277 = 109;
        v278 = 58;
        v213 = v313 + 1;
        v279 = -119;
        v280 = -68;
        v281 = -78;
        v282 = -97;
        v283 = 115;
        v284 = 35;
        v285 = -88;
        v286 = -2;
        v287 = -74;
        v288 = 73;
        v290 = 57;
        v292 = -118;
        v293 = -53;
        v294 = 99;
        v295 = -115;
        v296 = -22;
        v297 = 125;
        v298 = 43;
        v299 = 95;
        v300 = -61;
        v301 = -79;
        v302 = -23;
        v303 = -125;
        v304 = 1458065705;
        for ( m = v313; m != v213; ++m )
        {
          *(&v314 + m) ^= *((BYTE *)&v313 + m + 1) ^ *(&v276 + m % 32);
        }
        ++v313;
      }
      if ( v314 == 193 )
      {
        (BYTE)(v315[0]) = v313;
      }
      else if ( v314 == 194 )
      {
        v315[0] = v313;
      }
      v215 = v313;
      qmemcpy(v323, &v314, v313);
      v323[v215] = rand();
      v216 = (v323[0] != -63) + 2;
      PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
      v217 = g_byPacketSerialSend;
      v322[v216 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v217 + 1;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) != -1 )
      {
        v218 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
        v219 = v218[1] - 1;
        v218[1] = v219;
        if ( !v219 )
        {
          FUN_00423710(v218, &g_byPacketSerialSend);
        }
      }
      v220 = v216 - 1;
      v221 = v215 - v220;
      v222 = &v323[v220];
      v223 = FUN_0053cc30(0, (int)&v323[v220], v215 - v220);
      if ( v223 < 256 )
      {
        v274 = v215 - v220;
        v224 = v223 + 2;
        buf = -61;
        v317 = v223 + 2;
        FUN_0053cc30((int)v318, (int)&v323[v220], v274);
        v225 = 0;
        v124 = v224;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v226 = send(s, &buf + v225, v224 - v225, 0);
            v227 = v226;
            if ( v226 == -1 )
            {
              break;
            }
            if ( v226 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2((int)&buf, v226);
              }
              v124 -= v227;
              v225 += v227;
              if ( v124 > 0 )
              {
                continue;
              }
            }
            goto LABEL_574;
          }
          if ( WSAGetLastError() == 10035 && DAT_055cc16c + v224 <= 0x2000 )
          {
            goto LABEL_561;
          }
          goto LABEL_571;
        }
        goto LABEL_574;
      }
      v228 = v223 + 3;
      v319 = -60;
      v321 = v223 + 3;
      v320 = (v223 + 3) / 256;
      FUN_0053cc30((int)v322, (int)v222, v221);
      v229 = 0;
      v196 = v228;
      if ( s == -1 )
      {
        goto LABEL_574;
      }
      while ( 1 )
      {
        v230 = send(s, &v319 + v229, v228 - v229, 0);
        v231 = v230;
        if ( v230 == -1 )
        {
          break;
        }
        if ( v230 )
        {
          if ( DAT_055ce174 )
          {
            nullsub_2((int)&v319, v230);
          }
          v196 -= v231;
          v229 += v231;
          if ( v196 > 0 )
          {
            continue;
          }
        }
        goto LABEL_574;
      }
      if ( WSAGetLastError() != 10035 || DAT_055cc16c + v228 > 0x2000 )
      {
        goto LABEL_571;
      }
    }
    qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v319, v196);
    v199 = v196 + DAT_055cc16c;
LABEL_573:
    DAT_055cc16c = v199;
    goto LABEL_574;
  }
  if ( !DAT_00559f5f || DAT_07eaa148 )
  {
    if ( EquipmentItem )
    {
      goto LABEL_576;
    }
    EquipmentItem = 1;
    v312 = &DAT_00552460;
    v314 = -63;
    v324 = 1;
    v315[0] = 9217;
    v313 = 3;
    v276 = -25;
    v277 = 109;
    v278 = 58;
    (BYTE)(v315[1]) = 2;
    v137 = 3;
    v279 = -119;
    v280 = -68;
    v281 = -78;
    v282 = -97;
    v283 = 115;
    v284 = 35;
    v285 = -88;
    v286 = -2;
    v287 = -74;
    v288 = 73;
    v289 = 93;
    v290 = 57;
    v291 = 93;
    v292 = -118;
    v293 = -53;
    v294 = 99;
    v295 = -115;
    v296 = -22;
    v297 = 125;
    v298 = 43;
    v299 = 95;
    v300 = -61;
    v301 = -79;
    v302 = -23;
    v303 = -125;
    v304 = 1458065705;
    do
    {
      *(&v314 + v137) ^= *((BYTE *)&v313 + v137 + 1) ^ *(&v276 + v137 % 32);
      ++v137;
    }
    while ( v137 != 4 );
    if ( ++v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      *(&v314 + v313) = Inventory[32].Type;
      v138 = v313;
      v278 = 58;
      v279 = -119;
      v280 = -68;
      v139 = v313 + 1;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v139 )
      {
        do
        {
          *(&v314 + v138) ^= *((BYTE *)&v313 + v138 + 1) ^ *(&v276 + v138 % 32);
          ++v138;
        }
        while ( v138 != v139 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      *(&v314 + v313) = (unsigned char)pPickedItem;
      v140 = v313;
      v278 = 58;
      v279 = -119;
      v280 = -68;
      v141 = v313 + 1;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v141 )
      {
        do
        {
          *(&v314 + v140) ^= *((BYTE *)&v313 + v140 + 1) ^ *(&v276 + v140 % 32);
          ++v140;
        }
        while ( v140 != v141 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = Level;
      v142 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v143 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v143 )
      {
        do
        {
          *(&v314 + v142) ^= *((BYTE *)&v313 + v142 + 1) ^ *(&v276 + v142 % 32);
          ++v142;
        }
        while ( v142 != v143 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = DAT_07e9136a;
      v144 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v145 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v145 )
      {
        do
        {
          *(&v314 + v144) ^= *((BYTE *)&v313 + v144 + 1) ^ *(&v276 + v144 % 32);
          ++v144;
        }
        while ( v144 != v145 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = DAT_07e9136b;
      v146 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v147 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v147 )
      {
        do
        {
          *(&v314 + v146) ^= *((BYTE *)&v313 + v146 + 1) ^ *(&v276 + v146 % 32);
          ++v146;
        }
        while ( v146 != v147 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v276 = -25;
      v277 = 109;
      v278 = 58;
      *(&v314 + v313) = 0;
      v148 = v313;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v149 = v313 + 1;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v289 = 93;
      v290 = 57;
      v291 = 93;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      if ( v313 != v149 )
      {
        do
        {
          *(&v314 + v148) ^= *((BYTE *)&v313 + v148 + 1) ^ *(&v276 + v148 % 32);
          ++v148;
        }
        while ( v148 != v149 );
      }
      ++v313;
    }
    if ( v313 + 1 <= 1024 )
    {
      v289 = 93;
      v291 = 93;
      *(&v314 + v313) = DAT_07e11e78;
      v276 = -25;
      v277 = 109;
      v278 = 58;
      v150 = v313 + 1;
      v279 = -119;
      v280 = -68;
      v281 = -78;
      v282 = -97;
      v283 = 115;
      v284 = 35;
      v285 = -88;
      v286 = -2;
      v287 = -74;
      v288 = 73;
      v290 = 57;
      v292 = -118;
      v293 = -53;
      v294 = 99;
      v295 = -115;
      v296 = -22;
      v297 = 125;
      v298 = 43;
      v299 = 95;
      v300 = -61;
      v301 = -79;
      v302 = -23;
      v303 = -125;
      v304 = 1458065705;
      for ( n = v313; n != v150; ++n )
      {
        *(&v314 + n) ^= *((BYTE *)&v313 + n + 1) ^ *(&v276 + n % 32);
      }
      ++v313;
    }
    if ( v314 == 193 )
    {
      (BYTE)(v315[0]) = v313;
    }
    else if ( v314 == 194 )
    {
      v315[0] = v313;
    }
    v152 = v313;
    qmemcpy(v323, &v314, v313);
    v323[v152] = rand();
    v153 = (v323[0] != -63) + 2;
    PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    v154 = g_byPacketSerialSend;
    v322[v153 + 1024] = g_byPacketSerialSend;
    g_byPacketSerialSend = v154 + 1;
    PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    --v153;
    v155 = v152 - v153;
    v156 = &v323[v153];
    v157 = FUN_0053cc30(0, (int)v156, v155);
    if ( v157 < 256 )
    {
      v158 = v157 + 2;
      buf = -61;
      v317 = v157 + 2;
      FUN_0053cc30((int)v318, (int)v156, v155);
      v159 = 0;
      v124 = v158;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v160 = send(s, &buf + v159, v158 - v159, 0);
          v161 = v160;
          if ( v160 == -1 )
          {
            break;
          }
          if ( v160 )
          {
            if ( DAT_055ce174 )
            {
              nullsub_2((int)&buf, v160);
            }
            v124 -= v161;
            v159 += v161;
            if ( v124 > 0 )
            {
              continue;
            }
          }
          goto LABEL_574;
        }
        if ( WSAGetLastError() == 10035 && DAT_055cc16c + v158 <= 0x2000 )
        {
          goto LABEL_561;
        }
        goto LABEL_571;
      }
LABEL_574:
      v324 = -1;
      goto LABEL_576;
    }
    v162 = v157 + 3;
    v319 = -60;
    v321 = v157 + 3;
    v320 = (v157 + 3) / 256;
    FUN_0053cc30((int)v322, (int)v156, v155);
    v163 = 0;
    v129 = v162;
    if ( s == -1 )
    {
      goto LABEL_574;
    }
    while ( 1 )
    {
      v164 = send(s, &v319 + v163, v162 - v163, 0);
      v165 = v164;
      if ( v164 == -1 )
      {
        break;
      }
      if ( v164 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2((int)&v319, v164);
        }
        v129 -= v165;
        v163 += v165;
        if ( v129 > 0 )
        {
          continue;
        }
      }
      goto LABEL_574;
    }
    if ( WSAGetLastError() != 10035 || DAT_055cc16c + v162 > 0x2000 )
    {
      goto LABEL_571;
    }
LABEL_572:
    qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v319, v129);
    v199 = v129 + DAT_055cc16c;
    goto LABEL_573;
  }
  DAT_07ea9810 = v100;
  DAT_07ea9808 = *(DWORD *)&Inventory[32].Type;
  v132 = 0;
  DAT_07ea9804 = 2;
  DAT_07ea980c = 0;
  v133 = DAT_07e91394;
  do
  {
    *v133++ = v132++;
  }
  while ( (int)v133 < (int)g_GuildNotice );
  v134 = 20;
  do
  {
    v135 = rand() % 10;
    v136 = rand() % 10;
    if ( v135 != v136 )
    {
      DAT_07e91394[v135] ^= DAT_07e91394[v136];
      DAT_07e91394[v136] ^= DAT_07e91394[v135];
      DAT_07e91394[v135] ^= DAT_07e91394[v136];
    }
    --v134;
  }
  while ( v134 );
  DAT_07eaa14c = 1;
  *(DWORD *)DAT_07ea9814 = 0;
  DAT_07ea9818 = 0;
  DAT_07ea981c = 0;
  DAT_07ea981e = 0;
LABEL_576:
  v309 = (BYTE *)CharacterMachine;
  v307 = (BYTE *)CharacterMachine;
  v260 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v261 = 0;
  v308 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v308, (const char *)(DAT_055c9bd0 + 4 * v260), 4) )
    {
      if ( !memcmp((const char *)&v307, (const char *)(DAT_055c9bd0 + 4 * v260), 4) )
      {
        if ( v260 != -1 )
        {
          v307 = v309;
          v262 = (*(int (__cdecl **)(int *, BYTE *))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, v309);
          v263 = 0;
          v308 = 0;
          if ( DAT_055c9bd4 )
          {
            while ( memcmp((const char *)&v308, (const char *)(DAT_055c9bd0 + 4 * v262), 4) )
            {
              if ( !memcmp((const char *)&v307, (const char *)(DAT_055c9bd0 + 4 * v262), 4) )
              {
                if ( v262 == -1 )
                {
                  break;
                }
                v264 = *(char **)(DAT_055c9bcc + 4 * v262);
                v306 = v264;
                goto LABEL_588;
              }
              ++v263;
              v262 = (v262 + 1) % DAT_055c9bd4;
              if ( v263 >= DAT_055c9bd4 )
              {
                goto LABEL_586;
              }
            }
          }
          else
          {
LABEL_586:
            CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
          }
          v306 = 0;
          v264 = 0;
LABEL_588:
          v265 = v264[1412] - 1;
          v264[1412] = v265;
          if ( !v265 )
          {
            v266 = (BYTE *)operator_new(0x584u);
            v267 = v309;
            v268 = v266;
            qmemcpy(v266, v309, 0x584u);
            v269 = 0;
            v270 = v267 - v266;
            do
            {
              v271 = &v268[v269];
              v272 = ((v268[v269] + 71) ^ DAT_00559050[v269 & 0x8000000F]) + 35;
              v268[v269] = v272;
              if ( v269 < 0x583 )
              {
                *v271 = v272 ^ v268[v269 + 1];
              }
              ++v269;
              v271[v270] = rand();
            }
            while ( v269 < 0x584 );
            qmemcpy(v306, v268, 0x584u);
            delete__(v268);
          }
        }
        return;
      }
      v260 = (v260 + 1) % DAT_055c9bd4;
      if ( ++v261 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_580;
      }
    }
  }
  else
  {
LABEL_580:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
}
#endif

// ── FUN_004d1fc0 (IDA-only, gated) ──
#if defined(IDA_PORT_004D1FC0)
void FUN_004d1fc0()
{
  void *v0; // ebx
  unsigned int v1; // eax
  bool v2; // cf
  int v3; // eax
  const void *v4; // ebp
  unsigned int v5; // edx
  unsigned int v6; // eax
  BYTE *v7; // eax
  unsigned char v8; // cl
  unsigned int v9; // eax
  char *v10; // eax
  char v11; // cl
  char *v12; // ebx
  unsigned int v13; // esi
  int v14; // ebp
  char *v15; // edi
  char v16; // al
  int v17; // [esp+10h] [ebp-Ch]
  int v18; // [esp+10h] [ebp-Ch]
  char *v19; // [esp+10h] [ebp-Ch]
  DWORD v20; // [esp+14h] [ebp-8h] BYREF
  DWORD v21; // [esp+18h] [ebp-4h] BYREF

  FUN_004cdc70(15.0, 46.0, 40.0, 40.0, 8);
  FUN_004cdc70(115.0, 46.0, 60.0, 40.0, 7);
  v0 = (void *)CharacterMachine;
  v21 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v20 = 0;
  v17 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v20, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v6 = FUN_004041e0(&DAT_055c9bc8, (int)v0);
        if ( v6 == -1 )
        {
          v7 = 0;
        }
        else
        {
          v7 = *(BYTE **)(DAT_055c9bcc + 4 * v6);
        }
        v8 = v7[1412] + 1;
        v7[1412] = v8;
        if ( v8 < 2u )
        {
          FUN_00404370(v0, v7);
        }
        goto LABEL_7;
      }
      v2 = ++v17 < (unsigned int)DAT_055c9bd4;
      v1 = (v1 + 1) % DAT_055c9bd4;
      if ( !v2 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v3 = operator_new(0x585u);
  *(BYTE *)(v3 + 1412) = 1;
  FUN_00403f80(&DAT_055c9bc8, v3, (int)v0);
LABEL_7:
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) != 3 )
  {
    FUN_004cdc70(75.0, 46.0, 40.0, 40.0, 2);
  }
  v4 = (const void *)CharacterMachine;
  v20 = CharacterMachine;
  v5 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
  v21 = 0;
  v18 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
    {
      if ( !memcmp((const char *)&v20, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
      {
        if ( v5 != -1 )
        {
          v9 = FUN_004041e0(&DAT_055c9bc8, (int)v4);
          if ( v9 == -1 )
          {
            v19 = 0;
            v10 = 0;
          }
          else
          {
            v10 = *(char **)(DAT_055c9bcc + 4 * v9);
            v19 = v10;
          }
          v11 = v10[1412] - 1;
          v10[1412] = v11;
          if ( !v11 )
          {
            v12 = (char *)operator_new(0x584u);
            qmemcpy(v12, v4, 0x584u);
            v13 = 0;
            v14 = (BYTE *)v4 - v12;
            do
            {
              v15 = &v12[v13];
              v16 = ((v12[v13] + 71) ^ DAT_00559050[v13 & 0x8000000F]) + 35;
              v12[v13] = v16;
              if ( v13 < 0x583 )
              {
                *v15 = v16 ^ v12[v13 + 1];
              }
              ++v13;
              v15[v14] = rand();
            }
            while ( v13 < 0x584 );
            qmemcpy(v19, v12, 0x584u);
            delete__(v12);
          }
        }
        break;
      }
      v5 = (v5 + 1) % DAT_055c9bd4;
      if ( ++v18 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_13;
      }
    }
  }
  else
  {
LABEL_13:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  FUN_004cdc70(75.0, 89.0, 40.0, 60.0, 3);
  FUN_004cdc70(75.0, 152.0, 40.0, 40.0, 4);
  FUN_004cdc70(15.0, 89.0, 40.0, 60.0, 0);
  FUN_004cdc70(134.0, 89.0, 40.0, 60.0, 1);
  FUN_004cdc70(15.0, 152.0, 40.0, 40.0, 5);
  FUN_004cdc70(134.0, 152.0, 40.0, 40.0, 6);
  FUN_004cdc70(55.0, 89.0, 20.0, 20.0, 9);
  FUN_004cdc70(55.0, 152.0, 20.0, 20.0, 10);
  FUN_004cdc70(115.0, 152.0, 20.0, 20.0, 11);
}
#endif

// ── FUN_004d23b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004D23B0)
void __cdecl FUN_004d23b0(char *a1, int a2, short *a3, signed int a4, int a5, char a6)
{
  signed int v6; // ebx
  int v7; // edx
  int v8; // eax
  char *v9; // ecx
  ITEM_ATTRIBUTE *v10; // eax
  unsigned char *v11; // edi
  int Width; // ebp
  WORD *Height; // ecx
  int v14; // eax
  int v15; // esi
  int v16; // edx
  int v17; // eax
  short v18; // cx
  int v19; // esi
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // eax
  int v24; // ecx
  int v25; // ebp
  int v26; // esi
  BYTE v27; // al
  BYTE *v28; // eax
  char v29; // cl
  int v30; // esi
  int v31; // edi
  char *v32; // ebp
  int v33; // eax
  signed int v34; // ebx
  int v35; // edi
  signed int v36; // ebp
  int v37; // eax
  int v38; // esi
  int v39; // eax
  signed int v40; // esi
  int v41; // ebp
  signed int v42; // ebx
  int v43; // eax
  int v44; // edi
  int v45; // eax
  bool v46; // cc
  int v47; // ebx
  int v48; // ecx
  int v49; // ebx
  int v50; // esi
  BYTE v51; // al
  int v52; // edi
  char *v53; // esi
  int v54; // eax
  unsigned int v55; // ebx
  int v56; // edi
  unsigned int v57; // ebp
  int v58; // eax
  int v59; // esi
  char *v60; // edi
  char *v61; // edi
  char *v62; // esi
  char v63; // cl
  unsigned int v64; // ebx
  int v65; // edi
  int v66; // eax
  int v67; // esi
  char *v68; // edi
  int v69; // ebx
  int v70; // ecx
  int v71; // eax
  bool v72; // bl
  BYTE *v73; // eax
  char v74; // cl
  int v75; // eax
  int v76; // ebp
  int v77; // edi
  char *v78; // eax
  char v79; // cl
  int v80; // eax
  BYTE v81; // al
  int v82; // edi
  int v83; // esi
  char *v84; // ebp
  int v85; // eax
  signed int v86; // ebx
  int v87; // edi
  signed int v88; // ebp
  int v89; // eax
  int v90; // esi
  int v91; // eax
  signed int v92; // edi
  int v93; // ebp
  signed int v94; // ebx
  int v95; // eax
  int v96; // esi
  int *v97; // ebp
  short v98; // ax
  int v99; // ecx
  bool v100; // zf
  char *v101; // eax
  char v102; // cl
  int v103; // eax
  bool v104; // bl
  int v105; // ecx
  int v106; // ecx
  int v107; // eax
  int v108; // ebp
  int v109; // edi
  char *v110; // eax
  char v111; // cl
  int v112; // eax
  BYTE v113; // al
  int v114; // edi
  int v115; // esi
  char *v116; // ebp
  int v117; // eax
  signed int v118; // ebx
  int v119; // edi
  signed int v120; // ebp
  int v121; // eax
  int v122; // esi
  int v123; // eax
  signed int v124; // edi
  int v125; // ebp
  signed int v126; // ebx
  int v127; // eax
  int v128; // esi
  short v129; // ax
  int v130; // ecx
  char v131; // al
  int v132; // ecx
  int v133; // eax
  int v134; // ebp
  int v135; // edi
  signed int v136; // ebx
  int v137; // eax
  int v138; // esi
  int v139; // ecx
  char v140; // al
  int v141; // ecx
  int v142; // eax
  int v143; // ebp
  int v144; // edi
  int v145; // eax
  int v146; // esi
  unsigned int v147; // eax
  char *v148; // eax
  char v149; // cl
  int v150; // eax
  bool v151; // bl
  int v152; // ecx
  int v153; // ecx
  int v154; // eax
  int v155; // ebp
  int v156; // esi
  char *v157; // eax
  char v158; // cl
  int v159; // eax
  BYTE v160; // al
  BYTE *v161; // eax
  char v162; // cl
  int v163; // esi
  int v164; // edi
  char *v165; // ebp
  int v166; // eax
  signed int v167; // ebx
  int v168; // edi
  signed int v169; // ebp
  int v170; // eax
  int v171; // esi
  int v172; // eax
  signed int v173; // esi
  int v174; // ebp
  signed int v175; // ebx
  int v176; // eax
  int v177; // edi
  void *v178; // ebx
  unsigned int v179; // eax
  bool v180; // cf
  int v181; // eax
  unsigned int v182; // eax
  char *v183; // eax
  unsigned int v184; // eax
  BYTE *v185; // esi
  unsigned char v186; // al
  void *v187; // ebp
  unsigned int v188; // ecx
  int v189; // esi
  char v190; // cl
  int v191; // eax
  bool v192; // bl
  unsigned int v193; // eax
  BYTE *v194; // eax
  char v195; // cl
  int v196; // eax
  int v197; // eax
  int v198; // ecx
  int v199; // ebx
  int v200; // esi
  unsigned int v201; // eax
  char *v202; // eax
  char v203; // cl
  int v204; // eax
  BYTE v205; // al
  unsigned int v206; // eax
  BYTE *v207; // eax
  char v208; // cl
  int v209; // esi
  int v210; // edi
  char *v211; // ebp
  int v212; // eax
  signed int v213; // ebx
  int v214; // edi
  signed int v215; // ebp
  int v216; // eax
  int v217; // esi
  int v218; // eax
  signed int v219; // esi
  int v220; // ebp
  signed int v221; // ebx
  int v222; // eax
  int v223; // edi
  short v224; // ax
  unsigned int v225; // edx
  int v226; // ebp
  unsigned int v227; // eax
  unsigned int v228; // ebp
  short *v229; // eax
  char v230; // cl
  char *v231; // eax
  char *v232; // ebp
  BYTE *v233; // ebx
  unsigned int v234; // esi
  int v235; // ebp
  BYTE *v236; // edi
  char v237; // al
  unsigned int v238; // eax
  unsigned int v239; // ebx
  unsigned int v240; // eax
  unsigned int v241; // ebx
  unsigned int v242; // eax
  unsigned int v243; // ebx
  BYTE *v244; // esi
  unsigned char v245; // al
  void *v246; // ebp
  unsigned int v247; // ecx
  int v248; // esi
  unsigned int v249; // edx
  int v250; // ebx
  unsigned int v251; // edx
  int v252; // ebx
  char *v253; // edx
  char v254; // al
  char *v255; // eax
  char *v256; // ebx
  BYTE *v257; // ebp
  unsigned int v258; // esi
  int v259; // ebx
  BYTE *v260; // edi
  char v261; // al
  int v262; // eax
  char v263; // [esp+10h] [ebp-D84h]
  char v264; // [esp+11h] [ebp-D83h]
  char v265; // [esp+12h] [ebp-D82h]
  char v266; // [esp+13h] [ebp-D81h]
  char v267; // [esp+14h] [ebp-D80h]
  char v268; // [esp+15h] [ebp-D7Fh]
  char v269; // [esp+16h] [ebp-D7Eh]
  char v270; // [esp+17h] [ebp-D7Dh]
  char v271; // [esp+18h] [ebp-D7Ch]
  char v272; // [esp+19h] [ebp-D7Bh]
  char v273; // [esp+1Ah] [ebp-D7Ah]
  char v274; // [esp+1Bh] [ebp-D79h]
  char v275; // [esp+1Ch] [ebp-D78h]
  char v276; // [esp+1Dh] [ebp-D77h]
  char v277; // [esp+1Eh] [ebp-D76h]
  char v278; // [esp+1Fh] [ebp-D75h]
  int v279; // [esp+20h] [ebp-D74h]
  int v280; // [esp+24h] [ebp-D70h]
  int v281; // [esp+28h] [ebp-D6Ch]
  int v282; // [esp+2Ch] [ebp-D68h]
  int v283; // [esp+30h] [ebp-D64h] BYREF
  WORD *v284; // [esp+34h] [ebp-D60h] BYREF
  short *v285; // [esp+38h] [ebp-D5Ch]
  char *v286; // [esp+3Ch] [ebp-D58h]
  int v287; // [esp+40h] [ebp-D54h]
  short *v288; // [esp+44h] [ebp-D50h]
  char *v289; // [esp+48h] [ebp-D4Ch]
  int v290; // [esp+4Ch] [ebp-D48h]
  char v291[32]; // [esp+50h] [ebp-D44h] BYREF
  unsigned char *v292; // [esp+70h] [ebp-D24h]
  void *(__cdecl **v293)(std::locale::facet *__hidden, unsigned int); // [esp+74h] [ebp-D20h]
  unsigned short v294; // [esp+78h] [ebp-D1Ch]
  unsigned char v295; // [esp+7Ah] [ebp-D1Ah] BYREF
  WORD v296[512]; // [esp+7Bh] [ebp-D19h] BYREF
  char buf; // [esp+47Ch] [ebp-918h] BYREF
  char v298; // [esp+47Dh] [ebp-917h]
  char v299[258]; // [esp+47Eh] [ebp-916h] BYREF
  char v300[4]; // [esp+580h] [ebp-814h] BYREF
  char v301[5]; // [esp+584h] [ebp-810h] BYREF
  char v302; // [esp+589h] [ebp-80Bh]
  char v304[1024]; // [esp+988h] [ebp-40Ch] BYREF
  int v305; // [esp+D90h] [ebp-4h]

  if ( EnableUse > 0 )
  {
    return;
  }
  if ( EquipmentItem )
  {
    return;
  }
  if ( DAT_07e91388 > 0 )
  {
    return;
  }
  v286 = 0;
  if ( a5 <= 0 )
  {
    return;
  }
  v6 = a4;
  v7 = a2;
  v287 = a2;
  v8 = 68 * a4;
  v283 = 68 * a4;
  v288 = a3;
  while ( 1 )
  {
    v290 = 0;
    if ( v6 > 0 )
    {
      break;
    }
LABEL_87:
    v7 += 20;
    v46 = (int)++v286 < a5;
    v287 = v7;
    v288 = (short *)((char *)v288 + v8);
    if ( !v46 )
    {
      return;
    }
  }
  v9 = a1;
  v289 = a1;
  v285 = v288;
  while ( 1 )
  {
    if ( MouseX < (int)v9 || MouseX >= (int)(v9 + 20) || MouseY < v7 || MouseY >= v7 + 20 || *v285 == -1 )
    {
      goto LABEL_85;
    }
    v10 = &ItemAttribute[*v285];
    v11 = (unsigned char *)v285 + 63;
    Width = v10->Width;
    Height = (WORD *)v10->Height;
    v14 = *((unsigned char *)v285 + 63);
    v284 = Height;
    v15 = v14;
    if ( v14 < (int)Height + v14 )
    {
      v292 = (unsigned char *)(v285 + 31);
      v16 = v6 * v14;
      do
      {
        v17 = *v292;
        if ( v17 < v17 + Width )
        {
          do
          {
            if ( a3 != (short *)Inventory || (BYTE)(a3[34 * v16 + 32 + 34 * v17]) != 99 )
            {
              (BYTE)(a3[34 * v16 + 32 + 34 * v17]) = 2;
            }
            ++v17;
          }
          while ( v17 < Width + *v292 );
          Height = v284;
        }
        ++v15;
        v16 += a4;
      }
      while ( v15 < (int)Height + *v11 );
    }
    CheckInventory = (DWORD)v285;
    v18 = *v285;
    v19 = *((unsigned char *)v285 + 62);
    v20 = ItemAttribute[*v285].Width;
    DAT_07ea9844 = a6;
    sx = (int)&a1[20 * v19 + 20 * v20 / 2];
    v21 = *v11;
    sy = a2 + 20 * v21;
    if ( a3 == (short *)Inventory )
    {
      goto LABEL_85;
    }
    if ( a6 )
    {
      if ( !MouseLButtonPush )
      {
        return;
      }
      if ( *(DWORD *)&RepairEnable_0 )
      {
        return;
      }
      MouseLButtonPush = 0;
      if ( DAT_05826d18 )
      {
        return;
      }
      v47 = *((unsigned char *)v285 + 62) + a4 * *((unsigned char *)v285 + 63);
      v283 = ItemValue((int)&a3[34 * v47], 0);
      if ( DAT_05826d18 )
      {
        return;
      }
      v293 = &DAT_00552460;
      v295 = -63;
      v305 = 0;
      v296[0] = 12801;
      v294 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      (BYTE)(v296[1]) = v47;
      v48 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do
      {
        *(&v295 + v48) ^= *((BYTE *)&v294 + v48 + 1) ^ *(&v263 + v48 % 32);
        ++v48;
      }
      while ( v48 != 4 );
      ++v294;
      if ( v295 == 193 )
      {
        (BYTE)(v296[0]) = v294;
      }
      else if ( v295 == 194 )
      {
        v296[0] = v294;
      }
      v49 = v294;
      qmemcpy(v304, &v295, v294);
      v304[v49] = rand();
      v50 = (v304[0] != -63) + 2;
      PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
      v51 = g_byPacketSerialSend;
      v304[v50 - 1] = g_byPacketSerialSend;
      g_byPacketSerialSend = v51 + 1;
      PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
      --v50;
      v52 = v49 - v50;
      v53 = &v304[v50];
      v54 = FUN_0053cc30(0, (int)v53, v52);
      if ( v54 >= 256 )
      {
        v64 = v54 + 3;
        v300[0] = -60;
        v300[2] = v54 + 3;
        v300[1] = (v54 + 3) / 256;
        FUN_0053cc30((int)&v300[3], (int)v53, v52);
        v65 = 0;
        v57 = v64;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v66 = send(s, &v300[v65], v64 - v65, 0);
            v67 = v66;
            if ( v66 == -1 )
            {
              break;
            }
            if ( v66 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2((int)v300, v66);
              }
              v57 -= v67;
              v65 += v67;
              if ( (int)v57 > 0 )
              {
                continue;
              }
            }
            goto LABEL_123;
          }
          if ( WSAGetLastError() == 10035 && (int)(DAT_055cc16c + v64) <= 0x2000 )
          {
            v68 = (char *)&DAT_055ca16c + DAT_055cc16c;
            qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, v300, 4 * (v57 >> 2));
            v62 = &v300[4 * (v57 >> 2)];
            v61 = &v68[4 * (v57 >> 2)];
            v63 = v57;
            goto LABEL_122;
          }
LABEL_120:
          CWsctlc::Close((DWORD)&SocketClient);
          DAT_05826d18 = v283;
          return;
        }
      }
      else
      {
        v55 = v54 + 2;
        buf = -61;
        v298 = v54 + 2;
        FUN_0053cc30((int)v299, (int)v53, v52);
        v56 = 0;
        v57 = v55;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v58 = send(s, &buf + v56, v55 - v56, 0);
            v59 = v58;
            if ( v58 == -1 )
            {
              break;
            }
            if ( v58 )
            {
              if ( DAT_055ce174 )
              {
                nullsub_2((int)&buf, v58);
              }
              v57 -= v59;
              v56 += v59;
              if ( (int)v57 > 0 )
              {
                continue;
              }
            }
            goto LABEL_123;
          }
          if ( WSAGetLastError() == 10035 && (int)(DAT_055cc16c + v55) <= 0x2000 )
          {
            v60 = (char *)&DAT_055ca16c + DAT_055cc16c;
            qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf, 4 * (v57 >> 2));
            v62 = &buf + 4 * (v57 >> 2);
            v61 = &v60[4 * (v57 >> 2)];
            v63 = v57;
LABEL_122:
            qmemcpy(v61, v62, v63 & 3);
            DAT_055cc16c += v57;
            goto LABEL_123;
          }
          goto LABEL_120;
        }
      }
LABEL_123:
      DAT_05826d18 = v283;
      return;
    }
    if ( *(DWORD *)&RepairEnable_0 )
    {
      if ( (v18 < 416 || v18 > 419)
        && v18 != 426
        && v18 != 135
        && v18 != 143
        && v18 < 448
        && (v18 < 391 || v18 > 403)
        && (v18 < 430 || v18 > 435)
        && MouseLButtonPush )
      {
        MouseLButtonPush = 0;
        v293 = &DAT_00552460;
        v295 = -63;
        v305 = 1;
        v296[0] = 13313;
        v294 = 3;
        qmemcpy(v291, "çm:", 3);
        (BYTE)(v296[1]) = a4 * v21 + v19 + 12;
        v22 = 3;
        v291[3] = -119;
        v291[4] = -68;
        v291[5] = -78;
        v291[6] = -97;
        v291[7] = 115;
        v291[8] = 35;
        v291[9] = -88;
        v291[10] = -2;
        v291[11] = -74;
        v291[12] = 73;
        v291[13] = 93;
        v291[14] = 57;
        v291[15] = 93;
        v291[16] = -118;
        v291[17] = -53;
        v291[18] = 99;
        v291[19] = -115;
        v291[20] = -22;
        v291[21] = 125;
        v291[22] = 43;
        v291[23] = 95;
        v291[24] = -61;
        v291[25] = -79;
        v291[26] = -23;
        v291[27] = -125;
        v291[28] = 41;
        v291[29] = 81;
        v291[30] = -24;
        v291[31] = 86;
        do
        {
          *(&v295 + v22) ^= *((BYTE *)&v294 + v22 + 1) ^ v291[v22 % 32];
          ++v22;
        }
        while ( v22 != 4 );
        if ( ++v294 + 1 <= 1024 )
        {
          v263 = -25;
          v264 = 109;
          v265 = 58;
          *(&v295 + v294) = RepairEnable;
          v23 = v294;
          v266 = -119;
          v267 = -68;
          v268 = -78;
          v24 = v294 + 1;
          v269 = -97;
          v270 = 115;
          v271 = 35;
          v272 = -88;
          v273 = -2;
          v274 = -74;
          v275 = 73;
          v276 = 93;
          v277 = 57;
          v278 = 93;
          v279 = -1922839670;
          v280 = 1596685802;
          v281 = -2081836605;
          v282 = 1458065705;
          if ( v294 != v24 )
          {
            do
            {
              *(&v295 + v23) ^= *((BYTE *)&v294 + v23 + 1) ^ *(&v263 + v23 % 32);
              ++v23;
            }
            while ( v23 != v24 );
          }
          ++v294;
        }
        if ( v295 == 193 )
        {
          (BYTE)(v296[0]) = v294;
        }
        else if ( v295 == 194 )
        {
          v296[0] = v294;
        }
        v25 = v294;
        qmemcpy(v304, &v295, v294);
        v304[v25] = rand();
        v26 = (v304[0] != -63) + 2;
        PACKET_DECRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
        v27 = g_byPacketSerialSend;
        v304[v26 - 1] = g_byPacketSerialSend;
        g_byPacketSerialSend = v27 + 1;
        if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) != -1 )
        {
          v28 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
          v29 = v28[1] - 1;
          v28[1] = v29;
          if ( !v29 )
          {
            FUN_00423710(v28, &g_byPacketSerialSend);
          }
        }
        v30 = v26 - 1;
        v31 = v25 - v30;
        v32 = &v304[v30];
        v33 = FUN_0053cc30(0, (int)&v304[v30], v31);
        if ( v33 >= 256 )
        {
          v40 = v33 + 3;
          v300[0] = -60;
          v300[2] = v33 + 3;
          v300[1] = (v33 + 3) / 256;
          FUN_0053cc30((int)&v300[3], (int)v32, v31);
          v41 = 0;
          v42 = v40;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v43 = send(s, &v300[v41], v40 - v41, 0);
              v44 = v43;
              if ( v43 == -1 )
              {
                break;
              }
              if ( v43 )
              {
                if ( DAT_055ce174 )
                {
                  nullsub_2((int)v300, v43);
                }
                v42 -= v44;
                v41 += v44;
                if ( v42 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_74;
            }
            if ( WSAGetLastError() != 10035 || DAT_055cc16c + v40 > 0x2000 )
            {
              goto LABEL_71;
            }
            qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, v300, v42);
            v39 = v42 + DAT_055cc16c;
LABEL_73:
            DAT_055cc16c = v39;
          }
        }
        else
        {
          v34 = v33 + 2;
          buf = -61;
          v298 = v33 + 2;
          FUN_0053cc30((int)v299, (int)&v304[v30], v31);
          v35 = 0;
          v36 = v34;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v37 = send(s, &buf + v35, v34 - v35, 0);
              v38 = v37;
              if ( v37 == -1 )
              {
                break;
              }
              if ( v37 )
              {
                if ( DAT_055ce174 )
                {
                  nullsub_2((int)&buf, v37);
                }
                v36 -= v38;
                v35 += v38;
                if ( v36 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_74;
            }
            if ( WSAGetLastError() == 10035 && DAT_055cc16c + v34 <= 0x2000 )
            {
              qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf, v36);
              v39 = v36 + DAT_055cc16c;
              goto LABEL_73;
            }
LABEL_71:
            CWsctlc::Close((DWORD)&SocketClient);
          }
        }
LABEL_74:
        v305 = -1;
        v293 = &DAT_00552460;
        goto LABEL_85;
      }
      goto LABEL_85;
    }
    if ( MouseLButtonPush )
    {
      v69 = *((unsigned char *)v285 + 62);
      v70 = a4 * *((unsigned char *)v285 + 63);
      MouseLButtonPush = 0;
      v71 = v70 + v69;
      DAT_07ea9800 = (int)a3;
      qmemcpy(&pPickedItem, &a3[34 * v70 + 34 * v69], 0x44u);
      if ( a3 == (short *)&OffsetInventoryItems )
      {
        v71 += 12;
      }
      *(DWORD *)&Inventory[32].Type = v71;
      UI_Main(v71, a3, a4);
      CheckInventory = 0;
      FUN_0043d8a0(&DAT_055c9bc8, &TradeOpened);
      v72 = TradeOpened;
      if ( FUN_004041e0(&DAT_055c9bc8, (int)&TradeOpened) != -1 )
      {
        v73 = (BYTE *)FUN_00404280(&DAT_055c9bc8, (int)&TradeOpened);
        v74 = v73[1] - 1;
        v73[1] = v74;
        if ( !v74 )
        {
          FUN_00423710(v73, &TradeOpened);
        }
      }
      if ( v72 && m_bMyConfirm && a3 == OffsetTradeItems )
      {
        m_bMyConfirm = 0;
        v293 = &DAT_00552460;
        v295 = -63;
        v305 = 2;
        (BYTE)(v296[0]) = 1;
        strcpy((char *)v296 + 1, "<");
        v294 = 3;
        v263 = -25;
        v264 = 109;
        v265 = 58;
        v75 = 3;
        v266 = -119;
        v267 = -68;
        v268 = -78;
        v269 = -97;
        v270 = 115;
        v271 = 35;
        v272 = -88;
        v273 = -2;
        v274 = -74;
        v275 = 73;
        v276 = 93;
        v277 = 57;
        v278 = 93;
        v279 = -1922839670;
        v280 = 1596685802;
        v281 = -2081836605;
        v282 = 1458065705;
        do
        {
          *(&v295 + v75) ^= *((BYTE *)&v294 + v75 + 1) ^ *(&v263 + v75 % 32);
          ++v75;
        }
        while ( v75 != 4 );
        ++v294;
        if ( v295 == 193 )
        {
          (BYTE)(v296[0]) = v294;
        }
        else if ( v295 == 194 )
        {
          v296[0] = v294;
        }
        v76 = v294;
        qmemcpy(v304, &v295, v294);
        v304[v76] = rand();
        v77 = (v304[0] != -63) + 2;
        if ( FUN_004041e0(&DAT_055c9bc8, (int)&g_byPacketSerialSend) == -1 )
        {
          v80 = operator_new(2u);
          *(BYTE *)(v80 + 1) = 1;
          FUN_00403f80(&DAT_055c9bc8, v80, (int)&g_byPacketSerialSend);
        }
        else
        {
          v78 = (char *)FUN_00404280(&DAT_055c9bc8, (int)&g_byPacketSerialSend);
          v79 = v78[1] + 1;
          v78[1] = v79;
          if ( (unsigned char)v79 < 2u )
          {
            FUN_00404330(&g_byPacketSerialSend, v78);
          }
        }
        v81 = g_byPacketSerialSend;
        v304[v77 - 1] = g_byPacketSerialSend;
        g_byPacketSerialSend = v81 + 1;
        PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
        v82 = v77 - 1;
        v83 = v76 - v82;
        v84 = &v304[v82];
        v85 = FUN_0053cc30(0, (int)&v304[v82], v83);
        if ( v85 >= 256 )
        {
          v92 = v85 + 3;
          v300[0] = -60;
          v300[2] = v85 + 3;
          v300[1] = (v85 + 3) / 256;
          FUN_0053cc30((int)&v300[3], (int)v84, v83);
          v93 = 0;
          v94 = v92;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v95 = send(s, &v300[v93], v92 - v93, 0);
              v96 = v95;
              if ( v95 == -1 )
              {
                break;
              }
              if ( v95 )
              {
                if ( DAT_055ce174 )
                {
                  nullsub_2((int)v300, v95);
                }
                v94 -= v96;
                v93 += v96;
                if ( v94 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_166;
            }
            if ( WSAGetLastError() != 10035 || DAT_055cc16c + v92 > 0x2000 )
            {
              goto LABEL_163;
            }
            qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, v300, v94);
            v91 = v94 + DAT_055cc16c;
LABEL_165:
            DAT_055cc16c = v91;
          }
        }
        else
        {
          v86 = v85 + 2;
          buf = -61;
          v298 = v85 + 2;
          FUN_0053cc30((int)v299, (int)&v304[v82], v83);
          v87 = 0;
          v88 = v86;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v89 = send(s, &buf + v87, v86 - v87, 0);
              v90 = v89;
              if ( v89 == -1 )
              {
                break;
              }
              if ( v89 )
              {
                if ( DAT_055ce174 )
                {
                  nullsub_2((int)&buf, v89);
                }
                v88 -= v90;
                v87 += v90;
                if ( v88 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_166;
            }
            if ( WSAGetLastError() == 10035 && DAT_055cc16c + v86 <= 0x2000 )
            {
              qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf, v88);
              v91 = v88 + DAT_055cc16c;
              goto LABEL_165;
            }
LABEL_163:
            CWsctlc::Close((DWORD)&SocketClient);
          }
        }
LABEL_166:
        v305 = -1;
      }
      PlayBuffer(29, 0, 0);
      return;
    }
    if ( MouseRButtonPush )
    {
      break;
    }
    if ( !((unsigned short)GetAsyncKeyState(17) >> 8) )
    {
      goto LABEL_85;
    }
    if ( PressKey(81) )
    {
      v45 = 0;
    }
    else if ( PressKey(87) )
    {
      v45 = 1;
    }
    else
    {
      if ( !PressKey(69) )
      {
        goto LABEL_85;
      }
      v45 = 2;
    }
    DAT_00559c60[v45] = *(short *)CheckInventory;
LABEL_85:
    v6 = a4;
    v9 = v289 + 20;
    v46 = ++v290 < a4;
    v289 += 20;
    v285 += 34;
    if ( !v46 )
    {
      v8 = v283;
      v7 = v287;
      goto LABEL_87;
    }
    v7 = v287;
  }
  v97 = (int *)v285;
  v98 = *v285;
  v99 = *((unsigned char *)v285 + 62) + a4 * *((unsigned char *)v285 + 63);
  v100 = *v285 == 458;
  MouseRButtonPush = 0;
  v287 = v99;
  if ( !v100 )
  {
    if ( v98 == 467 )
    {
      *(DWORD *)v300 = &DAT_00552460;
      *(WORD *)&v301[2] = 449;
      v305 = 4;
      v301[4] = -111;
      *(WORD *)v301 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      v302 = 1;
      v130 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do
      {
        v301[v130 + 2] ^= v301[v130 + 1] ^ *(&v263 + v130 % 32);
        ++v130;
      }
      while ( v130 != 4 );
      ++*(WORD *)v301;
      v131 = (v97[1] >> 3) & 0xF;
      if ( *(unsigned short *)v301 + 1 <= 1024 )
      {
        v263 = -25;
        v264 = 109;
        v265 = 58;
        v301[*(unsigned short *)v301 + 2] = v131;
        v132 = *(unsigned short *)v301;
        v266 = -119;
        v133 = *(unsigned short *)v301 + 1;
        v267 = -68;
        v268 = -78;
        v269 = -97;
        v270 = 115;
        v271 = 35;
        v272 = -88;
        v273 = -2;
        v274 = -74;
        v275 = 73;
        v276 = 93;
        v277 = 57;
        v278 = 93;
        v279 = -1922839670;
        v280 = 1596685802;
        v281 = -2081836605;
        v282 = 1458065705;
        if ( *(unsigned short *)v301 != v133 )
        {
          do
          {
            v301[v132 + 2] ^= v301[v132 + 1] ^ *(&v263 + v132 % 32);
            ++v132;
          }
          while ( v132 != v133 );
        }
        ++*(WORD *)v301;
      }
      if ( (unsigned char)v301[2] == 193 )
      {
        v301[3] = v301[0];
      }
      else if ( (unsigned char)v301[2] == 194 )
      {
        *(WORD *)&v301[3] = *(WORD *)v301;
      }
      v134 = *(unsigned short *)v301;
      v135 = 0;
      v136 = *(unsigned short *)v301;
      if ( s == -1 )
      {
        return;
      }
      while ( 1 )
      {
        v137 = send(s, &v301[v135 + 2], v134 - v135, 0);
        v138 = v137;
        if ( v137 == -1 )
        {
          break;
        }
        if ( v137 )
        {
          if ( DAT_055ce174 )
          {
            nullsub_2((int)&v301[2], v137);
          }
          v136 -= v138;
          v135 += v138;
          if ( v136 > 0 )
          {
            continue;
          }
        }
        return;
      }
      if ( WSAGetLastError() == 10035 && DAT_055cc16c + v134 <= 0x2000 )
      {
        goto LABEL_239;
      }
      goto LABEL_260;
    }
    if ( v98 == 434 )
    {
      *(DWORD *)v300 = &DAT_00552460;
      *(WORD *)&v301[2] = 449;
      v305 = 5;
      v301[4] = -111;
      *(WORD *)v301 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      v302 = 2;
      v139 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do...
      ++*(WORD *)v301;
      v140 = (v97[1] >> 3) & 0xF;
      if...
      if...
      v143 = *(unsigned short *)v301;
      v144 = 0;
      v136 = *(unsigned short *)v301;
      if...
      while...
      if ( WSAGetLastError() == 10035 && DAT_055cc16c + v143 <= 0x2000 )
      {
LABEL_239:
        qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &v301[2], v136);
        DAT_055cc16c += v136;
        return;
      }
LABEL_260:
      CWsctlc::Close((DWORD)&SocketClient);
      return;
    }
    if ( v98 >= 448 && v98 <= 454 || v98 >= 456 && v98 <= 457 || v98 == 468 )
    {
      if ( WarehouseOpened )
      {
        goto LABEL_322;
      }
      if...
      v151 = TradeOpened;
      PACKET_ENCRYPT(&DAT_055c9bc8, &TradeOpened);
      if ( v151 )
      {
LABEL_322:
        UIChatLogWindow_AddText(DAT_07eaa188, GlobalText[474], 2);
        return;
      }
      if ( EnableUse > 0 )
      {
        return;
      }
      EnableUse = 10;
      v293 = &DAT_00552460;
      v295 = -63;
      v305 = 6;
      v296[0] = 9729;
      v294 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      (BYTE)(v296[1]) = v287 + 12;
      v152 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do...
      if...
      if...
      v155 = v294;
      qmemcpy(v304, &v295, v294);
      v304[v155] = rand();
      v156 = (v304[0] != -63) + 2;
      if...
      v160 = g_byPacketSerialSend;
      v304[v156 - 1] = g_byPacketSerialSend;
      g_byPacketSerialSend = v160 + 1;
      if...
      v163 = v156 - 1;
      v164 = v155 - v163;
      v165 = &v304[v163];
      v166 = FUN_0053cc30(0, (int)&v304[v163], v164);
      if...
      CWsctlc::Close((DWORD)&SocketClient);
      goto LABEL_318;
    }
    if ( v98 >= 480 && v98 < 512 || v98 >= 391 && v98 <= 398 || v98 >= 400 && v98 <= 403 )
    {
      v178 = (void *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v179 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
      v283 = 0;
      v286 = 0;
      if...
      v181 = operator_new(0x585u);
      *(BYTE *)(v181 + 1412) = 1;
      FUN_00403f80(&DAT_055c9bc8, v181, (int)v178);
LABEL_335:
      if ( (unsigned int)*(WORD *)(CharacterAttribute + 14) < ItemAttribute[*v285].RequireLevel
        || *(WORD *)(CharacterAttribute + 26) < (unsigned short)v285[16]
        || *(WORD *)(CharacterAttribute + 20) < (unsigned short)v285[14] )
      {
        goto LABEL_418;
      }
      if ( WarehouseOpened )
      {
        goto LABEL_417;
      }
      if...
      v192 = TradeOpened;
      if...
      if ( v192 )
      {
LABEL_417:
        UIChatLogWindow_AddText(DAT_07eaa18c, GlobalText[474], 2);
        goto LABEL_418;
      }
      if ( EnableUse > 0 )
      {
        goto LABEL_418;
      }
      EnableUse = 10;
      v293 = &DAT_00552460;
      v295 = -63;
      v305 = 7;
      v296[0] = 9729;
      v294 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      (BYTE)(v296[1]) = v287 + 12;
      v196 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do...
      if...
      if...
      v199 = v294;
      qmemcpy(v304, &v295, v294);
      v304[v199] = rand();
      v200 = (v304[0] != -63) + 2;
      if...
      v205 = g_byPacketSerialSend;
      v304[v200 - 1] = g_byPacketSerialSend;
      g_byPacketSerialSend = v205 + 1;
      if...
      v209 = v200 - 1;
      v210 = v199 - v209;
      v211 = &v304[v209];
      v212 = FUN_0053cc30(0, (int)&v304[v209], v199 - v209);
      if...
LABEL_411:
      v224 = *(&OffsetInventoryItems.Type + 34 * v287);
      if ( v224 == 448 )
      {
        PlayBuffer(33, 0, 0);
      }
      else if ( v224 >= 449 && v224 <= 457 )
      {
        PlayBuffer(32, 0, 0);
      }
      v305 = -1;
LABEL_418:
      v286 = (char *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v225 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
      v226 = 0;
      v283 = 0;
      if...
      return;
    }
    if ( v98 == 431 )
    {
      v285 = (short *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v238 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
      v239 = 0;
      v283 = 0;
      if...
      v286 = (char *)operator_new(0x585u);
      v286[1412] = 1;
      v283 = (int)v285;
      v240 = (*(int (__cdecl **)(int *, short *))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, v285);
      v241 = 0;
      v284 = 0;
      if...
LABEL_466:
      v290 = *(unsigned short *)(CharacterAttribute + 14);
      v286 = (char *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v249 = (*(int (__cdecl **)(int *, DWORD))(DAT_055c9bc8 + 12))(&DAT_055c9bc8, CharacterMachine);
      v250 = 0;
      v283 = 0;
      if...
      if ( v290 < 10 )
      {
        CreateOkMessageBox(GlobalText[749]);
      }
      else
      {
        qmemcpy(&DAT_07ea5240, (const void *)(68 * v287 + 132809744), 0x44u);
        DAT_07ea5249 = v287;
        ShowCheckBox(1, 376, 153);
      }
      return;
    }
    DAT_07ea9800 = (int)a3;
    qmemcpy(&pPickedItem, &a3[34 * v99], 68u);
    DAT_083a42e0 = MouseX;
    DAT_083a42e4 = MouseY;
    if ( a3 == (short *)&OffsetInventoryItems )
    {
      if ( WarehouseOpened )
      {
        DAT_083a42eb = FUN_004d6020(DAT_07eaa0c8 + 15, DAT_07eaa0cc + 50, (int)&Inventory[32].WalkSpeed, 8, 15);
      }
      if ( DAT_083a42eb )
      {
        v262 = a4 * *((unsigned char *)v97 + 63) + *((unsigned char *)v97 + 62) + 12;
        goto LABEL_498;
      }
    }
    else
    {
      if ( WarehouseOpened )
      {
        DAT_083a42eb = FUN_004d6020(InventoryStartX + 15, InventoryStartY + 200, (int)&OffsetInventoryItems, 8, 8);
      }
      if ( DAT_083a42eb )
      {
        v262 = a4 * *((unsigned char *)v97 + 63) + *((unsigned char *)v97 + 62);
LABEL_498:
        *(DWORD *)&Inventory[32].Type = v262;
        UI_Main(v262, a3, a4);
        if ( DAT_083a42eb )
        {
          CheckInventory = 0;
          PlayBuffer(29, 0, 0);
          return;
        }
      }
    }
    memset(&pPickedItem, 0, 0x44u);
    return;
  }
  if ( Teleport )
  {
    return;
  }
  if ( WarehouseOpened )
  {
    goto LABEL_217;
  }
  if...
  v104 = TradeOpened;
  PACKET_ENCRYPT(&DAT_055c9bc8, &TradeOpened);
  if ( v104 )
  {
LABEL_217:
    UIChatLogWindow_AddText(DAT_07eaa184, GlobalText[474], 2);
  }
  else
  {
    if ( EnableUse > 0 )
    {
      return;
    }
    EnableUse = 10;
    v293 = &DAT_00552460;
    v295 = -63;
    v305 = 3;
    v296[0] = 9729;
    v294 = 3;
    v263 = -25;
    v264 = 109;
    v265 = 58;
    (BYTE)(v296[1]) = v287 + 12;
    v105 = 3;
    v266 = -119;
    v267 = -68;
    v268 = -78;
    v269 = -97;
    v270 = 115;
    v271 = 35;
    v272 = -88;
    v273 = -2;
    v274 = -74;
    v275 = 73;
    v276 = 93;
    v277 = 57;
    v278 = 93;
    v279 = -1922839670;
    v280 = 1596685802;
    v281 = -2081836605;
    v282 = 1458065705;
    do...
    if...
    if...
    v108 = v294;
    qmemcpy(v304, &v295, v294);
    v304[v108] = rand();
    v109 = (v304[0] != -63) + 2;
    if...
    v113 = g_byPacketSerialSend;
    v304[v109 - 1] = g_byPacketSerialSend;
    g_byPacketSerialSend = v113 + 1;
    PACKET_ENCRYPT(&DAT_055c9bc8, &g_byPacketSerialSend);
    v114 = v109 - 1;
    v115 = v108 - v114;
    v116 = &v304[v114];
    v117 = FUN_0053cc30(0, (int)&v304[v114], v115);
    if...
LABEL_215:
    v129 = *(&OffsetInventoryItems.Type + 34 * v287);
    if ( v129 == 448 )
    {
LABEL_216:
      PlayBuffer(33, 0, 0);
      return;
    }
LABEL_319:
    if ( v129 >= 449 && v129 <= 457 )
    {
      PlayBuffer(32, 0, 0);
    }
  }
}
#endif

// ── FUN_004e13a0 (IDA-only, gated) ──
#if defined(IDA_PORT_004E13A0)
void __cdecl FUN_004e13a0(int Type, int ItemLevel, int Option1, float Target[3], int Select, bool PickUp)
{
  int Level; // edi
  short v9; // ax
  DWORD v10; // ecx
  int v11; // [esp+30h] [ebp-3BCh]
  float Position[3]; // [esp+34h] [ebp-3B8h] BYREF
  float Direction[3]; // [esp+40h] [ebp-3ACh] BYREF
  float Light[3]; // [esp+4Ch] [ebp-3A0h] BYREF
  DWORD o; // [esp+58h] [ebp-394h] BYREF
  int v16; // [esp+64h] [ebp-388h]
  float v17; // [esp+68h] [ebp-384h]
  float v18; // [esp+6Ch] [ebp-380h]
  float v19; // [esp+70h] [ebp-37Ch]
  char v20; // [esp+134h] [ebp-2B8h]
  float v21; // [esp+1C0h] [ebp-22Ch]
  char v22; // [esp+214h] [ebp-1D8h]

  Direction[0] = *Target - MousePosition[0];
  Level = (ItemLevel >> 3) & 0xF;
  Direction[1] = Target[1] - MousePosition[1];
  Direction[2] = Target[2] - MousePosition[2];
  if ( PickUp )
  {
    VectorMA(MousePosition, 0.07, Direction, Position);
  }
  else
  {
    VectorMA(MousePosition, 0.1, Direction, Position);
  }
  if ( Type == 535 || Type == 543 )
  {
    Angle[0] = 0.0;
    DAT_07ea9530 = 1132920832;
    goto LABEL_50;
  }
  if ( Type == 545 )
  {
    Angle[0] = 0.0;
    DAT_07ea9530 = 1119092736;
LABEL_50:
    DAT_07ea9534 = 1097859072;
    goto LABEL_51;
  }
  if ( Type >= 536 && Type < 560 )
  {
    Angle[0] = 90.0;
    DAT_07ea9530 = 1127481344;
    DAT_07ea9534 = 1101004800;
    goto LABEL_51;
  }
  if ( Type == 506 )
  {
    Angle[0] = 180.0;
    DAT_07ea9530 = 1132920832;
    DAT_07ea9534 = 1101004800;
    goto LABEL_51;
  }
  if ( Type >= 400 )
  {
    if ( Type < 592 )
    {
      Angle[0] = 180.0;
      DAT_07ea9530 = 1132920832;
      if ( *((BYTE *)&ItemAttribute[Type - 399] - 34) )
      {
        DAT_07ea9534 = 1103626240;
        goto LABEL_51;
      }
      goto LABEL_50;
    }
    if ( Type < 624 )
    {
      Angle[0] = 270.0;
      DAT_07ea9530 = 1132920832;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    }
  }
  switch ( Type )
  {
    case 819:
      Angle[0] = -90.0;
      DAT_07ea9530 = -1028390912;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    case 832:
    case 833:
      Angle[0] = 270.0;
      goto LABEL_48;
    case 834:
      Angle[0] = 290.0;
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    case 958:
      Angle[0] = -90.0;
      DAT_07ea9530 = -1046478848;
      DAT_07ea9534 = -1046478848;
      goto LABEL_51;
  }
  if ( Type >= 828 && Type < 848 && Type != 830 && Type != 831 )
  {
    Angle[0] = 360.0;
    DAT_07ea9530 = 0;
    DAT_07ea9534 = 0;
    goto LABEL_51;
  }
  if ( Type != 860 )
  {
    switch ( Type )
    {
      case 952:
        Angle[0] = 270.0;
        DAT_07ea9530 = 0;
        DAT_07ea9534 = 0;
        goto LABEL_51;
      case 953:
        goto LABEL_42;
      case 954:
        Angle[0] = 270.0;
        DAT_07ea9530 = 0;
        DAT_07ea9534 = 0;
        goto LABEL_51;
    }
    Angle[0] = 270.0;
    if ( Type == 868 )
    {
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    }
LABEL_48:
    DAT_07ea9530 = -1054867456;
    DAT_07ea9534 = 0;
    goto LABEL_51;
  }
  switch ( Level )
  {
    case 0:
      Angle[0] = 180.0;
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      break;
    case 1:
LABEL_42:
      Angle[0] = 270.0;
      DAT_07ea9530 = 1119092736;
      DAT_07ea9534 = 0;
      break;
    case 2:
      Angle[0] = 90.0;
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      break;
  }
LABEL_51:
  if ( Select == 1 )
  {
    *(float *)&DAT_07ea9530 = WorldTime * 0.44999999;
  }
  v9 = Type;
  DAT_07ea9512 = Type;
  if ( (short)Type < 624 || (short)Type >= 784 )
  {
    if ( (WORD)Type == 860 )
    {
      if ( Level )
      {
        if ( Level == 2 )
        {
          v9 = 948;
          Type = 948;
          DAT_07ea9512 = 948;
        }
      }
      else
      {
        v9 = 947;
        Type = 947;
        DAT_07ea9512 = 947;
      }
    }
  }
  else
  {
    v9 = 390;
    DAT_07ea9512 = 390;
  }
  DAT_07ea9618 = 0;
  DAT_07ea961c = 0;
  DAT_07ea9616 = 0;
  v10 = Models + 188 * v9;
  *(BYTE *)(v10 + 160) = 0;
  if ( Type >= 624 )
  {
    if ( Type < 656 )
    {
      *(DWORD *)(v10 + 132) = -1021313024;
      goto LABEL_74;
    }
    if ( Type < 688 )
    {
      *(DWORD *)(v10 + 132) = -1027080192;
      goto LABEL_74;
    }
  }
  if ( Type < 720 || Type >= 752 )
  {
    if ( Type < 688 || Type >= 720 )
    {
      *(DWORD *)(v10 + 132) = 0;
    }
    else
    {
      *(DWORD *)(v10 + 132) = -1035468800;
    }
  }
  else
  {
    *(DWORD *)(v10 + 132) = -1031012352;
  }
LABEL_74:
  if ( Type >= 624 && Type < 784 )
  {
    if ( Type >= 656 )
    {
      if ( Type >= 688 )
      {
        if ( Type < 720 || Type >= 752 )
        {
          if ( Type >= 720 )
          {
            v11 = 995211031;
          }
          else
          {
            v11 = 995640528;
          }
        }
        else
        {
          v11 = 997788012;
        }
      }
      else
      {
        v11 = 998217508;
      }
    }
    else
    {
      v11 = 998217508;
    }
    goto LABEL_142;
  }
  if ( Type == 790 )
  {
    v11 = 985963430;
    goto LABEL_142;
  }
  if ( Type >= 784 && Type < 816 )
  {
    v11 = 990057071;
    goto LABEL_142;
  }
  switch ( Type )
  {
    case 869:
      v11 = 990057071;
      goto LABEL_142;
    case 958:
      v11 = 985963430;
      goto LABEL_142;
    case 832:
      v11 = 990057071;
      goto LABEL_142;
    case 833:
      v11 = 988540410;
      goto LABEL_142;
    case 834:
      v11 = 988540410;
      goto LABEL_142;
    case 419:
      if ( ItemLevel >= 0 )
      {
        v11 = 992204554;
        goto LABEL_142;
      }
LABEL_104:
      v11 = 981668463;
      ItemLevel = 0;
      goto LABEL_142;
    case 570:
      if ( ItemLevel >= 0 )
      {
        v11 = 989399404;
        goto LABEL_142;
      }
      goto LABEL_104;
    case 546:
      if ( ItemLevel < 0 )
      {
        v11 = 985963430;
        ItemLevel = 0;
        goto LABEL_142;
      }
      break;
    default:
      if ( Type >= 870 )
      {
        if ( Type < 873 )
        {
          v11 = 992204554;
          goto LABEL_142;
        }
        if ( Type < 875 )
        {
          v11 = 993493044;
          goto LABEL_142;
        }
      }
      if ( Type == 830 || Type == 831 )
      {
        v11 = 994352038;
        goto LABEL_142;
      }
      if ( Type >= 848 && Type < 880 )
      {
        v11 = 996499522;
        goto LABEL_142;
      }
      if ( Type >= 496 && Type < 528 )
      {
        v11 = 988540410;
        goto LABEL_142;
      }
      if ( Type >= 560 && Type < 592 )
      {
        v11 = 990916064;
        goto LABEL_142;
      }
      switch ( Type )
      {
        case 543:
          v11 = 982527456;
          goto LABEL_142;
        case 535:
          v11 = 983386450;
          goto LABEL_142;
        case 953:
          v11 = 998217508;
          goto LABEL_142;
        case 955:
          v11 = 985963430;
          goto LABEL_142;
        case 956:
          v11 = 989399404;
          goto LABEL_142;
        case 957:
          v11 = 981668463;
          goto LABEL_142;
      }
      break;
  }
  v11 = 0x3B23D70A;
LABEL_142:
  BMD_Animation(v10, (float (*)[3][4])BoneMatrix, 0.0, 0.0, 0, Angle, DAT_07ea9538, 0, 0);
  (WORD)((o) >> 16) = Type;
  ItemObjectAttribute((DWORD)&o);
  v17 = Position[0];
  v16 = v11;
  v18 = Position[1];
  v19 = Position[2];
  v20 = 0;
  v22 = 2;
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  RenderPartObject((DWORD)&o, Type, 0, Light, v21, ItemLevel, Option1, 1, 1, 1, 0, 2);
}
#endif

// ── FUN_004f6850 (IDA-only, gated) ──
#if defined(IDA_PORT_004F6850)
char FUN_004f6850()
{
  char v0; // dl
  ITEM *v1; // eax
  int v2; // ecx
  int v3; // edi
  signed int v4; // ebp
  int v5; // eax
  int v6; // esi
  char buf[4]; // [esp+Ch] [ebp-410h] BYREF
  char v9; // [esp+10h] [ebp-40Ch]
  int v10; // [esp+418h] [ebp-4h]

  v0 = 1;
  v1 = &OffsetMixItems;
  do
  {
    v2 = 8;
    do
    {
      if ( v1->Type != -1 && (int)v1->Key > 0 )
      {
        v0 = 0;
      }
      ++v1;
      --v2;
    }
    while ( v2 );
  }
  while ( (int)v1 < (int)&DAT_07eaa0c8 );
  if ( v0 && (v3 = 0, DAT_07e91388 <= 0) )
  {
    buf[2] = -63;
    v10 = 0;
    v9 = -121;
    buf[3] = 3;
    v4 = 3;
    if ( s != -1 )
    {
      while ( 1 )
      {
        v5 = send(s, &buf[v3 + 2], 3 - v3, 0);
        v6 = v5;
        if ( v5 == -1 )
        {
          break;
        }
        if ( v5 )
        {
          if ( DAT_055ce174 )
          {
            nullsub_2((int)&buf[2], v5);
          }
          v4 -= v6;
          v3 += v6;
          if ( v4 > 0 )
          {
            continue;
          }
        }
        return 1;
      }
      if ( WSAGetLastError() != 10035 || DAT_055cc16c + 3 > 0x2000 )
      {
        CWsctlc::Close((DWORD)&SocketClient);
        return 1;
      }
      qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf[2], v4);
      DAT_055cc16c += v4;
    }
    return 1;
  }
  else
  {
    UIChatLogWindow_AddText(&strID, GlobalText[593], 2);
    return 0;
  }
}
#endif

// ── FUN_004f6a70 (IDA-only, gated) ──
#if defined(IDA_PORT_004F6A70)
char __cdecl FUN_004f6a70(int a1, int a2)
{
  int v3; // edi
  signed int v4; // ebx
  int v5; // eax
  int v6; // esi
  char buf[4]; // [esp+4h] [ebp-410h] BYREF
  char v8; // [esp+8h] [ebp-40Ch]
  int v9; // [esp+410h] [ebp-4h]

  if ( EquipmentItem )
  {
    return 0;
  }
  InventoryOpened = 0;
  CloseInventoryRelatedWindows();
  v3 = 0;
  if ( DAT_07e91388 > 0 )
  {
    FUN_004cd3b0(a1, a2);
  }
  buf[2] = -63;
  v9 = 0;
  v8 = -126;
  buf[3] = 3;
  v4 = 3;
  if ( s != -1 )
  {
    while ( 1 )
    {
      v5 = send(s, &buf[v3 + 2], 3 - v3, 0);
      v6 = v5;
      if ( v5 == -1 )
      {
        break;
      }
      if ( v5 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2((int)&buf[2], v5);
        }
        v4 -= v6;
        v3 += v6;
        if ( v4 > 0 )
        {
          continue;
        }
      }
      return 1;
    }
    if ( WSAGetLastError() == 10035 && DAT_055cc16c + 3 <= 0x2000 )
    {
      qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf[2], v4);
      DAT_055cc16c += v4;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
  return 1;
}
#endif

// ── FUN_004f6cb0 (IDA-only, gated) ──
#if defined(IDA_PORT_004F6CB0)
void __cdecl FUN_004f6cb0()
{
  MessageBoxA(g_hWnd, GlobalText[11], 0, 0);
  SendMessageA(g_hWnd, 2u, 0, 0);
}
#endif

// ── FUN_004f6ce0 (IDA-only, gated) ──
#if defined(IDA_PORT_004F6CE0)
int __cdecl FUN_004f6ce0(char *FileName)
{
  FILE *fp; // eax MAPDST
  int result; // eax
  int iSize; // esi
  bool Error; // al
  int i; // ecx
  CHAR Text[256]; // [esp+8h] [ebp-10104h] BYREF
  unsigned char *byBuffer; // [esp+108h] [ebp-10004h] BYREF

  fp = fopen(FileName, "rb");
  if ( fp )
  {
    fseek(fp, 0, 2);
    iSize = ftell(fp);
    fseek(fp, 0, 0);
    if ( iSize == 65539 )
    {
      fread(&byBuffer, 65539u, 1u, fp);
      BuxConvert((BYTE *)&byBuffer, 65539);
      qmemcpy(TerrainWall, (char *)&byBuffer + 3, sizeof(TerrainWall));
      Error = 0;
      if ( (BYTE)byBuffer || *(WORD *)((char *)&byBuffer + 1) != 0xFFFF )
      {
        Error = 1;
      }
      if ( !DAT_083a410c )
      {
        switch ( World )
        {
          case 0:
            if ( TerrainWall[31623] != 5 )
            {
              goto LABEL_19;
            }
            break;
          case 1:
            if ( TerrainWall[30947] != 4 )
            {
              goto LABEL_19;
            }
            break;
          case 2:
            if ( TerrainWall[14288] != 5 )
            {
              goto LABEL_19;
            }
            break;
          case 3:
            if ( TerrainWall[30650] != 5 )
            {
              goto LABEL_19;
            }
            break;
          case 4:
            if ( TerrainWall[19393] != 5 )
            {
LABEL_19:
              Error = 1;
            }
            break;
          default:
            break;
        }
      }
      for ( i = 0; i < 65536; ++i )
      {
        if ( TerrainWall[i] >= 0x80u )
        {
          Error = 1;
        }
      }
      if ( Error )
      {
        ExitProgram();
        (BYTE)(result) = 0;
      }
      else
      {
        result = fclose(fp);
        (BYTE)(result) = 1;
      }
    }
    else
    {
      ExitProgram();
      (BYTE)(result) = 0;
    }
  }
  else
  {
    sprintf(Text, "%s file not found.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    CErrorReport::Write((DWORD)&g_ErrorReport, "\r\n");
    MessageBoxA(g_hWnd, Text, 0, 0);
    result = SendMessageA(g_hWnd, 2u, 0, 0);
    (BYTE)(result) = 0;
  }
  return result;
}
#endif

// ── FUN_004f6eb0 (IDA-only, gated) ──
#if defined(IDA_PORT_004F6EB0)
void __cdecl FUN_004f6eb0(BYTE *Buffer, int Size)
{
  int i; // ecx

  for ( i = 0; i < Size; ++i )
  {
    Buffer[i] ^= DAT_0055a770[i % 3];
  }
}
#endif

// ── FUN_004f7250 (IDA-only, gated) ──
#if defined(IDA_PORT_004F7250)
void __cdecl FUN_004f7250(char *FileName)
{
  OpenJpegBuffer(FileName, &TerrainLight);
  CreateTerrainNormal();
  CreateTerrainLight();
}
#endif

// ── FUN_004f98c0 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_004f98c0(int a1, int a2, int a3, int a4, int a5)
{
  int v5; // esi
  int v6; // edi
  int v7; // ebx
  int v8; // ecx
  int result; // eax
  int v10; // ebp
  int v11; // edx
  int v12; // ebx
  int v13; // esi
  int v14; // ebp
  __int64 v15; // rax
  char *v16; // [esp+0h] [ebp-10h]
  float v17; // [esp+8h] [ebp-8h]
  int v18; // [esp+Ch] [ebp-4h]
  int v19; // [esp+14h] [ebp+4h]
  int v20; // [esp+18h] [ebp+8h]
  int v21; // [esp+24h] [ebp+14h]

  v5 = a3;
  v6 = a1;
  v16 = (char *)&DAT_0814b2e0 + 0x40000 * a5;
  v17 = 1024.0 / (double)a3 * (1024.0 / (double)a3);
  if ( a1 < 0 )
  {
    v6 = rand() % (255 - 2 * a3) + a3 + 1;
  }
  v7 = a2;
  if ( a2 < 0 )
  {
    v7 = rand() % (255 - 2 * a3) + a3 + 1;
  }
  v8 = a3 * a3;
  v18 = a3 * a3;
  result = -a3;
  v10 = -a3;
  v21 = -a3;
  v19 = a3;
  if ( v6 - a3 < 1 )
  {
    v21 = 1 - v6;
    v10 = 1 - v6;
  }
  if ( v7 - a3 < 1 )
  {
    result = 1 - v7;
  }
  if ( v6 + a3 > 255 )
  {
    a3 = 255 - v6;
  }
  if ( v7 + v5 > 255 )
  {
    v19 = 255 - v7;
  }
  v11 = result;
  v20 = result;
  if ( result < v19 )
  {
    v12 = v6 + ((result + v7) << 8);
    do
    {
      v13 = v10;
      if ( v10 < a3 )
      {
        v14 = v11 * v11;
        do
        {
          if ( v14 + v13 * v13 < v8 )
          {
            v15 = (__int64)((cos(sqrt((double)(v14 + v13 * v13) * v17)) + 65535.0) * (double)a4);
            v11 = v20;
            *(DWORD *)&v16[4 * v12 + 4 * v13] += (int)v15 >> 19;
            v8 = v18;
          }
          ++v13;
        }
        while ( v13 < a3 );
        v10 = v21;
      }
      result = v19;
      ++v11;
      v12 += 256;
      v20 = v11;
    }
    while ( v11 < v19 );
  }
  return result;
}

// ── FUN_004f9c20 (IDA-only, gated) ──
#if defined(IDA_PORT_004F9C20)
void __cdecl FUN_004f9c20(bool EditFlag)
{
  double v1; // st7
  double v2; // st7

  FUN_004f98c0(
    (__int64)(*(float *)(Hero + 16) * 0.039999999),
    (__int64)(*(float *)(Hero + 20) * 0.039999999),
    3,
    -70,
    DAT_0839bc88);
  v1 = WorldTime;
  if ( World == 8 )
  {
    v2 = (double)((int)(__int64)v1 % 40000) * 0.000024999999;
  }
  else
  {
    v2 = (double)((int)(__int64)v1 % 20000) * 0.000049999999;
  }
  WaterMove = v2;
  if ( EditFlag )
  {
    SelectFlag = 0;
    FUN_00512d30();
  }
  else
  {
    DisableAlphaBlend();
  }
  TerrainFlag = 0;
  RenderTerrainFrustrum(EditFlag);
  if ( EditFlag )
  {
    if ( SelectFlag )
    {
      RenderTerrainTile(SelectXF, SelectYF, (__int64)SelectXF, (__int64)SelectYF, 1.0, 1, EditFlag);
    }
  }
  else
  {
    EnableAlphaTest(1);
    if ( DAT_0055a76c && World != 7 )
    {
      TerrainFlag = 2;
      RenderTerrainFrustrum(0);
    }
    FUN_004f7060();
    DisableDepthTest();
    EnableCullFace();
    FUN_00479540();
    EnableDepthTest();
  }
  DAT_0839bc88 ^= 1u;
  FUN_004f9a30(DAT_0839bc88);
}
#endif

// ── FUN_004f9cb0 (IDA-only, gated) ──
#if defined(IDA_PORT_004F9CB0)
float *__cdecl FUN_004f9cb0(float *a1, float *a2, float *a3)
{
  float *result; // eax

  result = a1;
  *a3 = *a1 * *a2;
  a3[1] = a1[1] * a2[1];
  a3[2] = a1[2] * a2[2];
  return result;
}
#endif

// ── FUN_004f9ce0 (IDA-only, gated) ──
#if defined(IDA_PORT_004F9CE0)
void __cdecl FUN_004f9ce0(float va[3], float scale, float vb[3], float vc[3])
{
  *vc = scale * *vb + *va;
  vc[1] = scale * vb[1] + va[1];
  vc[2] = scale * vb[2] + va[2];
}
#endif

// ── FUN_004f9d20 (IDA-only, gated) ──
#if defined(IDA_PORT_004F9D20)
float *__cdecl FUN_004f9d20(float *a1, float *a2, float *a3)
{
  float *result; // eax

  result = a1;
  *a3 = a2[2] * a1[1] - a1[2] * a2[1];
  a3[1] = a1[2] * *a2 - *a1 * a2[2];
  a3[2] = *a1 * a2[1] - *a2 * a1[1];
  return result;
}
#endif

// ── FUN_004fa5a0 (IDA-only, gated) ──
#if defined(IDA_PORT_004FA5A0)
int FUN_004fa5a0()
{
  int result; // eax

  result = -1;
  DAT_0055a7bc = -1.0;
  DAT_0055a7b0 = -1;
  DAT_0055a7b4 = -1;
  DAT_0055a7b8 = -1;
  return result;
}
#endif

// ── FUN_004fdc00 (IDA-only, gated) ──
#if defined(IDA_PORT_004FDC00)
// Macros IDA locales para este port. #undef al final del bloque.
#define LODWORD(x)  (*(unsigned int*)&(x))
#define Models      DAT_05828d58
#define EditFlag    DAT_07e11d30
extern void __cdecl FUN_0046ca00(DWORD Object);   // World-4 gate FX (stubs_game.cpp)
extern "C" void DbgLogPublic(const char*);        // [DIAG activación temporal]
void __cdecl FUN_004fdc00(float o)
{
  double v1; // st7
  short v3; // ax
  short v4; // ax
  int v5; // ecx
  float *v6; // ebp
  float *v7; // edi
  double v8; // st7
  double v9; // st7
  double v10; // st7
  long double v11; // st7
  long double v12; // st7
  short v13; // cx
  double v14; // st6
  double v15; // st4
  long double v16; // st7
  int v17; // eax
  bool v18; // zf
  signed int v19; // eax
  int v20; // eax
  double v21; // st7
  long double v22; // st7
  long double v23; // st7
  double v24; // st7
  double v25; // st7
  long double v26; // st7
  int v27; // eax
  float xf; // [esp+0h] [ebp-40h]
  float yf; // [esp+4h] [ebp-3Ch]
  float v30[3]; // [esp+28h] [ebp-18h] BYREF
  float Light[3]; // [esp+34h] [ebp-Ch] BYREF
  float oa; // [esp+44h] [ebp+4h]
  float ob; // [esp+44h] [ebp+4h]

  if ( World == 9 )
  {
    if ( (__int64)WorldTime % 4000 < 1000 && !(rand() % 100) )
    {
      v1 = (double)(rand() % 12 + 4) * 0.1;
      Light[0] = v1 * 0.2;
      Light[1] = v1 * 0.30000001;
      Light[2] = v1 * 0.5;
      yf = (double)(rand() % 1200) + *(float *)(Hero + 20) - 600.0;
      xf = (double)(rand() % 1200) + *(float *)(Hero + 16) - 600.0;
      AddTerrainLight(xf, yf, Light, 12, PrimaryTerrainLight[0]);
    }
    PlayBuffer(1, 0, 1);
  }
  if ( !World )
  {
    v3 = *(WORD *)(LODWORD(o) + 2);
    if ( v3 != 125 && v3 != 126 )
    {
      goto LABEL_22;
    }
    if ( HeroTile == 4 )
    {
      *(DWORD *)(LODWORD(o) + 356) = 0;
    }
    else
    {
      *(DWORD *)(LODWORD(o) + 356) = 1065353216;
    }
  }
  if ( World == 2 )
  {
    v4 = *(WORD *)(LODWORD(o) + 2);
    if ( v4 == 81 || v4 == 82 || v4 == 96 || v4 == 98 || v4 == 99 )
    {
      if ( HeroTile == 3 || HeroTile >= 10 )
      {
        *(DWORD *)(LODWORD(o) + 356) = 0;
      }
      else
      {
        *(DWORD *)(LODWORD(o) + 356) = 1065353216;
      }
    }
  }
LABEL_22:
  Alpha(LODWORD(o));
  if ( *(float *)(LODWORD(o) + 360) < 0.0099999998 )
  {
    return;
  }
  v5 = Models + 188 * *(short *)(LODWORD(o) + 2);
  *(BYTE *)(v5 + 160) = *(BYTE *)(LODWORD(o) + 261);
  oa = *(float *)(LODWORD(o) + 204);
  if ( World == 8 && *(WORD *)(LODWORD(o) + 2) == 8 )
  {
    oa = oa * 4.0;
  }
  v6 = (float *)(LODWORD(o) + 28);
  v7 = (float *)(LODWORD(o) + 16);
  // NOTA: nuestro FUN_00440aa0 (BMD_Anim.cpp) es la variante de 5 args (avanza
  // el frame). El IDA sub_440AA0 toma 7 (los 2 últimos = pos/vel para root-motion
  // de la animación). Los omitimos: el avance de frame —lo que faltaba— funciona.
  FUN_00440aa0((void*)v5, (float*)(LODWORD(o) + 264), (float*)(LODWORD(o) + 268),
               (void*)(LODWORD(o) + 262), oa);
  if ( g_GameState == 2 || g_GameState == 4 )
  {
    if ( *(WORD *)(LODWORD(o) + 2) == 160 )
    {
      v9 = (double)((__int64)WorldTime % 4000);
    }
    else
    {
      if ( *(WORD *)(LODWORD(o) + 2) != 161 )
      {
        if ( *(WORD *)(LODWORD(o) + 2) == 162 )
        {
          if ( CameraWalkCut )
          {
            v8 = 1.5;
          }
          else
          {
            v8 = (double)CurrentCameraCount * 0.0020000001;
          }
          *(float *)(LODWORD(o) + 232) = v8;
          *(float *)(LODWORD(o) + 236) = v8;
          *(float *)(LODWORD(o) + 240) = v8;
          *(float *)(LODWORD(o) + 104) = v8;
        }
        goto LABEL_38;
      }
      v9 = (double)((__int64)WorldTime % 4000);
    }
    *(float *)(LODWORD(o) + 112) = -(v9 * 0.00025000001);
  }
LABEL_38:
  switch ( World )
  {
    case 0:
      switch ( *(WORD *)(LODWORD(o) + 2) )
      {
        case 0x32:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, 0.0, 200.0);
          break;
        case 0x33:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, -30.0, 60.0);
          break;
        case 0x34:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, 0.0, 60.0);
          *(float *)(LODWORD(o) + 104) = (double)(rand() % 6 + 4) * 0.1;
          break;
        case 0x37:
          FUN_0046c7f0(0, (int)LODWORD(o), -150.0, -150.0, 140.0);
          FUN_0046c7f0(0, (int)LODWORD(o), 150.0, -150.0, 140.0);
          break;
        case 0x50:
          FUN_0046c7f0(0, (int)LODWORD(o), 90.0, -200.0, 30.0);
          FUN_0046c7f0(0, (int)LODWORD(o), 90.0, 200.0, 30.0);
          break;
        case 0x5A:
          v10 = (double)(rand() % 2 + 6) * 0.1;
          v30[0] = v10;
          v30[1] = v10 * 0.80000001;
          v11 = v10 * 0.60000002;
          goto LABEL_116;
        case 0x75:
        case 0x7A:
          goto LABEL_59;
        case 0x76:
        case 0x77:
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 1000) * 0.001;
          break;
        case 0x82:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, 0.0, 0.0);
          *(DWORD *)(LODWORD(o) + 88) = -2;
          break;
        case 0x83:
          FUN_0046c7f0(1, (int)LODWORD(o), 0.0, 0.0, 0.0);
          *(DWORD *)(LODWORD(o) + 88) = -2;
          break;
        case 0x84:
          FUN_0046c7f0(2, (int)LODWORD(o), 0.0, 0.0, 0.0);
          *(DWORD *)(LODWORD(o) + 88) = -2;
          break;
        case 0x96:
          v12 = (double)(rand() % 4 + 3) * 0.1;
          goto LABEL_51;
        default:
          return;
      }
      break;
    case 1:
      switch ( *(WORD *)(LODWORD(o) + 2) )
      {
        case 0x16:
        case 0x17:
        case 0x18:
          *(BYTE *)(Models + 188 * *(short *)(LODWORD(o) + 2) + 136) = 1;
          *(float *)(LODWORD(o) + 112) = (double)((__int64)WorldTime % 1000) * -0.001;
          break;
        case 0x27:
        case 0x28:
        case 0x33:
          goto LABEL_132;
        case 0x29:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, -30.0, 240.0);
          break;
        case 0x2A:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, 0.0, 190.0);
          break;
        case 0x34:
          if ( rand() % 3 )
          {
            goto LABEL_132;
          }
          CreateEffect(
            215,
            (float *)(LODWORD(o) + 16),
            (float *)(LODWORD(o) + 28),
            (float *)(LODWORD(o) + 232),
            0,
            0,
            (float *)-1,
            0,
            0);
          *(DWORD *)(LODWORD(o) + 88) = -2;
          break;
        default:
          return;
      }
      break;
    case 2:
      v13 = *(WORD *)(LODWORD(o) + 2);
      switch ( v13 )
      {
        case 20:
        case 65:
        case 86:
        case 88:
          if ( !EditFlag )
          {
            v14 = *(float *)(Hero + 20) - *(float *)(LODWORD(o) + 56);
            v15 = *(float *)(Hero + 16) - *(float *)(LODWORD(o) + 52);
            v16 = sqrt(v14 * v14 + v15 * v15);
            ob = v16;
            if ( v16 >= 200.0 )
            {
              *(float *)(LODWORD(o) + 36) = FUN_0043e1b0(*(float *)(LODWORD(o) + 36), *(float *)(LODWORD(o) + 48), 10.0);
              *v7 = (*(float *)(LODWORD(o) + 52) - *v7) * 0.2 + *v7;
              *(float *)(LODWORD(o) + 20) = (*(float *)(LODWORD(o) + 56) - *(float *)(LODWORD(o) + 20)) * 0.2
                                          + *(float *)(LODWORD(o) + 20);
            }
            else if ( v13 == 86 )
            {
              if ( *(DWORD *)(LODWORD(o) + 36) == 1119092736 )
              {
                *(float *)(LODWORD(o) + 20) = 200.0 - ob + 200.0 - ob + *(float *)(LODWORD(o) + 56);
              }
              if ( *(DWORD *)(LODWORD(o) + 36) == 1132920832 )
              {
                *(float *)(LODWORD(o) + 20) = *(float *)(LODWORD(o) + 56) - (200.0 - ob + 200.0 - ob);
              }
              if ( *(float *)(LODWORD(o) + 36) == 0.0 )
              {
                *v7 = 200.0 - ob + 200.0 - ob + *(float *)(LODWORD(o) + 52);
              }
              if ( *(DWORD *)(LODWORD(o) + 36) == 1127481344 )
              {
                *v7 = *(float *)(LODWORD(o) + 52) - (200.0 - ob + 200.0 - ob);
              }
              PlayBuffer(18, 0, 0);
            }
            else
            {
              if ( *(DWORD *)(LODWORD(o) + 48) == 1119092736 )
              {
                *(float *)(LODWORD(o) + 36) = 30.0 - (200.0 - ob) * 0.5;
              }
              if ( *(DWORD *)(LODWORD(o) + 48) == 1132920832 )
              {
                *(float *)(LODWORD(o) + 36) = (200.0 - ob) * 0.5 + 330.0;
              }
              if ( *(float *)(LODWORD(o) + 48) == 0.0 )
              {
                *(float *)(LODWORD(o) + 36) = 300.0 - (200.0 - ob) * 0.5;
              }
              if ( *(DWORD *)(LODWORD(o) + 48) == 1127481344 )
              {
                *(float *)(LODWORD(o) + 36) = (200.0 - ob) * 0.5 + 240.0;
              }
              PlayBuffer(17, 0, 0);
            }
          }
          break;
        case 30:
        case 66:
          FUN_0046c7f0(0, (int)LODWORD(o), 0.0, 0.0, 50.0);
          break;
        case 78:
LABEL_59:
          *(float *)(LODWORD(o) + 104) = (double)(rand() % 4 + 4) * 0.1;
          break;
        default:
          return;
      }
      break;
    case 3:
      v17 = *(short *)(LODWORD(o) + 2);
      switch ( *(WORD *)(LODWORD(o) + 2) )
      {
        case 0x12:
          *(float *)(LODWORD(o) + 112) = (double)((__int64)WorldTime % 1000) * 0.001;
          break;
        case 0x27:
          goto LABEL_92;
        case 0x29:
          *(DWORD *)(LODWORD(o) + 100) = 0;
          *(float *)(LODWORD(o) + 112) = (double)((__int64)WorldTime % 2000) * 0.00050000002;
          break;
        case 0x2A:
          *(BYTE *)(Models + 188 * v17 + 136) = 0;
          *(float *)(LODWORD(o) + 108) = (double)((__int64)WorldTime % 500) * -0.0020000001;
          break;
        case 0x2B:
          *(BYTE *)(Models + 188 * v17 + 136) = 0;
          *(float *)(LODWORD(o) + 108) = (double)((__int64)WorldTime % 500) * 0.0020000001;
          break;
        default:
          return;
      }
      break;
    case 4:
      switch ( *(WORD *)(LODWORD(o) + 2) )
      {
        case 3:
        case 4:
          *(float *)(LODWORD(o) + 108) = (double)(-(__int64)WorldTime % 1000) * 0.001;
          break;
        case 0x12:
        case 0x17:
LABEL_92:
          *(DWORD *)(LODWORD(o) + 100) = 1;
          break;
        case 0x13:
        case 0x14:
          *(DWORD *)(LODWORD(o) + 100) = 4;
          *(float *)(LODWORD(o) + 108) = (double)(-(__int64)WorldTime % 1000) * 0.001;
          break;
        case 0x18:
          *(DWORD *)(LODWORD(o) + 88) = -2;
          v19 = rand() & 0x8000003F;
          v18 = v19 == 0;
          if ( v19 < 0 )
          {
            v18 = (((BYTE)v19 - 1) | 0xFFFFFFC0) == -1;
          }
          if ( v18 )
          {
            CreateEffect(1200, v7, v6, (float *)(LODWORD(o) + 232), 0, 0, (float *)-1, 0, 0);
          }
          break;
        case 0x19:
          goto LABEL_132;
        case 0x26:
        case 0x27:
          FUN_0046ca00(LODWORD(o));
          break;
        default:
          return;
      }
      break;
    case 5:
      if ( *(WORD *)(LODWORD(o) + 2) == 2 )
      {
LABEL_111:
        *(DWORD *)(LODWORD(o) + 100) = 0;
      }
      else if ( *(WORD *)(LODWORD(o) + 2) == 3 )
      {
        *(DWORD *)(LODWORD(o) + 100) = 0;
        *(float *)(LODWORD(o) + 104) = (double)(rand() % 4 + 6) * 0.1;
      }
      break;
    case 6:
      v20 = *(short *)(LODWORD(o) + 2);
      if ( v20 == 21 )
      {
        *(DWORD *)(LODWORD(o) + 100) = 3;
        *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 1000) * 0.001;
      }
      else if ( v20 == 38 )
      {
        *(DWORD *)(LODWORD(o) + 88) = -2;
      }
      break;
    case 7:
      switch ( *(WORD *)(LODWORD(o) + 2) )
      {
        case 0x16:
          v21 = *(float *)(LODWORD(o) + 128) + 0.1;
          *(DWORD *)(LODWORD(o) + 88) = -2;
          *(float *)(LODWORD(o) + 128) = v21;
          if ( v21 > 10.0 )
          {
            *(DWORD *)(LODWORD(o) + 128) = 0;
          }
          if ( *(float *)(LODWORD(o) + 128) > 5.0 )
          {
            Particle_Spawn(1241, v7, v6, (float *)(LODWORD(o) + 232), 0, 1.0, 0);
          }
          break;
        case 0x17:
          *(DWORD *)(LODWORD(o) + 100) = 0;
          *(float *)(LODWORD(o) + 104) = sin(WorldTime * 0.0020000001) * 0.30000001 + 0.5;
          break;
        case 0x20:
        case 0x22:
          *(DWORD *)(LODWORD(o) + 100) = 1;
          *(float *)(LODWORD(o) + 104) = (sin(WorldTime * 0.0040000002) + 1.0) * 0.5;
          break;
        case 0x26:
          goto LABEL_111;
        case 0x28:
          *(DWORD *)(LODWORD(o) + 100) = 0;
          v22 = WorldTime * 0.0040000002;
          *(DWORD *)(LODWORD(o) + 204) = 1028443341;
          *(float *)(LODWORD(o) + 104) = sin(v22) * 0.30000001 + 0.5;
          break;
        default:
          return;
      }
      break;
    case 8:
      switch ( *(WORD *)(LODWORD(o) + 2) )
      {
        case 2:
          *(DWORD *)(LODWORD(o) + 100) = 0;
          *(float *)(LODWORD(o) + 108) = (double)(-(__int64)WorldTime % 1000) * 0.001;
          return;
        case 4:
          v23 = WorldTime * 0.0020000001;
          *(DWORD *)(LODWORD(o) + 100) = 0;
          v11 = sin(v23) * 0.34999999 + 0.64999998;
          *(float *)(LODWORD(o) + 104) = v11;
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.000099999997;
          v30[0] = v11;
          v30[1] = v11;
          goto LABEL_116;
        case 7:
          v24 = *(float *)(LODWORD(o) + 36) * 100.0 + WorldTime;
          *(DWORD *)(LODWORD(o) + 100) = 0;
          v12 = sin(v24 * 0.0020000001) * 0.34999999 + 0.64999998;
          *(float *)(LODWORD(o) + 104) = v12;
LABEL_51:
          v30[0] = v12;
          v30[1] = v12 * 0.60000002;
          v11 = v12 * 0.2;
LABEL_116:
          v30[2] = v11;
          AddTerrainLight(*v7, *(float *)(LODWORD(o) + 20), v30, 3, PrimaryTerrainLight[0]);
          return;
        case 0xB:
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.00019999999;
          return;
        case 0xC:
          *(float *)(LODWORD(o) + 108) = (double)(-(__int64)WorldTime % 50000) * 0.000049999999;
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 50000) * 0.000049999999;
          return;
        case 0xD:
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.00019999999;
          return;
        case 0x3D:
          *(DWORD *)(LODWORD(o) + 100) = 1;
          v25 = (double)(-(__int64)WorldTime % 1000);
          goto LABEL_123;
        case 0x3F:
        case 0x40:
          goto LABEL_132;
        case 0x41:
        case 0x42:
          *(DWORD *)(LODWORD(o) + 100) = 1;
          v25 = (double)(-(__int64)WorldTime % 1000);
LABEL_123:
          *(float *)(LODWORD(o) + 112) = v25 * 0.001;
          v26 = sin(WorldTime * 0.0020000001) * 0.34999999 + 0.64999998;
          v30[0] = v26;
          v30[1] = v26 * 0.60000002;
          v30[2] = v26 * 0.2;
          AddTerrainLight(*v7, *(float *)(LODWORD(o) + 20), v30, 2, PrimaryTerrainLight[0]);
          break;
        case 0x48:
          *(DWORD *)(LODWORD(o) + 100) = 0;
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.00019999999;
          break;
        case 0x49:
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.00019999999;
          break;
        case 0x4B:
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.00019999999;
          break;
        case 0x4F:
          *(float *)(LODWORD(o) + 112) = (double)(-(__int64)WorldTime % 10000) * 0.00019999999;
          break;
        case 0x52:
          *(DWORD *)(LODWORD(o) + 100) = 0;
          *(DWORD *)(LODWORD(o) + 232) = 1065353216;
          *(DWORD *)(LODWORD(o) + 236) = 1065353216;
          *(DWORD *)(LODWORD(o) + 240) = 1065353216;
          break;
        default:
          return;
      }
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      v27 = *(short *)(LODWORD(o) + 2);
      if ( v27 >= 9 && v27 <= 10 && *(WORD *)(LODWORD(o) + 134) != 4 )
      {
LABEL_132:
        *(DWORD *)(LODWORD(o) + 88) = -2;
      }
      break;
    default:
      return;
  }
}
#undef LODWORD
#undef Models
#undef EditFlag
#endif

// ── FUN_004ffcc0 (IDA-only, gated) ──
#if defined(IDA_PORT_004FFCC0)
void __cdecl FUN_004ffcc0(DWORD *lpMem, int a2)
{
  int v2; // eax
  int v3; // ecx

  if ( lpMem )
  {
    v2 = lpMem[110];
    v3 = lpMem[109];
    if ( v2 )
    {
      if ( v3 )
      {
        *(DWORD *)(v3 + 440) = v2;
        *(DWORD *)(v2 + 436) = lpMem[109];
      }
      else
      {
        *(DWORD *)(v2 + 436) = 0;
        *(DWORD *)(a2 + 4) = v2;
      }
    }
    else if ( v3 )
    {
      *(DWORD *)(v3 + 440) = 0;
      *(DWORD *)(a2 + 8) = v3;
    }
    else
    {
      *(DWORD *)(a2 + 4) = 0;
      *(DWORD *)(a2 + 8) = 0;
    }
    delete__(lpMem);
  }
}
#endif

// ── FUN_00502b80 (IDA-only, gated) ──
#if defined(IDA_PORT_00502B80)
void __cdecl FUN_00502b80()
{
  BYTE *v0; // eax

  v0 = &Items[0][72];
  do
  {
    *v0 = 0;
    v0 += 516;
  }
  while ( (int)v0 < (int)&DAT_07e907e0 );
}
#endif

// ── FUN_00503fe0 (IDA-only, gated) ──
#if defined(IDA_PORT_00503FE0)
int __cdecl FUN_00503fe0(int a1, float a2, float a3, int a4)
{
  int result; // eax
  int v5; // ecx
  double v6; // st7
  int v7; // ecx
  double v8; // st6
  double v9; // st6

  result = a1;
  v5 = 0;
  switch ( a1 )
  {
    case 533:
    case 541:
    case 414:
    case 565:
LABEL_16:
      v5 = 2;
      break;
    case 418:
LABEL_6:
      v5 = 0;
      break;
    case 545:
      v5 = 0;
      break;
    case 569:
      v5 = 0;
      break;
    case 420:
LABEL_12:
      v5 = 1;
      break;
    default:
      result = (a1 - 400) / 32;
      if ( result >= 7 && result <= 11 )
      {
        result = (a1 - 400) % 32;
        switch ( result )
        {
          case 0:
          case 1:
          case 2:
          case 3:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
          case 12:
          case 13:
          case 16:
            goto LABEL_6;
          case 4:
          case 14:
          case 15:
          case 17:
            goto LABEL_12;
          case 18:
            goto LABEL_16;
          default:
            goto LABEL_17;
        }
      }
      break;
  }
LABEL_17:
  v6 = a2 * a3;
  if ( v5 )
  {
    v7 = v5 - 1;
    if ( v7 )
    {
      if ( v7 == 1 )
      {
        result = a4;
        v8 = v6 * *(float *)(a4 + 4);
        *(DWORD *)a4 = 0;
        *(float *)(a4 + 4) = v8 * 0.5;
        *(float *)(a4 + 8) = v6 * *(float *)(a4 + 8);
      }
    }
    else
    {
      result = a4;
      v9 = v6 * *(float *)a4;
      *(DWORD *)(a4 + 8) = 0;
      *(float *)a4 = v9;
      *(float *)(a4 + 4) = v6 * *(float *)(a4 + 4) * 0.5;
    }
  }
  else
  {
    result = a4;
    *(float *)a4 = v6 * *(float *)a4;
    *(float *)(a4 + 4) = v6 * *(float *)(a4 + 4);
    *(float *)(a4 + 8) = v6 * *(float *)(a4 + 8);
  }
  return result;
}
#endif

// ── FUN_00509190 (IDA-only, gated) ──
#if defined(IDA_PORT_00509190)
void __cdecl FUN_00509190()
{
  int i; // esi
  int j; // esi

  for ( i = 62980; i < 71440; i += 188 )
  {
    BMD::Release(i + Models);
  }
  for ( j = 120; j < 170; ++j )
  {
    ReleaseBuffer(j);
  }
}
#endif

// ── FUN_00509880 (IDA-only, gated) ──
#if defined(IDA_PORT_00509880)
void __cdecl FUN_00509880()
{
  int i; // esi
  int j; // esi

  for ( i = 50760; i < 62980; i += 188 )
  {
    BMD::Release(i + Models);
  }
  for ( j = 170; j < 420; ++j )
  {
    ReleaseBuffer(j);
  }
}
#endif

// ── FUN_0050c4d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0050C4D0)
void __cdecl FUN_0050c4d0()
{
  int i; // eax
  int v1; // esi
  int j; // esi
  int v3; // esi
  int v4; // ebx
  BITMAP_t *v5; // ebp
  char *v6; // edi
  int k; // esi
  int m; // esi
  int v9; // eax
  int v10; // esi
  int v11; // esi
  int v12; // esi
  int v13; // esi
  int v14; // esi
  int v15; // esi
  int v16; // esi
  int v17; // esi
  int v18; // esi
  int v19; // esi
  int v20; // esi
  int v21; // esi
  int v22; // esi
  int v23; // esi
  int v24; // esi
  int v25; // esi
  int v26; // esi
  int v27; // esi
  int v28; // esi
  int v29; // esi
  int v30; // esi
  int v31; // esi
  int ii; // esi
  int v33; // ebp
  int v34; // eax
  int v35; // ebx
  int v36; // eax
  int v37; // esi
  int n; // esi
  char v39; // [esp+3h] [ebp-385h]
  char FileName[32]; // [esp+4h] [ebp-384h] BYREF
  char Buffer[100]; // [esp+24h] [ebp-364h] BYREF
  char ModelFileName[256]; // [esp+88h] [ebp-300h] BYREF
  char v43[256]; // [esp+188h] [ebp-200h] BYREF
  char v44[256]; // [esp+288h] [ebp-100h] BYREF

  v39 = DAT_0055a7c4;
  if ( DAT_083a410c )
  {
    DAT_0055a7c4 = 0;
    World = 7;
  }
  OpenJPG(aObject8Drop01J, 0x4D9u, 0x2600u, 0x2900u, 0, 1);
  if ( !DAT_0055a7c4 )
  {
    switch ( World )
    {
      case 0:
        OpenModel(174, aData2Object1An, aBirdSmd, aBirdsFlySmd, aBirdsStopSmd, "end");
        OpenModel(181, aData2Object1An, aFishSmd, aFishsRunSmd, aFishsJumpSmd, "end");
        break;
      case 1:
      case 4:
        OpenModel(215, aData2Object2, aU, "end");
        OpenModel(176, aData2Object2, aBatSmd, aBatsSmd, "end");
        OpenModel(177, aData2Object2, aMouseSmd, aMousesSmd, "end");
        break;
      case 3:
        OpenModel(175, aData2Object1An, aButterflySmd, aButterflySSmd, "end");
        break;
      case 5:
        OpenModel(228, aData2Object6, aE_1, "end");
        OpenModel(229, aData2Object6, aE_2, "end");
        OpenModel(230, aData2Object6, aE_3, "end");
        OpenModel(231, aData2Object6, aE_4, "end");
        OpenModel(232, aData2Object6, aE_5, "end");
        OpenModel(234, aData2Monster, aAiud_0, aAiudOSSmd, aAiudOSAoSmd, "end");
        OpenModel(235, aData2Object6, aOaoSmd, aOaosAAu01Smd, aOaosAAu02Smd, "end");
        break;
      case 6:
        OpenModel(178, aData2Object7, &DAT_0055f560, &DAT_0055f56c, &DAT_0055f580, "end");
        *(BYTE *)(*(DWORD *)(Models + 33512) + 26) = 1;
        break;
      case 7:
        OpenModel(182, aData2Object8, aA_7, &DAT_0055f540, "end");
        OpenModel(183, aData2Object8, aB, &DAT_0055f514, "end");
        OpenModel(184, aData2Object8, aA_8, &DAT_0055f4f8, "end");
        OpenModel(185, aData2Object8, aB_0, &DAT_0055f4dc, "end");
        OpenModel(186, aData2Object8, aAuao, &DAT_0055f4c0, "end");
        OpenModel(187, aData2Object8, aA_9, &DAT_0055f4a0, "end");
        OpenModel(188, aData2Object8, aAo_1, &DAT_0055f480, "end");
        OpenModel(189, aData2Object8, aCoA, &DAT_0055f464, "end");
        for ( i = 188; i < 1692; *(BYTE *)(*(DWORD *)(i + Models + 33888) + 10) = 1 )
        {
          i += 188;
        }
        break;
      case 8:
        OpenModel(179, aData2Object9, aUua, &DAT_0055f444, "end");
        break;
      case 10:
        OpenJPG(aEffectCloudsJp, 0x4F4u, 0x2601u, 0x2900u, 0, 1);
        OpenModel(182, aData2Object11, aCloudSmd, "end");
        AccessModel(182, aDataObject11, aCloud, -1);
        OpenTexture(182, aObject11, 9728, 1);
        OpenJPG(aEffectCloudlig, 0x4F5u, 0x2601u, 0x2900u, 0, 1);
        break;
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
        OpenModel(184, aData2Object12, &DAT_0055f388, &DAT_0055f394, &DAT_0055f3a4, "end");
        break;
      default:
        break;
    }
  }
  SetMaxTextures(105);
  switch ( World )
  {
    case 0:
      AccessModel(174, aDataObject1, aBird, 1);
      OpenTexture(174, aObject1, 9728, 1);
      AccessModel(181, aDataObject1, aFish, 1);
      OpenTexture(181, aObject1, 9728, 1);
      break;
    case 1:
    case 4:
      AccessModel(215, aDataObject2, aDungeonstone, 1);
      OpenTexture(215, aObject2, 9728, 1);
      AccessModel(176, aDataObject2, DAT_0055f31c, 1);
      OpenTexture(176, aObject2, 9728, 1);
      AccessModel(177, aDataObject2, DAT_0055f318, 1);
      OpenTexture(177, aObject2, 9728, 1);
      break;
    case 3:
      AccessModel(175, aDataObject1, aButterfly, 1);
      OpenTexture(175, aObject1, 9728, 1);
      break;
    case 5:
      v1 = 228;
      do
      {
        AccessModel(v1, aDataObject6, aMeteo, v1 - 227);
        ++v1;
      }
      while ( v1 - 228 < 5 );
      AccessModel(234, aDataObject6, aBosshead, 1);
      AccessModel(235, aDataObject6, aPrincess, 1);
      for ( j = 228; j <= 235; ++j )
      {
        OpenTexture(j, aObject6, 9728, 1);
      }
      break;
    case 6:
      AccessModel(178, aDataObject7, DAT_0055f2cc, 1);
      OpenTexture(178, aObject7, 9728, 1);
      break;
    case 7:
      v3 = 182;
      do
      {
        AccessModel(v3, aDataObject8, aFish, v3 - 180);
        OpenTexture(v3++, aObject8, 9728, 1);
      }
      while ( v3 - 181 < 9 );
      v4 = 0;
      v5 = &Bitmaps[65];
      do
      {
        if ( (int)v5 >= (int)Bitmaps[75].FileName )
        {
          sprintf(Buffer, "Object8\\wt%d.jpg", v4);
        }
        else
        {
          sprintf(Buffer, "Object8\\wt0%d.jpg", v4);
        }
        OpenJPG(Buffer, v4 + 65, 0x2601u, 0x2901u, 0, 0);
        if ( (int)v5 >= (int)Bitmaps[75].FileName )
        {
          sprintf(Buffer, "wt%d.jpg", v4);
        }
        else
        {
          sprintf(Buffer, "wt0%d.jpg", v4);
        }
        v6 = (char *)v5++;
        strcpy(v6, Buffer);
        ++v4;
      }
      while ( (int)v5 < (int)Bitmaps[97].FileName );
      break;
    case 8:
      OpenJPG(aObject9Sand01J, 0x494u, 0x2601u, 0x2901u, 0, 1);
      OpenJPG(aObject9Sand02J, 0x495u, 0x2601u, 0x2901u, 0, 1);
      OpenJPG(aObject9Impack0, 0x597u, 0x2601u, 0x2900u, 0, 1);
      AccessModel(179, aDataObject9, DAT_0055f2cc, 2);
      OpenTexture(179, aObject9, 9728, 1);
      break;
    case 10:
      OpenJPG(aEffectCloudsJp, 0x4F4u, 0x2601u, 0x2900u, 0, 1);
      AccessModel(182, aDataObject11, aCloud, -1);
      OpenTexture(182, aObject11, 9728, 1);
      OpenJPG(aEffectCloudlig, 0x4F5u, 0x2601u, 0x2900u, 0, 1);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      if ( !DAT_0055a7c4 )
      {
        OpenModel(184, aData2Object12, &DAT_0055f388, &DAT_0055f394, &DAT_0055f3a4, "end");
        OpenModel(262, aData2Object12, &DAT_0055f1e8, "end");
        OpenModel(263, aData2Object12, &DAT_0055f1d8, "end");
        OpenModel(260, aData2Object12, &DAT_0055f1c8, "end");
        OpenModel(261, aData2Object12, &DAT_0055f1b8, "end");
        OpenModel(185, aData2Object12, aCOu2Smd, "end");
      }
      AccessModel(184, aDataObject12, aCrow, 1);
      OpenTexture(184, aObject12, 9728, 1);
      AccessModel(262, aDataObject12, aGate, 1);
      AccessModel(263, aDataObject12, aGate, 2);
      AccessModel(260, aDataObject12, aStonecoffin, 1);
      AccessModel(261, aDataObject12, aStonecoffin, 2);
      for ( k = 0; k < 2; ++k )
      {
        OpenTexture(k + 262, "Monster\\", 9728, 1);
      }
      for ( m = 0; m < 2; ++m )
      {
        OpenTexture(m + 260, "Monster\\", 9728, 1);
      }
      AccessModel(185, aDataObject12, aShine, 1);
      OpenTexture(185, aObject12, 9728, 1);
      OpenJPG(aEffectCloudsJp, 0x4F4u, 0x2601u, 0x2900u, 0, 1);
      LoadWaveFile(110, "Data\\Sound\\iBloodCastle.wav", 1, 0);
      DAT_0055a7c4 = 1;
      break;
    default:
      break;
  }
  SetMaxTextures(750);
  if ( World )
  {
    v33 = World + 1;
    if ( World >= 11 && World <= 16 )
    {
      v33 = 12;
    }
    if ( !DAT_0055a7c4 )
    {
      sprintf(FileName, "Data2\\Object%d\\_ÆÄÀÏ.txt", v33);
      ParserFileHandle = fopen(FileName, "rb");
      if ( ParserFileHandle )
      {
        while ( 1 )
        {
          v34 = ParseNextToken();
          v35 = (__int64)DAT_083a40f8;
          if ( v34 == 2 )
          {
            break;
          }
          ParseNextToken();
          strcpy(ModelFileName, (const char *)&ParserTokenString);
          ParseNextToken();
          strcpy(v43, (const char *)&ParserTokenString);
          ParseNextToken();
          strcpy(v44, (const char *)&ParserTokenString);
          sprintf(FileName, aData2ObjectD, v33);
          if ( strlen(v43) )
          {
            if ( strlen(v44) )
            {
              OpenModel(v35, FileName, ModelFileName, v43, v44, "end");
            }
            else
            {
              OpenModel(v35, FileName, ModelFileName, v43, "end");
            }
          }
          else
          {
            OpenModel(v35, FileName, ModelFileName, "end");
          }
        }
        fclose(ParserFileHandle);
      }
    }
    sprintf(FileName, "Data\\Object%d\\", v33);
    v36 = 0;
    do
    {
      v37 = v36 + 1;
      AccessModel(v36, FileName, aObject, v36 + 1);
      v36 = v37;
    }
    while ( v37 < 160 );
    SetMaxTextures(750);
    sprintf(FileName, "Object%d\\", v33);
    for ( n = 0; n < 160; ++n )
    {
      OpenTexture(n, FileName, 9728, 1);
    }
    if ( World == 1 )
    {
      *(DWORD *)(*(DWORD *)(Models + 7568) + 20) = 1053609165;
    }
    else if ( World == 8 )
    {
      *(BYTE *)(Models + 2204) = 0;
      *(BYTE *)(Models + 2392) = 0;
      *(BYTE *)(Models + 2580) = 0;
      *(BYTE *)(Models + 13860) = 0;
      *(BYTE *)(Models + 14236) = 0;
      *(BYTE *)(Models + 14988) = 0;
    }
  }
  else
  {
    if ( !DAT_0055a7c4 )
    {
      OpenModel(0, aData2Object1, aTreesmallSmd, aTreesmallsSmd, "end");
      OpenModel(1, aData2Object1, aTreebigSmd, aTreebigsSmd, "end");
      OpenModel(2, aData2Object1, aTreea01Smd, "end");
      OpenModel(3, aData2Object1, aTreea02Smd, "end");
      OpenModel(4, aData2Object1, aTreea03Smd, "end");
      OpenModel(5, aData2Object1, aTreea04Smd, "end");
      OpenModel(6, aData2Object1, aTreea05Smd, "end");
      OpenModel(7, aData2Object1, aTreea06Smd, "end");
      OpenModel(8, aData2Object1, aTreea07Smd, "end");
      OpenModel(9, aData2Object1, aTreea08Smd, "end");
      OpenModel(10, aData2Object1, aTreea09Smd, aTreeas09Smd, "end");
      OpenModel(11, aData2Object1, aTreea10Smd, aTreeas10Smd, "end");
      OpenModel(12, aData2Object1, aTreea11Smd, aTreeas11Smd, "end");
      OpenModel(20, aData2Object1, aGrass01Smd, "end");
      OpenModel(21, aData2Object1, aGrass02Smd, "end");
      OpenModel(22, aData2Object1, aGrass03Smd, "end");
      OpenModel(23, aData2Object1, aGrass04Smd, "end");
      OpenModel(24, aData2Object1, aGrass05Smd, "end");
      OpenModel(25, aData2Object1, aGrass06Smd, "end");
      OpenModel(26, aData2Object1, aMushroom01Smd, "end");
      OpenModel(27, aData2Object1, aMushroom02Smd, "end");
      OpenModel(30, aData2Object1, aSton01Smd, "end");
      OpenModel(31, aData2Object1, aSton02Smd, "end");
      OpenModel(32, aData2Object1, aSton03Smd, "end");
      OpenModel(33, aData2Object1, aSton04Smd, "end");
      OpenModel(34, aData2Object1, aSton05Smd, "end");
      OpenModel(40, aData2Object1, aStoneStatue01S, "end");
      OpenModel(41, aData2Object1, aStoneStatue02S, "end");
      OpenModel(42, aData2Object1, aAngelStoneSmd, "end");
      OpenModel(43, aData2Object1, aSteelBarredDoo, "end");
      OpenModel(44, aData2Object1, aTombArcSmd, "end");
      DAT_083a4100 = 1;
      OpenModel(45, aData2Object1, aTombCrossSmd, "end");
      OpenModel(46, aData2Object1, aTombstoneSmd, "end");
      OpenModel(50, aData2Object1, aFireLightSmd, "end");
      OpenModel(51, aData2Object1, aFireLight01Smd, "end");
      OpenModel(52, aData2Object1, aFireSmd, "end");
      OpenModel(55, aData2Object1, aDungeonGate01S, "end");
      OpenModel(58, aData2Object1, aDrumSmd, "end");
      OpenModel(59, aData2Object1, aTreasureChestS, aTreasureChests, "end");
      OpenModel(60, aData2Object1, aShipSmd, aShipsSmd, "end");
      OpenModel(69, aData2Object1, aWall01Smd, "end");
      OpenModel(70, aData2Object1, aWall02Smd, "end");
      OpenModel(71, aData2Object1, aWall03Smd, "end");
      OpenModel(72, aData2Object1, aCWall06Smd, aCWalls06Smd, "end");
      OpenModel(73, aData2Object1, aWall05Smd, "end");
      OpenModel(74, aData2Object1, aWall06Smd, aWalls06Smd, "end");
      OpenModel(75, aData2Object1, aCWall01Smd, "end");
      OpenModel(76, aData2Object1, aCWall02Smd, "end");
      OpenModel(77, aData2Object1, aCWall03Smd, "end");
      OpenModel(78, aData2Object1, aCWall05Smd, "end");
      DAT_083a4100 = 1;
      OpenModel(65, aData2Object1, aSteelBarred02S, "end");
      OpenModel(66, aData2Object1, aSteelBarred03S, "end");
      OpenModel(67, aData2Object1, aSteelBarred01S, "end");
      OpenModel(68, aData2Object1, aSteelBarredDoo_0, "end");
      OpenModel(91, aData2Object1, aGun01Smd, "end");
      OpenModel(92, aData2Object1, aGun02Smd, "end");
      OpenModel(93, aData2Object1, aGun03Smd, "end");
      OpenModel(80, aData2Object1, aBridge01Smd, "end");
      OpenModel(81, aData2Object1, aFenceSmd, "end");
      OpenModel(82, aData2Object1, aJoint01Smd, "end");
      OpenModel(83, aData2Object1, aJoint02Smd, "end");
      OpenModel(84, aData2Object1, aJoint03Smd, "end");
      OpenModel(85, aData2Object1, aBridge02Smd, "end");
      OpenModel(90, aData2Object1, aStreetlightSmd, aStreetlightsSm, "end");
      OpenModel(95, aData2Object1, aBadge01Smd, aBadges01Smd, "end");
      OpenModel(98, aData2Object1, aHorseDrawnSmd, aHorseDrawnsSmd, "end");
      OpenModel(99, aData2Object1, aCarriage01Smd, "end");
      OpenModel(100, aData2Object1, aCarriage02Smd, "end");
      OpenModel(101, aData2Object1, aCarriage03Smd, "end");
      OpenModel(102, aData2Object1, aRice01Smd, "end");
      OpenModel(103, aData2Object1, aRice02Smd, "end");
      OpenModel(96, aData2Object1, aSignboardUpSmd, aSignboardUpSSm, "end");
      OpenModel(97, aData2Object1, aSignboardDownS, "end");
      OpenModel(56, aData2Object1, aMonsterASmd, aMonsterAStop01, aMonsterAStop02, "end");
      OpenModel(57, aData2Object1, aMonsterBSmd, aMonsterBStop01, aMonsterBStop02, "end");
      OpenModel(105, aData2Object1, aWaterspoutSmd, aWaterspoutsSmd, "end");
      OpenModel(106, aData2Object1, aJar01Smd, "end");
      OpenModel(107, aData2Object1, aJar02Smd, "end");
      OpenModel(108, aData2Object1, aJar03Smd, "end");
      OpenModel(109, aData2Object1, aJar04Smd, "end");
      OpenModel(110, aData2Object1, aExecutionGroun_0, aExecutionGroun, "end");
      OpenModel(115, aData2Object1, aHouseStone01Sm, "end");
      OpenModel(116, aData2Object1, aHouseStone02Sm, "end");
      OpenModel(117, aData2Object1, aHouseSmithSmd, "end");
      OpenModel(118, aData2Object1, aHouseScienceSm, aHouseSciencesS, "end");
      OpenModel(119, aData2Object1, aHouseMillSmd, aHouseMillsSmd, "end");
      OpenModel(120, aData2Object1, aTent01Smd, aTents01Smd, "end");
      OpenModel(111, aData2Object1, aStairsSmd, "end");
      OpenModel(121, aData2Object1, aHouseJoint01Sm, "end");
      OpenModel(122, aData2Object1, aHouseJoint02Sm, "end");
      OpenModel(123, aData2Object1, aHouseJoint03Sm, "end");
      OpenModel(124, aData2Object1, aHouseJoint04Sm, "end");
      OpenModel(125, aData2Object1, aHouseJoint05Sm, "end");
      OpenModel(126, aData2Object1, aHouseJoint06Sm, "end");
      OpenModel(127, aData2Object1, aCWall07Smd, "end");
      OpenModel(128, aData2Object1, aHouseSmd, "end");
      OpenModel(129, aData2Object1, aCageSmd, "end");
      OpenModel(130, aData2Object1, aEffectSmd, "end");
      OpenModel(131, aData2Object1, aEffectSmd, "end");
      OpenModel(132, aData2Object1, aEffectSmd, "end");
      OpenModel(133, aData2Object1, aPoseBoxSmd, "end");
      OpenModel(140, aData2Object1, aHouseIn01Smd, "end");
      OpenModel(141, aData2Object1, aHouseIn02Smd, "end");
      OpenModel(142, aData2Object1, aHouseIn03Smd, "end");
      OpenModel(143, aData2Object1, aHouseIn04Smd, "end");
      OpenModel(144, aData2Object1, aHouseIn05Smd, "end");
      OpenModel(145, aData2Object1, aHouseIn06Smd, "end");
      OpenModel(146, aData2Object1, aHouseIn07Smd, "end");
      OpenModel(150, aData2Object1, aCandleSmd, aCandlesSmd, "end");
      OpenModel(151, aData2Object1, aHouseInBeer01S, "end");
      OpenModel(152, aData2Object1, aHouseInBeer02S, "end");
      OpenModel(153, aData2Object1, aHouseInBeer03S, "end");
    }
    v9 = 0;
    do
    {
      v10 = v9 + 1;
      AccessModel(v9, aDataObject1, aTree, v9 + 1);
      v9 = v10;
    }
    while ( v10 < 13 );
    v11 = 20;
    do
    {
      AccessModel(v11, aDataObject1, aGrass, v11 - 19);
      ++v11;
    }
    while ( v11 - 20 < 8 );
    v12 = 30;
    do
    {
      AccessModel(v12, aDataObject1, aStone, v12 - 29);
      ++v12;
    }
    while ( v12 - 30 < 5 );
    v13 = 40;
    do
    {
      AccessModel(v13, aDataObject1, aStonestatue, v13 - 39);
      ++v13;
    }
    while ( v13 - 40 < 3 );
    AccessModel(43, aDataObject1, aSteelstatue, 1);
    v14 = 44;
    do
    {
      AccessModel(v14, aDataObject1, aTomb, v14 - 43);
      ++v14;
    }
    while ( v14 - 44 < 3 );
    v15 = 50;
    do
    {
      AccessModel(v15, aDataObject1, aFirelight, v15 - 49);
      ++v15;
    }
    while ( v15 - 50 < 2 );
    AccessModel(52, aDataObject1, aBonfire, 1);
    AccessModel(55, aDataObject1, aDoungeongate, 1);
    AccessModel(58, aDataObject1, aTreasuredrum, 1);
    AccessModel(59, aDataObject1, aTreasurechest, 1);
    AccessModel(60, aDataObject1, aShip, 1);
    v16 = 69;
    do
    {
      AccessModel(v16, aDataObject1, aStonewall, v16 - 68);
      ++v16;
    }
    while ( v16 - 69 < 6 );
    v17 = 75;
    do
    {
      AccessModel(v17, aDataObject1, aStonemuwall, v17 - 74);
      ++v17;
    }
    while ( v17 - 75 < 4 );
    v18 = 65;
    do
    {
      AccessModel(v18, aDataObject1, aSteelwall, v18 - 64);
      ++v18;
    }
    while ( v18 - 65 < 3 );
    AccessModel(68, aDataObject1, aSteeldoor, 1);
    v19 = 91;
    do
    {
      AccessModel(v19, aDataObject1, aCannon, v19 - 90);
      ++v19;
    }
    while ( v19 - 91 < 3 );
    AccessModel(80, aDataObject1, aBridge, 1);
    v20 = 81;
    do
    {
      AccessModel(v20, aDataObject1, aFence, v20 - 80);
      ++v20;
    }
    while ( v20 - 81 < 4 );
    AccessModel(85, aDataObject1, aBridgestone, 1);
    AccessModel(90, aDataObject1, aStreetlight, 1);
    AccessModel(95, aDataObject1, aCurtain, 1);
    v21 = 98;
    do
    {
      AccessModel(v21, aDataObject1, aCarriage, v21 - 97);
      ++v21;
    }
    while ( v21 - 98 < 4 );
    v22 = 102;
    do
    {
      AccessModel(v22, aDataObject1, aStraw, v22 - 101);
      ++v22;
    }
    while ( v22 - 102 < 2 );
    v23 = 96;
    do
    {
      AccessModel(v23, aDataObject1, aSign, v23 - 95);
      ++v23;
    }
    while ( v23 - 96 < 2 );
    v24 = 56;
    do
    {
      AccessModel(v24, aDataObject1, aMerchantanimal, v24 - 55);
      ++v24;
    }
    while ( v24 - 56 < 2 );
    AccessModel(105, aDataObject1, aWaterspout, 1);
    v25 = 106;
    do
    {
      AccessModel(v25, aDataObject1, aWell, v25 - 105);
      ++v25;
    }
    while ( v25 - 106 < 4 );
    AccessModel(110, aDataObject1, aHanging, 1);
    v26 = 115;
    do
    {
      AccessModel(v26, aDataObject1, aHouse, v26 - 114);
      ++v26;
    }
    while ( v26 - 115 < 5 );
    AccessModel(120, aDataObject1, aTent, 1);
    AccessModel(111, aDataObject1, aStair, 1);
    v27 = 121;
    do
    {
      AccessModel(v27, aDataObject1, aHousewall, v27 - 120);
      ++v27;
    }
    while ( v27 - 121 < 6 );
    v28 = 127;
    do
    {
      AccessModel(v28, aDataObject1, aHouseetc, v28 - 126);
      ++v28;
    }
    while ( v28 - 127 < 3 );
    v29 = 130;
    do
    {
      AccessModel(v29, aDataObject1, aLight, v29 - 129);
      ++v29;
    }
    while ( v29 - 130 < 3 );
    AccessModel(133, aDataObject1, aPosebox, 1);
    v30 = 140;
    do
    {
      AccessModel(v30, aDataObject1, aFurniture, v30 - 139);
      ++v30;
    }
    while ( v30 - 140 < 7 );
    AccessModel(150, aDataObject1, aCandle, 1);
    v31 = 151;
    do
    {
      AccessModel(v31, aDataObject1, "Beer", v31 - 150);
      ++v31;
    }
    while ( v31 - 151 < 3 );
    for ( ii = 0; ii < 160; ++ii )
    {
      OpenTexture(ii, aObject1, 9728, 1);
    }
  }
  if ( DAT_083a410c )
  {
    DAT_0055a7c4 = v39;
  }
}
#endif

// ── FUN_0050f700 (IDA-only, gated) ──
#if defined(IDA_PORT_0050F700)
int __cdecl FUN_0050f700(char *FileName)
{
  FILE *v1; // edi
  const char *v2; // esi

  v1 = fopen(FileName, aWt);
  v2 = (const char *)&DAT_07e0ffc8;
  do
  {
    fprintf(v1, "%s\n", v2);
    v2 += 256;
  }
  while ( (int)v2 < (int)&ItemKey );
  return fclose(v1);
}
#endif

// FUN_00512d40 (IDA port) is now active above — see L11220+ block.

// ── FUN_005130f0 (IDA-only, gated) ──
#if defined(IDA_PORT_005130F0)
bool __cdecl FUN_005130f0(
        float a1,
        float a2,
        int a3,
        float a4,
        float a5,
        float a6,
        float a7,
        float a8,
        float a9,
        float a10,
        float a11,
        float a12,
        float a13,
        float a14,
        float a15)
{
  double v16; // st7
  double v17; // st6
  float v19; // [esp+Ch] [ebp-Ch]
  float v20; // [esp+10h] [ebp-8h]
  float v21; // [esp+14h] [ebp-4h]
  float v22; // [esp+1Ch] [ebp+4h]
  float v23; // [esp+1Ch] [ebp+4h]
  float v24; // [esp+20h] [ebp+8h]
  float v25; // [esp+20h] [ebp+8h]
  int v26; // [esp+24h] [ebp+Ch]
  float v27; // [esp+28h] [ebp+10h]

  v24 = *(float *)(LODWORD(a2) + 4) * *(float *)(LODWORD(a1) + 4)
      + *(float *)(LODWORD(a2) + 8) * *(float *)(LODWORD(a1) + 8)
      + *(float *)LODWORD(a1) * *(float *)LODWORD(a2);
  v22 = *(float *)(a3 + 4) * *(float *)(LODWORD(a1) + 4)
      + *(float *)(a3 + 8) * *(float *)(LODWORD(a1) + 8)
      + *(float *)LODWORD(a1) * *(float *)a3;
  v21 = FUN_00512a30(v24, v22);
  v20 = FUN_00512a10(v24, v22);
  v16 = a5 * *(float *)(LODWORD(a1) + 4) + a4 * *(float *)LODWORD(a1) + a6 * *(float *)(LODWORD(a1) + 8);
  v19 = v16;
  v23 = a8 * *(float *)(LODWORD(a1) + 4) + a7 * *(float *)LODWORD(a1) + a9 * *(float *)(LODWORD(a1) + 8);
  v25 = a11 * *(float *)(LODWORD(a1) + 4) + a10 * *(float *)LODWORD(a1) + a12 * *(float *)(LODWORD(a1) + 8);
  *(float *)&v26 = a14 * *(float *)(LODWORD(a1) + 4) + a13 * *(float *)LODWORD(a1) + a15 * *(float *)(LODWORD(a1) + 8);
  v27 = v16;
  if ( v23 <= 0.0 )
  {
    v16 = v23 + v19;
    v17 = v27;
  }
  else
  {
    v17 = v23 + v19;
  }
  if ( v25 <= 0.0 )
  {
    v16 = v16 + v25;
  }
  else
  {
    v17 = v17 + v25;
  }
  if ( *(float *)&v26 <= 0.0 )
  {
    v16 = v16 + *(float *)&v26;
  }
  else
  {
    v17 = v17 + *(float *)&v26;
  }
  return v20 <= v17 && v16 <= v21;
}
#endif

// ── FUN_00513260 (IDA-only, gated) ──
// Usa macros Hex-Rays sin portar (LODWORD/qmemcpy/FUN_004f9d20); queda gated.
// El hover de items del suelo (FUN_004afa40) usa proximidad world-space en su
// lugar (ver stubs_mouse_hover.cpp).
#if defined(IDA_PORT_00513260)
bool __cdecl FUN_00513260(
        float a1,
        int a2,
        char a3,
        int a4,
        int a5,
        float a6,
        int a7,
        int a8,
        float a9,
        int a10,
        int a11,
        float a12)
{
  double v12; // st7
  double v13; // st7
  bool result; // al
  float v15[12]; // [esp-38h] [ebp-70h] BYREF
  float v16[3]; // [esp+8h] [ebp-30h] BYREF
  float v17[3]; // [esp+14h] [ebp-24h] BYREF
  float v18[3]; // [esp+20h] [ebp-18h] BYREF
  float v19[3]; // [esp+2Ch] [ebp-Ch] BYREF

  v12 = *(float *)a2 - *(float *)LODWORD(a1);
  LODWORD(v15[11]) = v17;
  v16[0] = v12;
  v13 = *(float *)(a2 + 4) - *(float *)(LODWORD(a1) + 4);
  LODWORD(v15[10]) = &a6;
  LODWORD(v15[9]) = v16;
  v16[1] = v13;
  v16[2] = *(float *)(a2 + 8) - *(float *)(LODWORD(a1) + 8);
  FUN_004f9d20(v16, &a6, v17);
  FUN_004f9d20(v16, &a9, v18);
  FUN_004f9d20(v16, &a12, v19);
  qmemcpy(v15, &a3, sizeof(v15));
  result = FUN_005130f0(
             COERCE_FLOAT(v17),
             a1,
             a2,
             v15[0],
             v15[1],
             v15[2],
             v15[3],
             v15[4],
             v15[5],
             v15[6],
             v15[7],
             v15[8],
             v15[9],
             v15[10],
             v15[11]);
  if ( result )
  {
    qmemcpy(v15, &a3, sizeof(v15));
    result = FUN_005130f0(
               COERCE_FLOAT(v18),
               a1,
               a2,
               v15[0],
               v15[1],
               v15[2],
               v15[3],
               v15[4],
               v15[5],
               v15[6],
               v15[7],
               v15[8],
               v15[9],
               v15[10],
               v15[11]);
    if ( result )
    {
      qmemcpy(v15, &a3, sizeof(v15));
      result = FUN_005130f0(
                 COERCE_FLOAT(v19),
                 a1,
                 a2,
                 v15[0],
                 v15[1],
                 v15[2],
                 v15[3],
                 v15[4],
                 v15[5],
                 v15[6],
                 v15[7],
                 v15[8],
                 v15[9],
                 v15[10],
                 v15[11]);
      if ( result )
      {
        qmemcpy(v15, &a3, sizeof(v15));
        result = FUN_005130f0(
                   COERCE_FLOAT(&a6),
                   a1,
                   a2,
                   v15[0],
                   v15[1],
                   v15[2],
                   v15[3],
                   v15[4],
                   v15[5],
                   v15[6],
                   v15[7],
                   v15[8],
                   v15[9],
                   v15[10],
                   v15[11]);
        if ( result )
        {
          qmemcpy(v15, &a3, sizeof(v15));
          result = FUN_005130f0(
                     COERCE_FLOAT(&a9),
                     a1,
                     a2,
                     v15[0],
                     v15[1],
                     v15[2],
                     v15[3],
                     v15[4],
                     v15[5],
                     v15[6],
                     v15[7],
                     v15[8],
                     v15[9],
                     v15[10],
                     v15[11]);
          if ( result )
          {
            qmemcpy(v15, &a3, sizeof(v15));
            return FUN_005130f0(
                     COERCE_FLOAT(&a12),
                     a1,
                     a2,
                     v15[0],
                     v15[1],
                     v15[2],
                     v15[3],
                     v15[4],
                     v15[5],
                     v15[6],
                     v15[7],
                     v15[8],
                     v15[9],
                     v15[10],
                     v15[11]);
          }
        }
      }
    }
  }
  return result;
}
#endif

// ── FUN_00513c10 (IDA-only, gated) ──
#if defined(IDA_PORT_00513C10)
void FUN_00513c10()
{
  int v0; // edi
  int v1; // ecx
  int i; // esi
  const char *v3; // ebx
  unsigned int v4; // kr04_4
  int v5; // ecx
  int j; // esi
  unsigned int v7; // kr08_4
  unsigned short v8; // bx
  int v9; // ecx
  int k; // esi
  char *v11; // eax
  int v12; // eax
  int v13; // ecx
  int v14; // ebx
  int v15; // edi
  signed int v16; // ebp
  int v17; // eax
  int v18; // esi
  char v19; // [esp+10h] [ebp-838h]
  char v20; // [esp+11h] [ebp-837h]
  char v21; // [esp+12h] [ebp-836h]
  char v22; // [esp+13h] [ebp-835h]
  char v23; // [esp+14h] [ebp-834h]
  char v24; // [esp+15h] [ebp-833h]
  char v25; // [esp+16h] [ebp-832h]
  char v26; // [esp+17h] [ebp-831h]
  char v27; // [esp+18h] [ebp-830h]
  char v28; // [esp+19h] [ebp-82Fh]
  char v29; // [esp+1Ah] [ebp-82Eh]
  char v30; // [esp+1Bh] [ebp-82Dh]
  char v31; // [esp+1Ch] [ebp-82Ch]
  char v32; // [esp+1Dh] [ebp-82Bh]
  char v33; // [esp+1Eh] [ebp-82Ah]
  char v34; // [esp+1Fh] [ebp-829h]
  char v35; // [esp+20h] [ebp-828h]
  char v36; // [esp+21h] [ebp-827h]
  char v37; // [esp+22h] [ebp-826h]
  char v38; // [esp+23h] [ebp-825h]
  char v39; // [esp+24h] [ebp-824h]
  char v40; // [esp+25h] [ebp-823h]
  char v41; // [esp+26h] [ebp-822h]
  char v42; // [esp+27h] [ebp-821h]
  char v43; // [esp+28h] [ebp-820h]
  char v44; // [esp+29h] [ebp-81Fh]
  char v45; // [esp+2Ah] [ebp-81Eh]
  char v46; // [esp+2Bh] [ebp-81Dh]
  char v47; // [esp+2Ch] [ebp-81Ch]
  char v48; // [esp+2Dh] [ebp-81Bh]
  char v49; // [esp+2Eh] [ebp-81Ah]
  char v50; // [esp+2Fh] [ebp-819h]
  unsigned int v51; // [esp+30h] [ebp-818h]
  void *(__cdecl **v52)(std::locale::facet *__hidden, unsigned int); // [esp+34h] [ebp-814h]
  WORD buf[514]; // [esp+38h] [ebp-810h] BYREF
  char v54[1024]; // [esp+43Ch] [ebp-40Ch] BYREF
  int v55; // [esp+844h] [ebp-4h]

  v0 = SelectedHero;
  SelectedHero = -1;
  DAT_005615e0 = v0;
  CurrentProtocolState = 56;
  v52 = &DAT_00552460;
  v55 = 0;
  (BYTE)(buf[1]) = -63;
  *(WORD *)((char *)&buf[1] + 1) = -3327;
  buf[0] = 3;
  v19 = -25;
  v20 = 109;
  v21 = 58;
  *((BYTE *)&buf[1] + buf[0]) = 2;
  v22 = -119;
  v23 = -68;
  v24 = -78;
  v1 = buf[0] + 1;
  v25 = -97;
  v26 = 115;
  v27 = 35;
  v28 = -88;
  v29 = -2;
  v30 = -74;
  v31 = 73;
  v32 = 93;
  v33 = 57;
  v34 = 93;
  v35 = -118;
  v36 = -53;
  v37 = 99;
  v38 = -115;
  v39 = -22;
  v40 = 125;
  v41 = 43;
  v42 = 95;
  v43 = -61;
  v44 = -79;
  v45 = -23;
  v46 = -125;
  v47 = 41;
  v48 = 81;
  v49 = -24;
  v50 = 86;
  for ( i = buf[0]; i != v1; ++i )
  {
    *((BYTE *)&buf[1] + i) ^= *((BYTE *)buf + i + 1) ^ *(&v19 + i % 32);
  }
  ++buf[0];
  v3 = (const char *)(CharactersClient + 916 * v0 + 449);
  v4 = strlen(v3) + 1;
  v51 = v4 - 1;
  if ( buf[0] + (unsigned short)(v4 - 1) <= 1024 )
  {
    qmemcpy((char *)&buf[1] + buf[0], v3, 4 * ((unsigned short)v51 >> 2) + (((BYTE)v4 - 1) & 3));
    v19 = -25;
    v20 = 109;
    v21 = 58;
    v22 = -119;
    v23 = -68;
    v24 = -78;
    v5 = buf[0] + (unsigned short)(v4 - 1);
    v25 = -97;
    v26 = 115;
    v27 = 35;
    v28 = -88;
    v29 = -2;
    v30 = -74;
    v31 = 73;
    v32 = 93;
    v33 = 57;
    v34 = 93;
    v35 = -118;
    v36 = -53;
    v37 = 99;
    v38 = -115;
    v39 = -22;
    v40 = 125;
    v41 = 43;
    v42 = 95;
    v43 = -61;
    v44 = -79;
    v45 = -23;
    v46 = -125;
    v47 = 41;
    v48 = 81;
    v49 = -24;
    v50 = 86;
    for ( j = buf[0]; j != v5; ++j )
    {
      *((BYTE *)&buf[1] + j) ^= *((BYTE *)buf + j + 1) ^ *(&v19 + j % 32);
    }
    buf[0] += v51;
  }
  v7 = strlen(v3) + 1;
  v8 = 10 - (v7 - 1);
  memset(v54, 0, v8);
  if ( buf[0] + v8 <= 1024 )
  {
    qmemcpy((char *)&buf[1] + buf[0], v54, (unsigned short)(10 - (v7 - 1)));
    v19 = -25;
    v20 = 109;
    v21 = 58;
    v22 = -119;
    v23 = -68;
    v24 = -78;
    v9 = buf[0] + v8;
    v25 = -97;
    v26 = 115;
    v27 = 35;
    v28 = -88;
    v29 = -2;
    v30 = -74;
    v31 = 73;
    v32 = 93;
    v33 = 57;
    v34 = 93;
    v35 = -118;
    v36 = -53;
    v37 = 99;
    v38 = -115;
    v39 = -22;
    v40 = 125;
    v41 = 43;
    v42 = 95;
    v43 = -61;
    v44 = -79;
    v45 = -23;
    v46 = -125;
    v47 = 41;
    v48 = 81;
    v49 = -24;
    v50 = 86;
    for ( k = buf[0]; k != v9; ++k )
    {
      *((BYTE *)&buf[1] + k) ^= *((BYTE *)buf + k + 1) ^ *(&v19 + k % 32);
    }
    buf[0] += v8;
  }
  if ( buf[0] + 10 <= 1024 )
  {
    v11 = (char *)&buf[1] + buf[0];
    v19 = -25;
    v20 = 109;
    *(DWORD *)v11 = *(DWORD *)&InputText[0][0];
    v21 = 58;
    v22 = -119;
    *((DWORD *)v11 + 1) = *(DWORD *)&InputText[0][4];
    v23 = -68;
    v24 = -78;
    *((WORD *)v11 + 4) = *(WORD *)&InputText[0][8];
    v12 = buf[0];
    v25 = -97;
    v26 = 115;
    v27 = 35;
    v13 = buf[0] + 10;
    v28 = -88;
    v29 = -2;
    v30 = -74;
    v31 = 73;
    v32 = 93;
    v33 = 57;
    v34 = 93;
    v35 = -118;
    v36 = -53;
    v37 = 99;
    v38 = -115;
    v39 = -22;
    v40 = 125;
    v41 = 43;
    v42 = 95;
    v43 = -61;
    v44 = -79;
    v45 = -23;
    v46 = -125;
    v47 = 41;
    v48 = 81;
    v49 = -24;
    v50 = 86;
    if ( buf[0] != v13 )
    {
      do
      {
        *((BYTE *)&buf[1] + v12) ^= *((BYTE *)buf + v12 + 1) ^ *(&v19 + v12 % 32);
        ++v12;
      }
      while ( v12 != v13 );
    }
    buf[0] += 10;
  }
  if ( (BYTE)(buf[1]) == 193 )
  {
    (BYTE)((buf[1]) >> 8) = buf[0];
  }
  else if ( (BYTE)(buf[1]) == 194 )
  {
    *(WORD *)((char *)&buf[1] + 1) = buf[0];
  }
  v14 = buf[0];
  v15 = 0;
  v16 = buf[0];
  if ( s != -1 )
  {
    while ( 1 )
    {
      v17 = send(s, (const char *)&buf[1] + v15, v14 - v15, 0);
      v18 = v17;
      if ( v17 == -1 )
      {
        break;
      }
      if ( v17 )
      {
        if ( DAT_055ce174 )
        {
          nullsub_2((int)&buf[1], v17);
        }
        v16 -= v18;
        v15 += v18;
        if ( v16 > 0 )
        {
          continue;
        }
      }
      goto LABEL_32;
    }
    if ( WSAGetLastError() == 10035 && DAT_055cc16c + v14 <= 0x2000 )
    {
      qmemcpy((char *)&DAT_055ca16c + DAT_055cc16c, &buf[1], v16);
      DAT_055cc16c += v16;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
LABEL_32:
  v55 = -1;
  v52 = &DAT_00552460;
  DAT_083a7c14 = 24;
  DAT_083a7c18 = 21;
  PlayBuffer(27, 0, 0);
  ClearInput(1);
  InputEnable = 0;
}
#endif

// ── FUN_0051d740 (IDA-only, gated) ──
#if defined(IDA_PORT_0051D740)
void __cdecl FUN_0051d740(char *strMsg)
{
  int v1[5]; // [esp+8h] [ebp-14h] BYREF

  g_iNumLineMessageBoxCustom = SeparateTextIntoLines(strMsg, g_lpszMessageBoxCustom[0], 7, 38);
  memset(&DAT_083a42f8, 0, 0x28u);
  v1[0] = 1;
  v1[1] = 71;
  v1[2] = 140;
  v1[3] = 70;
  v1[4] = 21;
  qmemcpy(&DAT_083a42f8, v1, 20u);
  if ( ErrorMessage )
  {
    NextErrorMessage = 139;
  }
  else
  {
    ErrorMessage = 139;
  }
}
#endif

// ── FUN_0051d780 (IDA-only, gated) ──
#if defined(IDA_PORT_0051D780)
void __cdecl FUN_0051d780(int a1, char a2)
{
  int v2[5]; // [esp+8h] [ebp-14h] BYREF

  DAT_083a7c04 = a1;
  DAT_083a7c09[0] = a2;
  DAT_083a7c08 = 0;
  g_iNumLineMessageBoxCustom = SeparateTextIntoLines(GlobalText[a1], g_lpszMessageBoxCustom[0], 7, 38);
  memset(&DAT_083a42f8, 0, 0x28u);
  v2[0] = 1;
  v2[1] = 71;
  v2[2] = 140;
  v2[3] = 70;
  v2[4] = 21;
  qmemcpy(&DAT_083a42f8, v2, 0x14u);
  if ( ErrorMessage )
  {
    NextErrorMessage = 141;
  }
  else
  {
    ErrorMessage = 141;
  }
}
#endif

// ── FUN_00529360 (IDA-only, gated) ──
#if defined(IDA_PORT_00529360)
bool __cdecl FUN_00529360(char *filename, float *BufferFloat)
{
  char *v2; // ebx
  const char *v3; // edi
  signed int v4; // esi
  char *v5; // edx
  int v6; // ecx
  bool v7; // zf
  FILE *v8; // eax
  unsigned int v10; // esi
  const void **v11; // edi
  char *v12; // ebx
  int v13; // eax
  int v14; // ecx
  int v15; // edi
  int v16; // esi
  float *v17; // eax
  int v18; // edx
  int v19; // [esp-4h] [ebp-5B0h]
  char v20[256]; // [esp+Ch] [ebp-5A0h] BYREF
  CHAR Text[256]; // [esp+10Ch] [ebp-4A0h] BYREF
  void (__cdecl  *v22[33])(int *); // [esp+20Ch] [ebp-3A0h] BYREF
  char v23[64]; // [esp+290h] [ebp-31Ch] BYREF
  char FileName[256]; // [esp+2D0h] [ebp-2DCh] BYREF
  int a1[28]; // [esp+3D0h] [ebp-1DCh] BYREF
  int v26; // [esp+440h] [ebp-16Ch]
  unsigned int v27; // [esp+444h] [ebp-168h]
  int v28; // [esp+44Ch] [ebp-160h]
  unsigned int v29; // [esp+45Ch] [ebp-150h]
  FILE *Stream; // [esp+5A0h] [ebp-Ch]
  int i; // [esp+5A4h] [ebp-8h]
  const void **v32; // [esp+5A8h] [ebp-4h]

  if ( DAT_0055a7c4 )
  {
    v2 = filename;
    v4 = 0;
    if ( (int)strlen(filename) > 0 )
    {
      v5 = filename;
      v6 = v20 - filename;
      for ( i = v20 - filename; ; v6 = i )
      {
        v7 = *v5 == 46;
        v5[v6] = *v5;
        if ( v7 )
        {
          break;
        }
        ++v4;
        ++v5;
        if ( v4 >= (int)strlen(filename) )
        {
          break;
        }
      }
    }
    v20[v4 + 1] = 0;
    strcpy(FileName, "Data\\");
    strcat(FileName, v20);
    v3 = "OZJ";
  }
  else
  {
    v2 = filename;
    strcpy(FileName, "Data2\\");
    v3 = filename;
  }
  strcat(FileName, v3);
  v8 = fopen(FileName, "rb");
  Stream = v8;
  if ( v8 )
  {
    if ( DAT_0055a7c4 )
    {
      fseek(v8, 24, 0);
    }
    else
    {
      SaveImage(24, "OZJ", v2, 0, 0);
    }
    a1[0] = (int)jpeg_std_error(v22);
    v22[0] = my_error_exit;
    if ( _setjmp3(v23, 0, v19) )
    {
      jpeg_destroy_decompress((int)a1);
      fclose(Stream);
      return 0;
    }
    else
    {
      jpeg_create_decompress(a1, 62, 464);
      jpeg_stdio_src((int)a1, (int)Stream);
      jpeg_read_header(a1, 1);
      jpeg_start_decompress((int)a1);
      v10 = v26 * v28;
      i = v26 * v28;
      v11 = (const void **)(*(int (__cdecl **)(int *, int, int, int))(a1[1] + 8))(a1, 1, v26 * v28, 1);
      v32 = v11;
      v12 = (char *)operator_new(v26 * v28 * v27);
      v13 = v27;
      if ( v29 < v27 )
      {
        while ( 1 )
        {
          jpeg_read_scanlines(a1, v11, 1);
          qmemcpy(&v12[i * (v27 - v29)], *v11, v10);
          v13 = v27;
          if ( v29 >= v27 )
          {
            break;
          }
          v11 = v32;
          v10 = i;
        }
      }
      v14 = 0;
      if ( v13 )
      {
        v15 = v26;
        i = v13;
        do
        {
          if ( v15 )
          {
            v16 = v15;
            v17 = &BufferFloat[v14 + 2];
            do
            {
              v17 += 3;
              v18 = (unsigned char)v12[v14];
              v14 += 3;
              v32 = (const void **)v18;
              *(v17 - 5) = (double)v18 * 0.0039215689;
              v32 = (const void **)(unsigned char)v12[v14 - 2];
              --v16;
              *(v17 - 4) = (double)(int)v32 * 0.0039215689;
              v32 = (const void **)(unsigned char)v12[v14 - 1];
              *(v17 - 3) = (double)(int)v32 * 0.0039215689;
            }
            while ( v16 );
          }
          --i;
        }
        while ( i );
      }
      delete__(v12);
      jpeg_finish_decompress(a1);
      jpeg_destroy_decompress((int)a1);
      fclose(Stream);
      return 1;
    }
  }
  else
  {
    sprintf(Text, "%s - File not exist.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    CErrorReport::Write((DWORD)&g_ErrorReport, "\r\n");
    MessageBoxA(g_hWnd, Text, 0, 0);
    SendMessageA(g_hWnd, 2u, 0, 0);
    return 0;
  }
}
#endif

// ── FUN_0052f4d0 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_0052f4d0(int a1, int a2, int a3, int a4)
{
  return *(DWORD *)(*(DWORD *)(a1 + 4) + 44) - a4;
}

// ── FUN_0053ad80 (IDA-activated, absent in Ghidra) ──
void FUN_0053ad80()
{
  ;
}

// ── FUN_0053cbb0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CBB0)
DWORD *__cdecl FUN_0053cbb0(DWORD *_this)
{
  *_this = &DAT_0055389c;
  FUN_0053cc00();
  return _this;
}
#endif

// ── FUN_0053cbd0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CBD0)
LPVOID __cdecl FUN_0053cbd0(LPVOID lpMem, char a2)
{
  FUN_0053cbf0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif

// ── FUN_0053cbf0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CBF0)
void __cdecl FUN_0053cbf0(DWORD *_this)
{
  *_this = &DAT_0055389c;
}
#endif

// ── FUN_0053cc00 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_0053cc00(DWORD *_this)
{
  int result; // eax
  DWORD *v2; // edx
  DWORD *v3; // ecx

  result = 0;
  _this[5] = 0;
  _this[6] = 0;
  _this[7] = 0;
  _this[8] = 0;
  v2 = _this + 1;
  v3 = _this + 9;
  *v2 = 0;
  v2[1] = 0;
  v2[2] = 0;
  v2[3] = 0;
  *v3 = 0;
  v3[1] = 0;
  v3[2] = 0;
  v3[3] = 0;
  return result;
}

// ── FUN_0053ce30 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CE30)
int __cdecl FUN_0053ce30(DWORD *_this, DWORD *a2, int a3)
{
  int v4; // edi
  int *v5; // esi
  int v6; // edi
  int *v7; // ecx
  int v8; // edx
  DWORD *v9; // esi
  int v10; // ebx
  int v11; // eax
  int v12; // eax
  int v13; // eax
  short v14; // bx
  char *v15; // esi
  int v16; // eax
  short v17; // dx
  WORD *v18; // eax
  short v19; // dx
  bool v20; // zf
  char v21; // cl
  int i; // eax
  int v24; // [esp+10h] [ebp-20h] BYREF
  int v25; // [esp+14h] [ebp-1Ch]
  WORD *v26; // [esp+18h] [ebp-18h]
  int v27; // [esp+1Ch] [ebp-14h]
  int v28[2]; // [esp+20h] [ebp-10h] BYREF
  int v29; // [esp+28h] [ebp-8h] BYREF
  int v30; // [esp+2Ch] [ebp-4h]

  v26 = a2;
  v28[0] = 0;
  v28[1] = 0;
  *a2 = 0;
  v29 = 0;
  a2[1] = 0;
  v30 = 0;
  v4 = 0;
  v5 = v28;
  v25 = 4;
  do
  {
    FUN_0053cf90(v5, 0, a3, v4, 16);
    v6 = v4 + 16;
    FUN_0053cf90(v5, 22, a3, v6, 2);
    v4 = v6 + 2;
    ++v5;
    --v25;
  }
  while ( v25 );
  v7 = &v29;
  v8 = (unsigned short)v30;
  v9 = _this + 15;
  v10 = 3;
  do
  {
    v11 = *v7--;
    v12 = *v9-- ^ v11;
    v13 = v8 ^ v12;
    v7[1] = v13;
    --v10;
    v8 = (unsigned short)v13;
  }
  while ( v10 );
  v14 = 0;
  v25 = (int)v28;
  v15 = (char *)(_this + 1);
  v27 = 4;
  do
  {
    v16 = *((DWORD *)v15 + 8);
    v15 += 4;
    v17 = *((DWORD *)v15 + 11) ^ ((unsigned int)(*(DWORD *)v25 * v16) % *((DWORD *)v15 - 1));
    v18 = v26;
    v19 = v14 ^ v17;
    v14 = *(DWORD *)v25;
    *v26 = v19;
    v26 = v18 + 1;
    v20 = v27 == 1;
    v25 += 4;
    --v27;
  }
  while ( !v20 );
  (WORD)(v24) = 0;
  FUN_0053cf90(&v24, 0, a3, v4, 16);
  v21 = -8;
  (BYTE)(v24) = v24 ^ ((BYTE)((v24) >> 8)) ^ 0x3D;
  for ( i = 0; i < 8; ++i )
  {
    v21 ^= *((BYTE *)a2 + i);
  }
  if ( ((BYTE)((v24) >> 8)) == v21 )
  {
    return (unsigned char)v24;
  }
  else
  {
    return -1;
  }
}
#endif

// ── FUN_0053cf90 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CF90)
int __stdcall FUN_0053cf90(int a1, int a2, int a3, int a4, int a5)
{
  int v5; // ebx
  unsigned int v6; // ebx
  int v7; // eax
  int v8; // esi
  BYTE *v9; // eax
  int v10; // ecx
  char *lpMem; // [esp+10h] [ebp-8h]

  v5 = FUN_0053d170(a4 + a5 - 1);
  v6 = 1 - FUN_0053d170(a4) + v5;
  lpMem = (char *)operator_new(v6 + 1);
  memset(lpMem, 0, v6 + 1);
  qmemcpy(lpMem, (const void *)(a3 + FUN_0053d170(a4)), v6);
  v7 = (a4 + a5) % 8;
  if ( v7 )
  {
    lpMem[v6 - 1] &= -1 << (8 - v7);
  }
  FUN_0053d0d0(lpMem, v6, -(a4 % 8));
  FUN_0053d0d0(lpMem, v6 + 1, a2 % 8);
  v8 = v6 + (a2 % 8 > a4 % 8);
  v9 = (BYTE *)(a1 + FUN_0053d170(a2));
  if ( v8 > 0 )
  {
    v10 = lpMem - v9;
    do
    {
      *v9 |= v9[v10];
      ++v9;
      --v8;
    }
    while ( v8 );
  }
  delete__(lpMem);
  return a2 + a5;
}
#endif

// ── FUN_0053d0d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D0D0)
char __stdcall FUN_0053d0d0(BYTE *a1, int a2, int a3)
{
  char result; // al
  BYTE *v4; // ecx
  BYTE *v5; // edi
  int v6; // esi
  BYTE *v7; // esi
  int v8; // edi
  int v9; // ebp

  result = a3;
  if ( a3 )
  {
    if ( a3 <= 0 )
    {
      v7 = a1;
      v8 = -a3;
      if ( a2 - 1 > 0 )
      {
        v9 = a2 - 1;
        do
        {
          *v7 = (*v7 << v8) | (v7[1] >> (a3 + 8));
          ++v7;
          --v9;
        }
        while ( v9 );
      }
      result = *v7 << v8;
      *v7 = result;
    }
    else
    {
      v4 = a1;
      v5 = &a1[a2 - 1];
      if ( a2 - 1 > 0 )
      {
        v6 = a2 - 1;
        while ( 1 )
        {
          *v5 = (*v5 >> a3) | (v4[v6 - 1] << (8 - a3));
          --v5;
          if ( --v6 <= 0 )
          {
            break;
          }
          v4 = a1;
        }
      }
      result = *v5 >> a3;
      *v5 = result;
    }
  }
  return result;
}
#endif

// ── FUN_0053d1c0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D1C0)
int __cdecl FUN_0053d1c0(DWORD *_this, LPCSTR lpFileName, short a3, int a4, int a5, int a6, int a7)
{
  HANDLE FileA; // eax
  void *v9; // esi
  int v10; // ebp
  char *v11; // ecx
  int i; // eax
  int v13; // edx
  int *v14; // ecx
  int j; // eax
  int v16; // edx
  int *v17; // ecx
  int k; // eax
  int v19; // edx
  int *v20; // ecx
  int m; // eax
  int v22; // edx
  short Buffer; // [esp+10h] [ebp-18h] BYREF
  int v25; // [esp+12h] [ebp-16h]
  int v26[4]; // [esp+18h] [ebp-10h] BYREF

  FileA = CreateFileA(lpFileName, 0x80000000, 1u, 0, 3u, 0x80u, 0);
  v9 = FileA;
  if ( FileA == (HANDLE)-1 )
  {
    return 0;
  }
  ReadFile(FileA, &Buffer, 6u, (LPDWORD)&lpFileName, 0);
  if ( Buffer != a3 || (v10 = a5, v25 != 16 * (a7 + a6 + a4 + a5) + 6) )
  {
    CloseHandle(v9);
    return 0;
  }
  if ( a4 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v11 = (char *)(_this + 1);
    for ( i = 0; i < 4; ++i )
    {
      v11 += 4;
      v13 = v26[i] ^ DAT_00562e48[i];
      *((DWORD *)v11 - 1) = v13;
    }
  }
  if ( v10 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v14 = _this + 5;
    for ( j = 0; j < 4; ++j )
    {
      v16 = v26[j] ^ DAT_00562e48[j];
      *v14++ = v16;
    }
  }
  if ( a6 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v17 = _this + 9;
    for ( k = 0; k < 4; ++k )
    {
      v19 = v26[k] ^ DAT_00562e48[k];
      *v17++ = v19;
    }
  }
  if ( a7 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v20 = _this + 13;
    for ( m = 0; m < 4; ++m )
    {
      v22 = v26[m] ^ DAT_00562e48[m];
      *v20++ = v22;
    }
  }
  CloseHandle(v9);
  return 1;
}
#endif

// ── FUN_0053d430 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D430)
int __cdecl FUN_0053d430(unsigned char *a1)
{
  int v2; // eax
  int v3; // esi
  LONG (__stdcall *v4)(struct _EXCEPTION_POINTERS *); // esi

  if ( lpParameter )
  {
    return 0;
  }
  v2 = operator_new(0x34Cu);
  v3 = v2;
  if ( v2 )
  {
    *(BYTE *)v2 = 0;
    *(BYTE *)(v2 + 1) = 0;
    *(DWORD *)(v2 + 4) = 0;
    *(DWORD *)(v2 + 8) = 0;
    *(DWORD *)(v2 + 12) = 0;
    *(DWORD *)(v2 + 16) = 0;
    *(BYTE *)(v2 + 20) = 0;
    *(DWORD *)(v2 + 24) = 0;
    *(BYTE *)(v2 + 28) = 0;
    *(BYTE *)(v2 + 29) = 0;
    *(BYTE *)(v2 + 30) = 0;
    *(DWORD *)(v2 + 32) = -1;
    *(DWORD *)(v2 + 600) = 0;
    FUN_005404c0(v2 + 604);
    FUN_00540a70((HCRYPTPROV *)(v3 + 632));
    *(DWORD *)(v3 + 704) = 0;
    *(DWORD *)(v3 + 708) = 0;
    *(DWORD *)(v3 + 744) = 0;
    *(DWORD *)(v3 + 748) = 0;
    *(BYTE *)(v3 + 816) = 0;
    *(BYTE *)(v3 + 817) = 0;
    *(DWORD *)(v3 + 820) = 1000;
    *(DWORD *)(v3 + 824) = 0;
    *(DWORD *)(v3 + 828) = 0;
    *(BYTE *)(v3 + 832) = 0;
    *(DWORD *)(v3 + 836) = 0;
    *(DWORD *)(v3 + 840) = 0;
  }
  else
  {
    v3 = 0;
  }
  lpParameter = (LPVOID)v3;
  v4 = SetUnhandledExceptionFilter(TopLevelExceptionFilter);
  *((DWORD *)lpParameter + 3) = FUN_0053d890(lpParameter, a1);
  if ( v4 )
  {
    SetUnhandledExceptionFilter(v4);
  }
  return *((DWORD *)lpParameter + 3);
}
#endif

// ── FUN_0053d580 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D580)
int __cdecl FUN_0053d580(char a1)
{
  BYTE *v2; // esi
  char *v3; // eax
  char *v4; // eax
  char *v5; // eax
  const CHAR *v6; // eax
  HANDLE v7; // eax
  char *v8; // eax
  DWORD ExitCode; // [esp+0h] [ebp-4h] BYREF

  if ( !lpParameter )
  {
    return 0;
  }
  ExitCode = (DWORD)lpParameter;
  v2 = lpParameter;
  if ( *((BYTE *)lpParameter + 1) )
  {
    return 1877;
  }
  if ( *(BYTE *)lpParameter )
  {
    ExitCode = 0;
    if ( !GetExitCodeProcess(hProcess, &ExitCode) || ExitCode == 259 )
    {
      if ( v2[30] )
      {
        v5 = (char *)FUN_0053e8c0(&DAT_00563418);
        FUN_0053eba0((int)(v2 + 32), v5, a1);
        return 640;
      }
      else
      {
        v6 = (const CHAR *)FUN_0053e8c0(&DAT_005632fc);
        v7 = OpenEventA(0x100000u, 0, v6);
        if ( v7 )
        {
          CloseHandle(v7);
          return 1877;
        }
        else
        {
          v8 = (char *)FUN_0053e8c0(&DAT_005633f8);
          FUN_0053eba0((int)(v2 + 32), v8, a1);
          return 630;
        }
      }
    }
    else
    {
      v4 = (char *)FUN_0053e8c0(&DAT_00563438);
      FUN_0053eba0((int)(v2 + 32), v4, a1);
      DAT_083bbaf0 = 8;
      return 620;
    }
  }
  else
  {
    v3 = (char *)FUN_0053e8c0(&DAT_00563438);
    FUN_0053eba0((int)(v2 + 32), v3, a1);
    return 610;
  }
}
#endif

// ── FUN_0053d5a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D5A0)
char __cdecl FUN_0053d5a0(LPCSTR lpString)
{
  if ( lpParameter )
  {
    return FUN_0053ed30((char *)lpParameter, lpString);
  }
  else
  {
    return 0;
  }
}
#endif

// ── FUN_0053d620 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D620)
void __cdecl FUN_0053d620(DWORD *_this)
{
  int v2; // eax
  DWORD *v3; // esi
  unsigned int v4; // eax
  bool v5; // cc
  char *v6; // eax
  const char *v7; // eax
  signed int v8; // ecx
  BYTE *v9; // eax
  int v10; // ebx
  BYTE *v11; // eax
  int v12; // edx
  int v13; // [esp-2Ch] [ebp-40h]
  int v14; // [esp-28h] [ebp-3Ch]
  int v15; // [esp-24h] [ebp-38h]
  int v16; // [esp-20h] [ebp-34h]
  int v17; // [esp-1Ch] [ebp-30h]
  int v18; // [esp-18h] [ebp-2Ch]
  int v19; // [esp-14h] [ebp-28h]
  char *v20; // [esp-10h] [ebp-24h]
  DWORD NumberOfBytesWritten; // [esp+4h] [ebp-10h] BYREF
  int v22; // [esp+10h] [ebp-4h]

  NumberOfBytesWritten = (DWORD)_this;
  v22 = 0;
  FUN_00540ac0(_this + 158);
  FUN_00540510(_this + 151);
  v2 = _this[8];
  v3 = _this + 8;
  v22 = -1;
  if ( v2 != -1 )
  {
    GetLocalTime((LPSYSTEMTIME)(v3 + 135));
    v4 = v3[141];
    if ( !v4 || (v5 = v3[142] <= v4, v6 = (char *)&DAT_00562ecc, v5) )
    {
      v6 = &strID;
    }
    v20 = v6;
    v19 = *((unsigned short *)v3 + 277);
    v18 = *((unsigned short *)v3 + 276);
    v17 = *((unsigned short *)v3 + 275);
    v16 = *((unsigned short *)v3 + 274);
    v15 = *((unsigned short *)v3 + 273);
    v14 = *((unsigned short *)v3 + 271);
    v13 = *((unsigned short *)v3 + 270);
    v7 = (const char *)FUN_0053e8c0(&DAT_00562e74);
    sprintf((char *const)v3 + 28, v7, v13, v14, v15, v16, v17, v18, v19, v20);
    v8 = strlen((const char *)v3 + 28);
    if ( v3[140] )
    {
      if ( v8 > 0 )
      {
        v9 = v3 + 7;
        do
        {
          v10 = v3[140] + 2;
          v3[140] = v10;
          *v9++ ^= (BYTE)v10 + 67;
        }
        while ( (int)&v9[-28 - (DWORD)v3] < v8 );
      }
    }
    else if ( v8 > 0 )
    {
      v11 = v3 + 7;
      do
      {
        v12 = 3 * v3[139] + 1;
        v3[139] = v12;
        *v11++ ^= (BYTE)v12 + 70;
      }
      while ( (int)&v11[-28 - (DWORD)v3] < v8 );
    }
    if ( v3[141] != 999 )
    {
      WriteFile((HANDLE)*v3, v3 + 7, v8, &NumberOfBytesWritten, 0);
    }
    SetEndOfFile((HANDLE)*v3);
    CloseHandle((HANDLE)*v3);
    *v3 = -1;
    DeleteCriticalSection((LPCRITICAL_SECTION)(v3 + 1));
  }
}
#endif

// ── FUN_0053d7d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D7D0)
int __cdecl FUN_0053d7d0(int a1, char a2, const CHAR *lpString)
{
  char *v4; // eax
  char *v6; // eax
  char *v7; // eax
  char *v8; // eax
  char v9; // [esp-Ch] [ebp-10h]
  int v10; // [esp-4h] [ebp-8h]
  char v11; // [esp-4h] [ebp-8h]

  if ( *(BYTE *)a1 )
  {
    v11 = a2;
    *(DWORD *)(a1 + 704) = lpString;
    FUN_005404a0(2225, 1554, lpString);
    if ( *(DWORD *)(a1 + 4) )
    {
      v9 = *(DWORD *)(a1 + 4);
      v6 = (char *)FUN_0053e8c0(&DAT_00562f04);
      FUN_0053eba0(a1 + 32, v6, v9);
      if ( !FUN_004070d0(*(DWORD *)(a1 + 4), *(DWORD *)(a1 + 8)) )
      {
        v7 = (char *)FUN_0053e8c0(&DAT_00562ee8);
        FUN_0053eba0(a1 + 32, v7, a2);
      }
    }
    v8 = (char *)FUN_0053e8c0(&DAT_00562ed0);
    FUN_0053eba0(a1 + 32, v8, v11);
    return *(DWORD *)(a1 + 12);
  }
  else
  {
    v10 = *(DWORD *)(a1 + 12);
    v4 = (char *)FUN_0053e8c0(&DAT_00562f24);
    FUN_0053eba0(a1 + 32, v4, v10);
    return *(DWORD *)(a1 + 12);
  }
}
#endif

// ── FUN_0053e930 (IDA-only, gated) ──
#if defined(IDA_PORT_0053E930)
BYTE *__cdecl FUN_0053e8c0(BYTE *a1)
{
  BYTE *result; // eax
  int v2; // ecx
  int v3; // esi
  char v4; // bl
  int v5; // ecx
  unsigned short v6; // dx
  int v7; // ecx
  char v8; // bl

  result = a1;
  if ( a1 && *a1 == 1 )
  {
    v2 = 9 * (char)a1[1] + 3;
    a1[2] ^= 3 * a1[1] + 101;
    v3 = 0;
    v4 = (v2 + 101) ^ a1[3];
    (BYTE)((v6) >> 8) = a1[2];
    v5 = v2 + 1;
    (BYTE)(v6) = v4;
    a1[3] = v4;
    if ( v6 )
    {
      do
      {
        v7 = 3 * v5;
        v8 = (v7 + 101) ^ a1[v3 + 4];
        v5 = v7 + 1;
        a1[v3++] = v8;
      }
      while ( v3 < v6 );
    }
    a1[v6] = 0;
  }
  return result;
}
#endif

// ── FUN_0053ea90 (IDA-only, gated) ──
#if defined(IDA_PORT_0053EA90)
BYTE *__cdecl FUN_0053e8c0(BYTE *a1)
{
  BYTE *result; // eax
  int v2; // ecx
  int v3; // esi
  char v4; // bl
  int v5; // ecx
  unsigned short v6; // dx
  int v7; // ecx
  char v8; // bl

  result = a1;
  if ( a1 && *a1 == 1 )
  {
    v2 = 9 * (char)a1[1] + 3;
    a1[2] ^= 3 * a1[1] + 101;
    v3 = 0;
    v4 = (v2 + 101) ^ a1[3];
    (BYTE)((v6) >> 8) = a1[2];
    v5 = v2 + 1;
    (BYTE)(v6) = v4;
    a1[3] = v4;
    if ( v6 )
    {
      do
      {
        v7 = 3 * v5;
        v8 = (v7 + 101) ^ a1[v3 + 4];
        v5 = v7 + 1;
        a1[v3++] = v8;
      }
      while ( v3 < v6 );
    }
    a1[v6] = 0;
  }
  return result;
}
#endif

// ── FUN_0053ed30 (IDA-only, gated) ──
#if defined(IDA_PORT_0053ED30)
char __cdecl FUN_0053ed30(char *_this, LPCSTR lpString)
{
  if ( !*_this )
  {
    return 0;
  }
  strcpy(_this + 752, lpString);
  return FUN_005404a0(2225, 1555, lpString);
}
#endif

// ── FUN_0053ed80 (IDA-only, gated) ──
#if defined(IDA_PORT_0053ED80)
char __cdecl FUN_0053ed80(int _this)
{
  int v3; // esi
  char *v4; // eax
  bool v5; // zf
  void *v6; // eax
  char *v7; // eax
  unsigned int v8; // eax
  bool v9; // cc
  char *v10; // eax
  const char *v11; // eax
  signed int v12; // ecx
  BYTE *v13; // eax
  int v14; // edx
  BYTE *v15; // eax
  int v16; // edx
  int v17; // [esp-28h] [ebp-34h]
  int v18; // [esp-24h] [ebp-30h]
  int v19; // [esp-20h] [ebp-2Ch]
  int v20; // [esp-1Ch] [ebp-28h]
  int v21; // [esp-18h] [ebp-24h]
  int v22; // [esp-14h] [ebp-20h]
  int v23; // [esp-10h] [ebp-1Ch]
  char *v24; // [esp-Ch] [ebp-18h]
  DWORD NumberOfBytesWritten; // [esp+8h] [ebp-4h] BYREF

  if ( *(BYTE *)(_this + 1) )
  {
    return 1;
  }
  v3 = _this + 32;
  v4 = FUN_0053e8c0(DAT_00563488);
  FUN_0053eba0(_this + 32, v4);
  v5 = *(BYTE *)_this == 0;
  *(BYTE *)(_this + 1) = 1;
  if ( v5 )
  {
    if ( DAT_00562e5c )
    {
      if ( DAT_083bbaf0 )
      {
        FUN_0053f680((char *)_this);
      }
    }
    return 0;
  }
  else
  {
    if ( *(BYTE *)(_this + 20) )
    {
      FUN_0053f290(_this);
      *(BYTE *)(_this + 20) = 0;
    }
    v6 = *(void **)(_this + 824);
    if ( v6 )
    {
      CloseHandle(v6);
      *(DWORD *)(_this + 824) = 0;
    }
    if ( *(DWORD *)(_this + 828) )
    {
      CloseHandle(*(HANDLE *)(_this + 828));
      *(DWORD *)(_this + 828) = 0;
    }
    if ( DAT_00562e5c && DAT_083bbaf0 )
    {
      FUN_0053f680((char *)_this);
    }
    v7 = FUN_0053e8c0(DAT_00563470);
    FUN_0053eba0(_this + 32, v7);
    if ( *(DWORD *)v3 != -1 )
    {
      GetLocalTime((LPSYSTEMTIME)(_this + 572));
      v8 = *(DWORD *)(_this + 596);
      if ( !v8 || (v9 = *(DWORD *)(_this + 600) <= v8, v10 = (char *)&DAT_00562ecc, v9) )
      {
        v10 = &strID;
      }
      v24 = v10;
      v23 = *(unsigned short *)(_this + 586);
      v22 = *(unsigned short *)(_this + 584);
      v21 = *(unsigned short *)(_this + 582);
      v20 = *(unsigned short *)(_this + 580);
      v19 = *(unsigned short *)(_this + 578);
      v18 = *(unsigned short *)(_this + 574);
      v17 = *(unsigned short *)(_this + 572);
      v11 = FUN_0053e8c0(DAT_00562e74);
      sprintf((char *const)(v3 + 28), v11, v17, v18, v19, v20, v21, v22, v23, v24);
      v12 = strlen((const char *)(v3 + 28));
      if ( *(DWORD *)(_this + 592) )
      {
        if ( v12 > 0 )
        {
          v13 = (BYTE *)(_this + 60);
          do
          {
            v14 = *(DWORD *)(_this + 592) + 2;
            *(DWORD *)(_this + 592) = v14;
            *v13++ ^= (BYTE)v14 + 67;
          }
          while ( (int)&v13[-28 - v3] < v12 );
        }
      }
      else if ( v12 > 0 )
      {
        v15 = (BYTE *)(_this + 60);
        do
        {
          v16 = 3 * *(DWORD *)(_this + 588) + 1;
          *(DWORD *)(_this + 588) = v16;
          *v15++ ^= (BYTE)v16 + 70;
        }
        while ( (int)&v15[-28 - v3] < v12 );
      }
      if ( *(DWORD *)(_this + 596) != 999 )
      {
        WriteFile(*(HANDLE *)v3, (LPCVOID)(_this + 60), v12, &NumberOfBytesWritten, 0);
      }
      SetEndOfFile(*(HANDLE *)v3);
      CloseHandle(*(HANDLE *)v3);
      *(DWORD *)v3 = -1;
      DeleteCriticalSection((LPCRITICAL_SECTION)(_this + 36));
    }
    *(BYTE *)_this = 0;
    return FUN_005406f0(_this + 604);
  }
}
#endif

// ── FUN_0053f290 (IDA-only, gated) ──
#if defined(IDA_PORT_0053F290)
char __cdecl FUN_0053f290(int _this)
{
  char *v3; // eax
  void *v4; // eax
  char *v5; // eax
  char *v6; // eax

  if ( !*(BYTE *)(_this + 20) )
  {
    return 1;
  }
  v3 = FUN_0053e8c0(DAT_00563604);
  FUN_0053eba0(_this + 32, v3);
  (*(void (__cdecl **)(DWORD, int))(_this + 840))(*(DWORD *)(_this + 708), _this + 752);
  v4 = *(void **)(_this + 16);
  *(BYTE *)(_this + 20) = 0;
  if ( v4 )
  {
    SetEvent(v4);
  }
  if ( WaitForSingleObject(*(HANDLE *)(_this + 748), 0x7D0u) == 258 )
  {
    v5 = FUN_0053e8c0(DAT_005635ec);
    FUN_0053eba0(_this + 32, v5);
    TerminateThread(*(HANDLE *)(_this + 748), 0);
  }
  CloseHandle(*(HANDLE *)(_this + 748));
  if ( *(DWORD *)(_this + 744) )
  {
    FreeLibrary(*(HMODULE *)(_this + 744));
  }
  *(DWORD *)(_this + 836) = 0;
  *(DWORD *)(_this + 840) = 0;
  v6 = FUN_0053e8c0(DAT_005635d8);
  FUN_0053eba0(_this + 32, v6);
  return 1;
}
#endif

// ── FUN_0053f680 (IDA-only, gated) ──
#if defined(IDA_PORT_0053F680)
HMODULE __cdecl FUN_0053f680(char *_this)
{
  char *v1; // esi
  char *v2; // eax
  char v3; // bl
  const CHAR *v4; // eax
  const CHAR *v5; // eax
  const CHAR *v6; // eax
  const CHAR *v7; // eax
  const CHAR *v8; // eax
  HMODULE result; // eax
  char *v10; // eax
  unsigned int v11; // eax
  bool v12; // cc
  char *v13; // eax
  const char *v14; // eax
  signed int v15; // ecx
  BYTE *v16; // eax
  int v17; // edx
  BYTE *v18; // eax
  int v19; // edx
  const CHAR *v20; // eax
  HMODULE v21; // edi
  const CHAR *v22; // eax
  FARPROC ProcAddress; // ebx
  const CHAR *v24; // eax
  FARPROC v25; // ebp
  const CHAR *v26; // eax
  const CHAR *v27; // eax
  BYTE *v28; // eax
  HMODULE v29; // ebx
  BYTE *v30; // eax
  int v31; // esi
  BYTE *v32; // eax
  const char *v33; // eax
  FARPROC v34; // ebx
  char *v35; // [esp-Ch] [ebp-300h]
  BYTE *v36; // [esp-4h] [ebp-2F8h]
  int v37; // [esp+0h] [ebp-2F4h]
  int v38; // [esp+Ch] [ebp-2E8h]
  char *v39; // [esp+Ch] [ebp-2E8h]
  int v40; // [esp+10h] [ebp-2E4h]
  int wYear; // [esp+10h] [ebp-2E4h]
  int v42; // [esp+14h] [ebp-2E0h]
  int wMonth; // [esp+14h] [ebp-2E0h]
  int v44; // [esp+18h] [ebp-2DCh]
  BYTE *v45; // [esp+18h] [ebp-2DCh]
  int wDay; // [esp+18h] [ebp-2DCh]
  int v47; // [esp+1Ch] [ebp-2D8h]
  BYTE *v48; // [esp+1Ch] [ebp-2D8h]
  int wHour; // [esp+1Ch] [ebp-2D8h]
  int v50; // [esp+20h] [ebp-2D4h]
  int wMinute; // [esp+20h] [ebp-2D4h]
  int v52; // [esp+24h] [ebp-2D0h]
  int wSecond; // [esp+24h] [ebp-2D0h]
  DWORD v54; // [esp+28h] [ebp-2CCh]
  char *v55; // [esp+28h] [ebp-2CCh]
  int wMilliseconds; // [esp+28h] [ebp-2CCh]
  HMODULE v57; // [esp+28h] [ebp-2CCh]
  DWORD cbData; // [esp+3Ch] [ebp-2B8h] BYREF
  BYTE v59[4]; // [esp+40h] [ebp-2B4h] BYREF
  HKEY phkResult; // [esp+44h] [ebp-2B0h] BYREF
  FARPROC v61; // [esp+48h] [ebp-2ACh]
  DWORD nSize; // [esp+4Ch] [ebp-2A8h] BYREF
  DWORD NumberOfBytesWritten; // [esp+50h] [ebp-2A4h] BYREF
  char *v64; // [esp+54h] [ebp-2A0h]
  DWORD dwDisposition; // [esp+58h] [ebp-29Ch] BYREF
  struct _SYSTEMTIME SystemTime; // [esp+5Ch] [ebp-298h] BYREF
  BYTE Data[64]; // [esp+70h] [ebp-284h] BYREF
  CHAR String2[64]; // [esp+B0h] [ebp-244h] BYREF
  CHAR Buffer[128]; // [esp+F0h] [ebp-204h] BYREF
  CHAR v70[128]; // [esp+170h] [ebp-184h] BYREF
  char v71[260]; // [esp+1F0h] [ebp-104h] BYREF

  v64 = _this;
  v1 = _this + 32;
  v2 = FUN_0053e8c0(DAT_005638e8);
  FUN_0053eba0((int)v1, v2);
  *(DWORD *)v59 = 0;
  v3 = 0;
  v4 = FUN_0053e8c0(DAT_005638b0);
  RegCreateKeyExA(HKEY_LOCAL_MACHINE, v4, 0, 0, 0, 0xF003Fu, 0, &phkResult, &dwDisposition);
  if ( dwDisposition == 2 )
  {
    cbData = 64;
    v5 = FUN_0053e8c0(DAT_0056389c);
    RegQueryValueExA(phkResult, v5, 0, 0, Data, &cbData);
    wsprintfA(String2, DAT_00563898, aFriOct24111310);
    if ( cbData && !lstrcmpA((LPCSTR)Data, String2) )
    {
      v3 = 1;
    }
    cbData = 4;
    v6 = FUN_0053e8c0(DAT_00563888);
    RegQueryValueExA(phkResult, v6, 0, 0, v59, &cbData);
  }
  wsprintfA((LPSTR)Data, DAT_00563898, aFriOct24111310);
  v54 = lstrlenA((LPCSTR)Data) + 1;
  cbData = v54;
  v7 = FUN_0053e8c0(DAT_00563874);
  RegSetValueExA(phkResult, v7, 0, 1u, Data, v54);
  cbData = 4;
  if ( v3 )
  {
    ++*(DWORD *)v59;
  }
  else
  {
    *(DWORD *)v59 = 0;
  }
  v8 = FUN_0053e8c0(DAT_00563888);
  RegSetValueExA(phkResult, v8, 0, 4u, v59, 4u);
  RegCloseKey(phkResult);
  result = *(HMODULE *)v59;
  if ( *(DWORD *)v59 != 1 && *(DWORD *)v59 <= 0xAu && (!v3 || DAT_083bbaf0 < 9) )
  {
    v10 = FUN_0053e8c0(DAT_00563864);
    FUN_0053eba0((int)v1, v10);
    if ( *(DWORD *)v1 != -1 )
    {
      GetLocalTime((LPSYSTEMTIME)(v1 + 540));
      v11 = *((DWORD *)v1 + 141);
      if ( !v11 || (v12 = *((DWORD *)v1 + 142) <= v11, v13 = (char *)&DAT_00562ecc, v12) )
      {
        v13 = &strID;
      }
      v55 = v13;
      v52 = *((unsigned short *)v1 + 277);
      v50 = *((unsigned short *)v1 + 276);
      v47 = *((unsigned short *)v1 + 275);
      v44 = *((unsigned short *)v1 + 274);
      v42 = *((unsigned short *)v1 + 273);
      v40 = *((unsigned short *)v1 + 271);
      v38 = *((unsigned short *)v1 + 270);
      v14 = FUN_0053e8c0(DAT_00562e74);
      sprintf(v1 + 28, v14, v38, v40, v42, v44, v47, v50, v52, v55);
      v15 = strlen(v1 + 28);
      if ( *((DWORD *)v1 + 140) )
      {
        if ( v15 > 0 )
        {
          v16 = v1 + 28;
          do
          {
            v17 = *((DWORD *)v1 + 140) + 2;
            *((DWORD *)v1 + 140) = v17;
            *v16++ ^= (BYTE)v17 + 67;
          }
          while ( (int)&v16[-28 - (DWORD)v1] < v15 );
        }
      }
      else if ( v15 > 0 )
      {
        v18 = v1 + 28;
        do
        {
          v19 = 3 * *((DWORD *)v1 + 139) + 1;
          *((DWORD *)v1 + 139) = v19;
          *v18++ ^= (BYTE)v19 + 70;
        }
        while ( (int)&v18[-28 - (DWORD)v1] < v15 );
      }
      if ( *((DWORD *)v1 + 141) != 999 )
      {
        WriteFile(*(HANDLE *)v1, v1 + 28, v15, &NumberOfBytesWritten, 0);
      }
      SetEndOfFile(*(HANDLE *)v1);
      CloseHandle(*(HANDLE *)v1);
      *(DWORD *)v1 = -1;
      DeleteCriticalSection((LPCRITICAL_SECTION)(v1 + 4));
    }
    v20 = FUN_0053e8c0(DAT_00563854);
    result = LoadLibraryA(v20);
    v21 = result;
    if ( result )
    {
      v22 = FUN_0053e8c0(DAT_00563840);
      ProcAddress = GetProcAddress(v21, v22);
      v24 = FUN_0053e8c0(DAT_00563828);
      v25 = GetProcAddress(v21, v24);
      v26 = FUN_0053e8c0(&DAT_00563810);
      v61 = GetProcAddress(v21, v26);
      v27 = FUN_0053e8c0(DAT_00563800);
      result = (HMODULE)GetProcAddress(v21, v27);
      NumberOfBytesWritten = (DWORD)result;
      if ( ProcAddress )
      {
        if ( v25 )
        {
          if ( v61 )
          {
            if ( result )
            {
              v28 = FUN_0053e8c0(DAT_005637f4);
              result = (HMODULE)((int (__stdcall *)(BYTE *, DWORD, DWORD, DWORD, DWORD))ProcAddress)(
                                  v28,
                                  0,
                                  0,
                                  0,
                                  0);
              v29 = result;
              if ( result )
              {
                v48 = FUN_0053e8c0(DAT_005637e4);
                v45 = FUN_0053e8c0(DAT_005637d8);
                v30 = FUN_0053e8c0(DAT_005637c4);
                v31 = ((int (__stdcall *)(HMODULE, BYTE *, int, BYTE *, BYTE *, int, DWORD, DWORD))v25)(
                        v29,
                        v30,
                        21,
                        v45,
                        v48,
                        1,
                        0,
                        0);
                if ( v31 )
                {
                  Buffer[0] = 0;
                  nSize = 128;
                  GetComputerNameA(Buffer, &nSize);
                  v70[0] = 0;
                  nSize = 128;
                  GetUserNameA(v70, &nSize);
                  GetLocalTime(&SystemTime);
                  switch ( DAT_083bbaf0 )
                  {
                    case 1:
                      v32 = FUN_0053e8c0(DAT_005637b8);
                      break;
                    case 2:
                      v32 = FUN_0053e8c0(DAT_005637ac);
                      break;
                    case 3:
                      v32 = FUN_0053e8c0(DAT_005637a4);
                      break;
                    case 4:
                      v32 = FUN_0053e8c0(DAT_00563798);
                      break;
                    case 5:
                      v32 = FUN_0053e8c0(DAT_00563790);
                      break;
                    case 6:
                      v32 = FUN_0053e8c0(DAT_00563780);
                      break;
                    case 7:
                      v32 = FUN_0053e8c0(DAT_00563774);
                      break;
                    case 8:
                      v32 = FUN_0053e8c0(DAT_00563768);
                      break;
                    case 9:
                      v32 = FUN_0053e8c0(DAT_0056375c);
                      break;
                    default:
                      v32 = FUN_0053e8c0(DAT_00563754);
                      break;
                  }
                  wMilliseconds = SystemTime.wMilliseconds;
                  wSecond = SystemTime.wSecond;
                  wMinute = SystemTime.wMinute;
                  wHour = SystemTime.wHour;
                  wDay = SystemTime.wDay;
                  wMonth = SystemTime.wMonth;
                  wYear = SystemTime.wYear;
                  v39 = v64 + 752;
                  v37 = *((DWORD *)lpParameter + 3);
                  v36 = v32;
                  v35 = v64 + 712;
                  v33 = FUN_0053e8c0(DAT_005636f8);
                  sprintf(
                    v71,
                    v33,
                    v35,
                    18,
                    v36,
                    v37,
                    Buffer,
                    v70,
                    v39,
                    wYear,
                    wMonth,
                    wDay,
                    wHour,
                    wMinute,
                    wSecond,
                    wMilliseconds);
                  ((void (__stdcall *)(int, CHAR *, char *, void *, DWORD))NumberOfBytesWritten)(
                    v31,
                    &DAT_083bb9e0,
                    v71,
                    &DAT_04000002,
                    0);
                  GetLastError();
                  v57 = v29;
                  v34 = v61;
                  ((void (__stdcall *)(HMODULE))v61)(v57);
                  ((void (__stdcall *)(int))v34)(v31);
                  return (HMODULE)FreeLibrary(v21);
                }
                else
                {
                  return (HMODULE)((int (__stdcall *)(HMODULE))v61)(v29);
                }
              }
            }
          }
        }
      }
    }
  }
  return result;
}
#endif

// ── FUN_0053fcf0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053FCF0)
char FUN_0053fcf0()
{
  char result; // al
  const CHAR *v1; // eax
  HMODULE ModuleHandleA; // esi
  const CHAR *v3; // eax
  const CHAR *v4; // eax
  const CHAR *v5; // eax
  const CHAR *v6; // eax
  const CHAR *v7; // eax

  if ( DAT_083bbb18 )
  {
    return 1;
  }
  v1 = FUN_0053e8c0(DAT_005639a8);
  ModuleHandleA = GetModuleHandleA(v1);
  if ( !ModuleHandleA )
  {
    return 0;
  }
  v3 = FUN_0053e8c0(DAT_00563988);
  DAT_083bbae4 = (int)GetProcAddress(ModuleHandleA, v3);
  if ( !DAT_083bbae4 )
  {
    return 0;
  }
  v4 = FUN_0053e8c0(DAT_00563974);
  DAT_083bb9d8 = GetProcAddress(ModuleHandleA, v4);
  if ( !DAT_083bb9d8 )
  {
    return 0;
  }
  v5 = FUN_0053e8c0(DAT_00563960);
  DAT_083bbaec = (int)GetProcAddress(ModuleHandleA, v5);
  if ( !DAT_083bbaec )
  {
    return 0;
  }
  v6 = FUN_0053e8c0(DAT_0056394c);
  DAT_083bbae8 = (int)GetProcAddress(ModuleHandleA, v6);
  if ( !DAT_083bbae8 )
  {
    return 0;
  }
  v7 = FUN_0053e8c0(DAT_00563938);
  DAT_083bb9dc = GetProcAddress(ModuleHandleA, v7);
  if ( !DAT_083bb9dc )
  {
    return 0;
  }
  result = 1;
  DAT_083bbb18 = 1;
  return result;
}
#endif

// ── FUN_005400d0 (IDA-only, gated) ──
#if defined(IDA_PORT_005400D0)
char __stdcall FUN_005400d0(int ArgList, unsigned int a2)
{
  BYTE *v2; // ecx
  int v3; // edi
  char *v4; // eax
  int v5; // esi
  char *v6; // eax
  char result; // al
  char *v8; // eax
  char *v9; // eax
  char *v10; // eax
  int v11; // [esp-4h] [ebp-Ch]

  v2 = lpParameter;
  v3 = a2;
  if ( lpParameter )
  {
    v4 = FUN_0053e8c0(DAT_00563b38);
    FUN_0053eba0((int)lpParameter + 32, v4, ArgList, a2);
    v2 = lpParameter;
  }
  switch ( ArgList )
  {
    case 1551:
      v5 = 1001;
      goto LABEL_37;
    case 1552:
      v5 = 1002;
      goto LABEL_37;
    case 1556:
      if ( v2[29] || FUN_0053efa0(v2, a2) )
      {
        goto LABEL_10;
      }
      v5 = 1014;
      v3 = 220;
      DAT_083bbaf0 = 2;
      goto LABEL_37;
    case 1557:
      DAT_083bbb08 = a2;
      if ( a2 == DAT_083bbb0c )
      {
        goto LABEL_10;
      }
      if ( v2 )
      {
        v11 = DAT_083bbb0c;
        v9 = FUN_0053e8c0(DAT_00563ae4);
        FUN_0053eba0((int)lpParameter + 32, v9, a2, v11);
      }
      DAT_083bbaf0 = 5;
      return 1;
    case 1560:
      if ( v2 )
      {
        v6 = FUN_0053e8c0(DAT_00563b20);
        FUN_0053eba0((int)lpParameter + 32, v6, a2);
      }
      if ( a2 > 0xC8 )
      {
        DAT_083bbaf0 = 9;
      }
      goto LABEL_10;
    case 1581:
      v5 = 1011;
      goto LABEL_15;
    case 1582:
      v5 = 1012;
      goto LABEL_18;
    case 1583:
      v5 = 1013;
      goto LABEL_21;
    case 1584:
      v5 = 1015;
      goto LABEL_15;
    case 1591:
      v5 = 1014;
      v3 = 210;
LABEL_18:
      if ( *v2 )
      {
        goto LABEL_37;
      }
      *((DWORD *)v2 + 1) = v5;
      *((DWORD *)lpParameter + 2) = v3;
      result = 1;
      break;
    case 1592:
      v5 = 1014;
      v3 = 220;
      DAT_083bbaf0 = 2;
LABEL_21:
      if ( *v2 )
      {
        goto LABEL_37;
      }
      *((DWORD *)v2 + 1) = v5;
      result = 1;
      *((DWORD *)lpParameter + 2) = v3;
      break;
    case 1593:
      v5 = 1014;
      v3 = 230;
      DAT_083bbaf0 = 2;
LABEL_15:
      if ( *v2 )
      {
        goto LABEL_37;
      }
      *((DWORD *)v2 + 1) = v5;
      result = 1;
      *((DWORD *)lpParameter + 2) = v3;
      break;
    case 1594:
      if ( v2 )
      {
        v8 = FUN_0053e8c0(DAT_00563b0c);
        FUN_0053eba0((int)lpParameter + 32, v8, a2, a2);
      }
      v5 = 1016;
      goto LABEL_37;
    default:
      if ( ArgList == 500 )
      {
        DAT_083bbaf0 = 3;
        v5 = 1014;
        v3 = 500;
      }
      else
      {
        v5 = 1000;
      }
LABEL_37:
      if ( FUN_004070d0(v5, v3) )
      {
LABEL_10:
        result = 1;
      }
      else
      {
        if ( lpParameter )
        {
          v10 = FUN_0053e8c0(DAT_00563ac4);
          FUN_0053eba0((int)lpParameter + 32, v10, v5, v3);
        }
        result = 0;
        *((BYTE *)lpParameter + 30) = 1;
      }
      break;
  }
  return result;
}
#endif

// ── FUN_005403a0 (IDA-only, gated) ──
#if defined(IDA_PORT_005403A0)
bool __cdecl FUN_005403a0(int a1, int a2, LPCSTR lpString)
{
  bool result; // al
  int v4; // eax
  BOOL v5; // eax
  DWORD LastError; // eax
  DWORD NumberOfBytesWritten; // [esp+0h] [ebp-84h] BYREF
  int Buffer[4]; // [esp+4h] [ebp-80h] BYREF
  char v9[112]; // [esp+14h] [ebp-70h] BYREF

  if ( hFile == (HANDLE)-1 )
  {
    return 0;
  }
  DAT_083bbb60 = a1;
  *(DWORD *)DAT_083bbb68 = a2;
  DAT_083bbb64 = DAT_083bbb74;
  ::lpString = (DWORD)lpString;
  if ( a2 == 1555 )
  {
    v4 = lstrlenA(lpString);
    Buffer[0] = DAT_083bbb60;
    ++v4;
    Buffer[2] = *(DWORD *)DAT_083bbb68;
    Buffer[1] = DAT_083bbb64;
    ::lpString = v4;
    Buffer[3] = v4;
    qmemcpy(v9, lpString, v4);
    v5 = WriteFile(hFile, Buffer, v4 + 16, &NumberOfBytesWritten, 0);
  }
  else
  {
    v5 = WriteFile(hFile, &DAT_083bbb60, 0x10u, &NumberOfBytesWritten, 0);
  }
  result = 1;
  if ( !v5 )
  {
    LastError = GetLastError();
    if ( LastError == 109 || LastError == 232 || LastError == 6 )
    {
      return 0;
    }
  }
  return result;
}
#endif

// ── FUN_005404c0 (IDA-only, gated) ──
#if defined(IDA_PORT_005404C0)
void *__cdecl FUN_005404c0(void *_this)
{
  void *result; // eax
  struct _SYSTEMTIME SystemTime; // [esp+4h] [ebp-10h] BYREF

  GetLocalTime(&SystemTime);
  result = _this;
  DAT_083bbb74 = (unsigned int)&DAT_00dad53a ^ (SystemTime.wYear * SystemTime.wMonth * SystemTime.wDay);
  return result;
}
#endif

// ── FUN_005406f0 (IDA-only, gated) ──
#if defined(IDA_PORT_005406F0)
char __cdecl FUN_005406f0(HANDLE *_this)
{
  HANDLE v3; // eax

  if ( hFile == (HANDLE)-1 )
  {
    return 0;
  }
  FUN_005404a0(2225, 1552, 0);
  if ( hEvent )
  {
    SetEvent(hEvent);
  }
  v3 = _this[6];
  if ( v3 )
  {
    if ( WaitForSingleObject(v3, 0x64u) == 258 )
    {
      TerminateThread(_this[6], 0);
    }
    CloseHandle(_this[6]);
    _this[6] = 0;
  }
  if ( hFile != (HANDLE)-1 )
  {
    CloseHandle(hFile);
    hFile = (HANDLE)-1;
  }
  if ( hNamedPipe != (HANDLE)-1 )
  {
    CloseHandle(hNamedPipe);
    hNamedPipe = (HANDLE)-1;
  }
  return 1;
}
#endif

// ── FUN_00540a70 (IDA-only, gated) ──
#if defined(IDA_PORT_00540A70)
HCRYPTPROV *__cdecl FUN_00540a70(HCRYPTPROV *phProv)
{
  phProv[5] = 0;
  phProv[3] = 0;
  *phProv = 0;
  phProv[1] = 0;
  phProv[2] = 0;
  phProv[9] = 0;
  phProv[14] = 0;
  phProv[16] = 0;
  phProv[15] = 0;
  phProv[6] = 0;
  *((BYTE *)phProv + 28) = 0;
  phProv[8] = 0;
  phProv[17] = 0;
  if ( !CryptAcquireContextA(phProv, 0, szProvider, 1u, 0xF0000000) )
  {
    *phProv = 0;
  }
  return phProv;
}
#endif

// ── FUN_00540ac0 (IDA-only, gated) ──
#if defined(IDA_PORT_00540AC0)
int __cdecl FUN_00540ac0(void *_this)
{
  HCRYPTHASH v2; // eax
  HCRYPTKEY v3; // eax
  int result; // eax
  HCRYPTPROV v5; // esi

  if ( *((DWORD *)_this + 1) )
  {
    FUN_00543c98(*((LPVOID *)_this + 1));
  }
  if ( *((DWORD *)_this + 5) )
  {
    FUN_00543c98(*((LPVOID *)_this + 5));
  }
  v2 = *((DWORD *)_this + 3);
  if ( v2 )
  {
    CryptDestroyHash(v2);
  }
  v3 = *((DWORD *)_this + 4);
  if ( v3 )
  {
    CryptDestroyKey(v3);
  }
  if ( *((DWORD *)_this + 15) )
  {
    CryptDestroyKey(*((DWORD *)_this + 15));
  }
  result = *((DWORD *)_this + 16);
  if ( result )
  {
    result = CryptDestroyHash(*((DWORD *)_this + 16));
  }
  v5 = *(DWORD *)_this;
  if ( v5 )
  {
    return CryptReleaseContext(v5, 0);
  }
  return result;
}
#endif

// ── FUN_00541450 (IDA-only, gated) ──
#if defined(IDA_PORT_00541450)
_onexit_t __cdecl _onexit(_onexit_t Func)
{
  unsigned int v1; // eax
  _onexit_t *v2; // ecx
  int v3; // eax
  char *v4; // eax
  int (__cdecl *v5)(); // esi
  int v6; // ecx

  _lockexit();
  v1 = FUN_00544832(DAT_083bd2d0);
  v2 = (_onexit_t *)DAT_083bd2cc;
  if ( v1 >= DAT_083bd2cc - (int)DAT_083bd2d0 + 4 )
  {
    goto LABEL_5;
  }
  v3 = FUN_00544832(DAT_083bd2d0);
  v4 = (char *)FUN_00544503((LPVOID)DAT_083bd2d0, v3 + 16);
  if ( v4 )
  {
    v6 = DAT_083bd2cc - (DWORD)DAT_083bd2d0;
    DAT_083bd2d0 = v4;
    v2 = (_onexit_t *)&v4[4 * (v6 >> 2)];
    DAT_083bd2cc = (int)v2;
LABEL_5:
    *v2 = Func;
    DAT_083bd2cc += 4;
    v5 = Func;
    goto LABEL_6;
  }
  v5 = 0;
LABEL_6:
  _unlockexit();
  return v5;
}
#endif

// ── FUN_005414ce (IDA-only, gated) ──
#if defined(IDA_PORT_005414CE)
int __cdecl FUN_005414ce(void (__cdecl *Func)())
{
  return (_onexit((_onexit_t)Func) != 0) - 1;
}
#endif

// ── FUN_005416bc (IDA-only, gated) ──
#if defined(IDA_PORT_005416BC)
int FUN_005416bc(char *const Buffer, const char *const Format, ...)
{
  int v2; // eax
  bool v3; // sf
  int v4; // esi
  FILE File; // [esp+4h] [ebp-20h] BYREF
  va_list va; // [esp+34h] [ebp+10h] BYREF

  va_start(va, Format);
  File._base = Buffer;
  File._ptr = Buffer;
  File._flag = 66;
  File._cnt = 0x7FFFFFFF;
  v2 = _output(&File, (int)Format, (int)va);
  v3 = --File._cnt < 0;
  v4 = v2;
  if ( v3 )
  {
    _flsbuf(0, &File);
  }
  else
  {
    *File._ptr = 0;
  }
  return v4;
}
#endif

// ── FUN_005420dd (IDA-only, gated) ──
#if defined(IDA_PORT_005420DD)
int __cdecl FUN_005420dd(int C)
{
  if ( (int)SrcSizeInBytes <= 1 )
  {
    return *((BYTE *)DAT_00564228 + 2 * C) & 8;
  }
  else
  {
    return _isctype(C, 8);
  }
}
#endif

// ── FUN_00542133 (IDA-only, gated) ──
#if defined(IDA_PORT_00542133)
double __cdecl FUN_00542133(const char *String)
{
  char v4[24]; // [esp+4h] [ebp-18h] BYREF

  while ( (int)SrcSizeInBytes <= 1 ? *((BYTE *)DAT_00564228 + 2 * *(unsigned char *)String) & 8 : _isctype(
                                                                                                     *(unsigned char *)String,
                                                                                                     8) )
  {
    ++String;
  }
  strlen(String);
  return *(double *)(_fltin2(v4, String) + 16);
}
#endif

// ── FUN_0054218a (IDA-only, gated) ──
#if defined(IDA_PORT_0054218A)
int __cdecl FUN_0054218a(FILE *Stream)
{
  int v2; // edi

  _lock_file(Stream);
  if ( --Stream->_cnt < 0 )
  {
    v2 = _filbuf(Stream);
  }
  else
  {
    v2 = *(unsigned char *)Stream->_ptr++;
  }
  _unlock_file(Stream);
  return v2;
}
#endif

// ── FUN_00542eb4 (IDA-only, gated) ──
#if defined(IDA_PORT_00542EB4)
int __cdecl FUN_00542eb4(FILE *Stream)
{
  int v1; // edi

  _lock_file(Stream);
  v1 = _ftell_lk(Stream);
  _unlock_file(Stream);
  return v1;
}
#endif

// ── FUN_00543264 (IDA-only, gated) ──
#if defined(IDA_PORT_00543264)
int __cdecl FUN_00543264(int Character, FILE *Stream)
{
  return fputc(Character, Stream);
}
#endif

// ── FUN_0054337b (IDA-only, gated) ──
#if defined(IDA_PORT_0054337B)
int FUN_0054337b(FILE *const Stream, const char *const Format, ...)
{
  int v2; // esi
  va_list va; // [esp+14h] [ebp+10h] BYREF

  va_start(va, Format);
  _lock_file(Stream);
  v2 = _input(Stream, (int)Format, (int)va);
  _unlock_file(Stream);
  return v2;
}
#endif

// ── FUN_005433cd (IDA-only, gated) ──
#if defined(IDA_PORT_005433CD)
double __cdecl _CIsin(double x)
{
  int v2; // [esp+0h] [ebp-8h]

  _checkTOS_withFB(LODWORD(x), HIDWORD(*(unsigned __int64 *)&x));
  return FUN_005433cd(v2);
}
#endif

// ── FUN_005436a6 (IDA-only, gated) ──
#if defined(IDA_PORT_005436A6)
int __cdecl FUN_005436a6(FILE *Stream)
{
  int v2; // edi

  if ( !Stream )
  {
    return flsall(0);
  }
  _lock_file(Stream);
  v2 = _fflush_lk(Stream);
  _unlock_file(Stream);
  return v2;
}
#endif

// ── FUN_00543839 (IDA-only, gated) ──
#if defined(IDA_PORT_00543839)
void __cdecl  FUN_00543839(int Code)
{
  doexit(Code, 0, 0);
}
#endif

// ── FUN_0054385b (IDA-only, gated) ──
#if defined(IDA_PORT_0054385B)
int __cdecl FUN_0054385b(UINT uExitCode, int a2, int a3)
{
  HANDLE CurrentProcess; // eax
  void (**v4)(void); // esi

  _lockexit();
  if ( DAT_083bbbf8 == 1 )
  {
    CurrentProcess = GetCurrentProcess();
    TerminateProcess(CurrentProcess, uExitCode);
  }
  DAT_083bbbf4 = 1;
  DAT_083bbbf0 = a3;
  if ( !a2 )
  {
    if ( DAT_083bd2d0 )
    {
      v4 = (void (**)(void))(DAT_083bd2cc - 4);
      if ( DAT_083bd2cc - 4 >= (unsigned int)DAT_083bd2d0 )
      {
        do
        {
          if ( *v4 )
          {
            (*v4)();
          }
          --v4;
        }
        while ( v4 >= DAT_083bd2d0 );
      }
    }
    _initterm(&DAT_00558070, &DAT_0055807c);
  }
  _initterm(&DAT_00558080, &DAT_00558088);
  if ( !a3 )
  {
    DAT_083bbbf8 = 1;
    ExitProcess(uExitCode);
  }
  return _unlockexit();
}
#endif

// ── FUN_005439e8 (IDA-only, gated) ──
#if defined(IDA_PORT_005439E8)
void __cdecl _lock_file(FILE *Stream)
{
  if ( Stream < (FILE *)&DAT_00563dc0 || Stream > &DAT_00564020 )
  {
    EnterCriticalSection((LPCRITICAL_SECTION)&Stream[1]);
  }
  else
  {
    _lock((((char *)Stream - (char *)&DAT_00563dc0) >> 5) + 28);
  }
}
#endif

// ── FUN_00543a3a (IDA-only, gated) ──
#if defined(IDA_PORT_00543A3A)
void __cdecl _unlock_file(FILE *Stream)
{
  if ( Stream < (FILE *)&DAT_00563dc0 || Stream > &DAT_00564020 )
  {
    LeaveCriticalSection((LPCRITICAL_SECTION)&Stream[1]);
  }
  else
  {
    _unlock((((char *)Stream - (char *)&DAT_00563dc0) >> 5) + 28);
  }
}
#endif

// ── FUN_00543a8c (IDA-only, gated) ──
#if defined(IDA_PORT_00543A8C)
int FUN_00543a8c(const char *const Buffer, const char *const Format, ...)
{
  FILE Stream; // [esp+0h] [ebp-20h] BYREF
  va_list va; // [esp+30h] [ebp+10h] BYREF

  va_start(va, Format);
  Stream._flag = 73;
  Stream._base = (char *)Buffer;
  Stream._ptr = (char *)Buffer;
  Stream._cnt = strlen(Buffer);
  return _input(&Stream, (int)Format, (int)va);
}
#endif

// ── FUN_00543ac0 (IDA-only, gated) ──
#if defined(IDA_PORT_00543AC0)
char *__cdecl FUN_00543ac0(char *VarName)
{
  char *v1; // esi

  _lock(12);
  v1 = getenv(VarName);
  _unlock(12);
  return v1;
}
#endif

// ── FUN_00543c98 (IDA-only, gated) ──
#if defined(IDA_PORT_00543C98)
void __cdecl FUN_00543c98(LPVOID lpMem)
{
  int block; // eax
  bool v2; // zf
  int v3; // eax
  int v4; // [esp+Ch] [ebp-28h] BYREF
  int v5; // [esp+10h] [ebp-24h]
  int v6; // [esp+14h] [ebp-20h] BYREF
  int v7; // [esp+18h] [ebp-1Ch]
  CPPEH_RECORD ms_exc; // [esp+1Ch] [ebp-18h]

  if ( lpMem )
  {
    if ( DAT_083bbf7c == 3 )
    {
      _lock(9);
      ms_exc.registration.TryLevel = 0;
      block = __sbh_find_block(lpMem);
      v7 = block;
      if ( block )
      {
        FUN_0054b3f4(block, lpMem);
      }
      ms_exc.registration.TryLevel = -1;
      _unlock(9);
      v2 = v7 == 0;
    }
    else
    {
      if ( DAT_083bbf7c != 2 )
      {
LABEL_11:
        HeapFree(hHeap, 0, lpMem);
        return;
      }
      _lock(9);
      ms_exc.registration.TryLevel = 1;
      v3 = FUN_0054c124(lpMem, &v4, &v6);
      v5 = v3;
      if ( v3 )
      {
        FUN_0054c17b(v4, v6, v3);
      }
      ms_exc.registration.TryLevel = -1;
      _unlock(9);
      v2 = v5 == 0;
    }
    if ( !v2 )
    {
      return;
    }
    goto LABEL_11;
  }
}
#endif

// ── FUN_00543d81 (IDA-only, gated) ──
#if defined(IDA_PORT_00543D81)
FILE *__cdecl FUN_00543d81()
{
  FILE *v0; // esi
  int v1; // ebp
  char *v2; // eax
  int v3; // eax

  _lock(3);
  if ( FileName )
  {
    if ( genfname((unsigned char *)&FileName) )
    {
      goto LABEL_13;
    }
  }
  else
  {
    init_namebuf(1);
  }
  v0 = (FILE *)_getstream();
  if ( v0 )
  {
    v1 = _sopen(&FileName, 34114, 64, 384);
    if ( v1 != -1 )
    {
      goto LABEL_10;
    }
    do
    {
      if ( *_errno() != 17 )
      {
        break;
      }
      if ( genfname((unsigned char *)&FileName) )
      {
        break;
      }
      v1 = _sopen(&FileName, 34114, 64, 384);
    }
    while ( v1 == -1 );
    if ( v1 != -1 )
    {
LABEL_10:
      v2 = _strdup(&FileName);
      v0->_tmpfname = v2;
      if ( v2 )
      {
        v0->_cnt = 0;
        v0->_ptr = 0;
        v0->_base = 0;
        v3 = DAT_083bbe30;
        (BYTE)(v3) = DAT_083bbe30 | 0x80;
        v0->_flag = v3;
        v0->_file = v1;
        _unlock_file(v0);
        goto LABEL_15;
      }
      _close(v1);
    }
    _unlock_file(v0);
  }
LABEL_13:
  v0 = 0;
LABEL_15:
  _unlock(3);
  return v0;
}
#endif

// ── FUN_00546a50 (IDA-only, gated) ──
#if defined(IDA_PORT_00546A50)
int __cdecl _stbuf(int a1)
{
  int v1; // eax
  int *v2; // edi
  void *v3; // eax
  int v4; // edi

  if ( !_isatty(*(DWORD *)(a1 + 16)) )
  {
    return 0;
  }
  if ( (FILE *)a1 == &File )
  {
    v1 = 0;
  }
  else
  {
    if ( (FILE *)a1 != &DAT_00563e00 )
    {
      return 0;
    }
    v1 = 1;
  }
  ++DAT_083bbbfc;
  if ( (*(WORD *)(a1 + 12) & 0x10C) != 0 )
  {
    return 0;
  }
  v2 = (int *)(4 * v1 + 138132512);
  if ( DAT_083bbc20[v1] || (v3 = malloc(0x1000u), (*v2 = (int)v3) != 0) )
  {
    v4 = *v2;
    *(DWORD *)(a1 + 24) = 4096;
    *(DWORD *)(a1 + 8) = v4;
    *(DWORD *)a1 = v4;
    *(DWORD *)(a1 + 4) = 4096;
  }
  else
  {
    *(DWORD *)(a1 + 8) = a1 + 20;
    *(DWORD *)a1 = a1 + 20;
    *(DWORD *)(a1 + 24) = 2;
    *(DWORD *)(a1 + 4) = 2;
  }
  *(WORD *)(a1 + 12) |= 0x1102u;
  return 1;
}
#endif

// ── FUN_00546add (IDA-only, gated) ──
#if defined(IDA_PORT_00546ADD)
int __cdecl _ftbuf(int a1, int a2)
{
  int result; // eax

  if ( a1 )
  {
    if ( (*(BYTE *)(a2 + 13) & 0x10) != 0 )
    {
      result = _flush((DWORD *)a2);
      *(BYTE *)(a2 + 13) &= 0xEEu;
      *(DWORD *)(a2 + 24) = 0;
      *(DWORD *)a2 = 0;
      *(DWORD *)(a2 + 8) = 0;
    }
  }
  return result;
}
#endif

// ── FUN_00549ae8 (IDA-only, gated) ──
#if defined(IDA_PORT_00549AE8)
int __cdecl _checkTOS_withFB(int a1, int a2)
{
  int result; // eax

  result = a2 & 0x7FF00000;
  if ( (a2 & 0x7FF00000) == 2146435072 )
  {
    return a2;
  }
  return result;
}
#endif

// ── FUN_005524b4 (IDA-only, gated) ──
#if defined(IDA_PORT_005524B4)
// attributes: thunk
void __stdcall FUN_005524b4(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif

// ── FUN_005524c8 (IDA-only, gated) ──
#if defined(IDA_PORT_005524C8)
// attributes: thunk
void __stdcall FUN_005524c8(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif

// ── FUN_00552568 (IDA-only, gated) ──
#if defined(IDA_PORT_00552568)
// attributes: thunk
void __stdcall FUN_00552568(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif

// ── FUN_00552574 (IDA-only, gated) ──
#if defined(IDA_PORT_00552574)
// attributes: thunk
void __stdcall FUN_00552574(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif

// ── FUN_00552588 (IDA-only, gated) ──
#if defined(IDA_PORT_00552588)
// attributes: thunk
void __stdcall FUN_00552588(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif

// ── FUN_00552668 (IDA-only, gated) ──
#if defined(IDA_PORT_00552668)
// attributes: thunk
void __stdcall FUN_00552668(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif

// ── FUN_00552760 (IDA-only, gated) ──
#if defined(IDA_PORT_00552760)
// attributes: thunk
void __stdcall FUN_00552760(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
