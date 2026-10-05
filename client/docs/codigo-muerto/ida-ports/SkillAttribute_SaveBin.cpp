// 0x00479950 SkillAttribute_SaveBin — nunca activado: IDA_PORT_00479950 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SkillAttribute_SaveBin (IDA-only, gated) ──
#if defined(IDA_PORT_00479950)
int __cdecl SkillAttribute_SaveBin(FILE *Stream)
{
  char (*v1)[300]; // ebx
  char (*v2)[300]; // ebp
  FILE *Streama; // [esp+14h] [ebp+4h]

  Streama = fopen((const char *)Stream, aWb);
  v1 = (char (*)[300])operator_new(0x12Cu);
  v2 = GlobalText;
  do
  {
    qmemcpy(v1, v2, sizeof(char[300]));
    BuxConvert_0((BYTE *)v1, 300);
    crt_fwrite(v1, 0x12Cu, 1u, Streama);
    ++v2;
  }
  while ( (int)v2 < (int)DAT_07d73104 );
  delete__(v1);
  return fclose(Streama);
}
#endif
