// 0x00406EC0 FUN_00406ec0 — nunca activado: IDA_PORT_00406EC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406ec0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406EC0)
DWORD *__cdecl FUN_00406ec0(DWORD *lpMem, char a2)
{
  lpMem[3] = 0;
  lpMem[1] = 0;
  lpMem[2] = 0;
  *lpMem = &DAT_005524d8;
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
