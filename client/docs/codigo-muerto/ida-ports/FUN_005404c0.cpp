// 0x005404C0 FUN_005404c0 — nunca activado: IDA_PORT_005404C0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005404c0 (IDA-only, gated) ──
#if defined(IDA_PORT_005404C0)
void *__cdecl FUN_005404c0(void *_this)
{
  void *result; // eax
  struct _SYSTEMTIME SystemTime; // [esp+4h] [ebp-10h] BYREF

  GetLocalTime(&SystemTime);
  result = _this;
  DAT_083bbb74 = (unsigned int)&DAT_00dad53a ^ (SystemTime.wYear * SystemTime.wMonth * SystemTime.wDay);
  return result;
}
#endif
