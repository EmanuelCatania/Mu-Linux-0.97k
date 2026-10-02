// 0x00411840 FUN_00411840 — nunca activado: IDA_PORT_00411840 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411840 (IDA-only, gated) ──
#if defined(IDA_PORT_00411840)
int __stdcall FUN_00411840(int a1, int a2)
{
  int result; // eax

  result = operator_new(0x138u);
  *(DWORD *)(result + 4) = a1;
  *(DWORD *)(result + 308) = a2;
  return result;
}
#endif
