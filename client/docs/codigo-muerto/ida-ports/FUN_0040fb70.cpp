// 0x0040FB70 FUN_0040fb70 — nunca activado: IDA_PORT_0040FB70 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040fb70 (IDA-only, gated) ──
#if defined(IDA_PORT_0040FB70)
double __cdecl FUN_0040fb70(char *_this, int a2, int a3, LPCSTR lpString, int a5, int a6, int a7, int a8, int a9)
{
  int v10; // eax
  int v11; // esi
  double v12; // st7
  double v13; // st6
  int v14; // edi
  struct tagSIZE sz; // [esp+Ch] [ebp-8h] BYREF
  LPCSTR lpStringa; // [esp+20h] [ebp+Ch]

  if ( !lpString || !*lpString || !strlen(lpString) && !a5 )
  {
    return 0.0;
  }
  v10 = lstrlenA(lpString);
  GetTextExtentPointA(m_hFontDC, lpString, v10, &sz);
  v11 = 0;
  lpStringa = (LPCSTR)sz.cx;
  switch ( a7 )
  {
    case 1:
      if ( a5 > 0 )
      {
        lpStringa = (LPCSTR)a5;
      }
      break;
    case 2:
      v11 = (a5 - sz.cx) / 2;
      lpStringa = (LPCSTR)(sz.cx + 2 * v11);
      break;
    case 3:
      v11 = a5 - sz.cx;
      lpStringa = (LPCSTR)a5;
      break;
  }
  v12 = (double)(int)lpStringa / g_fScreenRate_x;
  v13 = (double)a9;
  if ( (double)a2 + v12 <= v13 )
  {
    v14 = a2;
  }
  else
  {
    v14 = (__int64)(v13 - v12);
  }
  if ( !FUN_004106d0(lpString, v14, a3, v11) )
  {
    FUN_0040fcd0(_this, (char *)lpString, a5, a6, v11, a7, a8);
    FUN_004106d0(lpString, v14, a3, v11);
  }
  if ( *lpString == 10 )
  {
    return (double)sz.cy / g_fScreenRate_y / 2.0;
  }
  else
  {
    return (double)sz.cy / g_fScreenRate_y / 1.0;
  }
}
#endif
