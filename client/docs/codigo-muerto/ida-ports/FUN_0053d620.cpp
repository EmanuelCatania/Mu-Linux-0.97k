// 0x0053D620 FUN_0053d620 — nunca activado: IDA_PORT_0053D620 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053d620 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D620)
void __cdecl FUN_0053d620(DWORD *_this)
{
  int v2; // eax
  DWORD *v3; // esi
  unsigned int v4; // eax
  bool v5; // cc
  char *v6; // eax
  const char *v7; // eax
  signed int v8; // ecx
  BYTE *v9; // eax
  int v10; // ebx
  BYTE *v11; // eax
  int v12; // edx
  int v13; // [esp-2Ch] [ebp-40h]
  int v14; // [esp-28h] [ebp-3Ch]
  int v15; // [esp-24h] [ebp-38h]
  int v16; // [esp-20h] [ebp-34h]
  int v17; // [esp-1Ch] [ebp-30h]
  int v18; // [esp-18h] [ebp-2Ch]
  int v19; // [esp-14h] [ebp-28h]
  char *v20; // [esp-10h] [ebp-24h]
  DWORD NumberOfBytesWritten; // [esp+4h] [ebp-10h] BYREF
  int v22; // [esp+10h] [ebp-4h]

  NumberOfBytesWritten = (DWORD)_this;
  v22 = 0;
  FUN_00540ac0(_this + 158);
  FUN_00540510(_this + 151);
  v2 = _this[8];
  v3 = _this + 8;
  v22 = -1;
  if ( v2 != -1 )
  {
    GetLocalTime((LPSYSTEMTIME)(v3 + 135));
    v4 = v3[141];
    if ( !v4 || (v5 = v3[142] <= v4, v6 = (char *)&DAT_00562ecc, v5) )
    {
      v6 = &strID;
    }
    v20 = v6;
    v19 = *((unsigned short *)v3 + 277);
    v18 = *((unsigned short *)v3 + 276);
    v17 = *((unsigned short *)v3 + 275);
    v16 = *((unsigned short *)v3 + 274);
    v15 = *((unsigned short *)v3 + 273);
    v14 = *((unsigned short *)v3 + 271);
    v13 = *((unsigned short *)v3 + 270);
    v7 = (const char *)FUN_0053e8c0(&DAT_00562e74);
    sprintf((char *const)v3 + 28, v7, v13, v14, v15, v16, v17, v18, v19, v20);
    v8 = strlen((const char *)v3 + 28);
    if ( v3[140] )
    {
      if ( v8 > 0 )
      {
        v9 = v3 + 7;
        do
        {
          v10 = v3[140] + 2;
          v3[140] = v10;
          *v9++ ^= (BYTE)v10 + 67;
        }
        while ( (int)&v9[-28 - (DWORD)v3] < v8 );
      }
    }
    else if ( v8 > 0 )
    {
      v11 = v3 + 7;
      do
      {
        v12 = 3 * v3[139] + 1;
        v3[139] = v12;
        *v11++ ^= (BYTE)v12 + 70;
      }
      while ( (int)&v11[-28 - (DWORD)v3] < v8 );
    }
    if ( v3[141] != 999 )
    {
      WriteFile((HANDLE)*v3, v3 + 7, v8, &NumberOfBytesWritten, 0);
    }
    SetEndOfFile((HANDLE)*v3);
    CloseHandle((HANDLE)*v3);
    *v3 = -1;
    DeleteCriticalSection((LPCRITICAL_SECTION)(v3 + 1));
  }
}
#endif
