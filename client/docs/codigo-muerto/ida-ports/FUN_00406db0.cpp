// 0x00406DB0 FUN_00406db0 — nunca activado: IDA_PORT_00406DB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406db0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406DB0)
int __cdecl FUN_00406db0(DWORD *_this, int a2, int a3)
{
  int result; // eax

  _this[4] = a2;
  _this[5] = a3;
  result = rand() % 240 / 2;
  _this[7] = 2 * result + 111;
  return result;
}
#endif
