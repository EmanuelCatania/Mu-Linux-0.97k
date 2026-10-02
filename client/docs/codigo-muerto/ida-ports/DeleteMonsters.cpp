// 0x00509880 DeleteMonsters — nunca activado: IDA_PORT_00509880 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── DeleteMonsters (IDA-only, gated) ──
#if defined(IDA_PORT_00509880)
void __cdecl DeleteMonsters()
{
  int i; // esi
  int j; // esi

  for ( i = 50760; i < 62980; i += 188 )
  {
    BMD::Release(i + Models);
  }
  for ( j = 170; j < 420; ++j )
  {
    ReleaseBuffer(j);
  }
}
#endif
