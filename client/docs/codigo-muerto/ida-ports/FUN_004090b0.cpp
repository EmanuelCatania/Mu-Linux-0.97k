// 0x004090B0 FUN_004090b0 — nunca activado: IDA_PORT_004090B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004090b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004090B0)
void __cdecl FUN_004090b0(DWORD *_this, int a2, GLfloat t, int a4)
{
  GLfloat v5; // ebp
  int i; // edi
  int j; // edi
  GLfloat ta; // [esp+18h] [ebp+8h]

  BindTexture(SLODWORD(t));
  glBegin(7u);
  v5 = 0.0;
  if ( a2 )
  {
    ta = 0.0;
    if ( _this[11] - 1 > 0 )
    {
      do
      {
        for ( i = 0; i < _this[10] - 1; ++i )
        {
          FUN_004091d0(a4, i, ta);
          FUN_004091d0(a4, i + 1, ta);
          FUN_004091d0(a4, i + 1, COERCE_GLFLOAT(LODWORD(ta) + 1));
          FUN_004091d0(a4, i, COERCE_GLFLOAT(LODWORD(ta) + 1));
        }
        ++LODWORD(ta);
      }
      while ( SLODWORD(ta) < _this[11] - 1 );
    }
  }
  else if ( _this[11] - 1 > 0 )
  {
    do
    {
      for ( j = 0; j < _this[10] - 1; FUN_004091d0(a4, j, v5) )
      {
        FUN_004091d0(a4, j, v5);
        FUN_004091d0(a4, j++, COERCE_GLFLOAT(LODWORD(v5) + 1));
        FUN_004091d0(a4, j, COERCE_GLFLOAT(LODWORD(v5) + 1));
      }
      ++LODWORD(v5);
    }
    while ( SLODWORD(v5) < _this[11] - 1 );
  }
  glEnd();
}
#endif
