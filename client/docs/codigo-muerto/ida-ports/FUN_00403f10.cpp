// 0x00403F10 FUN_00403f10 — nunca activado: IDA_PORT_00403F10 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00403f10 (IDA-only, gated) ──
#if defined(IDA_PORT_00403F10)
DWORD *__cdecl FUN_00403f10(DWORD *lpMem, char a2)
{
  FUN_00403ef0(lpMem);
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
