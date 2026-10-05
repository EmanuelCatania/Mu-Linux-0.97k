// 0x004088B0 Spring_StoreEdge — nunca activado: IDA_PORT_004088B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Spring_StoreEdge (IDA-only, gated) ──
#if defined(IDA_PORT_004088B0)
int __cdecl Spring_StoreEdge(DWORD *_this, int a2, short a3, short a4, float a5, float a6, char a7)
{
  int result; // eax

  result = 16 * a2;
  *(WORD *)(_this[15] + result) = a3;
  *(WORD *)(_this[15] + result + 2) = a4;
  *(float *)(_this[15] + result + 4) = a5;
  *(float *)(_this[15] + result + 8) = a6;
  *(BYTE *)(_this[15] + result + 12) = a7;
  return result;
}
#endif
