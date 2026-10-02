// 0x0053CE30 FUN_0053ce30 — nunca activado: IDA_PORT_0053CE30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053ce30 (IDA-only, gated) ──
#if defined(IDA_PORT_0053CE30)
int __cdecl FUN_0053ce30(DWORD *_this, DWORD *a2, int a3)
{
  int v4; // edi
  int *v5; // esi
  int v6; // edi
  int *v7; // ecx
  int v8; // edx
  DWORD *v9; // esi
  int v10; // ebx
  int v11; // eax
  int v12; // eax
  int v13; // eax
  short v14; // bx
  char *v15; // esi
  int v16; // eax
  short v17; // dx
  WORD *v18; // eax
  short v19; // dx
  bool v20; // zf
  char v21; // cl
  int i; // eax
  int v24; // [esp+10h] [ebp-20h] BYREF
  int v25; // [esp+14h] [ebp-1Ch]
  WORD *v26; // [esp+18h] [ebp-18h]
  int v27; // [esp+1Ch] [ebp-14h]
  int v28[2]; // [esp+20h] [ebp-10h] BYREF
  int v29; // [esp+28h] [ebp-8h] BYREF
  int v30; // [esp+2Ch] [ebp-4h]

  v26 = a2;
  v28[0] = 0;
  v28[1] = 0;
  *a2 = 0;
  v29 = 0;
  a2[1] = 0;
  v30 = 0;
  v4 = 0;
  v5 = v28;
  v25 = 4;
  do
  {
    CSimpleModulus_AddBits(v5, 0, a3, v4, 16);
    v6 = v4 + 16;
    CSimpleModulus_AddBits(v5, 22, a3, v6, 2);
    v4 = v6 + 2;
    ++v5;
    --v25;
  }
  while ( v25 );
  v7 = &v29;
  v8 = (unsigned short)v30;
  v9 = _this + 15;
  v10 = 3;
  do
  {
    v11 = *v7--;
    v12 = *v9-- ^ v11;
    v13 = v8 ^ v12;
    v7[1] = v13;
    --v10;
    v8 = (unsigned short)v13;
  }
  while ( v10 );
  v14 = 0;
  v25 = (int)v28;
  v15 = (char *)(_this + 1);
  v27 = 4;
  do
  {
    v16 = *((DWORD *)v15 + 8);
    v15 += 4;
    v17 = *((DWORD *)v15 + 11) ^ ((unsigned int)(*(DWORD *)v25 * v16) % *((DWORD *)v15 - 1));
    v18 = v26;
    v19 = v14 ^ v17;
    v14 = *(DWORD *)v25;
    *v26 = v19;
    v26 = v18 + 1;
    v20 = v27 == 1;
    v25 += 4;
    --v27;
  }
  while ( !v20 );
  (WORD)(v24) = 0;
  CSimpleModulus_AddBits(&v24, 0, a3, v4, 16);
  v21 = -8;
  (BYTE)(v24) = v24 ^ ((BYTE)((v24) >> 8)) ^ 0x3D;
  for ( i = 0; i < 8; ++i )
  {
    v21 ^= *((BYTE *)a2 + i);
  }
  if ( ((BYTE)((v24) >> 8)) == v21 )
  {
    return (unsigned char)v24;
  }
  else
  {
    return -1;
  }
}
#endif
