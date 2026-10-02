// 0x004236C0 FUN_004236c0 — nunca activado: IDA_PORT_004236C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004236c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004236C0)
void __cdecl FUN_004236c0(DWORD *_this, LPVOID *a2)
{
  int v2; // eax

  v2 = *((DWORD *)*a2 + 4);
  if ( v2 )
  {
    if ( *(LPVOID *)(v2 + 8) == *a2 )
    {
      *(DWORD *)(v2 + 8) = 0;
    }
    else
    {
      *(DWORD *)(v2 + 12) = 0;
    }
  }
  else
  {
    _this[2] = 0;
  }
  --_this[1];
  if ( *a2 )
  {
    delete__(*a2);
  }
  *a2 = 0;
}
#endif
