// 0x0054218A crt_fgetc — nunca activado: IDA_PORT_0054218A nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_fgetc (IDA-only, gated) ──
#if defined(IDA_PORT_0054218A)
int __cdecl crt_fgetc(FILE *Stream)
{
  int v2; // edi

  _lock_file(Stream);
  if ( --Stream->_cnt < 0 )
  {
    v2 = _filbuf(Stream);
  }
  else
  {
    v2 = *(unsigned char *)Stream->_ptr++;
  }
  _unlock_file(Stream);
  return v2;
}
#endif
