// 0x00513C10 FUN_00513c10 — nunca activado: IDA_PORT_00513C10 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00513c10 (IDA-only, gated) ──
#if defined(IDA_PORT_00513C10)
void FUN_00513c10()
{
  int v0; // edi
  int v1; // ecx
  int i; // esi
  const char *v3; // ebx
  unsigned int v4; // kr04_4
  int v5; // ecx
  int j; // esi
  unsigned int v7; // kr08_4
  unsigned short v8; // bx
  int v9; // ecx
  int k; // esi
  char *v11; // eax
  int v12; // eax
  int v13; // ecx
  int v14; // ebx
  int v15; // edi
  signed int v16; // ebp
  int v17; // eax
  int v18; // esi
  char v19; // [esp+10h] [ebp-838h]
  char v20; // [esp+11h] [ebp-837h]
  char v21; // [esp+12h] [ebp-836h]
  char v22; // [esp+13h] [ebp-835h]
  char v23; // [esp+14h] [ebp-834h]
  char v24; // [esp+15h] [ebp-833h]
  char v25; // [esp+16h] [ebp-832h]
  char v26; // [esp+17h] [ebp-831h]
  char v27; // [esp+18h] [ebp-830h]
  char v28; // [esp+19h] [ebp-82Fh]
  char v29; // [esp+1Ah] [ebp-82Eh]
  char v30; // [esp+1Bh] [ebp-82Dh]
  char v31; // [esp+1Ch] [ebp-82Ch]
  char v32; // [esp+1Dh] [ebp-82Bh]
  char v33; // [esp+1Eh] [ebp-82Ah]
  char v34; // [esp+1Fh] [ebp-829h]
  char v35; // [esp+20h] [ebp-828h]
  char v36; // [esp+21h] [ebp-827h]
  char v37; // [esp+22h] [ebp-826h]
  char v38; // [esp+23h] [ebp-825h]
  char v39; // [esp+24h] [ebp-824h]
  char v40; // [esp+25h] [ebp-823h]
  char v41; // [esp+26h] [ebp-822h]
  char v42; // [esp+27h] [ebp-821h]
  char v43; // [esp+28h] [ebp-820h]
  char v44; // [esp+29h] [ebp-81Fh]
  char v45; // [esp+2Ah] [ebp-81Eh]
  char v46; // [esp+2Bh] [ebp-81Dh]
  char v47; // [esp+2Ch] [ebp-81Ch]
  char v48; // [esp+2Dh] [ebp-81Bh]
  char v49; // [esp+2Eh] [ebp-81Ah]
  char v50; // [esp+2Fh] [ebp-819h]
  unsigned int v51; // [esp+30h] [ebp-818h]
  void *(__cdecl **v52)(std::locale::facet *__hidden, unsigned int); // [esp+34h] [ebp-814h]
  WORD buf[514]; // [esp+38h] [ebp-810h] BYREF
  char v54[1024]; // [esp+43Ch] [ebp-40Ch] BYREF
  int v55; // [esp+844h] [ebp-4h]

  v0 = SelectedHero;
  SelectedHero = -1;
  DAT_005615e0 = v0;
  CurrentProtocolState = 56;
  v52 = &DAT_00552460;
  v55 = 0;
  (BYTE)(buf[1]) = -63;
  *(WORD *)((char *)&buf[1] + 1) = -3327;
  buf[0] = 3;
  v19 = -25;
  v20 = 109;
  v21 = 58;
  *((BYTE *)&buf[1] + buf[0]) = 2;
  v22 = -119;
  v23 = -68;
  v24 = -78;
  v1 = buf[0] + 1;
  v25 = -97;
  v26 = 115;
  v27 = 35;
  v28 = -88;
  v29 = -2;
  v30 = -74;
  v31 = 73;
  v32 = 93;
  v33 = 57;
  v34 = 93;
  v35 = -118;
  v36 = -53;
  v37 = 99;
  v38 = -115;
  v39 = -22;
  v40 = 125;
  v41 = 43;
  v42 = 95;
  v43 = -61;
  v44 = -79;
  v45 = -23;
  v46 = -125;
  v47 = 41;
  v48 = 81;
  v49 = -24;
  v50 = 86;
  for ( i = buf[0]; i != v1; ++i )
  {
    *((BYTE *)&buf[1] + i) ^= *((BYTE *)buf + i + 1) ^ *(&v19 + i % 32);
  }
  ++buf[0];
  v3 = (const char *)(CharactersClient + 916 * v0 + 449);
  v4 = strlen(v3) + 1;
  v51 = v4 - 1;
  if ( buf[0] + (unsigned short)(v4 - 1) <= 1024 )
  {
    qmemcpy((char *)&buf[1] + buf[0], v3, 4 * ((unsigned short)v51 >> 2) + (((BYTE)v4 - 1) & 3));
    v19 = -25;
    v20 = 109;
    v21 = 58;
    v22 = -119;
    v23 = -68;
    v24 = -78;
    v5 = buf[0] + (unsigned short)(v4 - 1);
    v25 = -97;
    v26 = 115;
    v27 = 35;
    v28 = -88;
    v29 = -2;
    v30 = -74;
    v31 = 73;
    v32 = 93;
    v33 = 57;
    v34 = 93;
    v35 = -118;
    v36 = -53;
    v37 = 99;
    v38 = -115;
    v39 = -22;
    v40 = 125;
    v41 = 43;
    v42 = 95;
    v43 = -61;
    v44 = -79;
    v45 = -23;
    v46 = -125;
    v47 = 41;
    v48 = 81;
    v49 = -24;
    v50 = 86;
    for ( j = buf[0]; j != v5; ++j )
    {
      *((BYTE *)&buf[1] + j) ^= *((BYTE *)buf + j + 1) ^ *(&v19 + j % 32);
    }
    buf[0] += v51;
  }
  v7 = strlen(v3) + 1;
  v8 = 10 - (v7 - 1);
  memset(v54, 0, v8);
  if ( buf[0] + v8 <= 1024 )
  {
    qmemcpy((char *)&buf[1] + buf[0], v54, (unsigned short)(10 - (v7 - 1)));
    v19 = -25;
    v20 = 109;
    v21 = 58;
    v22 = -119;
    v23 = -68;
    v24 = -78;
    v9 = buf[0] + v8;
    v25 = -97;
    v26 = 115;
    v27 = 35;
    v28 = -88;
    v29 = -2;
    v30 = -74;
    v31 = 73;
    v32 = 93;
    v33 = 57;
    v34 = 93;
    v35 = -118;
    v36 = -53;
    v37 = 99;
    v38 = -115;
    v39 = -22;
    v40 = 125;
    v41 = 43;
    v42 = 95;
    v43 = -61;
    v44 = -79;
    v45 = -23;
    v46 = -125;
    v47 = 41;
    v48 = 81;
    v49 = -24;
    v50 = 86;
    for ( k = buf[0]; k != v9; ++k )
    {
      *((BYTE *)&buf[1] + k) ^= *((BYTE *)buf + k + 1) ^ *(&v19 + k % 32);
    }
    buf[0] += v8;
  }
  if ( buf[0] + 10 <= 1024 )
  {
    v11 = (char *)&buf[1] + buf[0];
    v19 = -25;
    v20 = 109;
    *(DWORD *)v11 = *(DWORD *)&InputText[0][0];
    v21 = 58;
    v22 = -119;
    *((DWORD *)v11 + 1) = *(DWORD *)&InputText[0][4];
    v23 = -68;
    v24 = -78;
    *((WORD *)v11 + 4) = *(WORD *)&InputText[0][8];
    v12 = buf[0];
    v25 = -97;
    v26 = 115;
    v27 = 35;
    v13 = buf[0] + 10;
    v28 = -88;
    v29 = -2;
    v30 = -74;
    v31 = 73;
    v32 = 93;
    v33 = 57;
    v34 = 93;
    v35 = -118;
    v36 = -53;
    v37 = 99;
    v38 = -115;
    v39 = -22;
    v40 = 125;
    v41 = 43;
    v42 = 95;
    v43 = -61;
    v44 = -79;
    v45 = -23;
    v46 = -125;
    v47 = 41;
    v48 = 81;
    v49 = -24;
    v50 = 86;
    if ( buf[0] != v13 )
    {
      do
      {
        *((BYTE *)&buf[1] + v12) ^= *((BYTE *)buf + v12 + 1) ^ *(&v19 + v12 % 32);
        ++v12;
      }
      while ( v12 != v13 );
    }
    buf[0] += 10;
  }
  if ( (BYTE)(buf[1]) == 193 )
  {
    (BYTE)((buf[1]) >> 8) = buf[0];
  }
  else if ( (BYTE)(buf[1]) == 194 )
  {
    *(WORD *)((char *)&buf[1] + 1) = buf[0];
  }
  v14 = buf[0];
  v15 = 0;
  v16 = buf[0];
  if ( s != -1 )
  {
    while ( 1 )
    {
      v17 = send(s, (const char *)&buf[1] + v15, v14 - v15, 0);
      v18 = v17;
      if ( v17 == -1 )
      {
        break;
      }
      if ( v17 )
      {
        if ( SocketClientLogPrint )
        {
          nullsub_2((int)&buf[1], v17);
        }
        v16 -= v18;
        v15 += v18;
        if ( v16 > 0 )
        {
          continue;
        }
      }
      goto LABEL_32;
    }
    if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v14 <= 0x2000 )
    {
      qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf[1], v16);
      SocketClientSendBufferLength += v16;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
LABEL_32:
  v55 = -1;
  v52 = &DAT_00552460;
  DAT_083a7c14 = 24;
  DAT_083a7c18 = 21;
  PlayBuffer(27, 0, 0);
  ClearInput(1);
  InputEnable = 0;
}
#endif
