// 0x0043DE70 CWsctlc_nRecv — nunca activado: IDA_PORT_0043DE70 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CWsctlc_nRecv (IDA-only, gated) ──
#if defined(IDA_PORT_0043DE70)
int __cdecl CWsctlc_nRecv(SOCKET *_this)
{
  int v2; // eax
  int v4; // edx
  int v5; // edi
  char v6; // al
  int v7; // ebx
  int v8; // eax
  signed int v9; // ecx

  v2 = recv(_this[2], (char *)_this + _this[4100] + 8208, 0x2000 - _this[4100], 0);
  if ( !v2 )
  {
    return 1;
  }
  if ( v2 == -1 )
  {
    WSAGetLastError();
    return 1;
  }
  else
  {
    v4 = v2 + _this[4100];
    _this[4100] = v4;
    if ( v4 >= 3 )
    {
      v5 = 0;
      do
      {
        v6 = *((BYTE *)_this + v5 + 8208);
        if ( v6 == -63 || v6 == -61 )
        {
          v7 = *((unsigned char *)_this + v5 + 8209);
        }
        else
        {
          if ( v6 != -62 && v6 != -60 )
          {
            _this[4100] = 0;
            return 0;
          }
          v7 = *((unsigned char *)_this + v5 + 8210) + (*((unsigned char *)_this + v5 + 8209) << 8);
        }
        if ( v7 <= 0 )
        {
          break;
        }
        if ( v7 > (int)_this[4100] )
        {
          if ( v5 > 0 )
          {
            v9 = _this[4100];
            if ( v9 >= 1 )
            {
              qmemcpy(_this + 2052, (char *)_this + v5 + 8208, v9);
            }
          }
          return 0;
        }
        CPacketQueue_PushPacket((char *)_this + v5 + 8208, v7);
        if ( _this[4101] )
        {
          nullsub_2((int)_this + v5 + 8208, v7);
        }
        v5 += v7;
        v8 = _this[4100] - v7;
        _this[4100] = v8;
      }
      while ( v8 > 0 );
      return 0;
    }
    else
    {
      return 3;
    }
  }
}
#endif
