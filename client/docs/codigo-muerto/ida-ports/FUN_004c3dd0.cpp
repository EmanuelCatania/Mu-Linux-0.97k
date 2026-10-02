// 0x004C3DD0 FUN_004c3dd0 — nunca activado: IDA_PORT_004C3DD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004c3dd0 (IDA-only, gated) ──
#if defined(IDA_PORT_004C3DD0)
int __cdecl FUN_004c3dd0(int a1)
{
  if ( a1 >= (int)&DAT_00989680 )
  {
    return -16776961;
  }
  if ( a1 < 1000000 )
  {
    return a1 < 100000 ? -6890241 : -15152896;
  }
  return -16738561;
}
#endif
