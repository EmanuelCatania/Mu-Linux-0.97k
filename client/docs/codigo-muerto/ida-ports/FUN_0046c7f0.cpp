// 0x0046C7F0 FUN_0046c7f0 — nunca activado: IDA_PORT_0046C7F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0046c7f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0046C7F0)
void __cdecl FUN_0046c7f0(int a1, float *a2, float a3, float a4, float a5)
{
  float *v5; // edi
  bool v6; // zf
  signed int v7; // eax
  bool v8; // zf
  signed int v9; // eax
  double v10; // st7
  bool v11; // zf
  signed int v12; // eax
  int v13; // eax
  float out[3]; // [esp+8h] [ebp-54h] BYREF
  float Light[3]; // [esp+14h] [ebp-48h] BYREF
  float in1[3]; // [esp+20h] [ebp-3Ch] BYREF
  float in2[3][4]; // [esp+2Ch] [ebp-30h] BYREF

  v5 = a2 + 7;
  AngleMatrix(a2 + 7, in2);
  in1[0] = a3;
  in1[1] = a4;
  in1[2] = a5;
  VectorRotate(in1, in2, out);
  out[0] = out[0] + a2[4];
  out[1] = out[1] + a2[5];
  out[2] = out[2] + a2[6];
  out[0] = (double)(rand() % 16 - 8) + out[0];
  out[1] = (double)(rand() % 16 - 8) + out[1];
  out[2] = (double)(rand() % 16 - 8) + out[2];
  if ( a1 )
  {
    if ( a1 == 1 )
    {
      v9 = rand() & 0x80000001;
      v8 = v9 == 0;
      if ( v9 < 0 )
      {
        v8 = (((BYTE)v9 - 1) | 0xFFFFFFFE) == -1;
      }
      if ( v8 )
      {
        Particle_Spawn(1220, out, v5, a2 + 58, 0, 1.0, 0);
      }
    }
    else if ( a1 == 2 )
    {
      v7 = rand() & 0x80000001;
      v6 = v7 == 0;
      if ( v7 < 0 )
      {
        v6 = (((BYTE)v7 - 1) | 0xFFFFFFFE) == -1;
      }
      if ( v6 )
      {
        Particle_Spawn(1220, out, v5, a2 + 58, 2, 1.0, 0);
      }
    }
  }
  else
  {
    v10 = (double)(rand() % 6 + 6) * 0.1;
    Light[0] = v10;
    Light[1] = v10 * 0.60000002;
    Light[2] = v10 * 0.40000001;
    v12 = rand() & 0x80000001;
    v11 = v12 == 0;
    if ( v12 < 0 )
    {
      v11 = (((BYTE)v12 - 1) | 0xFFFFFFFE) == -1;
    }
    if ( v11 )
    {
      v13 = rand() % 4;
      Particle_Spawn(1195, out, v5, Light, v13, 1.0, 0);
    }
    AddTerrainLight(out[0], out[1], (float*)Light, 4, (float*)PrimaryTerrainLight[0]);
  }
}
#endif
