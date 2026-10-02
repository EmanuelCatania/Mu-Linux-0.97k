// 0x005436A6 crt_fflush — nunca activado: IDA_PORT_005436A6 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_fflush (IDA-only, gated) ──
#if defined(IDA_PORT_005436A6)
int __cdecl crt_fflush(FILE *Stream)
{
  int v2; // edi

  if ( !Stream )
  {
    return flsall(0);
  }
  _lock_file(Stream);
  v2 = _fflush_lk(Stream);
  _unlock_file(Stream);
  return v2;
}
#endif
