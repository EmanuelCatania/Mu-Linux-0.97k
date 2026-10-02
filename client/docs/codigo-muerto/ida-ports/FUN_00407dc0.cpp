// 0x00407DC0 FUN_00407dc0 — nunca activado: IDA_PORT_00407DC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00407dc0 (IDA-only, gated) ──
#if defined(IDA_PORT_00407DC0)
LPVOID __cdecl FUN_00407dc0(LPVOID lpMem, char a2)
{
  FUN_00407de0();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
