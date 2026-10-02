// 0x00406D40 FUN_00406d40 — nunca activado: IDA_PORT_00406D40 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406d40 (IDA-only, gated) ──
#if defined(IDA_PORT_00406D40)
void __cdecl FUN_00406d40(int _this)
{
  unsigned int i; // edi
  void *v3; // eax

  for ( i = 0; i < 0x1000; i += 4 )
  {
    if ( *(DWORD *)(*(DWORD *)(_this + 4) + i) )
    {
      delete__(*(LPVOID *)(*(DWORD *)(_this + 4) + i));
      *(DWORD *)(*(DWORD *)(_this + 4) + i) = 0;
    }
  }
  if ( *(DWORD *)(_this + 24) )
  {
    delete__(*(LPVOID *)(_this + 24));
  }
  v3 = *(void **)(_this + 36);
  *(DWORD *)(_this + 4) = *(DWORD *)(_this + 32);
  *(DWORD *)(_this + 8) = v3;
  delete__(v3);
  delete__(*(LPVOID *)(_this + 4));
  *(DWORD *)(_this + 12) = 0;
}
#endif
