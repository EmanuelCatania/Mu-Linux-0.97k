// 0x0040A6C0 FUN_0040a6c0 — nunca activado: IDA_PORT_0040A6C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a6c0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A6C0)
LPVOID __cdecl FUN_0040a6c0(LPVOID lpMem, char a2)
{
  FUN_0040a6e0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
