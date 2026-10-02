// 0x00408FF0 FUN_00408ff0 — nunca activado: IDA_PORT_00408FF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00408ff0 (IDA-only, gated) ──
#if defined(IDA_PORT_00408FF0)
void __cdecl FUN_00408ff0(int _this, int a2)
{
  DWORD *v3; // ebx
  int i; // ebp
  int v5; // eax
  int j; // edi

  v3 = (DWORD *)operator_new(12 * *(DWORD *)(_this + 48));
  for ( i = 0; i < *(DWORD *)(_this + 44); ++i )
  {
    v5 = *(DWORD *)(_this + 40);
    for ( j = 0; j < v5; ++j )
    {
      SpringNode_GetPos((int *)(*(DWORD *)(_this + 52) + 60 * (j + i * v5)), &v3[3 * j + 3 * i * v5]);
      v5 = *(DWORD *)(_this + 40);
    }
  }
  if ( (*(DWORD *)(_this + 20) & 0x3000) != 0 )
  {
    if ( (*(DWORD *)(_this + 20) & 0x3000) == 4096 )
    {
      EnableAlphaTest(1);
    }
  }
  else
  {
    DisableAlphaBlend();
  }
  glColor3f(1.0, 1.0, 1.0);
  FUN_004090b0((DWORD *)_this, 1, *(GLfloat *)(_this + 12), (int)v3);
  FUN_004090b0((DWORD *)_this, 0, *(GLfloat *)(_this + 16), (int)v3);
  delete__(v3);
}
#endif
