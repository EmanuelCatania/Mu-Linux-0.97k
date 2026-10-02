// 0x004C4080 FUN_004c4080 — nunca activado: IDA_PORT_004C4080 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004c4080 (IDA-only, gated) ──
#if defined(IDA_PORT_004C4080)
void FUN_004c4080()
{
  void *v0; // ebx
  unsigned int v1; // eax
  bool v2; // cf
  int v3; // eax
  int v4; // ebx
  int i; // edi
  short *v6; // esi
  WORD v7; // ax
  short v8; // cx
  int v9; // eax
  ITEM *v10; // esi
  int v11; // edi
  WORD v12; // ax
  short Type; // cx
  int v14; // eax
  unsigned int v15; // edx
  unsigned int v16; // eax
  BYTE *v17; // esi
  unsigned char v18; // al
  void *v19; // ebp
  unsigned int v20; // ecx
  int v21; // esi
  unsigned int v22; // edx
  int v23; // ebx
  char *v24; // edx
  char v25; // al
  BYTE *v26; // eax
  char *v27; // ebx
  BYTE *v28; // ebp
  unsigned int v29; // esi
  int v30; // ebx
  BYTE *v31; // edi
  char v32; // al
  int v33; // [esp-10h] [ebp-90h]
  int Durability; // [esp-10h] [ebp-90h]
  int v35; // [esp-Ch] [ebp-8Ch]
  int v36; // [esp-Ch] [ebp-8Ch]
  short v37; // [esp-8h] [ebp-88h]
  short v38; // [esp-8h] [ebp-88h]
  char *v39; // [esp+10h] [ebp-70h] BYREF
  char *v40; // [esp+14h] [ebp-6Ch] BYREF
  DWORD v41; // [esp+18h] [ebp-68h] BYREF
  char Buffer[100]; // [esp+1Ch] [ebp-64h] BYREF

  v0 = (void *)CharacterMachine;
  DAT_07eaa0f8 = 0;
  v41 = CharacterMachine;
  v1 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v39 = 0;
  v40 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v39, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
    {
      if ( !memcmp((const char *)&v41, (const char *)(DAT_055c9bd0 + 4 * v1), 4) )
      {
        if ( v1 == -1 )
        {
          break;
        }
        v16 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v0);
        if ( v16 == -1 )
        {
          v17 = 0;
        }
        else
        {
          v17 = *(BYTE **)(DAT_055c9bcc + 4 * v16);
        }
        v18 = v17[1412] + 1;
        v17[1412] = v18;
        if ( v18 < 2u )
        {
          v19 = (void *)operator_new(0x584u);
          qmemcpy(v19, v17, 0x584u);
          v20 = 1411;
          v21 = 1412;
          do
          {
            if ( v20 < 0x583 )
            {
              *((BYTE *)v19 + v20) ^= *((BYTE *)v19 + v20 + 1);
            }
            *((BYTE *)v19 + v20) = ((*((BYTE *)v19 + v20) - 35) ^ DAT_00559050[(int)v20 % 16]) - 71;
            --v20;
            --v21;
          }
          while ( v21 );
          qmemcpy(v0, v19, 0x584u);
          delete__(v19);
        }
        goto LABEL_7;
      }
      v2 = (unsigned int)++v40 < DAT_055c9bd4;
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
  v4 = 0;
  for ( i = 0; i < 816; i += 68 )
  {
    v6 = (short *)(i + CharacterMachine + 536);
    if ( *v6 != -1 )
    {
      if ( *(DWORD *)(i + CharacterMachine + 592) )
      {
        v7 = CalcMaxDurability(
               (ITEM *)(i + CharacterMachine + 536),
               &ItemAttribute[*v6],
               (*(int *)(i + CharacterMachine + 540) >> 3) & 0xF);
        v8 = *v6;
        if ( (*v6 < 416 || v8 > 419)
          && v8 != 426
          && v8 != 135
          && v8 != 143
          && v8 < 448
          && (v8 < 391 || v8 > 403)
          && (v8 < 430 || v8 > 435)
          && *((unsigned char *)v6 + 26) < (int)v7 )
        {
          v37 = *v6;
          v35 = v7;
          v33 = *((unsigned char *)v6 + 26);
          v9 = ItemValue((int)v6, 2);
          DAT_07eaa0f8 += ConvertRepairGold(v9, v33, v35, v37, Buffer);
        }
      }
    }
  }
  v10 = &OffsetInventoryItems;
  do
  {
    v11 = 8;
    do
    {
      if ( v10->Key )
      {
        v12 = CalcMaxDurability(v10, &ItemAttribute[v10->Type], (v10->Level >> 3) & 0xF);
        Type = v10->Type;
        if ( (v10->Type < 416 || Type > 419)
          && Type != 426
          && Type != 135
          && Type != 143
          && Type < 448
          && (Type < 391 || Type > 403)
          && (Type < 430 || Type > 435)
          && v10->Durability < (int)v12 )
        {
          v38 = v10->Type;
          v36 = v12;
          Durability = v10->Durability;
          v14 = ItemValue((int)v10, 2);
          DAT_07eaa0f8 += ConvertRepairGold(v14, Durability, v36, v38, Buffer);
        }
      }
      ++v10;
      --v11;
    }
    while ( v11 );
  }
  while ( (int)v10 < (int)&DAT_07ea9510 );
  v39 = (char *)CharacterMachine;
  v40 = (char *)CharacterMachine;
  v15 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v41 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v41, (const char *)(DAT_055c9bd0 + 4 * v15), 4) )
    {
      if ( !memcmp((const char *)&v40, (const char *)(DAT_055c9bd0 + 4 * v15), 4) )
      {
        if ( v15 != -1 )
        {
          v40 = v39;
          v22 = (*(int (__cdecl **)(int *, char *))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, v39);
          v23 = 0;
          v41 = 0;
          if ( DAT_055c9bd4 )
          {
            while ( memcmp((const char *)&v41, (const char *)(DAT_055c9bd0 + 4 * v22), 4) )
            {
              if ( !memcmp((const char *)&v40, (const char *)(DAT_055c9bd0 + 4 * v22), 4) )
              {
                if ( v22 == -1 )
                {
                  break;
                }
                v24 = *(char **)(DAT_055c9bcc + 4 * v22);
                v40 = v24;
                goto LABEL_62;
              }
              v22 = (v22 + 1) % DAT_055c9bd4;
              if ( ++v23 >= (unsigned int)DAT_055c9bd4 )
              {
                goto LABEL_60;
              }
            }
          }
          else
          {
LABEL_60:
            CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
          }
          v40 = 0;
          v24 = 0;
LABEL_62:
          v25 = v24[1412] - 1;
          v24[1412] = v25;
          if ( !v25 )
          {
            v26 = (BYTE *)operator_new(0x584u);
            v27 = v39;
            v28 = v26;
            qmemcpy(v26, v39, 0x584u);
            v29 = 0;
            v30 = v27 - v26;
            do
            {
              v31 = &v28[v29];
              v32 = ((v28[v29] + 71) ^ DAT_00559050[v29 & 0x8000000F]) + 35;
              v28[v29] = v32;
              if ( v29 < 0x583 )
              {
                *v31 = v32 ^ v28[v29 + 1];
              }
              ++v29;
              v31[v30] = rand();
            }
            while ( v29 < 0x584 );
            qmemcpy(v40, v28, 0x584u);
            delete__(v28);
          }
        }
        return;
      }
      v15 = (v15 + 1) % DAT_055c9bd4;
      if ( ++v4 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_44;
      }
    }
  }
  else
  {
LABEL_44:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
}
#endif
