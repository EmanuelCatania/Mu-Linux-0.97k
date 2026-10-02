// 0x0040F520 FUN_0040f520 — nunca activado: IDA_PORT_0040F520 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040f520 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F520)
DWORD *__cdecl FUN_0040f520(DWORD *lpMem, char a2)
{
  FUN_0040f540(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
