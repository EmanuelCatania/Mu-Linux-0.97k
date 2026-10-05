// 0x0040C6F0 FUN_0040c6f0 — nunca activado: IDA_PORT_0040C6F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c6f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C6F0)
int __cdecl FUN_0040c6f0(DWORD *_this, int a2, int a3, int a4)
{
  int result; // eax

  _this[16] = a2;
  result = a4;
  _this[19] = a3;
  _this[20] = a4;
  return result;
}
#endif
