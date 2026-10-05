// 0x00552588 FUN_00552588 — nunca activado: IDA_PORT_00552588 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00552588 (IDA-only, gated) ──
#if defined(IDA_PORT_00552588)
// attributes: thunk
void __stdcall FUN_00552588(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
