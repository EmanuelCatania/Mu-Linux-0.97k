// 0x0040F540 FUN_0040f540 — nunca activado: IDA_PORT_0040F540 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040f540 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F540)
DWORD *__cdecl FUN_0040f540(DWORD *_this)
{
  DWORD *result; // eax
  int v2; // ecx

  result = _this;
  v2 = _this[1];
  *result = &DAT_005527e0;
  if ( v2 )
  {
    return (DWORD *)(*(int (__cdecl **)(int, int))(*(DWORD *)v2 + 20))(v2, 1);
  }
  return result;
}
#endif
