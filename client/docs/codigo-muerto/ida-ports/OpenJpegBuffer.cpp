// 0x00529360 OpenJpegBuffer — nunca activado: IDA_PORT_00529360 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── OpenJpegBuffer (IDA-only, gated) ──
#if defined(IDA_PORT_00529360)
bool __cdecl OpenJpegBuffer(char *filename, float *BufferFloat)
{
  char *v2; // ebx
  const char *v3; // edi
  signed int v4; // esi
  char *v5; // edx
  int v6; // ecx
  bool v7; // zf
  FILE *v8; // eax
  unsigned int v10; // esi
  const void **v11; // edi
  char *v12; // ebx
  int v13; // eax
  int v14; // ecx
  int v15; // edi
  int v16; // esi
  float *v17; // eax
  int v18; // edx
  int v19; // [esp-4h] [ebp-5B0h]
  char v20[256]; // [esp+Ch] [ebp-5A0h] BYREF
  CHAR Text[256]; // [esp+10Ch] [ebp-4A0h] BYREF
  void (__cdecl  *v22[33])(int *); // [esp+20Ch] [ebp-3A0h] BYREF
  char v23[64]; // [esp+290h] [ebp-31Ch] BYREF
  char FileName[256]; // [esp+2D0h] [ebp-2DCh] BYREF
  int a1[28]; // [esp+3D0h] [ebp-1DCh] BYREF
  int v26; // [esp+440h] [ebp-16Ch]
  unsigned int v27; // [esp+444h] [ebp-168h]
  int v28; // [esp+44Ch] [ebp-160h]
  unsigned int v29; // [esp+45Ch] [ebp-150h]
  FILE *Stream; // [esp+5A0h] [ebp-Ch]
  int i; // [esp+5A4h] [ebp-8h]
  const void **v32; // [esp+5A8h] [ebp-4h]

  if ( DAT_0055a7c4 )
  {
    v2 = filename;
    v4 = 0;
    if ( (int)strlen(filename) > 0 )
    {
      v5 = filename;
      v6 = v20 - filename;
      for ( i = v20 - filename; ; v6 = i )
      {
        v7 = *v5 == 46;
        v5[v6] = *v5;
        if ( v7 )
        {
          break;
        }
        ++v4;
        ++v5;
        if ( v4 >= (int)strlen(filename) )
        {
          break;
        }
      }
    }
    v20[v4 + 1] = 0;
    strcpy(FileName, "Data\\");
    strcat(FileName, v20);
    v3 = "OZJ";
  }
  else
  {
    v2 = filename;
    strcpy(FileName, "Data2\\");
    v3 = filename;
  }
  strcat(FileName, v3);
  v8 = fopen(FileName, "rb");
  Stream = v8;
  if ( v8 )
  {
    if ( DAT_0055a7c4 )
    {
      fseek(v8, 24, 0);
    }
    else
    {
      SaveImage(24, "OZJ", v2, 0, 0);
    }
    a1[0] = (int)jpeg_std_error(v22);
    v22[0] = my_error_exit;
    if ( _setjmp3(v23, 0, v19) )
    {
      jpeg_destroy_decompress((int)a1);
      fclose(Stream);
      return 0;
    }
    else
    {
      jpeg_create_decompress(a1, 62, 464);
      jpeg_stdio_src((int)a1, (int)Stream);
      jpeg_read_header(a1, 1);
      jpeg_start_decompress((int)a1);
      v10 = v26 * v28;
      i = v26 * v28;
      v11 = (const void **)(*(int (__cdecl **)(int *, int, int, int))(a1[1] + 8))(a1, 1, v26 * v28, 1);
      v32 = v11;
      v12 = (char *)operator_new(v26 * v28 * v27);
      v13 = v27;
      if ( v29 < v27 )
      {
        while ( 1 )
        {
          jpeg_read_scanlines(a1, v11, 1);
          qmemcpy(&v12[i * (v27 - v29)], *v11, v10);
          v13 = v27;
          if ( v29 >= v27 )
          {
            break;
          }
          v11 = v32;
          v10 = i;
        }
      }
      v14 = 0;
      if ( v13 )
      {
        v15 = v26;
        i = v13;
        do
        {
          if ( v15 )
          {
            v16 = v15;
            v17 = &BufferFloat[v14 + 2];
            do
            {
              v17 += 3;
              v18 = (unsigned char)v12[v14];
              v14 += 3;
              v32 = (const void **)v18;
              *(v17 - 5) = (double)v18 * 0.0039215689;
              v32 = (const void **)(unsigned char)v12[v14 - 2];
              --v16;
              *(v17 - 4) = (double)(int)v32 * 0.0039215689;
              v32 = (const void **)(unsigned char)v12[v14 - 1];
              *(v17 - 3) = (double)(int)v32 * 0.0039215689;
            }
            while ( v16 );
          }
          --i;
        }
        while ( i );
      }
      delete__(v12);
      jpeg_finish_decompress(a1);
      jpeg_destroy_decompress((int)a1);
      fclose(Stream);
      return 1;
    }
  }
  else
  {
    sprintf(Text, "%s - File not exist.", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    CErrorReport::Write((DWORD)&g_ErrorReport, "\r\n");
    MessageBoxA(g_hWnd, Text, 0, 0);
    SendMessageA(g_hWnd, 2u, 0, 0);
    return 0;
  }
}
#endif
