// 0x00407980 SpringNode_ZeroFields — nunca activado: IDA_PORT_00407980 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SpringNode_ZeroFields (IDA-only, gated) ──
#if defined(IDA_PORT_00407980)
DWORD *__cdecl SpringNode_ZeroFields(int _this)
{
  DWORD *result; // eax
  int v2; // esi

  *(DWORD *)(_this + 44) = 0;
  result = (DWORD *)(_this + 28);
  v2 = 3;
  do
  {
    result[5] = 0;
    *result = 0;
    *(result - 3) = 0;
    *(result - 6) = 0;
    ++result;
    --v2;
  }
  while ( v2 );
  *(BYTE *)(_this + 40) = 0;
  return result;
}
#endif
