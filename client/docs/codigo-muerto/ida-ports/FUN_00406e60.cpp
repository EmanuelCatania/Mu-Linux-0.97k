// 0x00406E60 FUN_00406e60 — nunca activado: IDA_PORT_00406E60 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406e60 (IDA-only, gated) ──
#if defined(IDA_PORT_00406E60)
void __cdecl FUN_00406e60(int _this)
{
  delete__(*(LPVOID *)(_this + 8));
  delete__(*(LPVOID *)(_this + 4));
  *(DWORD *)(_this + 12) = 0;
}
#endif
