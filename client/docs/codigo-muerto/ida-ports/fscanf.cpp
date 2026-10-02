// 0x0054337B fscanf — nunca activado: IDA_PORT_0054337B nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── fscanf (IDA-only, gated) ──
#if defined(IDA_PORT_0054337B)
int fscanf(FILE *const Stream, const char *const Format, ...)
{
  int v2; // esi
  va_list va; // [esp+14h] [ebp+10h] BYREF

  va_start(va, Format);
  _lock_file(Stream);
  v2 = _input(Stream, (int)Format, (int)va);
  _unlock_file(Stream);
  return v2;
}
#endif
