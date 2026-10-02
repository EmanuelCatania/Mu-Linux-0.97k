// 0x00404E60 waveIO__CloseWaveFile — nunca activado: IDA_PORT_00404E60 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── waveIO__CloseWaveFile (IDA-only, gated) ──
#if defined(IDA_PORT_00404E60)
bool __cdecl waveIO::CloseWaveFile(DWORD This)
{
  HMMIO v1; // eax

  *(DWORD *)This = DAT_005524c0;
  v1 = *(HMMIO *)(This + 4);
  if ( v1 )
  {
    mmioClose(v1, 0);
  }
  return 1;
}
#endif
