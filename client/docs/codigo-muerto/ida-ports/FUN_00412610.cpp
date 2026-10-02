// 0x00412610 FUN_00412610 — nunca activado: IDA_PORT_00412610 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00412610 (IDA-only, gated) ──
#if defined(IDA_PORT_00412610)
int __fastcall FUN_00412610(DWORD *a1)
{
  DWORD *v1; // edi
  DWORD **v2; // ebx
  DWORD *v3; // esi
  DWORD **v4; // eax
  DWORD *v5; // ebx
  DWORD **v6; // ebp
  DWORD *v7; // esi
  DWORD **v8; // eax
  DWORD **v9; // ebx
  DWORD *v10; // esi
  DWORD **v11; // eax
  DWORD *v13; // [esp+10h] [ebp-8h]
  int v14; // [esp+14h] [ebp-4h] BYREF

  v1 = a1 + 22;
  *a1 = &DAT_00552760;
  v13 = a1;
  v2 = (DWORD **)a1[23];
  v3 = *v2;
  if ( *v2 != v2 )
  {
    do
    {
      v4 = (DWORD **)v3;
      v3 = (DWORD *)*v3;
      FUN_00411360(v1, &v14, v4);
    }
    while ( v3 != v2 );
    a1 = v13;
  }
  v5 = a1 + 30;
  a1[34] = 0;
  v6 = (DWORD **)a1[31];
  v7 = *v6;
  while ( v7 != v6 )
  {
    v8 = (DWORD **)v7;
    v7 = (DWORD *)*v7;
    FUN_00411360(v5, &v14, v8);
  }
  delete__((LPVOID)v5[1]);
  v5[1] = 0;
  v5[2] = 0;
  v9 = (DWORD **)v1[1];
  v10 = *v9;
  while ( v10 != v9 )
  {
    v11 = (DWORD **)v10;
    v10 = (DWORD *)*v10;
    FUN_00411360(v1, &v14, v11);
  }
  delete__((LPVOID)v1[1]);
  v1[1] = 0;
  v1[2] = 0;
  *v13 = DAT_005525c8;
  FUN_00410de0(v13 + 1, &v14, *(DWORD **)v13[2], (DWORD *)v13[2]);
  return FUN_00410d90((int)(v13 + 1));
}
#endif
