// 0x004F6CE0 OpenTerrainAttribute — nunca activado: IDA_PORT_004F6CE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── OpenTerrainAttribute (IDA-only, gated) ──
#if defined(IDA_PORT_004F6CE0)
int __cdecl OpenTerrainAttribute(char *FileName)
{
  FILE *fp; // eax MAPDST
  int result; // eax
  int iSize; // esi
  bool Error; // al
  int i; // ecx
  CHAR Text[256]; // [esp+8h] [ebp-10104h] BYREF
  unsigned char *byBuffer; // [esp+108h] [ebp-10004h] BYREF

  fp = fopen(FileName, "rb");
  if ( fp )
  {
    fseek(fp, 0, 2);
    iSize = ftell(fp);
    fseek(fp, 0, 0);
    if ( iSize == 65539 )
    {
      fread(&byBuffer, 65539u, 1u, fp);
      BuxConvert_1((BYTE *)&byBuffer, 65539);
      qmemcpy(TerrainWall, (char *)&byBuffer + 3, sizeof(TerrainWall));
      Error = 0;
      if ( (BYTE)byBuffer || *(WORD *)((char *)&byBuffer + 1) != 0xFFFF )
      {
        Error = 1;
      }
      if ( !DAT_083a410c )
      {
        switch ( World )
        {
          case 0:
            if ( TerrainWall[31623] != 5 )
            {
              goto LABEL_19;
            }
            break;
          case 1:
            if ( TerrainWall[30947] != 4 )
            {
              goto LABEL_19;
            }
            break;
          case 2:
            if ( TerrainWall[14288] != 5 )
            {
              goto LABEL_19;
            }
            break;
          case 3:
            if ( TerrainWall[30650] != 5 )
            {
              goto LABEL_19;
            }
            break;
          case 4:
            if ( TerrainWall[19393] != 5 )
            {
LABEL_19:
              Error = 1;
            }
            break;
          default:
            break;
        }
      }
      for ( i = 0; i < 65536; ++i )
      {
        if ( TerrainWall[i] >= 0x80u )
        {
          Error = 1;
        }
      }
      if ( Error )
      {
        ExitProgram();
        (BYTE)(result) = 0;
      }
      else
      {
        result = fclose(fp);
        (BYTE)(result) = 1;
      }
    }
    else
    {
      ExitProgram();
      (BYTE)(result) = 0;
    }
  }
  else
  {
    sprintf(Text, "%s file not found.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    CErrorReport::Write((DWORD)&g_ErrorReport, "\r\n");
    MessageBoxA(g_hWnd, Text, 0, 0);
    result = SendMessageA(g_hWnd, 2u, 0, 0);
    (BYTE)(result) = 0;
  }
  return result;
}
#endif
