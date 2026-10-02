// 0x00543A8C crt_sscanf — nunca activado: IDA_PORT_00543A8C nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_sscanf (IDA-only, gated) ──
#if defined(IDA_PORT_00543A8C)
int crt_sscanf(const char *const Buffer, const char *const Format, ...)
{
  FILE Stream; // [esp+0h] [ebp-20h] BYREF
  va_list va; // [esp+30h] [ebp+10h] BYREF

  va_start(va, Format);
  Stream._flag = 73;
  Stream._base = (char *)Buffer;
  Stream._ptr = (char *)Buffer;
  Stream._cnt = strlen(Buffer);
  return _input(&Stream, (int)Format, (int)va);
}
#endif
