// 0x00542EB4 crt_ftell — nunca activado: IDA_PORT_00542EB4 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_ftell (IDA-only, gated) ──
#if defined(IDA_PORT_00542EB4)
int __cdecl crt_ftell(FILE *Stream)
{
  int v1; // edi

  _lock_file(Stream);
  v1 = _ftell_lk(Stream);
  _unlock_file(Stream);
  return v1;
}
#endif
