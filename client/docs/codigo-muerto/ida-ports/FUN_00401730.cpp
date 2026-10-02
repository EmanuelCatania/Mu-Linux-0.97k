// 0x00401730 FUN_00401730 — nunca activado: IDA_PORT_00401730 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00401730 (IDA-only, gated) ──
#if defined(IDA_PORT_00401730)
BYTE __cdecl CSQuest::CheckQuestState(DWORD This, BYTE state)
{
  DWORD lpQuest; // edi
  short QuestContext; // ax

  lpQuest = This + 584 * *(unsigned char *)(This + 116858) + 8;
  if ( state == 0xFF )
  {
    CSQuest::getQuestState(This, -1);
  }
  else
  {
    *(BYTE *)(This + 116866) = state;
  }
  if ( *(BYTE *)(This + 116866) == 1 )
  {
    (BYTE)(QuestContext) = CSQuest::CheckActCondition(This, lpQuest);
    if ( (BYTE)QuestContext )
    {
      QuestContext = CSQuest::FindQuestContext(This, lpQuest, 2);
      *(WORD *)(This + 116864) = QuestContext;
      *(BYTE *)(This + 116866) = 1;
    }
  }
  else if ( *(BYTE *)(This + 116866) == 2 )
  {
    QuestContext = CSQuest::FindQuestContext(This, lpQuest, 3);
    *(WORD *)(This + 116864) = QuestContext;
  }
  else
  {
    (BYTE)(QuestContext) = *(BYTE *)(This + 116866) - 3;
    if ( *(BYTE *)(This + 116866) == 3 )
    {
      (BYTE)(QuestContext) = CSQuest::CheckRequestCondition(This, lpQuest, 0);
      if ( (BYTE)QuestContext )
      {
        QuestContext = CSQuest::FindQuestContext(This, lpQuest, 0);
        *(WORD *)(This + 116864) = QuestContext;
      }
    }
  }
  return QuestContext;
}
#endif
