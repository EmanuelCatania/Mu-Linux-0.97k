// 0x0045C720 Character_UpdateEquipSlotAnimations — nunca activado: IDA_PORT_0045C720 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Character_UpdateEquipSlotAnimations (IDA-only, gated) ──
#if defined(IDA_PORT_0045C720)
void __cdecl Character_UpdateEquipSlotAnimations(DWORD c)
{
  unsigned char v1; // al
  char v2; // cl
  int v3; // eax
  int v4; // ecx

  if ( *(WORD *)(c + 2) == 390 )
  {
    v1 = *(BYTE *)(c + 261);
    v2 = 1;
    if ( v1 >= 0x85u && v1 <= 0x8Cu )
    {
      v2 = 0;
    }
    if ( (v1 < 0x22u || v1 > 0x5Bu) && v2 )
    {
      SetPlayerStop(c);
    }
    v3 = *(BYTE *)(c + 444) & 7;
    v4 = *(unsigned char *)(c + 444) >> 3;
    if ( *(short *)(c + 504) == v3 + 912 )
    {
      *(BYTE *)(c + 506) = 0;
      *(WORD *)(c + 504) = v3 + 4 * v4 + 912;
      *(BYTE *)(c + 507) = 0;
    }
    if ( *(short *)(c + 552) == v3 + 919 )
    {
      *(BYTE *)(c + 530) = 0;
      *(WORD *)(c + 528) = v3 + 4 * v4 + 919;
      *(BYTE *)(c + 531) = 0;
    }
    if ( *(short *)(c + 576) == v3 + 926 )
    {
      *(BYTE *)(c + 554) = 0;
      *(WORD *)(c + 552) = v3 + 4 * v4 + 926;
      *(BYTE *)(c + 555) = 0;
    }
    if ( *(short *)(c + 600) == v3 + 933 )
    {
      *(BYTE *)(c + 578) = 0;
      *(WORD *)(c + 576) = v3 + 4 * v4 + 933;
      *(BYTE *)(c + 579) = 0;
    }
    if ( *(short *)(c + 624) == v3 + 940 )
    {
      *(BYTE *)(c + 602) = 0;
      *(WORD *)(c + 600) = v3 + 4 * v4 + 940;
      *(BYTE *)(c + 603) = 0;
    }
    SetCharacterScale(c);
  }
}
#endif
