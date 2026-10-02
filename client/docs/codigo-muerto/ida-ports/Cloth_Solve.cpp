// 0x00408E30 Cloth_Solve — nunca activado: IDA_PORT_00408E30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Cloth_Solve (IDA-only, gated) ──
#if defined(IDA_PORT_00408E30)
int __cdecl Cloth_Solve(DWORD *a1, double a2)
{
  int v3; // ebx
  int i; // ebp
  DWORD *v5; // edi
  float v6; // eax
  int v7; // ebp
  int v8; // edi
  float *v9; // eax
  int j; // ebp
  int v11; // eax
  int k; // edi
  float *v13; // eax
  int v14; // edi
  int v16; // [esp+10h] [ebp-28h]
  float Position[3]; // [esp+14h] [ebp-24h] BYREF
  float v18; // [esp+20h] [ebp-18h]
  float v19; // [esp+24h] [ebp-14h]
  float v20; // [esp+28h] [ebp-10h]
  float WorldPosition[3]; // [esp+2Ch] [ebp-Ch] BYREF

  v3 = 0;
  for ( i = *(DWORD *)(a1[19] + 8); a1[20] != i && i; i = *(DWORD *)(i + 8) )
  {
    v5 = *(DWORD **)i;
    VerletNode_GetPos(*(DWORD **)i, Position);
    v19 = Position[1];
    v18 = Position[0];
    v6 = Position[0];
    v20 = Position[2];
    a2 = -Position[1];
    Position[1] = a2;
    Position[0] = Position[2];
    Position[2] = v6;
    TransformPosition(
      Models + 188 * *(short *)(a1[1] + 2),
      (float (*)[4])(*(DWORD *)(a1[1] + 276) + 48 * v5[4]),
      Position,
      WorldPosition,
      1);
    VerletNode_SetTarget(v5, SLODWORD(WorldPosition[0]), SLODWORD(WorldPosition[1]), SLODWORD(WorldPosition[2]));
  }
  Cloth_CollideAnchors(a1);
  v7 = 0;
  if ( (int)a1[14] > 0 )
  {
    v8 = 0;
    do
    {
      v9 = (float *)(a1[15] + v8);
      if ( ((BYTE)v9[3] & 1) != 0 )
      {
        Cloth_SpringEqual(a1[13] + 60 * *(short *)v9, a2, a1[13] + 60 * *((short *)v9 + 1), v9[2]);
      }
      ++v7;
      v8 += 16;
    }
    while ( v7 < a1[14] );
  }
  for ( j = 0; j < a1[11]; ++j )
  {
    v11 = a1[10];
    for ( k = 0; k < v11; ++k )
    {
      VerletSystem_Flush(a1[13] + 60 * (k + j * v11));
      v11 = a1[10];
    }
  }
  v16 = 0;
  if ( (int)a1[14] <= 0 )
  {
    return 1;
  }
  while ( 1 )
  {
    v13 = (float *)(a1[15] + v3);
    v14 = *((short *)v13 + 1);
    if ( v14 >= a1[10]
      && ((BYTE)v13[3] & 4) != 0
      && !Cloth_SpringRange(a1[13] + 60 * v14, a2, a1[13] + 60 * *(short *)v13, v13 + 1) )
    {
      break;
    }
    v3 += 16;
    if ( ++v16 >= a1[14] )
    {
      return 1;
    }
  }
  return 0;
}
#endif
