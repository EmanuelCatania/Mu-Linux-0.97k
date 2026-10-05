// 0x00485780 UseSkillWarrior — nunca activado: IDA_PORT_00485780 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── UseSkillWarrior (IDA-only, gated) ──
#if defined(IDA_PORT_00485780)
void __cdecl UseSkillWarrior(DWORD c, DWORD o)
{
  void *v2; // esi
  unsigned int v3; // eax
  BYTE *v4; // eax
  unsigned char v5; // cl
  int v6; // eax
  const void *v7; // esi
  unsigned int v8; // eax
  BYTE *v9; // eax
  char v10; // cl
  char *v11; // esi
  char v12; // al
  double v13; // st7
  int v14; // edx
  int i; // esi
  int v16; // ecx
  int j; // esi
  int v18; // ecx
  int k; // esi
  int v20; // ebp
  int v21; // edi
  int v22; // eax
  int v23; // esi
  bool v24; // zf
  int v25; // eax
  DWORD v26; // ecx
  int v27; // eax
  float v28; // edx
  double v29; // st7
  float v30; // ecx
  float v31; // eax
  DWORD TickCount; // eax
  double v33; // st7
  float v34; // edx
  float v35; // eax
  double v36; // st7
  double v37; // st7
  char *v38; // ebp
  char *v39; // esi
  int v40; // eax
  unsigned int v41; // kr14_4
  int v42; // edx
  unsigned int v43; // kr18_4
  int v44; // ecx
  int m; // esi
  int v46; // ecx
  int n; // esi
  int v48; // ecx
  int ii; // esi
  __int64 v50; // rax
  int v51; // ecx
  int jj; // esi
  int v53; // ecx
  int kk; // esi
  int v55; // ecx
  int mm; // esi
  int v57; // ebp
  int v58; // esi
  BYTE v59; // al
  int v60; // edi
  char *v61; // esi
  int v62; // eax
  signed int v63; // ebp
  int v64; // edi
  int v65; // eax
  int v66; // esi
  int v67; // eax
  float *v68; // ebp
  int v69; // edi
  int v70; // eax
  int v71; // esi
  int v72; // esi
  short v73; // dx
  void *v74; // esi
  unsigned int v75; // eax
  BYTE *v76; // eax
  unsigned char v77; // cl
  int v78; // eax
  BYTE *v79; // esi
  int v80; // eax
  unsigned int v81; // eax
  BYTE *v82; // eax
  char v83; // cl
  DWORD v84; // edx
  unsigned int v85; // kr1C_4
  int v86; // esi
  unsigned int v87; // kr20_4
  int v88; // ecx
  int v89; // eax
  int v90; // ecx
  int nn; // esi
  int v92; // ecx
  int i1; // esi
  int v94; // ecx
  int i2; // esi
  char v96; // cl
  int v97; // ecx
  int i3; // esi
  int v99; // edx
  int v100; // ecx
  int i4; // esi
  int *v102; // edi
  int v103; // ebp
  int v104; // eax
  int v105; // ecx
  int i5; // esi
  char v107; // cl
  int v108; // ecx
  int i6; // esi
  int v110; // ebp
  int v111; // edi
  char *v112; // eax
  char v113; // cl
  int v114; // eax
  BYTE v115; // al
  int v116; // edi
  int v117; // esi
  char *v118; // ebp
  int v119; // eax
  signed int v120; // ebp
  int v121; // edi
  int v122; // eax
  int v123; // esi
  int v124; // eax
  float *v125; // edi
  int v126; // ebp
  int v127; // eax
  int v128; // esi
  int v129; // eax
  float *__attribute__((__org_arrdim(0,3))) v130; // edx
  unsigned int v131; // ecx
  void *v132; // edi
  float v133; // edx
  double v134; // st7
  BYTE *v135; // eax
  unsigned char v136; // cl
  int v137; // eax
  float v138; // eax
  int v139; // edx
  void *v140; // edi
  BYTE *v141; // eax
  unsigned char v142; // cl
  int v143; // eax
  int v144; // eax
  int v145; // edi
  int v146; // eax
  int v147; // edi
  int v148; // eax
  BYTE *v149; // eax
  char v150; // cl
  int v151; // edi
  int v152; // eax
  BYTE *v153; // eax
  char v154; // cl
  float v155; // eax
  const void *v156; // edi
  int v157; // edx
  BYTE *v158; // eax
  char v159; // cl
  DWORD v160; // edx
  unsigned int v161; // kr24_4
  int v162; // esi
  unsigned int v163; // kr28_4
  int v164; // ecx
  int v165; // eax
  int v166; // ecx
  int i7; // esi
  int v168; // eax
  WORD *v169; // edi
  int v170; // eax
  int v171; // ecx
  int i8; // esi
  char v173; // cl
  int v174; // ecx
  int i9; // esi
  int v176; // ebp
  int v177; // edi
  char *v178; // eax
  char v179; // cl
  int v180; // eax
  BYTE v181; // al
  int v182; // edi
  int v183; // esi
  char *v184; // ebp
  int v185; // eax
  int v186; // edi
  int v187; // eax
  int v188; // esi
  float *v189; // edi
  int v190; // ebp
  int v191; // eax
  int v192; // esi
  int v193; // ecx
  double v194; // st7
  char v195; // cl
  unsigned char v196; // al
  BYTE v197; // al
  int v198; // esi
  int v199; // eax
  int v200; // ecx
  int v201; // eax
  int v202; // ebx
  int v203; // edi
  signed int v204; // ebp
  int v205; // eax
  int v206; // esi
  int v207; // [esp-4h] [ebp-DD4h]
  signed int Position; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positiona; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionj; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionb; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionc; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positiond; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positione; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionf; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positiong; // [esp+10h] [ebp-DC0h]
  float *__attribute__((__org_arrdim(0,3))) Positionh; // [esp+10h] [ebp-DC0h]
  unsigned char Positioni; // [esp+10h] [ebp-DC0h]
  char v219; // [esp+14h] [ebp-DBCh]
  char v220; // [esp+15h] [ebp-DBBh]
  char v221; // [esp+16h] [ebp-DBAh]
  char v222; // [esp+17h] [ebp-DB9h]
  char v223; // [esp+18h] [ebp-DB8h]
  char v224; // [esp+19h] [ebp-DB7h]
  char v225; // [esp+1Ah] [ebp-DB6h]
  char v226; // [esp+1Bh] [ebp-DB5h]
  char v227; // [esp+1Ch] [ebp-DB4h]
  char v228; // [esp+1Dh] [ebp-DB3h]
  char v229; // [esp+1Eh] [ebp-DB2h]
  char v230; // [esp+1Fh] [ebp-DB1h]
  char v231; // [esp+20h] [ebp-DB0h]
  char v232; // [esp+21h] [ebp-DAFh]
  char v233; // [esp+22h] [ebp-DAEh]
  char v234; // [esp+23h] [ebp-DADh]
  char v235; // [esp+24h] [ebp-DACh]
  char v236; // [esp+25h] [ebp-DABh]
  char v237; // [esp+26h] [ebp-DAAh]
  char v238; // [esp+27h] [ebp-DA9h]
  char v239; // [esp+28h] [ebp-DA8h]
  char v240; // [esp+29h] [ebp-DA7h]
  char v241; // [esp+2Ah] [ebp-DA6h]
  char v242; // [esp+2Bh] [ebp-DA5h]
  char v243; // [esp+2Ch] [ebp-DA4h]
  char v244; // [esp+2Dh] [ebp-DA3h]
  char v245; // [esp+2Eh] [ebp-DA2h]
  char v246; // [esp+2Fh] [ebp-DA1h]
  char v247; // [esp+30h] [ebp-DA0h]
  char v248; // [esp+31h] [ebp-D9Fh]
  char v249; // [esp+32h] [ebp-D9Eh]
  char v250; // [esp+33h] [ebp-D9Dh]
  char v251; // [esp+34h] [ebp-D9Ch]
  char v252; // [esp+35h] [ebp-D9Bh]
  char v253; // [esp+36h] [ebp-D9Ah]
  char v254; // [esp+37h] [ebp-D99h]
  char v255; // [esp+38h] [ebp-D98h]
  char v256; // [esp+39h] [ebp-D97h]
  char v257; // [esp+3Ah] [ebp-D96h]
  char v258; // [esp+3Bh] [ebp-D95h]
  char v259; // [esp+3Ch] [ebp-D94h]
  char v260; // [esp+3Dh] [ebp-D93h]
  char v261; // [esp+3Eh] [ebp-D92h]
  char v262; // [esp+3Fh] [ebp-D91h]
  char v263; // [esp+40h] [ebp-D90h]
  char v264; // [esp+41h] [ebp-D8Fh]
  char v265; // [esp+42h] [ebp-D8Eh]
  char v266; // [esp+43h] [ebp-D8Dh]
  char v267; // [esp+44h] [ebp-D8Ch]
  char v268; // [esp+45h] [ebp-D8Bh]
  char v269; // [esp+46h] [ebp-D8Ah]
  char v270; // [esp+47h] [ebp-D89h]
  char v271; // [esp+48h] [ebp-D88h]
  char v272; // [esp+49h] [ebp-D87h]
  char v273; // [esp+4Ah] [ebp-D86h]
  char v274; // [esp+4Bh] [ebp-D85h]
  char v275; // [esp+4Ch] [ebp-D84h]
  char v276; // [esp+4Dh] [ebp-D83h]
  char v277; // [esp+4Eh] [ebp-D82h]
  char v278; // [esp+4Fh] [ebp-D81h]
  char v279; // [esp+50h] [ebp-D80h]
  char v280; // [esp+51h] [ebp-D7Fh]
  char v281; // [esp+52h] [ebp-D7Eh]
  char v282; // [esp+53h] [ebp-D7Dh]
  WORD SkillIndex[2]; // [esp+54h] [ebp-D7Ch] BYREF
  DWORD v284; // [esp+58h] [ebp-D78h]
  int v285; // [esp+5Ch] [ebp-D74h] BYREF
  char v286; // [esp+60h] [ebp-D70h]
  char v287; // [esp+61h] [ebp-D6Fh]
  char v288; // [esp+62h] [ebp-D6Eh]
  char v289; // [esp+63h] [ebp-D6Dh]
  char v290; // [esp+64h] [ebp-D6Ch]
  char v291; // [esp+65h] [ebp-D6Bh]
  char v292; // [esp+66h] [ebp-D6Ah]
  char v293; // [esp+67h] [ebp-D69h]
  char v294; // [esp+68h] [ebp-D68h]
  char v295; // [esp+69h] [ebp-D67h]
  char v296; // [esp+6Ah] [ebp-D66h]
  char v297; // [esp+6Bh] [ebp-D65h]
  char v298; // [esp+6Ch] [ebp-D64h]
  char v299; // [esp+6Dh] [ebp-D63h]
  char v300; // [esp+6Eh] [ebp-D62h]
  char v301; // [esp+6Fh] [ebp-D61h]
  char v302; // [esp+70h] [ebp-D60h]
  char v303; // [esp+71h] [ebp-D5Fh]
  char v304; // [esp+72h] [ebp-D5Eh]
  char v305; // [esp+73h] [ebp-D5Dh]
  char v306; // [esp+74h] [ebp-D5Ch]
  char v307; // [esp+75h] [ebp-D5Bh]
  char v308; // [esp+76h] [ebp-D5Ah]
  char v309; // [esp+77h] [ebp-D59h]
  char v310; // [esp+78h] [ebp-D58h]
  char v311; // [esp+79h] [ebp-D57h]
  char v312; // [esp+7Ah] [ebp-D56h]
  char v313; // [esp+7Bh] [ebp-D55h]
  float x2; // [esp+7Ch] [ebp-D54h]
  float Angle[3]; // [esp+80h] [ebp-D50h] BYREF
  char v316; // [esp+8Fh] [ebp-D41h]
  int v317; // [esp+90h] [ebp-D40h]
  WORD v318[2]; // [esp+94h] [ebp-D3Ch]
  int v319; // [esp+98h] [ebp-D38h]
  float v320; // [esp+9Ch] [ebp-D34h] BYREF
  float v321; // [esp+A0h] [ebp-D30h]
  float v322; // [esp+A4h] [ebp-D2Ch]
  float Light[3]; // [esp+A8h] [ebp-D28h] BYREF
  void *(__cdecl **v324)(std::locale::facet *__hidden, unsigned int); // [esp+B4h] [ebp-D1Ch]
  WORD buf[514]; // [esp+B8h] [ebp-D18h] BYREF
  char v326; // [esp+4BCh] [ebp-914h] BYREF
  char v327; // [esp+4BDh] [ebp-913h]
  char v328[258]; // [esp+4BEh] [ebp-912h] BYREF
  char v329; // [esp+5C0h] [ebp-810h] BYREF
  char v330; // [esp+5C1h] [ebp-80Fh]
  char v331; // [esp+5C2h] [ebp-80Eh]
  char v332[1025]; // [esp+5C3h] [ebp-80Dh] BYREF
  char v333[1024]; // [esp+9C4h] [ebp-40Ch] BYREF
  int v334; // [esp+DCCh] [ebp-4h]

  v2 = (void *)CharacterMachine;
  if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
  {
    v6 = operator_new(0x585u);
    *(BYTE *)(v6 + 1412) = 1;
    HashTable_Insert(&MAIN_HASH_CLASS, v6, (int)v2);
  }
  else
  {
    v3 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v2);
    if ( v3 == -1 )
    {
      v4 = 0;
    }
    else
    {
      v4 = *(BYTE **)(DAT_055c9bcc + 4 * v3);
    }
    v5 = v4[1412] + 1;
    v4[1412] = v5;
    if ( v5 < 2u )
    {
      Packet_DecryptBuffer(v2, v4);
    }
  }
  if ( (BYTE)DAT_07d78098 )
  {
    v317 = *(unsigned char *)(DAT_07d7809c + CharacterAttribute + 87);
  }
  else
  {
    v317 = DAT_07d7809c;
  }
  v7 = (const void *)CharacterMachine;
  if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) != -1 )
  {
    v8 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v7);
    v9 = v8 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v8);
    v10 = v9[1412] - 1;
    v9[1412] = v10;
    if ( !v10 )
    {
      Packet_EncryptBuffer(v9, v7);
    }
  }
  v11 = (char *)(Hero + 904);
  FUN_0043d3e0(&MAIN_HASH_CLASS, (DWORD *)(Hero + 904));
  *(float *)SkillIndex = *(float *)v11;
  FUN_004233e0(&MAIN_HASH_CLASS, v11);
  v12 = FUN_0043d670(&MAIN_HASH_CLASS, (char *)(Hero + 908));
  v324 = &DAT_00552460;
  v13 = *(float *)(Hero + 36);
  buf[1] = 0x1C1;
  v334 = 0;
  (BYTE)(buf[2]) = 0x10;
  buf[0] = 3;
  (WORD)(v285) = 28135;
  *((BYTE *)&buf[1] + buf[0]) = SkillIndex[0];
  (WORD)((v285) >> 16) = 0x893A;
  v286 = -68;
  v14 = buf[0] + 1;
  v287 = -78;
  v288 = -97;
  v289 = 115;
  v290 = 35;
  v291 = -88;
  v292 = -2;
  v293 = -74;
  v294 = 73;
  v295 = 93;
  v296 = 57;
  v297 = 93;
  v298 = -118;
  v299 = -53;
  v300 = 99;
  v301 = -115;
  v302 = -22;
  v303 = 125;
  v304 = 43;
  v305 = 95;
  v306 = -61;
  v307 = -79;
  v308 = -23;
  v309 = -125;
  v310 = 41;
  v311 = 81;
  v312 = -24;
  v313 = 86;
  for ( i = buf[0]; i != v14; ++i )
  {
    *((BYTE *)&buf[1] + i) ^= *((BYTE *)buf + i + 1) ^ *((BYTE *)&v285 + i % 32);
  }
  if ( ++buf[0] + 1 <= 1024 )
  {
    (WORD)(v285) = 28135;
    ((BYTE)((v285) >> 16)) = 58;
    *((BYTE *)&buf[1] + buf[0]) = v12;
    (BYTE)((v285) >> 8) = -119;
    v286 = -68;
    v287 = -78;
    v16 = buf[0] + 1;
    v288 = -97;
    v289 = 115;
    v290 = 35;
    v291 = -88;
    v292 = -2;
    v293 = -74;
    v294 = 73;
    v295 = 93;
    v296 = 57;
    v297 = 93;
    v298 = -118;
    v299 = -53;
    v300 = 99;
    v301 = -115;
    v302 = -22;
    v303 = 125;
    v304 = 43;
    v305 = 95;
    v306 = -61;
    v307 = -79;
    v308 = -23;
    v309 = -125;
    v310 = 41;
    v311 = 81;
    v312 = -24;
    v313 = 86;
    for ( j = buf[0]; j != v16; ++j )
    {
      *((BYTE *)&buf[1] + j) ^= *((BYTE *)buf + j + 1) ^ *((BYTE *)&v285 + j % 32);
    }
    ++buf[0];
  }
  *(DWORD *)v318 = 0;
  v319 = 0;
  if ( buf[0] + 1 <= 1024 )
  {
    (WORD)(v285) = 28135;
    ((BYTE)((v285) >> 16)) = 58;
    *((BYTE *)&buf[1] + buf[0]) = 16 * ((__int64)((v13 + 22.5) * 0.022222223 + 1.0) & 7);
    (BYTE)((v285) >> 8) = -119;
    v286 = -68;
    v287 = -78;
    v18 = buf[0] + 1;
    v288 = -97;
    v289 = 115;
    v290 = 35;
    v291 = -88;
    v292 = -2;
    v293 = -74;
    v294 = 73;
    v295 = 93;
    v296 = 57;
    v297 = 93;
    v298 = -118;
    v299 = -53;
    v300 = 99;
    v301 = -115;
    v302 = -22;
    v303 = 125;
    v304 = 43;
    v305 = 95;
    v306 = -61;
    v307 = -79;
    v308 = -23;
    v309 = -125;
    v310 = 41;
    v311 = 81;
    v312 = -24;
    v313 = 86;
    for ( k = buf[0]; k != v18; ++k )
    {
      *((BYTE *)&buf[1] + k) ^= *((BYTE *)buf + k + 1) ^ *((BYTE *)&v285 + k % 32);
    }
    ++buf[0];
  }
  if ( (BYTE)(buf[1]) == 193 )
  {
    (BYTE)((buf[1]) >> 8) = buf[0];
  }
  else if ( (BYTE)(buf[1]) == 194 )
  {
    *(WORD *)((char *)&buf[1] + 1) = buf[0];
  }
  v20 = buf[0];
  v21 = 0;
  Position = buf[0];
  if ( s != -1 )
  {
    while ( 1 )
    {
      v22 = send(s, (const char *)&buf[1] + v21, v20 - v21, 0);
      v23 = v22;
      if ( v22 == -1 )
      {
        break;
      }
      if ( v22 )
      {
        if ( SocketClientLogPrint )
        {
          nullsub_2((int)&buf[1], v22);
        }
        v21 += v23;
        Position -= v23;
        if ( Position > 0 )
        {
          continue;
        }
      }
      goto LABEL_44;
    }
    if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v20 <= 0x2000 )
    {
      qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf[1], Position);
      SocketClientSendBufferLength += Position;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
LABEL_44:
  v334 = -1;
  v24 = *(WORD *)(o + 2) == 390;
  *(BYTE *)(c + 748) = 0;
  if ( v24 )
  {
    SetAttackSpeed();
    switch ( v317 )
    {
      case 43:
        SetAction(o, 67);
        break;
      case 47:
        SetAction(o, 66);
        break;
      case 49:
        if ( World == 8 || World == 10 )
        {
          SetAction(o, 65);
        }
        else
        {
          SetAction(o, 64);
        }
        break;
      default:
        SetAction(o, DAT_07d7809c + 37);
        break;
    }
  }
  else
  {
    SetPlayerAttack(c);
  }
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  Positiona = (float *)(o + 16);
  Particle_Spawn(1232, (float *)(o + 16), (float *)(o + 28), Light, 0, 0.0, o);
  v25 = rand() % 2;
  PlayBuffer(v25 + 40, 0, 0);
  v26 = CharactersClient;
  v27 = 229 * MovementSkillTarget;
  *(DWORD *)(c + 796) = *(DWORD *)(CharactersClient + 916 * MovementSkillTarget + 24);
  v28 = *(float *)(o + 20);
  x2 = *(float *)(v26 + 4 * v27 + 16);
  *(float *)(c + 788) = x2;
  v29 = *(float *)(v26 + 4 * v27 + 20);
  v30 = x2;
  *(float *)SkillIndex = v29;
  v31 = *(float *)SkillIndex;
  *(float *)(c + 792) = v29;
  *(float *)SkillIndex = Movement_Tick(*(float *)(o + 16), v28, v30, v31);
  v24 = v317 == 43;
  *(float *)(o + 36) = *(float *)SkillIndex;
  if ( v24 )
  {
    TickCount = GetTickCount();
    v33 = *(float *)(c + 788);
    DAT_07e11d84 = TickCount;
    v34 = *(float *)(o + 20);
    Angle[0] = *Positiona;
    v35 = *(float *)(o + 24);
    Angle[1] = v34;
    Angle[2] = v35;
    v320 = v33 - Angle[0];
    v321 = *(float *)(c + 792) - v34;
    v322 = *(float *)(c + 796) - v35;
    if ( Vec3_Length(&v320) >= 1.0 )
    {
      v36 = Vec3_Length(&v320);
    }
    else
    {
      v36 = 1.0;
    }
    v37 = 120.0 / v36;
    v320 = v37 * v320;
    v321 = v37 * v321;
    v322 = v37 * v322;
    v38 = (char *)(c + 908);
    LODWORD(x2) = (__int64)(*(float *)(c + 788) * 0.0099999998);
    FUN_0043d3e0(&MAIN_HASH_CLASS, (DWORD *)(c + 908));
    FUN_004233e0(&MAIN_HASH_CLASS, (char *)(c + 908));
    v39 = (char *)(c + 904);
    FUN_0043d3e0(&MAIN_HASH_CLASS, (DWORD *)(c + 904));
    Positionj = *(float **)(c + 904);
    FUN_004233e0(&MAIN_HASH_CLASS, (char *)(c + 904));
    v316 = (16 * ((BYTE)(x2) - (BYTE)Positionj + 8)) | ((BYTE)(x2) - (BYTE)Positionj - 8) & 0xF;
    v284 = Hero + 449;
    v40 = 0;
    v41 = strlen(aWebzen_8) + 1;
    v42 = v41 - 1;
    v43 = strlen((const char *)(Hero + 449)) + 1;
    if ( (int)(v43 - v41) >= 0 )
    {
      x2 = 0.0;
      do
      {
        Positionb = 0;
        if ( v42 <= 0 )
        {
          goto LABEL_121;
        }
        *(DWORD *)v318 = v284 + v40;
        while ( *((BYTE *)Positionb + *(DWORD *)v318) == aWebzen_8[(DWORD)Positionb] )
        {
          Positionb = (float *)((char *)Positionb + 1);
          if ( (int)Positionb >= v42 )
          {
            goto LABEL_121;
          }
        }
        v40 = ++LODWORD(x2);
      }
      while ( SLODWORD(x2) <= (int)(v43 - v41) );
    }
    CurrentSkill = 0x2B;
    v324 = &DAT_00552460;
    buf[1] = 0x1C1;
    v334 = 1;
    (BYTE)(buf[2]) = 0x1E;
    buf[0] = 3;
    FUN_0043d3e0(&MAIN_HASH_CLASS, v38);
    x2 = *(float *)v38;
    FUN_004233e0(&MAIN_HASH_CLASS, v38);
    FUN_0043d3e0(&MAIN_HASH_CLASS, v39);
    *(DWORD *)v318 = *(DWORD *)v39;
    FUN_004233e0(&MAIN_HASH_CLASS, v39);
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = 43;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v44 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( m = buf[0]; m != v44; ++m )
      {
        *((BYTE *)&buf[1] + m) ^= *((BYTE *)buf + m + 1) ^ *(&v219 + m % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v318[0];
      v221 = 58;
      v222 = -119;
      v223 = -68;
      v46 = buf[0] + 1;
      v224 = -78;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( n = buf[0]; n != v46; ++n )
      {
        *((BYTE *)&buf[1] + n) ^= *((BYTE *)buf + n + 1) ^ *(&v219 + n % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      *((BYTE *)&buf[1] + buf[0]) = (BYTE)(x2);
      v221 = 58;
      v222 = -119;
      v223 = -68;
      v48 = buf[0] + 1;
      v224 = -78;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( ii = buf[0]; ii != v48; ++ii )
      {
        *((BYTE *)&buf[1] + ii) ^= *((BYTE *)buf + ii + 1) ^ *(&v219 + ii % 32);
      }
      ++buf[0];
    }
    v50 = (__int64)(*(float *)(o + 36) * 0.71111113);
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = v50;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v51 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( jj = buf[0]; jj != v51; ++jj )
      {
        *((BYTE *)&buf[1] + jj) ^= *((BYTE *)buf + jj + 1) ^ *(&v219 + jj % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v316;
      v221 = 58;
      v222 = -119;
      v223 = -68;
      v53 = buf[0] + 1;
      v224 = -78;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( kk = buf[0]; kk != v53; ++kk )
      {
        *((BYTE *)&buf[1] + kk) ^= *((BYTE *)buf + kk + 1) ^ *(&v219 + kk % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = 0;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v55 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( mm = buf[0]; mm != v55; ++mm )
      {
        *((BYTE *)&buf[1] + mm) ^= *((BYTE *)buf + mm + 1) ^ *(&v219 + mm % 32);
      }
      ++buf[0];
    }
    if ( (BYTE)(buf[1]) == 193 )
    {
      (BYTE)((buf[1]) >> 8) = buf[0];
    }
    else if ( (BYTE)(buf[1]) == 194 )
    {
      *(WORD *)((char *)&buf[1] + 1) = buf[0];
    }
    v57 = buf[0];
    qmemcpy(v333, &buf[1], buf[0]);
    v333[v57] = rand();
    v58 = (v333[0] != -63) + 2;
    PACKET_DECRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
    v59 = g_byPacketSerialSend;
    v332[v58 + 1024] = g_byPacketSerialSend;
    g_byPacketSerialSend = v59 + 1;
    PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
    --v58;
    v60 = v57 - v58;
    v61 = &v333[v58];
    v62 = CSimpleModulus_Encode(0, (int)v61, v60);
    if ( v62 >= 256 )
    {
      v68 = (float *)(v62 + 3);
      v329 = -60;
      v331 = v62 + 3;
      v330 = (v62 + 3) / 256;
      CSimpleModulus_Encode((int)v332, (int)v61, v60);
      v69 = 0;
      Positiond = v68;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v70 = send(s, &v329 + v69, (int)v68 - v69, 0);
          v71 = v70;
          if ( v70 == -1 )
          {
            break;
          }
          if ( v70 )
          {
            if ( SocketClientLogPrint )
            {
              nullsub_2((int)&v329, v70);
            }
            v69 += v71;
            Positiond = (float *)((char *)Positiond - v71);
            if ( (int)Positiond > 0 )
            {
              continue;
            }
          }
          goto LABEL_120;
        }
        if ( WSAGetLastError() == 10035 && (int)v68 + SocketClientSendBufferLength <= 0x2000 )
        {
          qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &v329, (unsigned int)Positiond);
          v67 = (int)Positiond + SocketClientSendBufferLength;
          goto LABEL_119;
        }
        goto LABEL_117;
      }
    }
    else
    {
      Positionc = (float *)(v62 + 2);
      v326 = -61;
      v327 = v62 + 2;
      CSimpleModulus_Encode((int)v328, (int)v61, v60);
      v63 = (signed int)Positionc;
      v64 = 0;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v65 = send(s, &v326 + v64, (int)Positionc - v64, 0);
          v66 = v65;
          if ( v65 == -1 )
          {
            break;
          }
          if ( v65 )
          {
            if ( SocketClientLogPrint )
            {
              nullsub_2((int)&v326, v65);
            }
            v63 -= v66;
            v64 += v66;
            if ( v63 > 0 )
            {
              continue;
            }
          }
          goto LABEL_120;
        }
        if ( WSAGetLastError() == 10035 && (int)Positionc + SocketClientSendBufferLength <= 0x2000 )
        {
          qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &v326, v63);
          v67 = v63 + SocketClientSendBufferLength;
LABEL_119:
          SocketClientSendBufferLength = v67;
          goto LABEL_120;
        }
LABEL_117:
        CWsctlc::Close((DWORD)&SocketClient);
      }
    }
LABEL_120:
    v334 = -1;
LABEL_121:
    v72 = 2;
    v285 = *(short *)(CharactersClient + 916 * MovementSkillTarget + 476);
    *(DWORD *)SkillIndex = 1;
    do
    {
      v73 = *(WORD *)(o + 134);
      Angle[0] = v320 + Angle[0];
      Angle[1] = v321 + Angle[1];
      Angle[2] = v322 + Angle[2];
      FUN_0045fdb0((int)Angle, 100.0, v73, (int)&v285, (int)SkillIndex, 6);
      --v72;
    }
    while ( v72 );
    v74 = (void *)CharacterMachine;
    if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
    {
      v78 = operator_new(0x585u);
      *(BYTE *)(v78 + 1412) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v78, (int)v74);
    }
    else
    {
      v75 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v74);
      if ( v75 == -1 )
      {
        v76 = 0;
      }
      else
      {
        v76 = *(BYTE **)(DAT_055c9bcc + 4 * v75);
      }
      v77 = v76[1412] + 1;
      v76[1412] = v77;
      if ( v77 < 2u )
      {
        Packet_DecryptBuffer(v74, v76);
      }
    }
    LODWORD(x2) = *(unsigned char *)(*(unsigned char *)(Hero + 913) + CharacterAttribute + 87);
    v79 = (BYTE *)CharacterMachine;
    v80 = *(unsigned char *)(CharacterMachine + 1408);
    v207 = CharacterMachine;
    *(BYTE *)(o + 136) = v80;
    v79[1408] = v80 + 1;
    if ( FUN_004041e0(&MAIN_HASH_CLASS, v207) != -1 )
    {
      v81 = FUN_004041e0(&MAIN_HASH_CLASS, (int)v79);
      v82 = v81 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v81);
      v83 = v82[1412] - 1;
      v82[1412] = v83;
      if ( !v83 )
      {
        Packet_EncryptBuffer(v82, v79);
      }
    }
    v84 = Hero + 449;
    v85 = strlen(aWebzen_9) + 1;
    v86 = v85 - 1;
    v87 = strlen((const char *)(Hero + 449)) + 1;
    *(DWORD *)v318 = Hero + 449;
    if ( (int)(v87 - v85) >= 0 )
    {
      v88 = 0;
      while ( 1 )
      {
        v89 = 0;
        if ( v86 <= 0 )
        {
          goto LABEL_305;
        }
        while ( *(BYTE *)(v88 + v84 + v89) == aWebzen_9[v89] )
        {
          if ( ++v89 >= v86 )
          {
            goto LABEL_305;
          }
        }
        if ( ++v88 > (int)(v87 - v85) )
        {
          break;
        }
        v84 = *(DWORD *)v318;
      }
    }
    v324 = &DAT_00552460;
    buf[1] = 0x1C1;
    v334 = 2;
    (BYTE)(buf[2]) = 0x1D;
    buf[0] = 3;
    v219 = -25;
    v220 = 109;
    *((BYTE *)&buf[1] + buf[0]) = (BYTE)(x2);
    v221 = 58;
    v222 = -119;
    v223 = -68;
    v90 = buf[0] + 1;
    v224 = -78;
    v225 = -97;
    v226 = 115;
    v227 = 35;
    v228 = -88;
    v229 = -2;
    v230 = -74;
    v231 = 73;
    v232 = 93;
    v233 = 57;
    v234 = 93;
    v235 = -118;
    v236 = -53;
    v237 = 99;
    v238 = -115;
    v239 = -22;
    v240 = 125;
    v241 = 43;
    v242 = 95;
    v243 = -61;
    v244 = -79;
    v245 = -23;
    v246 = -125;
    v247 = 41;
    v248 = 81;
    v249 = -24;
    v250 = 86;
    for ( nn = buf[0]; nn != v90; ++nn )
    {
      *((BYTE *)&buf[1] + nn) ^= *((BYTE *)buf + nn + 1) ^ *(&v219 + nn % 32);
    }
    if ( ++buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = (__int64)(Angle[0] * 0.0099999998);
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v92 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i1 = buf[0]; i1 != v92; ++i1 )
      {
        *((BYTE *)&buf[1] + i1) ^= *((BYTE *)buf + i1 + 1) ^ *(&v219 + i1 % 32);
      }
      ++buf[0];
    }
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = (__int64)(Angle[1] * 0.0099999998);
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v94 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i2 = buf[0]; i2 != v94; ++i2 )
      {
        *((BYTE *)&buf[1] + i2) ^= *((BYTE *)buf + i2 + 1) ^ *(&v219 + i2 % 32);
      }
      ++buf[0];
    }
    v96 = *(BYTE *)(o + 136);
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = v96;
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v97 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i3 = buf[0]; i3 != v97; ++i3 )
      {
        *((BYTE *)&buf[1] + i3) ^= *((BYTE *)buf + i3 + 1) ^ *(&v219 + i3 % 32);
      }
      ++buf[0];
    }
    v99 = *(DWORD *)SkillIndex;
    if ( buf[0] + 1 <= 1024 )
    {
      v219 = -25;
      v220 = 109;
      v221 = 58;
      *((BYTE *)&buf[1] + buf[0]) = SkillIndex[0];
      v222 = -119;
      v223 = -68;
      v224 = -78;
      v100 = buf[0] + 1;
      v225 = -97;
      v226 = 115;
      v227 = 35;
      v228 = -88;
      v229 = -2;
      v230 = -74;
      v231 = 73;
      v232 = 93;
      v233 = 57;
      v234 = 93;
      v235 = -118;
      v236 = -53;
      v237 = 99;
      v238 = -115;
      v239 = -22;
      v240 = 125;
      v241 = 43;
      v242 = 95;
      v243 = -61;
      v244 = -79;
      v245 = -23;
      v246 = -125;
      v247 = 41;
      v248 = 81;
      v249 = -24;
      v250 = 86;
      for ( i4 = buf[0]; i4 != v100; ++i4 )
      {
        *((BYTE *)&buf[1] + i4) ^= *((BYTE *)buf + i4 + 1) ^ *(&v219 + i4 % 32);
      }
      ++buf[0];
    }
    if ( v99 > 0 )
    {
      v102 = &v285;
      v103 = v99;
      do
      {
        v104 = *v102 >> 8;
        if ( buf[0] + 1 <= 1024 )
        {
          v219 = -25;
          v220 = 109;
          v221 = 58;
          *((BYTE *)&buf[1] + buf[0]) = v104;
          v222 = -119;
          v223 = -68;
          v224 = -78;
          v105 = buf[0] + 1;
          v225 = -97;
          v226 = 115;
          v227 = 35;
          v228 = -88;
          v229 = -2;
          v230 = -74;
          v231 = 73;
          v232 = 93;
          v233 = 57;
          v234 = 93;
          v235 = -118;
          v236 = -53;
          v237 = 99;
          v238 = -115;
          v239 = -22;
          v240 = 125;
          v241 = 43;
          v242 = 95;
          v243 = -61;
          v244 = -79;
          v245 = -23;
          v246 = -125;
          v247 = 41;
          v248 = 81;
          v249 = -24;
          v250 = 86;
          for ( i5 = buf[0]; i5 != v105; ++i5 )
          {
            *((BYTE *)&buf[1] + i5) ^= *((BYTE *)buf + i5 + 1) ^ *(&v219 + i5 % 32);
          }
          ++buf[0];
        }
        v107 = *(BYTE *)v102;
        if ( buf[0] + 1 <= 1024 )
        {
          v251 = -25;
          v252 = 109;
          v253 = 58;
          *((BYTE *)&buf[1] + buf[0]) = v107;
          v254 = -119;
          v255 = -68;
          v256 = -78;
          v108 = buf[0] + 1;
          v257 = -97;
          v258 = 115;
          v259 = 35;
          v260 = -88;
          v261 = -2;
          v262 = -74;
          v263 = 73;
          v264 = 93;
          v265 = 57;
          v266 = 93;
          v267 = -118;
          v268 = -53;
          v269 = 99;
          v270 = -115;
          v271 = -22;
          v272 = 125;
          v273 = 43;
          v274 = 95;
          v275 = -61;
          v276 = -79;
          v277 = -23;
          v278 = -125;
          v279 = 41;
          v280 = 81;
          v281 = -24;
          v282 = 86;
          for ( i6 = buf[0]; i6 != v108; ++i6 )
          {
            *((BYTE *)&buf[1] + i6) ^= *((BYTE *)buf + i6 + 1) ^ *(&v251 + i6 % 32);
          }
          ++buf[0];
        }
        ++v102;
        --v103;
      }
      while ( v103 );
    }
    if ( (BYTE)(buf[1]) == 193 )
    {
      (BYTE)((buf[1]) >> 8) = buf[0];
    }
    else if ( (BYTE)(buf[1]) == 194 )
    {
      *(WORD *)((char *)&buf[1] + 1) = buf[0];
    }
    v110 = buf[0];
    qmemcpy(v333, &buf[1], buf[0]);
    v333[v110] = rand();
    v111 = (v333[0] != -63) + 2;
    if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) == -1 )
    {
      v114 = operator_new(2u);
      *(BYTE *)(v114 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v114, (int)&g_byPacketSerialSend);
    }
    else
    {
      v112 = (char *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
      v113 = v112[1] + 1;
      v112[1] = v113;
      if ( (unsigned char)v113 < 2u )
      {
        Packet_DecryptByte(&g_byPacketSerialSend, v112);
      }
    }
    v115 = g_byPacketSerialSend;
    v332[v111 + 1024] = g_byPacketSerialSend;
    g_byPacketSerialSend = v115 + 1;
    PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
    v116 = v111 - 1;
    v117 = v110 - v116;
    v118 = &v333[v116];
    v119 = CSimpleModulus_Encode(0, (int)&v333[v116], v117);
    if ( v119 < 256 )
    {
      Positione = (float *)(v119 + 2);
      v326 = -61;
      v327 = v119 + 2;
      CSimpleModulus_Encode((int)v328, (int)&v333[v116], v117);
      v120 = (signed int)Positione;
      v121 = 0;
      if ( s != -1 )
      {
        while ( 1 )
        {
          v122 = send(s, &v326 + v121, (int)Positione - v121, 0);
          v123 = v122;
          if ( v122 == -1 )
          {
            break;
          }
          if ( v122 )
          {
            if ( SocketClientLogPrint )
            {
              nullsub_2((int)&v326, v122);
            }
            v120 -= v123;
            v121 += v123;
            if ( v120 > 0 )
            {
              continue;
            }
          }
          goto LABEL_304;
        }
        if ( WSAGetLastError() == 10035 && (int)Positione + SocketClientSendBufferLength <= 0x2000 )
        {
LABEL_194:
          qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &v326, v120);
          v124 = v120 + SocketClientSendBufferLength;
LABEL_303:
          SocketClientSendBufferLength = v124;
          goto LABEL_304;
        }
        goto LABEL_300;
      }
LABEL_304:
      v334 = -1;
      goto LABEL_305;
    }
    v125 = (float *)(v119 + 3);
    v329 = -60;
    v331 = v119 + 3;
    v330 = (v119 + 3) / 256;
    CSimpleModulus_Encode((int)v332, (int)v118, v117);
    v126 = 0;
    Positionf = v125;
    if ( s == -1 )
    {
      goto LABEL_304;
    }
    while ( 1 )
    {
      v127 = send(s, &v329 + v126, (int)v125 - v126, 0);
      v128 = v127;
      if ( v127 == -1 )
      {
        break;
      }
      if ( v127 )
      {
        if ( SocketClientLogPrint )
        {
          nullsub_2((int)&v329, v127);
        }
        v126 += v128;
        Positionf = (float *)((char *)Positionf - v128);
        if ( (int)Positionf > 0 )
        {
          continue;
        }
      }
      goto LABEL_304;
    }
    if ( WSAGetLastError() != 10035 || (v129 = SocketClientSendBufferLength, (int)v125 + SocketClientSendBufferLength > 0x2000) )
    {
LABEL_300:
      CWsctlc::Close((DWORD)&SocketClient);
      goto LABEL_304;
    }
    v130 = Positionf;
    v131 = (unsigned int)Positionf;
    goto LABEL_302;
  }
  if ( v317 == 56 )
  {
    v132 = (void *)CharacterMachine;
    v133 = *(float *)(o + 32);
    Angle[0] = *(float *)(o + 28);
    v134 = *(float *)SkillIndex - 40.0;
    Angle[1] = v133;
    *(float *)SkillIndex = 0.0;
    Angle[2] = v134;
    if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
    {
      v137 = operator_new(0x585u);
      *(BYTE *)(v137 + 1412) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v137, (int)v132);
    }
    else
    {
      v135 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)v132);
      v136 = v135[1412] + 1;
      v135[1412] = v136;
      if ( v136 < 2u )
      {
        Packet_DecryptBuffer(v132, v135);
      }
    }
    v138 = 0.0;
    while ( *(BYTE *)(CharacterAttribute + LODWORD(v138) + 87) != 56 )
    {
      ++LODWORD(v138);
      if ( SLODWORD(v138) >= 20 )
      {
        goto LABEL_216;
      }
    }
    *(float *)SkillIndex = v138;
LABEL_216:
    STRUCT_ENCRYPT(&MAIN_HASH_CLASS, (const void *)CharacterMachine);
    (WORD)(v139) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, v139, *(int *)SkillIndex, 0);
    v140 = (void *)CharacterMachine;
    *(DWORD *)v318 = 0;
    Angle[2] = Angle[2] + 20.0;
    if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) == -1 )
    {
      v143 = operator_new(0x585u);
      *(BYTE *)(v143 + 1412) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v143, (int)v140);
    }
    else
    {
      v141 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)v140);
      v142 = v141[1412] + 1;
      v141[1412] = v142;
      if ( v142 < 2u )
      {
        Packet_DecryptBuffer(v140, v141);
      }
    }
    v144 = 0;
    while ( *(BYTE *)(CharacterAttribute + v144 + 87) != 56 )
    {
      if ( ++v144 >= 20 )
      {
        v145 = *(DWORD *)v318;
        goto LABEL_225;
      }
    }
    v145 = v144;
LABEL_225:
    STRUCT_ENCRYPT(&MAIN_HASH_CLASS, (const void *)CharacterMachine);
    (WORD)(v146) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, v146, v145, 0);
    v147 = 0;
    Angle[2] = Angle[2] + 20.0;
    STRUCT_DECRYPT(&MAIN_HASH_CLASS, (void *)CharacterMachine);
    v148 = 0;
    while ( *(BYTE *)(CharacterAttribute + v148 + 87) != 56 )
    {
      if ( ++v148 >= 20 )
      {
        goto LABEL_230;
      }
    }
    v147 = v148;
LABEL_230:
    *(float *)SkillIndex = *(float *)&CharacterMachine;
    v149 = (BYTE *)FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine);
    if ( v149 != (BYTE *)-1 )
    {
      v149 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, *(int *)SkillIndex);
      v150 = v149[1412] - 1;
      v149[1412] = v150;
      if ( !v150 )
      {
        Packet_EncryptBuffer(v149, *(const void **)SkillIndex);
      }
    }
    (WORD)(v149) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, (int)v149, v147, 0);
    v151 = 0;
    Angle[2] = Angle[2] + 20.0;
    STRUCT_DECRYPT(&MAIN_HASH_CLASS, (void *)CharacterMachine);
    v152 = 0;
    while ( *(BYTE *)(CharacterAttribute + v152 + 87) != 56 )
    {
      if ( ++v152 >= 20 )
      {
        goto LABEL_238;
      }
    }
    v151 = v152;
LABEL_238:
    *(float *)SkillIndex = *(float *)&CharacterMachine;
    v153 = (BYTE *)FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine);
    if ( v153 != (BYTE *)-1 )
    {
      v153 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, *(int *)SkillIndex);
      v154 = v153[1412] - 1;
      v153[1412] = v154;
      if ( !v154 )
      {
        Packet_EncryptBuffer(v153, *(const void **)SkillIndex);
      }
    }
    (WORD)(v153) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, (int)v153, v151, 0);
    *(float *)SkillIndex = 0.0;
    Angle[2] = Angle[2] + 20.0;
    STRUCT_DECRYPT(&MAIN_HASH_CLASS, (void *)CharacterMachine);
    v155 = 0.0;
    while ( *(BYTE *)(CharacterAttribute + LODWORD(v155) + 87) != 56 )
    {
      ++LODWORD(v155);
      if ( SLODWORD(v155) >= 20 )
      {
        goto LABEL_246;
      }
    }
    *(float *)SkillIndex = v155;
LABEL_246:
    v156 = (const void *)CharacterMachine;
    if ( FUN_004041e0(&MAIN_HASH_CLASS, CharacterMachine) != -1 )
    {
      v158 = (BYTE *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)v156);
      v159 = v158[1412] - 1;
      v158[1412] = v159;
      if ( !v159 )
      {
        Packet_EncryptBuffer(v158, v156);
      }
    }
    (WORD)(v157) = *(WORD *)(o + 134);
    CreateEffect(203, Positiona, Angle, (float *)(o + 232), 2, o, v157, *(int *)SkillIndex, 0);
  }
  else
  {
    v160 = Hero + 449;
    v161 = strlen(aWebzen_10) + 1;
    v162 = v161 - 1;
    v163 = strlen((const char *)(Hero + 449)) + 1;
    *(DWORD *)v318 = Hero + 449;
    if ( (int)(v163 - v161) < 0 )
    {
LABEL_258:
      if ( (int)abs32(GetTickCount() - g_dwLatestMagicTick) <= 300 )
      {
        goto LABEL_305;
      }
      g_dwLatestMagicTick = GetTickCount();
      v324 = &DAT_00552460;
      buf[1] = 449;
      v334 = 3;
      (BYTE)(buf[2]) = 25;
      buf[0] = 3;
      v251 = -25;
      v252 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v317;
      v253 = 58;
      v254 = -119;
      v255 = -68;
      v166 = buf[0] + 1;
      v256 = -78;
      v257 = -97;
      v258 = 115;
      v259 = 35;
      v260 = -88;
      v261 = -2;
      v262 = -74;
      v263 = 73;
      v264 = 93;
      v265 = 57;
      v266 = 93;
      v267 = -118;
      v268 = -53;
      v269 = 99;
      v270 = -115;
      v271 = -22;
      v272 = 125;
      v273 = 43;
      v274 = 95;
      v275 = -61;
      v276 = -79;
      v277 = -23;
      v278 = -125;
      v279 = 41;
      v280 = 81;
      v281 = -24;
      v282 = 86;
      for ( i7 = buf[0]; i7 != v166; ++i7 )
      {
        *((BYTE *)&buf[1] + i7) ^= *((BYTE *)buf + i7 + 1) ^ *(&v251 + i7 % 32);
      }
      ++buf[0];
      v168 = 229 * MovementSkillTarget;
      v169 = (WORD *)(CharactersClient + 916 * MovementSkillTarget + 476);
      (WORD)(v168) = *v169;
      v170 = v168 >> 8;
      if ( buf[0] + 1 <= 1024 )
      {
        v251 = -25;
        v252 = 109;
        v253 = 58;
        *((BYTE *)&buf[1] + buf[0]) = v170;
        v254 = -119;
        v255 = -68;
        v256 = -78;
        v171 = buf[0] + 1;
        v257 = -97;
        v258 = 115;
        v259 = 35;
        v260 = -88;
        v261 = -2;
        v262 = -74;
        v263 = 73;
        v264 = 93;
        v265 = 57;
        v266 = 93;
        v267 = -118;
        v268 = -53;
        v269 = 99;
        v270 = -115;
        v271 = -22;
        v272 = 125;
        v273 = 43;
        v274 = 95;
        v275 = -61;
        v276 = -79;
        v277 = -23;
        v278 = -125;
        v279 = 41;
        v280 = 81;
        v281 = -24;
        v282 = 86;
        for ( i8 = buf[0]; i8 != v171; ++i8 )
        {
          *((BYTE *)&buf[1] + i8) ^= *((BYTE *)buf + i8 + 1) ^ *(&v251 + i8 % 32);
        }
        ++buf[0];
      }
      v173 = *(BYTE *)v169;
      if ( buf[0] + 1 <= 1024 )
      {
        v251 = -25;
        v252 = 109;
        v253 = 58;
        *((BYTE *)&buf[1] + buf[0]) = v173;
        v254 = -119;
        v255 = -68;
        v256 = -78;
        v174 = buf[0] + 1;
        v257 = -97;
        v258 = 115;
        v259 = 35;
        v260 = -88;
        v261 = -2;
        v262 = -74;
        v263 = 73;
        v264 = 93;
        v265 = 57;
        v266 = 93;
        v267 = -118;
        v268 = -53;
        v269 = 99;
        v270 = -115;
        v271 = -22;
        v272 = 125;
        v273 = 43;
        v274 = 95;
        v275 = -61;
        v276 = -79;
        v277 = -23;
        v278 = -125;
        v279 = 41;
        v280 = 81;
        v281 = -24;
        v282 = 86;
        for ( i9 = buf[0]; i9 != v174; ++i9 )
        {
          *((BYTE *)&buf[1] + i9) ^= *((BYTE *)buf + i9 + 1) ^ *(&v251 + i9 % 32);
        }
        ++buf[0];
      }
      if ( (BYTE)(buf[1]) == 193 )
      {
        (BYTE)((buf[1]) >> 8) = buf[0];
      }
      else if ( (BYTE)(buf[1]) == 194 )
      {
        *(WORD *)((char *)&buf[1] + 1) = buf[0];
      }
      v176 = buf[0];
      qmemcpy(v333, &buf[1], buf[0]);
      v333[v176] = rand();
      v177 = (v333[0] != -63) + 2;
      if ( FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend) == -1 )
      {
        v180 = operator_new(2u);
        *(BYTE *)(v180 + 1) = 1;
        HashTable_Insert(&MAIN_HASH_CLASS, v180, (int)&g_byPacketSerialSend);
      }
      else
      {
        v178 = (char *)HashTable_GetNode(&MAIN_HASH_CLASS, (int)&g_byPacketSerialSend);
        v179 = v178[1] + 1;
        v178[1] = v179;
        if ( (unsigned char)v179 < 2u )
        {
          Packet_DecryptByte(&g_byPacketSerialSend, v178);
        }
      }
      v181 = g_byPacketSerialSend;
      v332[v177 + 1024] = g_byPacketSerialSend;
      g_byPacketSerialSend = v181 + 1;
      PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_byPacketSerialSend);
      v182 = v177 - 1;
      v183 = v176 - v182;
      v184 = &v333[v182];
      v185 = CSimpleModulus_Encode(0, (int)&v333[v182], v183);
      if ( v185 < 256 )
      {
        Positiong = (float *)(v185 + 2);
        v326 = -61;
        v327 = v185 + 2;
        CSimpleModulus_Encode((int)v328, (int)&v333[v182], v183);
        v120 = (signed int)Positiong;
        v186 = 0;
        if ( s == -1 )
        {
          goto LABEL_304;
        }
        while ( 1 )
        {
          v187 = send(s, &v326 + v186, (int)Positiong - v186, 0);
          v188 = v187;
          if ( v187 == -1 )
          {
            break;
          }
          if ( v187 )
          {
            if ( SocketClientLogPrint )
            {
              nullsub_2((int)&v326, v187);
            }
            v120 -= v188;
            v186 += v188;
            if ( v120 > 0 )
            {
              continue;
            }
          }
          goto LABEL_304;
        }
        if ( WSAGetLastError() == 10035 && (int)Positiong + SocketClientSendBufferLength <= 0x2000 )
        {
          goto LABEL_194;
        }
        goto LABEL_300;
      }
      v189 = (float *)(v185 + 3);
      v329 = -60;
      v331 = v185 + 3;
      v330 = (v185 + 3) / 256;
      CSimpleModulus_Encode((int)v332, (int)v184, v183);
      v190 = 0;
      Positionh = v189;
      if ( s == -1 )
      {
        goto LABEL_304;
      }
      while ( 1 )
      {
        v191 = send(s, &v329 + v190, (int)v189 - v190, 0);
        v192 = v191;
        if ( v191 == -1 )
        {
          break;
        }
        if ( v191 )
        {
          if ( SocketClientLogPrint )
          {
            nullsub_2((int)&v329, v191);
          }
          v190 += v192;
          Positionh = (float *)((char *)Positionh - v192);
          if ( (int)Positionh > 0 )
          {
            continue;
          }
        }
        goto LABEL_304;
      }
      if ( WSAGetLastError() != 10035 )
      {
        goto LABEL_300;
      }
      v129 = SocketClientSendBufferLength;
      if ( (int)v189 + SocketClientSendBufferLength > 0x2000 )
      {
        goto LABEL_300;
      }
      v130 = Positionh;
      v131 = (unsigned int)Positionh;
LABEL_302:
      qmemcpy((char *)&SocketClientSendBuffer + v129, &v329, v131);
      v124 = (int)v130 + SocketClientSendBufferLength;
      goto LABEL_303;
    }
    v164 = 0;
    while ( 1 )
    {
      v165 = 0;
      if ( v162 <= 0 )
      {
        break;
      }
      while ( *(BYTE *)(v165 + v164 + v160) == aWebzen_10[v165] )
      {
        if ( ++v165 >= v162 )
        {
          goto LABEL_305;
        }
      }
      if ( ++v164 > (int)(v163 - v161) )
      {
        goto LABEL_258;
      }
      v160 = *(DWORD *)v318;
    }
  }
LABEL_305:
  v193 = *(DWORD *)(o + 120);
  *(BYTE *)(c + 757) = 1;
  if ( (v193 & 0x20) != 32 )
  {
    v194 = *(float *)(c + 792) * 0.0099999998;
    (BYTE)(v284) = (__int64)(*(float *)(c + 788) * 0.0099999998);
    Positioni = (__int64)v194;
    if ( World >= 11 && World <= 16 )
    {
      switch ( abs32((__int64)(*(float *)(o + 36) * 0.022222223)) )
      {
        case 0u:
          goto LABEL_317;
        case 1u:
          v195 = v284 - 1;
          goto LABEL_316;
        case 2u:
          (BYTE)(v284) = v284 - 1;
          break;
        case 3u:
          v196 = Positioni - 1;
          (BYTE)(v284) = v284 - 1;
          goto LABEL_318;
        case 4u:
          goto LABEL_313;
        case 5u:
          (BYTE)(v284) = v284 + 1;
LABEL_313:
          v196 = Positioni - 1;
          goto LABEL_318;
        case 6u:
          (BYTE)(v284) = v284 + 1;
          break;
        case 7u:
          v195 = v284 + 1;
LABEL_316:
          (BYTE)(v284) = v195;
LABEL_317:
          v196 = Positioni + 1;
LABEL_318:
          Positioni = v196;
          break;
        default:
          break;
      }
    }
    v197 = TerrainWall[TERRAIN_INDEX((unsigned char)v284, Positioni)];
    if ( (v197 & 4) != 4 && (v197 & 8) != 8 && v317 != 47 && v317 != 43 && v317 != 49 )
    {
      v324 = &DAT_00552460;
      buf[1] = 449;
      v334 = 4;
      (BYTE)(buf[2]) = 17;
      buf[0] = 3;
      v251 = -25;
      v252 = 109;
      *((BYTE *)&buf[1] + buf[0]) = v284;
      v198 = buf[0];
      v199 = buf[0] + 1;
      v253 = 58;
      v254 = -119;
      v255 = -68;
      v256 = -78;
      v257 = -97;
      v258 = 115;
      v259 = 35;
      v260 = -88;
      v261 = -2;
      v262 = -74;
      v263 = 73;
      v264 = 93;
      v265 = 57;
      v266 = 93;
      v267 = -118;
      v268 = -53;
      v269 = 99;
      v270 = -115;
      v271 = -22;
      v272 = 125;
      v273 = 43;
      v274 = 95;
      v275 = -61;
      v276 = -79;
      v277 = -23;
      v278 = -125;
      v279 = 41;
      v280 = 81;
      v281 = -24;
      v282 = 86;
      if ( buf[0] != v199 )
      {
        do
        {
          *((BYTE *)&buf[1] + v198) ^= *((BYTE *)buf + v198 + 1) ^ *(&v251 + v198 % 32);
          ++v198;
        }
        while ( v198 != v199 );
      }
      if ( ++buf[0] + 1 <= 1024 )
      {
        v251 = -25;
        v252 = 109;
        *((BYTE *)&buf[1] + buf[0]) = Positioni;
        v200 = buf[0];
        v253 = 58;
        v201 = buf[0] + 1;
        v254 = -119;
        v255 = -68;
        v256 = -78;
        v257 = -97;
        v258 = 115;
        v259 = 35;
        v260 = -88;
        v261 = -2;
        v262 = -74;
        v263 = 73;
        v264 = 93;
        v265 = 57;
        v266 = 93;
        v267 = -118;
        v268 = -53;
        v269 = 99;
        v270 = -115;
        v271 = -22;
        v272 = 125;
        v273 = 43;
        v274 = 95;
        v275 = -61;
        v276 = -79;
        v277 = -23;
        v278 = -125;
        v279 = 41;
        v280 = 81;
        v281 = -24;
        v282 = 86;
        if ( buf[0] != v201 )
        {
          do
          {
            *((BYTE *)&buf[1] + v200) ^= *((BYTE *)buf + v200 + 1) ^ *(&v251 + v200 % 32);
            ++v200;
          }
          while ( v200 != v201 );
        }
        ++buf[0];
      }
      if ( (BYTE)(buf[1]) == 193 )
      {
        (BYTE)((buf[1]) >> 8) = buf[0];
      }
      else if ( (BYTE)(buf[1]) == 194 )
      {
        *(WORD *)((char *)&buf[1] + 1) = buf[0];
      }
      v202 = buf[0];
      v203 = 0;
      v204 = buf[0];
      if ( s != -1 )
      {
        while ( 1 )
        {
          v205 = send(s, (const char *)&buf[1] + v203, v202 - v203, 0);
          v206 = v205;
          if ( v205 == -1 )
          {
            break;
          }
          if ( v205 )
          {
            if ( SocketClientLogPrint )
            {
              nullsub_2((int)&buf[1], v205);
            }
            v204 -= v206;
            v203 += v206;
            if ( v204 > 0 )
            {
              continue;
            }
          }
          return;
        }
        if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + v202 <= 0x2000 )
        {
          qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf[1], v204);
          SocketClientSendBufferLength += v204;
        }
        else
        {
          CWsctlc::Close((DWORD)&SocketClient);
        }
      }
    }
  }
}
#endif
