// 0x005420DD crt_isspace — nunca activado: IDA_PORT_005420DD nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_isspace (IDA-only, gated) ──
#if defined(IDA_PORT_005420DD)
int __cdecl crt_isspace(int C)
{
  if ( (int)SrcSizeInBytes <= 1 )
  {
    return *((BYTE *)DAT_00564228 + 2 * C) & 8;
  }
  else
  {
    return _isctype(C, 8);
  }
}
#endif
