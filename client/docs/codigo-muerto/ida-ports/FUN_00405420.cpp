// 0x00405420 FUN_00405420 — nunca activado: IDA_PORT_00405420 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00405420 (IDA-only, gated) ──
#if defined(IDA_PORT_00405420)
char *__stdcall FUN_00405420(char *Str, int a2)
{
  char *result; // eax
  int v3; // ebp
  char *v4; // esi
  size_t v5; // edi
  char *v6; // ebx
  char *v7; // eax
  char *v8; // esi
  int v9[4]; // [esp+0h] [ebp-210h]
  char v10; // [esp+10h] [ebp-200h] BYREF

  result = Str;
  v3 = 0;
  v4 = Str;
  v5 = strlen(Str2);
  if ( Str )
  {
    v6 = &v10;
    do
    {
      if ( !*v4 )
      {
        break;
      }
      v7 = strchr(v4, 35);
      v8 = v7;
      if ( !v7 )
      {
        break;
      }
      if ( !strncmp(v7, Str2, v5) )
      {
        *(DWORD *)v6 = v8;
        ++v3;
        v6 += 4;
        v4 = &v8[v5];
      }
      else
      {
        v4 = v8 + 1;
      }
    }
    while ( v4 );
    if ( v3 < 5 )
    {
      return Str;
    }
    else
    {
      return (char *)v9[v3];
    }
  }
  return result;
}
#endif
