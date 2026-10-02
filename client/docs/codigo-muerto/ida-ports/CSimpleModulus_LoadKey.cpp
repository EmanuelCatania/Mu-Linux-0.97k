// 0x0053D1C0 CSimpleModulus_LoadKey — nunca activado: IDA_PORT_0053D1C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CSimpleModulus_LoadKey (IDA-only, gated) ──
#if defined(IDA_PORT_0053D1C0)
int __cdecl CSimpleModulus_LoadKey(DWORD *_this, LPCSTR lpFileName, short a3, int a4, int a5, int a6, int a7)
{
  HANDLE FileA; // eax
  void *v9; // esi
  int v10; // ebp
  char *v11; // ecx
  int i; // eax
  int v13; // edx
  int *v14; // ecx
  int j; // eax
  int v16; // edx
  int *v17; // ecx
  int k; // eax
  int v19; // edx
  int *v20; // ecx
  int m; // eax
  int v22; // edx
  short Buffer; // [esp+10h] [ebp-18h] BYREF
  int v25; // [esp+12h] [ebp-16h]
  int v26[4]; // [esp+18h] [ebp-10h] BYREF

  FileA = CreateFileA(lpFileName, 0x80000000, 1u, 0, 3u, 0x80u, 0);
  v9 = FileA;
  if ( FileA == (HANDLE)-1 )
  {
    return 0;
  }
  ReadFile(FileA, &Buffer, 6u, (LPDWORD)&lpFileName, 0);
  if ( Buffer != a3 || (v10 = a5, v25 != 16 * (a7 + a6 + a4 + a5) + 6) )
  {
    CloseHandle(v9);
    return 0;
  }
  if ( a4 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v11 = (char *)(_this + 1);
    for ( i = 0; i < 4; ++i )
    {
      v11 += 4;
      v13 = v26[i] ^ DAT_00562e48[i];
      *((DWORD *)v11 - 1) = v13;
    }
  }
  if ( v10 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v14 = _this + 5;
    for ( j = 0; j < 4; ++j )
    {
      v16 = v26[j] ^ DAT_00562e48[j];
      *v14++ = v16;
    }
  }
  if ( a6 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v17 = _this + 9;
    for ( k = 0; k < 4; ++k )
    {
      v19 = v26[k] ^ DAT_00562e48[k];
      *v17++ = v19;
    }
  }
  if ( a7 )
  {
    ReadFile(v9, v26, 0x10u, (LPDWORD)&lpFileName, 0);
    v20 = _this + 13;
    for ( m = 0; m < 4; ++m )
    {
      v22 = v26[m] ^ DAT_00562e48[m];
      *v20++ = v22;
    }
  }
  CloseHandle(v9);
  return 1;
}
#endif
