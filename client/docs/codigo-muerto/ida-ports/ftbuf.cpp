// 0x00546ADD ftbuf — nunca activado: IDA_PORT_00546ADD nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ftbuf (IDA-only, gated) ──
#if defined(IDA_PORT_00546ADD)
int __cdecl _ftbuf(int a1, int a2)
{
  int result; // eax

  if ( a1 )
  {
    if ( (*(BYTE *)(a2 + 13) & 0x10) != 0 )
    {
      result = _flush((DWORD *)a2);
      *(BYTE *)(a2 + 13) &= 0xEEu;
      *(DWORD *)(a2 + 24) = 0;
      *(DWORD *)a2 = 0;
      *(DWORD *)(a2 + 8) = 0;
    }
  }
  return result;
}
#endif
