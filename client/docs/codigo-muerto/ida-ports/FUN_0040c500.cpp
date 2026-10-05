// 0x0040C500 FUN_0040c500 — nunca activado: IDA_PORT_0040C500 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040c500 (IDA-only, gated) ──
#if defined(IDA_PORT_0040C500)
int __cdecl FUN_0040c500(DWORD *_this, int a2, int a3, int a4)
{
  DWORD *v5; // esi
  DWORD *v6; // edi
  DWORD *v7; // eax
  DWORD *v8; // ecx
  DWORD *v9; // ecx
  DWORD *v10; // eax
  int result; // eax

  DAT_055c9b50 = a2;
  DAT_055c9b54 = a3;
  DAT_055c9b58 = a4;
  v5 = (DWORD *)_this[2];
  v6 = (DWORD *)v5[1];
  v7 = (DWORD *)operator_new(0x14u);
  v8 = v5;
  if ( !v5 )
  {
    v8 = v7;
  }
  *v7 = v8;
  v9 = v6;
  if ( !v6 )
  {
    v9 = v7;
  }
  v7[1] = v9;
  v5[1] = v7;
  *(DWORD *)v7[1] = v7;
  v10 = v7 + 2;
  if ( v10 )
  {
    *v10 = DAT_055c9b50;
    v10[1] = DAT_055c9b54;
    v10[2] = DAT_055c9b58;
  }
  result = _this[3] + 1;
  _this[3] = result;
  return result;
}
#endif
