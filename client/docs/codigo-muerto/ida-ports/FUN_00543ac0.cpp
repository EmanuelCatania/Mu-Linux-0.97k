// 0x00543AC0 FUN_00543ac0 — nunca activado: IDA_PORT_00543AC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00543ac0 (IDA-only, gated) ──
#if defined(IDA_PORT_00543AC0)
char *__cdecl FUN_00543ac0(char *VarName)
{
  char *v1; // esi

  _lock(12);
  v1 = getenv(VarName);
  _unlock(12);
  return v1;
}
#endif
