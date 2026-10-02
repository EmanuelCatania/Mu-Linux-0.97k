// 0x004D1FC0 FUN_004d1fc0 — nunca activado: IDA_PORT_004D1FC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004d1fc0 (IDA-only, gated) ──
#if defined(IDA_PORT_004D1FC0)
void FUN_004d1fc0()
{
  void *v0; // ebx
  unsigned int v1; // eax
  bool v2; // cf
  int v3; // eax
  const void *v4; // ebp
  unsigned int v5; // edx
  unsigned int v6; // eax
  BYTE *v7; // eax
  unsigned char v8; // cl
  unsigned int v9; // eax
  char *v10; // eax
  char v11; // cl
  char *v12; // ebx
  unsigned int v13; // esi
  int v14; // ebp
  char *v15; // edi
  char v16; // al
  int v17; // [esp+10h] [ebp-Ch]
  int v18; // [esp+10h] [ebp-Ch]
  char *v19; // [esp+10h] [ebp-Ch]
  DWORD v20; // [esp+14h] [ebp-8h] BYREF
  DWORD v21; // [esp+18h] [ebp-4h] BYREF

  FUN_004cdc70(15.0, 46.0, 40.0, 40.0, 8);
  FUN_004cdc70(115.0, 46.0, 60.0, 40.0, 7);
  v0 = (void *)CharacterMachine;
  v21 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v20 = 0;
  v17 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v20, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v6 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v0);
        if ( v6 == -1 )
        {
          v7 = 0;
        }
        else
        {
          v7 = *(BYTE **)(DAT_055c9bcc + 4 * v6);
        }
        v8 = v7[1412] + 1;
        v7[1412] = v8;
        if ( v8 < 2u )
        {
          Packet_DecryptBuffer(v0, v7);
        }
        goto LABEL_7;
      }
      v2 = ++v17 < (unsigned int)DAT_055c9bd4;
      v1 = (v1 + 1) % DAT_055c9bd4;
      if ( !v2 )
      {
        goto LABEL_5;
      }
    }
  }
  else
  {
LABEL_5:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  v3 = operator_new(0x585u);
  *(BYTE *)(v3 + 1412) = 1;
  HashTable_Insert(&MAIN_HASH_CLASS, v3, (int)v0);
LABEL_7:
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) != 3 )
  {
    FUN_004cdc70(75.0, 46.0, 40.0, 40.0, 2);
  }
  v4 = (const void *)CharacterMachine;
  v20 = CharacterMachine;
  v5 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v21 = 0;
  v18 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
    {
      if ( !memcmp((const char *)&v20, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
      {
        if ( v5 != -1 )
        {
          v9 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v4);
          if ( v9 == -1 )
          {
            v19 = 0;
            v10 = 0;
          }
          else
          {
            v10 = *(char **)(DAT_055c9bcc + 4 * v9);
            v19 = v10;
          }
          v11 = v10[1412] - 1;
          v10[1412] = v11;
          if ( !v11 )
          {
            v12 = (char *)operator_new(0x584u);
            qmemcpy(v12, v4, 0x584u);
            v13 = 0;
            v14 = (BYTE *)v4 - v12;
            do
            {
              v15 = &v12[v13];
              v16 = ((v12[v13] + 71) ^ DAT_00559050[v13 & 0x8000000F]) + 35;
              v12[v13] = v16;
              if ( v13 < 0x583 )
              {
                *v15 = v16 ^ v12[v13 + 1];
              }
              ++v13;
              v15[v14] = rand();
            }
            while ( v13 < 0x584 );
            qmemcpy(v19, v12, 0x584u);
            delete__(v12);
          }
        }
        break;
      }
      v5 = (v5 + 1) % DAT_055c9bd4;
      if ( ++v18 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_13;
      }
    }
  }
  else
  {
LABEL_13:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  FUN_004cdc70(75.0, 89.0, 40.0, 60.0, 3);
  FUN_004cdc70(75.0, 152.0, 40.0, 40.0, 4);
  FUN_004cdc70(15.0, 89.0, 40.0, 60.0, 0);
  FUN_004cdc70(134.0, 89.0, 40.0, 60.0, 1);
  FUN_004cdc70(15.0, 152.0, 40.0, 40.0, 5);
  FUN_004cdc70(134.0, 152.0, 40.0, 40.0, 6);
  FUN_004cdc70(55.0, 89.0, 20.0, 20.0, 9);
  FUN_004cdc70(55.0, 152.0, 20.0, 20.0, 10);
  FUN_004cdc70(115.0, 152.0, 20.0, 20.0, 11);
}
#endif
