// 0x004016E0 FUN_004016e0 — nunca activado: IDA_PORT_004016E0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004016e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004016E0)
BYTE __cdecl CSQuest::getQuestState(DWORD This, int questIndex)
{
  int Index; // eax
  int SubIndex; // ecx
  char v5; // al
  BYTE byCurrState; // al

  if ( questIndex == -1 )
  {
    Index = 0;
  }
  else
  {
    Index = *(unsigned char *)(This + 116858) >> 2;
  }
  SubIndex = *(unsigned char *)(This + 116858) - Index;
  if ( SubIndex )
  {
    v5 = *(BYTE *)(Index + This + 116808) >> (2 * SubIndex);
  }
  else
  {
    v5 = *(BYTE *)(Index + This + 116808);
  }
  byCurrState = v5 & 3;
  if ( questIndex == -1 )
  {
    *(BYTE *)(This + 116866) = byCurrState;
  }
  return byCurrState;
}
#endif
