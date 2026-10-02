// 0x005406F0 FUN_005406f0 — nunca activado: IDA_PORT_005406F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_005406f0 (IDA-only, gated) ──
#if defined(IDA_PORT_005406F0)
char __cdecl FUN_005406f0(HANDLE *_this)
{
  HANDLE v3; // eax

  if ( hFile == (HANDLE)-1 )
  {
    return 0;
  }
  Pipe_Write(2225, 1552, 0);
  if ( hEvent )
  {
    SetEvent(hEvent);
  }
  v3 = _this[6];
  if ( v3 )
  {
    if ( WaitForSingleObject(v3, 0x64u) == 258 )
    {
      TerminateThread(_this[6], 0);
    }
    CloseHandle(_this[6]);
    _this[6] = 0;
  }
  if ( hFile != (HANDLE)-1 )
  {
    CloseHandle(hFile);
    hFile = (HANDLE)-1;
  }
  if ( hNamedPipe != (HANDLE)-1 )
  {
    CloseHandle(hNamedPipe);
    hNamedPipe = (HANDLE)-1;
  }
  return 1;
}
#endif
