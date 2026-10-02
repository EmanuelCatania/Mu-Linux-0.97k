// 0x00552668 FUN_00552668 — nunca activado: IDA_PORT_00552668 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00552668 (IDA-only, gated) ──
#if defined(IDA_PORT_00552668)
// attributes: thunk
void __stdcall FUN_00552668(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
  __imp_RtlUnwind(TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#endif
