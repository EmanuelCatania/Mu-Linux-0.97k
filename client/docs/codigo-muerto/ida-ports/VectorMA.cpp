// 0x004F9CE0 VectorMA — nunca activado: IDA_PORT_004F9CE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── VectorMA (IDA-only, gated) ──
#if defined(IDA_PORT_004F9CE0)
void __cdecl VectorMA(float va[3], float scale, float vb[3], float vc[3])
{
  *vc = scale * *vb + *va;
  vc[1] = scale * vb[1] + va[1];
  vc[2] = scale * vb[2] + va[2];
}
#endif
