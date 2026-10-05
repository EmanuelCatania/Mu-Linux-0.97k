// 0x00405590 CErrorReport__WriteLogBegin — nunca activado: IDA_PORT_00405590 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CErrorReport__WriteLogBegin (IDA-only, gated) ──
#if defined(IDA_PORT_00405590)
void __cdecl CErrorReport::WriteLogBegin(DWORD This)
{
  CErrorReport::Write(This, aLogBegin);
}
#endif
