// 0x004797B0 SkillAttribute_LoadNames — nunca activado: IDA_PORT_004797B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SkillAttribute_LoadNames (IDA-only, gated) ──
#if defined(IDA_PORT_004797B0)
FILE *__cdecl SkillAttribute_LoadNames(char *FileName)
{
  FILE *result; // eax
  int v2; // esi

  result = fopen(FileName, "rb");
  SMDFile_0 = result;
  if ( result )
  {
    while ( 1 )
    {
      result = (FILE *)GetToken();
      if ( result == (FILE *)2 )
      {
        break;
      }
      if ( result == (FILE *)1 )
      {
        v2 = (__int64)TokenNumber;
        GetToken();
        strcpy(GlobalText[v2], TokenString);
      }
    }
  }
  return result;
}
#endif
