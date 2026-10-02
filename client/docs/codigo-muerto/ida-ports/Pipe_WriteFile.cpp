// 0x005403A0 Pipe_WriteFile — nunca activado: IDA_PORT_005403A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Pipe_WriteFile (IDA-only, gated) ──
#if defined(IDA_PORT_005403A0)
bool __cdecl Pipe_WriteFile(int a1, int a2, LPCSTR lpString)
{
  bool result; // al
  int v4; // eax
  BOOL v5; // eax
  DWORD LastError; // eax
  DWORD NumberOfBytesWritten; // [esp+0h] [ebp-84h] BYREF
  int Buffer[4]; // [esp+4h] [ebp-80h] BYREF
  char v9[112]; // [esp+14h] [ebp-70h] BYREF

  if ( hFile == (HANDLE)-1 )
  {
    return 0;
  }
  DAT_083bbb60 = a1;
  *(DWORD *)DAT_083bbb68 = a2;
  DAT_083bbb64 = DAT_083bbb74;
  ::lpString = (DWORD)lpString;
  if ( a2 == 1555 )
  {
    v4 = lstrlenA(lpString);
    Buffer[0] = DAT_083bbb60;
    ++v4;
    Buffer[2] = *(DWORD *)DAT_083bbb68;
    Buffer[1] = DAT_083bbb64;
    ::lpString = v4;
    Buffer[3] = v4;
    qmemcpy(v9, lpString, v4);
    v5 = WriteFile(hFile, Buffer, v4 + 16, &NumberOfBytesWritten, 0);
  }
  else
  {
    v5 = WriteFile(hFile, &DAT_083bbb60, 0x10u, &NumberOfBytesWritten, 0);
  }
  result = 1;
  if ( !v5 )
  {
    LastError = GetLastError();
    if ( LastError == 109 || LastError == 232 || LastError == 6 )
    {
      return 0;
    }
  }
  return result;
}
#endif
