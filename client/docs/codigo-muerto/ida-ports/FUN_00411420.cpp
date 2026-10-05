// 0x00411420 FUN_00411420 — nunca activado: IDA_PORT_00411420 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411420 (IDA-only, gated) ──
#if defined(IDA_PORT_00411420)
int __stdcall FUN_00411420(LPVOID *lpMem)
{
  int result; // eax
  LPVOID *v2; // edi
  LPVOID *i; // esi

  result = DAT_055c9b98;
  v2 = lpMem;
  for ( i = lpMem; i != (LPVOID *)DAT_055c9b98; v2 = i )
  {
    FUN_00411420(i[2]);
    i = (LPVOID *)*i;
    delete__(v2);
    result = DAT_055c9b98;
  }
  return result;
}
#endif
