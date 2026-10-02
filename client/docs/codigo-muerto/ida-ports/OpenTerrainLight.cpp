// 0x004F7250 OpenTerrainLight — nunca activado: IDA_PORT_004F7250 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── OpenTerrainLight (IDA-only, gated) ──
#if defined(IDA_PORT_004F7250)
void __cdecl OpenTerrainLight(char *FileName)
{
  OpenJpegBuffer(FileName, &TerrainLight);
  CreateTerrainNormal();
  CreateTerrainLight();
}
#endif
