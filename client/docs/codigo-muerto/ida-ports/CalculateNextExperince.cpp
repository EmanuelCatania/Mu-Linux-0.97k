// 0x0047E350 CalculateNextExperince — nunca activado: IDA_PORT_0047E350 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CalculateNextExperince (IDA-only, gated) ──
#if defined(IDA_PORT_0047E350)
int __cdecl CalculateNextExperince(int _this)
{
  unsigned short v1; // di
  int result; // eax
  int v3; // esi

  v1 = *(WORD *)(_this + 14);
  result = v1 * v1 * (v1 + 9);
  v3 = 10 * result;
  *(DWORD *)(_this + 52) = 10 * result;
  if ( v1 > 0xFFu )
  {
    result = 125 * (v1 - 255) * (v1 - 255) * (v1 - 255 + 9);
    *(DWORD *)(_this + 52) = v3 + 1000 * (v1 - 255) * (v1 - 255) * (v1 - 255 + 9);
  }
  return result;
}
#endif
