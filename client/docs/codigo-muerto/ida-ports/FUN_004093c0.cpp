// 0x004093C0 FUN_004093c0 — nunca activado: IDA_PORT_004093C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004093c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004093C0)
void __cdecl FUN_004093c0(void *_this)
{
  *(DWORD *)_this = &DAT_00552548;
  FUN_00408070(_this);
}
#endif
