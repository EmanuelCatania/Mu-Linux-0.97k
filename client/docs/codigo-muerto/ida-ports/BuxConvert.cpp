// 0x004F6EB0 BuxConvert — nunca activado: IDA_PORT_004F6EB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── BuxConvert (IDA-only, gated) ──
#if defined(IDA_PORT_004F6EB0)
void __cdecl BuxConvert(BYTE *Buffer, int Size)
{
  int i; // ecx

  for ( i = 0; i < Size; ++i )
  {
    Buffer[i] ^= DAT_0055a770[i % 3];
  }
}
#endif
