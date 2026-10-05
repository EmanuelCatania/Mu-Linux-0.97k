// 0x00479A50 Filter_SaveBMD — nunca activado: IDA_PORT_00479A50 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Filter_SaveBMD (IDA-only, gated) ──
#if defined(IDA_PORT_00479A50)
int __cdecl Filter_SaveBMD(char *FileName)
{
  char *v1; // ebx
  char *v2; // ebp
  int v3; // ebp
  unsigned int i; // edx
  int v5; // esi
  FILE *Stream; // [esp+10h] [ebp-8h]
  int Buffer; // [esp+14h] [ebp-4h] BYREF
  char *FileNamea; // [esp+1Ch] [ebp+4h]

  Stream = fopen(FileName, aWb);
  FileNamea = (char *)operator_new(0x4E20u);
  v1 = FileNamea;
  v2 = DAT_07d73104;
  do
  {
    qmemcpy(v1, v2, 0x14u);
    BuxConvert_0((BYTE *)v1, 20);
    v2 += 20;
    v1 += 20;
  }
  while ( (int)v2 < (int)&DAT_07d77f24 );
  v3 = (int)&DAT_007cfa00;
  for ( i = 0; i <= 0x4E1C; i += 4 )
  {
    v5 = *(DWORD *)&FileNamea[i];
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
      v3 ^= (unsigned int)(v3 + 15997) >> (((i >> 2) & 7) + 1);
    }
  }
  Buffer = v3;
  crt_fwrite(FileNamea, 0x4E20u, 1u, Stream);
  crt_fwrite(&Buffer, 4u, 1u, Stream);
  delete__(FileNamea);
  return fclose(Stream);
}
#endif
