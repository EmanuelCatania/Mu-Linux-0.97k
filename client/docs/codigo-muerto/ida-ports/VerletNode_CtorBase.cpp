// 0x00407DA0 VerletNode_CtorBase — nunca activado: IDA_PORT_00407DA0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── VerletNode_CtorBase (IDA-only, gated) ──
#if defined(IDA_PORT_00407DA0)
DWORD *__cdecl VerletNode_CtorBase(DWORD *_this)
{
  *_this = (DWORD)&DAT_00552508;
  VerletNode_ZeroFields((int)_this);
  return _this;
}
#endif
