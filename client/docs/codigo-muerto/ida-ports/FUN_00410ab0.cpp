// 0x00410AB0 FUN_00410ab0 — nunca activado: IDA_PORT_00410AB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410ab0 (IDA-only, gated) ──
#if defined(IDA_PORT_00410AB0)
LPVOID __cdecl FUN_00410ab0(LPVOID lpMem, char a2)
{
  FUN_00410ad0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
