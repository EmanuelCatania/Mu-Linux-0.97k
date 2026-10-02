// 0x00407AC0 VerletNode_AddAccel — nunca activado: IDA_PORT_00407AC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── VerletNode_AddAccel (IDA-only, gated) ──
#if defined(IDA_PORT_00407AC0)
void __cdecl VerletNode_AddAccel(float *_this, float a2, float a3, float a4)
{
  _this[1] = a2 + _this[1];
  _this[2] = a3 + _this[2];
  _this[3] = a4 + _this[3];
}
#endif
