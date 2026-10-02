// 0x00411700 FUN_00411700 — nunca activado: IDA_PORT_00411700 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411700 (IDA-only, gated) ──
#if defined(IDA_PORT_00411700)
DWORD *__cdecl FUN_00411700(DWORD *_this, int a2)
{
  DWORD *result; // eax
  int v3; // ecx
  DWORD *v4; // ecx

  result = *(DWORD **)(a2 + 8);
  *(DWORD *)(a2 + 8) = *result;
  if ( *result != DAT_055c9b98 )
  {
    *(DWORD *)(*result + 4) = a2;
  }
  result[1] = *(DWORD *)(a2 + 4);
  v3 = _this[1];
  if ( a2 == *(DWORD *)(v3 + 4) )
  {
    *(DWORD *)(v3 + 4) = result;
    *result = a2;
    *(DWORD *)(a2 + 4) = result;
  }
  else
  {
    v4 = *(DWORD **)(a2 + 4);
    if ( a2 == *v4 )
    {
      *v4 = result;
    }
    else
    {
      v4[2] = result;
    }
    *result = a2;
    *(DWORD *)(a2 + 4) = result;
  }
  return result;
}
#endif
