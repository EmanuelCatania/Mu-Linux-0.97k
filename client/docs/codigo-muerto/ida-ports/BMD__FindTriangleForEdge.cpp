// 0x004422F0 BMD__FindTriangleForEdge — nunca activado: IDA_PORT_004422F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── BMD__FindTriangleForEdge (IDA-only, gated) ──
#if defined(IDA_PORT_004422F0)
void __cdecl BMD::FindTriangleForEdge(DWORD This, int iMesh, int iTri1, int iIndex11)
{
  int v4; // ecx
  int v5; // edx
  int v6; // ebx
  int v7; // ecx
  int v8; // eax
  WORD *v9; // esi
  int v10; // ecx
  int v11; // [esp+10h] [ebp-Ch]
  int v12; // [esp+18h] [ebp-4h]
  int iMesha; // [esp+20h] [ebp+4h]

  v4 = *(DWORD *)(This + 40);
  v5 = iTri1;
  v6 = *(DWORD *)(v4 + 40 * iMesh + 28);
  v11 = v6 + 36 * iTri1;
  if ( *(WORD *)(v11 + 2 * iIndex11 + 26) == 0xFFFF )
  {
    v7 = *(short *)(v4 + 40 * iMesh + 10);
    v8 = 0;
    v12 = v7;
    iMesha = 0;
    if ( v7 > 0 )
    {
      while ( v5 == v8 )
      {
LABEL_10:
        ++v8;
        v6 += 36;
        iMesha = v8;
        if ( v8 >= v7 )
        {
          return;
        }
      }
      v9 = (WORD *)(v6 + 2);
      v10 = 0;
      while ( v9[12] != 0xFFFF
           || *(WORD *)(v11 + 2 * (iIndex11 + 1)) != *(WORD *)(v6 + 2 * ((v10 + 1) % 3) + 2)
           || *(WORD *)(v11 + 2 * ((iIndex11 + 1) % 3) + 2) != *v9 )
      {
        ++v10;
        ++v9;
        if ( v10 >= 3 )
        {
          v8 = iMesha;
          v7 = v12;
          v5 = iTri1;
          goto LABEL_10;
        }
      }
      *(WORD *)(v11 + 2 * iIndex11 + 26) = iMesha;
      *(WORD *)(v6 + 2 * v10 + 26) = iTri1;
    }
  }
}
#endif
