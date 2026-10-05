// 0x00552568 FUN_00552568 — nunca activado: IDA_PORT_00552568 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00552568 (IDA-only, gated) ──
#if defined(IDA_PORT_00552568)
// attributes: thunk
void __stdcall FUN_00552568(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
