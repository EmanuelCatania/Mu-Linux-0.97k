// 0x004F6850 SecondPassword_CancelReturn — nunca activado: IDA_PORT_004F6850 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── SecondPassword_CancelReturn (IDA-only, gated) ──
#if defined(IDA_PORT_004F6850)
char SecondPassword_CancelReturn()
{
  char v0; // dl
  ITEM *v1; // eax
  int v2; // ecx
  int v3; // edi
  signed int v4; // ebp
  int v5; // eax
  int v6; // esi
  char buf[4]; // [esp+Ch] [ebp-410h] BYREF
  char v9; // [esp+10h] [ebp-40Ch]
  int v10; // [esp+418h] [ebp-4h]

  v0 = 1;
  v1 = &OffsetMixItems;
  do
  {
    v2 = 8;
    do
    {
      if ( v1->Type != -1 && (int)v1->Key > 0 )
      {
        v0 = 0;
      }
      ++v1;
      --v2;
    }
    while ( v2 );
  }
  while ( (int)v1 < (int)&DAT_07eaa0c8 );
  if ( v0 && (v3 = 0, DAT_07e91388 <= 0) )
  {
    buf[2] = -63;
    v10 = 0;
    v9 = -121;
    buf[3] = 3;
    v4 = 3;
    if ( s != -1 )
    {
      while ( 1 )
      {
        v5 = send(s, &buf[v3 + 2], 3 - v3, 0);
        v6 = v5;
        if ( v5 == -1 )
        {
          break;
        }
        if ( v5 )
        {
          if ( SocketClientLogPrint )
          {
            nullsub_2((int)&buf[2], v5);
          }
          v4 -= v6;
          v3 += v6;
          if ( v4 > 0 )
          {
            continue;
          }
        }
        return 1;
      }
      if ( WSAGetLastError() != 10035 || SocketClientSendBufferLength + 3 > 0x2000 )
      {
        CWsctlc::Close((DWORD)&SocketClient);
        return 1;
      }
      qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf[2], v4);
      SocketClientSendBufferLength += v4;
    }
    return 1;
  }
  else
  {
    UIChatLogWindow_AddText(&strID, GlobalText[593], 2);
    return 0;
  }
}
#endif
