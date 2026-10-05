// 0x00543264 putc — nunca activado: IDA_PORT_00543264 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── putc (IDA-only, gated) ──
#if defined(IDA_PORT_00543264)
int __cdecl putc(int Character, FILE *Stream)
{
  return fputc(Character, Stream);
}
#endif
