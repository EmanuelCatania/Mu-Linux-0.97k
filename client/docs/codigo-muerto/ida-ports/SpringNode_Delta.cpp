// 0x00407B50 SpringNode_Delta — nunca activado: IDA_PORT_00407B50 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SpringNode_Delta (IDA-only, gated) ──
#if defined(IDA_PORT_00407B50)
void __cdecl SpringNode_Delta(char *_this, int a2, int a3)
{
  float *v3; // eax
  char *v4; // ecx
  int v5; // edx
  int v6; // esi
  double v7; // st7

  v3 = (float *)(a2 + 28);
  v4 = &_this[-a2];
  v5 = a3;
  v6 = 3;
  do
  {
    v7 = *(float *)((char *)v3 + (DWORD)v4) - *v3;
    ++v3;
    v5 += 4;
    --v6;
    *(float *)(v5 - 4) = v7;
  }
  while ( v6 );
  Vec3_Length(a3);
}
#endif
