// 0x0040F950 FUN_0040f950 — nunca activado: IDA_PORT_0040F950 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040f950 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F950)
void __cdecl FUN_0040f950(DWORD *_this)
{
  int *v2; // eax
  int v3; // ecx
  DWORD *v4; // esi
  int *v5; // edi
  int v6; // eax
  int **v7; // ecx
  int v8; // ecx
  DWORD *v9; // eax
  DWORD *v10; // eax
  DWORD **v11; // ebx
  DWORD *v12; // ebp
  DWORD *v13; // edi
  void *v14; // eax
  int v15; // ecx
  int *v16; // edi
  int v17; // eax
  int **v18; // edx
  int v19; // ecx
  DWORD *v20; // eax
  DWORD *v21; // eax
  void *v22; // esi
  int *v23; // [esp+10h] [ebp-20h] BYREF
  DWORD *v24; // [esp+14h] [ebp-1Ch]
  char v25[4]; // [esp+18h] [ebp-18h] BYREF
  char v26[4]; // [esp+1Ch] [ebp-14h] BYREF
  char v27[4]; // [esp+20h] [ebp-10h] BYREF
  int v28; // [esp+2Ch] [ebp-4h]

  v24 = _this;
  *_this = &DAT_005527f8;
  v2 = (int *)_this[2];
  v3 = _this[4];
  v4 = _this + 1;
  v5 = v2;
  v6 = *v2;
  v28 = 2;
  v23 = (int *)v6;
  if ( v3 && (v7 = (int **)FUN_00410e40(v25), v6 = (int)v23, v23 == *v7) && (v8 = _this[2], v5 == (int *)v8) )
  {
    FUN_00411420(*(LPVOID *)(v8 + 4));
    *(DWORD *)(_this[2] + 4) = DAT_055c9b98;
    v9 = (DWORD *)_this[2];
    _this[4] = 0;
    *v9 = v9;
    *(DWORD *)(_this[2] + 8) = _this[2];
    FUN_00410e40(v26);
  }
  else if ( (int *)v6 != v5 )
  {
    do
    {
      v10 = (DWORD *)BSTIterator_PostIncrement(v25, 0);
      FUN_00410e50(v27, *v10);
    }
    while ( v23 != v5 );
  }
  v11 = (DWORD **)_this[7];
  v12 = _this + 6;
  (BYTE)(v28) = 1;
  v13 = *v11;
  while ( v13 != v11 )
  {
    v14 = v13;
    v13 = (DWORD *)*v13;
    FUN_00411360(v27, v14);
  }
  delete__((LPVOID)v12[1]);
  v12[1] = 0;
  v12[2] = 0;
  v15 = v4[3];
  v16 = (int *)v4[1];
  v17 = *v16;
  (BYTE)(v28) = 0;
  v23 = (int *)v17;
  if ( v15 && (v18 = (int **)FUN_00410e40(v27), v17 = (int)v23, v23 == *v18) && (v19 = v4[1], v16 == (int *)v19) )
  {
    FUN_00411420(*(LPVOID *)(v19 + 4));
    *(DWORD *)(v4[1] + 4) = DAT_055c9b98;
    v20 = (DWORD *)v4[1];
    v4[3] = 0;
    *v20 = v20;
    *(DWORD *)(v4[1] + 8) = v4[1];
    FUN_00410e40(v26);
  }
  else if ( (int *)v17 != v16 )
  {
    do
    {
      v21 = (DWORD *)BSTIterator_PostIncrement(v27, 0);
      FUN_00410e50(v25, *v21);
    }
    while ( v23 != v16 );
  }
  delete__((LPVOID)v4[1]);
  v4[1] = 0;
  v4[3] = 0;
  v22 = 0;
  std::_Lockit::_Lockit((std::_Lockit *)&v23);
  if ( !--DAT_055c9b94 )
  {
    v22 = (void *)DAT_055c9b98;
    DAT_055c9b98 = 0;
  }
  std::_Lockit::~_Lockit((std::_Lockit *)&v23);
  if ( v22 )
  {
    delete__(v22);
  }
  FUN_0040f690(v24);
}
#endif
