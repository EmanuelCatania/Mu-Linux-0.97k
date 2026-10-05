// 0x00401230 FUN_00401230 — nunca activado: IDA_PORT_00401230 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00401230 (IDA-only, gated) ──
#if defined(IDA_PORT_00401230)
bool __cdecl CSQuest::CheckRequestCondition(DWORD This, DWORD pQuest, bool bLastCheck)
{
  short *v4; // ecx
  int v5; // ebx
  short *v6; // ebp
  void *v7; // esi
  unsigned int v8; // eax
  BYTE *v9; // eax
  unsigned char v10; // cl
  int v11; // eax
  const void *v12; // esi
  unsigned short v13; // di
  unsigned int v14; // eax
  BYTE *v15; // eax
  char v16; // cl
  void *v17; // esi
  unsigned int v18; // eax
  BYTE *v19; // eax
  unsigned char v20; // cl
  int v21; // eax
  const void *v22; // esi
  unsigned short v23; // di
  unsigned int v24; // eax
  BYTE *v25; // eax
  char v26; // cl
  int v27; // eax
  void *v28; // esi
  unsigned int v29; // eax
  BYTE *v30; // eax
  unsigned char v31; // cl
  int v32; // eax
  const void *v33; // esi
  int v34; // edi
  BYTE *v35; // eax
  char v36; // cl
  int v37; // eax
  short v40; // cx
  short v41; // ax
  DWORD v42; // [esp+10h] [ebp-14h]
  int v43; // [esp+14h] [ebp-10h]
  unsigned char *i; // [esp+18h] [ebp-Ch]
  int v45; // [esp+1Ch] [ebp-8h]
  int v46; // [esp+20h] [ebp-4h]

  v4 = (short *)pQuest;
  v42 = This;
  v45 = 0;
  if ( *(short *)pQuest <= 0 )
  {
    return 1;
  }
  v43 = 0;
  for ( i = (unsigned char *)(pQuest + 43); ; i += 18 )
  {
    if ( *((unsigned char *)&v4[v43 + 22] + *(unsigned char *)(This + 4)) == *(unsigned char *)(This + 5) + 1 )
    {
      v5 = 0;
      v46 = *i;
      if ( v4[1] > 0 )
      {
        break;
      }
    }
LABEL_57:
    v43 += 9;
    if ( ++v45 >= *v4 )
    {
      return 1;
    }
  }
  v6 = v4 + 166;
  while ( 1 )
  {
    if ( *((unsigned char *)v6 - 3) == v46 || *((BYTE *)v6 - 3) == 0xFF )
    {
      if ( *v6 )
      {
        v7 = (void *)CharacterMachine;
        if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
        {
          v11 = operator_new(0x585u);
          *(BYTE *)(v11 + 1412) = 1;
          HashTable_Insert(&MAIN_HASH_CLASS, v11, (int)v7);
        }
        else
        {
          v8 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v7);
          if ( v8 == -1 )
          {
            v9 = 0;
          }
          else
          {
            v9 = *(BYTE **)(DAT_055c9bcc + 4 * v8);
          }
          v10 = v9[1412] + 1;
          v9[1412] = v10;
          if ( v10 < 2u )
          {
            Packet_DecryptBuffer(v7, v9);
          }
        }
        v12 = (const void *)CharacterMachine;
        v13 = *(WORD *)(CharacterAttribute + 14);
        if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) != -1 )
        {
          v14 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v12);
          v15 = v14 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v14);
          v16 = v15[1412] - 1;
          v15[1412] = v16;
          if ( !v16 )
          {
            Packet_EncryptBuffer(v15, v12);
          }
        }
        if ( (unsigned short)*v6 > v13 )
        {
          *(WORD *)(v42 + 116864) = *(WORD *)(16 * v5 + pQuest + 340);
          *(BYTE *)(v42 + 116866) = 5;
          return 0;
        }
        v4 = (short *)pQuest;
      }
      if ( v6[1] )
      {
        v17 = (void *)CharacterMachine;
        if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
        {
          v21 = operator_new(0x585u);
          *(BYTE *)(v21 + 1412) = 1;
          HashTable_Insert(&MAIN_HASH_CLASS, v21, (int)v17);
        }
        else
        {
          v18 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v17);
          if ( v18 == -1 )
          {
            v19 = 0;
          }
          else
          {
            v19 = *(BYTE **)(DAT_055c9bcc + 4 * v18);
          }
          v20 = v19[1412] + 1;
          v19[1412] = v20;
          if ( v20 < 2u )
          {
            Packet_DecryptBuffer(v17, v19);
          }
        }
        v22 = (const void *)CharacterMachine;
        v23 = *(WORD *)(CharacterAttribute + 14);
        if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) != -1 )
        {
          v24 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v22);
          v25 = v24 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v24);
          v26 = v25[1412] - 1;
          v25[1412] = v26;
          if ( !v26 )
          {
            Packet_EncryptBuffer(v25, v22);
          }
        }
        if ( (unsigned short)v6[1] < v23 )
        {
          v40 = *(WORD *)(16 * v5 + pQuest + 340);
          *(BYTE *)(v42 + 116866) = 5;
          *(WORD *)(v42 + 116864) = v40;
          return 0;
        }
        v4 = (short *)pQuest;
      }
      v27 = *((DWORD *)v6 + 1);
      if ( v27 )
      {
        break;
      }
    }
LABEL_55:
    ++v5;
    v6 += 8;
    if ( v5 >= v4[1] )
    {
      This = v42;
      goto LABEL_57;
    }
  }
  if ( !bLastCheck )
  {
    *(DWORD *)(v42 + 116868) = v27;
    goto LABEL_55;
  }
  v28 = (void *)CharacterMachine;
  if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
  {
    v32 = operator_new(0x585u);
    *(BYTE *)(v32 + 1412) = 1;
    HashTable_Insert(&MAIN_HASH_CLASS, v32, (int)v28);
  }
  else
  {
    v29 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v28);
    if ( v29 == -1 )
    {
      v30 = 0;
    }
    else
    {
      v30 = *(BYTE **)(DAT_055c9bcc + 4 * v29);
    }
    v31 = v30[1412] + 1;
    v30[1412] = v31;
    if ( v31 < 2u )
    {
      Packet_DecryptBuffer(v28, v30);
    }
  }
  v33 = (const void *)CharacterMachine;
  v34 = *(DWORD *)(CharacterMachine + 1352);
  if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) != -1 )
  {
    v35 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, v33);
    v36 = v35[1412] - 1;
    v35[1412] = v36;
    if ( !v36 )
    {
      Packet_EncryptBuffer(v35, v33);
    }
  }
  v37 = *((DWORD *)v6 + 1);
  *(DWORD *)(v42 + 116868) = v37;
  if ( v37 <= v34 )
  {
    v4 = (short *)pQuest;
    goto LABEL_55;
  }
  v41 = *(WORD *)(16 * v5 + pQuest + 340);
  *(BYTE *)(v42 + 116866) = 5;
  *(WORD *)(v42 + 116864) = v41;
  return 0;
}
#endif
