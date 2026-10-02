// 0x0053ED30 Pipe_SetTarget — nunca activado: IDA_PORT_0053ED30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Pipe_SetTarget (IDA-only, gated) ──
#if defined(IDA_PORT_0053ED30)
char __cdecl Pipe_SetTarget(char *_this, LPCSTR lpString)
{
  if ( !*_this )
  {
    return 0;
  }
  strcpy(_this + 752, lpString);
  return Pipe_Write(2225, 1555, lpString);
}
#endif
