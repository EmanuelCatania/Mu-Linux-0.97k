// 0x00540AC0 FUN_00540ac0 — nunca activado: IDA_PORT_00540AC0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00540ac0 (IDA-only, gated) ──
#if defined(IDA_PORT_00540AC0)
int __cdecl FUN_00540ac0(void *_this)
{
  HCRYPTHASH v2; // eax
  HCRYPTKEY v3; // eax
  int result; // eax
  HCRYPTPROV v5; // esi

  if ( *((DWORD *)_this + 1) )
  {
    FUN_00543c98(*((LPVOID *)_this + 1));
  }
  if ( *((DWORD *)_this + 5) )
  {
    FUN_00543c98(*((LPVOID *)_this + 5));
  }
  v2 = *((DWORD *)_this + 3);
  if ( v2 )
  {
    CryptDestroyHash(v2);
  }
  v3 = *((DWORD *)_this + 4);
  if ( v3 )
  {
    CryptDestroyKey(v3);
  }
  if ( *((DWORD *)_this + 15) )
  {
    CryptDestroyKey(*((DWORD *)_this + 15));
  }
  result = *((DWORD *)_this + 16);
  if ( result )
  {
    result = CryptDestroyHash(*((DWORD *)_this + 16));
  }
  v5 = *(DWORD *)_this;
  if ( v5 )
  {
    return CryptReleaseContext(v5, 0);
  }
  return result;
}
#endif
