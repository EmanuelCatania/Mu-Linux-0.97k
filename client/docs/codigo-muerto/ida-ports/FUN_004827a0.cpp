// 0x004827A0 FUN_004827a0 — nunca activado: IDA_PORT_004827A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004827a0 (IDA-only, gated) ──
#if defined(IDA_PORT_004827A0)
int FUN_004824c0()
{
  void *v0; // ebp
  unsigned int v1; // eax
  bool v2; // cf
  BYTE *v3; // eax
  unsigned char v4; // cl
  const void *v5; // ebp
  int v6; // esi
  unsigned int v7; // eax
  int v8; // eax
  unsigned int v9; // eax
  BYTE *v10; // eax
  char v11; // cl
  int v12; // ebx
  int v13; // edi
  int *v14; // esi
  int v15; // ecx
  int result; // eax
  int *v17; // edx
  const void *v18; // ebp
  unsigned int v19; // eax
  unsigned int v20; // eax
  BYTE *v21; // eax
  char v22; // cl
  int v23; // [esp+10h] [ebp-14h]
  int v24; // [esp+10h] [ebp-14h]
  int v25; // [esp+14h] [ebp-10h] BYREF
  DWORD v26; // [esp+18h] [ebp-Ch] BYREF
  DWORD v27; // [esp+1Ch] [ebp-8h] BYREF
  int v28; // [esp+20h] [ebp-4h] BYREF

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
        v3 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)v0);
        v4 = v3[1412] + 1;
        v3[1412] = v4;
        if ( v4 < 2u )
        {
          Packet_DecryptBuffer(v0, v3);
        }
        goto LABEL_9;
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
  v8 = operator_new(0x585u);
  *(BYTE *)(v8 + 1412) = 1;
  HashTable_Insert(&MAIN_HASH_CLASS, v8, (int)v0);
LABEL_9:
  if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 )
  {
    v5 = (const void *)CharacterMachine;
    v6 = *(short *)(CharacterMachine + 536);
    v28 = *(short *)(CharacterMachine + 604);
    v24 = v6;
    v27 = CharacterMachine;
    v7 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
      {
        if ( !memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
        {
          if ( v7 != -1 )
          {
            v9 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v5);
            v10 = v9 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v9);
            v11 = v10[1412] - 1;
            v10[1412] = v11;
            if ( !v11 )
            {
              Packet_EncryptBuffer(v10, v5);
            }
          }
          break;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v7 = (v7 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          v6 = v24;
          goto LABEL_15;
        }
      }
      v6 = v24;
    }
    else
    {
LABEL_15:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v28 >= 128 && v28 < 135 || v28 == 145 )
    {
      v12 = 143;
    }
    else if ( (v6 < 136 || v6 >= 143) && (v6 < 144 || v6 >= 160) )
    {
      v12 = v28;
    }
    else
    {
      v12 = 135;
    }
    v13 = 7;
    v14 = (int *)&DAT_07ea9504;
LABEL_35:
    v15 = 7;
    result = v13 + 56;
    v17 = v14;
    while ( *((short *)v17 - 28) != v12 || *v17 <= 0 )
    {
      --v15;
      v17 -= 136;
      result -= 8;
      if ( v15 < 0 )
      {
        v14 -= 17;
        --v13;
        if ( (int)v14 >= (int)&DAT_07ea9328 )
        {
          goto LABEL_35;
        }
        return -1;
      }
    }
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
          return -1;
        }
        v2 = ++v25 < (unsigned int)DAT_055c9bd4;
        v19 = (v19 + 1) % DAT_055c9bd4;
        if ( !v2 )
        {
          goto LABEL_45;
        }
      }
      return -1;
    }
    else
    {
LABEL_45:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      return -1;
    }
  }
  return result;
}
#endif
