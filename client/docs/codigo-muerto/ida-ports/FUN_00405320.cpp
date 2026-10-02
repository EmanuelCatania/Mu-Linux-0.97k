// 0x00405320 FUN_00405320 — nunca activado: IDA_PORT_00405320 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00405320 (IDA-only, gated) ──
#if defined(IDA_PORT_00405320)
DWORD __cdecl FUN_004052b0(int _this, const char *a2)
{
  strcpy((char *)(_this + 8), a2);
  *(DWORD *)(_this + 268) = 0;
  *(DWORD *)(_this + 4) = CreateFileA((LPCSTR)(_this + 8), 0xC0000000, 1u, 0, 4u, 0x80u, 0);
  FUN_00405340(_this);
  return SetFilePointer(*(HANDLE *)(_this + 4), 0, 0, 2u);
}
#endif
