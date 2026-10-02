// 0x004079B0 SpringNode_SetPos — nunca activado: IDA_PORT_004079B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SpringNode_SetPos (IDA-only, gated) ──
#if defined(IDA_PORT_004079B0)
int __cdecl SpringNode_SetPos(int _this, int a2, int a3, int a4, int a5)
{
  int result; // eax

  *(DWORD *)(_this + 28) = a2;
  *(DWORD *)(_this + 36) = a4;
  result = a5;
  *(DWORD *)(_this + 32) = a3;
  if ( a5 )
  {
    *(BYTE *)(_this + 40) |= 1u;
  }
  return result;
}
#endif
