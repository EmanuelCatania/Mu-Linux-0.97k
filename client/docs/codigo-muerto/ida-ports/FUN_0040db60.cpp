// 0x0040DB60 FUN_0040db60 — nunca activado: IDA_PORT_0040DB60 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040db60 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DB60)
LPVOID __cdecl FUN_0040db60(LPVOID lpMem, char a2)
{
  FUN_0040d550(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
