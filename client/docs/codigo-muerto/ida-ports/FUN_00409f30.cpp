// 0x00409F30 FUN_00409f30 — nunca activado: IDA_PORT_00409F30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00409f30 (IDA-only, gated) ──
#if defined(IDA_PORT_00409F30)
int __cdecl FUN_00409f30(int a1, int a2, int a3, int a4, int a5, char a6)
{
  short v6; // cx
  int v7; // esi
  int v8; // eax
  int v9; // ebp
  int v10; // edx
  int v11; // ecx
  int v12; // eax
  int v13; // esi
  int v14; // ebp
  short v17; // [esp+8h] [ebp-4h]
  int v18; // [esp+18h] [ebp+Ch]

  if ( *(float *)(a5 + 360) < 0.0099999998 )
  {
    return 0;
  }
  (WORD)(a2) = *(WORD *)(a5 + 88);
  v6 = *(WORD *)(a5 + 100);
  v18 = a2;
  v17 = v6;
  if ( (WORD)a2 == 0xFFFE || v6 == -2 )
  {
    return 0;
  }
  v7 = 0;
  v8 = 0;
  v9 = *(short *)(a4 + 36);
  if ( v9 > 0 )
  {
    v10 = 0;
    do
    {
      if ( (short)a2 != v8
        && v17 != v8
        && (Bitmaps[*(short *)(*(DWORD *)(a4 + 56) + 2 * v8)].Components != 4 || !a6) )
      {
        (WORD)(v11) = *(WORD *)(*(DWORD *)(a4 + 40) + v10 + 10);
        if ( (v11 & 0x8000u) == 0 )
        {
          v11 = (short)v11;
        }
        else
        {
          v11 = 0;
        }
        v7 += v11;
      }
      (WORD)(a2) = v18;
      ++v8;
      v10 += 40;
    }
    while ( v8 < v9 );
  }
  *(DWORD *)(a1 + 24) = 0;
  v12 = operator_new(30 * v7);
  v13 = 0;
  *(DWORD *)(a1 + 28) = v12;
  if ( *(short *)(a4 + 36) > 0 )
  {
    v14 = 0;
    do
    {
      if ( (short)a2 != v13 && v17 != v13 )
      {
        (BYTE)(v18) = 0;
        if ( Bitmaps[*(short *)(*(DWORD *)(a4 + 56) + 2 * v13)].Components != 4 || ((BYTE)(v18) = 1, !a6) )
        {
          FUN_0040a1c0(
            v13,
            a3,
            *(WORD *)(v14 + *(DWORD *)(a4 + 40) + 10),
            *(DWORD *)(v14 + *(DWORD *)(a4 + 40) + 28),
            v18);
        }
      }
      ++v13;
      v14 += 40;
    }
    while ( v13 < *(short *)(a4 + 36) );
  }
  return 1;
}
#endif
