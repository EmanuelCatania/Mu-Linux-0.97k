// 0x00422C50 FUN_00422c50 — nunca activado: IDA_PORT_00422C50 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00422c50 (IDA-only, gated) ──
#if defined(IDA_PORT_00422C50)
void __cdecl FUN_00422c50(int _this)
{
  LPVOID *v2; // ecx
  DWORD *v3; // esi
  int v4; // eax
  int *v5; // ecx
  int v6; // edi
  int v7; // ebx
  DWORD *v8; // edx
  int v9; // eax
  LPVOID lpMem; // [esp+8h] [ebp-8h] BYREF
  int v11; // [esp+Ch] [ebp-4h] BYREF

  if ( *(DWORD *)(_this + 1020) )
  {
    delete__(*(LPVOID *)(_this + 1020));
    *(DWORD *)(_this + 1020) = 0;
  }
  if ( *(DWORD *)(_this + 1032) )
  {
    delete__(*(LPVOID *)(_this + 1032));
    *(DWORD *)(_this + 1032) = 0;
  }
  if ( *(DWORD *)(_this + 1036) )
  {
    delete__(*(LPVOID *)(_this + 1036));
    *(DWORD *)(_this + 1036) = 0;
  }
  if ( *(DWORD *)(_this + 1040) )
  {
    delete__(*(LPVOID *)(_this + 1040));
    *(DWORD *)(_this + 1040) = 0;
  }
  *(DWORD *)(_this + 1024) = 1950000000;
  *(DWORD *)(_this + 1028) = -1;
  v2 = *(LPVOID **)(_this + 1052);
  v3 = (DWORD *)(_this + 1044);
  *v3 = &DAT_00552840;
  if ( !v2 )
  {
    v3[1] = 0;
    return;
  }
  lpMem = v2;
  if ( v2[2] )
  {
    CBTree_RemoveFrom(v2[2]);
    v2 = (LPVOID *)lpMem;
  }
  if ( v2[3] )
  {
    CBTree_RemoveFrom(v2[3]);
    v2 = (LPVOID *)lpMem;
  }
  v11 = 0;
  v4 = (int)v2[2];
  if ( !v4 )
  {
    v4 = (int)v2[3];
    if ( !v4 )
    {
      FUN_004236c0(&lpMem);
      v3[2] = 0;
      v3[1] = 0;
      return;
    }
    if ( v2 == (LPVOID *)v3[2] )
    {
      v3[2] = v4;
      *(DWORD *)(v4 + 16) = 0;
      goto LABEL_27;
    }
LABEL_23:
    v8 = v2[4];
    if ( v2 == (LPVOID *)v8[3] )
    {
      v8[3] = v4;
    }
    else
    {
      v8[2] = v4;
    }
    *(DWORD *)(v4 + 16) = v8;
    goto LABEL_27;
  }
  if ( !v2[3] )
  {
    if ( v2 == (LPVOID *)v3[2] )
    {
      v3[2] = v4;
      *(DWORD *)(v4 + 16) = 0;
LABEL_27:
      if ( lpMem )
      {
        delete__(lpMem);
      }
      v9 = v3[1];
      v3[2] = 0;
      v3[1] = v9 - 1;
      v3[1] = 0;
      return;
    }
    goto LABEL_23;
  }
  do
  {
    v5 = (int *)v4;
    v11 = v4;
    v4 = *(DWORD *)(v4 + 12);
  }
  while ( v4 );
  v6 = *v5;
  v7 = v5[1];
  CBTree_RemoveNode(&v11);
  *(DWORD *)lpMem = v6;
  *((DWORD *)lpMem + 1) = v7;
  v3[2] = 0;
  v3[1] = 0;
}
#endif
