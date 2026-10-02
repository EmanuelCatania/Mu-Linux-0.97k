// 0x00409B80 Locimp_dtor — nunca activado: IDA_PORT_00409B80 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Locimp_dtor (IDA-only, gated) ──
#if defined(IDA_PORT_00409B80)
void __cdecl std::locale::_Locimp::~_Locimp(std::locale::_Locimp *_this)
{
  DWORD *v2; // esi
  void *i; // eax
  void *v4; // eax
  void *v5; // edi

  *(DWORD *)_this = &DAT_00552568;
  FUN_00409d20();
  *(DWORD *)(*(DWORD *)(*((DWORD *)_this + 3) + 4) + 8) = 0;
  v2 = *(DWORD **)(*((DWORD *)_this + 2) + 8);
  for ( i = v2; v2; i = v2 )
  {
    v2 = (DWORD *)v2[2];
    if ( i )
    {
      delete__(i);
    }
  }
  *(DWORD *)(*((DWORD *)_this + 2) + 8) = *((DWORD *)_this + 3);
  *(DWORD *)(*((DWORD *)_this + 3) + 4) = *((DWORD *)_this + 2);
  v4 = (void *)*((DWORD *)_this + 3);
  *((DWORD *)_this + 1) = 0;
  if ( v4 )
  {
    delete__(v4);
  }
  v5 = (void *)*((DWORD *)_this + 2);
  if ( v5 )
  {
    delete__(v5);
  }
}
#endif
