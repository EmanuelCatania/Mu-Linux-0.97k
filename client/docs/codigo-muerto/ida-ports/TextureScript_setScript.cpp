// 0x0040C170 TextureScript_setScript — nunca activado: IDA_PORT_0040C170 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── TextureScript_setScript (IDA-only, gated) ──
#if defined(IDA_PORT_0040C170)
void __cdecl TextureScript::setScript(DWORD This, DWORD That)
{
  *(BYTE *)This = *(BYTE *)That;
  *(BYTE *)(This + 1) = *(BYTE *)(That + 1);
  *(BYTE *)(This + 2) = *(BYTE *)(That + 2);
  *(BYTE *)(This + 3) = *(BYTE *)(That + 3);
}
#endif
