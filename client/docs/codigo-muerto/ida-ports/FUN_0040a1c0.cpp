// 0x0040A1C0 FUN_0040a1c0 — nunca activado: IDA_PORT_0040A1C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040a1c0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040A1C0)
int __cdecl FUN_0040a1c0(int _this, short a2, int a3, short a4, int a5, int a6)
{
  short *v7; // ebp
  int v8; // esi
  float *v9; // ecx
  float *v10; // edx
  int result; // eax
  int v12; // ebp
  int v13; // esi
  int v14; // [esp+10h] [ebp-1Ch]
  float Normal[3]; // [esp+14h] [ebp-18h] BYREF
  float *__attribute__((__org_arrdim(0,3))) v3; // [esp+28h] [ebp-4h]

  if ( a4 > 0 )
  {
    v14 = a4;
    v7 = (short *)(a5 + 6);
    v8 = 15000 * a2;
    do
    {
      v9 = (float *)(a3 + 12 * (v8 + *(v7 - 2)));
      v10 = (float *)(a3 + 12 * (v8 + *(v7 - 1)));
      v3 = (float *)(a3 + 12 * (v8 + *v7));
      FaceNormalize(v9, v10, v3, Normal);
      *((BYTE *)v7 + 28) = Normal[2] * *(float *)(_this + 20)
                          + Normal[1] * *(float *)(_this + 16)
                          + Normal[0] * *(float *)(_this + 12) <= 0.0;
      v7 += 18;
      --v14;
    }
    while ( v14 );
  }
  result = a4;
  v12 = 0;
  if ( a4 > 0 )
  {
    v13 = a5 + 6;
    do
    {
      if ( *(BYTE *)(v13 + 28) )
      {
        FUN_0040a110((DWORD *)_this, *(WORD *)(v13 - 4), *(WORD *)(v13 - 2), a2, v12, 0, a5);
        FUN_0040a110((DWORD *)_this, *(WORD *)(v13 - 2), *(WORD *)v13, a2, v12, 1, a5);
        FUN_0040a110((DWORD *)_this, *(WORD *)v13, *(WORD *)(v13 - 4), a2, v12, 2, a5);
      }
      result = a4;
      ++v12;
      v13 += 36;
    }
    while ( v12 < a4 );
  }
  return result;
}
#endif
