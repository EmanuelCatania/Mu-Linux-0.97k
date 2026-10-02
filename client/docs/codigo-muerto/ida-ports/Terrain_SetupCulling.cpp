// 0x004F9C20 Terrain_SetupCulling — nunca activado: IDA_PORT_004F9C20 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Terrain_SetupCulling (IDA-only, gated) ──
#if defined(IDA_PORT_004F9C20)
void __cdecl Terrain_SetupCulling(bool EditFlag)
{
  double v1; // st7
  double v2; // st7

  FUN_004f98c0(
    (__int64)(*(float *)(Hero + 16) * 0.039999999),
    (__int64)(*(float *)(Hero + 20) * 0.039999999),
    3,
    -70,
    DAT_0839bc88);
  v1 = WorldTime;
  if ( World == 8 )
  {
    v2 = (double)((int)(__int64)v1 % 40000) * 0.000024999999;
  }
  else
  {
    v2 = (double)((int)(__int64)v1 % 20000) * 0.000049999999;
  }
  WaterMove = v2;
  if ( EditFlag )
  {
    SelectFlag = 0;
    Map_InitRayCast();
  }
  else
  {
    DisableAlphaBlend();
  }
  TerrainFlag = 0;
  RenderTerrainFrustrum(EditFlag);
  if ( EditFlag )
  {
    if ( SelectFlag )
    {
      RenderTerrainTile(SelectXF, SelectYF, (__int64)SelectXF, (__int64)SelectYF, 1.0, 1, EditFlag);
    }
  }
  else
  {
    EnableAlphaTest(1);
    if ( DAT_0055a76c && World != 7 )
    {
      TerrainFlag = 2;
      RenderTerrainFrustrum(0);
    }
    Terrain_SpawnAmbientObjects();
    DisableDepthTest();
    EnableCullFace();
    RenderTerrainAlphaBitmaps();
    EnableDepthTest();
  }
  DAT_0839bc88 ^= 1u;
  Terrain_WaterWaveUpdate(DAT_0839bc88);
}
#endif
