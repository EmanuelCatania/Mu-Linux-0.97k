// 0x0053D430 FUN_0053d430 — nunca activado: IDA_PORT_0053D430 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053d430 (IDA-only, gated) ──
#if defined(IDA_PORT_0053D430)
int __cdecl FUN_0053d430(unsigned char *a1)
{
  int v2; // eax
  int v3; // esi
  LONG (__stdcall *v4)(struct _EXCEPTION_POINTERS *); // esi

  if ( lpParameter )
  {
    return 0;
  }
  v2 = operator_new(0x34Cu);
  v3 = v2;
  if ( v2 )
  {
    *(BYTE *)v2 = 0;
    *(BYTE *)(v2 + 1) = 0;
    *(DWORD *)(v2 + 4) = 0;
    *(DWORD *)(v2 + 8) = 0;
    *(DWORD *)(v2 + 12) = 0;
    *(DWORD *)(v2 + 16) = 0;
    *(BYTE *)(v2 + 20) = 0;
    *(DWORD *)(v2 + 24) = 0;
    *(BYTE *)(v2 + 28) = 0;
    *(BYTE *)(v2 + 29) = 0;
    *(BYTE *)(v2 + 30) = 0;
    *(DWORD *)(v2 + 32) = -1;
    *(DWORD *)(v2 + 600) = 0;
    FUN_005404c0(v2 + 604);
    FUN_00540a70((HCRYPTPROV *)(v3 + 632));
    *(DWORD *)(v3 + 704) = 0;
    *(DWORD *)(v3 + 708) = 0;
    *(DWORD *)(v3 + 744) = 0;
    *(DWORD *)(v3 + 748) = 0;
    *(BYTE *)(v3 + 816) = 0;
    *(BYTE *)(v3 + 817) = 0;
    *(DWORD *)(v3 + 820) = 1000;
    *(DWORD *)(v3 + 824) = 0;
    *(DWORD *)(v3 + 828) = 0;
    *(BYTE *)(v3 + 832) = 0;
    *(DWORD *)(v3 + 836) = 0;
    *(DWORD *)(v3 + 840) = 0;
  }
  else
  {
    v3 = 0;
  }
  lpParameter = (LPVOID)v3;
  v4 = SetUnhandledExceptionFilter(TopLevelExceptionFilter);
  *((DWORD *)lpParameter + 3) = FUN_0053d890(lpParameter, a1);
  if ( v4 )
  {
    SetUnhandledExceptionFilter(v4);
  }
  return *((DWORD *)lpParameter + 3);
}
#endif
