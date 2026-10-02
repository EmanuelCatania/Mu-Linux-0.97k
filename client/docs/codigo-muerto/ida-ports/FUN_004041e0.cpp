// 0x004041E0 FUN_004041e0 — nunca activado: IDA_PORT_004041E0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004041e0 (IDA-only, gated) ──
#if defined(IDA_PORT_004041E0)
unsigned int __cdecl FUN_004041e0(DWORD *_this, int a2)
{
  unsigned int result; // eax
  unsigned int v4; // ebp
  bool v5; // cf
  int v6; // [esp+10h] [ebp-Ch]
  int v7; // [esp+14h] [ebp-8h]
  int v8; // [esp+18h] [ebp-4h] BYREF

  result = (*(int (__cdecl **)(DWORD *, int))(*_this + 12))(_this, a2);
  v4 = _this[3];
  v8 = 0;
  v6 = 0;
  if ( v4 )
  {
    v7 = _this[2];
    while ( memcmp((const char *)&v8, (const char *)(v7 + 4 * result), 4) )
    {
      if ( !memcmp((const char *)&a2, (const char *)(v7 + 4 * result), 4) )
      {
        return result;
      }
      v5 = ++v6 < v4;
      result = (result + 1) % v4;
      if ( !v5 )
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
  return -1;
}
#endif
