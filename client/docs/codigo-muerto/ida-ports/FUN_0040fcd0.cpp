// 0x0040FCD0 FUN_0040fcd0 — nunca activado: IDA_PORT_0040FCD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040fcd0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040FCD0)
void __cdecl FUN_0040fcd0(char *_this, char *Source, int a3, int a4, int x, int a6, int a7)
{
  int v8; // eax
  int cx; // eax
  int cy; // ecx
  int v11; // ebp
  signed int i; // ebp
  int v13; // eax
  GLuint v14; // edi
  int v15; // edx
  int j; // ecx
  char *v17; // ebx
  bool v18; // al
  DWORD *v19; // ecx
  DWORD *v20; // edi
  DWORD *v21; // ebp
  DWORD *v22; // esi
  DWORD *v23; // eax
  int v24; // eax
  DWORD *v25; // eax
  DWORD *v26; // eax
  DWORD *v27; // edx
  DWORD *v28; // ecx
  int v29; // ecx
  int v30; // [esp-Ch] [ebp-284h]
  int v31; // [esp-Ch] [ebp-284h]
  unsigned int v32; // [esp+Ch] [ebp-26Ch] BYREF
  char v33; // [esp+11h] [ebp-267h] BYREF
  char v34; // [esp+12h] [ebp-266h] BYREF
  char v35; // [esp+13h] [ebp-265h] BYREF
  int v36; // [esp+14h] [ebp-264h] BYREF
  int v37; // [esp+18h] [ebp-260h] BYREF
  struct tagSIZE sz; // [esp+24h] [ebp-254h] BYREF
  char Destination[292]; // [esp+2Ch] [ebp-24Ch] BYREF
  int v40; // [esp+150h] [ebp-128h] BYREF
  char v41[292]; // [esp+154h] [ebp-124h] BYREF

  if ( Source && strlen(Source) <= 0xFF )
  {
    memset(Destination, 0, sizeof(Destination));
    strncpy(Destination, Source, 0x100u);
    FUN_004104b0((LONG)_this, Source);
    *(DWORD *)&Destination[272] = m_dwTextColor;
    *(DWORD *)&Destination[276] = m_dwBackColor;
    *(DWORD *)&Destination[264] = x;
    v8 = lstrlenA(Source);
    GetTextExtentPointA(m_hFontDC, Source, v8, &sz);
    cx = a3;
    if ( a3 )
    {
      v37 = a3;
    }
    else
    {
      cx = sz.cx;
      v37 = sz.cx;
    }
    cy = a4;
    if ( !a4 )
    {
      cy = sz.cy;
    }
    v36 = cy;
    if ( *Source != 10 )
    {
      v11 = 0;
      if ( v36 > 0 )
      {
        v32 = 3 * cx;
        do
        {
          memset((char *)ppvBits + 768 * v11 * DAT_005590bc, 0, v32);
          ++v11;
        }
        while ( v11 < v36 );
      }
      SetTextColor(m_hFontDC, (COLORREF)&DAT_00ffffff);
      TextOutA(m_hFontDC, x, 0, Source, strlen(Source));
      cx = v37;
    }
    *(DWORD *)&Destination[256] = cx;
    *(DWORD *)&Destination[260] = v36;
    if ( cx <= 256 )
    {
      v32 = 1;
    }
    else
    {
      v32 = 2;
      *(DWORD *)&Destination[288] = 1;
    }
    for ( i = 0; i < (int)v32; ++i )
    {
      if ( v32 == 1 )
      {
        v13 = v37;
      }
      else if ( i )
      {
        v13 = v37 - 256;
      }
      else
      {
        v13 = 256;
      }
      FUN_004105f0(Bitmaps[0].Buffer, i << 8, v13, v36);
      v14 = Pool_AllocNextSlot(_this);
      glBindTexture(0xDE1u, v14);
      glPixelStorei(0xCF5u, 1);
      glTexEnvf(0x2300u, 0x2200u, 8448.0);
      glTexParameteri(0xDE1u, 0x2800u, 9728);
      glTexParameteri(0xDE1u, 0x2801u, 9728);
      glTexParameteri(0xDE1u, 0x2802u, 10496);
      glTexParameteri(0xDE1u, 0x2803u, 10496);
      glTexImage2D(
        0xDE1u,
        0,
        Bitmaps[0].Components,
        (__int64)Bitmaps[0].Width,
        (__int64)Bitmaps[0].Height,
        0,
        0x1908u,
        0x1401u,
        Bitmaps[0].Buffer);
      if ( i )
      {
        *(DWORD *)&Destination[284] = v14;
      }
      else
      {
        *(DWORD *)&Destination[280] = v14;
      }
    }
    v15 = 0;
    for ( j = 0; j < 4; ++j )
    {
      if ( !Destination[j] )
      {
        break;
      }
      v15 += (unsigned char)Destination[j];
    }
    v40 = v15;
    v17 = _this + 4;
    v18 = 1;
    qmemcpy(v41, Destination, sizeof(v41));
    v19 = (DWORD *)*((DWORD *)v17 + 1);
    v20 = v19;
    v21 = (DWORD *)v19[1];
    while ( v21 != (DWORD *)DAT_055c9b98 )
    {
      v20 = v21;
      v18 = v15 < v21[3];
      if ( v15 >= v21[3] )
      {
        v21 = (DWORD *)v21[2];
      }
      else
      {
        v21 = (DWORD *)*v21;
      }
    }
    if ( v17[8] )
    {
      v22 = (DWORD *)FUN_00411840(v20, 0);
      *v22 = DAT_055c9b98;
      v22[2] = DAT_055c9b98;
      FUN_004124d0(v22 + 3, &v40);
      v23 = (DWORD *)*((DWORD *)v17 + 1);
      ++*((DWORD *)v17 + 3);
      if ( v20 == v23 || v21 != (DWORD *)DAT_055c9b98 || v40 < v20[3] )
      {
        *v20 = v22;
        v25 = (DWORD *)*((DWORD *)v17 + 1);
        if ( v20 == v25 )
        {
          v25[1] = v22;
          *(DWORD *)(*((DWORD *)v17 + 1) + 8) = v22;
        }
        else if ( v20 == (DWORD *)*v25 )
        {
          *v25 = v22;
        }
      }
      else
      {
        v20[2] = v22;
        v24 = *((DWORD *)v17 + 1);
        if ( v20 == *(DWORD **)(v24 + 8) )
        {
          *(DWORD *)(v24 + 8) = v22;
        }
      }
      while ( v22 != *(DWORD **)(*((DWORD *)v17 + 1) + 4) )
      {
        v26 = (DWORD *)v22[1];
        if ( v26[77] )
        {
          break;
        }
        v27 = (DWORD *)v26[1];
        v28 = (DWORD *)*v27;
        if ( v26 == (DWORD *)*v27 )
        {
          v29 = v27[2];
          if ( *(DWORD *)(v29 + 308) )
          {
            if ( v22 == (DWORD *)v26[2] )
            {
              v22 = (DWORD *)v22[1];
              FUN_00411700(v26);
            }
            *(DWORD *)(v22[1] + 308) = 1;
            *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
            FUN_00411760(*(DWORD *)(v22[1] + 4));
          }
          else
          {
            v26[77] = 1;
            *(DWORD *)(v29 + 308) = 1;
            *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
            v22 = *(DWORD **)(v22[1] + 4);
          }
        }
        else if ( v28[77] )
        {
          if ( v22 == (DWORD *)*v26 )
          {
            v22 = (DWORD *)v22[1];
            FUN_00411760(v26);
          }
          *(DWORD *)(v22[1] + 308) = 1;
          *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
          FUN_00411700(*(DWORD *)(v22[1] + 4));
        }
        else
        {
          v26[77] = 1;
          v28[77] = 1;
          *(DWORD *)(*(DWORD *)(v22[1] + 4) + 308) = 0;
          v22 = *(DWORD **)(v22[1] + 4);
        }
      }
      *(DWORD *)(*(DWORD *)(*((DWORD *)v17 + 1) + 4) + 308) = 1;
      goto LABEL_66;
    }
    v32 = (unsigned int)v20;
    if ( v18 )
    {
      if ( v20 == (DWORD *)*v19 )
      {
        v35 = 1;
        v30 = FUN_00411460(&v36, v21, v20, &v40);
        FUN_00411820(v30, &v35);
LABEL_66:
        if ( strcmp(Source, Destination) )
        {
          strncpy(Source, Destination, 0x100u);
        }
        return;
      }
      FUN_00411870(&v32);
    }
    if ( *(DWORD *)(v32 + 12) >= v40 )
    {
      v34 = 0;
      FUN_00411820(&v32, &v34);
    }
    else
    {
      v33 = 1;
      v31 = FUN_00411460(&v37, v21, v20, &v40);
      FUN_00411820(v31, &v33);
    }
    goto LABEL_66;
  }
}
#endif
