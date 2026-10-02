// 0x0040EAC0 FUN_0040eac0 — nunca activado: IDA_PORT_0040EAC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040eac0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040EAC0)
char *__cdecl FUN_0040eac0(char *lpMem, char a2)
{
  FUN_0040eae0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
