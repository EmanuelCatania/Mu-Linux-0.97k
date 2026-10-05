// 0x0047D000 FUN_0047d000 — nunca activado: IDA_PORT_0047D000 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0047d000 (IDA-only, gated) ──
#if defined(IDA_PORT_0047D000)
short __cdecl FUN_0047d000(WORD *a1)
{
  if ( *a1 == 0xFFFF )
  {
    return 0;
  }
  else
  {
    return a1[12];
  }
}
#endif
