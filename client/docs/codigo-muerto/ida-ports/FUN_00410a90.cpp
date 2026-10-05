// 0x00410A90 FUN_00410a90 — nunca activado: IDA_PORT_00410A90 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410a90 (IDA-only, gated) ──
#if defined(IDA_PORT_00410A90)
DWORD *__cdecl FUN_00410a90(DWORD *_this)
{
  FUN_0040f680(_this);
  *_this = &DAT_00552810;
  return _this;
}
#endif
