// 0x00409310 Cloth_CollideAnchors — nunca activado: IDA_PORT_00409310 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Cloth_CollideAnchors (IDA-only, gated) ──
#if defined(IDA_PORT_00409310)
int __cdecl Cloth_CollideAnchors(DWORD *_this)
{
  int result; // eax
  DWORD *v3; // eax
  int v4; // ebx
  int v5; // edi
  int v6; // ebp
  int v7; // edi
  int v8; // ebx
  DWORD *i; // [esp+Ch] [ebp-4h]

  result = _this[18];
  if ( result > 0 )
  {
    v3 = *(DWORD **)(_this[19] + 8);
    if ( v3 != (DWORD *)_this[20] )
    {
      for ( i = *(DWORD **)(_this[19] + 8); v3; i = v3 )
      {
        v4 = *v3;
        v5 = 0;
        if ( (int)_this[12] > 0 )
        {
          v6 = 0;
          do
          {
            (*(void (__cdecl **)(int, int))(*(DWORD *)v4 + 8))(v4, v6 + _this[13]);
            ++v5;
            v6 += 60;
          }
          while ( v5 < _this[12] );
          v3 = i;
        }
        v3 = (DWORD *)v3[2];
        if ( (DWORD *)_this[20] == v3 )
        {
          break;
        }
      }
    }
    result = _this[12];
    v7 = 0;
    if ( result > 0 )
    {
      v8 = 0;
      do
      {
        VerletSystem_Flush(v8 + _this[13]);
        result = _this[12];
        ++v7;
        v8 += 60;
      }
      while ( v7 < result );
    }
  }
  return result;
}
#endif
