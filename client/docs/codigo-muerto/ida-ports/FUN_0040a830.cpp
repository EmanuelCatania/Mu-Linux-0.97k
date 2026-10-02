// 0x0040A830 FUN_0040a830 — nunca activado: IDA_PORT_0040A830 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a830 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A830)
void __cdecl FUN_0040a830(int _this)
{
  void *v2; // esi

  if ( *(DWORD *)(_this + 28) )
  {
    delete__(*(LPVOID *)(_this + 28));
  }
  v2 = *(void **)(_this + 8);
  if ( v2 )
  {
    delete__(v2);
  }
}
#endif
