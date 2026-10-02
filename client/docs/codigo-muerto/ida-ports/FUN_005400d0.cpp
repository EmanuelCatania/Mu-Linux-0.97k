// 0x005400D0 FUN_005400d0 — nunca activado: IDA_PORT_005400D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005400d0 (IDA-only, gated) ──
#if defined(IDA_PORT_005400D0)
char __stdcall FUN_005400d0(int ArgList, unsigned int a2)
{
  BYTE *v2; // ecx
  int v3; // edi
  char *v4; // eax
  int v5; // esi
  char *v6; // eax
  char result; // al
  char *v8; // eax
  char *v9; // eax
  char *v10; // eax
  int v11; // [esp-4h] [ebp-Ch]

  v2 = lpParameter;
  v3 = a2;
  if ( lpParameter )
  {
    v4 = FUN_0053e8c0(DAT_00563b38);
    FUN_0053eba0((int)lpParameter + 32, v4, ArgList, a2);
    v2 = lpParameter;
  }
  switch ( ArgList )
  {
    case 1551:
      v5 = 1001;
      goto LABEL_37;
    case 1552:
      v5 = 1002;
      goto LABEL_37;
    case 1556:
      if ( v2[29] || FUN_0053efa0(v2, a2) )
      {
        goto LABEL_10;
      }
      v5 = 1014;
      v3 = 220;
      DAT_083bbaf0 = 2;
      goto LABEL_37;
    case 1557:
      DAT_083bbb08 = a2;
      if ( a2 == DAT_083bbb0c )
      {
        goto LABEL_10;
      }
      if ( v2 )
      {
        v11 = DAT_083bbb0c;
        v9 = FUN_0053e8c0(DAT_00563ae4);
        FUN_0053eba0((int)lpParameter + 32, v9, a2, v11);
      }
      DAT_083bbaf0 = 5;
      return 1;
    case 1560:
      if ( v2 )
      {
        v6 = FUN_0053e8c0(DAT_00563b20);
        FUN_0053eba0((int)lpParameter + 32, v6, a2);
      }
      if ( a2 > 0xC8 )
      {
        DAT_083bbaf0 = 9;
      }
      goto LABEL_10;
    case 1581:
      v5 = 1011;
      goto LABEL_15;
    case 1582:
      v5 = 1012;
      goto LABEL_18;
    case 1583:
      v5 = 1013;
      goto LABEL_21;
    case 1584:
      v5 = 1015;
      goto LABEL_15;
    case 1591:
      v5 = 1014;
      v3 = 210;
LABEL_18:
      if ( *v2 )
      {
        goto LABEL_37;
      }
      *((DWORD *)v2 + 1) = v5;
      *((DWORD *)lpParameter + 2) = v3;
      result = 1;
      break;
    case 1592:
      v5 = 1014;
      v3 = 220;
      DAT_083bbaf0 = 2;
LABEL_21:
      if ( *v2 )
      {
        goto LABEL_37;
      }
      *((DWORD *)v2 + 1) = v5;
      result = 1;
      *((DWORD *)lpParameter + 2) = v3;
      break;
    case 1593:
      v5 = 1014;
      v3 = 230;
      DAT_083bbaf0 = 2;
LABEL_15:
      if ( *v2 )
      {
        goto LABEL_37;
      }
      *((DWORD *)v2 + 1) = v5;
      result = 1;
      *((DWORD *)lpParameter + 2) = v3;
      break;
    case 1594:
      if ( v2 )
      {
        v8 = FUN_0053e8c0(DAT_00563b0c);
        FUN_0053eba0((int)lpParameter + 32, v8, a2, a2);
      }
      v5 = 1016;
      goto LABEL_37;
    default:
      if ( ArgList == 500 )
      {
        DAT_083bbaf0 = 3;
        v5 = 1014;
        v3 = 500;
      }
      else
      {
        v5 = 1000;
      }
LABEL_37:
      if ( FUN_004070d0(v5, v3) )
      {
LABEL_10:
        result = 1;
      }
      else
      {
        if ( lpParameter )
        {
          v10 = FUN_0053e8c0(DAT_00563ac4);
          FUN_0053eba0((int)lpParameter + 32, v10, v5, v3);
        }
        result = 0;
        *((BYTE *)lpParameter + 30) = 1;
      }
      break;
  }
  return result;
}
#endif
