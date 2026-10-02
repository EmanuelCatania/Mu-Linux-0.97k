// 0x00482850 FUN_00482850 — nunca activado: IDA_PORT_00482850 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00482850 (IDA-only, gated) ──
#if defined(IDA_PORT_00482850)
int FUN_00482850()
{
  void *v0; // ebp
  unsigned int v1; // eax
  bool v2; // cf
  int v3; // eax
  const void *v4; // ebp
  int v5; // esi
  unsigned int v6; // eax
  BYTE *v7; // eax
  unsigned char v8; // cl
  unsigned int v9; // eax
  BYTE *v10; // eax
  char v11; // cl
  int v12; // esi
  int v13; // edi
  int *v14; // edx
  int *v15; // eax
  int v16; // ecx
  const void *v18; // ebp
  unsigned int v19; // eax
  unsigned int v20; // eax
  BYTE *v21; // eax
  char v22; // cl
  int v23; // [esp+14h] [ebp-14h]
  int v24; // [esp+14h] [ebp-14h]
  int v25; // [esp+18h] [ebp-10h] BYREF
  DWORD v26; // [esp+1Ch] [ebp-Ch] BYREF
  DWORD v27; // [esp+20h] [ebp-8h] BYREF
  int v28; // [esp+24h] [ebp-4h] BYREF

  v0 = (void *)CharacterMachine;
  v26 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v25 = 0;
  v23 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v25, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v7 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)v0);
        v8 = v7[1412] + 1;
        v7[1412] = v8;
        if ( v8 < 2u )
        {
          Packet_DecryptBuffer(v0, v7);
        }
        goto LABEL_7;
      }
      v2 = ++v23 < (unsigned int)DAT_055c9bd4;
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
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 )
  {
    v4 = (const void *)CharacterMachine;
    v5 = *(short *)(CharacterMachine + 536);
    v28 = *(short *)(CharacterMachine + 604);
    v24 = v5;
    v27 = CharacterMachine;
    v6 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
        {
          if ( v6 != -1 )
          {
            v9 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v4);
            v10 = v9 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v9);
            v11 = v10[1412] - 1;
            v10[1412] = v11;
            if ( !v11 )
            {
              Packet_EncryptBuffer(v10, v4);
            }
          }
          break;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v6 = (v6 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          v5 = v24;
          goto LABEL_13;
        }
      }
      v5 = v24;
    }
    else
    {
LABEL_13:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v28 >= 128 && v28 < 135 || v28 == 145 )
    {
      v12 = 143;
    }
    else if ( (v5 < 136 || v5 >= 143) && (v5 < 144 || v5 >= 160) )
    {
      v12 = v28;
    }
    else
    {
      v12 = 135;
    }
    v13 = 0;
    v14 = (int *)&DAT_07ea9504;
    do
    {
      v15 = v14;
      v16 = 8;
      do
      {
        if ( *((short *)v15 - 28) == v12 && *v15 > 0 )
        {
          ++v13;
        }
        v15 -= 136;
        --v16;
      }
      while ( v16 );
      v14 -= 17;
    }
    while ( (int)v14 >= (int)&DAT_07ea9328 );
    return v13;
  }
  else
  {
    v18 = (const void *)CharacterMachine;
    v27 = CharacterMachine;
    v19 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
    v28 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v19), 4) )
        {
          if ( v19 != -1 )
          {
            v20 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v18);
            v21 = v20 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v20);
            v22 = v21[1412] - 1;
            v21[1412] = v22;
            if ( !v22 )
            {
              Packet_EncryptBuffer(v21, v18);
            }
          }
          return 0;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v19 = (v19 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          goto LABEL_46;
        }
      }
      return 0;
    }
    else
    {
LABEL_46:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      return 0;
    }
  }
}
#endif
