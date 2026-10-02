// 0x00409DB0 LinkedList_DestroyAll — nunca activado: IDA_PORT_00409DB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── LinkedList_DestroyAll (IDA-only, gated) ──
#if defined(IDA_PORT_00409DB0)
int __cdecl FUN_00409d20(DWORD *_this)
{
  int *i; // esi
  DWORD *v3; // esi
  void *j; // eax
  int result; // eax

  for ( i = *(int **)(_this[2] + 8); (int *)_this[3] != i && i; i = (int *)i[2] )
  {
    Widget_Release(*i);
    if ( *i )
    {
      (**(void (__cdecl ***)(int, int))*i)(*i, 1);
    }
  }
  *(DWORD *)(*(DWORD *)(_this[3] + 4) + 8) = 0;
  v3 = *(DWORD **)(_this[2] + 8);
  for ( j = v3; v3; j = v3 )
  {
    v3 = (DWORD *)v3[2];
    if ( j )
    {
      delete__(j);
    }
  }
  *(DWORD *)(_this[2] + 8) = _this[3];
  result = _this[2];
  *(DWORD *)(_this[3] + 4) = result;
  _this[1] = 0;
  return result;
}
#endif
