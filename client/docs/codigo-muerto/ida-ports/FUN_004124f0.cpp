// 0x004124F0 FUN_004124f0 — nunca activado: IDA_PORT_004124F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004124f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004124F0)
DWORD *__cdecl FUN_004124f0(DWORD *lpMem, char a2)
{
  FUN_00412510(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
