// 0x0040DCE0 FUN_0040dce0 — nunca activado: IDA_PORT_0040DCE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040dce0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040DCE0)
void __stdcall FUN_0040dce0(int Texture, int a2, float x, float y, float Width, float a6, GLfloat alpha, float a8)
{
  int v8; // eax
  float v9; // [esp+0h] [ebp-30h]
  float v10; // [esp+4h] [ebp-2Ch]
  float Heighta; // [esp+Ch] [ebp-24h]
  float Height; // [esp+Ch] [ebp-24h]
  float v13; // [esp+10h] [ebp-20h]
  int v14; // [esp+14h] [ebp-1Ch]
  float v15; // [esp+14h] [ebp-1Ch]
  float uWidth; // [esp+18h] [ebp-18h]
  float uWidtha; // [esp+18h] [ebp-18h]
  float vHeight; // [esp+1Ch] [ebp-14h]
  float vHeighta; // [esp+1Ch] [ebp-14h]
  float v20; // [esp+50h] [ebp+20h]

  if ( a2 == 1 )
  {
    if ( MouseLButton )
    {
      glColor4f(0.80000001, 0.80000001, 0.80000001, alpha);
      if ( a8 == 0.0 )
      {
        v14 = 0;
        Height = a6;
      }
      else
      {
        v14 = 1031798784;
        Height = a6 * -1.0;
      }
      v10 = y + 1.0;
      v9 = x + 1.0;
      uWidtha = (Width - 1.0) * 0.0625;
      vHeighta = (a6 - 1.0) * 0.0625;
      RenderBitmap(Texture, v9, v10, Width, Height, 0.0, *(float *)&v14, uWidtha, vHeighta, 1, 1);
      glColor4f(1.0, 1.0, 1.0, alpha);
    }
    else
    {
      v20 = (double)(LODWORD(a8) != 0 ? -1 : 1) * a6;
      v15 = a6 * 0.0625;
      v13 = Width * 0.0625;
      RenderBitmap(Texture, x, y, Width, v20, 0.0, 0.0, v13, v15, 1, 1);
      glColor4f(1.0, 1.0, 1.0, 0.1);
      RenderColor(x, y, Width, v20);
      glEnable(0xDE1u);
      glColor4f(1.0, 1.0, 1.0, alpha);
    }
  }
  else
  {
    vHeight = a6 * 0.0625;
    v8 = -(LODWORD(a8) != 0);
    (BYTE)(v8) = v8 & 0xFE;
    uWidth = Width * 0.0625;
    Heighta = (double)(v8 + 1) * a6;
    RenderBitmap(Texture, x, y, Width, Heighta, 0.0, 0.0, uWidth, vHeight, 1, 1);
  }
}
#endif
