// 0x00452030 FUN_00452030 — nunca activado: IDA_PORT_00452030 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00452030 (IDA-only, gated) ──
#if defined(IDA_PORT_00452030)
void __cdecl FUN_00452030(int a1)
{
  float Position[3]; // [esp+4h] [ebp-Ch] BYREF
  int v3; // [esp+14h] [ebp+4h]

  if ( *(BYTE *)(a1 + 261) == 2 )
  {
    Position[0] = (double)(rand() % 200) + *(float *)(a1 + 16) - 100.0;
    v3 = rand() % 200;
    Position[2] = *(float *)(a1 + 24);
    Position[1] = (double)v3 + *(float *)(a1 + 20) - 100.0;
    Particle_Spawn(1221, Position, (float *)(a1 + 28), (float *)(a1 + 232), 0, 1.0, 0);
  }
}
#endif
