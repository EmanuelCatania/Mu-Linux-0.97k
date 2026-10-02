// 0x00552760 FUN_00552760 — nunca activado: IDA_PORT_00552760 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00552760 (IDA-only, gated) ──
#if defined(IDA_PORT_00552760)
// attributes: thunk
void __stdcall FUN_00552760(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
