// 0x0053ED80 FUN_0053ed80 — nunca activado: IDA_PORT_0053ED80 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053ed80 (IDA-only, gated) ──
#if defined(IDA_PORT_0053ED80)
char __cdecl FUN_0053ed80(int _this)
{
  int v3; // esi
  char *v4; // eax
  bool v5; // zf
  void *v6; // eax
  char *v7; // eax
  unsigned int v8; // eax
  bool v9; // cc
  char *v10; // eax
  const char *v11; // eax
  signed int v12; // ecx
  BYTE *v13; // eax
  int v14; // edx
  BYTE *v15; // eax
  int v16; // edx
  int v17; // [esp-28h] [ebp-34h]
  int v18; // [esp-24h] [ebp-30h]
  int v19; // [esp-20h] [ebp-2Ch]
  int v20; // [esp-1Ch] [ebp-28h]
  int v21; // [esp-18h] [ebp-24h]
  int v22; // [esp-14h] [ebp-20h]
  int v23; // [esp-10h] [ebp-1Ch]
  char *v24; // [esp-Ch] [ebp-18h]
  DWORD NumberOfBytesWritten; // [esp+8h] [ebp-4h] BYREF

  if ( *(BYTE *)(_this + 1) )
  {
    return 1;
  }
  v3 = _this + 32;
  v4 = FUN_0053e8c0(DAT_00563488);
  FUN_0053eba0(_this + 32, v4);
  v5 = *(BYTE *)_this == 0;
  *(BYTE *)(_this + 1) = 1;
  if ( v5 )
  {
    if ( DAT_00562e5c )
    {
      if ( DAT_083bbaf0 )
      {
        FUN_0053f680((char *)_this);
      }
    }
    return 0;
  }
  else
  {
    if ( *(BYTE *)(_this + 20) )
    {
      FUN_0053f290(_this);
      *(BYTE *)(_this + 20) = 0;
    }
    v6 = *(void **)(_this + 824);
    if ( v6 )
    {
      CloseHandle(v6);
      *(DWORD *)(_this + 824) = 0;
    }
    if ( *(DWORD *)(_this + 828) )
    {
      CloseHandle(*(HANDLE *)(_this + 828));
      *(DWORD *)(_this + 828) = 0;
    }
    if ( DAT_00562e5c && DAT_083bbaf0 )
    {
      FUN_0053f680((char *)_this);
    }
    v7 = FUN_0053e8c0(DAT_00563470);
    FUN_0053eba0(_this + 32, v7);
    if ( *(DWORD *)v3 != -1 )
    {
      GetLocalTime((LPSYSTEMTIME)(_this + 572));
      v8 = *(DWORD *)(_this + 596);
      if ( !v8 || (v9 = *(DWORD *)(_this + 600) <= v8, v10 = (char *)&DAT_00562ecc, v9) )
      {
        v10 = &strID;
      }
      v24 = v10;
      v23 = *(unsigned short *)(_this + 586);
      v22 = *(unsigned short *)(_this + 584);
      v21 = *(unsigned short *)(_this + 582);
      v20 = *(unsigned short *)(_this + 580);
      v19 = *(unsigned short *)(_this + 578);
      v18 = *(unsigned short *)(_this + 574);
      v17 = *(unsigned short *)(_this + 572);
      v11 = FUN_0053e8c0(DAT_00562e74);
      sprintf((char *const)(v3 + 28), v11, v17, v18, v19, v20, v21, v22, v23, v24);
      v12 = strlen((const char *)(v3 + 28));
      if ( *(DWORD *)(_this + 592) )
      {
        if ( v12 > 0 )
        {
          v13 = (BYTE *)(_this + 60);
          do
          {
            v14 = *(DWORD *)(_this + 592) + 2;
            *(DWORD *)(_this + 592) = v14;
            *v13++ ^= (BYTE)v14 + 67;
          }
          while ( (int)&v13[-28 - v3] < v12 );
        }
      }
      else if ( v12 > 0 )
      {
        v15 = (BYTE *)(_this + 60);
        do
        {
          v16 = 3 * *(DWORD *)(_this + 588) + 1;
          *(DWORD *)(_this + 588) = v16;
          *v15++ ^= (BYTE)v16 + 70;
        }
        while ( (int)&v15[-28 - v3] < v12 );
      }
      if ( *(DWORD *)(_this + 596) != 999 )
      {
        WriteFile(*(HANDLE *)v3, (LPCVOID)(_this + 60), v12, &NumberOfBytesWritten, 0);
      }
      SetEndOfFile(*(HANDLE *)v3);
      CloseHandle(*(HANDLE *)v3);
      *(DWORD *)v3 = -1;
      DeleteCriticalSection((LPCRITICAL_SECTION)(_this + 36));
    }
    *(BYTE *)_this = 0;
    return FUN_005406f0(_this + 604);
  }
}
#endif
