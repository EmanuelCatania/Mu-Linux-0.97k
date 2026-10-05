// 0x00408070 FUN_00408070 — nunca activado: IDA_PORT_00408070 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00408070 (IDA-only, gated) ──
#if defined(IDA_PORT_00408070)
void __cdecl FUN_00408070(void *_this)
{
  DWORD *v2; // esi
  void *i; // eax
  void *v4; // eax
  void *v5; // edi

  *(DWORD *)_this = &DAT_00552520;
  *(DWORD *)(*(DWORD *)(*((DWORD *)_this + 20) + 4) + 8) = 0;
  v2 = *(DWORD **)(*((DWORD *)_this + 19) + 8);
  for ( i = v2; v2; i = v2 )
  {
    v2 = (DWORD *)v2[2];
    if ( i )
    {
      delete__(i);
    }
  }
  *(DWORD *)(*((DWORD *)_this + 19) + 8) = *((DWORD *)_this + 20);
  *(DWORD *)(*((DWORD *)_this + 20) + 4) = *((DWORD *)_this + 19);
  v4 = (void *)*((DWORD *)_this + 20);
  *((DWORD *)_this + 18) = 0;
  if ( v4 )
  {
    delete__(v4);
  }
  v5 = (void *)*((DWORD *)_this + 19);
  if ( v5 )
  {
    delete__(v5);
  }
}
#endif
