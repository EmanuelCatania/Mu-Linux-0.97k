// 0x004CBDD0 FUN_004cbdd0 — nunca activado: IDA_PORT_004CBDD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004cbdd0 (IDA-only, gated) ──
#if defined(IDA_PORT_004CBDD0)
int *FUN_004cbdd0()
{
  int *result; // eax

  result = DAT_07e11fb0;
  do
  {
    *((WORD *)result - 28) = -1;
    *result = 0;
    result += 17;
  }
  while ( (int)result < (int)&Items[0][56] );
  return result;
}
#endif
