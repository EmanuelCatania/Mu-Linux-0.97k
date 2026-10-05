// 0x0046B980 FUN_0046b980 — nunca activado: IDA_PORT_0046B980 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0046b980 (IDA-only, gated) ──
#if defined(IDA_PORT_0046B980)
void __cdecl FUN_0046b980(DWORD o)
{
  int v2; // edi
  DWORD v3; // ebx
  char v4; // al
  char v5; // cl
  int v6; // edx
  int v7; // eax
  int v8; // ecx
  DWORD v9; // eax
  float v10; // eax
  double v11; // st7
  float v12; // eax
  float v13; // ecx
  double v14; // st7
  float v15; // eax
  float v16; // [esp-20h] [ebp-44h]
  float alpha; // [esp+4h] [ebp-20h]
  float v19; // [esp+8h] [ebp-1Ch]
  float Light[3]; // [esp+Ch] [ebp-18h] BYREF
  float Angle[3]; // [esp+18h] [ebp-Ch] BYREF
  DWORD oa; // [esp+28h] [ebp+4h]

  if ( (double)*(int *)(o + 96) > 10.0 )
  {
    v2 = *(unsigned char *)(*(DWORD *)(o + 252) + 136) + 400;
    alpha = *(float *)(o + 360);
    v3 = Models + 188 * v2;
    v4 = *(BYTE *)(Hero + 444);
    v5 = *(BYTE *)(o + 261);
    *(DWORD *)(v3 + 108) = *(DWORD *)(o + 16);
    v6 = *(short *)(o + 2);
    *(BYTE *)(v3 + 152) = v4 & 7;
    v7 = *(DWORD *)(o + 20);
    *(BYTE *)(v3 + 160) = v5;
    v8 = *(DWORD *)(o + 24);
    *(DWORD *)(v3 + 112) = v7;
    v9 = *(DWORD *)(o + 216);
    *(DWORD *)(v3 + 116) = v8;
    v19 = (float)v6;
    oa = v9;
    *(WORD *)(o + 2) = v2;
    ItemObjectAttribute(o);
    v10 = *(float *)(o + 20);
    *(DWORD *)(o + 216) = oa;
    RequestTerrainLight(*(float *)(o + 16), v10, Light);
    v11 = Light[0] + *(float *)(o + 232);
    v12 = *(float *)(o + 32);
    v13 = *(float *)(o + 36);
    Angle[0] = *(float *)(o + 28);
    Angle[1] = v12;
    Light[0] = v11;
    v14 = Light[1] + *(float *)(o + 236);
    Angle[2] = v13;
    (BYTE)(v13) = *(BYTE *)(o + 262);
    Light[1] = v14;
    v15 = *(float *)(o + 264);
    v16 = *(float *)(o + 268);
    Light[2] = Light[2] + *(float *)(o + 240);
    BMD_Animation(v3, (float (*)[3][4])BoneMatrix, v15, v16, (BYTE)(v13), Angle, (float *)(o + 40), 0, 0);
    RenderPartObject(o, v2, 0, Light, alpha, 8 * *(unsigned char *)(*(DWORD *)(o + 252) + 137), 0, 1, 1, 1, 0, 2);
    *(WORD *)(o + 2) = (__int64)v19;
  }
}
#endif
