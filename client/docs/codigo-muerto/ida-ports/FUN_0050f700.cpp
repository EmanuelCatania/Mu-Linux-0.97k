// 0x0050F700 FUN_0050f700 — nunca activado: IDA_PORT_0050F700 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0050f700 (IDA-only, gated) ──
#if defined(IDA_PORT_0050F700)
int __cdecl FUN_0050f700(char *FileName)
{
  FILE *v1; // edi
  const char *v2; // esi

  v1 = fopen(FileName, aWt);
  v2 = (const char *)&MacroText;
  do
  {
    fprintf(v1, "%s\n", v2);
    v2 += 256;
  }
  while ( (int)v2 < (int)&ItemKey );
  return fclose(v1);
}
#endif
