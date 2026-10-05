// 0x004FA5A0 ClearActionObject — nunca activado: IDA_PORT_004FA5A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ClearActionObject (IDA-only, gated) ──
#if defined(IDA_PORT_004FA5A0)
int ClearActionObject()
{
  int result; // eax

  result = -1;
  DAT_0055a7bc = -1.0;
  DAT_0055a7b0 = -1;
  DAT_0055a7b4 = -1;
  DAT_0055a7b8 = -1;
  return result;
}
#endif
