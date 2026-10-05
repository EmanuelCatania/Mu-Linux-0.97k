// 0x00411760 FUN_00411760 — nunca activado: IDA_PORT_00411760 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411760 (IDA-only, gated) ──
#if defined(IDA_PORT_00411760)
int __cdecl FUN_00411760(DWORD *_this, DWORD *a2)
{
  int result; // eax
  int v3; // esi
  int v4; // ecx
  DWORD *v5; // ecx

  result = *a2;
  *a2 = *(DWORD *)(*a2 + 8);
  v3 = *(DWORD *)(result + 8);
  if ( v3 != DAT_055c9b98 )
  {
    *(DWORD *)(v3 + 4) = a2;
  }
  *(DWORD *)(result + 4) = a2[1];
  v4 = _this[1];
  if ( a2 == *(DWORD **)(v4 + 4) )
  {
    *(DWORD *)(v4 + 4) = result;
    *(DWORD *)(result + 8) = a2;
    a2[1] = result;
  }
  else
  {
    v5 = (DWORD *)a2[1];
    if ( a2 == (DWORD *)v5[2] )
    {
      v5[2] = result;
    }
    else
    {
      *v5 = result;
    }
    *(DWORD *)(result + 8) = a2;
    a2[1] = result;
  }
  return result;
}
#endif
