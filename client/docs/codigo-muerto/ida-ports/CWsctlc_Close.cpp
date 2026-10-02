// 0x0043DC90 CWsctlc_Close — nunca activado: IDA_PORT_0043DC90 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CWsctlc_Close (IDA-only, gated) ──
#if defined(IDA_PORT_0043DC90)
BOOL __cdecl CWsctlc::Close(DWORD This)
{
  g_bGameServerConnected = 0;
  closesocket(*(DWORD *)(This + 8));
  *(DWORD *)(This + 8) = -1;
  return 1;
}
#endif
