// 0x00479CF0 FilterName_LoadData — nunca activado: IDA_PORT_00479CF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FilterName_LoadData (IDA-only, gated) ──
#if defined(IDA_PORT_00479CF0)
FILE *__cdecl FilterName_LoadData(char *FileName)
{
  FILE *result; // eax
  int v2; // edx

  result = fopen(FileName, "rb");
  SMDFile_0 = result;
  if ( result )
  {
    while ( GetToken() != 2 )
    {
      v2 = DAT_07d78074;
      strcpy((char *)(20 * DAT_07d78074 + 131233296), TokenString);
      DAT_07d78074 = v2 + 1;
    }
    return (FILE *)fclose(SMDFile_0);
  }
  return result;
}
#endif
