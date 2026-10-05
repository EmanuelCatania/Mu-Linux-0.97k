// 0x004017E0 FUN_004017e0 — nunca activado: IDA_PORT_004017E0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
