// 0x0040F730 FUN_0040f730 — nunca activado: IDA_PORT_0040F730 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040f730 (IDA-only, gated) ──
#if defined(IDA_PORT_0040F730)
int __cdecl FUN_0040f730(int _this)
{
  DWORD *v2; // ebp
  DWORD *v3; // eax
  DWORD *v4; // eax
  char v6; // [esp+13h] [ebp-19h]
  void *lpMem; // [esp+14h] [ebp-18h]
  char v8[4]; // [esp+18h] [ebp-14h] BYREF
  int v9; // [esp+1Ch] [ebp-10h]
  int v10; // [esp+28h] [ebp-4h]

  v9 = _this;
  FUN_0040f680((DWORD *)_this);
  v10 = 0;
  *(BYTE *)(_this + 4) = v6;
  *(BYTE *)(_this + 5) = v6;
  *(BYTE *)(_this + 12) = 1;
  v2 = (DWORD *)operator_new(0x138u);
  lpMem = v2;
  v2[1] = 0;
  v2[77] = 1;
  std::_Lockit::_Lockit((std::_Lockit *)v8);
  if ( !DAT_055c9b98 )
  {
    DAT_055c9b98 = (int)v2;
    *v2 = 0;
    lpMem = 0;
    *(DWORD *)(DAT_055c9b98 + 8) = 0;
  }
  ++DAT_055c9b94;
  std::_Lockit::~_Lockit((std::_Lockit *)v8);
  if ( lpMem )
  {
    delete__(lpMem);
  }
  v3 = (DWORD *)FUN_00411840(DAT_055c9b98, 0);
  *(DWORD *)(_this + 8) = v3;
  *(DWORD *)(_this + 16) = 0;
  *v3 = v3;
  *(DWORD *)(*(DWORD *)(_this + 8) + 8) = *(DWORD *)(_this + 8);
  (BYTE)(v10) = 1;
  *(BYTE *)(_this + 24) = v6;
  v4 = (DWORD *)operator_new(0xCu);
  *v4 = v4;
  v4[1] = v4;
  *(DWORD *)(_this + 28) = v4;
  *(DWORD *)(_this + 32) = 0;
  *(DWORD *)_this = &DAT_005527f8;
  *(DWORD *)(_this + 204) = 0;
  return _this;
}
#endif
