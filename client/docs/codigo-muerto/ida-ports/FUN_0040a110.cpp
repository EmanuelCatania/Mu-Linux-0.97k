// 0x0040A110 FUN_0040a110 — nunca activado: IDA_PORT_0040A110 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a110 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A110)
int __cdecl FUN_0040a110(DWORD *_this, short a2, short a3, short a4, int a5, int a6, int a7)
{
  int v7; // esi
  short v8; // ax
  int result; // eax

  v7 = a7 + 36 * a5;
  v8 = *(WORD *)(v7 + 2 * a6 + 26);
  if ( v8 == -1 || (result = 9 * v8, !*(BYTE *)(a7 + 4 * result + 34)) )
  {
    *(WORD *)(_this[7] + 10 * _this[6]) = a2;
    *(WORD *)(_this[7] + 10 * _this[6] + 2) = a3;
    *(WORD *)(_this[7] + 10 * _this[6] + 4) = a4;
    *(WORD *)(_this[7] + 10 * _this[6] + 6) = *(WORD *)(v7 + 2 * a6 + 10);
    *(WORD *)(_this[7] + 10 * _this[6] + 8) = *(WORD *)(v7 + 2 * ((a6 + 1) % 3) + 10);
    result = _this[6] + 1;
    _this[6] = result;
  }
  return result;
}
#endif
