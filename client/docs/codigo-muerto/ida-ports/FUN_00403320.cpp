// 0x00403320 FUN_00403320 — nunca activado: IDA_PORT_00403320 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
