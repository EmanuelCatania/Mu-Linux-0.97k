// 0x00540A70 FUN_00540a70 — nunca activado: IDA_PORT_00540A70 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00540a70 (IDA-only, gated) ──
#if defined(IDA_PORT_00540A70)
HCRYPTPROV *__cdecl FUN_00540a70(HCRYPTPROV *phProv)
{
  phProv[5] = 0;
  phProv[3] = 0;
  *phProv = 0;
  phProv[1] = 0;
  phProv[2] = 0;
  phProv[9] = 0;
  phProv[14] = 0;
  phProv[16] = 0;
  phProv[15] = 0;
  phProv[6] = 0;
  *((BYTE *)phProv + 28) = 0;
  phProv[8] = 0;
  phProv[17] = 0;
  if ( !CryptAcquireContextA(phProv, 0, szProvider, 1u, 0xF0000000) )
  {
    *phProv = 0;
  }
  return phProv;
}
#endif
