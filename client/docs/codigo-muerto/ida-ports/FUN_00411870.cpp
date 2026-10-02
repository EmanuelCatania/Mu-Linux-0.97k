// 0x00411870 FUN_00411870 — nunca activado: IDA_PORT_00411870 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411870 (IDA-only, gated) ──
#if defined(IDA_PORT_00411870)
DWORD *__cdecl FUN_00411870(void *_this)
{
  DWORD *v1; // eax
  DWORD *result; // eax
  DWORD *v3; // edx

  v1 = *(DWORD **)_this;
  if ( *(DWORD *)(*(DWORD *)_this + 308) || *(DWORD **)(v1[1] + 4) != v1 )
  {
    v3 = (DWORD *)*v1;
    if ( *v1 == DAT_055c9b98 )
    {
      for ( result = (DWORD *)v1[1]; *(DWORD *)_this == *result; result = (DWORD *)result[1] )
      {
        *(DWORD *)_this = result;
      }
      *(DWORD *)_this = result;
    }
    else
    {
      for ( result = (DWORD *)v3[2]; result != (DWORD *)DAT_055c9b98; result = (DWORD *)result[2] )
      {
        v3 = result;
      }
      *(DWORD *)_this = v3;
    }
  }
  else
  {
    result = (DWORD *)v1[2];
    *(DWORD *)_this = result;
  }
  return result;
}
#endif
