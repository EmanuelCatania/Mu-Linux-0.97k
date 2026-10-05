// 0x0051D780 FUN_0051d780 — nunca activado: IDA_PORT_0051D780 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0051d780 (IDA-only, gated) ──
#if defined(IDA_PORT_0051D780)
void __cdecl FUN_0051d780(int a1, char a2)
{
  int v2[5]; // [esp+8h] [ebp-14h] BYREF

  DAT_083a7c04 = a1;
  DAT_083a7c09[0] = a2;
  DAT_083a7c08 = 0;
  g_iNumLineMessageBoxCustom = SeparateTextIntoLines(GlobalText[a1], g_lpszMessageBoxCustom[0], 7, 38);
  memset(&DAT_083a42f8, 0, 0x28u);
  v2[0] = 1;
  v2[1] = 71;
  v2[2] = 140;
  v2[3] = 70;
  v2[4] = 21;
  qmemcpy(&DAT_083a42f8, v2, 0x14u);
  if ( ErrorMessage )
  {
    NextErrorMessage = 141;
  }
  else
  {
    ErrorMessage = 141;
  }
}
#endif
