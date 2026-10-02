// 0x0053CF90 CSimpleModulus_AddBits — nunca activado: IDA_PORT_0053CF90 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CSimpleModulus_AddBits (IDA-only, gated) ──
#if defined(IDA_PORT_0053CF90)
int __stdcall CSimpleModulus_AddBits(int a1, int a2, int a3, int a4, int a5)
{
  int v5; // ebx
  unsigned int v6; // ebx
  int v7; // eax
  int v8; // esi
  BYTE *v9; // eax
  int v10; // ecx
  char *lpMem; // [esp+10h] [ebp-8h]

  v5 = CSimpleModulus_GetByteOfBit(a4 + a5 - 1);
  v6 = 1 - CSimpleModulus_GetByteOfBit(a4) + v5;
  lpMem = (char *)operator_new(v6 + 1);
  memset(lpMem, 0, v6 + 1);
  qmemcpy(lpMem, (const void *)(a3 + CSimpleModulus_GetByteOfBit(a4)), v6);
  v7 = (a4 + a5) % 8;
  if ( v7 )
  {
    lpMem[v6 - 1] &= -1 << (8 - v7);
  }
  CSimpleModulus_Shift(lpMem, v6, -(a4 % 8));
  CSimpleModulus_Shift(lpMem, v6 + 1, a2 % 8);
  v8 = v6 + (a2 % 8 > a4 % 8);
  v9 = (BYTE *)(a1 + CSimpleModulus_GetByteOfBit(a2));
  if ( v8 > 0 )
  {
    v10 = lpMem - v9;
    do
    {
      *v9 |= v9[v10];
      ++v9;
      --v8;
    }
    while ( v8 );
  }
  delete__(lpMem);
  return a2 + a5;
}
#endif
