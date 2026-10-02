// 0x00482DD0 CSQuest_FindQuestItemsInInven — nunca activado: IDA_PORT_00482DD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CSQuest_FindQuestItemsInInven (IDA-only, gated) ──
#if defined(IDA_PORT_00482DD0)
int __cdecl CSQuest::FindQuestItemsInInven(DWORD This, int nType, int nCount, int nLevel)
{
  int v4; // ebx
  int *v5; // edi
  int v6; // edx
  int *v7; // ecx

  v4 = 0;
  v5 = (int *)&DAT_07ea9504;
  while ( 2 )
  {
    v6 = 7;
    v7 = v5;
    do
    {
      if ( *((short *)v7 - 28) == nType
        && *v7 > 0
        && (nLevel == -1 || ((*(v7 - 13) >> 3) & 0xF) == nLevel)
        && ++v4 >= nCount )
      {
        return 0;
      }
      --v6;
      v7 -= 136;
    }
    while ( v6 >= 0 );
    v5 -= 17;
    if ( (int)v5 >= (int)&DAT_07ea9328 )
    {
      continue;
    }
    break;
  }
  return nCount - v4;
}
#endif
