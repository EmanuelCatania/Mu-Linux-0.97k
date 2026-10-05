// 0x004127D0 FUN_004127d0 — nunca activado: IDA_PORT_004127D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004127d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004127D0)
// Microsoft VisualC 2-14/net runtime
DWORD *FUN_004127d0()
{
  g_ErrorReport = (DWORD)DAT_005524c4;
  FUN_00405290((int)&g_ErrorReport);
  FUN_004052b0((int)&g_ErrorReport, aMuerrorLog);
  return &g_ErrorReport;
}
#endif
