// 0x00407EF0 ClothAnchor_SetParams — nunca activado: IDA_PORT_00407EF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ClothAnchor_SetParams (IDA-only, gated) ──
#if defined(IDA_PORT_00407EF0)
int __cdecl ClothAnchor_SetParams(DWORD *_this, int a2, int a3, int a4, int a5, int a6)
{
  int result; // eax

  _this[1] = a2;
  _this[2] = a3;
  _this[3] = a4;
  result = a6;
  _this[8] = a5;
  _this[4] = a6;
  return result;
}
#endif
