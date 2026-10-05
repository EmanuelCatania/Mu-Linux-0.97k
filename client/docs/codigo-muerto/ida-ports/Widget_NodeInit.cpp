// 0x004080F0 Widget_NodeInit — nunca activado: IDA_PORT_004080F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Widget_NodeInit (IDA-only, gated) ──
#if defined(IDA_PORT_004080F0)
int __cdecl Widget_NodeInit(DWORD *_this)
{
  int result; // eax

  result = 0;
  _this[1] = 0;
  _this[2] = 0;
  _this[4] = 0;
  _this[3] = 0;
  _this[5] = 0;
  _this[7] = 0;
  _this[6] = 0;
  _this[9] = 1065353216;
  _this[8] = 1065353216;
  _this[17] = 1065353216;
  _this[16] = 1065353216;
  _this[10] = 0;
  _this[11] = 0;
  _this[12] = 0;
  _this[13] = 0;
  _this[14] = 0;
  _this[15] = 0;
  return result;
}
#endif
