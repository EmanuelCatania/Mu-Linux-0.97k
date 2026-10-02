// 0x00405340 FUN_00405340 — nunca activado: IDA_PORT_00405340 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00405340 (IDA-only, gated) ──
#if defined(IDA_PORT_00405340)
char *__cdecl FUN_00405340(DWORD _this)
{
  char *v2; // edi
  char *result; // eax
  HANDLE FileA; // eax
  DWORD v5; // ecx
  int v6; // [esp-4h] [ebp-20010h]
  DWORD NumberOfBytesRead; // [esp+8h] [ebp-20004h] BYREF
  char Buffer[32767]; // [esp+Ch] [ebp-20000h] BYREF
  char v9; // [esp+800Bh] [ebp-18001h] BYREF

  ReadFile(*(HANDLE *)(_this + 4), Buffer, 0x1FFFFu, &NumberOfBytesRead, 0);
  *(DWORD *)(_this + 268) = Xor_ConvertBuffer(Buffer, NumberOfBytesRead, 0);
  v6 = NumberOfBytesRead;
  Buffer[NumberOfBytesRead] = 0;
  v2 = (char *)FUN_00405420(Buffer, v6);
  if ( NumberOfBytesRead < 0x7FFF )
  {
    result = Buffer;
    if ( v2 == Buffer )
    {
      return result;
    }
  }
  else
  {
    v2 = &v9;
  }
  CloseHandle(*(HANDLE *)(_this + 4));
  DeleteFileA((LPCSTR)(_this + 8));
  FileA = CreateFileA((LPCSTR)(_this + 8), 0xC0000000, 1u, 0, 4u, 0x80u, 0);
  v5 = NumberOfBytesRead - (DWORD)v2;
  *(DWORD *)(_this + 4) = FileA;
  *(DWORD *)(_this + 268) = 0;
  return (char *)CErrorReport::WriteFile(_this, FileA, v2, (DWORD)&Buffer[v5], &NumberOfBytesRead, 0);
}
#endif
