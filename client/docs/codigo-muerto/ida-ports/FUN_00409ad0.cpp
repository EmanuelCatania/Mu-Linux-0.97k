// 0x00409AD0 FUN_00409ad0 — nunca activado: IDA_PORT_00409AD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00409ad0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409AD0)
DWORD *__cdecl FUN_00409ad0(DWORD *This)
{
  int v2; // eax
  int v3; // eax
  int v4; // ecx

  v2 = operator_new(0xCu);
  if ( v2 )
  {
    *(DWORD *)(v2 + 8) = 0;
    *(DWORD *)(v2 + 4) = 0;
  }
  else
  {
    v2 = 0;
  }
  This[2] = v2;
  v3 = operator_new(0xCu);
  if ( v3 )
  {
    *(DWORD *)(v3 + 8) = 0;
    *(DWORD *)(v3 + 4) = 0;
  }
  else
  {
    v3 = 0;
  }
  v4 = This[2];
  This[3] = v3;
  *(DWORD *)(v4 + 8) = v3;
  *(DWORD *)(This[3] + 4) = This[2];
  This[1] = 0;
  *This = &DAT_00552568;
  CWsctlc::LogPrintOn((DWORD)This);
  return This;
}
#endif
