// 0x0040EAE0 FUN_0040eae0 — nunca activado: IDA_PORT_0040EAE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040eae0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040EAE0)
int __cdecl FUN_0040eae0(char *_this)
{
  DWORD **v2; // ebx
  char *v3; // esi
  DWORD *v4; // edi
  void *v5; // eax
  int *v6; // ebx
  LPVOID *v7; // eax
  int *v8; // edi
  LPVOID *v9; // eax
  LPVOID *v11; // [esp-4h] [ebp-30h]
  int v12[2]; // [esp+10h] [ebp-1Ch] BYREF
  char v13[4]; // [esp+18h] [ebp-14h] BYREF
  char v14[4]; // [esp+1Ch] [ebp-10h] BYREF
  int v15; // [esp+28h] [ebp-4h]

  v12[1] = (int)_this;
  *(DWORD *)_this = &DAT_00552760;
  v2 = (DWORD **)*((DWORD *)_this + 23);
  v3 = _this + 88;
  v15 = 2;
  v4 = *v2;
  while ( v4 != v2 )
  {
    v5 = v4;
    v4 = (DWORD *)*v4;
    FUN_00411360(v12, v5);
  }
  *((DWORD *)_this + 34) = 0;
  (BYTE)(v15) = 1;
  v6 = (int *)*((DWORD *)_this + 31);
  v12[0] = *v6;
  while ( (int *)v12[0] != v6 )
  {
    v7 = (LPVOID *)FUN_00410e30(v13, 0);
    FUN_00411360(v14, *v7);
  }
  delete__(*((LPVOID *)_this + 31));
  *((DWORD *)_this + 31) = 0;
  *((DWORD *)_this + 32) = 0;
  v8 = (int *)*((DWORD *)v3 + 1);
  (BYTE)(v15) = 0;
  v12[0] = *v8;
  while ( (int *)v12[0] != v8 )
  {
    v9 = (LPVOID *)FUN_00410e30(v14, 0);
    FUN_00411360(v13, *v9);
  }
  delete__(*((LPVOID *)v3 + 1));
  *((DWORD *)v3 + 1) = 0;
  *((DWORD *)v3 + 2) = 0;
  *(DWORD *)_this = DAT_005525c8;
  v11 = (LPVOID *)*((DWORD *)_this + 2);
  v15 = 3;
  FUN_00410de0(v14, *v11, v11);
  v15 = -1;
  return FUN_00410d90(_this + 4);
}
#endif
