// 0x004070D0 FUN_004070d0 — nunca activado: IDA_PORT_004070D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004070d0 (IDA-only, gated) ──
#if defined(IDA_PORT_004070D0)
char __stdcall FUN_004070d0(int a1, int a2)
{
  int v2; // ecx
  int v3; // eax
  int v4; // ecx
  int v5; // eax
  int v6; // ebx
  int v7; // esi
  unsigned int v8; // eax
  char *v9; // eax
  char v10; // cl
  int v11; // eax
  BYTE v12; // al
  BYTE *v13; // eax
  char v14; // cl
  int v15; // esi
  int v16; // edi
  char *v17; // ebp
  int v18; // eax
  unsigned int v19; // ebx
  int v20; // edi
  unsigned int v21; // ebp
  int v22; // eax
  int v23; // esi
  char *v24; // edi
  char *v25; // edi
  char *v26; // esi
  char v27; // cl
  unsigned int v28; // esi
  int v29; // ebx
  int v30; // eax
  int v31; // edi
  char *v33; // edi
  int v34; // [esp-14h] [ebp-DD0h]
  char v35; // [esp+0h] [ebp-DBCh]
  char v36; // [esp+1h] [ebp-DBBh]
  char v37; // [esp+2h] [ebp-DBAh]
  char v38; // [esp+3h] [ebp-DB9h]
  char v39; // [esp+4h] [ebp-DB8h]
  char v40; // [esp+5h] [ebp-DB7h]
  char v41; // [esp+6h] [ebp-DB6h]
  char v42; // [esp+7h] [ebp-DB5h]
  char v43; // [esp+8h] [ebp-DB4h]
  char v44; // [esp+9h] [ebp-DB3h]
  char v45; // [esp+Ah] [ebp-DB2h]
  char v46; // [esp+Bh] [ebp-DB1h]
  char v47; // [esp+Ch] [ebp-DB0h]
  char v48; // [esp+Dh] [ebp-DAFh]
  char v49; // [esp+Eh] [ebp-DAEh]
  char v50; // [esp+Fh] [ebp-DADh]
  char v51; // [esp+10h] [ebp-DACh]
  char v52; // [esp+11h] [ebp-DABh]
  char v53; // [esp+12h] [ebp-DAAh]
  char v54; // [esp+13h] [ebp-DA9h]
  char v55; // [esp+14h] [ebp-DA8h]
  char v56; // [esp+15h] [ebp-DA7h]
  char v57; // [esp+16h] [ebp-DA6h]
  char v58; // [esp+17h] [ebp-DA5h]
  char v59; // [esp+18h] [ebp-DA4h]
  char v60; // [esp+19h] [ebp-DA3h]
  char v61; // [esp+1Ah] [ebp-DA2h]
  char v62; // [esp+1Bh] [ebp-DA1h]
  char v63; // [esp+1Ch] [ebp-DA0h]
  char v64; // [esp+1Dh] [ebp-D9Fh]
  char v65; // [esp+1Eh] [ebp-D9Eh]
  char v66; // [esp+1Fh] [ebp-D9Dh]
  CHAR Text[128]; // [esp+20h] [ebp-D9Ch] BYREF
  void *(__cdecl **v68)(std::locale::facet *__hidden, unsigned int); // [esp+A0h] [ebp-D1Ch]
  BYTE v69[1025]; // [esp+A4h] [ebp-D18h] BYREF
  char buf[2]; // [esp+4A8h] [ebp-914h] BYREF
  char v71[258]; // [esp+4AAh] [ebp-912h] BYREF
  char v72[3]; // [esp+5ACh] [ebp-810h] BYREF
  char v73[1025]; // [esp+5AFh] [ebp-80Dh] BYREF
  char v74[1024]; // [esp+9B0h] [ebp-40Ch] BYREF
  int v75; // [esp+DB8h] [ebp-4h]

  switch ( a1 )
  {
    case 1001:
    case 1002:
      wsprintfA(Text, GlobalText[792], a1);
      goto LABEL_55;
    case 1011:
      MessageBoxA(g_hWnd, GlobalText[793], aError, 0);
      CloseHack(g_hWnd, 1);
      return 0;
    case 1012:
      wsprintfA(Text, GlobalText[794], a1);
      MessageBoxA(g_hWnd, Text, aError, 0);
      CloseHack(g_hWnd, 1);
      return 1;
    case 1013:
      wsprintfA(Text, GlobalText[794], a1);
LABEL_55:
      MessageBoxA(g_hWnd, Text, aError, 0);
      CloseHack(g_hWnd, 1);
      return 0;
    case 1014:
      wsprintfA(Text, GlobalText[792], a2);
      MessageBoxA(g_hWnd, Text, aError, 0);
      CloseHack(g_hWnd, 1);
      return 0;
    case 1016:
      v68 = &DAT_00552460;
      v75 = 0;
      *(DWORD *)v69 = 29425667;
      v69[4] = 115;
      v35 = -25;
      v36 = 109;
      v37 = 58;
      v69[*(unsigned short *)v69 + 2] = 0;
      v2 = *(unsigned short *)v69;
      v38 = -119;
      v3 = *(unsigned short *)v69 + 1;
      v39 = -68;
      v40 = -78;
      v41 = -97;
      v42 = 115;
      v43 = 35;
      v44 = -88;
      v45 = -2;
      v46 = -74;
      v47 = 73;
      v48 = 93;
      v49 = 57;
      v50 = 93;
      v51 = -118;
      v52 = -53;
      v53 = 99;
      v54 = -115;
      v55 = -22;
      v56 = 125;
      v57 = 43;
      v58 = 95;
      v59 = -61;
      v60 = -79;
      v61 = -23;
      v62 = -125;
      v63 = 41;
      v64 = 81;
      v65 = -24;
      v66 = 86;
      if ( *(unsigned short *)v69 != v3 )
      {
        do
        {
          v69[v2 + 2] ^= v69[v2 + 1] ^ *(&v35 + v2 % 32);
          ++v2;
        }
        while ( v2 != v3 );
      }
      ++*(WORD *)v69;
      if ( *(unsigned short *)v69 + 4 <= 1024 )
      {
        v35 = -25;
        v36 = 109;
        v37 = 58;
        *(DWORD *)&v69[*(unsigned short *)v69 + 2] = a2;
        v4 = *(unsigned short *)v69;
        v38 = -119;
        v5 = *(unsigned short *)v69 + 4;
        v39 = -68;
        v40 = -78;
        v41 = -97;
        v42 = 115;
        v43 = 35;
        v44 = -88;
        v45 = -2;
        v46 = -74;
        v47 = 73;
        v48 = 93;
        v49 = 57;
        v50 = 93;
        v51 = -118;
        v52 = -53;
        v53 = 99;
        v54 = -115;
        v55 = -22;
        v56 = 125;
        v57 = 43;
        v58 = 95;
        v59 = -61;
        v60 = -79;
        v61 = -23;
        v62 = -125;
        v63 = 41;
        v64 = 81;
        v65 = -24;
        v66 = 86;
        if ( *(unsigned short *)v69 != v5 )
        {
          do
          {
            v69[v4 + 2] ^= v69[v4 + 1] ^ *(&v35 + v4 % 32);
            ++v4;
          }
          while ( v4 != v5 );
        }
        *(WORD *)v69 += 4;
      }
      if ( v69[2] == 193 )
      {
        v69[3] = v69[0];
      }
      else if ( v69[2] == 194 )
      {
        *(WORD *)&v69[3] = *(WORD *)v69;
      }
      v6 = *(unsigned short *)v69;
      qmemcpy(v74, &v69[2], *(unsigned short *)v69);
      v74[v6] = rand();
      v7 = (v74[0] != -63) + 2;
      if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) == -1 )
      {
        v11 = operator_new(2u);
        *(BYTE *)(v11 + 1) = 1;
        HashTable_Insert(&MAIN_HASH_CLASS, v11, (int)&g_byPacketSerialSend);
      }
      else
      {
        v8 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
        if ( v8 == -1 )
        {
          v9 = 0;
        }
        else
        {
          v9 = *(char **)(DAT_055c9bcc + 4 * v8);
        }
        v10 = v9[1] + 1;
        v9[1] = v10;
        if ( (unsigned char)v10 < 2u )
        {
          Packet_DecryptByte(&g_byPacketSerialSend, v9);
        }
      }
      v12 = g_byPacketSerialSend;
      v73[v7 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v12 + 1;
      if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) != -1 )
      {
        v13 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
        v14 = v13[1] - 1;
        v13[1] = v14;
        if ( !v14 )
        {
          Packet_EncryptByte(v13, &g_byPacketSerialSend);
        }
      }
      v15 = v7 - 1;
      v16 = v6 - v15;
      v17 = &v74[v15];
      v18 = CSimpleModulus_Encode(0, (int)&v74[v15], v6 - v15);
      if ( v18 >= 256 )
      {
        v28 = v18 + 3;
        v72[0] = -60;
        v72[2] = v18 + 3;
        v72[1] = (v18 + 3) / 256;
        CSimpleModulus_Encode((int)v73, (int)v17, v16);
        v29 = 0;
        v21 = v28;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v30 = send(s, &v72[v29], v28 - v29, 0);
            v31 = v30;
            if ( v30 == -1 )
            {
              break;
            }
            if ( v30 )
            {
              if ( SocketClientLogPrint )
              {
                nullsub_2(v72, v30);
              }
              v21 -= v31;
              v29 += v31;
              if ( (int)v21 > 0 )
              {
                continue;
              }
            }
            return 1;
          }
          if ( WSAGetLastError() == 10035 && (int)(SocketClientSendBufferLength + v28) <= 0x2000 )
          {
            v33 = (char *)&SocketClientSendBuffer + SocketClientSendBufferLength;
            qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, v72, 4 * (v21 >> 2));
            v26 = &v72[4 * (v21 >> 2)];
            v25 = &v33[4 * (v21 >> 2)];
            v27 = v21;
            goto LABEL_47;
          }
LABEL_45:
          CWsctlc::Close((DWORD)&SocketClient);
          return 1;
        }
      }
      else
      {
        v34 = v6 - v15;
        v19 = v18 + 2;
        buf[0] = -61;
        buf[1] = v18 + 2;
        CSimpleModulus_Encode((int)v71, (int)&v74[v15], v34);
        v20 = 0;
        v21 = v19;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v22 = send(s, &buf[v20], v19 - v20, 0);
            v23 = v22;
            if ( v22 == -1 )
            {
              break;
            }
            if ( v22 )
            {
              if ( SocketClientLogPrint )
              {
                nullsub_2(buf, v22);
              }
              v21 -= v23;
              v20 += v23;
              if ( (int)v21 > 0 )
              {
                continue;
              }
            }
            return 1;
          }
          if ( WSAGetLastError() == 10035 && (int)(SocketClientSendBufferLength + v19) <= 0x2000 )
          {
            v24 = (char *)&SocketClientSendBuffer + SocketClientSendBufferLength;
            qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, buf, 4 * (v21 >> 2));
            v26 = &buf[4 * (v21 >> 2)];
            v25 = &v24[4 * (v21 >> 2)];
            v27 = v21;
LABEL_47:
            qmemcpy(v25, v26, v27 & 3);
            SocketClientSendBufferLength += v21;
            return 1;
          }
          goto LABEL_45;
        }
      }
      return 1;
    default:
      return 1;
  }
}
#endif
