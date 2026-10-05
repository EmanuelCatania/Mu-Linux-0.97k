// 0x00403700 FUN_00403700 — nunca activado: IDA_PORT_00403700 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
