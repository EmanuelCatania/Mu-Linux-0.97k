// 0x004102E0 FUN_004102e0 — nunca activado: IDA_PORT_004102E0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004102e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004102E0)
DWORD __cdecl FUN_004102e0(DWORD *_this, int a2, char a3)
{
  DWORD result; // eax
  DWORD v4; // edx

  result = 0;
  switch ( a3 )
  {
    case 1:
    case -12:
      v4 = -16776961;
      goto LABEL_13;
    case -16:
      v4 = m_dwTextColor;
      result = m_dwBackColor;
      goto LABEL_14;
    case -15:
      v4 = m_dwBackColor;
      result = m_dwTextColor;
      goto LABEL_14;
    case -14:
      v4 = -14116;
      goto LABEL_13;
    case -13:
      v4 = -16711736;
      goto LABEL_13;
    case -11:
      result = 0;
      v4 = -11521516;
      goto LABEL_14;
    case -10:
      result = 0;
      v4 = -16751556;
      goto LABEL_14;
    case -9:
      result = 0;
      v4 = -16777116;
      goto LABEL_14;
    case -8:
      v4 = -3613466;
      if ( !g_bUseChatListBox )
      {
        result = -1778384896;
      }
      goto LABEL_14;
    default:
      v4 = -1;
LABEL_13:
      result = 0;
LABEL_14:
      _this[4 * a2 + 11] = v4;
      _this[4 * a2 + 12] = result;
      return result;
  }
}
#endif
