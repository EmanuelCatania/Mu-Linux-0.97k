// 0x0053D7D0 FUN_0053d7d0 — nunca activado: IDA_PORT_0053D7D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053d7d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D7D0)
int __cdecl FUN_0053d7d0(int a1, char a2, const CHAR *lpString)
{
  char *v4; // eax
  char *v6; // eax
  char *v7; // eax
  char *v8; // eax
  char v9; // [esp-Ch] [ebp-10h]
  int v10; // [esp-4h] [ebp-8h]
  char v11; // [esp-4h] [ebp-8h]

  if ( *(BYTE *)a1 )
  {
    v11 = a2;
    *(DWORD *)(a1 + 704) = lpString;
    Pipe_Write(2225, 1554, lpString);
    if ( *(DWORD *)(a1 + 4) )
    {
      v9 = *(DWORD *)(a1 + 4);
      v6 = (char *)FUN_0053e8c0(&DAT_00562f04);
      FUN_0053eba0(a1 + 32, v6, v9);
      if ( !FUN_004070d0(*(DWORD *)(a1 + 4), *(DWORD *)(a1 + 8)) )
      {
        v7 = (char *)FUN_0053e8c0(&DAT_00562ee8);
        FUN_0053eba0(a1 + 32, v7, a2);
      }
    }
    v8 = (char *)FUN_0053e8c0(&DAT_00562ed0);
    FUN_0053eba0(a1 + 32, v8, v11);
    return *(DWORD *)(a1 + 12);
  }
  else
  {
    v10 = *(DWORD *)(a1 + 12);
    v4 = (char *)FUN_0053e8c0(&DAT_00562f24);
    FUN_0053eba0(a1 + 32, v4, v10);
    return *(DWORD *)(a1 + 12);
  }
}
#endif
