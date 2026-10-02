// 0x00403EF0 FUN_00403ef0 — nunca activado: IDA_PORT_00403EF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00403ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00403EF0)
void __cdecl FUN_00403ef0(DWORD *_this)
{
  *_this = &DAT_005524b8;
  g_csQuest = 0;
}
#endif
