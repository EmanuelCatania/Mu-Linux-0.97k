// 0x00408CB0 Cloth_Integrate — nunca activado: IDA_PORT_00408CB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Cloth_Integrate (IDA-only, gated) ──
#if defined(IDA_PORT_00408CB0)
int __cdecl Cloth_Integrate(DWORD *a1, double a2, float a3)
{
  int v4; // esi
  float *v5; // esi
  int v6; // eax
  char *v7; // ebp
  float *v8; // ebx
  double v9; // st6
  int v10; // ecx
  int i; // eax
  double v12; // st5
  bool v13; // cc
  int result; // eax
  int v15; // esi
  int v16; // ebx
  float v17; // [esp+0h] [ebp-3Ch]
  float v18; // [esp+4h] [ebp-38h]
  float v19; // [esp+8h] [ebp-34h]
  int v20; // [esp+1Ch] [ebp-20h]
  int v21; // [esp+20h] [ebp-1Ch]
  float v22; // [esp+24h] [ebp-18h]
  float v23; // [esp+28h] [ebp-14h]
  float v24; // [esp+2Ch] [ebp-10h]
  float v25[3]; // [esp+30h] [ebp-Ch] BYREF

  (*(void (__cdecl **)(DWORD *))(*a1 + 8))(a1);
  v4 = 0;
  v20 = 0;
  if ( (int)a1[14] > 0 )
  {
    v21 = 0;
    do
    {
      v5 = (float *)(a1[15] + v4);
      if ( ((BYTE)v5[3] & 2) != 0 )
      {
        v6 = a1[13];
        v7 = (char *)(v6 + 60 * *(short *)v5);
        v8 = (float *)(v6 + 60 * *((short *)v5 + 1));
        SpringNode_Delta(v7, (int)v8, (int)v25);
        if ( a2 >= 0.001 )
        {
          SpringNode_Delta(v7, (int)v8, (int)v25);
        }
        else
        {
          a2 = 0.001;
        }
        if ( a2 > v5[2] + 0.0099999998 )
        {
          v9 = a2 - v5[2];
          v10 = a1[5] & 0x300;
          for ( i = 0; i < 3; ++i )
          {
            v12 = v9 * v25[i] / a2;
            *(float *)((char *)&v22 + i * 4) = v12;
            if ( v10 == 256 )
            {
              *(float *)((char *)&v22 + i * 4) = v12 * 3.0;
            }
          }
          v19 = -v24;
          v18 = -v23;
          a2 = -v22;
          v17 = a2;
          VerletNode_AddAccel((float *)v7, v17, v18, v19);
          VerletNode_AddAccel(v8, v22, v23, v24);
        }
      }
      v4 = v21 + 16;
      v13 = ++v20 < a1[14];
      v21 += 16;
    }
    while ( v13 );
  }
  (*(void (__cdecl **)(DWORD *, int))(*a1 + 4))(a1, 48 * a1[2] + *(DWORD *)(a1[1] + 276));
  result = a1[12];
  v15 = 0;
  if ( result > 0 )
  {
    v16 = 0;
    do
    {
      ClothNode_Integrate((float *)(v16 + a1[13]), a3);
      result = a1[12];
      ++v15;
      v16 += 60;
    }
    while ( v15 < result );
  }
  return result;
}
#endif
