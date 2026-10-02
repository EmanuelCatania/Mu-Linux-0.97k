// 0x004106D0 FUN_004106d0 — nunca activado: IDA_PORT_004106D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004106d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004106D0)
int __cdecl FUN_004106d0(DWORD *_this, float a2, int a3, int a4, float a5)
{
  const char *v5; // ebx
  int v6; // edx
  int i; // ecx
  float **v8; // edi
  float *v9; // eax
  float *v10; // ebp
  float **v12; // edi
  float *v13; // edi
  __int64 Height; // rax
  int v15; // eax
  int v16; // esi
  int v17; // eax
  int v18; // edx
  float v19; // [esp-4h] [ebp-20h]
  int v20; // [esp+10h] [ebp-Ch] BYREF
  DWORD *v21; // [esp+14h] [ebp-8h]
  char v22[4]; // [esp+18h] [ebp-4h] BYREF

  v5 = (const char *)LODWORD(a2);
  v6 = 0;
  v21 = _this;
  v20 = 0;
  for ( i = 0; i < 4; ++i )
  {
    if ( !*(BYTE *)(i + LODWORD(a2)) )
    {
      break;
    }
    v6 += *(unsigned char *)(i + LODWORD(a2));
  }
  v20 = v6;
  v8 = (float **)FUN_004113e0(&a2, &v20);
  v9 = *(float **)FUN_004113a0(v22, &v20);
  v10 = *v8;
  if ( v9 == *v8 )
  {
    return 0;
  }
  v12 = (float **)(v21 + 5);
  v21[5] = v9;
  do
  {
    if ( *((DWORD *)v9 + 72) == m_dwTextColor
      && *((DWORD *)v9 + 73) == m_dwBackColor
      && *((DWORD *)v9 + 70) == LODWORD(a5)
      && !strcmp((const char *)v9 + 16, v5) )
    {
      break;
    }
    FUN_004112b0(v12);
    v9 = *v12;
  }
  while ( *v12 != v10 );
  v13 = *v12;
  if ( v13 == v10 )
  {
    return 0;
  }
  Height = (__int64)Bitmaps[0].Height;
  if ( *((DWORD *)v13 + 69) > (int)Height )
  {
    *((DWORD *)v13 + 69) = Height;
  }
  v15 = *((DWORD *)v13 + 76);
  v16 = 0;
  if ( (v15 != 0) + 1 > 0 )
  {
    a2 = (float)a4;
    a5 = (float)a3;
    do
    {
      if ( v15 )
      {
        if ( v16 )
        {
          v17 = *((DWORD *)v13 + 68) - 256;
        }
        else
        {
          v17 = 256;
        }
      }
      else
      {
        v17 = *((DWORD *)v13 + 68);
      }
      if ( v16 )
      {
        v18 = *((DWORD *)v13 + 75);
      }
      else
      {
        v18 = *((DWORD *)v13 + 74);
      }
      v19 = v13[69];
      DAT_055c9b90 = v18;
      a4 = v16 << 8;
      FUN_004108b0(
        (__int64)(a5 * g_fScreenRate_x + (double)(v16 << 8)),
        (__int64)(a2 * g_fScreenRate_y),
        *(float *)&v17,
        v19);
      v15 = *((DWORD *)v13 + 76);
      ++v16;
    }
    while ( v16 < (v15 != 0) + 1 );
  }
  ++*((DWORD *)v13 + 71);
  return 1;
}
#endif
