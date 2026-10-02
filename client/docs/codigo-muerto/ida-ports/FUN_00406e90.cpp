// 0x00406E90 FUN_00406e90 — nunca activado: IDA_PORT_00406E90 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406e90 (IDA-only, gated) ──
#if defined(IDA_PORT_00406E90)
int __cdecl FUN_00406e90(DWORD *_this, char a2)
{
  unsigned int v2; // eax
  char *v3; // edx
  int v4; // esi

  v2 = 0;
  v3 = &a2;
  v4 = 4;
  do
  {
    v2 = (unsigned char)*v3++ + 131 * v2;
    --v4;
  }
  while ( v4 );
  return v2 % _this[3];
}
#endif
