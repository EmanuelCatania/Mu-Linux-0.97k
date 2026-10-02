// 0x0040A6E0 FUN_0040a6e0 — nunca activado: IDA_PORT_0040A6E0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a6e0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A6E0)
void __cdecl FUN_0040a6e0(DWORD *_this)
{
  *_this = &DAT_00552588;
  WidgetB_SetVtable(_this);
}
#endif
