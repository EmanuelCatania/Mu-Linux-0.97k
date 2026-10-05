// 0x00509190 DeleteNpcs — nunca activado: IDA_PORT_00509190 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── DeleteNpcs (IDA-only, gated) ──
#if defined(IDA_PORT_00509190)
void __cdecl DeleteNpcs()
{
  int i; // esi
  int j; // esi

  for ( i = 62980; i < 71440; i += 188 )
  {
    BMD::Release(i + Models);
  }
  for ( j = 120; j < 170; ++j )
  {
    ReleaseBuffer(j);
  }
}
#endif
