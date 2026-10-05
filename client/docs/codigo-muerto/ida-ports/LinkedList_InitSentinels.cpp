// 0x0040A600 LinkedList_InitSentinels — nunca activado: IDA_PORT_0040A600 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── LinkedList_InitSentinels (IDA-only, gated) ──
#if defined(IDA_PORT_0040A600)
int __cdecl FUN_0040a300(int _this, int a2)
{
  int v3; // ebp
  int v4; // eax
  int result; // eax
  int v6; // ebx
  short *v7; // eax
  double v8; // st7
  double v9; // st7
  float scale; // [esp+0h] [ebp-4Ch]
  float scalea; // [esp+0h] [ebp-4Ch]
  float va[3]; // [esp+1Ch] [ebp-30h] BYREF
  float v13[3]; // [esp+28h] [ebp-24h] BYREF
  float vc[3]; // [esp+34h] [ebp-18h] BYREF
  float v15[3]; // [esp+40h] [ebp-Ch] BYREF

  v3 = 0;
  v4 = *(DWORD *)(_this + 24);
  *(WORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = operator_new(72 * v4);
  result = *(DWORD *)(_this + 24);
  if ( result > 0 )
  {
    v6 = 0;
    do
    {
      v7 = (short *)(v6 + *(DWORD *)(_this + 28));
      va[0] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]));
      va[1] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]) + 4);
      va[2] = *(float *)(a2 + 12 * (*v7 + 15000 * v7[2]) + 8);
      v13[0] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]));
      v13[1] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]) + 4);
      v13[2] = *(float *)(a2 + 12 * (v7[1] + 15000 * v7[2]) + 8);
      if ( va[2] >= 22.5 )
      {
        v8 = va[2];
      }
      else
      {
        v8 = 22.5;
      }
      scale = -(v8 / *(float *)(_this + 20));
      VectorMA(va, scale, (float *)(_this + 12), vc);
      if ( v13[2] >= 22.5 )
      {
        v9 = v13[2];
      }
      else
      {
        v9 = 22.5;
      }
      scalea = -(v9 / *(float *)(_this + 20));
      VectorMA(v13, scalea, (float *)(_this + 12), v15);
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = va[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = va[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = va[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = vc[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = vc[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = vc[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v13[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v13[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v13[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v13[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v13[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v13[2];
      v6 += 10;
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = vc[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = vc[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = vc[2];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4)) = v15[0];
      *(float *)(*(DWORD *)(_this + 8) + 12 * *(short *)(_this + 4) + 4) = v15[1];
      *(float *)(*(DWORD *)(_this + 8) + 12 * (short)(*(WORD *)(_this + 4))++ + 8) = v15[2];
      result = *(DWORD *)(_this + 24);
      ++v3;
    }
    while ( v3 < result );
  }
  return result;
}
#endif
