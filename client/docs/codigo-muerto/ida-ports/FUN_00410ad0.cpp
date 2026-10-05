// 0x00410AD0 FUN_00410ad0 — nunca activado: IDA_PORT_00410AD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410ad0 (IDA-only, gated) ──
#if defined(IDA_PORT_00410AD0)
void __cdecl FUN_00410ad0(DWORD *_this)
{
  *_this = &DAT_00552810;
  FUN_0040f690(_this);
}
#endif
