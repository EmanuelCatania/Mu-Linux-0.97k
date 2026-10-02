// 0x00405760 CErrorReport__WriteImeInfo — nunca activado: IDA_PORT_00405760 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CErrorReport__WriteImeInfo (IDA-only, gated) ──
#if defined(IDA_PORT_00405760)
void __cdecl CErrorReport::WriteImeInfo(DWORD This, HWND hWnd)
{
  HIMC hImc; // ebx
  HKL hKl; // edi
  char lpszTemp[256]; // [esp+Ch] [ebp-100h] BYREF

  CErrorReport::Write(This, aImeInformation);
  hImc = ImmGetContext(hWnd);
  if ( hImc )
  {
    hKl = GetKeyboardLayout(0);
    ImmGetDescriptionA(hKl, lpszTemp, 0x100u);
    CErrorReport::Write(This, "IME Name \t\t: %s\r\n", lpszTemp);
    ImmGetIMEFileNameA(hKl, lpszTemp, 0x100u);
    CErrorReport::Write(This, "IME File Name \t\t: %s\r\n", lpszTemp);
    ImmReleaseContext(hWnd, hImc);
  }
  GetKeyboardLayoutNameA(lpszTemp);
  CErrorReport::Write(This, "Keyboard type\t\t: %s\r\n", lpszTemp);
}
#endif
