// 0x004122C0 FUN_004122c0 — nunca activado: IDA_PORT_004122C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004122c0 (IDA-only, gated) ──
#if defined(IDA_PORT_004122C0)
int __cdecl FUN_004122c0(DWORD *_this)
{
  int v2; // ebx
  int result; // eax
  int v4; // edi
  DWORD **v5; // [esp-4h] [ebp-10h]

  v2 = _this[24];
  result = _this[33];
  if ( v2 >= result )
  {
    v4 = 0;
    if ( v2 - result > 0 )
    {
      do
      {
        v5 = *(DWORD ***)(_this[23] + 4);
        *v5[1] = *v5;
        (*v5)[1] = v5[1];
        delete__(v5);
        --_this[24];
        ++v4;
        result = v2 - _this[33];
      }
      while ( v4 < result );
    }
  }
  return result;
}
#endif
