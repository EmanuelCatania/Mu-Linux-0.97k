// 0x00482FF0 FUN_00482ff0 — nunca activado: IDA_PORT_00482FF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00482ff0 (IDA-only, gated) ──
#if defined(IDA_PORT_00482FF0)
int __cdecl FUN_00482ff0(int iType, int iLevel)
{
  int result; // eax
  int *v3; // edi
  int *v4; // ecx
  int v5; // edx

  result = 0;
  v3 = (int *)&DAT_07ea9504;
  do
  {
    v4 = v3;
    v5 = 8;
    do
    {
      if ( *((short *)v4 - 28) == iType
        && (iType == -1 || *v4 > 0)
        && (iLevel == -1 || ((*(v4 - 13) >> 3) & 0xF) == iLevel) )
      {
        ++result;
      }
      v4 -= 136;
      --v5;
    }
    while ( v5 );
    v3 -= 17;
  }
  while ( (int)v3 >= (int)&DAT_07ea9328 );
  return result;
}
#endif
