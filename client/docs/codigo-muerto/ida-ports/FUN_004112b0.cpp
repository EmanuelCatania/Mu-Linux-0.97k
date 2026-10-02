// 0x004112B0 FUN_004112b0 — nunca activado: IDA_PORT_004112B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004112b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004112B0)
DWORD *__cdecl FUN_004112b0(void *_this)
{
  DWORD *v1; // edx
  DWORD *result; // eax

  v1 = *(DWORD **)(*(DWORD *)_this + 8);
  if ( v1 == (DWORD *)DAT_055c9b98 )
  {
    for ( result = *(DWORD **)(*(DWORD *)_this + 4); *(DWORD *)_this == result[2]; result = (DWORD *)result[1] )
    {
      *(DWORD *)_this = result;
    }
    if ( *(DWORD **)(*(DWORD *)_this + 8) != result )
    {
      *(DWORD *)_this = result;
    }
  }
  else
  {
    for ( result = (DWORD *)*v1; result != (DWORD *)DAT_055c9b98; result = (DWORD *)*result )
    {
      v1 = result;
    }
    *(DWORD *)_this = v1;
  }
  return result;
}
#endif
