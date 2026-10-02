// 0x0054385B crt_doexit — nunca activado: IDA_PORT_0054385B nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_doexit (IDA-only, gated) ──
#if defined(IDA_PORT_0054385B)
int __cdecl crt_doexit(UINT uExitCode, int a2, int a3)
{
  HANDLE CurrentProcess; // eax
  void (**v4)(void); // esi

  _lockexit();
  if ( DAT_083bbbf8 == 1 )
  {
    CurrentProcess = GetCurrentProcess();
    TerminateProcess(CurrentProcess, uExitCode);
  }
  DAT_083bbbf4 = 1;
  DAT_083bbbf0 = a3;
  if ( !a2 )
  {
    if ( DAT_083bd2d0 )
    {
      v4 = (void (**)(void))(DAT_083bd2cc - 4);
      if ( DAT_083bd2cc - 4 >= (unsigned int)DAT_083bd2d0 )
      {
        do
        {
          if ( *v4 )
          {
            (*v4)();
          }
          --v4;
        }
        while ( v4 >= DAT_083bd2d0 );
      }
    }
    _initterm(&DAT_00558070, &DAT_0055807c);
  }
  _initterm(&DAT_00558080, &DAT_00558088);
  if ( !a3 )
  {
    DAT_083bbbf8 = 1;
    ExitProcess(uExitCode);
  }
  return _unlockexit();
}
#endif
