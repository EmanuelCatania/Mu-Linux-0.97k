// 0x00409EB0 FUN_00409eb0 — nunca activado: IDA_PORT_00409EB0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00409eb0 (IDA-only, gated) ──
#if defined(IDA_PORT_00409EB0)
// Microsoft VisualC 2-14/net runtime
DWORD *FUN_00409eb0()
{
  int v0; // eax
  int v1; // eax
  int v2; // ecx

  v0 = operator_new(0xCu);
  if ( v0 )
  {
    *(DWORD *)(v0 + 8) = 0;
    *(DWORD *)(v0 + 4) = 0;
  }
  else
  {
    v0 = 0;
  }
  DAT_00590b00[1] = v0;
  v1 = operator_new(0xCu);
  if ( v1 )
  {
    *(DWORD *)(v1 + 8) = 0;
    *(DWORD *)(v1 + 4) = 0;
  }
  else
  {
    v1 = 0;
  }
  v2 = DAT_00590b00[1];
  DAT_00590b00[2] = v1;
  *(DWORD *)(v2 + 8) = v1;
  *(DWORD *)(DAT_00590b00[2] + 4) = DAT_00590b00[1];
  DAT_00590b00[0] = 0;
  return DAT_00590b00;
}
#endif
