// 0x00410E40 FUN_00410e40 — nunca activado: IDA_PORT_00410E40 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410e40 (IDA-only, gated) ──
#if defined(IDA_PORT_00410E40)
DWORD *__cdecl FUN_00410e40(DWORD **_this, DWORD *a2)
{
  DWORD *result; // eax

  result = a2;
  *a2 = *_this[1];
  return result;
}
#endif
