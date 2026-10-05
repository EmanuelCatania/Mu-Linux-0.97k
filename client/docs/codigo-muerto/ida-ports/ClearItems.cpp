// 0x00502B80 ClearItems — nunca activado: IDA_PORT_00502B80 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ClearItems (IDA-only, gated) ──
#if defined(IDA_PORT_00502B80)
void __cdecl ClearItems()
{
  BYTE *v0; // eax

  v0 = &Items[0][72];
  do
  {
    *v0 = 0;
    v0 += 516;
  }
  while ( (int)v0 < (int)&DAT_07e907e0 );
}
#endif
