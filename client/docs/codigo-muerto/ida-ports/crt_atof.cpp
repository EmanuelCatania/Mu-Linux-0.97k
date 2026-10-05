// 0x00542133 crt_atof — nunca activado: IDA_PORT_00542133 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_atof (IDA-only, gated) ──
#if defined(IDA_PORT_00542133)
double __cdecl crt_atof(const char *String)
{
  char v4[24]; // [esp+4h] [ebp-18h] BYREF

  while ( (int)SrcSizeInBytes <= 1 ? *((BYTE *)DAT_00564228 + 2 * *(unsigned char *)String) & 8 : _isctype(
                                                                                                     *(unsigned char *)String,
                                                                                                     8) )
  {
    ++String;
  }
  strlen(String);
  return *(double *)(_fltin2(v4, String) + 16);
}
#endif
