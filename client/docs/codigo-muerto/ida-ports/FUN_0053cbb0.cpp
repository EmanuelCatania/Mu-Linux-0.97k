// 0x0053CBB0 FUN_0053cbb0 — nunca activado: IDA_PORT_0053CBB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053cbb0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CBB0)
DWORD *__cdecl FUN_0053cbb0(DWORD *_this)
{
  *_this = &DAT_0055389c;
  FUN_0053cc00();
  return _this;
}
#endif
