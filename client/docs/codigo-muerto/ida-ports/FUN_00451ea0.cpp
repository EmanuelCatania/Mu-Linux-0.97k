// 0x00451EA0 FUN_00451ea0 — nunca activado: IDA_PORT_00451EA0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00451ea0 (IDA-only, gated) ──
#if defined(IDA_PORT_00451EA0)
void __cdecl FUN_00451ea0(int a1, DWORD This, int a3, int a4)
{
  float Position[3]; // [esp+8h] [ebp-Ch] BYREF

  memset(Position, 0, sizeof(Position));
  TransformPosition(This, (float (*)[4])(*(DWORD *)(a1 + 276) + 48 * a3), Position, (float *)(a1 + 76), 1);
  memset(Position, 0, sizeof(Position));
  TransformPosition(This, (float (*)[4])(*(DWORD *)(a1 + 276) + 48 * a4), Position, (float *)(a1 + 64), 1);
}
#endif
