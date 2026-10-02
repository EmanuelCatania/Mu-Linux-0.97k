// 0x00442E00 BMD__Init — nunca activado: IDA_PORT_00442E00 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── BMD__Init (IDA-only, gated) ──
#if defined(IDA_PORT_00442E00)
void __cdecl BMD::Init(DWORD This, bool Dummy)
{
  int v2; // esi
  int v3; // edx
  BYTE *v4; // eax

  if ( Dummy )
  {
    v2 = 0;
    if ( *(short *)(This + 34) > 0 )
    {
      v3 = 0;
      do
      {
        v4 = (BYTE *)(v3 + *(DWORD *)(This + 44));
        v4[34] = *v4 == 68 && v4[1] == 117;
        ++v2;
        v3 += 140;
      }
      while ( v2 < *(short *)(This + 34) );
    }
  }
  *(DWORD *)(This + 84) = -1;
  *(BYTE *)(This + 136) = -1;
  BMD::CreateBoundingBox(This);
}
#endif
