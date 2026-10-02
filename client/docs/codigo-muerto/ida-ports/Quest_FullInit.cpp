// 0x00403EA0 Quest_FullInit — nunca activado: IDA_PORT_00403EA0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Quest_FullInit (IDA-only, gated) ──
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
