// 0x004D23B0 FUN_004d23b0 — nunca activado: IDA_PORT_004D23B0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_004d23b0 (IDA-only, gated) ──
#if defined(IDA_PORT_004D23B0)
void __cdecl FUN_004d23b0(char *a1, int a2, short *a3, signed int a4, int a5, char a6)
{
  signed int v6; // ebx
  int v7; // edx
  int v8; // eax
  char *v9; // ecx
  ITEM_ATTRIBUTE *v10; // eax
  unsigned char *v11; // edi
  int Width; // ebp
  WORD *Height; // ecx
  int v14; // eax
  int v15; // esi
  int v16; // edx
  int v17; // eax
  short v18; // cx
  int v19; // esi
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int v23; // eax
  int v24; // ecx
  int v25; // ebp
  int v26; // esi
  BYTE v27; // al
  BYTE *v28; // eax
  char v29; // cl
  int v30; // esi
  int v31; // edi
  char *v32; // ebp
  int v33; // eax
  signed int v34; // ebx
  int v35; // edi
  signed int v36; // ebp
  int v37; // eax
  int v38; // esi
  int v39; // eax
  signed int v40; // esi
  int v41; // ebp
  signed int v42; // ebx
  int v43; // eax
  int v44; // edi
  int v45; // eax
  bool v46; // cc
  int v47; // ebx
  int v48; // ecx
  int v49; // ebx
  int v50; // esi
  BYTE v51; // al
  int v52; // edi
  char *v53; // esi
  int v54; // eax
  unsigned int v55; // ebx
  int v56; // edi
  unsigned int v57; // ebp
  int v58; // eax
  int v59; // esi
  char *v60; // edi
  char *v61; // edi
  char *v62; // esi
  char v63; // cl
  unsigned int v64; // ebx
  int v65; // edi
  int v66; // eax
  int v67; // esi
  char *v68; // edi
  int v69; // ebx
  int v70; // ecx
  int v71; // eax
  bool v72; // bl
  BYTE *v73; // eax
  char v74; // cl
  int v75; // eax
  int v76; // ebp
  int v77; // edi
  char *v78; // eax
  char v79; // cl
  int v80; // eax
  BYTE v81; // al
  int v82; // edi
  int v83; // esi
  char *v84; // ebp
  int v85; // eax
  signed int v86; // ebx
  int v87; // edi
  signed int v88; // ebp
  int v89; // eax
  int v90; // esi
  int v91; // eax
  signed int v92; // edi
  int v93; // ebp
  signed int v94; // ebx
  int v95; // eax
  int v96; // esi
  int *v97; // ebp
  short v98; // ax
  int v99; // ecx
  bool v100; // zf
  char *v101; // eax
  char v102; // cl
  int v103; // eax
  bool v104; // bl
  int v105; // ecx
  int v106; // ecx
  int v107; // eax
  int v108; // ebp
  int v109; // edi
  char *v110; // eax
  char v111; // cl
  int v112; // eax
  BYTE v113; // al
  int v114; // edi
  int v115; // esi
  char *v116; // ebp
  int v117; // eax
  signed int v118; // ebx
  int v119; // edi
  signed int v120; // ebp
  int v121; // eax
  int v122; // esi
  int v123; // eax
  signed int v124; // edi
  int v125; // ebp
  signed int v126; // ebx
  int v127; // eax
  int v128; // esi
  short v129; // ax
  int v130; // ecx
  char v131; // al
  int v132; // ecx
  int v133; // eax
  int v134; // ebp
  int v135; // edi
  signed int v136; // ebx
  int v137; // eax
  int v138; // esi
  int v139; // ecx
  char v140; // al
  int v141; // ecx
  int v142; // eax
  int v143; // ebp
  int v144; // edi
  int v145; // eax
  int v146; // esi
  unsigned int v147; // eax
  char *v148; // eax
  char v149; // cl
  int v150; // eax
  bool v151; // bl
  int v152; // ecx
  int v153; // ecx
  int v154; // eax
  int v155; // ebp
  int v156; // esi
  char *v157; // eax
  char v158; // cl
  int v159; // eax
  BYTE v160; // al
  BYTE *v161; // eax
  char v162; // cl
  int v163; // esi
  int v164; // edi
  char *v165; // ebp
  int v166; // eax
  signed int v167; // ebx
  int v168; // edi
  signed int v169; // ebp
  int v170; // eax
  int v171; // esi
  int v172; // eax
  signed int v173; // esi
  int v174; // ebp
  signed int v175; // ebx
  int v176; // eax
  int v177; // edi
  void *v178; // ebx
  unsigned int v179; // eax
  bool v180; // cf
  int v181; // eax
  unsigned int v182; // eax
  char *v183; // eax
  unsigned int v184; // eax
  BYTE *v185; // esi
  unsigned char v186; // al
  void *v187; // ebp
  unsigned int v188; // ecx
  int v189; // esi
  char v190; // cl
  int v191; // eax
  bool v192; // bl
  unsigned int v193; // eax
  BYTE *v194; // eax
  char v195; // cl
  int v196; // eax
  int v197; // eax
  int v198; // ecx
  int v199; // ebx
  int v200; // esi
  unsigned int v201; // eax
  char *v202; // eax
  char v203; // cl
  int v204; // eax
  BYTE v205; // al
  unsigned int v206; // eax
  BYTE *v207; // eax
  char v208; // cl
  int v209; // esi
  int v210; // edi
  char *v211; // ebp
  int v212; // eax
  signed int v213; // ebx
  int v214; // edi
  signed int v215; // ebp
  int v216; // eax
  int v217; // esi
  int v218; // eax
  signed int v219; // esi
  int v220; // ebp
  signed int v221; // ebx
  int v222; // eax
  int v223; // edi
  short v224; // ax
  unsigned int v225; // edx
  int v226; // ebp
  unsigned int v227; // eax
  unsigned int v228; // ebp
  short *v229; // eax
  char v230; // cl
  char *v231; // eax
  char *v232; // ebp
  BYTE *v233; // ebx
  unsigned int v234; // esi
  int v235; // ebp
  BYTE *v236; // edi
  char v237; // al
  unsigned int v238; // eax
  unsigned int v239; // ebx
  unsigned int v240; // eax
  unsigned int v241; // ebx
  unsigned int v242; // eax
  unsigned int v243; // ebx
  BYTE *v244; // esi
  unsigned char v245; // al
  void *v246; // ebp
  unsigned int v247; // ecx
  int v248; // esi
  unsigned int v249; // edx
  int v250; // ebx
  unsigned int v251; // edx
  int v252; // ebx
  char *v253; // edx
  char v254; // al
  char *v255; // eax
  char *v256; // ebx
  BYTE *v257; // ebp
  unsigned int v258; // esi
  int v259; // ebx
  BYTE *v260; // edi
  char v261; // al
  int v262; // eax
  char v263; // [esp+10h] [ebp-D84h]
  char v264; // [esp+11h] [ebp-D83h]
  char v265; // [esp+12h] [ebp-D82h]
  char v266; // [esp+13h] [ebp-D81h]
  char v267; // [esp+14h] [ebp-D80h]
  char v268; // [esp+15h] [ebp-D7Fh]
  char v269; // [esp+16h] [ebp-D7Eh]
  char v270; // [esp+17h] [ebp-D7Dh]
  char v271; // [esp+18h] [ebp-D7Ch]
  char v272; // [esp+19h] [ebp-D7Bh]
  char v273; // [esp+1Ah] [ebp-D7Ah]
  char v274; // [esp+1Bh] [ebp-D79h]
  char v275; // [esp+1Ch] [ebp-D78h]
  char v276; // [esp+1Dh] [ebp-D77h]
  char v277; // [esp+1Eh] [ebp-D76h]
  char v278; // [esp+1Fh] [ebp-D75h]
  int v279; // [esp+20h] [ebp-D74h]
  int v280; // [esp+24h] [ebp-D70h]
  int v281; // [esp+28h] [ebp-D6Ch]
  int v282; // [esp+2Ch] [ebp-D68h]
  int v283; // [esp+30h] [ebp-D64h] BYREF
  WORD *v284; // [esp+34h] [ebp-D60h] BYREF
  short *v285; // [esp+38h] [ebp-D5Ch]
  char *v286; // [esp+3Ch] [ebp-D58h]
  int v287; // [esp+40h] [ebp-D54h]
  short *v288; // [esp+44h] [ebp-D50h]
  char *v289; // [esp+48h] [ebp-D4Ch]
  int v290; // [esp+4Ch] [ebp-D48h]
  char v291[32]; // [esp+50h] [ebp-D44h] BYREF
  unsigned char *v292; // [esp+70h] [ebp-D24h]
  void *(__cdecl **v293)(std::locale::facet *__hidden, unsigned int); // [esp+74h] [ebp-D20h]
  unsigned short v294; // [esp+78h] [ebp-D1Ch]
  unsigned char v295; // [esp+7Ah] [ebp-D1Ah] BYREF
  WORD v296[512]; // [esp+7Bh] [ebp-D19h] BYREF
  char buf; // [esp+47Ch] [ebp-918h] BYREF
  char v298; // [esp+47Dh] [ebp-917h]
  char v299[258]; // [esp+47Eh] [ebp-916h] BYREF
  char v300[4]; // [esp+580h] [ebp-814h] BYREF
  char v301[5]; // [esp+584h] [ebp-810h] BYREF
  char v302; // [esp+589h] [ebp-80Bh]
  char v304[1024]; // [esp+988h] [ebp-40Ch] BYREF
  int v305; // [esp+D90h] [ebp-4h]

  if ( EnableUse > 0 )
  {
    return;
  }
  if ( EquipmentItem )
  {
    return;
  }
  if ( DAT_07e91388 > 0 )
  {
    return;
  }
  v286 = 0;
  if ( a5 <= 0 )
  {
    return;
  }
  v6 = a4;
  v7 = a2;
  v287 = a2;
  v8 = 68 * a4;
  v283 = 68 * a4;
  v288 = a3;
  while ( 1 )
  {
    v290 = 0;
    if ( v6 > 0 )
    {
      break;
    }
LABEL_87:
    v7 += 20;
    v46 = (int)++v286 < a5;
    v287 = v7;
    v288 = (short *)((char *)v288 + v8);
    if ( !v46 )
    {
      return;
    }
  }
  v9 = a1;
  v289 = a1;
  v285 = v288;
  while ( 1 )
  {
    if ( MouseX < (int)v9 || MouseX >= (int)(v9 + 20) || MouseY < v7 || MouseY >= v7 + 20 || *v285 == -1 )
    {
      goto LABEL_85;
    }
    v10 = &ItemAttribute[*v285];
    v11 = (unsigned char *)v285 + 63;
    Width = v10->Width;
    Height = (WORD *)v10->Height;
    v14 = *((unsigned char *)v285 + 63);
    v284 = Height;
    v15 = v14;
    if ( v14 < (int)Height + v14 )
    {
      v292 = (unsigned char *)(v285 + 31);
      v16 = v6 * v14;
      do
      {
        v17 = *v292;
        if ( v17 < v17 + Width )
        {
          do
          {
            if ( a3 != (short *)Inventory || (BYTE)(a3[34 * v16 + 32 + 34 * v17]) != 99 )
            {
              (BYTE)(a3[34 * v16 + 32 + 34 * v17]) = 2;
            }
            ++v17;
          }
          while ( v17 < Width + *v292 );
          Height = v284;
        }
        ++v15;
        v16 += a4;
      }
      while ( v15 < (int)Height + *v11 );
    }
    CheckInventory = (DWORD)v285;
    v18 = *v285;
    v19 = *((unsigned char *)v285 + 62);
    v20 = ItemAttribute[*v285].Width;
    DAT_07ea9844 = a6;
    sx = (int)&a1[20 * v19 + 20 * v20 / 2];
    v21 = *v11;
    sy = a2 + 20 * v21;
    if ( a3 == (short *)Inventory )
    {
      goto LABEL_85;
    }
    if ( a6 )
    {
      if ( !MouseLButtonPush )
      {
        return;
      }
      if ( *(DWORD *)&RepairEnable_0 )
      {
        return;
      }
      MouseLButtonPush = 0;
      if ( BuyCost )
      {
        return;
      }
      v47 = *((unsigned char *)v285 + 62) + a4 * *((unsigned char *)v285 + 63);
      v283 = ItemValue((int)&a3[34 * v47], 0);
      if ( BuyCost )
      {
        return;
      }
      v293 = &DAT_00552460;
      v295 = -63;
      v305 = 0;
      v296[0] = 12801;
      v294 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      (BYTE)(v296[1]) = v47;
      v48 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do
      {
        *(&v295 + v48) ^= *((BYTE *)&v294 + v48 + 1) ^ *(&v263 + v48 % 32);
        ++v48;
      }
      while ( v48 != 4 );
      ++v294;
      if ( v295 == 193 )
      {
        (BYTE)(v296[0]) = v294;
      }
      else if ( v295 == 194 )
      {
        v296[0] = v294;
      }
      v49 = v294;
      qmemcpy(v304, &v295, v294);
      v304[v49] = rand();
      v50 = (v304[0] != -63) + 2;
      PACKET_DECRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
      v51 = g_byPacketSerialSend;
      v304[v50 - 1] = g_byPacketSerialSend;
      g_byPacketSerialSend = v51 + 1;
      PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
      --v50;
      v52 = v49 - v50;
      v53 = &v304[v50];
      v54 = CSimpleModulus_Encode(0, (int)v53, v52);
      if ( v54 >= 256 )
      {
        v64 = v54 + 3;
        v300[0] = -60;
        v300[2] = v54 + 3;
        v300[1] = (v54 + 3) / 256;
        CSimpleModulus_Encode((int)&v300[3], (int)v53, v52);
        v65 = 0;
        v57 = v64;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v66 = send(s, &v300[v65], v64 - v65, 0);
            v67 = v66;
            if ( v66 == -1 )
            {
              break;
            }
            if ( v66 )
            {
              if ( SocketClientLogPrint )
              {
                nullsub_2((int)v300, v66);
              }
              v57 -= v67;
              v65 += v67;
              if ( (int)v57 > 0 )
              {
                continue;
              }
            }
            goto LABEL_123;
          }
          if ( WSAGetLastError() == 10035 && (int)(SocketClientSendBufferLength + v64) <= 0x2000 )
          {
            v68 = (char *)&SocketClientSendBuffer + SocketClientSendBufferLength;
            qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, v300, 4 * (v57 >> 2));
            v62 = &v300[4 * (v57 >> 2)];
            v61 = &v68[4 * (v57 >> 2)];
            v63 = v57;
            goto LABEL_122;
          }
LABEL_120:
          CWsctlc::Close((DWORD)&SocketClient);
          BuyCost = v283;
          return;
        }
      }
      else
      {
        v55 = v54 + 2;
        buf = -61;
        v298 = v54 + 2;
        CSimpleModulus_Encode((int)v299, (int)v53, v52);
        v56 = 0;
        v57 = v55;
        if ( s != -1 )
        {
          while ( 1 )
          {
            v58 = send(s, &buf + v56, v55 - v56, 0);
            v59 = v58;
            if ( v58 == -1 )
            {
              break;
            }
            if ( v58 )
            {
              if ( SocketClientLogPrint )
              {
                nullsub_2((int)&buf, v58);
              }
              v57 -= v59;
              v56 += v59;
              if ( (int)v57 > 0 )
              {
                continue;
              }
            }
            goto LABEL_123;
          }
          if ( WSAGetLastError() == 10035 && (int)(SocketClientSendBufferLength + v55) <= 0x2000 )
          {
            v60 = (char *)&SocketClientSendBuffer + SocketClientSendBufferLength;
            qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf, 4 * (v57 >> 2));
            v62 = &buf + 4 * (v57 >> 2);
            v61 = &v60[4 * (v57 >> 2)];
            v63 = v57;
LABEL_122:
            qmemcpy(v61, v62, v63 & 3);
            SocketClientSendBufferLength += v57;
            goto LABEL_123;
          }
          goto LABEL_120;
        }
      }
LABEL_123:
      BuyCost = v283;
      return;
    }
    if ( *(DWORD *)&RepairEnable_0 )
    {
      if ( (v18 < 416 || v18 > 419)
        && v18 != 426
        && v18 != 135
        && v18 != 143
        && v18 < 448
        && (v18 < 391 || v18 > 403)
        && (v18 < 430 || v18 > 435)
        && MouseLButtonPush )
      {
        MouseLButtonPush = 0;
        v293 = &DAT_00552460;
        v295 = -63;
        v305 = 1;
        v296[0] = 13313;
        v294 = 3;
        qmemcpy(v291, "çm:", 3);
        (BYTE)(v296[1]) = a4 * v21 + v19 + 12;
        v22 = 3;
        v291[3] = -119;
        v291[4] = -68;
        v291[5] = -78;
        v291[6] = -97;
        v291[7] = 115;
        v291[8] = 35;
        v291[9] = -88;
        v291[10] = -2;
        v291[11] = -74;
        v291[12] = 73;
        v291[13] = 93;
        v291[14] = 57;
        v291[15] = 93;
        v291[16] = -118;
        v291[17] = -53;
        v291[18] = 99;
        v291[19] = -115;
        v291[20] = -22;
        v291[21] = 125;
        v291[22] = 43;
        v291[23] = 95;
        v291[24] = -61;
        v291[25] = -79;
        v291[26] = -23;
        v291[27] = -125;
        v291[28] = 41;
        v291[29] = 81;
        v291[30] = -24;
        v291[31] = 86;
        do
        {
          *(&v295 + v22) ^= *((BYTE *)&v294 + v22 + 1) ^ v291[v22 % 32];
          ++v22;
        }
        while ( v22 != 4 );
        if ( ++v294 + 1 <= 1024 )
        {
          v263 = -25;
          v264 = 109;
          v265 = 58;
          *(&v295 + v294) = RepairEnable;
          v23 = v294;
          v266 = -119;
          v267 = -68;
          v268 = -78;
          v24 = v294 + 1;
          v269 = -97;
          v270 = 115;
          v271 = 35;
          v272 = -88;
          v273 = -2;
          v274 = -74;
          v275 = 73;
          v276 = 93;
          v277 = 57;
          v278 = 93;
          v279 = -1922839670;
          v280 = 1596685802;
          v281 = -2081836605;
          v282 = 1458065705;
          if ( v294 != v24 )
          {
            do
            {
              *(&v295 + v23) ^= *((BYTE *)&v294 + v23 + 1) ^ *(&v263 + v23 % 32);
              ++v23;
            }
            while ( v23 != v24 );
          }
          ++v294;
        }
        if ( v295 == 193 )
        {
          (BYTE)(v296[0]) = v294;
        }
        else if ( v295 == 194 )
        {
          v296[0] = v294;
        }
        v25 = v294;
        qmemcpy(v304, &v295, v294);
        v304[v25] = rand();
        v26 = (v304[0] != -63) + 2;
        PACKET_DECRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
        v27 = g_byPacketSerialSend;
        v304[v26 - 1] = g_byPacketSerialSend;
        g_byPacketSerialSend = v27 + 1;
        if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) != -1 )
        {
          v28 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
          v29 = v28[1] - 1;
          v28[1] = v29;
          if ( !v29 )
          {
            Packet_EncryptByte(v28, &g_byPacketSerialSend);
          }
        }
        v30 = v26 - 1;
        v31 = v25 - v30;
        v32 = &v304[v30];
        v33 = CSimpleModulus_Encode(0, (int)&v304[v30], v31);
        if ( v33 >= 256 )
        {
          v40 = v33 + 3;
          v300[0] = -60;
          v300[2] = v33 + 3;
          v300[1] = (v33 + 3) / 256;
          CSimpleModulus_Encode((int)&v300[3], (int)v32, v31);
          v41 = 0;
          v42 = v40;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v43 = send(s, &v300[v41], v40 - v41, 0);
              v44 = v43;
              if ( v43 == -1 )
              {
                break;
              }
              if ( v43 )
              {
                if ( SocketClientLogPrint )
                {
                  nullsub_2((int)v300, v43);
                }
                v42 -= v44;
                v41 += v44;
                if ( v42 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_74;
            }
            if ( WSAGetLastError() != 10035 || SocketClientSendBufferLength + v40 > 0x2000 )
            {
              goto LABEL_71;
            }
            qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, v300, v42);
            v39 = v42 + SocketClientSendBufferLength;
LABEL_73:
            SocketClientSendBufferLength = v39;
          }
        }
        else
        {
          v34 = v33 + 2;
          buf = -61;
          v298 = v33 + 2;
          CSimpleModulus_Encode((int)v299, (int)&v304[v30], v31);
          v35 = 0;
          v36 = v34;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v37 = send(s, &buf + v35, v34 - v35, 0);
              v38 = v37;
              if ( v37 == -1 )
              {
                break;
              }
              if ( v37 )
              {
                if ( SocketClientLogPrint )
                {
                  nullsub_2((int)&buf, v37);
                }
                v36 -= v38;
                v35 += v38;
                if ( v36 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_74;
            }
            if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v34 <= 0x2000 )
            {
              qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf, v36);
              v39 = v36 + SocketClientSendBufferLength;
              goto LABEL_73;
            }
LABEL_71:
            CWsctlc::Close((DWORD)&SocketClient);
          }
        }
LABEL_74:
        v305 = -1;
        v293 = &DAT_00552460;
        goto LABEL_85;
      }
      goto LABEL_85;
    }
    if ( MouseLButtonPush )
    {
      v69 = *((unsigned char *)v285 + 62);
      v70 = a4 * *((unsigned char *)v285 + 63);
      MouseLButtonPush = 0;
      v71 = v70 + v69;
      DAT_07ea9800 = (int)a3;
      qmemcpy(&pPickedItem, &a3[34 * v70 + 34 * v69], 0x44u);
      if ( a3 == (short *)&OffsetInventoryItems )
      {
        v71 += 12;
      }
      *(DWORD *)&Inventory[32].Type = v71;
      UI_Main(v71, a3, a4);
      CheckInventory = 0;
      HashTable_Insert_Short(&MAIN_HASH_CLASS, &TradeOpened);
      v72 = TradeOpened;
      if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&TradeOpened) != -1 )
      {
        v73 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)&TradeOpened);
        v74 = v73[1] - 1;
        v73[1] = v74;
        if ( !v74 )
        {
          Packet_EncryptByte(v73, &TradeOpened);
        }
      }
      if ( v72 && m_bMyConfirm && a3 == OffsetTradeItems )
      {
        m_bMyConfirm = 0;
        v293 = &DAT_00552460;
        v295 = -63;
        v305 = 2;
        (BYTE)(v296[0]) = 1;
        strcpy((char *)v296 + 1, "<");
        v294 = 3;
        v263 = -25;
        v264 = 109;
        v265 = 58;
        v75 = 3;
        v266 = -119;
        v267 = -68;
        v268 = -78;
        v269 = -97;
        v270 = 115;
        v271 = 35;
        v272 = -88;
        v273 = -2;
        v274 = -74;
        v275 = 73;
        v276 = 93;
        v277 = 57;
        v278 = 93;
        v279 = -1922839670;
        v280 = 1596685802;
        v281 = -2081836605;
        v282 = 1458065705;
        do
        {
          *(&v295 + v75) ^= *((BYTE *)&v294 + v75 + 1) ^ *(&v263 + v75 % 32);
          ++v75;
        }
        while ( v75 != 4 );
        ++v294;
        if ( v295 == 193 )
        {
          (BYTE)(v296[0]) = v294;
        }
        else if ( v295 == 194 )
        {
          v296[0] = v294;
        }
        v76 = v294;
        qmemcpy(v304, &v295, v294);
        v304[v76] = rand();
        v77 = (v304[0] != -63) + 2;
        if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) == -1 )
        {
          v80 = operator_new(2u);
          *(BYTE *)(v80 + 1) = 1;
          HashTable_Insert(&MAIN_HASH_CLASS, v80, (int)&g_byPacketSerialSend);
        }
        else
        {
          v78 = (char *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
          v79 = v78[1] + 1;
          v78[1] = v79;
          if ( (unsigned char)v79 < 2u )
          {
            Packet_DecryptByte(&g_byPacketSerialSend, v78);
          }
        }
        v81 = g_byPacketSerialSend;
        v304[v77 - 1] = g_byPacketSerialSend;
        g_byPacketSerialSend = v81 + 1;
        PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
        v82 = v77 - 1;
        v83 = v76 - v82;
        v84 = &v304[v82];
        v85 = CSimpleModulus_Encode(0, (int)&v304[v82], v83);
        if ( v85 >= 256 )
        {
          v92 = v85 + 3;
          v300[0] = -60;
          v300[2] = v85 + 3;
          v300[1] = (v85 + 3) / 256;
          CSimpleModulus_Encode((int)&v300[3], (int)v84, v83);
          v93 = 0;
          v94 = v92;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v95 = send(s, &v300[v93], v92 - v93, 0);
              v96 = v95;
              if ( v95 == -1 )
              {
                break;
              }
              if ( v95 )
              {
                if ( SocketClientLogPrint )
                {
                  nullsub_2((int)v300, v95);
                }
                v94 -= v96;
                v93 += v96;
                if ( v94 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_166;
            }
            if ( WSAGetLastError() != 10035 || SocketClientSendBufferLength + v92 > 0x2000 )
            {
              goto LABEL_163;
            }
            qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, v300, v94);
            v91 = v94 + SocketClientSendBufferLength;
LABEL_165:
            SocketClientSendBufferLength = v91;
          }
        }
        else
        {
          v86 = v85 + 2;
          buf = -61;
          v298 = v85 + 2;
          CSimpleModulus_Encode((int)v299, (int)&v304[v82], v83);
          v87 = 0;
          v88 = v86;
          if ( s != -1 )
          {
            while ( 1 )
            {
              v89 = send(s, &buf + v87, v86 - v87, 0);
              v90 = v89;
              if ( v89 == -1 )
              {
                break;
              }
              if ( v89 )
              {
                if ( SocketClientLogPrint )
                {
                  nullsub_2((int)&buf, v89);
                }
                v88 -= v90;
                v87 += v90;
                if ( v88 > 0 )
                {
                  continue;
                }
              }
              goto LABEL_166;
            }
            if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v86 <= 0x2000 )
            {
              qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf, v88);
              v91 = v88 + SocketClientSendBufferLength;
              goto LABEL_165;
            }
LABEL_163:
            CWsctlc::Close((DWORD)&SocketClient);
          }
        }
LABEL_166:
        v305 = -1;
      }
      PlayBuffer(29, 0, 0);
      return;
    }
    if ( MouseRButtonPush )
    {
      break;
    }
    if ( !((unsigned short)GetAsyncKeyState(17) >> 8) )
    {
      goto LABEL_85;
    }
    if ( PressKey(81) )
    {
      v45 = 0;
    }
    else if ( PressKey(87) )
    {
      v45 = 1;
    }
    else
    {
      if ( !PressKey(69) )
      {
        goto LABEL_85;
      }
      v45 = 2;
    }
    DAT_00559c60[v45] = *(short *)CheckInventory;
LABEL_85:
    v6 = a4;
    v9 = v289 + 20;
    v46 = ++v290 < a4;
    v289 += 20;
    v285 += 34;
    if ( !v46 )
    {
      v8 = v283;
      v7 = v287;
      goto LABEL_87;
    }
    v7 = v287;
  }
  v97 = (int *)v285;
  v98 = *v285;
  v99 = *((unsigned char *)v285 + 62) + a4 * *((unsigned char *)v285 + 63);
  v100 = *v285 == 458;
  MouseRButtonPush = 0;
  v287 = v99;
  if ( !v100 )
  {
    if ( v98 == 467 )
    {
      *(DWORD *)v300 = &DAT_00552460;
      *(WORD *)&v301[2] = 449;
      v305 = 4;
      v301[4] = -111;
      *(WORD *)v301 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      v302 = 1;
      v130 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do
      {
        v301[v130 + 2] ^= v301[v130 + 1] ^ *(&v263 + v130 % 32);
        ++v130;
      }
      while ( v130 != 4 );
      ++*(WORD *)v301;
      v131 = (v97[1] >> 3) & 0xF;
      if ( *(unsigned short *)v301 + 1 <= 1024 )
      {
        v263 = -25;
        v264 = 109;
        v265 = 58;
        v301[*(unsigned short *)v301 + 2] = v131;
        v132 = *(unsigned short *)v301;
        v266 = -119;
        v133 = *(unsigned short *)v301 + 1;
        v267 = -68;
        v268 = -78;
        v269 = -97;
        v270 = 115;
        v271 = 35;
        v272 = -88;
        v273 = -2;
        v274 = -74;
        v275 = 73;
        v276 = 93;
        v277 = 57;
        v278 = 93;
        v279 = -1922839670;
        v280 = 1596685802;
        v281 = -2081836605;
        v282 = 1458065705;
        if ( *(unsigned short *)v301 != v133 )
        {
          do
          {
            v301[v132 + 2] ^= v301[v132 + 1] ^ *(&v263 + v132 % 32);
            ++v132;
          }
          while ( v132 != v133 );
        }
        ++*(WORD *)v301;
      }
      if ( (unsigned char)v301[2] == 193 )
      {
        v301[3] = v301[0];
      }
      else if ( (unsigned char)v301[2] == 194 )
      {
        *(WORD *)&v301[3] = *(WORD *)v301;
      }
      v134 = *(unsigned short *)v301;
      v135 = 0;
      v136 = *(unsigned short *)v301;
      if ( s == -1 )
      {
        return;
      }
      while ( 1 )
      {
        v137 = send(s, &v301[v135 + 2], v134 - v135, 0);
        v138 = v137;
        if ( v137 == -1 )
        {
          break;
        }
        if ( v137 )
        {
          if ( SocketClientLogPrint )
          {
            nullsub_2((int)&v301[2], v137);
          }
          v136 -= v138;
          v135 += v138;
          if ( v136 > 0 )
          {
            continue;
          }
        }
        return;
      }
      if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v134 <= 0x2000 )
      {
        goto LABEL_239;
      }
      goto LABEL_260;
    }
    if ( v98 == 434 )
    {
      *(DWORD *)v300 = &DAT_00552460;
      *(WORD *)&v301[2] = 449;
      v305 = 5;
      v301[4] = -111;
      *(WORD *)v301 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      v302 = 2;
      v139 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do...
      ++*(WORD *)v301;
      v140 = (v97[1] >> 3) & 0xF;
      if...
      if...
      v143 = *(unsigned short *)v301;
      v144 = 0;
      v136 = *(unsigned short *)v301;
      if...
      while...
      if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v143 <= 0x2000 )
      {
LABEL_239:
        qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &v301[2], v136);
        SocketClientSendBufferLength += v136;
        return;
      }
LABEL_260:
      CWsctlc::Close((DWORD)&SocketClient);
      return;
    }
    if ( v98 >= 448 && v98 <= 454 || v98 >= 456 && v98 <= 457 || v98 == 468 )
    {
      if ( WarehouseOpened )
      {
        goto LABEL_322;
      }
      if...
      v151 = TradeOpened;
      PACKET_ENCRYPT(&MAIN_HASH_CLASS, &TradeOpened);
      if ( v151 )
      {
LABEL_322:
        UIChatLogWindow_AddText(DAT_07eaa188, GlobalText[474], 2);
        return;
      }
      if ( EnableUse > 0 )
      {
        return;
      }
      EnableUse = 10;
      v293 = &DAT_00552460;
      v295 = -63;
      v305 = 6;
      v296[0] = 9729;
      v294 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      (BYTE)(v296[1]) = v287 + 12;
      v152 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do...
      if...
      if...
      v155 = v294;
      qmemcpy(v304, &v295, v294);
      v304[v155] = rand();
      v156 = (v304[0] != -63) + 2;
      if...
      v160 = g_byPacketSerialSend;
      v304[v156 - 1] = g_byPacketSerialSend;
      g_byPacketSerialSend = v160 + 1;
      if...
      v163 = v156 - 1;
      v164 = v155 - v163;
      v165 = &v304[v163];
      v166 = CSimpleModulus_Encode(0, (int)&v304[v163], v164);
      if...
      CWsctlc::Close((DWORD)&SocketClient);
      goto LABEL_318;
    }
    if ( v98 >= 480 && v98 < 512 || v98 >= 391 && v98 <= 398 || v98 >= 400 && v98 <= 403 )
    {
      v178 = (void *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v179 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
      v283 = 0;
      v286 = 0;
      if...
      v181 = operator_new(0x585u);
      *(BYTE *)(v181 + 1412) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v181, (int)v178);
LABEL_335:
      if ( (unsigned int)*(WORD *)(CharacterAttribute + 14) < ItemAttribute[*v285].RequireLevel
        || *(WORD *)(CharacterAttribute + 26) < (unsigned short)v285[16]
        || *(WORD *)(CharacterAttribute + 20) < (unsigned short)v285[14] )
      {
        goto LABEL_418;
      }
      if ( WarehouseOpened )
      {
        goto LABEL_417;
      }
      if...
      v192 = TradeOpened;
      if...
      if ( v192 )
      {
LABEL_417:
        UIChatLogWindow_AddText(DAT_07eaa18c, GlobalText[474], 2);
        goto LABEL_418;
      }
      if ( EnableUse > 0 )
      {
        goto LABEL_418;
      }
      EnableUse = 10;
      v293 = &DAT_00552460;
      v295 = -63;
      v305 = 7;
      v296[0] = 9729;
      v294 = 3;
      v263 = -25;
      v264 = 109;
      v265 = 58;
      (BYTE)(v296[1]) = v287 + 12;
      v196 = 3;
      v266 = -119;
      v267 = -68;
      v268 = -78;
      v269 = -97;
      v270 = 115;
      v271 = 35;
      v272 = -88;
      v273 = -2;
      v274 = -74;
      v275 = 73;
      v276 = 93;
      v277 = 57;
      v278 = 93;
      v279 = -1922839670;
      v280 = 1596685802;
      v281 = -2081836605;
      v282 = 1458065705;
      do...
      if...
      if...
      v199 = v294;
      qmemcpy(v304, &v295, v294);
      v304[v199] = rand();
      v200 = (v304[0] != -63) + 2;
      if...
      v205 = g_byPacketSerialSend;
      v304[v200 - 1] = g_byPacketSerialSend;
      g_byPacketSerialSend = v205 + 1;
      if...
      v209 = v200 - 1;
      v210 = v199 - v209;
      v211 = &v304[v209];
      v212 = CSimpleModulus_Encode(0, (int)&v304[v209], v199 - v209);
      if...
LABEL_411:
      v224 = *(&OffsetInventoryItems.Type + 34 * v287);
      if ( v224 == 448 )
      {
        PlayBuffer(33, 0, 0);
      }
      else if ( v224 >= 449 && v224 <= 457 )
      {
        PlayBuffer(32, 0, 0);
      }
      v305 = -1;
LABEL_418:
      v286 = (char *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v225 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
      v226 = 0;
      v283 = 0;
      if...
      return;
    }
    if ( v98 == 431 )
    {
      v285 = (short *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v238 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
      v239 = 0;
      v283 = 0;
      if...
      v286 = (char *)operator_new(0x585u);
      v286[1412] = 1;
      v283 = (int)v285;
      v240 = (*(int (__cdecl **)(int *, short *))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, v285);
      v241 = 0;
      v284 = 0;
      if...
LABEL_466:
      v290 = *(unsigned short *)(CharacterAttribute + 14);
      v286 = (char *)CharacterMachine;
      v284 = (WORD *)CharacterMachine;
      v249 = (*(int (__cdecl **)(int *, DWORD))(MAIN_HASH_CLASS + 12))(&MAIN_HASH_CLASS, CharacterMachine);
      v250 = 0;
      v283 = 0;
      if...
      if ( v290 < 10 )
      {
        CreateOkMessageBox(GlobalText[749]);
      }
      else
      {
        qmemcpy(&DAT_07ea5240, (const void *)(68 * v287 + 132809744), 0x44u);
        DAT_07ea5249 = v287;
        ShowCheckBox(1, 376, 153);
      }
      return;
    }
    DAT_07ea9800 = (int)a3;
    qmemcpy(&pPickedItem, &a3[34 * v99], 68u);
    DAT_083a42e0 = MouseX;
    DAT_083a42e4 = MouseY;
    if ( a3 == (short *)&OffsetInventoryItems )
    {
      if ( WarehouseOpened )
      {
        DAT_083a42eb = FUN_004d6020(DAT_07eaa0c8 + 15, DAT_07eaa0cc + 50, (int)&Inventory[32].WalkSpeed, 8, 15);
      }
      if ( DAT_083a42eb )
      {
        v262 = a4 * *((unsigned char *)v97 + 63) + *((unsigned char *)v97 + 62) + 12;
        goto LABEL_498;
      }
    }
    else
    {
      if ( WarehouseOpened )
      {
        DAT_083a42eb = FUN_004d6020(InventoryStartX + 15, InventoryStartY + 200, (int)&OffsetInventoryItems, 8, 8);
      }
      if ( DAT_083a42eb )
      {
        v262 = a4 * *((unsigned char *)v97 + 63) + *((unsigned char *)v97 + 62);
LABEL_498:
        *(DWORD *)&Inventory[32].Type = v262;
        UI_Main(v262, a3, a4);
        if ( DAT_083a42eb )
        {
          CheckInventory = 0;
          PlayBuffer(29, 0, 0);
          return;
        }
      }
    }
    memset(&pPickedItem, 0, 0x44u);
    return;
  }
  if ( Teleport )
  {
    return;
  }
  if ( WarehouseOpened )
  {
    goto LABEL_217;
  }
  if...
  v104 = TradeOpened;
  PACKET_ENCRYPT(&MAIN_HASH_CLASS, &TradeOpened);
  if ( v104 )
  {
LABEL_217:
    UIChatLogWindow_AddText(DAT_07eaa184, GlobalText[474], 2);
  }
  else
  {
    if ( EnableUse > 0 )
    {
      return;
    }
    EnableUse = 10;
    v293 = &DAT_00552460;
    v295 = -63;
    v305 = 3;
    v296[0] = 9729;
    v294 = 3;
    v263 = -25;
    v264 = 109;
    v265 = 58;
    (BYTE)(v296[1]) = v287 + 12;
    v105 = 3;
    v266 = -119;
    v267 = -68;
    v268 = -78;
    v269 = -97;
    v270 = 115;
    v271 = 35;
    v272 = -88;
    v273 = -2;
    v274 = -74;
    v275 = 73;
    v276 = 93;
    v277 = 57;
    v278 = 93;
    v279 = -1922839670;
    v280 = 1596685802;
    v281 = -2081836605;
    v282 = 1458065705;
    do...
    if...
    if...
    v108 = v294;
    qmemcpy(v304, &v295, v294);
    v304[v108] = rand();
    v109 = (v304[0] != -63) + 2;
    if...
    v113 = g_byPacketSerialSend;
    v304[v109 - 1] = g_byPacketSerialSend;
    g_byPacketSerialSend = v113 + 1;
    PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
    v114 = v109 - 1;
    v115 = v108 - v114;
    v116 = &v304[v114];
    v117 = CSimpleModulus_Encode(0, (int)&v304[v114], v115);
    if...
LABEL_215:
    v129 = *(&OffsetInventoryItems.Type + 34 * v287);
    if ( v129 == 448 )
    {
LABEL_216:
      PlayBuffer(33, 0, 0);
      return;
    }
LABEL_319:
    if ( v129 >= 449 && v129 <= 457 )
    {
      PlayBuffer(32, 0, 0);
    }
  }
}
#endif
