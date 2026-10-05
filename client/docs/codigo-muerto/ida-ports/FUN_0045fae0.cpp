// 0x0045FAE0 FUN_0045fae0 — nunca activado: IDA_PORT_0045FAE0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0045fae0 (IDA-only, gated) ──
#if defined(IDA_PORT_0045FAE0)
char __cdecl FUN_0045fae0(int *_this, BYTE *a2)
{
  BYTE *v3; // esi
  int v4; // eax
  int v5; // eax
  int v6; // ecx
  unsigned int v7; // edx
  unsigned int v8; // ecx
  unsigned int v9; // eax
  char *v10; // esi
  unsigned char v11; // al
  char *v12; // eax
  char v13; // cl
  char v14; // cl
  int v15; // edx
  int v16; // eax
  int v17; // ecx
  unsigned int v18; // ebx
  unsigned int v19; // ecx
  bool v20; // cf
  int v22; // eax
  unsigned int v23; // eax
  BYTE *v24; // eax
  char v25; // cl
  char v26; // [esp+13h] [ebp-Dh]
  int v27; // [esp+14h] [ebp-Ch]
  int v28; // [esp+14h] [ebp-Ch]
  BYTE *v29; // [esp+18h] [ebp-8h] BYREF
  BYTE *v30; // [esp+1Ch] [ebp-4h] BYREF

  v3 = a2;
  v4 = *_this;
  v30 = a2;
  v5 = (*(int (__cdecl **)(int *, BYTE *))(v4 + 12))(_this, a2);
  v6 = _this[3];
  v7 = v5;
  v29 = 0;
  v27 = 0;
  if ( v6 )
  {
    while ( 1 )
    {
      if ( !memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v7), 4) )
      {
        goto LABEL_19;
      }
      if ( !memcmp((const char *)&v30, (const char *)(_this[2] + 4 * v7), 4) )
      {
        break;
      }
      v8 = _this[3];
      v7 = (v7 + 1) % v8;
      if ( ++v27 >= v8 )
      {
        v3 = a2;
        goto LABEL_6;
      }
    }
    if ( v7 == -1 )
    {
LABEL_19:
      v3 = a2;
      goto LABEL_20;
    }
    v9 = FUN_004041e0(_this, (int)a2);
    if ( v9 == -1 )
    {
      v10 = 0;
    }
    else
    {
      v10 = *(char **)(_this[1] + 4 * v9);
    }
    v11 = v10[1] + 1;
    v10[1] = v11;
    if ( v11 < 2u )
    {
      v12 = (char *)operator_new(1u);
      v13 = *v10;
      *v12 = *v10;
      v13 -= 35;
      *v12 = v13;
      v14 = (DAT_00559050[0] ^ v13) - 71;
      *v12 = v14;
      *a2 = v14;
      delete__(v12);
    }
    v3 = a2;
  }
  else
  {
LABEL_6:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
LABEL_20:
    v22 = operator_new(2u);
    *(BYTE *)(v22 + 1) = 1;
    HashTable_Insert(_this, v22, (int)v3);
  }
  v15 = *_this;
  v26 = *v3;
  v29 = v3;
  v16 = (*(int (__cdecl **)(int *, BYTE *))(v15 + 12))(_this, v3);
  v17 = _this[3];
  v18 = v16;
  v30 = 0;
  v28 = 0;
  if ( v17 )
  {
    while ( memcmp((const char *)&v30, (const char *)(_this[2] + 4 * v18), 4) )
    {
      if ( !memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v18), 4) )
      {
        if ( v18 != -1 )
        {
          v23 = FUN_004041e0(_this, (int)a2);
          if ( v23 == -1 )
          {
            v24 = 0;
          }
          else
          {
            v24 = *(BYTE **)(_this[1] + 4 * v23);
          }
          v25 = v24[1] - 1;
          v24[1] = v25;
          if ( !v25 )
          {
            Packet_EncryptByte(v24, a2);
          }
        }
        return v26;
      }
      v19 = _this[3];
      v20 = ++v28 < v19;
      v18 = (v18 + 1) % v19;
      if ( !v20 )
      {
        goto LABEL_18;
      }
    }
    return v26;
  }
  else
  {
LABEL_18:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    return v26;
  }
}
#endif
