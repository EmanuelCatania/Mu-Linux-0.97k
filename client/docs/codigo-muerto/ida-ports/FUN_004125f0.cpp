// 0x004125F0 FUN_004125f0 — nunca activado: IDA_PORT_004125F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004125f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004125F0)
LPVOID __cdecl FUN_004125f0(LPVOID lpMem, char a2)
{
  FUN_00412610();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
