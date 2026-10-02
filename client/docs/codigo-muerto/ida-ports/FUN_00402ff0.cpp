// 0x00402FF0 FUN_00402ff0 — nunca activado: IDA_PORT_00402FF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
