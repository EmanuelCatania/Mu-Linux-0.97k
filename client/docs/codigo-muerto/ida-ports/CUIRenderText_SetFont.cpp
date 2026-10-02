// 0x0040F650 CUIRenderText_SetFont — nunca activado: IDA_PORT_0040F650 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CUIRenderText_SetFont (IDA-only, gated) ──
#if defined(IDA_PORT_0040F650)
int __cdecl CUIRenderText::SetFont(DWORD **_this, int a2)
{
  int result; // eax

  result = a2;
  if ( a2 )
  {
    return (*(int (__cdecl **)(DWORD *, int))(*_this[1] + 8))(_this[1], a2);
  }
  return result;
}
#endif
