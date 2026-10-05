// 0x00541450 crt_onexit — nunca activado: IDA_PORT_00541450 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_onexit (IDA-only, gated) ──
#if defined(IDA_PORT_00541450)
_onexit_t __cdecl _onexit(_onexit_t Func)
{
  unsigned int v1; // eax
  _onexit_t *v2; // ecx
  int v3; // eax
  char *v4; // eax
  int (__cdecl *v5)(); // esi
  int v6; // ecx

  _lockexit();
  v1 = FUN_00544832(DAT_083bd2d0);
  v2 = (_onexit_t *)DAT_083bd2cc;
  if ( v1 >= DAT_083bd2cc - (int)DAT_083bd2d0 + 4 )
  {
    goto LABEL_5;
  }
  v3 = FUN_00544832(DAT_083bd2d0);
  v4 = (char *)FUN_00544503((LPVOID)DAT_083bd2d0, v3 + 16);
  if ( v4 )
  {
    v6 = DAT_083bd2cc - (DWORD)DAT_083bd2d0;
    DAT_083bd2d0 = v4;
    v2 = (_onexit_t *)&v4[4 * (v6 >> 2)];
    DAT_083bd2cc = (int)v2;
LABEL_5:
    *v2 = Func;
    DAT_083bd2cc += 4;
    v5 = Func;
    goto LABEL_6;
  }
  v5 = 0;
LABEL_6:
  _unlockexit();
  return v5;
}
#endif
