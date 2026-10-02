// 0x00479830 LoadGlobalTextBin — nunca activado: IDA_PORT_00479830 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── LoadGlobalTextBin (IDA-only, gated) ──
#if defined(IDA_PORT_00479830)
int __cdecl LoadGlobalTextBin(char *FileName)
{
  FILE *v1; // ebp
  void *v3; // ebx
  char (*v4)[300]; // eax
  char *v5; // esi
  char (*v6)[300]; // edi
  CHAR Text[256]; // [esp+8h] [ebp-100h] BYREF

  v1 = fopen(FileName, "rb");
  if ( v1 )
  {
    v3 = (void *)operator_new(0x493E0u);
    fread(v3, 0x493E0u, 1u, v1);
    BuxConvert_0((BYTE *)v3, 300000);
    v4 = GlobalText;
    do
    {
      v5 = &(*v4)[(BYTE *)v3 - (BYTE *)GlobalText];
      v6 = v4++;
      qmemcpy(v6, v5, sizeof(char[300]));
    }
    while ( (int)v4 < (int)DAT_07d73104 );
    delete__(v3);
    return fclose(v1);
  }
  else
  {
    sprintf(Text, "%s file not found.\r\n", FileName);
    CErrorReport::Write((DWORD)&g_ErrorReport, Text);
    MessageBoxA(g_hWnd, Text, 0, 0);
    return SendMessageA(g_hWnd, 2u, 0, 0);
  }
}
#endif
