// 0x00410D90 FUN_00410d90 — nunca activado: IDA_PORT_00410D90 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410d90 (IDA-only, gated) ──
#if defined(IDA_PORT_00410D90)
int __cdecl FUN_00410d90(int _this)
{
  DWORD **v2; // ebx
  DWORD *i; // esi
  DWORD **v4; // eax
  int result; // eax

  v2 = *(DWORD ***)(_this + 4);
  for ( i = *v2; i != v2; --*(DWORD *)(_this + 8) )
  {
    v4 = (DWORD **)i;
    i = (DWORD *)*i;
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
  }
  delete__(*(LPVOID *)(_this + 4));
  result = 0;
  *(DWORD *)(_this + 4) = 0;
  *(DWORD *)(_this + 8) = 0;
  return result;
}
#endif
