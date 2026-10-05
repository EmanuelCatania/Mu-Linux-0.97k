// 0x0040F860 FUN_0040f860 — nunca activado: IDA_PORT_0040F860 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040f860 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F860)
void __cdecl FUN_0040f860(int _this)
{
  int *v2; // edi
  int v3; // ecx
  int v4; // eax
  LPVOID *i; // ebx
  int v6; // eax
  LPVOID *v7; // edi
  DWORD *v8; // eax
  int v9; // ebx
  void *v10; // esi
  int *v11; // [esp+10h] [ebp-Ch] BYREF
  char v12[4]; // [esp+14h] [ebp-8h] BYREF
  char v13[4]; // [esp+18h] [ebp-4h] BYREF

  v2 = *(int **)(_this + 4);
  v3 = *(DWORD *)(_this + 12);
  v4 = *v2;
  v11 = (int *)*v2;
  if ( v3 )
  {
    v6 = DAT_055c9b98;
    v7 = (LPVOID *)v2[1];
    for ( i = v7; v7 != (LPVOID *)DAT_055c9b98; i = v7 )
    {
      FUN_00411420(v7[2]);
      v7 = (LPVOID *)*v7;
      delete__(i);
      v6 = DAT_055c9b98;
    }
    *(DWORD *)(*(DWORD *)(_this + 4) + 4) = v6;
    v8 = *(DWORD **)(_this + 4);
    *(DWORD *)(_this + 12) = 0;
    *v8 = v8;
    *(DWORD *)(*(DWORD *)(_this + 4) + 8) = *(DWORD *)(_this + 4);
    FUN_00410e40(v12);
  }
  else if ( (int *)v4 != v2 )
  {
    do
    {
      v9 = v4;
      FUN_004112b0(&v11);
      FUN_00410e50(v13, v9);
      v4 = (int)v11;
    }
    while ( v11 != v2 );
  }
  delete__(*(LPVOID *)(_this + 4));
  *(DWORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 12) = 0;
  v10 = 0;
  std::_Lockit::_Lockit((std::_Lockit *)&v11);
  if ( !--DAT_055c9b94 )
  {
    v10 = (void *)DAT_055c9b98;
    DAT_055c9b98 = 0;
  }
  std::_Lockit::~_Lockit((std::_Lockit *)&v11);
  if ( v10 )
  {
    delete__(v10);
  }
}
#endif
