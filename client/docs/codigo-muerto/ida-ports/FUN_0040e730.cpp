// 0x0040E730 FUN_0040e730 — nunca activado: IDA_PORT_0040E730 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040e730 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E730)
int __cdecl FUN_0040e730(const char *_this, char *Text)
{
  const char *v2; // esi
  int i; // edi

  v2 = _this + 200;
  if ( !_this[200] )
  {
    return 1;
  }
  for ( i = 0; i < 5; ++i )
  {
    if ( !*v2 )
    {
      break;
    }
    if ( FindText(Text, v2, 0) )
    {
      return 1;
    }
    v2 += 256;
  }
  return 0;
}
#endif
