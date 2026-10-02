// 0x0053D580 FUN_0053d580 — nunca activado: IDA_PORT_0053D580 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053d580 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D580)
int __cdecl FUN_0053d580(char a1)
{
  BYTE *v2; // esi
  char *v3; // eax
  char *v4; // eax
  char *v5; // eax
  const CHAR *v6; // eax
  HANDLE v7; // eax
  char *v8; // eax
  DWORD ExitCode; // [esp+0h] [ebp-4h] BYREF

  if ( !lpParameter )
  {
    return 0;
  }
  ExitCode = (DWORD)lpParameter;
  v2 = lpParameter;
  if ( *((BYTE *)lpParameter + 1) )
  {
    return 1877;
  }
  if ( *(BYTE *)lpParameter )
  {
    ExitCode = 0;
    if ( !GetExitCodeProcess(hProcess, &ExitCode) || ExitCode == 259 )
    {
      if ( v2[30] )
      {
        v5 = (char *)FUN_0053e8c0(&DAT_00563418);
        FUN_0053eba0((int)(v2 + 32), v5, a1);
        return 640;
      }
      else
      {
        v6 = (const CHAR *)FUN_0053e8c0(&DAT_005632fc);
        v7 = OpenEventA(0x100000u, 0, v6);
        if ( v7 )
        {
          CloseHandle(v7);
          return 1877;
        }
        else
        {
          v8 = (char *)FUN_0053e8c0(&DAT_005633f8);
          FUN_0053eba0((int)(v2 + 32), v8, a1);
          return 630;
        }
      }
    }
    else
    {
      v4 = (char *)FUN_0053e8c0(&DAT_00563438);
      FUN_0053eba0((int)(v2 + 32), v4, a1);
      DAT_083bbaf0 = 8;
      return 620;
    }
  }
  else
  {
    v3 = (char *)FUN_0053e8c0(&DAT_00563438);
    FUN_0053eba0((int)(v2 + 32), v3, a1);
    return 610;
  }
}
#endif
