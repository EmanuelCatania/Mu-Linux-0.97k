// 0x005524B4 FUN_005524b4 — nunca activado: IDA_PORT_005524B4 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005524b4 (IDA-only, gated) ──
#if defined(IDA_PORT_005524B4)
// attributes: thunk
void __stdcall FUN_005524b4(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
