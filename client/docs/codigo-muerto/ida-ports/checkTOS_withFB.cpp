// 0x00549AE8 checkTOS_withFB — nunca activado: IDA_PORT_00549AE8 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── checkTOS_withFB (IDA-only, gated) ──
#if defined(IDA_PORT_00549AE8)
int __cdecl _checkTOS_withFB(int a1, int a2)
{
  int result; // eax

  result = a2 & 0x7FF00000;
  if ( (a2 & 0x7FF00000) == 2146435072 )
  {
    return a2;
  }
  return result;
}
#endif
