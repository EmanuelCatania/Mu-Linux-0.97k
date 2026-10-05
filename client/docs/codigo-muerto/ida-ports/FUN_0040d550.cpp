// 0x0040D550 FUN_0040d550 — nunca activado: IDA_PORT_0040D550 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040d550 (IDA-only, gated) ──
#if defined(IDA_PORT_0040D550)
void __cdecl FUN_0040d550(void *_this)
{
  DWORD **v2; // ebx
  DWORD *i; // edi
  DWORD **v4; // eax
  DWORD **v5; // ebx
  DWORD *j; // edi
  DWORD **v7; // [esp-4h] [ebp-18h]
  char v8[4]; // [esp+10h] [ebp-4h] BYREF

  *(DWORD *)_this = DAT_005525c8;
  v2 = (DWORD **)*((DWORD *)_this + 2);
  for ( i = *v2; i != v2; --*((DWORD *)_this + 3) )
  {
    v4 = (DWORD **)i;
    i = (DWORD *)*i;
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
  }
  v5 = (DWORD **)*((DWORD *)_this + 2);
  for ( j = *v5; j != v5; --*((DWORD *)_this + 3) )
  {
    j = (DWORD *)*j;
    v7 = *(DWORD ***)FUN_00410e30(v8, 0);
    *v7[1] = *v7;
    (*v7)[1] = v7[1];
    delete__(v7);
  }
  delete__(*((LPVOID *)_this + 2));
  *((DWORD *)_this + 2) = 0;
  *((DWORD *)_this + 3) = 0;
}
#endif
