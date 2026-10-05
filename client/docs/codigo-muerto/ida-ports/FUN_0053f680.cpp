// 0x0053F680 FUN_0053f680 — nunca activado: IDA_PORT_0053F680 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053f680 (IDA-only, gated) ──
#if defined(IDA_PORT_0053F680)
HMODULE __cdecl FUN_0053f680(char *_this)
{
  char *v1; // esi
  char *v2; // eax
  char v3; // bl
  const CHAR *v4; // eax
  const CHAR *v5; // eax
  const CHAR *v6; // eax
  const CHAR *v7; // eax
  const CHAR *v8; // eax
  HMODULE result; // eax
  char *v10; // eax
  unsigned int v11; // eax
  bool v12; // cc
  char *v13; // eax
  const char *v14; // eax
  signed int v15; // ecx
  BYTE *v16; // eax
  int v17; // edx
  BYTE *v18; // eax
  int v19; // edx
  const CHAR *v20; // eax
  HMODULE v21; // edi
  const CHAR *v22; // eax
  FARPROC ProcAddress; // ebx
  const CHAR *v24; // eax
  FARPROC v25; // ebp
  const CHAR *v26; // eax
  const CHAR *v27; // eax
  BYTE *v28; // eax
  HMODULE v29; // ebx
  BYTE *v30; // eax
  int v31; // esi
  BYTE *v32; // eax
  const char *v33; // eax
  FARPROC v34; // ebx
  char *v35; // [esp-Ch] [ebp-300h]
  BYTE *v36; // [esp-4h] [ebp-2F8h]
  int v37; // [esp+0h] [ebp-2F4h]
  int v38; // [esp+Ch] [ebp-2E8h]
  char *v39; // [esp+Ch] [ebp-2E8h]
  int v40; // [esp+10h] [ebp-2E4h]
  int wYear; // [esp+10h] [ebp-2E4h]
  int v42; // [esp+14h] [ebp-2E0h]
  int wMonth; // [esp+14h] [ebp-2E0h]
  int v44; // [esp+18h] [ebp-2DCh]
  BYTE *v45; // [esp+18h] [ebp-2DCh]
  int wDay; // [esp+18h] [ebp-2DCh]
  int v47; // [esp+1Ch] [ebp-2D8h]
  BYTE *v48; // [esp+1Ch] [ebp-2D8h]
  int wHour; // [esp+1Ch] [ebp-2D8h]
  int v50; // [esp+20h] [ebp-2D4h]
  int wMinute; // [esp+20h] [ebp-2D4h]
  int v52; // [esp+24h] [ebp-2D0h]
  int wSecond; // [esp+24h] [ebp-2D0h]
  DWORD v54; // [esp+28h] [ebp-2CCh]
  char *v55; // [esp+28h] [ebp-2CCh]
  int wMilliseconds; // [esp+28h] [ebp-2CCh]
  HMODULE v57; // [esp+28h] [ebp-2CCh]
  DWORD cbData; // [esp+3Ch] [ebp-2B8h] BYREF
  BYTE v59[4]; // [esp+40h] [ebp-2B4h] BYREF
  HKEY phkResult; // [esp+44h] [ebp-2B0h] BYREF
  FARPROC v61; // [esp+48h] [ebp-2ACh]
  DWORD nSize; // [esp+4Ch] [ebp-2A8h] BYREF
  DWORD NumberOfBytesWritten; // [esp+50h] [ebp-2A4h] BYREF
  char *v64; // [esp+54h] [ebp-2A0h]
  DWORD dwDisposition; // [esp+58h] [ebp-29Ch] BYREF
  struct _SYSTEMTIME SystemTime; // [esp+5Ch] [ebp-298h] BYREF
  BYTE Data[64]; // [esp+70h] [ebp-284h] BYREF
  CHAR String2[64]; // [esp+B0h] [ebp-244h] BYREF
  CHAR Buffer[128]; // [esp+F0h] [ebp-204h] BYREF
  CHAR v70[128]; // [esp+170h] [ebp-184h] BYREF
  char v71[260]; // [esp+1F0h] [ebp-104h] BYREF

  v64 = _this;
  v1 = _this + 32;
  v2 = FUN_0053e8c0(DAT_005638e8);
  FUN_0053eba0((int)v1, v2);
  *(DWORD *)v59 = 0;
  v3 = 0;
  v4 = FUN_0053e8c0(DAT_005638b0);
  RegCreateKeyExA(HKEY_LOCAL_MACHINE, v4, 0, 0, 0, 0xF003Fu, 0, &phkResult, &dwDisposition);
  if ( dwDisposition == 2 )
  {
    cbData = 64;
    v5 = FUN_0053e8c0(DAT_0056389c);
    RegQueryValueExA(phkResult, v5, 0, 0, Data, &cbData);
    wsprintfA(String2, DAT_00563898, aFriOct24111310);
    if ( cbData && !lstrcmpA((LPCSTR)Data, String2) )
    {
      v3 = 1;
    }
    cbData = 4;
    v6 = FUN_0053e8c0(DAT_00563888);
    RegQueryValueExA(phkResult, v6, 0, 0, v59, &cbData);
  }
  wsprintfA((LPSTR)Data, DAT_00563898, aFriOct24111310);
  v54 = lstrlenA((LPCSTR)Data) + 1;
  cbData = v54;
  v7 = FUN_0053e8c0(DAT_00563874);
  RegSetValueExA(phkResult, v7, 0, 1u, Data, v54);
  cbData = 4;
  if ( v3 )
  {
    ++*(DWORD *)v59;
  }
  else
  {
    *(DWORD *)v59 = 0;
  }
  v8 = FUN_0053e8c0(DAT_00563888);
  RegSetValueExA(phkResult, v8, 0, 4u, v59, 4u);
  RegCloseKey(phkResult);
  result = *(HMODULE *)v59;
  if ( *(DWORD *)v59 != 1 && *(DWORD *)v59 <= 0xAu && (!v3 || DAT_083bbaf0 < 9) )
  {
    v10 = FUN_0053e8c0(DAT_00563864);
    FUN_0053eba0((int)v1, v10);
    if ( *(DWORD *)v1 != -1 )
    {
      GetLocalTime((LPSYSTEMTIME)(v1 + 540));
      v11 = *((DWORD *)v1 + 141);
      if ( !v11 || (v12 = *((DWORD *)v1 + 142) <= v11, v13 = (char *)&DAT_00562ecc, v12) )
      {
        v13 = &strID;
      }
      v55 = v13;
      v52 = *((unsigned short *)v1 + 277);
      v50 = *((unsigned short *)v1 + 276);
      v47 = *((unsigned short *)v1 + 275);
      v44 = *((unsigned short *)v1 + 274);
      v42 = *((unsigned short *)v1 + 273);
      v40 = *((unsigned short *)v1 + 271);
      v38 = *((unsigned short *)v1 + 270);
      v14 = FUN_0053e8c0(DAT_00562e74);
      sprintf(v1 + 28, v14, v38, v40, v42, v44, v47, v50, v52, v55);
      v15 = strlen(v1 + 28);
      if ( *((DWORD *)v1 + 140) )
      {
        if ( v15 > 0 )
        {
          v16 = v1 + 28;
          do
          {
            v17 = *((DWORD *)v1 + 140) + 2;
            *((DWORD *)v1 + 140) = v17;
            *v16++ ^= (BYTE)v17 + 67;
          }
          while ( (int)&v16[-28 - (DWORD)v1] < v15 );
        }
      }
      else if ( v15 > 0 )
      {
        v18 = v1 + 28;
        do
        {
          v19 = 3 * *((DWORD *)v1 + 139) + 1;
          *((DWORD *)v1 + 139) = v19;
          *v18++ ^= (BYTE)v19 + 70;
        }
        while ( (int)&v18[-28 - (DWORD)v1] < v15 );
      }
      if ( *((DWORD *)v1 + 141) != 999 )
      {
        WriteFile(*(HANDLE *)v1, v1 + 28, v15, &NumberOfBytesWritten, 0);
      }
      SetEndOfFile(*(HANDLE *)v1);
      CloseHandle(*(HANDLE *)v1);
      *(DWORD *)v1 = -1;
      DeleteCriticalSection((LPCRITICAL_SECTION)(v1 + 4));
    }
    v20 = FUN_0053e8c0(DAT_00563854);
    result = LoadLibraryA(v20);
    v21 = result;
    if ( result )
    {
      v22 = FUN_0053e8c0(DAT_00563840);
      ProcAddress = GetProcAddress(v21, v22);
      v24 = FUN_0053e8c0(DAT_00563828);
      v25 = GetProcAddress(v21, v24);
      v26 = FUN_0053e8c0(&DAT_00563810);
      v61 = GetProcAddress(v21, v26);
      v27 = FUN_0053e8c0(DAT_00563800);
      result = (HMODULE)GetProcAddress(v21, v27);
      NumberOfBytesWritten = (DWORD)result;
      if ( ProcAddress )
      {
        if ( v25 )
        {
          if ( v61 )
          {
            if ( result )
            {
              v28 = FUN_0053e8c0(DAT_005637f4);
              result = (HMODULE)((int (__stdcall *)(BYTE *, DWORD, DWORD, DWORD, DWORD))ProcAddress)(
                                  v28,
                                  0,
                                  0,
                                  0,
                                  0);
              v29 = result;
              if ( result )
              {
                v48 = FUN_0053e8c0(DAT_005637e4);
                v45 = FUN_0053e8c0(DAT_005637d8);
                v30 = FUN_0053e8c0(DAT_005637c4);
                v31 = ((int (__stdcall *)(HMODULE, BYTE *, int, BYTE *, BYTE *, int, DWORD, DWORD))v25)(
                        v29,
                        v30,
                        21,
                        v45,
                        v48,
                        1,
                        0,
                        0);
                if ( v31 )
                {
                  Buffer[0] = 0;
                  nSize = 128;
                  GetComputerNameA(Buffer, &nSize);
                  v70[0] = 0;
                  nSize = 128;
                  GetUserNameA(v70, &nSize);
                  GetLocalTime(&SystemTime);
                  switch ( DAT_083bbaf0 )
                  {
                    case 1:
                      v32 = FUN_0053e8c0(DAT_005637b8);
                      break;
                    case 2:
                      v32 = FUN_0053e8c0(DAT_005637ac);
                      break;
                    case 3:
                      v32 = FUN_0053e8c0(DAT_005637a4);
                      break;
                    case 4:
                      v32 = FUN_0053e8c0(DAT_00563798);
                      break;
                    case 5:
                      v32 = FUN_0053e8c0(DAT_00563790);
                      break;
                    case 6:
                      v32 = FUN_0053e8c0(DAT_00563780);
                      break;
                    case 7:
                      v32 = FUN_0053e8c0(DAT_00563774);
                      break;
                    case 8:
                      v32 = FUN_0053e8c0(DAT_00563768);
                      break;
                    case 9:
                      v32 = FUN_0053e8c0(DAT_0056375c);
                      break;
                    default:
                      v32 = FUN_0053e8c0(DAT_00563754);
                      break;
                  }
                  wMilliseconds = SystemTime.wMilliseconds;
                  wSecond = SystemTime.wSecond;
                  wMinute = SystemTime.wMinute;
                  wHour = SystemTime.wHour;
                  wDay = SystemTime.wDay;
                  wMonth = SystemTime.wMonth;
                  wYear = SystemTime.wYear;
                  v39 = v64 + 752;
                  v37 = *((DWORD *)lpParameter + 3);
                  v36 = v32;
                  v35 = v64 + 712;
                  v33 = FUN_0053e8c0(DAT_005636f8);
                  sprintf(
                    v71,
                    v33,
                    v35,
                    18,
                    v36,
                    v37,
                    Buffer,
                    v70,
                    v39,
                    wYear,
                    wMonth,
                    wDay,
                    wHour,
                    wMinute,
                    wSecond,
                    wMilliseconds);
                  ((void (__stdcall *)(int, CHAR *, char *, void *, DWORD))NumberOfBytesWritten)(
                    v31,
                    &DAT_083bb9e0,
                    v71,
                    &DAT_04000002,
                    0);
                  GetLastError();
                  v57 = v29;
                  v34 = v61;
                  ((void (__stdcall *)(HMODULE))v61)(v57);
                  ((void (__stdcall *)(int))v34)(v31);
                  return (HMODULE)FreeLibrary(v21);
                }
                else
                {
                  return (HMODULE)((int (__stdcall *)(HMODULE))v61)(v29);
                }
              }
            }
          }
        }
      }
    }
  }
  return result;
}
#endif
