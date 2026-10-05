// 0x0053D5A0 Resource_Load — nunca activado: IDA_PORT_0053D5A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Resource_Load (IDA-only, gated) ──
#if defined(IDA_PORT_0053D5A0)
char __cdecl Resource_Load(LPCSTR lpString)
{
  if ( lpParameter )
  {
    return Pipe_SetTarget((char *)lpParameter, lpString);
  }
  else
  {
    return 0;
  }
}
#endif
