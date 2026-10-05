// 0x004011D0 FUN_004011d0 — nunca activado: IDA_PORT_004011D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004011d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004011D0)
short __cdecl CSQuest::FindQuestContext(DWORD This, DWORD pQuest, int index)
{
  int v3; // eax
  int v5; // edx
  BYTE *v6; // ecx

  v3 = 0;
  v5 = *(short *)pQuest;
  if ( v5 <= 0 )
  {
LABEL_5:
    --*(BYTE *)(This + 116858);
    CSQuest::CheckQuestState(This, 0xFFu);
    return *(WORD *)(This + 116864);
  }
  else
  {
    v6 = (BYTE *)(*(unsigned char *)(This + 4) + pQuest + 44);
    while ( !*v6 )
    {
      ++v3;
      v6 += 18;
      if ( v3 >= v5 )
      {
        goto LABEL_5;
      }
    }
    return *(WORD *)(pQuest + 2 * (index + 8 * v3 + v3) + 48);
  }
}
#endif
