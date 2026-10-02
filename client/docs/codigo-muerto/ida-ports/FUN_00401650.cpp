// 0x00401650 FUN_00401650 — nunca activado: IDA_PORT_00401650 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00401650 (IDA-only, gated) ──
#if defined(IDA_PORT_00401650)
// positive sp value has been detected, the output may be wrong!
bool __cdecl CSQuest::CheckActCondition(DWORD This, DWORD pQuest)
{
  int v2; // ebx
  unsigned char *v4; // esi
  DWORD v5; // ecx
  short *v7; // [esp+8h] [ebp-8h]

  v2 = 0;
  if ( *v7 <= 0 )
  {
    return 1;
  }
  v4 = (unsigned char *)(v7 + 20);
  v5 = -40 - (DWORD)v7;
  while ( 1 )
  {
    if ( v4[v5 + 44 + *(unsigned char *)(This + 4) + (DWORD)v7] != 1 || *(v4 - 1) != 1 )
    {
      goto LABEL_7;
    }
    if ( CSQuest::FindQuestItemsInInven(v4[2], 32 * *v4 + v4[1], v4[2], -1) )
    {
      break;
    }
    v5 = pQuest;
LABEL_7:
    ++v2;
    v4 += 18;
    if ( v2 >= *v7 )
    {
      return 1;
    }
  }
  *(WORD *)(This + 116864) = CSQuest::FindQuestContext(This, (DWORD)v7, 1);
  return 0;
}
#endif
