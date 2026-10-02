// 0x0053CBD0 FUN_0053cbd0 — nunca activado: IDA_PORT_0053CBD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053cbd0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CBD0)
LPVOID __cdecl FUN_0053cbd0(LPVOID lpMem, char a2)
{
  FUN_0053cbf0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
