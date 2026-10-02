// 0x0040A8F0 FUN_0040a8f0 — nunca activado: IDA_PORT_0040A8F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a8f0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A8F0)
void __cdecl FUN_0040a8f0(char *_this, int a2, GLfloat a3, int a4, int a5)
{
  double v6; // st7
  float v7; // ecx
  float v8; // edx
  float v9; // edx
  double v10; // st7
  GLfloat x; // [esp+0h] [ebp-50h]
  GLfloat xa; // [esp+0h] [ebp-50h]
  GLfloat xb; // [esp+0h] [ebp-50h]
  GLfloat xc; // [esp+0h] [ebp-50h]
  GLfloat y; // [esp+4h] [ebp-4Ch]
  GLfloat ya; // [esp+4h] [ebp-4Ch]
  GLfloat yb; // [esp+4h] [ebp-4Ch]
  GLfloat yc; // [esp+4h] [ebp-4Ch]
  GLfloat z; // [esp+8h] [ebp-48h]
  GLfloat za; // [esp+8h] [ebp-48h]
  GLfloat zb; // [esp+8h] [ebp-48h]
  GLfloat zc; // [esp+8h] [ebp-48h]
  GLfloat t; // [esp+1Ch] [ebp-34h]
  float v24; // [esp+20h] [ebp-30h] BYREF
  float v25; // [esp+24h] [ebp-2Ch]
  float v26; // [esp+28h] [ebp-28h]
  float v27; // [esp+2Ch] [ebp-24h] BYREF
  float v28; // [esp+30h] [ebp-20h]
  float v29; // [esp+34h] [ebp-1Ch]
  float v30; // [esp+38h] [ebp-18h]
  float v31; // [esp+3Ch] [ebp-14h]
  float v32; // [esp+40h] [ebp-10h]
  float v33; // [esp+44h] [ebp-Ch]
  float v34; // [esp+48h] [ebp-8h]
  float v35; // [esp+4Ch] [ebp-4h]
  float v36; // [esp+54h] [ebp+4h]
  GLfloat v37; // [esp+58h] [ebp+8h]
  GLfloat v38; // [esp+58h] [ebp+8h]

  glColor3f(1.0, 1.0, 1.0);
  v27 = *(float *)LODWORD(a3) - *(float *)a2;
  v28 = *(float *)(LODWORD(a3) + 4) - *(float *)(a2 + 4);
  v29 = *(float *)(LODWORD(a3) + 8) - *(float *)(a2 + 8);
  v6 = Vec3_Length(&v27);
  v7 = *(float *)(a2 + 4);
  v8 = *(float *)(a2 + 8);
  v30 = *(float *)LODWORD(a3);
  v34 = v7;
  v31 = *(float *)(LODWORD(a3) + 4);
  v35 = v8;
  v9 = *(float *)(LODWORD(a3) + 8);
  v37 = (50.0 - v6) * 0.0099999998;
  v10 = *(float *)a2;
  v28 = v31 - v7;
  v29 = v9 - v35;
  v27 = (v30 - v10) * 0.1;
  v28 = v28 * 0.1;
  v29 = v29 * 0.1;
  v33 = v10 - v27;
  v34 = v7 - v28;
  v35 = v35 - v29;
  v30 = v30 + v27;
  v31 = v31 + v28;
  v32 = v9 + v29;
  v36 = (double)(rand() % 100) * 0.0099999998;
  glColor3f(1.0, 1.0, 1.0);
  BindTexture(494);
  EnableAlphaBlendMinus();
  Vec3_Cross(_this + 12, &v27, &v24);
  Vec3_Normalize(&v24);
  v24 = v24 * 10.0;
  v25 = v25 * 10.0;
  v26 = v26 * 10.0;
  glBegin(7u);
  t = v36 + v37;
  glTexCoord2f(0.0, t);
  z = v35 - v26;
  y = v34 - v25;
  x = v33 - v24;
  glVertex3f(x, y, z);
  v38 = 1.0 - v37 + v36;
  glTexCoord2f(0.0, v38);
  za = v32 - v26;
  ya = v31 - v25;
  xa = v30 - v24;
  glVertex3f(xa, ya, za);
  glTexCoord2f(1.0, v38);
  zb = v26 + v32;
  yb = v25 + v31;
  xb = v24 + v30;
  glVertex3f(xb, yb, zb);
  glTexCoord2f(1.0, t);
  zc = v26 + v35;
  yc = v25 + v34;
  xc = v24 + v33;
  glVertex3f(xc, yc, zc);
  glEnd();
}
#endif
