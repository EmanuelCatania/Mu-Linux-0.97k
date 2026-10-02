// 0x0047FED0 FUN_0047fed0 — nunca activado: IDA_PORT_0047FED0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0047fed0 (IDA-only, gated) ──
#if defined(IDA_PORT_0047FED0)
char __cdecl FUN_0047fed0(int a1, const char *a2)
{
  void *v2; // ebx
  unsigned int v3; // eax
  bool v4; // cf
  int v5; // eax
  const void *v6; // ebx
  unsigned int v7; // edx
  unsigned int v8; // eax
  BYTE *v9; // esi
  unsigned char v10; // al
  void *v11; // ebp
  unsigned int v12; // ecx
  int v13; // esi
  unsigned int v14; // eax
  BYTE *v15; // eax
  char v16; // cl
  char (*v18)[10]; // edi
  int v19; // [esp+10h] [ebp-10h]
  int v20; // [esp+10h] [ebp-10h]
  DWORD v21; // [esp+14h] [ebp-Ch] BYREF
  DWORD v22; // [esp+18h] [ebp-8h] BYREF
  int v23; // [esp+1Ch] [ebp-4h]

  v2 = (void *)CharacterMachine;
  v22 = CharacterMachine;
  v3 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v21 = 0;
  v19 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v3), 4) )
    {
      if ( !memcmp((const char *)&v22, (const char *)(DAT_055c9bd0 + 4 * v3), 4) )
      {
        if ( v3 == -1 )
        {
          break;
        }
        v8 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v2);
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
          v11 = (void *)operator_new(0x584u);
          qmemcpy(v11, v9, 0x584u);
          v12 = 1411;
          v13 = 1412;
          do
          {
            if ( v12 < 0x583 )
            {
              *((BYTE *)v11 + v12) ^= *((BYTE *)v11 + v12 + 1);
            }
            *((BYTE *)v11 + v12) = ((*((BYTE *)v11 + v12) - 35) ^ DAT_00559050[(int)v12 % 16]) - 71;
            --v12;
            --v13;
          }
          while ( v13 );
          qmemcpy(v2, v11, 0x584u);
          delete__(v11);
        }
        goto LABEL_7;
      }
      v4 = ++v19 < (unsigned int)DAT_055c9bd4;
      v3 = (v3 + 1) % DAT_055c9bd4;
      if ( !v4 )
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
  v5 = operator_new(0x585u);
  *(BYTE *)(v5 + 1412) = 1;
  HashTable_Insert(&MAIN_HASH_CLASS, v5, (int)v2);
LABEL_7:
  v6 = (const void *)CharacterMachine;
  v23 = *(unsigned short *)(CharacterAttribute + 14);
  v21 = CharacterMachine;
  v7 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
  v22 = 0;
  v20 = 0;
  if ( DAT_055c9bd4 )
  {
    while ( memcmp((const char *)&v22, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
    {
      if ( !memcmp((const char *)&v21, (const char *)(DAT_055c9bd0 + 4 * v7), 4) )
      {
        if ( v7 != -1 )
        {
          v14 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v6);
          if ( v14 == -1 )
          {
            v15 = 0;
          }
          else
          {
            v15 = *(BYTE **)(DAT_055c9bcc + 4 * v14);
          }
          v16 = v15[1412] - 1;
          v15[1412] = v16;
          if ( !v16 )
          {
            Packet_EncryptBuffer(v15, v6);
          }
        }
        break;
      }
      v7 = (v7 + 1) % DAT_055c9bd4;
      if ( ++v20 >= (unsigned int)DAT_055c9bd4 )
      {
        goto LABEL_11;
      }
    }
  }
  else
  {
LABEL_11:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
  }
  if ( v23 < a1 )
  {
    v18 = WhisperRegistID;
    while ( strcmp(a2, (const char *)v18) )
    {
      if ( (int)++v18 >= (int)WhisperRegistID[10] )
      {
        UIChatLogWindow_AddText(DAT_07e11dd4, GlobalText[479], 1);
        return 0;
      }
    }
  }
  return 1;
}
#endif
