// 0x0047E4F0 CHARACTER_MACHINE_GetMagicSkillDamage — nunca activado: IDA_PORT_0047E4F0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj. Ver src/UI/UI_LegacyExterns.cpp (sin esto el tooltip de skill muestra 0~0 de dano).
// ── CHARACTER_MACHINE_GetMagicSkillDamage (IDA-only, gated) ──
#if defined(IDA_PORT_0047E4F0)
void __cdecl CHARACTER_MACHINE::GetMagicSkillDamage(DWORD This, int iType, int *piMinDamage, int *piMaxDamage)
{
  int v4; // ebx
  int v5; // esi
  unsigned int v6; // eax
  bool v7; // cf
  unsigned int v8; // eax
  BYTE *v9; // esi
  unsigned char v10; // al
  void *v11; // ebp
  unsigned int v12; // ecx
  int v13; // esi
  char v14; // al
  int v15; // eax
  DWORD v16; // esi
  BYTE *v17; // eax
  int *v18; // ebx
  unsigned int v19; // eax
  unsigned int v20; // eax
  int v21; // eax
  char v22; // cl
  int v23; // [esp+10h] [ebp-10h]
  int v24; // [esp+14h] [ebp-Ch] BYREF
  BYTE *v25; // [esp+18h] [ebp-8h] BYREF
  DWORD v26; // [esp+1Ch] [ebp-4h]
  BYTE *iTypea; // [esp+24h] [ebp+4h]
  int iTypeb; // [esp+24h] [ebp+4h]

  v4 = 40 * iType;
  v26 = This;
  iTypea = &SkillAttribute[40 * iType];
  v5 = (int)iTypea;
  v25 = iTypea;
  v6 = (*(int (__cdecl **)(int *, BYTE *))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, iTypea);
  v24 = 0;
  v23 = 0;
  if...
  v16 = v26;
  v17 = &SkillAttribute[v4 + 33];
  *piMinDamage = *v17 + *(unsigned short *)(v26 + 70);
  *piMaxDamage = (*v17 >> 1) + *v17 + *(unsigned short *)(v16 + 72);
  v18 = (int *)&SkillAttribute[v4];
  piMaxDamage = v18;
  v19 = (*(int (__cdecl **)(int *, int *))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, v18);
  piMinDamage = 0;
  iTypeb = 0;
  if...
}
#endif
