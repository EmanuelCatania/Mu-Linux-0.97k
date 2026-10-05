// 0x0051D740 FUN_0051d740 — nunca activado: IDA_PORT_0051D740 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0051d740 (IDA-only, gated) ──
#if defined(IDA_PORT_0051D740)
void __cdecl FUN_0051d740(char *strMsg)
{
  int v1[5]; // [esp+8h] [ebp-14h] BYREF

  g_iNumLineMessageBoxCustom = SeparateTextIntoLines(strMsg, g_lpszMessageBoxCustom[0], 7, 38);
  memset(&DAT_083a42f8, 0, 0x28u);
  v1[0] = 1;
  v1[1] = 71;
  v1[2] = 140;
  v1[3] = 70;
  v1[4] = 21;
  qmemcpy(&DAT_083a42f8, v1, 20u);
  if ( ErrorMessage )
  {
    NextErrorMessage = 139;
  }
  else
  {
    ErrorMessage = 139;
  }
}
#endif
