// 0x0040C6B0 FUN_0040c6b0 — nunca activado: IDA_PORT_0040C6B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c6b0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C6B0)
int __cdecl FUN_0040c6b0(DWORD *_this, int a2, int a3)
{
  int result; // eax

  result = a2;
  _this[13] = a2;
  _this[14] = a3;
  return result;
}
#endif
