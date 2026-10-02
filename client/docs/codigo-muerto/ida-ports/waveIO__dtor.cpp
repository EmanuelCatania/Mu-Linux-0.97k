// 0x00404E40 waveIO__dtor — nunca activado: IDA_PORT_00404E40 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── waveIO__dtor (IDA-only, gated) ──
#if defined(IDA_PORT_00404E40)
DWORD __cdecl waveIO::_waveIO(DWORD This, bool IO)
{
  waveIO::CloseWaveFile(This);
  if ( IO )
  {
    delete__((LPVOID)This);
  }
  return This;
}
#endif
