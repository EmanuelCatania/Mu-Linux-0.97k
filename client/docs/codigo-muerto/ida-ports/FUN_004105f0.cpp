// 0x004105F0 FUN_004105f0 — nunca activado: IDA_PORT_004105F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004105f0 (IDA-only, gated) ──
#if defined(IDA_PORT_004105F0)
int __cdecl FUN_004105f0(DWORD *_this, DWORD *a2, int a3, int a4, int a5)
{
  int result; // eax
  int v6; // edx
  DWORD *v7; // ebp
  int v8; // edi
  BYTE *v9; // eax
  int v10; // edx
  DWORD *v11; // esi
  DWORD v12; // edx
  int v13; // [esp+0h] [ebp-4h]

  result = a5;
  v6 = 0;
  v13 = 0;
  if ( a5 > 0 )
  {
    v7 = a2;
    do
    {
      v8 = 0;
      v9 = (char *)ppvBits + 512 * v6 * DAT_005590bc + 256 * v6 * DAT_005590bc + 2 * a3 + a3;
      if ( a4 > 0 )
      {
        do
        {
          v10 = _this[49] - 1;
          if ( v10 >= 0 )
          {
            v11 = &_this[4 * v10 + 10];
            do
            {
              if ( v8 + a3 > *v11 )
              {
                break;
              }
              --v10;
              v11 -= 4;
            }
            while ( v10 >= 0 );
          }
          if ( *v9 == 0xFF )
          {
            if ( v10 == -1 )
            {
              v12 = m_dwTextColor;
            }
            else
            {
              v12 = _this[4 * v10 + 11];
            }
          }
          else if ( v10 == -1 )
          {
            v12 = m_dwBackColor;
          }
          else
          {
            v12 = _this[4 * v10 + 12];
          }
          *v7 = v12;
          v9 += 3;
          ++v7;
          ++v8;
        }
        while ( v8 < a4 );
        v6 = v13;
      }
      result = a5;
      ++v6;
      v7 = a2 + 256;
      v13 = v6;
      a2 += 256;
    }
    while ( v6 < a5 );
  }
  return result;
}
#endif
