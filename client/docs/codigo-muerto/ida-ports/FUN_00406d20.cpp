// 0x00406D20 FUN_00406d20 — nunca activado: IDA_PORT_00406D20 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
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
