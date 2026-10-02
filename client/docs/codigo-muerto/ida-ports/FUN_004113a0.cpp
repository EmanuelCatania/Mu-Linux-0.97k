// 0x004113A0 FUN_004113a0 — nunca activado: IDA_PORT_004113A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004113a0 (IDA-only, gated) ──
#if defined(IDA_PORT_004113A0)
DWORD *__cdecl FUN_004113a0(DWORD *_this, DWORD *a2, DWORD *a3)
{
  DWORD *v3; // ecx
  DWORD *v4; // eax
  DWORD *result; // eax

  v3 = (DWORD *)_this[1];
  v4 = (DWORD *)v3[1];
  if ( v4 == (DWORD *)DAT_055c9b98 )
  {
    result = a2;
    *a2 = v3;
  }
  else
  {
    do
    {
      if ( v4[3] >= *a3 )
      {
        v3 = v4;
        v4 = (DWORD *)*v4;
      }
      else
      {
        v4 = (DWORD *)v4[2];
      }
    }
    while ( v4 != (DWORD *)DAT_055c9b98 );
    result = a2;
    *a2 = v3;
  }
  return result;
}
#endif
