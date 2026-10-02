// 0x0043DAF0 NetContext_ClearSendBuffer — nunca activado: IDA_PORT_0043DAF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── NetContext_ClearSendBuffer (IDA-only, gated) ──
#if defined(IDA_PORT_0043DAF0)
DWORD *__cdecl NetContext_ClearSendBuffer(DWORD *_this)
{
  memset(_this + 4103, 0, 0x258960u);
  _this[4101] = 0;
  _this[4102] = 0;
  return _this;
}
#endif
