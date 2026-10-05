// 0x0048B680 FUN_0048b680 — nunca activado: IDA_PORT_0048B680 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0048b680 (IDA-only, gated) ──
#if defined(IDA_PORT_0048B680)
void __cdecl FUN_0048b680(int a1)
{
  void *v1; // ebx
  unsigned int v2; // eax
  bool v3; // cf
  int v4; // eax
  const void *v5; // ebx
  unsigned int v6; // edx
  unsigned int v7; // eax
  BYTE *v8; // eax
  unsigned char v9; // cl
  unsigned int v10; // eax
  char *v11; // eax
  char v12; // cl
  char *v13; // ebp
  unsigned int v14; // esi
  int v15; // ebx
  char *v16; // edi
  char v17; // al
  short v18; // cx
  char v19; // si
  short v20; // ax
  char v21; // si
  char v22; // [esp+Bh] [ebp-19h]
  int v23; // [esp+Ch] [ebp-18h]
  char *v24; // [esp+Ch] [ebp-18h]
  short *v25; // [esp+10h] [ebp-14h]
  int v26; // [esp+14h] [ebp-10h] BYREF
  DWORD v27; // [esp+18h] [ebp-Ch] BYREF
  int v28; // [esp+1Ch] [ebp-8h] BYREF
  short *v29; // [esp+20h] [ebp-4h]
  void *retaddr; // [esp+24h] [ebp+0h]

  v26 = FUN_004824c0();
  if ( v26 != -1 )
  {
    v1 = (void *)CharacterMachine;
    v27 = CharacterMachine;
    v2 = (*(int (__cdecl **)(int *, DWORD, int))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine, a1);
    v26 = 0;
    v25 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v26, (const char *)(DAT_055c9bd0 + 4 * v2), 4) )
      {
        if ( !memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v2), 4) )
        {
          if ( v2 == -1 )
          {
            break;
          }
          v7 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v1);
          if ( v7 == -1 )
          {
            v8 = 0;
          }
          else
          {
            v8 = *(BYTE **)(DAT_055c9bcc + 4 * v7);
          }
          v9 = v8[1412] + 1;
          v8[1412] = v9;
          if ( v9 < 2u )
          {
            Packet_DecryptBuffer(v1, v8);
          }
          goto LABEL_8;
        }
        v3 = (unsigned int)v25 + 1 < DAT_055c9bd4;
        v25 = (short *)((char *)v25 + 1);
        v2 = (v2 + 1) % DAT_055c9bd4;
        if ( !v3 )
        {
          goto LABEL_6;
        }
      }
    }
    else
    {
LABEL_6:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    v4 = operator_new(0x585u);
    *(BYTE *)(v4 + 1412) = 1;
    HashTable_Insert(&MAIN_HASH_CLASS, v4, (int)v1);
LABEL_8:
    v5 = (const void *)CharacterMachine;
    if ( (*(BYTE *)(CharacterAttribute + 11) & 7) == 2 && !DAT_07e91388 )
    {
      v26 = CharacterMachine + 536;
      retaddr = (void *)(CharacterMachine + 604);
    }
    v29 = (short *)CharacterMachine;
    v6 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
    v27 = 0;
    v23 = 0;
    if ( DAT_055c9bd4 )
    {
      while ( memcmp((const char *)&v27, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
      {
        if ( !memcmp((const char *)&v28, (const char *)(DAT_055c9bd0 + 4 * v6), 4) )
        {
          if ( v6 != -1 )
          {
            v10 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v5);
            if ( v10 == -1 )
            {
              v24 = 0;
              v11 = 0;
            }
            else
            {
              v11 = *(char **)(DAT_055c9bcc + 4 * v10);
              v24 = v11;
            }
            v12 = v11[1412] - 1;
            v11[1412] = v12;
            if ( !v12 )
            {
              v13 = (char *)operator_new(0x584u);
              qmemcpy(v13, v5, 0x584u);
              v14 = 0;
              v15 = (BYTE *)v5 - v13;
              do
              {
                v16 = &v13[v14];
                v17 = ((v13[v14] + 71) ^ DAT_00559050[v14 & 0x8000000F]) + 35;
                v13[v14] = v17;
                if ( v14 < 0x583 )
                {
                  *v16 = v17 ^ v13[v14 + 1];
                }
                ++v14;
                v16[v15] = rand();
              }
              while ( v14 < 0x584 );
              qmemcpy(v24, v13, 0x584u);
              delete__(v13);
            }
          }
          break;
        }
        v6 = (v6 + 1) % DAT_055c9bd4;
        if ( ++v23 >= (unsigned int)DAT_055c9bd4 )
        {
          goto LABEL_15;
        }
      }
    }
    else
    {
LABEL_15:
      CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    }
    if ( v22 )
    {
      v18 = *v29;
      if ( (*v29 >= 128 && v18 < 135 || v18 == 145) && *v25 == -1 )
      {
        qmemcpy(&pPickedItem, &OffsetInventoryItems + v26, 0x44u);
        v19 = v26 + 12;
        *(DWORD *)&Inventory[32].Type = v26 + 12;
        UI_Main(v26 + 12, &OffsetInventoryItems.Type, 8u);
        DAT_07e11e78 = 0;
        SendRequestEquipmentItem(0, v19, 0, 0);
        UIChatLogWindow_AddText(DAT_07e11dec, GlobalText[250], 1);
      }
      else
      {
        v20 = *v25;
        if ( *v25 >= 136 && v20 < 143 || v20 >= 144 && v20 < 160 && v18 == -1 )
        {
          qmemcpy(&pPickedItem, &OffsetInventoryItems + v26, 0x44u);
          v21 = v26 + 12;
          *(DWORD *)&Inventory[32].Type = v26 + 12;
          UI_Main(v26 + 12, &OffsetInventoryItems.Type, 8u);
          DAT_07e11e78 = 1;
          SendRequestEquipmentItem(0, v21, 0, 1);
          UIChatLogWindow_AddText(DAT_07e11df0, GlobalText[250], 1);
        }
      }
    }
  }
}
#endif
