// 0x00406660 FUN_00406660 — nunca activado: IDA_PORT_00406660 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406660 (IDA-only, gated) ──
#if defined(IDA_PORT_00406660)
void FUN_00406660()
{
  HANDLE CurrentThread; // eax
  DWORD *Ebp; // esi
  int i; // edi
  CONTEXT Context; // [esp+Ch] [ebp-2CCh] BYREF

  memset(&Context, 0, sizeof(Context));
  Context.ContextFlags = 65543;
  CurrentThread = GetCurrentThread();
  GetThreadContext(CurrentThread, &Context);
  CErrorReport::Write((DWORD)&g_ErrorReport, aRegister);
  CErrorReport::Write(
    (DWORD)&g_ErrorReport,
    "EAX : 0x%08X\tEBX : 0x%08X\tECX : 0x%08X\tEDX : 0x%08X\r\n",
    Context.Eax,
    Context.Ebx,
    Context.Ecx,
    Context.Edx);
  CErrorReport::Write(
    (DWORD)&g_ErrorReport,
    "ESI : 0x%08X\tEDI : 0x%08X\tEBP : 0x%08X\tEIP : 0x%08X\r\n",
    Context.Esi,
    Context.Edi,
    Context.Ebp,
    Context.Eip);
  CErrorReport::Write((DWORD)&g_ErrorReport, aCallStack);
  Ebp = (DWORD *)Context.Ebp;
  for ( i = 0; i < 1024; ++i )
  {
    if ( IsBadReadPtr(Ebp, 4u) )
    {
      break;
    }
    if ( !*Ebp )
    {
      break;
    }
    CErrorReport::Write((DWORD)&g_ErrorReport, "0x%08X\r\n", Ebp[1]);
    Ebp = (DWORD *)*Ebp;
  }
}
#endif
