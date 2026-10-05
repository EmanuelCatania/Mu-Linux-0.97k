// 0x004F9D20 Vec3_Cross — nunca activado: IDA_PORT_004F9D20 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Vec3_Cross (IDA-only, gated) ──
#if defined(IDA_PORT_004F9D20)
float *__cdecl Vec3_Cross(float *a1, float *a2, float *a3)
{
  float *result; // eax

  result = a1;
  *a3 = a2[2] * a1[1] - a1[2] * a2[1];
  a3[1] = a1[2] * *a2 - *a1 * a2[2];
  a3[2] = *a1 * a2[1] - *a2 * a1[1];
  return result;
}
#endif
