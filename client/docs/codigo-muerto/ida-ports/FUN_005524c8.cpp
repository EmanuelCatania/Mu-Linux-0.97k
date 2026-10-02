// 0x005524C8 FUN_005524c8 — nunca activado: IDA_PORT_005524C8 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005524c8 (IDA-only, gated) ──
#if defined(IDA_PORT_005524C8)
// attributes: thunk
void __stdcall FUN_005524c8(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
