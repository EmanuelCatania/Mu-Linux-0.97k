// 0x004056B0 CErrorReport__WriteOpenGLInfo — nunca activado: IDA_PORT_004056B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CErrorReport__WriteOpenGLInfo (IDA-only, gated) ──
#if defined(IDA_PORT_004056B0)
void __cdecl CErrorReport::WriteOpenGLInfo(DWORD This)
{
  const char *String; // eax MAPDST
  GLint iResult[2]; // [esp+8h] [ebp-8h] BYREF

  CErrorReport::Write(This, aOpenglInformat);
  String = (const char *)glGetString(0x1F00u);
  CErrorReport::Write(This, "Vendor \t\t\t: %s\r\n", String);
  String = (const char *)glGetString(0x1F01u);
  CErrorReport::Write(This, "Render \t\t\t: %s\r\n", String);
  String = (const char *)glGetString(0x1F02u);
  CErrorReport::Write(This, "OpenGL version \t\t: %s\r\n", String);
  glGetIntegerv(0xD33u, iResult);
  CErrorReport::Write(This, "Max Texture size \t: %d x %d\r\n", iResult[0], iResult[0]);
  glGetIntegerv(0xD3Au, iResult);
  CErrorReport::Write(This, "Max Viewport size \t: %d x %d\r\n", iResult[0], iResult[1]);
}
#endif
