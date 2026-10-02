// 0x0053D0D0 CSimpleModulus_Shift — nunca activado: IDA_PORT_0053D0D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CSimpleModulus_Shift (IDA-only, gated) ──
#if defined(IDA_PORT_0053D0D0)
char __stdcall CSimpleModulus_Shift(BYTE *a1, int a2, int a3)
{
  char result; // al
  BYTE *v4; // ecx
  BYTE *v5; // edi
  int v6; // esi
  BYTE *v7; // esi
  int v8; // edi
  int v9; // ebp

  result = a3;
  if ( a3 )
  {
    if ( a3 <= 0 )
    {
      v7 = a1;
      v8 = -a3;
      if ( a2 - 1 > 0 )
      {
        v9 = a2 - 1;
        do
        {
          *v7 = (*v7 << v8) | (v7[1] >> (a3 + 8));
          ++v7;
          --v9;
        }
        while ( v9 );
      }
      result = *v7 << v8;
      *v7 = result;
    }
    else
    {
      v4 = a1;
      v5 = &a1[a2 - 1];
      if ( a2 - 1 > 0 )
      {
        v6 = a2 - 1;
        while ( 1 )
        {
          *v5 = (*v5 >> a3) | (v4[v6 - 1] << (8 - a3));
          --v5;
          if ( --v6 <= 0 )
          {
            break;
          }
          v4 = a1;
        }
      }
      result = *v5 >> a3;
      *v5 = result;
    }
  }
  return result;
}
#endif
