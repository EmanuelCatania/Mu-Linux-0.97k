// 0x00411300 FUN_00411300 — nunca activado: IDA_PORT_00411300 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411300 (IDA-only, gated) ──
#if defined(IDA_PORT_00411300)
DWORD *__cdecl FUN_00411300(DWORD *_this, DWORD *a2, DWORD *a3, DWORD *a4)
{
  DWORD *v5; // edi
  DWORD *v6; // eax
  DWORD *v7; // ecx
  DWORD *v8; // ecx

  v5 = (DWORD *)a3[1];
  v6 = (DWORD *)operator_new(0xCu);
  v7 = a3;
  if ( !a3 )
  {
    v7 = v6;
  }
  *v6 = v7;
  v8 = v5;
  if ( !v5 )
  {
    v8 = v6;
  }
  v6[1] = v8;
  a3[1] = v6;
  *(DWORD *)v6[1] = v6;
  if ( v6 != (DWORD *)-8 )
  {
    v6[2] = *a4;
  }
  ++_this[2];
  *a2 = v6;
  return a2;
}
#endif
