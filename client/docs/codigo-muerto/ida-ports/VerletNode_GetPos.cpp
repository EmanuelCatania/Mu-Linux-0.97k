// 0x00407E30 VerletNode_GetPos — nunca activado: IDA_PORT_00407E30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── VerletNode_GetPos (IDA-only, gated) ──
#if defined(IDA_PORT_00407E30)
DWORD *__cdecl VerletNode_GetPos(DWORD *_this, DWORD *a2)
{
  DWORD *result; // eax
  DWORD *v3; // ecx

  result = a2;
  v3 = _this + 1;
  *a2 = *v3;
  a2[1] = v3[1];
  a2[2] = v3[2];
  return result;
}
#endif
