// 0x00406EF0 FUN_00406ef0 — nunca activado: IDA_PORT_00406EF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406ef0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406EF0)
int __cdecl FUN_00406ef0(DWORD *_this, char a2)
{
  unsigned int v2; // eax
  char *v3; // edx
  int v4; // esi

  v2 = 0;
  v3 = &a2;
  v4 = 4;
  do
  {
    v2 = v2 * _this[7] + (unsigned char)*v3++;
    --v4;
  }
  while ( v4 );
  return v2 % _this[3];
}
#endif
