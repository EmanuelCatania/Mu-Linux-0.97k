// 0x00482D70 FUN_00482d70 — nunca activado: IDA_PORT_00482D70 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00482d70 (IDA-only, gated) ──
#if defined(IDA_PORT_00482D70)
int __cdecl FUN_00482d70(int iType, int iLevel)
{
  int v2; // ebx
  int *v3; // esi
  int v4; // edx
  int result; // eax
  int *v6; // ecx

  v2 = 7;
  v3 = (int *)&DAT_07ea9504;
LABEL_2:
  v4 = 7;
  result = v2 + 56;
  v6 = v3;
  while ( *((short *)v6 - 28) != iType || *v6 <= 0 || iLevel != -1 && ((*(v6 - 13) >> 3) & 0xF) != iLevel )
  {
    --v4;
    v6 -= 136;
    result -= 8;
    if ( v4 < 0 )
    {
      v3 -= 17;
      --v2;
      if ( (int)v3 >= (int)&DAT_07ea9328 )
      {
        goto LABEL_2;
      }
      return -1;
    }
  }
  return result;
}
#endif
