// 0x00407B30 SpringNode_GetPos — nunca activado: IDA_PORT_00407B30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SpringNode_GetPos (IDA-only, gated) ──
#if defined(IDA_PORT_00407B30)
int *__cdecl SpringNode_GetPos(int *_this, DWORD *a2)
{
  int *result; // eax
  int v4; // ecx
  int v5; // esi

  result = _this + 7;
  v4 = 3;
  do
  {
    v5 = *result++;
    *a2++ = v5;
    --v4;
  }
  while ( v4 );
  return result;
}
#endif
