// 0x00401AF0 FUN_00401af0 — nunca activado: IDA_PORT_00401AF0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00401af0 (IDA-only, gated) ──
#if defined(IDA_PORT_00401AF0)
void __cdecl FUN_00401af0(DWORD This)
{
  int v1; // esi
  int v2; // eax
  int v3; // ecx
  int v4; // eax
  char v5; // dl
  int v6; // edx
  int i; // esi
  int v8; // eax
  int v9; // edx
  int v10; // ebx
  int v11; // edi
  int v12; // eax
  unsigned char v13; // cl
  int v14; // eax
  BYTE v15; // al
  int v16; // edi
  int v17; // esi
  char *v18; // ebx
  int v19; // eax
  int v20; // ebp
  int v21; // edi
  signed int v22; // ebx
  int v23; // eax
  int v24; // esi
  int v25; // eax
  signed int v26; // edi
  int v27; // ebx
  signed int v28; // ebp
  int v29; // eax
  int v30; // esi
  char v31; // dl
  int v32; // edx
  int j; // esi
  int v34; // eax
  int v35; // edx
  int v36; // ebx
  int v37; // esi
  unsigned int v38; // eax
  int v39; // eax
  unsigned char v40; // cl
  int v41; // eax
  BYTE v42; // al
  unsigned int v43; // eax
  BYTE *v44; // eax
  char v45; // cl
  int v46; // esi
  int v47; // edi
  char *v48; // ebp
  int v49; // eax
  unsigned int v50; // ebx
  int v51; // edi
  unsigned int v52; // ebp
  int v53; // eax
  int v54; // esi
  char *v55; // edi
  char *v56; // edi
  char *v57; // esi
  char v58; // cl
  unsigned int v59; // esi
  int v60; // ebx
  int v61; // eax
  int v62; // edi
  char *v63; // edi
  int v64; // eax
  int v65; // [esp-8h] [ebp-D60h]
  char v66; // [esp+Ch] [ebp-D4Ch]
  char v67; // [esp+Dh] [ebp-D4Bh]
  char v68; // [esp+Eh] [ebp-D4Ah]
  char v69; // [esp+Fh] [ebp-D49h]
  char v70; // [esp+10h] [ebp-D48h]
  char v71; // [esp+11h] [ebp-D47h]
  char v72; // [esp+12h] [ebp-D46h]
  char v73; // [esp+13h] [ebp-D45h]
  char v74; // [esp+14h] [ebp-D44h]
  char v75; // [esp+15h] [ebp-D43h]
  char v76; // [esp+16h] [ebp-D42h]
  char v77; // [esp+17h] [ebp-D41h]
  char v78; // [esp+18h] [ebp-D40h]
  char v79; // [esp+19h] [ebp-D3Fh]
  char v80; // [esp+1Ah] [ebp-D3Eh]
  char v81; // [esp+1Bh] [ebp-D3Dh]
  char v82; // [esp+1Ch] [ebp-D3Ch]
  char v83; // [esp+1Dh] [ebp-D3Bh]
  char v84; // [esp+1Eh] [ebp-D3Ah]
  char v85; // [esp+1Fh] [ebp-D39h]
  char v86; // [esp+20h] [ebp-D38h]
  char v87; // [esp+21h] [ebp-D37h]
  char v88; // [esp+22h] [ebp-D36h]
  char v89; // [esp+23h] [ebp-D35h]
  char v90; // [esp+24h] [ebp-D34h]
  char v91; // [esp+25h] [ebp-D33h]
  char v92; // [esp+26h] [ebp-D32h]
  char v93; // [esp+27h] [ebp-D31h]
  char v94; // [esp+28h] [ebp-D30h]
  char v95; // [esp+29h] [ebp-D2Fh]
  char v96; // [esp+2Ah] [ebp-D2Eh]
  char v97; // [esp+2Bh] [ebp-D2Dh]
  char v98; // [esp+2Fh] [ebp-D29h]
  DWORD v99; // [esp+30h] [ebp-D28h]
  int v100; // [esp+34h] [ebp-D24h]
  int v101; // [esp+38h] [ebp-D20h]
  void *(__cdecl **v102)(std::locale::facet *__hidden, unsigned int); // [esp+3Ch] [ebp-D1Ch]
  BYTE v103[1025]; // [esp+40h] [ebp-D18h] BYREF
  char buf; // [esp+444h] [ebp-914h] BYREF
  char v105; // [esp+445h] [ebp-913h]
  char v106[258]; // [esp+446h] [ebp-912h] BYREF
  char v107; // [esp+548h] [ebp-810h] BYREF
  char v108; // [esp+549h] [ebp-80Fh]
  char v109; // [esp+54Ah] [ebp-80Eh]
  char v110[1025]; // [esp+54Bh] [ebp-80Dh] BYREF
  char v111[1024]; // [esp+94Ch] [ebp-40Ch] BYREF
  int v112; // [esp+D54h] [ebp-4h]

  v1 = This;
  v99 = This;
  v98 = 0;
  v2 = 18 * (7 - (g_iNumAnswer + g_iNumLineMessageBoxCustom)) / 2 + 18 * g_iNumLineMessageBoxCustom + 66;
  if ( *(BYTE *)(This + 116866) != 1 && *(BYTE *)(This + 116863) == 1 )
  {
    v2 = 250;
  }
  if ( MouseY >= 0 )
  {
    v3 = MouseY - v2;
    if ( MouseY - v2 < 18 * g_iNumAnswer && (int)abs32(566 - MouseX) <= 106 && MouseLButtonPush )
    {
      MouseLButtonPush = 0;
      MouseLButton = 0;
      v100 = v3 / 18;
      MouseUpdateTime = 0;
      MouseUpdateTimeMax = 6;
      if ( v3 / 18 >= 0 )
      {
        v4 = g_DialogScript[g_iCurrentDialogScript].m_iReturnForAnswer[v3 / 18];
        if ( v4 == 1 )
        {
          if ( !CSQuest::CheckRequestCondition(v1, v1 + 584 * *(unsigned char *)(v1 + 116858) + 8, 1) )
          {
            v65 = *(short *)(v1 + 116864);
            v98 = 1;
            CSQuest::ShowDialogText(v1, v65);
            goto LABEL_107;
          }
          v102 = &DAT_00552460;
          v112 = 0;
          *(DWORD *)v103 = 29425667;
          v103[4] = -94;
          v5 = *(BYTE *)(v1 + 116858);
          v66 = -25;
          v67 = 109;
          v68 = 58;
          v103[*(unsigned short *)v103 + 2] = v5;
          v69 = -119;
          v70 = -68;
          v71 = -78;
          v6 = *(unsigned short *)v103 + 1;
          v72 = -97;
          v73 = 115;
          v74 = 35;
          v75 = -88;
          v76 = -2;
          v77 = -74;
          v78 = 73;
          v79 = 93;
          v80 = 57;
          v81 = 93;
          v82 = -118;
          v83 = -53;
          v84 = 99;
          v85 = -115;
          v86 = -22;
          v87 = 125;
          v88 = 43;
          v89 = 95;
          v90 = -61;
          v91 = -79;
          v92 = -23;
          v93 = -125;
          v94 = 41;
          v95 = 81;
          v96 = -24;
          v97 = 86;
          for ( i = *(unsigned short *)v103; i != v6; ++i )
          {
            v103[i + 2] ^= v103[i + 1] ^ *(&v66 + i % 32);
          }
          ++*(WORD *)v103;
          if ( *(unsigned short *)v103 + 1 <= 1024 )
          {
            v66 = -25;
            v67 = 109;
            v68 = 58;
            v103[*(unsigned short *)v103 + 2] = 1;
            v8 = *(unsigned short *)v103;
            v69 = -119;
            v70 = -68;
            v71 = -78;
            v9 = *(unsigned short *)v103 + 1;
            v72 = -97;
            v73 = 115;
            v74 = 35;
            v75 = -88;
            v76 = -2;
            v77 = -74;
            v78 = 73;
            v79 = 93;
            v80 = 57;
            v81 = 93;
            v82 = -118;
            v83 = -53;
            v84 = 99;
            v85 = -115;
            v86 = -22;
            v87 = 125;
            v88 = 43;
            v89 = 95;
            v90 = -61;
            v91 = -79;
            v92 = -23;
            v93 = -125;
            v94 = 41;
            v95 = 81;
            v96 = -24;
            v97 = 86;
            if ( *(unsigned short *)v103 != v9 )
            {
              do
              {
                v103[v8 + 2] ^= v103[v8 + 1] ^ *(&v66 + v8 % 32);
                ++v8;
              }
              while ( v8 != v9 );
            }
            ++*(WORD *)v103;
          }
          if ( v103[2] == 193 )
          {
            v103[3] = v103[0];
          }
          else if ( v103[2] == 194 )
          {
            *(WORD *)&v103[3] = *(WORD *)v103;
          }
          v10 = *(unsigned short *)v103;
          qmemcpy(v111, &v103[2], *(unsigned short *)v103);
          v111[v10] = rand();
          v11 = (v111[0] != -63) + 2;
          if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) == -1 )
          {
            v14 = operator_new(2u);
            *(BYTE *)(v14 + 1) = 1;
            HashTable_Insert(&MAIN_HASH_CLASS, v14, (int)&g_byPacketSerialSend);
          }
          else
          {
            v12 = HashTable_GetNode(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
            v13 = *(BYTE *)(v12 + 1) + 1;
            *(BYTE *)(v12 + 1) = v13;
            if ( v13 < 2u )
            {
              Packet_DecryptByte(&g_byPacketSerialSend, v12);
            }
          }
          v15 = g_byPacketSerialSend;
          v110[v11 + 1024] = g_byPacketSerialSend;
          g_byPacketSerialSend = v15 + 1;
          PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
          v16 = v11 - 1;
          v17 = v10 - v16;
          v18 = &v111[v16];
          v19 = CSimpleModulus_Encode(0, (int)&v111[v16], v17);
          if ( v19 >= 256 )
          {
            v26 = v19 + 3;
            v107 = -60;
            v109 = v19 + 3;
            v108 = (v19 + 3) / 256;
            CSimpleModulus_Encode((int)v110, (int)v18, v17);
            v27 = 0;
            v28 = v26;
            if ( s != -1 )
            {
              while ( 1 )
              {
                v29 = send(s, &v107 + v27, v26 - v27, 0);
                v30 = v29;
                if ( v29 == -1 )
                {
                  break;
                }
                if ( v29 )
                {
                  if ( SocketClientLogPrint )
                  {
                    nullsub_2(&v107, v29);
                  }
                  v28 -= v30;
                  v27 += v30;
                  if ( v28 > 0 )
                  {
                    continue;
                  }
                }
                goto LABEL_52;
              }
              if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v26 <= 0x2000 )
              {
                qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &v107, v28);
                v25 = v28 + SocketClientSendBufferLength;
                goto LABEL_51;
              }
LABEL_49:
              CWsctlc::Close((DWORD)&SocketClient);
              v112 = -1;
LABEL_106:
              v1 = v99;
              goto LABEL_107;
            }
          }
          else
          {
            v101 = v19 + 2;
            buf = -61;
            v105 = v19 + 2;
            CSimpleModulus_Encode((int)v106, (int)&v111[v16], v17);
            v20 = v101;
            v21 = 0;
            v22 = v101;
            if ( s != -1 )
            {
              while ( 1 )
              {
                v23 = send(s, &buf + v21, v20 - v21, 0);
                v24 = v23;
                if ( v23 == -1 )
                {
                  break;
                }
                if ( v23 )
                {
                  if ( SocketClientLogPrint )
                  {
                    nullsub_2(&buf, v23);
                  }
                  v22 -= v24;
                  v21 += v24;
                  if ( v22 > 0 )
                  {
                    continue;
                  }
                }
                goto LABEL_52;
              }
              if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v20 <= 0x2000 )
              {
                qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf, v22);
                v25 = v22 + SocketClientSendBufferLength;
LABEL_51:
                SocketClientSendBufferLength = v25;
                goto LABEL_52;
              }
              goto LABEL_49;
            }
          }
LABEL_52:
          v112 = -1;
          goto LABEL_106;
        }
        if ( v4 == 2 )
        {
          MouseUpdateTimeMax = 6;
          MouseLButtonPush = 0;
          MouseUpdateTime = 0;
          CSQuest::clearQuest(v1);
          goto LABEL_107;
        }
        if ( v4 != 3 )
        {
LABEL_107:
          PlayBuffer(28, 0, 0);
          v64 = g_DialogScript[g_iCurrentDialogScript].m_iLinkForAnswer[v100];
          if ( v64 > 0 && !v98 )
          {
            CSQuest::ShowDialogText(v1, v64);
          }
          return;
        }
        MouseLButtonPush = 0;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        v102 = &DAT_00552460;
        v112 = 1;
        *(DWORD *)v103 = 29425667;
        v103[4] = -94;
        v31 = *(BYTE *)(v1 + 116858);
        v66 = -25;
        v67 = 109;
        v68 = 58;
        v103[*(unsigned short *)v103 + 2] = v31;
        v69 = -119;
        v70 = -68;
        v71 = -78;
        v32 = *(unsigned short *)v103 + 1;
        v72 = -97;
        v73 = 115;
        v74 = 35;
        v75 = -88;
        v76 = -2;
        v77 = -74;
        v78 = 73;
        v79 = 93;
        v80 = 57;
        v81 = 93;
        v82 = -118;
        v83 = -53;
        v84 = 99;
        v85 = -115;
        v86 = -22;
        v87 = 125;
        v88 = 43;
        v89 = 95;
        v90 = -61;
        v91 = -79;
        v92 = -23;
        v93 = -125;
        v94 = 41;
        v95 = 81;
        v96 = -24;
        v97 = 86;
        for ( j = *(unsigned short *)v103; j != v32; ++j )
        {
          v103[j + 2] ^= v103[j + 1] ^ *(&v66 + j % 32);
        }
        ++*(WORD *)v103;
        if ( *(unsigned short *)v103 + 1 <= 1024 )
        {
          v66 = -25;
          v67 = 109;
          v68 = 58;
          v103[*(unsigned short *)v103 + 2] = 1;
          v34 = *(unsigned short *)v103;
          v69 = -119;
          v70 = -68;
          v71 = -78;
          v35 = *(unsigned short *)v103 + 1;
          v72 = -97;
          v73 = 115;
          v74 = 35;
          v75 = -88;
          v76 = -2;
          v77 = -74;
          v78 = 73;
          v79 = 93;
          v80 = 57;
          v81 = 93;
          v82 = -118;
          v83 = -53;
          v84 = 99;
          v85 = -115;
          v86 = -22;
          v87 = 125;
          v88 = 43;
          v89 = 95;
          v90 = -61;
          v91 = -79;
          v92 = -23;
          v93 = -125;
          v94 = 41;
          v95 = 81;
          v96 = -24;
          v97 = 86;
          if ( *(unsigned short *)v103 != v35 )
          {
            do
            {
              v103[v34 + 2] ^= v103[v34 + 1] ^ *(&v66 + v34 % 32);
              ++v34;
            }
            while ( v34 != v35 );
          }
          ++*(WORD *)v103;
        }
        if ( v103[2] == 193 )
        {
          v103[3] = v103[0];
        }
        else if ( v103[2] == 194 )
        {
          *(WORD *)&v103[3] = *(WORD *)v103;
        }
        v36 = *(unsigned short *)v103;
        qmemcpy(v111, &v103[2], *(unsigned short *)v103);
        v111[v36] = rand();
        v37 = (v111[0] != -63) + 2;
        if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) == -1 )
        {
          v41 = operator_new(2u);
          *(BYTE *)(v41 + 1) = 1;
          HashTable_Insert(&MAIN_HASH_CLASS, v41, (int)&g_byPacketSerialSend);
        }
        else
        {
          v38 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
          if ( v38 == -1 )
          {
            v39 = 0;
          }
          else
          {
            v39 = *(DWORD *)(DAT_055c9bcc + 4 * v38);
          }
          v40 = *(BYTE *)(v39 + 1) + 1;
          *(BYTE *)(v39 + 1) = v40;
          if ( v40 < 2u )
          {
            Packet_DecryptByte(&g_byPacketSerialSend, v39);
          }
        }
        v42 = g_byPacketSerialSend;
        v110[v37 + 1024] = g_byPacketSerialSend;
        g_byPacketSerialSend = v42 + 1;
        if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) != -1 )
        {
          v43 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
          v44 = v43 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v43);
          v45 = v44[1] - 1;
          v44[1] = v45;
          if ( !v45 )
          {
            Packet_EncryptByte(v44, &g_byPacketSerialSend);
          }
        }
        v46 = v37 - 1;
        v47 = v36 - v46;
        v48 = &v111[v46];
        v49 = CSimpleModulus_Encode(0, (int)&v111[v46], v36 - v46);
        if ( v49 >= 256 )
        {
          v59 = v49 + 3;
          v107 = -60;
          v109 = v49 + 3;
          v108 = (v49 + 3) / 256;
          CSimpleModulus_Encode((int)v110, (int)v48, v47);
          v60 = 0;
          v52 = v59;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v61 = send(s, &v107 + v60, v59 - v60, 0);
              v62 = v61;
              if ( v61 == -1 )
              {
                break;
              }
              if ( v61 )
              {
                if ( SocketClientLogPrint )
                {
                  nullsub_2(&v107, v61);
                }
                v52 -= v62;
                v60 += v62;
                if ( (int)v52 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_105;
            }
            if ( WSAGetLastError() == 10035 && (int)(SocketClientSendBufferLength + v59) <= 0x2000 )
            {
              v63 = (char *)&SocketClientSendBuffer + SocketClientSendBufferLength;
              qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &v107, 4 * (v52 >> 2));
              v57 = &v107 + 4 * (v52 >> 2);
              v56 = &v63[4 * (v52 >> 2)];
              v58 = v52;
              goto LABEL_104;
            }
            goto LABEL_102;
          }
        }
        else
        {
          v50 = v49 + 2;
          buf = -61;
          v105 = v49 + 2;
          CSimpleModulus_Encode((int)v106, (int)&v111[v46], v47);
          v51 = 0;
          v52 = v50;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v53 = send(s, &buf + v51, v50 - v51, 0);
              v54 = v53;
              if ( v53 == -1 )
              {
                break;
              }
              if ( v53 )
              {
                if ( SocketClientLogPrint )
                {
                  nullsub_2(&buf, v53);
                }
                v52 -= v54;
                v51 += v54;
                if ( (int)v52 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_105;
            }
            if ( WSAGetLastError() == 10035 && (int)(SocketClientSendBufferLength + v50) <= 0x2000 )
            {
              v55 = (char *)&SocketClientSendBuffer + SocketClientSendBufferLength;
              qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf, 4 * (v52 >> 2));
              v57 = &buf + 4 * (v52 >> 2);
              v56 = &v55[4 * (v52 >> 2)];
              v58 = v52;
LABEL_104:
              qmemcpy(v56, v57, v58 & 3);
              SocketClientSendBufferLength += v52;
              goto LABEL_105;
            }
LABEL_102:
            CWsctlc::Close((DWORD)&SocketClient);
          }
        }
LABEL_105:
        v112 = -1;
        v102 = &DAT_00552460;
        goto LABEL_106;
      }
    }
  }
}
#endif
