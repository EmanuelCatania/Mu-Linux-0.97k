// 0x0043DDD0 CWsctlc_FDWriteSend — nunca activado: IDA_PORT_0043DDD0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── CWsctlc_FDWriteSend (IDA-only, gated) ──
#if defined(IDA_PORT_0043DDD0)
int __cdecl CWsctlc_FDWriteSend(const char *This)
{
  int v2; // ebx
  const char *v3; // ebp
  int v4; // eax
  int v5; // edi
  int v6; // eax

  v2 = 0;
  if ( *((int *)This + 2051) <= 0 )
  {
    return 1;
  }
  v3 = This + 12;
  while ( 1 )
  {
    v4 = send(*((DWORD *)This + 2), &v3[v2], *((DWORD *)This + 2051) - v2, 0);
    v5 = v4;
    if ( v4 == -1 )
    {
      break;
    }
    if ( v4 <= 0 )
    {
      goto LABEL_10;
    }
    if ( *((DWORD *)This + 4101) )
    {
      nullsub_2(This + 12, v4);
    }
    v2 += v5;
    v6 = *((DWORD *)This + 2051) - v5;
    *((DWORD *)This + 2051) = v6;
    if ( v6 <= 0 )
    {
      return 1;
    }
  }
  if ( WSAGetLastError() == 10035 )
  {
    return 1;
  }
LABEL_10:
  CWsctlc::Close((DWORD)This);
  return 0;
}
#endif
