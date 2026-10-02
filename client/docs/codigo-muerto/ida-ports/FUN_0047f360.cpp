// 0x0047F360 FUN_0047f360 — nunca activado: IDA_PORT_0047F360 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0047f360 (IDA-only, gated) ──
#if defined(IDA_PORT_0047F360)
int __cdecl FUN_0047f360(int a1, int a2, LPCSTR a3, int a4, int a5, int x, int a7, int a8, LPCSTR lpString)
{
  int v9; // esi
  int v10; // edi
  bool v11; // zf
  int v12; // eax
  __int64 Height; // rax
  BYTE *Buffer; // ecx
  char *v15; // ebp
  LONG v16; // eax
  BYTE *i; // esi
  DWORD v18; // edx
  struct tagSIZE sz; // [esp+10h] [ebp-8h] BYREF
  int v21; // [esp+1Ch] [ebp+4h]
  BYTE *v22; // [esp+20h] [ebp+8h]

  v9 = a2;
  v10 = a1;
  v11 = *a3 == 10;
  sz.cx = a1;
  sz.cy = a2;
  if ( !v11 )
  {
    if ( lpString )
    {
      v12 = lstrlenA(lpString);
      GetTextExtentPointA(m_hFontDC, lpString, v12, &sz);
      TextOutA(m_hFontDC, x, 0, lpString, strlen(lpString));
    }
    else
    {
      sz.cx = 0;
    }
    SetTextColor(m_hFontDC, (COLORREF)&DAT_00ffffff);
    TextOutA(m_hFontDC, x + sz.cx, 0, a3, strlen(a3));
    v9 = a2;
    v10 = a1;
  }
  if ( !a8 )
  {
    a8 = v10;
  }
  Height = (__int64)Bitmaps[0].Height;
  if ( v9 > (int)Height )
  {
    v9 = (__int64)Bitmaps[0].Height;
  }
  if ( v9 > 0 )
  {
    Buffer = Bitmaps[0].Buffer;
    v15 = (char *)ppvBits;
    v22 = Bitmaps[0].Buffer;
    v21 = v9;
    do
    {
      v16 = 0;
      for ( i = v15; v16 < v10; ++v16 )
      {
        if ( *i )
        {
          if ( v16 >= sz.cx )
          {
            v18 = m_dwTextColor;
          }
          else
          {
            v18 = SetTextColor_0;
          }
        }
        else
        {
          v18 = v16 >= a8 ? 0 : m_dwBackColor;
        }
        *(DWORD *)Buffer = v18;
        i += 3;
        Buffer += 4;
      }
      v15 += 1536;
      Buffer = v22 + 1024;
      LODWORD(Height) = v21 - 1;
      v11 = v21 == 1;
      v22 += 1024;
      --v21;
    }
    while ( !v11 );
  }
  return Height;
}
#endif
