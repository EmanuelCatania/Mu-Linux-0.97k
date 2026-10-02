// 0x0040E780 FUN_0040e780 — nunca activado: IDA_PORT_0040E780 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040e780 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E780)
char __cdecl FUN_0040e780(BYTE *_this, char *a2)
{
  char result; // al
  int v3; // ebx
  BYTE *v4; // edx

  *a2 = 0;
  result = _this[200];
  if ( result )
  {
    v3 = 0;
    v4 = _this + 456;
    do
    {
      result = 0;
      strcat(a2, v4 - 256);
      if ( !*v4 )
      {
        break;
      }
      result = 0;
      v4 += 256;
      strcat(a2, DAT_005590f0);
      ++v3;
    }
    while ( v3 < 5 );
  }
  return result;
}
#endif
