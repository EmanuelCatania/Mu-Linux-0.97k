// 0x0040C710 FUN_0040c710 — nunca activado: IDA_PORT_0040C710 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c710 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C710)
int __cdecl FUN_0040c710(DWORD *_this, int a2)
{
  int v4; // eax

  while ( _this[3] )
  {
    ChatListBox_DequeueFront((int)_this);
    if ( !(*(int (__cdecl **)(DWORD *))(*_this + 36))(_this) )
    {
      (*(void (__cdecl **)(DWORD *))(*_this + 32))(_this);
    }
  }
  (*(void (__cdecl **)(DWORD *, int))(*_this + 24))(_this, a2);
  if ( a2 == 1 )
  {
    return 0;
  }
  if ( !FUN_0040c490(_this[11], _this[12], _this[13], _this[14], _this[21]) )
  {
    return (*(int (__cdecl **)(DWORD *))(*_this + 28))(_this);
  }
  if ( !DAT_055c9b80 )
  {
    if ( DAT_055c9b7c && DAT_055c9b7c != _this[7] )
    {
      return 0;
    }
    DAT_055c9b80 = _this[7];
  }
  if ( FUN_0040c680(_this) )
  {
    return (*(int (__cdecl **)(DWORD *))(*_this + 28))(_this);
  }
  v4 = _this[7];
  if ( DAT_055c9b80 == v4 || DAT_055c9b7c == v4 )
  {
    return (*(int (__cdecl **)(DWORD *))(*_this + 28))(_this);
  }
  else
  {
    return 0;
  }
}
#endif
