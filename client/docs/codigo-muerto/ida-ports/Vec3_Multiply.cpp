// 0x004F9CB0 Vec3_Multiply — nunca activado: IDA_PORT_004F9CB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Vec3_Multiply (IDA-only, gated) ──
#if defined(IDA_PORT_004F9CB0)
float *__cdecl Vec3_Multiply(float *a1, float *a2, float *a3)
{
  float *result; // eax

  result = a1;
  *a3 = *a1 * *a2;
  a3[1] = a1[1] * a2[1];
  a3[2] = a1[2] * a2[2];
  return result;
}
#endif
