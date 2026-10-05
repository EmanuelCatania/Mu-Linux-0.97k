// 0x0040C190 TextureScriptParsing_parsingTScript — nunca activado: IDA_PORT_0040C190 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── TextureScriptParsing_parsingTScript (IDA-only, gated) ──
#if defined(IDA_PORT_0040C190)
bool __cdecl TextureScriptParsing::parsingTScript(DWORD This, char *filename)
{
  char *v3; // eax
  char *v4; // ebp
  char *v5; // edx
  signed int v6; // edi
  int v7; // esi
  char Control[8]; // [esp+10h] [ebp-28h] BYREF
  char Str[32]; // [esp+18h] [ebp-20h] BYREF

  qmemcpy(Str, filename, sizeof(Str));
  strcpy(Control, "RHSN");
  v3 = strchr(Str, 95);
  v4 = v3;
  if ( v3 )
  {
    v5 = strtok(v3, Delimiter);
    v6 = strlen(v5) <= 5 ? strlen(v5) : 5;
    if ( strcspn(v5, Control) )
    {
      v7 = 1;
      if ( v6 > 1 )
      {
        while ( 2 )
        {
          switch ( v4[v7] )
          {
            case 'H':
              *(BYTE *)(This + 1) = 1;
              goto LABEL_12;
            case 'N':
              *(BYTE *)(This + 3) = 1;
              goto LABEL_12;
            case 'R':
              *(BYTE *)This = 1;
              goto LABEL_12;
            case 'S':
              *(BYTE *)(This + 2) = 1;
LABEL_12:
              ++v7;
              *(BYTE *)(This + 4) = 1;
              if ( v7 >= v6 )
              {
                return *(BYTE *)(This + 4);
              }
              continue;
            default:
              *(BYTE *)(This + 4) = 0;
              return 0;
          }
        }
      }
    }
  }
  return *(BYTE *)(This + 4);
}
#endif
