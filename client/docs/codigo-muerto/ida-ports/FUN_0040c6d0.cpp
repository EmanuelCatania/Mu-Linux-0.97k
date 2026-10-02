// 0x0040C6D0 FUN_0040c6d0 — nunca activado: IDA_PORT_0040C6D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c6d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C6D0)
int __cdecl FUN_0040c6d0(DWORD *_this, int a2, int a3, int a4)
{
  int result; // eax

  _this[15] = a2;
  result = a4;
  _this[17] = a3;
  _this[18] = a4;
  return result;
}
#endif
