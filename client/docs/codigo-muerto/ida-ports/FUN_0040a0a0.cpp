// 0x0040A0A0 FUN_0040a0a0 — nunca activado: IDA_PORT_0040A0A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a0a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A0A0)
void __cdecl FUN_0040a0a0(int a1, int a2, int a3, int a4, int a5, char a6)
{
  *(DWORD *)(a1 + 16) = 1022739087;
  *(DWORD *)(a1 + 20) = -1082130432;
  *(DWORD *)(a1 + 12) = -1082130432;
  Vec3_Normalize(a1 + 12);
  if ( FUN_00409f30(a1, a2, a3, a4, a5, a6) )
  {
    FUN_0040a300(a3);
    delete__(*(LPVOID *)(a1 + 28));
  }
}
#endif
