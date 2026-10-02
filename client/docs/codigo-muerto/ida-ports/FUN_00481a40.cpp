// 0x00481A40 FUN_00481a40 — nunca activado: IDA_PORT_00481A40 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00481a40 (IDA-only, gated) ──
#if defined(IDA_PORT_00481A40)
void __cdecl FUN_00481a40(int x, int y, DWORD c)
{
  double v3; // st7
  int v4; // eax
  LONG v5; // edi
  LONG v6; // ecx
  int j; // eax
  int v9; // esi
  char *v10; // edx
  char *v11; // edi
  char v12; // al
  int v13; // ecx
  char v14; // bl
  int v15; // ecx
  DWORD v16; // eax
  float v17; // edi
  char v18; // al
  unsigned char v19; // bl
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // eax
  int v24; // eax
  int v25; // ebx
  int cx; // esi
  LONG v27; // ecx
  int i; // eax
  int v29; // eax
  char *v30; // edx
  unsigned int v31; // esi
  char *v32; // edi
  char v33; // al
  DWORD v34; // edi
  int v35; // ecx
  int v36; // ecx
  char v37; // al
  int v38; // eax
  int v39; // eax
  int v40; // esi
  char *v41; // edx
  unsigned int v42; // ecx
  char *v43; // edi
  int v44; // eax
  int v45; // ecx
  int v46; // eax
  char *v47; // edx
  char *v48; // edi
  int v49; // eax
  int v50; // esi
  int v51; // eax
  char *v52; // edx
  char *v53; // edi
  const char *v54; // [esp-8h] [ebp-140h]
  GLfloat red; // [esp+0h] [ebp-138h]
  GLfloat reda; // [esp+0h] [ebp-138h]
  GLfloat green; // [esp+4h] [ebp-134h]
  GLfloat greena; // [esp+4h] [ebp-134h]
  float greenb; // [esp+4h] [ebp-134h]
  GLfloat blue; // [esp+8h] [ebp-130h]
  float bluea; // [esp+8h] [ebp-130h]
  GLfloat blueb; // [esp+8h] [ebp-130h]
  float bluec; // [esp+8h] [ebp-130h]
  GLfloat alpha; // [esp+Ch] [ebp-12Ch]
  float alphaa; // [esp+Ch] [ebp-12Ch]
  float alphab; // [esp+Ch] [ebp-12Ch]
  int ya; // [esp+20h] [ebp-118h]
  int yd; // [esp+20h] [ebp-118h]
  int ye; // [esp+20h] [ebp-118h]
  int yf; // [esp+20h] [ebp-118h]
  float yb; // [esp+20h] [ebp-118h]
  int yc; // [esp+20h] [ebp-118h]
  unsigned int v73; // [esp+24h] [ebp-114h]
  unsigned char v74; // [esp+24h] [ebp-114h]
  unsigned char v75; // [esp+24h] [ebp-114h]
  int v76; // [esp+24h] [ebp-114h]
  int v77; // [esp+24h] [ebp-114h]
  unsigned int v78; // [esp+24h] [ebp-114h]
  unsigned int v79; // [esp+24h] [ebp-114h]
  int a2; // [esp+28h] [ebp-110h]
  int a2b; // [esp+28h] [ebp-110h]
  int a2a; // [esp+28h] [ebp-110h]
  unsigned char v83; // [esp+2Ch] [ebp-10Ch] BYREF
  short v84; // [esp+2Dh] [ebp-10Bh]
  int v85; // [esp+30h] [ebp-108h]
  float xa; // [esp+34h] [ebp-104h]
  char a4[253]; // [esp+38h] [ebp-100h] BYREF
  short v88; // [esp+135h] [ebp-3h]
  char v89; // [esp+137h] [ebp-1h]
  int v90; // [esp+13Ch] [ebp+4h]
  int a3; // [esp+140h] [ebp+8h]

  if ( *(DWORD *)(g_pRenderText + 8) != 1 )
  {
    EnableAlphaTest(1);
    glColor3f(1.0, 1.0, 1.0);
    v25 = 1;
    cx = *(DWORD *)(c + 576);
    v27 = *(DWORD *)(c + 580);
    TextSize.cx = cx;
    TextSize.cy = v27;
    do
    {
      if ( v25 >= cx )
      {
        break;
      }
      v25 *= 2;
    }
    while ( v25 < 256 );
    for ( i = 1; i < 256; i *= 2 )
    {
      if ( i >= v27 )
      {
        break;
      }
    }
    yc = i;
    switch ( *(BYTE *)(c + 36) )
    {
      case 0:
        m_dwTextColor = -983146;
        break;
      case 1:
        m_dwTextColor = -34716;
        break;
      case 2:
        m_dwTextColor = -19316;
        break;
      case 3:
        m_dwTextColor = -9016;
        break;
      case 4:
        m_dwTextColor = -12806401;
        break;
      case 5:
        m_dwTextColor = -14790401;
        break;
      default:
        m_dwTextColor = -16776961;
        break;
    }
    DAT_07e11d6e = 1;
    v90 = (int)(x * WindowWidth) / 640;
    v29 = FontHeight;
    a3 = (int)(y * WindowHeight) / 480;
    if ( FontHeight > 32 )
    {
      v29 = 32;
      FontHeight = 32;
    }
    if ( v29 > 0 )
    {
      v30 = (char *)ppvBits;
      v76 = v29;
      while ( 1 )
      {
        if ( cx > 512 )
        {
          cx = 512;
          TextSize.cx = 512;
        }
        v31 = 3 * cx;
        memset(v30, 0, 4 * (v31 >> 2));
        v32 = &v30[4 * (v31 >> 2)];
        v30 += 1536;
        memset(v32, 0, v31 & 3);
        if ( !--v76 )
        {
          break;
        }
        cx = TextSize.cx;
      }
    }
    v33 = *(BYTE *)(c + 37);
    if ( v33 )
    {
      if ( v33 == 1 )
      {
        v34 = -1778359236;
        SetTextColor_0 = -16711736;
        m_dwBackColor = -1778359236;
      }
      else
      {
        v34 = -1778384796;
        SetTextColor_0 = -16776961;
        m_dwBackColor = -1778384796;
      }
    }
    else
    {
      v34 = -1773129196;
      SetTextColor_0 = -14116;
      m_dwBackColor = -1773129196;
    }
    v35 = *(DWORD *)(c + 568);
    if ( v35 <= MouseX && MouseX < (int)(v35 + 640 * *(DWORD *)(c + 576) / WindowWidth) )
    {
      v36 = *(DWORD *)(c + 572);
      if ( v36 <= MouseY
        && MouseY < (int)(v36 + 480 * *(DWORD *)(c + 580) / WindowHeight)
        && InputEnable
        && *(BYTE *)(Hero + 846)
        && strcmp((const char *)c, (const char *)(Hero + 449))
        && DAT_07e11da8 % 6 < 3 )
      {
        m_dwBackColor = m_dwTextColor;
        m_dwTextColor = v34;
      }
    }
    FUN_0047f360(TextSize.cx, FontHeight, (LPCSTR)c, v25, 0, 0, 0, 0, (LPCSTR)(c + 24));
    FUN_0047f4c0(v90, a3, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
    v37 = *(BYTE *)(c + 37);
    if ( v37 )
    {
      m_dwBackColor = v37 != 1 ? -1778384846 : -1778372066;
    }
    else
    {
      m_dwBackColor = -1775100406;
    }
    v38 = *(DWORD *)(c + 560);
    if ( v38 <= 0 )
    {
      v49 = *(DWORD *)(c + 556);
      if ( v49 > 0 )
      {
        m_dwTextColor = -3613466;
        if ( v49 < 10 )
        {
          m_dwTextColor = -2134319898;
        }
        v50 = FontHeight;
        v51 = TextSize.cx;
        if ( FontHeight > 0 )
        {
          v79 = 3 * TextSize.cx;
          v52 = (char *)ppvBits;
          do
          {
            memset(v52, 0, 4 * (v79 >> 2));
            v53 = &v52[4 * (v79 >> 2)];
            v52 += 1536;
            --v50;
            memset(v53, 0, v79 & 3);
          }
          while ( v50 );
          v50 = FontHeight;
          v51 = TextSize.cx;
        }
        FUN_0047f360(v51, v50, (LPCSTR)(c + 44), v25, 0, 0, 0, 0, 0);
        FUN_0047f4c0(v90, a3 + FontHeight, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
      }
    }
    else
    {
      m_dwTextColor = -3613466;
      if ( v38 < 10 )
      {
        m_dwTextColor = -2134319898;
      }
      v39 = FontHeight;
      v40 = TextSize.cx;
      if ( FontHeight > 0 )
      {
        v41 = (char *)ppvBits;
        v77 = FontHeight;
        do
        {
          v42 = (unsigned int)(3 * v40) >> 2;
          memset(v41, 0, 4 * v42);
          v43 = &v41[4 * v42];
          v41 += 1536;
          memset(v43, 0, (3 * (BYTE)v40) & 3);
          --v77;
        }
        while ( v77 );
        v39 = FontHeight;
      }
      FUN_0047f360(v40, v39, (LPCSTR)(c + 300), v25, 0, 0, 0, 0, 0);
      FUN_0047f4c0(v90, a3 + FontHeight, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
      v44 = *(DWORD *)(c + 556);
      m_dwTextColor = -3613466;
      if ( v44 < 10 )
      {
        m_dwTextColor = -2134319898;
      }
      v45 = FontHeight;
      v46 = TextSize.cx;
      if ( FontHeight > 0 )
      {
        a2a = FontHeight;
        v78 = 3 * TextSize.cx;
        v47 = (char *)ppvBits;
        do
        {
          memset(v47, 0, 4 * (v78 >> 2));
          v48 = &v47[4 * (v78 >> 2)];
          v47 += 1536;
          memset(v48, 0, v78 & 3);
          --a2a;
        }
        while ( a2a );
        v46 = TextSize.cx;
        v45 = FontHeight;
      }
      FUN_0047f360(v46, v45, (LPCSTR)(c + 44), v25, 0, 0, 0, 0, 0);
      FUN_0047f4c0(v90, a3 + 2 * FontHeight, *(float *)&TextSize.cx, *(float *)&FontHeight, v25, yc, 0.0, 640);
    }
    return;
  }
  EnableAlphaTest(1);
  glColor3f(1.0, 1.0, 1.0);
  v4 = 1;
  v5 = *(DWORD *)(c + 576);
  v6 = *(DWORD *)(c + 580);
  TextSize.cx = v5;
  TextSize.cy = v6;
  do
  {
    if ( v4 >= v5 )
    {
      break;
    }
    v4 *= 2;
  }
  while ( v4 < 256 );
  for ( j = 1; j < 256; j *= 2 )
  {
    if ( j >= v6 )
    {
      break;
    }
  }
  switch ( *(BYTE *)(c + 36) )
  {
    case 0:
      m_dwTextColor = -983146;
      break;
    case 1:
      m_dwTextColor = -34716;
      break;
    case 2:
      m_dwTextColor = -19316;
      break;
    case 3:
      m_dwTextColor = -9016;
      break;
    case 4:
      m_dwTextColor = -12806401;
      break;
    case 5:
      m_dwTextColor = -14790401;
      break;
    default:
      m_dwTextColor = -16776961;
      break;
  }
  a2 = x;
  if ( x < 0 )
  {
    a2 = 0;
  }
  __asm
  {
    fild    [esp+128h+a2]
    fmul    g_fScreenRate_x
    fiadd   TextSize._cx
    fild    WindowWidth
    fcompp
    fnstsw  ax
  }
  if ( (_AX & 0x100) != 0 )
  {
    a2b = WindowWidth - v5;
    __asm
    {
      fild    [esp+128h+a2]
      fdiv    g_fScreenRate_x
    }
    a2 = _ftol(v3);
  }
  v9 = FontHeight;
  if ( FontHeight > 0 )
  {
    v73 = 3 * v5;
    v10 = (char *)ppvBits;
    do
    {
      memset(v10, 0, 4 * (v73 >> 2));
      v11 = &v10[4 * (v73 >> 2)];
      v10 += 768;
      --v9;
      memset(v11, 0, v73 & 3);
    }
    while ( v9 );
  }
  v12 = *(BYTE *)(c + 37);
  (BYTE)(ya) = -106;
  if ( v12 )
  {
    if ( v12 == 1 )
    {
      v74 = 60;
      (BYTE)(v85) = 100;
    }
    else
    {
      v74 = 100;
      (BYTE)(v85) = 0;
    }
    v83 = 0;
  }
  else
  {
    v74 = 20;
    (BYTE)(v85) = 50;
    v83 = 80;
  }
  v13 = *(DWORD *)(c + 568);
  v14 = 0;
  m_dwBackColor = 0;
  if ( v13 <= MouseX && MouseX < (int)(v13 + 640 * *(DWORD *)(c + 576) / WindowWidth) )
  {
    v15 = *(DWORD *)(c + 572);
    if ( v15 <= MouseY
      && MouseY < (int)(v15 + 480 * *(DWORD *)(c + 580) / WindowHeight)
      && InputEnable
      && *(BYTE *)(Hero + 846)
      && strcmp((const char *)c, (const char *)(Hero + 449))
      && DAT_07e11da8 % 6 < 3 )
    {
      v16 = m_dwTextColor;
      m_dwTextColor = -16777216;
      v74 = v16;
      v14 = 3;
      (BYTE)(v85) = ((BYTE)((v16) >> 8));
      v83 = ((BYTE)((v16) >> 16));
      (BYTE)(ya) = (BYTE)((v16) >> 8);
    }
  }
  ya = (unsigned char)ya;
  __asm { fild    [esp+12Ch+y] }
  yd = v83;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+12Ch+alpha]; alpha
    fild    [esp+12Ch+y]
    fmul    ds:DAT_00552b70
  }
  ye = (unsigned char)v85;
  __asm
  {
    fstp    [esp+130h+blue]; blue
    fild    [esp+130h+y]
  }
  yf = v74;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+134h+green]; green
    fild    [esp+134h+y]
    fmul    ds:DAT_00552b70
    fstp    [esp+138h+red]; red
  }
  glColor4f(red, green, blue, alpha);
  __asm
  {
    fild    [esp+128h+a3]
    fstp    [esp+12Ch+y]
    fild    [esp+12Ch+a2]
    fstp    [esp+12Ch+x]
    fild    FontHeight
  }
  v17 = xa;
  __asm
  {
    fdiv    g_fScreenRate_y
    fstp    [esp+12Ch+alpha]; Height
    fild    TextSize._cx
    fdiv    g_fScreenRate_x
    fstp    [esp+130h+blue]; Width
  }
  RenderColor(xa, yb, bluea, alphaa);
  v18 = *(BYTE *)(c + 37);
  if ( v18 )
  {
    if ( v18 == 1 )
    {
      v75 = 30;
      (BYTE)(v85) = 50;
    }
    else
    {
      v75 = 50;
      (BYTE)(v85) = 0;
    }
    v83 = 0;
  }
  else
  {
    v75 = 10;
    (BYTE)(v85) = 30;
    v83 = 50;
  }
  LODWORD(xa) = v83;
  __asm { fild    [esp+130h+x] }
  LODWORD(xa) = (unsigned char)v85;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+130h+blue]; blue
    fild    [esp+130h+x]
  }
  LODWORD(xa) = v75;
  __asm
  {
    fmul    ds:DAT_00552b70
    fstp    [esp+134h+green]; green
    fild    [esp+134h+x]
    fmul    ds:DAT_00552b70
    fstp    [esp+138h+red]; red
  }
  glColor4f(reda, greena, blueb, 0.58823532);
  if ( *(int *)(c + 560) > 0 )
  {
    __asm
    {
      fild    FontHeight
      fld     st
      fadd    st, st
    }
LABEL_48:
    __asm
    {
      fdiv    g_fScreenRate_y
      fstp    [esp+12Ch+alpha]; Height
      fild    TextSize._cx
      fdiv    g_fScreenRate_x
      fstp    [esp+130h+blue]; Width
      fdiv    g_fScreenRate_x
      fadd    [esp+134h+y]
      fstp    [esp+134h+green]; y
    }
    RenderColor(v17, greenb, bluec, alphab);
    goto LABEL_49;
  }
  if ( *(int *)(c + 556) > 0 )
  {
    __asm
    {
      fild    FontHeight
      fld     st
    }
    goto LABEL_48;
  }
LABEL_49:
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glEnable(0xDE1u);
  a4[0] = DAT_07e11de0;
  memset(&a4[1], 0, 0xFCu);
  v88 = 0;
  v89 = 0;
  if ( *(BYTE *)(c + 24) )
  {
    v19 = *(BYTE *)(c + 37) + v14 - 14;
    v83 = 2;
    v84 = v19;
    strcat(a4, (const char *)&v83);
    (BYTE)(v84) = -16;
    strcat(a4, (const char *)(c + 24));
    strcat(a4, (const char *)&v83);
  }
  strcat(a4, (const char *)c);
  CUIRenderText::RenderText(g_pRenderText, a2, y, a4, 0, 0, 1, 0, 640);
  v20 = *(DWORD *)(c + 560);
  __asm { fstp    st }
  if ( v20 > 0 )
  {
    m_dwTextColor = -3613466;
    if ( v20 < 10 )
    {
      m_dwTextColor = -2134319898;
    }
    __asm
    {
      fild    FontHeight
      fdiv    g_fScreenRate_y
      fadd    [esp+140h+y]
      fadd    ds:DAT_00552504
    }
    v21 = _ftol(v3);
    CUIRenderText::RenderText(g_pRenderText, a2, v21, (const char *)(c + 300), 0, 0, 1, 0, 640);
    v22 = *(DWORD *)(c + 556);
    m_dwTextColor = -3613466;
    __asm { fstp    st }
    if ( v22 < 10 )
    {
      m_dwTextColor = -2134319898;
    }
    __asm
    {
      fild    FontHeight
      fdiv    g_fScreenRate_y
    }
    v54 = (const char *)(c + 44);
    __asm { fadd    st, st }
LABEL_57:
    __asm
    {
      fadd    [esp+140h+y]
      fadd    ds:DAT_00552504
    }
    v23 = _ftol(v3);
    CUIRenderText::RenderText(g_pRenderText, a2, v23, v54, 0, 0, 1, 0, 640);
    __asm { fstp    st }
    return;
  }
  v24 = *(DWORD *)(c + 556);
  if ( v24 > 0 )
  {
    m_dwTextColor = -3613466;
    if ( v24 < 10 )
    {
      m_dwTextColor = -2134319898;
    }
    __asm
    {
      fild    FontHeight
      fdiv    g_fScreenRate_y
    }
    v54 = (const char *)(c + 44);
    goto LABEL_57;
  }
}
#endif
