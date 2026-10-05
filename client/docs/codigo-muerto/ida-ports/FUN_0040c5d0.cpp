// 0x0040C5D0 FUN_0040c5d0 — nunca activado: IDA_PORT_0040C5D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c5d0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C5D0)
int __cdecl FUN_0040c5d0(int _this)
{
  DWORD *v2; // eax
  char v4; // [esp+Bh] [ebp-11h]

  *(BYTE *)(_this + 4) = v4;
  v2 = (DWORD *)operator_new(0x14u);
  *v2 = v2;
  v2[1] = v2;
  *(DWORD *)(_this + 8) = v2;
  *(DWORD *)(_this + 12) = 0;
  *(DWORD *)_this = &DAT_005525a0;
  *(DWORD *)(_this + 28) = FUN_0040c480();
  *(DWORD *)(_this + 32) = 0;
  FUN_0040c670(0);
  *(DWORD *)(_this + 40) = 0;
  Object_SetRectFields((DWORD *)_this, 0, 0);
  FUN_0040c6b0(100, 100);
  FUN_0040c6d0(0, 0, 0);
  FUN_0040c6f0(0, 0, 0);
  *(DWORD *)(_this + 84) = 1;
  return _this;
}
#endif
