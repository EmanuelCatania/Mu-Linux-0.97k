// 0x00409ED0 WidgetB_Ctor — nunca activado: IDA_PORT_00409ED0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── WidgetB_Ctor (IDA-only, gated) ──
#if defined(IDA_PORT_00409ED0)
DWORD *__cdecl WidgetB_Ctor(DWORD *_this)
{
  *_this = &DAT_00552574;
  WidgetB_ZeroFields();
  return _this;
}
#endif
