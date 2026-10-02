// 0x0040E230 FUN_0040e230 — nunca activado: IDA_PORT_0040E230 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
