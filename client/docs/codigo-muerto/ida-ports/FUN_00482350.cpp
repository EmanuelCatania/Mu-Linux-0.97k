// 0x00482350 FUN_00482350 — nunca activado: IDA_PORT_00482350 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00482350 (IDA-only, gated) ──
#if defined(IDA_PORT_00482350)
void __cdecl FUN_00482350()
{
  const char *v0; // ebp
  int v1; // eax
  int v2; // eax
  BYTE *v3; // eax
  int v4; // ecx
  int v5; // ecx

  v0 = (const char *)DAT_07e01720;
  do
  {
    if ( *(int *)v0 > 0 )
    {
      --*(DWORD *)v0;
    }
    v1 = *((DWORD *)v0 + 129);
    if ( v1 > 0 )
    {
      *((DWORD *)v0 + 129) = v1 - 1;
    }
    v2 = *((DWORD *)v0 + 130);
    if ( v2 > 0 )
    {
      *((DWORD *)v0 + 130) = v2 - 1;
    }
    v3 = (BYTE *)*((DWORD *)v0 + 131);
    if ( v3 && (!*v3 || !v3[352]) )
    {
      *(DWORD *)v0 = 0;
      *((DWORD *)v0 + 129) = 0;
      *((DWORD *)v0 + 130) = 0;
    }
    v4 = *((DWORD *)v0 + 132);
    if ( v4 <= MouseX && MouseX < (int)(v4 + 640 * *((DWORD *)v0 + 134) / WindowWidth) )
    {
      v5 = *((DWORD *)v0 + 133);
      if ( v5 <= MouseY
        && MouseY < (int)(v5 + 480 * *((DWORD *)v0 + 135) / WindowHeight)
        && InputEnable
        && *(BYTE *)(Hero + 846)
        && strcmp(v0 - 40, (const char *)(Hero + 449))
        && MouseRButtonPush )
      {
        strcpy(InputText[1], v0 - 40);
        MouseRButtonPush = 0;
        InputLength[1] = strlen(InputText[1]);
        PlayBuffer(25, 0, 0);
      }
    }
    v0 += 596;
  }
  while ( (int)v0 < (int)&DAT_07e0fff0 );
}
#endif
