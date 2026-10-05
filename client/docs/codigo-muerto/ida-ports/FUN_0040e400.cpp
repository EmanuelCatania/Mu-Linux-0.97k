// 0x0040E400 FUN_0040e400 — nunca activado: IDA_PORT_0040E400 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
      ChatListBox_ScrollByN(_this);
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
