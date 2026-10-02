// 0x00410E30 FUN_00410e30 — nunca activado: IDA_PORT_00410E30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410e30 (IDA-only, gated) ──
#if defined(IDA_PORT_00410E30)
DWORD *__cdecl FUN_00410e30(DWORD **_this, DWORD *a2, int a3)
{
  DWORD *v3; // edx
  DWORD *result; // eax

  v3 = *_this;
  *_this = (DWORD *)**_this;
  result = a2;
  *a2 = v3;
  return result;
}
#endif
