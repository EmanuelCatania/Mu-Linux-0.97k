// 0x004091D0 FUN_004091d0 — nunca activado: IDA_PORT_004091D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004091d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004091D0)
void __cdecl FUN_004091d0(DWORD *_this, int a2, int a3, GLfloat t)
{
  int v4; // edx
  double v5; // st7
  GLfloat *v6; // esi
  GLfloat s; // [esp+0h] [ebp-Ch]
  float ta; // [esp+18h] [ebp+Ch]

  v4 = _this[10];
  v5 = (double)SLODWORD(t) / (double)(_this[11] - 1);
  v6 = (GLfloat *)(a2 + 12 * (a3 + LODWORD(t) * v4));
  if ( v5 <= 0.99000001 )
  {
    ta = v5;
  }
  else
  {
    ta = 0.99000001;
  }
  s = (double)a3 / (double)(v4 - 1);
  glTexCoord2f(s, ta);
  glVertex3f(*v6, v6[1], v6[2]);
}
#endif
