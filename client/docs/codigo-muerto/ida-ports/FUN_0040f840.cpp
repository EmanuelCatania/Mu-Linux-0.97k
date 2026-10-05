// 0x0040F840 FUN_0040f840 — nunca activado: IDA_PORT_0040F840 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040f840 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F840)
LPVOID __cdecl FUN_0040f840(LPVOID lpMem, char a2)
{
  FUN_0040f950();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
