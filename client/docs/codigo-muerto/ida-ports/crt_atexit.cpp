// 0x005414CE crt_atexit — nunca activado: IDA_PORT_005414CE nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_atexit (IDA-only, gated) ──
#if defined(IDA_PORT_005414CE)
int __cdecl crt_atexit(void (__cdecl *Func)())
{
  return (_onexit((_onexit_t)Func) != 0) - 1;
}
#endif
