// 0x0040C2A0 FUN_0040c2a0 — nunca activado: IDA_PORT_0040C2A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c2a0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C2A0)
int __cdecl FUN_0040c2a0(LPCSTR lpString, int a2, int a3, int a4, size_t Count, int a6, int a7)
{
  const CHAR *v7; // edi
  int v8; // ebx
  size_t v10; // eax
  int v11; // eax
  int v12; // esi
  int v13; // ebp
  int v14; // ebx
  int v15; // esi
  double v16; // st7
  signed int j; // esi
  size_t i; // [esp+10h] [ebp-20h]
  int v19; // [esp+14h] [ebp-1Ch]
  char *Destination; // [esp+18h] [ebp-18h]
  int v21; // [esp+20h] [ebp-10h]
  struct tagSIZE sz; // [esp+28h] [ebp-8h] BYREF
  LPCSTR lpStringa; // [esp+34h] [ebp+4h]

  v7 = lpString;
  v8 = 0;
  if ( !lpString )
  {
    return 0;
  }
  lpStringa = 0;
  v10 = Count * (a4 - 1);
  for ( i = v10; ; i -= Count )
  {
    v21 = v8 + 1;
    if ( v8 + 1 > a4 )
    {
      break;
    }
    if ( a7 != 1 )
    {
      v10 = (size_t)lpStringa;
    }
    Destination = (char *)(a2 + v10);
    v11 = lstrlenA(v7);
    GetTextExtentPointA(m_hFontDC, v7, v11, &sz);
    v12 = (__int64)((double)sz.cx / g_fScreenRate_x);
    if ( !sz.cx )
    {
      return v8;
    }
    v13 = a3 - (v8 == 0 ? a6 : 0);
    if ( v12 <= v13 )
    {
      strncpy(Destination, v7, Count);
      return v8 + 1;
    }
    v14 = v13 / (v12 / lstrlenA(v7));
    v15 = (__int64)(((double)v14 + 1.0) * 0.5);
    v19 = v15;
    while ( v15 )
    {
      v15 = (__int64)(((double)v19 + 1.0) * 0.5);
      v19 = v15;
      GetTextExtentPointA(m_hFontDC, v7, v14, &sz);
      v16 = (double)sz.cx / g_fScreenRate_x;
      if ( v16 <= (double)(v13 + 4) )
      {
        if ( v16 >= (double)(v13 - 4) )
        {
          break;
        }
        v14 += v15;
      }
      else
      {
        v14 -= v15;
        if ( v15 == 1 )
        {
          break;
        }
      }
    }
    for ( j = 0; j < v14; j += _mbclen((const unsigned char *)&v7[j]) )
    {
      ;
    }
    strncpy(Destination, v7, j);
    lpStringa += Count;
    v7 += j;
    v10 = i - Count;
    Destination[j] = 0;
    v8 = v21;
  }
  return v8 + 1;
}
#endif
