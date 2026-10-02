// 0x004FFCC0 Entity_GridUnlink — nunca activado: IDA_PORT_004FFCC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Entity_GridUnlink (IDA-only, gated) ──
#if defined(IDA_PORT_004FFCC0)
void __cdecl Entity_GridUnlink(DWORD *lpMem, int a2)
{
  int v2; // eax
  int v3; // ecx

  if ( lpMem )
  {
    v2 = lpMem[110];
    v3 = lpMem[109];
    if ( v2 )
    {
      if ( v3 )
      {
        *(DWORD *)(v3 + 440) = v2;
        *(DWORD *)(v2 + 436) = lpMem[109];
      }
      else
      {
        *(DWORD *)(v2 + 436) = 0;
        *(DWORD *)(a2 + 4) = v2;
      }
    }
    else if ( v3 )
    {
      *(DWORD *)(v3 + 440) = 0;
      *(DWORD *)(a2 + 8) = v3;
    }
    else
    {
      *(DWORD *)(a2 + 4) = 0;
      *(DWORD *)(a2 + 8) = 0;
    }
    delete__(lpMem);
  }
}
#endif
