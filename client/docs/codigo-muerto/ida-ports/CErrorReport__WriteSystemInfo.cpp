// 0x00405620 CErrorReport__WriteSystemInfo — nunca activado: IDA_PORT_00405620 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CErrorReport__WriteSystemInfo (IDA-only, gated) ──
#if defined(IDA_PORT_00405620)
void __cdecl CErrorReport::WriteSystemInfo(DWORD This, DWORD si)
{
  CErrorReport::Write(This, aSystemInformat);
  CErrorReport::Write(This, "OS \t\t\t: %s\r\n", (const char *)(si + 128));
  CErrorReport::Write(This, "CPU \t\t\t: %s\r\n", (const char *)si);
  CErrorReport::Write(This, "RAM \t\t\t: %dMB\r\n", *(DWORD *)(si + 256) / 1024 / 1024 + 1);
  CErrorReport::AddSeparator(This);
  CErrorReport::Write(This, "Direct-X \t\t: %s\r\n", (const char *)(si + 260));
}
#endif
