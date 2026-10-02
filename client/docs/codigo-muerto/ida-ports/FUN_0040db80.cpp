// 0x0040DB80 FUN_0040db80 — nunca activado: IDA_PORT_0040DB80 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040db80 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DB80)
char *__cdecl FUN_0040db80(char *lpMem, char a2)
{
  FUN_0040dba0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
