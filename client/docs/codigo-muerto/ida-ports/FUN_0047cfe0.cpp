// 0x0047CFE0 FUN_0047cfe0 — nunca activado: IDA_PORT_0047CFE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0047cfe0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047CFE0)
short __cdecl FUN_0047cfe0(WORD *a1)
{
  if ( *a1 == 0xFFFF )
  {
    return 0;
  }
  else
  {
    return a1[10];
  }
}
#endif
