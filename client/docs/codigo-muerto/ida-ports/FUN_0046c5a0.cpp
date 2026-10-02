// 0x0046C5A0 FUN_0046c5a0 — nunca activado: IDA_PORT_0046C5A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0046c5a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0046C5A0)
int __cdecl FUN_0046c5a0(int a1, int a2, float Position[3], float angles[3])
{
  int v4; // ebx
  int result; // eax
  float out[3]; // [esp+Ch] [ebp-54h] BYREF
  float Light[3]; // [esp+18h] [ebp-48h] BYREF
  float in1[3]; // [esp+24h] [ebp-3Ch] BYREF
  float in2[3][4]; // [esp+30h] [ebp-30h] BYREF

  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  Particle_Spawn(1176, Position, (float *)(a2 + 28), Light, 0, 1.0, 0);
  in1[0] = 0.0;
  in1[1] = 50.0;
  in1[2] = 0.0;
  AngleMatrix(angles, in2);
  VectorRotate(in1, in2, out);
  v4 = 20;
  out[0] = out[0] + *Position;
  out[1] = out[1] + Position[1];
  out[2] = out[2] + Position[2];
  do
  {
    rand();
    rand();
    result = Particle_Spawn(1175, Position, (float *)(a2 + 28), Light, 0, 1.0, 0);
    --v4;
  }
  while ( v4 );
  return result;
}
#endif
