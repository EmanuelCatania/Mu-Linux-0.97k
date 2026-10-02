// 0x00423C30 FUN_00423c30 — nunca activado: IDA_PORT_00423C30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00423c30 (IDA-only, gated) ──
#if defined(IDA_PORT_00423C30)
// Microsoft VisualC 2-14/net runtime
BOOL FUN_00423c30()
{
  return CWsctlc::Close((DWORD)&SocketClient);
}
#endif
