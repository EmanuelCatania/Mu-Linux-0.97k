// 0x0053F290 FUN_0053f290 — nunca activado: IDA_PORT_0053F290 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053f290 (IDA-only, gated) ──
#if defined(IDA_PORT_0053F290)
char __cdecl FUN_0053f290(int _this)
{
  char *v3; // eax
  void *v4; // eax
  char *v5; // eax
  char *v6; // eax

  if ( !*(BYTE *)(_this + 20) )
  {
    return 1;
  }
  v3 = FUN_0053e8c0(DAT_00563604);
  FUN_0053eba0(_this + 32, v3);
  (*(void (__cdecl **)(DWORD, int))(_this + 840))(*(DWORD *)(_this + 708), _this + 752);
  v4 = *(void **)(_this + 16);
  *(BYTE *)(_this + 20) = 0;
  if ( v4 )
  {
    SetEvent(v4);
  }
  if ( WaitForSingleObject(*(HANDLE *)(_this + 748), 0x7D0u) == 258 )
  {
    v5 = FUN_0053e8c0(DAT_005635ec);
    FUN_0053eba0(_this + 32, v5);
    TerminateThread(*(HANDLE *)(_this + 748), 0);
  }
  CloseHandle(*(HANDLE *)(_this + 748));
  if ( *(DWORD *)(_this + 744) )
  {
    FreeLibrary(*(HMODULE *)(_this + 744));
  }
  *(DWORD *)(_this + 836) = 0;
  *(DWORD *)(_this + 840) = 0;
  v6 = FUN_0053e8c0(DAT_005635d8);
  FUN_0053eba0(_this + 32, v6);
  return 1;
}
#endif
