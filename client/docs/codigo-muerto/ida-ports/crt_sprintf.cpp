// 0x005416BC crt_sprintf — nunca activado: IDA_PORT_005416BC nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_sprintf (IDA-only, gated) ──
#if defined(IDA_PORT_005416BC)
int crt_sprintf(char *const Buffer, const char *const Format, ...)
{
  int v2; // eax
  bool v3; // sf
  int v4; // esi
  FILE File; // [esp+4h] [ebp-20h] BYREF
  va_list va; // [esp+34h] [ebp+10h] BYREF

  va_start(va, Format);
  File._base = Buffer;
  File._ptr = Buffer;
  File._flag = 66;
  File._cnt = 0x7FFFFFFF;
  v2 = _output(&File, (int)Format, (int)va);
  v3 = --File._cnt < 0;
  v4 = v2;
  if ( v3 )
  {
    _flsbuf(0, &File);
  }
  else
  {
    *File._ptr = 0;
  }
  return v4;
}
#endif
