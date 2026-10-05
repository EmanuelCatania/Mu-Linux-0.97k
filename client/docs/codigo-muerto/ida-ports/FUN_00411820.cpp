// 0x00411820 FUN_00411820 — nunca activado: IDA_PORT_00411820 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411820 (IDA-only, gated) ──
#if defined(IDA_PORT_00411820)
BYTE *__cdecl FUN_00411820(BYTE *_this, DWORD *a2, BYTE *a3)
{
  BYTE *result; // eax

  result = _this;
  *(DWORD *)_this = *a2;
  _this[4] = *a3;
  return result;
}
#endif
