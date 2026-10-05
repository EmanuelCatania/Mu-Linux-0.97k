// 0x00406CD0 FUN_00406cd0 — nunca activado: IDA_PORT_00406CD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
