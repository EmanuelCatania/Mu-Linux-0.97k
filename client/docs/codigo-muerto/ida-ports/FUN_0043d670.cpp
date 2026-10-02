// 0x0043D670 FUN_0043d670 — nunca activado: IDA_PORT_0043D670 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0043d670 (IDA-only, gated) ──
#if defined(IDA_PORT_0043D670)
int __cdecl FUN_0043d670(int *_this, char *a2)
{
  char *v3; // esi
  int v4; // eax
  int v5; // eax
  int v6; // ecx
  unsigned int v7; // edx
  unsigned int v8; // ecx
  unsigned int v9; // eax
  int v10; // edi
  unsigned char v11; // al
  DWORD *v12; // esi
  unsigned int v13; // ecx
  int v14; // edi
  int v15; // edx
  int v16; // eax
  int v17; // ecx
  unsigned int v18; // ebx
  unsigned int v19; // ecx
  bool v20; // cf
  int v22; // eax
  unsigned int v23; // eax
  int v24; // eax
  char v25; // cl
  int v26; // [esp+10h] [ebp-10h]
  int v27; // [esp+10h] [ebp-10h]
  int v28; // [esp+14h] [ebp-Ch] BYREF
  char *v29; // [esp+18h] [ebp-8h] BYREF
  char *v30; // [esp+1Ch] [ebp-4h] BYREF

  v3 = a2;
  v4 = *_this;
  v29 = a2;
  v5 = (*(int (__cdecl **)(int *, char *))(v4 + 12))(_this, a2);
  v6 = _this[3];
  v7 = v5;
  v28 = 0;
  v26 = 0;
  if ( v6 )
  {
    while ( 1 )
    {
      if ( !memcmp((const char *)&v28, (const char *)(_this[2] + 4 * v7), 4) )
      {
        goto LABEL_23;
      }
      if ( !memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v7), 4) )
      {
        break;
      }
      v8 = _this[3];
      v7 = (v7 + 1) % v8;
      if ( ++v26 >= v8 )
      {
        v3 = a2;
        goto LABEL_6;
      }
    }
    if ( v7 == -1 )
    {
LABEL_23:
      v3 = a2;
      goto LABEL_24;
    }
    v9 = FUN_004041e0(_this, (int)a2);
    if ( v9 == -1 )
    {
      v10 = 0;
    }
    else
    {
      v10 = *(DWORD *)(_this[1] + 4 * v9);
    }
    v11 = *(BYTE *)(v10 + 4) + 1;
    *(BYTE *)(v10 + 4) = v11;
    if ( v11 < 2u )
    {
      v12 = (DWORD *)operator_new(4u);
      v13 = 3;
      *v12 = *(DWORD *)v10;
      v14 = 4;
      do
      {
        if ( v13 < 3 )
        {
          *((BYTE *)v12 + v13) ^= *((BYTE *)v12 + v13 + 1);
        }
        *((BYTE *)v12 + v13) = ((*((BYTE *)v12 + v13) - 35) ^ DAT_00559050[(int)v13 % 16]) - 71;
        --v13;
        --v14;
      }
      while ( v14 );
      *(DWORD *)a2 = *v12;
      delete__(v12);
    }
    v3 = a2;
  }
  else
  {
LABEL_6:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
LABEL_24:
    v22 = operator_new(5u);
    *(BYTE *)(v22 + 4) = 1;
    HashTable_Insert(_this, v22, (int)v3);
  }
  v15 = *_this;
  v27 = *(DWORD *)v3;
  v30 = v3;
  v16 = (*(int (__cdecl **)(int *, char *))(v15 + 12))(_this, v3);
  v17 = _this[3];
  v18 = v16;
  v29 = 0;
  v28 = 0;
  if ( v17 )
  {
    while ( memcmp((const char *)&v29, (const char *)(_this[2] + 4 * v18), 4) )
    {
      if ( !memcmp((const char *)&v30, (const char *)(_this[2] + 4 * v18), 4) )
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
            v24 = *(DWORD *)(_this[1] + 4 * v23);
          }
          v25 = *(BYTE *)(v24 + 4) - 1;
          *(BYTE *)(v24 + 4) = v25;
          if ( !v25 )
          {
            Packet_EncryptDword((DWORD *)v24, a2);
          }
        }
        return v27;
      }
      v19 = _this[3];
      v20 = ++v28 < v19;
      v18 = (v18 + 1) % v19;
      if ( !v20 )
      {
        goto LABEL_22;
      }
    }
    return v27;
  }
  else
  {
LABEL_22:
    CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
    return v27;
  }
}
#endif
