// 0x00405260 FUN_00405260 — nunca activado: IDA_PORT_00405260 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00405260 (IDA-only, gated) ──
#if defined(IDA_PORT_00405260)
LPVOID __cdecl FUN_00405260(LPVOID lpMem, char a2)
{
  FUN_00405280();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
