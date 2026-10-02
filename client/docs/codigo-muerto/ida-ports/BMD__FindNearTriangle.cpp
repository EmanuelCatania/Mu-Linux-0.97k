// 0x00442260 BMD__FindNearTriangle — nunca activado: IDA_PORT_00442260 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── BMD__FindNearTriangle (IDA-only, gated) ──
#if defined(IDA_PORT_00442260)
void __cdecl BMD::FindNearTriangle(DWORD This)
{
  int v2; // ebx
  int v3; // eax
  int v4; // ebp
  int v5; // eax
  int v6; // ecx
  int v7; // esi
  int i; // esi
  int v9; // [esp+8h] [ebp-4h]

  v2 = 0;
  if ( *(short *)(This + 36) > 0 )
  {
    v9 = 0;
    do
    {
      v3 = v9 + *(DWORD *)(This + 40);
      v4 = *(short *)(v3 + 10);
      if ( v4 > 0 )
      {
        v5 = *(DWORD *)(v3 + 28) + 26;
        v6 = v4;
        do
        {
          v7 = v5;
          v5 += 36;
          --v6;
          *(DWORD *)v7 = -1;
          *(WORD *)(v7 + 4) = -1;
        }
        while ( v6 );
      }
      for ( i = 0; i < v4; ++i )
      {
        BMD::FindTriangleForEdge(This, v2, i, 0);
        BMD::FindTriangleForEdge(This, v2, i, 1);
        BMD::FindTriangleForEdge(This, v2, i, 2);
      }
      ++v2;
      v9 += 40;
    }
    while ( v2 < *(short *)(This + 36) );
  }
}
#endif
