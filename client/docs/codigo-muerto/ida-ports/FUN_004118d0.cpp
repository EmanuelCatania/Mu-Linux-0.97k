// 0x004118D0 FUN_004118d0 — nunca activado: IDA_PORT_004118D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004118d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004118D0)
void __cdecl FUN_004118d0(DWORD *_this)
{
  DWORD **v2; // ebx
  DWORD *i; // esi
  DWORD **v4; // eax

  v2 = (DWORD **)_this[23];
  for ( i = *v2; i != v2; --_this[24] )
  {
    v4 = (DWORD **)i;
    i = (DWORD *)*i;
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
  }
  _this[34] = 0;
}
#endif
