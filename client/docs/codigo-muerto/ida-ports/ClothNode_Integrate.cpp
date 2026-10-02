// 0x00407AF0 ClothNode_Integrate — nunca activado: IDA_PORT_00407AF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ClothNode_Integrate (IDA-only, gated) ──
#if defined(IDA_PORT_00407AF0)
void __cdecl ClothNode_Integrate(float *_this, float a2)
{
  float *v2; // eax
  int v3; // ecx
  double v4; // st7
  double v5; // st7

  if ( ((BYTE)_this[10] & 1) == 0 )
  {
    v2 = _this + 4;
    v3 = 3;
    do
    {
      v4 = DAT_00559070 * *(v2 - 3);
      ++v2;
      --v3;
      v5 = v4 * a2 + *(v2 - 1);
      *(v2 - 1) = v5;
      v2[2] = v5 * a2 + v2[2];
    }
    while ( v3 );
  }
}
#endif
