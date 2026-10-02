// 0x0040E5B0 FUN_0040e5b0 — nunca activado: IDA_PORT_0040E5B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_0040e5b0 (IDA-only, gated) ──
#if defined(IDA_PORT_0040E5B0)
void __cdecl FUN_0040e5b0(const char *_this, char *Source)
{
  int v3; // eax
  char *v4; // edi
  char *v5; // edx
  char *Destination; // [esp+10h] [ebp-10Ch]
  int v7; // [esp+14h] [ebp-108h]
  int v8; // [esp+18h] [ebp-104h]
  char String[253]; // [esp+1Ch] [ebp-100h] BYREF
  short v10; // [esp+119h] [ebp-3h]
  char v11; // [esp+11Bh] [ebp-1h]

  if ( Source )
  {
    v3 = 0;
    if ( strlen(Source) <= 0xFF )
    {
      (BYTE)(v3) = _this[200] != 0;
      v8 = v3;
      memset(String, 0, sizeof(String));
      v10 = 0;
      v11 = 0;
      strncpy(String, Source, 0x100u);
      strtok(String, DAT_005590ec);
      v4 = (char *)(_this + 200);
      v7 = 0;
      Destination = (char *)(_this + 200);
      do
      {
        v5 = strtok(0, DAT_005590ec);
        if ( v5 )
        {
          if ( strlen(v5)
             + strlen(_this + 200)
             + strlen(_this + 456)
             + strlen(_this + 712)
             + strlen(_this + 968)
             + strlen(_this + 1224) <= 0x1E )
          {
            v4 = Destination;
            strncpy(Destination, v5, 0x100u);
          }
          else
          {
            if ( !v7 )
            {
              return;
            }
            v4 = Destination;
            *Destination = 0;
          }
        }
        else
        {
          *v4 = 0;
        }
        v4 += 256;
        Destination = v4;
        ++v7;
      }
      while ( v7 < 5 );
      if ( _this[200] )
      {
        UIChatLogWindow_AddText(&strID, GlobalText[755], 1);
      }
      else if ( v8 == 1 )
      {
        UIChatLogWindow_AddText(&strID, GlobalText[756], 1);
      }
    }
  }
}
#endif
