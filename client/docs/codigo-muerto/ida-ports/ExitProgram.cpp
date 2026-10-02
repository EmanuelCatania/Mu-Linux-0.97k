// 0x004F6CB0 ExitProgram — nunca activado: IDA_PORT_004F6CB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ExitProgram (IDA-only, gated) ──
#if defined(IDA_PORT_004F6CB0)
void __cdecl ExitProgram()
{
  MessageBoxA(g_hWnd, GlobalText[11], 0, 0);
  SendMessageA(g_hWnd, 2u, 0, 0);
}
#endif
