// 0x00411360 FUN_00411360 — nunca activado: IDA_PORT_00411360 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411360 (IDA-only, gated) ──
#if defined(IDA_PORT_00411360)
DWORD *__cdecl FUN_00411360(DWORD *_this, DWORD *a2, DWORD **lpMem)
{
  DWORD *v4; // edi
  DWORD *result; // eax

  v4 = *lpMem;
  *lpMem[1] = *lpMem;
  (*lpMem)[1] = lpMem[1];
  delete__(lpMem);
  --_this[2];
  result = a2;
  *a2 = v4;
  return result;
}
#endif
