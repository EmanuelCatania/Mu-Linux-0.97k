// 0x00407E10 VerletNode_SetTarget — nunca activado: IDA_PORT_00407E10 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── VerletNode_SetTarget (IDA-only, gated) ──
#if defined(IDA_PORT_00407E10)
int __cdecl VerletNode_SetTarget(DWORD *_this, int a2, int a3, int a4)
{
  int result; // eax

  _this[5] = a2;
  result = a4;
  _this[6] = a3;
  _this[7] = a4;
  return result;
}
#endif
