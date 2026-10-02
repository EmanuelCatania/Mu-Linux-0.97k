// 0x005439E8 lock_file — nunca activado: IDA_PORT_005439E8 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── lock_file (IDA-only, gated) ──
#if defined(IDA_PORT_005439E8)
void __cdecl _lock_file(FILE *Stream)
{
  if ( Stream < (FILE *)&DAT_00563dc0 || Stream > &DAT_00564020 )
  {
    EnterCriticalSection((LPCRITICAL_SECTION)&Stream[1]);
  }
  else
  {
    _lock((((char *)Stream - (char *)&DAT_00563dc0) >> 5) + 28);
  }
}
#endif
