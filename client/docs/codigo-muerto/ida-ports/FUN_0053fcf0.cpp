// 0x0053FCF0 FUN_0053fcf0 — nunca activado: IDA_PORT_0053FCF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053fcf0 (IDA-only, gated) ──
#if defined(IDA_PORT_0053FCF0)
char FUN_0053fcf0()
{
  char result; // al
  const CHAR *v1; // eax
  HMODULE ModuleHandleA; // esi
  const CHAR *v3; // eax
  const CHAR *v4; // eax
  const CHAR *v5; // eax
  const CHAR *v6; // eax
  const CHAR *v7; // eax

  if ( DAT_083bbb18 )
  {
    return 1;
  }
  v1 = FUN_0053e8c0(DAT_005639a8);
  ModuleHandleA = GetModuleHandleA(v1);
  if ( !ModuleHandleA )
  {
    return 0;
  }
  v3 = FUN_0053e8c0(DAT_00563988);
  DAT_083bbae4 = (int)GetProcAddress(ModuleHandleA, v3);
  if ( !DAT_083bbae4 )
  {
    return 0;
  }
  v4 = FUN_0053e8c0(DAT_00563974);
  DAT_083bb9d8 = GetProcAddress(ModuleHandleA, v4);
  if ( !DAT_083bb9d8 )
  {
    return 0;
  }
  v5 = FUN_0053e8c0(DAT_00563960);
  DAT_083bbaec = (int)GetProcAddress(ModuleHandleA, v5);
  if ( !DAT_083bbaec )
  {
    return 0;
  }
  v6 = FUN_0053e8c0(DAT_0056394c);
  DAT_083bbae8 = (int)GetProcAddress(ModuleHandleA, v6);
  if ( !DAT_083bbae8 )
  {
    return 0;
  }
  v7 = FUN_0053e8c0(DAT_00563938);
  DAT_083bb9dc = GetProcAddress(ModuleHandleA, v7);
  if ( !DAT_083bb9dc )
  {
    return 0;
  }
  result = 1;
  DAT_083bbb18 = 1;
  return result;
}
#endif
