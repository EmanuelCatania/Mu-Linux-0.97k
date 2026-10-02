// 0x00513260 Collision_SegmentToOBB — nunca activado: IDA_PORT_00513260 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj. Usa macros Hex-Rays sin portar (LODWORD/qmemcpy/Vec3_Cross); queda gated. El hover de items del suelo (FUN_004afa40) usa proximidad world-space en su lugar (ver stubs_mouse_hover.cpp).
// ── Collision_SegmentToOBB (IDA-only, gated) ──
// Usa macros Hex-Rays sin portar (LODWORD/qmemcpy/Vec3_Cross); queda gated.
// El hover de items del suelo (FUN_004afa40) usa proximidad world-space en su
// lugar (ver stubs_mouse_hover.cpp).
#if defined(IDA_PORT_00513260)
bool __cdecl Collision_SegmentToOBB(
        float a1,
        int a2,
        char a3,
        int a4,
        int a5,
        float a6,
        int a7,
        int a8,
        float a9,
        int a10,
        int a11,
        float a12)
{
  double v12; // st7
  double v13; // st7
  bool result; // al
  float v15[12]; // [esp-38h] [ebp-70h] BYREF
  float v16[3]; // [esp+8h] [ebp-30h] BYREF
  float v17[3]; // [esp+14h] [ebp-24h] BYREF
  float v18[3]; // [esp+20h] [ebp-18h] BYREF
  float v19[3]; // [esp+2Ch] [ebp-Ch] BYREF

  v12 = *(float *)a2 - *(float *)LODWORD(a1);
  LODWORD(v15[11]) = v17;
  v16[0] = v12;
  v13 = *(float *)(a2 + 4) - *(float *)(LODWORD(a1) + 4);
  LODWORD(v15[10]) = &a6;
  LODWORD(v15[9]) = v16;
  v16[1] = v13;
  v16[2] = *(float *)(a2 + 8) - *(float *)(LODWORD(a1) + 8);
  Vec3_Cross(v16, &a6, v17);
  Vec3_Cross(v16, &a9, v18);
  Vec3_Cross(v16, &a12, v19);
  qmemcpy(v15, &a3, sizeof(v15));
  result = FUN_005130f0(
             COERCE_FLOAT(v17),
             a1,
             a2,
             v15[0],
             v15[1],
             v15[2],
             v15[3],
             v15[4],
             v15[5],
             v15[6],
             v15[7],
             v15[8],
             v15[9],
             v15[10],
             v15[11]);
  if ( result )
  {
    qmemcpy(v15, &a3, sizeof(v15));
    result = FUN_005130f0(
               COERCE_FLOAT(v18),
               a1,
               a2,
               v15[0],
               v15[1],
               v15[2],
               v15[3],
               v15[4],
               v15[5],
               v15[6],
               v15[7],
               v15[8],
               v15[9],
               v15[10],
               v15[11]);
    if ( result )
    {
      qmemcpy(v15, &a3, sizeof(v15));
      result = FUN_005130f0(
                 COERCE_FLOAT(v19),
                 a1,
                 a2,
                 v15[0],
                 v15[1],
                 v15[2],
                 v15[3],
                 v15[4],
                 v15[5],
                 v15[6],
                 v15[7],
                 v15[8],
                 v15[9],
                 v15[10],
                 v15[11]);
      if ( result )
      {
        qmemcpy(v15, &a3, sizeof(v15));
        result = FUN_005130f0(
                   COERCE_FLOAT(&a6),
                   a1,
                   a2,
                   v15[0],
                   v15[1],
                   v15[2],
                   v15[3],
                   v15[4],
                   v15[5],
                   v15[6],
                   v15[7],
                   v15[8],
                   v15[9],
                   v15[10],
                   v15[11]);
        if ( result )
        {
          qmemcpy(v15, &a3, sizeof(v15));
          result = FUN_005130f0(
                     COERCE_FLOAT(&a9),
                     a1,
                     a2,
                     v15[0],
                     v15[1],
                     v15[2],
                     v15[3],
                     v15[4],
                     v15[5],
                     v15[6],
                     v15[7],
                     v15[8],
                     v15[9],
                     v15[10],
                     v15[11]);
          if ( result )
          {
            qmemcpy(v15, &a3, sizeof(v15));
            return FUN_005130f0(
                     COERCE_FLOAT(&a12),
                     a1,
                     a2,
                     v15[0],
                     v15[1],
                     v15[2],
                     v15[3],
                     v15[4],
                     v15[5],
                     v15[6],
                     v15[7],
                     v15[8],
                     v15[9],
                     v15[10],
                     v15[11]);
          }
        }
      }
    }
  }
  return result;
}
#endif
