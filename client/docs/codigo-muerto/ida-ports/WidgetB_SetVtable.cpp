// 0x00409F10 WidgetB_SetVtable — nunca activado: IDA_PORT_00409F10 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── WidgetB_SetVtable (IDA-only, gated) ──
#if defined(IDA_PORT_00409F10)
void __cdecl WidgetB_SetVtable(DWORD *_this)
{
  *_this = &DAT_00552574;
}
#endif
