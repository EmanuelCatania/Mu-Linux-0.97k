// 0x00401010 FUN_00401010 — nunca activado: IDA_PORT_00401010 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00401010 (IDA-only, gated) ──
#if defined(IDA_PORT_00401010)
// Microsoft VisualC 2-14/net runtime
void *FUN_00401010()
{
  void *result; // eax

  result = &DAT_00567500;
  DAT_00567500 = &DAT_005524b8;
  if ( !g_csQuest )
  {
    g_csQuest = (DWORD)&DAT_00567500;
  }
  *((BYTE *)&DAT_00567500 + 4) = -1;
  *((BYTE *)&DAT_00567500 + 116858) = 0;
  *((BYTE *)&DAT_00567500 + 116859) = 0;
  *((BYTE *)&DAT_00567500 + 116861) = 0;
  *((BYTE *)&DAT_00567500 + 116862) = 0;
  *((BYTE *)&DAT_00567500 + 116863) = 0;
  *((WORD *)&DAT_00567500 + 58432) = 0;
  DAT_00567500 = DAT_005524b4;
  return result;
}
#endif
