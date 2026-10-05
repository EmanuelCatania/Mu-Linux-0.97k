// 0x0053E930 FUN_0053e930 — nunca activado: IDA_PORT_0053E930 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0053e930 (IDA-only, gated) ──
#if defined(IDA_PORT_0053E930)
BYTE *__cdecl FUN_0053e8c0(BYTE *a1)
{
  BYTE *result; // eax
  int v2; // ecx
  int v3; // esi
  char v4; // bl
  int v5; // ecx
  unsigned short v6; // dx
  int v7; // ecx
  char v8; // bl

  result = a1;
  if ( a1 && *a1 == 1 )
  {
    v2 = 9 * (char)a1[1] + 3;
    a1[2] ^= 3 * a1[1] + 101;
    v3 = 0;
    v4 = (v2 + 101) ^ a1[3];
    (BYTE)((v6) >> 8) = a1[2];
    v5 = v2 + 1;
    (BYTE)(v6) = v4;
    a1[3] = v4;
    if ( v6 )
    {
      do
      {
        v7 = 3 * v5;
        v8 = (v7 + 101) ^ a1[v3 + 4];
        v5 = v7 + 1;
        a1[v3++] = v8;
      }
      while ( v3 < v6 );
    }
    a1[v6] = 0;
  }
  return result;
}
#endif
