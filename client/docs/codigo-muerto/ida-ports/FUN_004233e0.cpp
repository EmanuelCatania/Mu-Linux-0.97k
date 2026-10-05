// 0x004233E0 FUN_004233e0 — nunca activado: IDA_PORT_004233E0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004233e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004233E0)
void __cdecl FUN_004233e0(int *_this, char *a2)
{
  int v3; // edx
  int v4; // eax
  int v5; // ecx
  unsigned int v6; // edx
  unsigned int v7; // ecx
  int v8; // edx
  unsigned int v9; // edx
  int v10; // eax
  unsigned int v11; // ecx
  int v12; // edx
  char v13; // al
  DWORD *v14; // ebx
  unsigned int v15; // edi
  BYTE *v16; // esi
  char v17; // al
  char v18; // al
  int v19; // [esp+10h] [ebp-Ch]
  int v20; // [esp+10h] [ebp-Ch]
  DWORD *v21; // [esp+10h] [ebp-Ch]
  char *v22; // [esp+14h] [ebp-8h] BYREF
  char *v23; // [esp+18h] [ebp-4h] BYREF

  v3 = *_this;
  v23 = a2;
  v4 = (*(int (__cdecl **)(int *, char *))(v3 + 12))(_this, a2);
  v5 = _this[3];
  v6 = v4;
  v22 = 0;
  v19 = 0;
  if ( v5 )
  {
    while ( memcmp((const char *)&v22, (const char *)(_this[2] + 4 * v6), 4) )
    {
      if ( !memcmp((const char *)&v23, (const char *)(_this[2] + 4 * v6), 4) )
      {
        if ( v6 != -1 )
        {
          v8 = *_this;
          v22 = a2;
          v9 = (*(int (__cdecl **)(int *, char *))(v8 + 12))(_this, a2);
          v10 = _this[3];
          v23 = 0;
          v20 = 0;
          if ( v10 )
          {
            while ( memcmp((const char *)&v23, (const char *)(_this[2] + 4 * v9), 4) )
            {
              if ( !memcmp((const char *)&v22, (const char *)(_this[2] + 4 * v9), 4) )
              {
                if ( v9 == -1 )
                {
                  break;
                }
                v12 = *(DWORD *)(_this[1] + 4 * v9);
                v21 = (DWORD *)v12;
                goto LABEL_13;
              }
              v11 = _this[3];
              v9 = (v9 + 1) % v11;
              if ( ++v20 >= v11 )
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
          v21 = 0;
          v12 = 0;
LABEL_13:
          v13 = *(BYTE *)(v12 + 4) - 1;
          *(BYTE *)(v12 + 4) = v13;
          if ( !v13 )
          {
            v14 = (DWORD *)operator_new(4u);
            v15 = 0;
            *v14 = *(DWORD *)a2;
            v16 = v14;
            do
            {
              v17 = *v16 + 71;
              *v16 = v17;
              v18 = (v17 ^ DAT_00559050[v15 & 0x8000000F]) + 35;
              *v16 = v18;
              if ( v15 < 3 )
              {
                *v16 = v18 ^ *((BYTE *)v14 + v15 + 1);
              }
              v16[a2 - (char *)v14] = rand();
              ++v15;
              ++v16;
            }
            while ( v15 < 4 );
            *v21 = *v14;
            delete__(v14);
          }
        }
        return;
      }
      v7 = _this[3];
      v6 = (v6 + 1) % v7;
      if ( ++v19 >= v7 )
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
}
#endif
