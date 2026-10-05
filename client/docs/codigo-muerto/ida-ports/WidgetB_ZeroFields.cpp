// 0x00409F20 WidgetB_ZeroFields — nunca activado: IDA_PORT_00409F20 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── WidgetB_ZeroFields (IDA-only, gated) ──
#if defined(IDA_PORT_00409F20)
int __cdecl WidgetB_ZeroFields(int _this)
{
  int result; // eax

  result = 0;
  *(WORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = 0;
  *(DWORD *)(_this + 24) = 0;
  *(DWORD *)(_this + 28) = 0;
  return result;
}
#endif
