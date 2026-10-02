// 0x00444B30 SetPlayerTeleport — nunca activado: IDA_PORT_00444B30 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SetPlayerTeleport (IDA-only, gated) ──
#if defined(IDA_PORT_00444B30)
void __cdecl SetPlayerTeleport(DWORD o)
{
  if ( *(WORD *)(o + 2) == 390 )
  {
    SetAction(o, 87);
  }
  else
  {
    SetAction(o, 5);
  }
}
#endif
