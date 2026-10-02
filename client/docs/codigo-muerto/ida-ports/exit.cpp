// 0x00543839 exit — nunca activado: IDA_PORT_00543839 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── exit (IDA-only, gated) ──
#if defined(IDA_PORT_00543839)
void __cdecl  exit(int Code)
{
  doexit(Code, 0, 0);
}
#endif
