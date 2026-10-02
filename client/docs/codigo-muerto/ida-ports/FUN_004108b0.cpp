// 0x004108B0 FUN_004108b0 — nunca activado: IDA_PORT_004108B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004108b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004108B0)
void __stdcall FUN_004108b0(int a1, int a2, float Width, float Height)
{
  int v4; // eax
  float x; // [esp+0h] [ebp-28h]
  float y; // [esp+4h] [ebp-24h]
  float uWidth; // [esp+18h] [ebp-10h]
  float vHeight; // [esp+1Ch] [ebp-Ch]
  float Widtha; // [esp+34h] [ebp+Ch]
  float Heighta; // [esp+38h] [ebp+10h]

  v4 = a1;
  if ( a1 < 0 )
  {
    v4 = 0;
    a1 = 0;
  }
  if ( v4 + LODWORD(Width) > (int)WindowWidth )
  {
    a1 = WindowWidth - LODWORD(Width);
  }
  Heighta = (float)SLODWORD(Height);
  Widtha = (float)SLODWORD(Width);
  vHeight = (Heighta + 0.0099999998) / Bitmaps[0].Height;
  uWidth = (Widtha + 0.0099999998) / Bitmaps[0].Width;
  y = (float)a2;
  x = (float)a1;
  RenderBitmap(-DAT_055c9b90, x, y, Widtha, Heighta, 0.0, 0.0, uWidth, vHeight, 0, 0);
}
#endif
