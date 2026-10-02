// 0x00546A50 stbuf — nunca activado: IDA_PORT_00546A50 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── stbuf (IDA-only, gated) ──
#if defined(IDA_PORT_00546A50)
int __cdecl _stbuf(int a1)
{
  int v1; // eax
  int *v2; // edi
  void *v3; // eax
  int v4; // edi

  if ( !_isatty(*(DWORD *)(a1 + 16)) )
  {
    return 0;
  }
  if ( (FILE *)a1 == &File )
  {
    v1 = 0;
  }
  else
  {
    if ( (FILE *)a1 != &DAT_00563e00 )
    {
      return 0;
    }
    v1 = 1;
  }
  ++DAT_083bbbfc;
  if ( (*(WORD *)(a1 + 12) & 0x10C) != 0 )
  {
    return 0;
  }
  v2 = (int *)(4 * v1 + 138132512);
  if ( DAT_083bbc20[v1] || (v3 = malloc(0x1000u), (*v2 = (int)v3) != 0) )
  {
    v4 = *v2;
    *(DWORD *)(a1 + 24) = 4096;
    *(DWORD *)(a1 + 8) = v4;
    *(DWORD *)a1 = v4;
    *(DWORD *)(a1 + 4) = 4096;
  }
  else
  {
    *(DWORD *)(a1 + 8) = a1 + 20;
    *(DWORD *)a1 = a1 + 20;
    *(DWORD *)(a1 + 24) = 2;
    *(DWORD *)(a1 + 4) = 2;
  }
  *(WORD *)(a1 + 12) |= 0x1102u;
  return 1;
}
#endif
