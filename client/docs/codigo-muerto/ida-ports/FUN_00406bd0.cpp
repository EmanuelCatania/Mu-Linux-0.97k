// 0x00406BD0 FUN_00406bd0 — nunca activado: IDA_PORT_00406BD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00406bd0 (IDA-only, gated) ──
#if defined(IDA_PORT_00406BD0)
bool __cdecl FUN_00406bd0(unsigned char *Text)
{
  unsigned char *lpszCheck; // esi
  unsigned char v2; // al
  unsigned char v3; // cl
  unsigned char lpszTrail; // al

  lpszCheck = Text;
  if ( !*Text )
  {
    return 0;
  }
  while ( _mbclen(lpszCheck) == 1 )
  {
    v2 = *lpszCheck;
    if ( *lpszCheck < 0x30u || v2 >= 0x3Au && v2 < 0x41u || v2 >= 0x5Bu && v2 < 0x61u || v2 > 0x7Au )
    {
      return 1;
    }
LABEL_27:
    if ( !*++lpszCheck )
    {
      return 0;
    }
  }
  v3 = *lpszCheck;
  if ( *lpszCheck >= 0x81u && v3 <= 0xC8u )
  {
    if ( (lpszTrail = lpszCheck[1], lpszTrail >= 0x41u) && lpszTrail <= 0x5Au
      || lpszTrail >= 0x61u && lpszTrail <= 0x7Au
      || lpszTrail >= 0x81u && lpszTrail != 0xFF )
    {
      if ( (v3 < 0xA1u || v3 > 0xAFu || lpszTrail < 0xA1u)
        && (v3 != 0xC6 || lpszTrail < 0x53u || lpszTrail > 0xA0u)
        && (v3 < 0xC7u || lpszTrail > 0xA0u) )
      {
        ++lpszCheck;
        goto LABEL_27;
      }
    }
  }
  return 1;
}
#endif
