// 0x00407B90 Cloth_SpringRange — nunca activado: IDA_PORT_00407B90 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Cloth_SpringRange (IDA-only, gated) ──
#if defined(IDA_PORT_00407B90)
int __cdecl Cloth_SpringRange(int a1, double a2, int a3, float *a4)
{
  double v6; // st6
  double v7; // st7
  double v8; // st7
  float v9; // [esp+4h] [ebp-Ch] BYREF
  float v10; // [esp+8h] [ebp-8h]
  float v11; // [esp+Ch] [ebp-4h]

  if ( (*(BYTE *)(a1 + 40) & 1) != 0 )
  {
    return 1;
  }
  SpringNode_Delta((char *)a1, a3, (int)&v9);
  if ( a2 >= 0.001 )
  {
    SpringNode_Delta((char *)a1, a3, (int)&v9);
  }
  else
  {
    a2 = 0.001;
  }
  if ( a2 > a4[1] * 20.0 )
  {
    return 0;
  }
  if ( a2 > a4[1] )
  {
    v6 = a2 - a4[1];
LABEL_11:
    v7 = v6 / a2;
    v9 = v7 * v9;
    v10 = v7 * v10;
    v8 = v7 * v11;
    *(float *)(a1 + 28) = *(float *)(a1 + 28) - v9;
    *(float *)(a1 + 32) = *(float *)(a1 + 32) - v10;
    *(float *)(a1 + 36) = *(float *)(a1 + 36) - v8;
    return 1;
  }
  if ( a2 < *a4 )
  {
    v6 = a2 - *a4;
    goto LABEL_11;
  }
  return 1;
}
#endif
