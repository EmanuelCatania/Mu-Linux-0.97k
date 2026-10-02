// 0x00412780 FUN_00412780 — nunca activado: IDA_PORT_00412780 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00412780 (IDA-only, gated) ──
#if defined(IDA_PORT_00412780)
// Microsoft VisualC 2-14/net runtime
int *FUN_00412780()
{
  int v0; // eax
  int v1; // eax
  int v2; // edx
  int v3; // eax
  void *v4; // edi
  unsigned int v5; // edx
  int v6; // eax

  MAIN_HASH_CLASS = (int)&DAT_005524c8;
  FUN_00406d20(&MAIN_HASH_CLASS);
  if ( *(&MAIN_HASH_CLASS + 6) )
  {
    delete__((LPVOID)*(&MAIN_HASH_CLASS + 6));
  }
  v0 = rand();
  *(&MAIN_HASH_CLASS + 6) = operator_new(v0 % 3271 + 345);
  *(&MAIN_HASH_CLASS + 3) = 1024;
  v1 = operator_new(0x1000u);
  v2 = *(&MAIN_HASH_CLASS + 3);
  *(&MAIN_HASH_CLASS + 1) = v1;
  v3 = operator_new(4 * v2);
  v4 = (void *)*(&MAIN_HASH_CLASS + 1);
  v5 = 4 * *(&MAIN_HASH_CLASS + 3);
  *(&MAIN_HASH_CLASS + 2) = v3;
  memset(v4, 0, v5);
  memset((void *)*(&MAIN_HASH_CLASS + 2), 0, 4 * *(&MAIN_HASH_CLASS + 3));
  v6 = *(&MAIN_HASH_CLASS + 1);
  *(&MAIN_HASH_CLASS + 9) = *(&MAIN_HASH_CLASS + 2);
  *(&MAIN_HASH_CLASS + 8) = v6;
  return &MAIN_HASH_CLASS;
}
#endif
