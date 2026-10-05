// 0x005130F0 FUN_005130f0 — nunca activado: IDA_PORT_005130F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005130f0 (IDA-only, gated) ──
#if defined(IDA_PORT_005130F0)
bool __cdecl FUN_005130f0(
        float a1,
        float a2,
        int a3,
        float a4,
        float a5,
        float a6,
        float a7,
        float a8,
        float a9,
        float a10,
        float a11,
        float a12,
        float a13,
        float a14,
        float a15)
{
  double v16; // st7
  double v17; // st6
  float v19; // [esp+Ch] [ebp-Ch]
  float v20; // [esp+10h] [ebp-8h]
  float v21; // [esp+14h] [ebp-4h]
  float v22; // [esp+1Ch] [ebp+4h]
  float v23; // [esp+1Ch] [ebp+4h]
  float v24; // [esp+20h] [ebp+8h]
  float v25; // [esp+20h] [ebp+8h]
  int v26; // [esp+24h] [ebp+Ch]
  float v27; // [esp+28h] [ebp+10h]

  v24 = *(float *)(LODWORD(a2) + 4) * *(float *)(LODWORD(a1) + 4)
      + *(float *)(LODWORD(a2) + 8) * *(float *)(LODWORD(a1) + 8)
      + *(float *)LODWORD(a1) * *(float *)LODWORD(a2);
  v22 = *(float *)(a3 + 4) * *(float *)(LODWORD(a1) + 4)
      + *(float *)(a3 + 8) * *(float *)(LODWORD(a1) + 8)
      + *(float *)LODWORD(a1) * *(float *)a3;
  v21 = Math_Fmax(v24, v22);
  v20 = Math_Fmin(v24, v22);
  v16 = a5 * *(float *)(LODWORD(a1) + 4) + a4 * *(float *)LODWORD(a1) + a6 * *(float *)(LODWORD(a1) + 8);
  v19 = v16;
  v23 = a8 * *(float *)(LODWORD(a1) + 4) + a7 * *(float *)LODWORD(a1) + a9 * *(float *)(LODWORD(a1) + 8);
  v25 = a11 * *(float *)(LODWORD(a1) + 4) + a10 * *(float *)LODWORD(a1) + a12 * *(float *)(LODWORD(a1) + 8);
  *(float *)&v26 = a14 * *(float *)(LODWORD(a1) + 4) + a13 * *(float *)LODWORD(a1) + a15 * *(float *)(LODWORD(a1) + 8);
  v27 = v16;
  if ( v23 <= 0.0 )
  {
    v16 = v23 + v19;
    v17 = v27;
  }
  else
  {
    v17 = v23 + v19;
  }
  if ( v25 <= 0.0 )
  {
    v16 = v16 + v25;
  }
  else
  {
    v17 = v17 + v25;
  }
  if ( *(float *)&v26 <= 0.0 )
  {
    v16 = v16 + *(float *)&v26;
  }
  else
  {
    v17 = v17 + *(float *)&v26;
  }
  return v20 <= v17 && v16 <= v21;
}
#endif
