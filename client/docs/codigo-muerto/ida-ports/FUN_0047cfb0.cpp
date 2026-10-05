// 0x0047CFB0 FUN_0047cfb0 — nunca activado: IDA_PORT_0047CFB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0047cfb0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047CFB0)
short __cdecl FUN_0047cfb0(WORD *Value)
{
  WORD *v1; // ecx
  DWORD v3; // [esp-4h] [ebp-4h]

  if ( *Value == 0xFFFF )
  {
    return 0;
  }
  (WORD)(v1) = Value[9];
  v3 = (DWORD)Value;
  Value = v1;
  PlusSpecial((WORD *)&Value, 63, v3);
  return (short)Value;
}
#endif
