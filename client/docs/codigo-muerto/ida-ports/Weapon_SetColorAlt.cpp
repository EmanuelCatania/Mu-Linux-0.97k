// 0x00503FE0 Weapon_SetColorAlt — nunca activado: IDA_PORT_00503FE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Weapon_SetColorAlt (IDA-only, gated) ──
#if defined(IDA_PORT_00503FE0)
int __cdecl Weapon_SetColorAlt(int a1, float a2, float a3, int a4)
{
  int result; // eax
  int v5; // ecx
  double v6; // st7
  int v7; // ecx
  double v8; // st6
  double v9; // st6

  result = a1;
  v5 = 0;
  switch ( a1 )
  {
    case 533:
    case 541:
    case 414:
    case 565:
LABEL_16:
      v5 = 2;
      break;
    case 418:
LABEL_6:
      v5 = 0;
      break;
    case 545:
      v5 = 0;
      break;
    case 569:
      v5 = 0;
      break;
    case 420:
LABEL_12:
      v5 = 1;
      break;
    default:
      result = (a1 - 400) / 32;
      if ( result >= 7 && result <= 11 )
      {
        result = (a1 - 400) % 32;
        switch ( result )
        {
          case 0:
          case 1:
          case 2:
          case 3:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
          case 12:
          case 13:
          case 16:
            goto LABEL_6;
          case 4:
          case 14:
          case 15:
          case 17:
            goto LABEL_12;
          case 18:
            goto LABEL_16;
          default:
            goto LABEL_17;
        }
      }
      break;
  }
LABEL_17:
  v6 = a2 * a3;
  if ( v5 )
  {
    v7 = v5 - 1;
    if ( v7 )
    {
      if ( v7 == 1 )
      {
        result = a4;
        v8 = v6 * *(float *)(a4 + 4);
        *(DWORD *)a4 = 0;
        *(float *)(a4 + 4) = v8 * 0.5;
        *(float *)(a4 + 8) = v6 * *(float *)(a4 + 8);
      }
    }
    else
    {
      result = a4;
      v9 = v6 * *(float *)a4;
      *(DWORD *)(a4 + 8) = 0;
      *(float *)a4 = v9;
      *(float *)(a4 + 4) = v6 * *(float *)(a4 + 4) * 0.5;
    }
  }
  else
  {
    result = a4;
    *(float *)a4 = v6 * *(float *)a4;
    *(float *)(a4 + 4) = v6 * *(float *)(a4 + 4);
    *(float *)(a4 + 8) = v6 * *(float *)(a4 + 8);
  }
  return result;
}
#endif
