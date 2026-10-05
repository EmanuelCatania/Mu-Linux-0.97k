// 0x00405500 CErrorReport__WriteDebugInfoStr — nunca activado: IDA_PORT_00405500 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CErrorReport__WriteDebugInfoStr (IDA-only, gated) ──
#if defined(IDA_PORT_00405500)
void __cdecl CErrorReport::WriteDebugInfoStr(DWORD This, char *lpszToWrite)
{
  HANDLE m_hFile; // esi

  m_hFile = *(HANDLE *)(This + 4);
  if ( m_hFile != (HANDLE)-1 )
  {
    CErrorReport::WriteFile(This, m_hFile, lpszToWrite, strlen(lpszToWrite), (LPDWORD)&lpszToWrite, 0);
  }
}
#endif
