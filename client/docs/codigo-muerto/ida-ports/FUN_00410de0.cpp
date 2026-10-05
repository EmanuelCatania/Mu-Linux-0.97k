// 0x00410DE0 FUN_00410de0 — nunca activado: IDA_PORT_00410DE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410de0 (IDA-only, gated) ──
#if defined(IDA_PORT_00410DE0)
DWORD *__cdecl FUN_00410de0(DWORD *_this, DWORD *a2, DWORD *lpMem, DWORD *a4)
{
  DWORD *i; // esi
  DWORD **v6; // eax
  DWORD *result; // eax

  for ( i = lpMem; i != a4; --_this[2] )
  {
    v6 = (DWORD **)i;
    i = (DWORD *)*i;
    *v6[1] = *v6;
    (*v6)[1] = v6[1];
    delete__(v6);
  }
  result = a2;
  *a2 = i;
  return result;
}
#endif
