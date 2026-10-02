// 0x00411920 FUN_00411920 — nunca activado: IDA_PORT_00411920 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411920 (IDA-only, gated) ──
#if defined(IDA_PORT_00411920)
int __cdecl FUN_00411920(DWORD *_this)
{
  int i; // edi
  int v3; // eax

  (*(void (__cdecl **)(DWORD *))(*_this + 88))(_this);
  (*(void (__cdecl **)(DWORD *))(*_this + 80))(_this);
  glColor3f(1.0, 1.0, 1.0);
  SelectObject(m_hFontDC, g_hFont);
  for ( i = 0; i < _this[35]; _this[25] = *(DWORD *)_this[25] )
  {
    if ( _this[25] == _this[23] )
    {
      break;
    }
    v3 = (*(int (__cdecl **)(DWORD *, int))(*_this + 92))(_this, i);
    if ( v3 >= 0 )
    {
      if ( !v3 )
      {
        --i;
      }
    }
    else
    {
      i -= v3;
    }
    ++i;
  }
  return (*(int (__cdecl **)(DWORD *))(*_this + 96))(_this);
}
#endif
