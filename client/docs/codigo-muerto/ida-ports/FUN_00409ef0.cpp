// 0x00409EF0 FUN_00409ef0 — nunca activado: IDA_PORT_00409EF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00409ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409EF0)
LPVOID __cdecl FUN_00409ef0(LPVOID lpMem, char a2)
{
  WidgetB_SetVtable();
  if ( (a2 & 1) != 0 )
  {
    delete__(lpMem);
  }
  return lpMem;
}
#endif
