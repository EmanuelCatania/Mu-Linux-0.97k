// 0x00543A3A unlock_file — nunca activado: IDA_PORT_00543A3A nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── unlock_file (IDA-only, gated) ──
#if defined(IDA_PORT_00543A3A)
void __cdecl _unlock_file(FILE *Stream)
{
  if ( Stream < (FILE *)&DAT_00563dc0 || Stream > &DAT_00564020 )
  {
    LeaveCriticalSection((LPCRITICAL_SECTION)&Stream[1]);
  }
  else
  {
    _unlock((((char *)Stream - (char *)&DAT_00563dc0) >> 5) + 28);
  }
}
#endif
