// 0x004042B0 FUN_004042b0 — nunca activado: IDA_PORT_004042B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004042b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004042B0)
int __cdecl HashTable_GetNode(int *_this, int a2)
{
  int v3; // edx
  int v4; // eax
  int v5; // ecx
  unsigned int v6; // edx
  unsigned int v7; // ecx
  int v9; // [esp+10h] [ebp-8h] BYREF
  int v10; // [esp+14h] [ebp-4h] BYREF
  int v11; // [esp+1Ch] [ebp+4h]

  v3 = *_this;
  v10 = a2;
  v4 = (*(int (__cdecl **)(int *, int))(v3 + 12))(_this, a2);
  v5 = _this[3];
  v6 = v4;
  v9 = 0;
  v11 = 0;
  if ( v5 )
  {
    while ( memcmp((const char *)&v9, (const char *)(_this[2] + 4 * v6), 4) )
    {
      if ( !memcmp((const char *)&v10, (const char *)(_this[2] + 4 * v6), 4) )
      {
        if ( v6 == -1 )
        {
          return 0;
        }
        return *(DWORD *)(_this[1] + 4 * v6);
      }
      v7 = _this[3];
      v6 = (v6 + 1) % v7;
      if ( ++v11 >= v7 )
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
  return 0;
}
#endif
