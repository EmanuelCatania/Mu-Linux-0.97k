// 0x00479E50 OpenNameFilterFile — nunca activado: IDA_PORT_00479E50 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── OpenNameFilterFile (IDA-only, gated) ──
#if defined(IDA_PORT_00479E50)
void __cdecl OpenNameFilterFile(char *FileName)
{
  FILE *v1; // esi
  char *v2; // ebp
  unsigned int v3; // edi
  unsigned int i; // edx
  int v5; // esi
  char *v6; // ebx
  int v7; // [esp+8h] [ebp-10Ch]
  void *lpMem; // [esp+Ch] [ebp-108h]
  void *Buffer; // [esp+10h] [ebp-104h] BYREF
  CHAR Text[256]; // [esp+14h] [ebp-100h] BYREF

  v1 = fopen(FileName, "rb");
  if ( v1 )
  {
    v2 = (char *)operator_new(0x4E20u);
    lpMem = v2;
    fread(v2, 0x4E20u, 1u, v1);
    fread(&Buffer, 4u, 1u, v1);
    fclose(v1);
    v3 = (unsigned int)&DAT_00578200;
    for ( i = 0; i <= 0x4E1C; i += 4 )
    {
      v5 = *(DWORD *)&v2[i];
      if ( (((unsigned char)(i >> 2) - 1) & 1) != 0 )
      {
        if ( (((unsigned char)(i >> 2) - 1) & 1) == 1 )
        {
          v3 += v5;
        }
      }
      else
      {
        v3 ^= v5;
      }
      if ( (i & 0xF) == 0 )
      {
        v3 ^= (v3 + 11201) >> (((i >> 2) & 7) + 1);
      }
    }
    if ( Buffer == (void *)v3 )
    {
      v7 = 0;
      v6 = DAT_07d27610;
      while ( 1 )
      {
        BuxConvert_0((BYTE *)v2, 20);
        qmemcpy(v6, v2, 0x14u);
        if ( !*v6 )
        {
          break;
        }
        v2 += 20;
        v6 += 20;
        ++v7;
        if ( (int)v6 >= (int)&GlobalText[33][96] )
        {
          goto LABEL_18;
        }
      }
      DAT_07d78074 = v7;
LABEL_18:
      delete__(lpMem);
    }
    else
    {
      sprintf(Text, "%s - File corrupted.", FileName);
      CErrorReport::Write((DWORD)&g_ErrorReport, Text);
      MessageBoxA(g_hWnd, Text, 0, 0);
      SendMessageA(g_hWnd, 2u, 0, 0);
      delete__(v2);
    }
  }
  else
  {
    sprintf(Text, "%s - File not exist.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    MessageBoxA(g_hWnd, Text, 0, 0);
    SendMessageA(g_hWnd, 2u, 0, 0);
  }
}
#endif
