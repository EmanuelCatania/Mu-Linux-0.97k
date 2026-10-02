// 0x0040C670 FUN_0040c670 — nunca activado: IDA_PORT_0040C670 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c670 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C670)
int __cdecl FUN_0040c670(DWORD *_this, int a2)
{
  int result; // eax

  result = a2;
  _this[9] = a2;
  return result;
}
#endif
