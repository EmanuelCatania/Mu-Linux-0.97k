// 0x00412700 FUN_00412700 — nunca activado: IDA_PORT_00412700 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00412700 (IDA-only, gated) ──
#if defined(IDA_PORT_00412700)
int FUN_00412700()
{
  return FUN_0053d430(aMu);
}
#endif
