// 0x0040B350 FUN_0040b350 — nunca activado: IDA_PORT_0040B350 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040b350 (IDA-only, gated) ──
#if defined(IDA_PORT_0040B350)
int FUN_0040b350()
{
  char i; // bl
  int result; // eax
  char *v2; // esi
  char k; // bl
  FILE *v4; // edx
  char *v5; // esi
  char j; // al
  FILE *v7; // edx
  char m; // bl
  FILE *v9; // edx
  char String[100]; // [esp+8h] [ebp-64h] BYREF

  DAT_00590b10 = 0;
  i = fgetc(fp);
  if ( i == -1 )
  {
    return 2;
  }
  while ( 1 )
  {
    if ( i == 47 )
    {
      i = fgetc(fp);
      if ( i == 47 )
      {
        for ( i = fgetc(fp); i != 10; i = fgetc(fp) )
        {
          ;
        }
      }
    }
    if ( !isspace(i) )
    {
      break;
    }
    i = fgetc(fp);
    if ( i == -1 )
    {
      return 2;
    }
  }
  switch ( i )
  {
    case '"':
      v5 = &DAT_00590b10;
      for ( j = getc(fp); j != -1; j = getc(v7) )
      {
        if ( j == 34 )
        {
          goto LABEL_25;
        }
        v7 = SMDFile;
        *v5++ = j;
      }
      ungetc(j, SMDFile);
      goto LABEL_25;
    case '#':
      result = 35;
      DAT_00809794 = 35;
      break;
    case ',':
      result = 44;
      DAT_00809794 = 44;
      break;
    case '-':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      ungetc(i, SMDFile);
      v2 = String;
      for ( k = getc(fp); k != -1; k = getc(v4) )
      {
        if ( k != 46 && !isdigit(k) && k != 45 )
        {
          break;
        }
        v4 = SMDFile;
        *v2++ = k;
      }
      *v2 = 0;
      DAT_00809798 = atof(String);
      result = 1;
      DAT_00809794 = 1;
      break;
    case ';':
      result = 59;
      DAT_00809794 = 59;
      break;
    case '{':
      result = 123;
      DAT_00809794 = 123;
      break;
    case '}':
      result = 125;
      DAT_00809794 = 125;
      break;
    default:
      if ( isalpha(i) )
      {
        DAT_00590b10 = i;
        v5 = (char *)&DAT_00590b11;
        for ( m = getc(fp); m != -1; m = getc(v9) )
        {
          if ( m != 46 && m != 95 && !isalnum(m) )
          {
            break;
          }
          v9 = SMDFile;
          *v5++ = m;
        }
        ungetc(m, SMDFile);
LABEL_25:
        *v5 = 0;
        DAT_00809794 = 0;
        result = 0;
      }
      else
      {
        result = 60;
        DAT_00809794 = 60;
      }
      break;
  }
  return result;
}
#endif
