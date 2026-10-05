// 0x0040DEF0 FUN_0040def0 — nunca activado: IDA_PORT_0040DEF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
