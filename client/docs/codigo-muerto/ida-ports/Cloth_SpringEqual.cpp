// 0x00407C60 Cloth_SpringEqual — nunca activado: IDA_PORT_00407C60 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Cloth_SpringEqual (IDA-only, gated) ──
#if defined(IDA_PORT_00407C60)
int __cdecl Cloth_SpringEqual(int a1, double a2, int a3, float a4)
{
  double v5; // st7
  double v6; // st7
  int result; // eax
  float v8; // [esp+8h] [ebp-Ch] BYREF
  float v9; // [esp+Ch] [ebp-8h]
  float v10; // [esp+10h] [ebp-4h]

  SpringNode_Delta((char *)a1, a3, (int)&v8);
  if ( a2 >= 0.001 )
  {
    SpringNode_Delta((char *)a1, a3, (int)&v8);
  }
  else
  {
    a2 = 0.001;
  }
  v5 = (a2 - a4) * 0.5 / a2;
  v8 = v5 * v8;
  v9 = v5 * v9;
  v6 = v5 * v10;
  *(float *)(a1 + 48) = *(float *)(a1 + 48) - v8;
  *(float *)(a1 + 52) = *(float *)(a1 + 52) - v9;
  *(float *)(a1 + 56) = *(float *)(a1 + 56) - v6;
  *(float *)(a3 + 48) = v8 + *(float *)(a3 + 48);
  *(float *)(a3 + 52) = v9 + *(float *)(a3 + 52);
  *(float *)(a3 + 56) = v6 + *(float *)(a3 + 56);
  ++*(DWORD *)(a1 + 44);
  result = *(DWORD *)(a3 + 44) + 1;
  *(DWORD *)(a3 + 44) = result;
  return result;
}
#endif
