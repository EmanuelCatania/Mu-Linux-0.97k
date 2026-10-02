// 0x005433CD FUN_005433cd — nunca activado: IDA_PORT_005433CD nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005433cd (IDA-only, gated) ──
#if defined(IDA_PORT_005433CD)
double __cdecl _CIsin(double x)
{
  int v2; // [esp+0h] [ebp-8h]

  _checkTOS_withFB(LODWORD(x), HIDWORD(*(unsigned __int64 *)&x));
  return FUN_005433cd(v2);
}
#endif
