// 0x00422074 FUN_00422074 — nunca activado: IDA_PORT_00422074 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj. Ver src/UI/UI_LegacyWidgets.cpp (salvapantallas: si se activa, vuelven las dos lineas de restauracion de SystemParametersInfoA).
// ── FUN_00422074 (IDA-only, gated) ──
#if defined(IDA_PORT_00422074)
int __stdcall FUN_00422074(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
  void *v4; // esp
  unsigned int v6; // eax
  int v7; // eax
  int v8; // [esp-Ch] [ebp-1778h] BYREF
  int v9; // [esp+0h] [ebp-176Ch]
  int v10; // [esp+4h] [ebp-1768h]
  unsigned int v11; // [esp+8h] [ebp-1764h]
  int v12; // [esp+Ch] [ebp-1760h]
  unsigned int v13; // [esp+10h] [ebp-175Ch]
  int v14; // [esp+14h] [ebp-1758h]
  unsigned int v15; // [esp+18h] [ebp-1754h]
  unsigned int v16; // [esp+1Ch] [ebp-1750h]
  unsigned int v17; // [esp+20h] [ebp-174Ch]
  unsigned int v18; // [esp+24h] [ebp-1748h]
  unsigned int v19; // [esp+28h] [ebp-1744h]
  unsigned int v20; // [esp+2Ch] [ebp-1740h]
  int v21; // [esp+30h] [ebp-173Ch]
  unsigned int v22; // [esp+34h] [ebp-1738h]
  unsigned int v23; // [esp+38h] [ebp-1734h]
  int v24; // [esp+3Ch] [ebp-1730h]
  int v25; // [esp+40h] [ebp-172Ch]
  unsigned int v26; // [esp+44h] [ebp-1728h]
  unsigned int v27; // [esp+48h] [ebp-1724h]
  unsigned int v28; // [esp+4Ch] [ebp-1720h]
  int v29; // [esp+50h] [ebp-171Ch]
  unsigned int v30; // [esp+54h] [ebp-1718h]
  unsigned int v31; // [esp+58h] [ebp-1714h]
  unsigned int v32; // [esp+5Ch] [ebp-1710h]
  unsigned int v33; // [esp+60h] [ebp-170Ch]
  int v34; // [esp+64h] [ebp-1708h]
  unsigned int v35; // [esp+68h] [ebp-1704h]
  unsigned int v36; // [esp+6Ch] [ebp-1700h]
  unsigned int v37; // [esp+70h] [ebp-16FCh]
  unsigned int v38; // [esp+74h] [ebp-16F8h]
  int v39; // [esp+78h] [ebp-16F4h]
  unsigned int v40; // [esp+7Ch] [ebp-16F0h]
  unsigned int v41; // [esp+80h] [ebp-16ECh]
  unsigned int v42; // [esp+84h] [ebp-16E8h]
  unsigned int v43; // [esp+88h] [ebp-16E4h]
  unsigned int v44; // [esp+8Ch] [ebp-16E0h]
  unsigned int v45; // [esp+90h] [ebp-16DCh]
  int v46; // [esp+94h] [ebp-16D8h]
  unsigned int v47; // [esp+98h] [ebp-16D4h]
  unsigned int v48; // [esp+9Ch] [ebp-16D0h]
  int v49; // [esp+A0h] [ebp-16CCh]
  int v50; // [esp+A4h] [ebp-16C8h]
  int v51; // [esp+A8h] [ebp-16C4h]
  unsigned int v52; // [esp+ACh] [ebp-16C0h]
  char v53; // [esp+B0h] [ebp-16BCh]
  char v54; // [esp+B4h] [ebp-16B8h]
  unsigned int v55; // [esp+B8h] [ebp-16B4h]
  unsigned int v56; // [esp+BCh] [ebp-16B0h]
  int v57; // [esp+C0h] [ebp-16ACh]
  unsigned int v58; // [esp+C4h] [ebp-16A8h]
  unsigned int v59; // [esp+C8h] [ebp-16A4h]
  unsigned int v60; // [esp+CCh] [ebp-16A0h]
  unsigned int v61; // [esp+D0h] [ebp-169Ch]
  int v62; // [esp+D4h] [ebp-1698h]
  unsigned int v63; // [esp+D8h] [ebp-1694h]
  unsigned int v64; // [esp+DCh] [ebp-1690h]
  unsigned int v65; // [esp+E0h] [ebp-168Ch]
  unsigned int v66; // [esp+E4h] [ebp-1688h]
  int v67; // [esp+E8h] [ebp-1684h]
  unsigned int v68; // [esp+ECh] [ebp-1680h]
  unsigned int v69; // [esp+F0h] [ebp-167Ch]
  unsigned int v70; // [esp+F4h] [ebp-1678h]
  unsigned int v71; // [esp+F8h] [ebp-1674h]
  int v72; // [esp+FCh] [ebp-1670h]
  unsigned int v73; // [esp+100h] [ebp-166Ch]
  unsigned int v74; // [esp+104h] [ebp-1668h]
  DWORD v75; // [esp+108h] [ebp-1664h]
  exception *v76; // [esp+10Ch] [ebp-1660h]
  exception *v77; // [esp+110h] [ebp-165Ch]
  int v78; // [esp+114h] [ebp-1658h]
  int v79; // [esp+118h] [ebp-1654h]
  void *v80; // [esp+11Ch] [ebp-1650h]
  int v81; // [esp+120h] [ebp-164Ch]
  int v82; // [esp+124h] [ebp-1648h]
  void *v83; // [esp+128h] [ebp-1644h]
  int v84; // [esp+12Ch] [ebp-1640h]
  BYTE *v85; // [esp+130h] [ebp-163Ch]
  BYTE *v86; // [esp+134h] [ebp-1638h]
  BYTE *v87; // [esp+138h] [ebp-1634h]
  BYTE *v88; // [esp+13Ch] [ebp-1630h]
  BYTE *v89; // [esp+140h] [ebp-162Ch]
  int v90; // [esp+144h] [ebp-1628h]
  BYTE *v91; // [esp+148h] [ebp-1624h]
  DWORD Width; // [esp+14Ch] [ebp-1620h]
  int v93; // [esp+150h] [ebp-161Ch]
  int v94; // [esp+154h] [ebp-1618h]
  int v95; // [esp+158h] [ebp-1614h]
  unsigned int v99; // [esp+168h] [ebp-1604h]
  unsigned int v100; // [esp+16Ch] [ebp-1600h]
  int v101; // [esp+170h] [ebp-15FCh]
  unsigned int v102; // [esp+174h] [ebp-15F8h]
  unsigned int v103; // [esp+178h] [ebp-15F4h]
  int v104; // [esp+17Ch] [ebp-15F0h]
  int v105; // [esp+180h] [ebp-15ECh]
  int v106; // [esp+184h] [ebp-15E8h]
  int v107; // [esp+188h] [ebp-15E4h]
  unsigned int v108; // [esp+18Ch] [ebp-15E0h]
  int v109; // [esp+190h] [ebp-15DCh]
  int v110; // [esp+194h] [ebp-15D8h]
  int v111; // [esp+198h] [ebp-15D4h]
  int v112; // [esp+19Ch] [ebp-15D0h]
  unsigned int v113; // [esp+1A0h] [ebp-15CCh]
  int v114; // [esp+1A4h] [ebp-15C8h]
  int v115; // [esp+1A8h] [ebp-15C4h]
  unsigned int v116; // [esp+1ACh] [ebp-15C0h]
  int v117; // [esp+1B0h] [ebp-15BCh]
  int v118; // [esp+1B4h] [ebp-15B8h]
  unsigned int v119; // [esp+1B8h] [ebp-15B4h]
  int v120; // [esp+1BCh] [ebp-15B0h]
  int v121; // [esp+1C0h] [ebp-15ACh]
  int v122; // [esp+1C4h] [ebp-15A8h]
  int v123; // [esp+1C8h] [ebp-15A4h]
  int v124; // [esp+1CCh] [ebp-15A0h]
  int v125; // [esp+1D0h] [ebp-159Ch]
  int v126; // [esp+1D4h] [ebp-1598h]
  int v127; // [esp+1D8h] [ebp-1594h]
  unsigned int v128; // [esp+1DCh] [ebp-1590h]
  int v129; // [esp+1E0h] [ebp-158Ch]
  int v130; // [esp+1E4h] [ebp-1588h]
  unsigned int v131; // [esp+1E8h] [ebp-1584h]
  int v132; // [esp+1ECh] [ebp-1580h]
  int v133; // [esp+1F0h] [ebp-157Ch]
  int v134; // [esp+1F4h] [ebp-1578h]
  int v135; // [esp+1F8h] [ebp-1574h]
  unsigned int v136; // [esp+1FCh] [ebp-1570h]
  int v137; // [esp+200h] [ebp-156Ch]
  int v138; // [esp+204h] [ebp-1568h]
  unsigned int v139; // [esp+208h] [ebp-1564h]
  int v140; // [esp+20Ch] [ebp-1560h]
  int v141; // [esp+210h] [ebp-155Ch]
  int v142; // [esp+214h] [ebp-1558h]
  int v143; // [esp+218h] [ebp-1554h]
  unsigned int v144; // [esp+21Ch] [ebp-1550h]
  int v145; // [esp+220h] [ebp-154Ch]
  int v146; // [esp+224h] [ebp-1548h]
  unsigned int v147; // [esp+228h] [ebp-1544h]
  int v148; // [esp+22Ch] [ebp-1540h]
  int v149; // [esp+230h] [ebp-153Ch]
  int v150; // [esp+234h] [ebp-1538h]
  int v151; // [esp+238h] [ebp-1534h]
  unsigned int v152; // [esp+23Ch] [ebp-1530h]
  int v153; // [esp+240h] [ebp-152Ch]
  int v154; // [esp+244h] [ebp-1528h]
  unsigned int v155; // [esp+248h] [ebp-1524h]
  int v156; // [esp+24Ch] [ebp-1520h]
  int v157; // [esp+250h] [ebp-151Ch]
  unsigned int v158; // [esp+254h] [ebp-1518h]
  int v159; // [esp+258h] [ebp-1514h]
  int v160; // [esp+25Ch] [ebp-1510h]
  int v161; // [esp+260h] [ebp-150Ch]
  int v162; // [esp+264h] [ebp-1508h]
  int v163; // [esp+268h] [ebp-1504h]
  WORD *v164; // [esp+26Ch] [ebp-1500h]
  int v165; // [esp+270h] [ebp-14FCh]
  int v166; // [esp+274h] [ebp-14F8h]
  signed int v167; // [esp+278h] [ebp-14F4h]
  int v168; // [esp+27Ch] [ebp-14F0h]
  int v169; // [esp+280h] [ebp-14ECh]
  int v170; // [esp+284h] [ebp-14E8h]
  signed int v171; // [esp+288h] [ebp-14E4h]
  int v172; // [esp+28Ch] [ebp-14E0h]
  int v173; // [esp+290h] [ebp-14DCh]
  int v174; // [esp+294h] [ebp-14D8h]
  int v175; // [esp+298h] [ebp-14D4h]
  int v176; // [esp+2ACh] [ebp-14C0h]
  int v177; // [esp+2B0h] [ebp-14BCh]
  int v178; // [esp+2B4h] [ebp-14B8h]
  unsigned char v179[1024]; // [esp+2B8h] [ebp-14B4h] BYREF
  int v180; // [esp+6B8h] [ebp-10B4h]
  char v181[3]; // [esp+6BCh] [ebp-10B0h] BYREF
  char v182[1025]; // [esp+6BFh] [ebp-10ADh] BYREF
  char buf[2]; // [esp+AC0h] [ebp-CACh] BYREF
  char v184[258]; // [esp+AC2h] [ebp-CAAh] BYREF
  char v185[4]; // [esp+BC4h] [ebp-BA8h] BYREF
  int v186; // [esp+BC8h] [ebp-BA4h]
  int v187; // [esp+BCCh] [ebp-BA0h]
  int n; // [esp+BD0h] [ebp-B9Ch]
  char v189[32]; // [esp+BD4h] [ebp-B98h] BYREF
  char v190[4]; // [esp+BF4h] [ebp-B78h] BYREF
  int v191; // [esp+BF8h] [ebp-B74h]
  int v192; // [esp+BFCh] [ebp-B70h]
  int m; // [esp+C00h] [ebp-B6Ch]
  char v194[32]; // [esp+C04h] [ebp-B68h] BYREF
  char v195[4]; // [esp+C24h] [ebp-B48h] BYREF
  int v196; // [esp+C28h] [ebp-B44h]
  int v197; // [esp+C2Ch] [ebp-B40h]
  int k; // [esp+C30h] [ebp-B3Ch]
  char v199[32]; // [esp+C34h] [ebp-B38h] BYREF
  char v200[4]; // [esp+C54h] [ebp-B18h] BYREF
  char v201[4]; // [esp+C58h] [ebp-B14h] BYREF
  unsigned int v202; // [esp+D0Ch] [ebp-A60h]
  int v203; // [esp+D10h] [ebp-A5Ch]
  int v204; // [esp+D14h] [ebp-A58h]
  unsigned int v205; // [esp+D18h] [ebp-A54h]
  int v206; // [esp+D1Ch] [ebp-A50h]
  int v207; // [esp+D20h] [ebp-A4Ch]
  int v208; // [esp+D24h] [ebp-A48h]
  int v209; // [esp+D28h] [ebp-A44h]
  unsigned int v210; // [esp+D2Ch] [ebp-A40h]
  int v211; // [esp+D30h] [ebp-A3Ch]
  int v212; // [esp+D34h] [ebp-A38h]
  unsigned int v213; // [esp+D38h] [ebp-A34h]
  int v214; // [esp+D3Ch] [ebp-A30h]
  int v215; // [esp+D40h] [ebp-A2Ch]
  int v216; // [esp+D44h] [ebp-A28h]
  int v217; // [esp+D48h] [ebp-A24h]
  unsigned int v218; // [esp+D4Ch] [ebp-A20h]
  int v219; // [esp+D50h] [ebp-A1Ch]
  int v220; // [esp+D54h] [ebp-A18h]
  unsigned int v221; // [esp+D58h] [ebp-A14h]
  int v222; // [esp+D5Ch] [ebp-A10h]
  int v223; // [esp+D60h] [ebp-A0Ch]
  int v224; // [esp+D64h] [ebp-A08h]
  int v225; // [esp+D68h] [ebp-A04h]
  unsigned int v226; // [esp+D6Ch] [ebp-A00h]
  int v227; // [esp+D70h] [ebp-9FCh]
  int v228; // [esp+D74h] [ebp-9F8h]
  unsigned int v229; // [esp+D78h] [ebp-9F4h]
  int v230; // [esp+D7Ch] [ebp-9F0h]
  int v231; // [esp+D80h] [ebp-9ECh]
  int v232; // [esp+D84h] [ebp-9E8h]
  int v233; // [esp+D88h] [ebp-9E4h]
  unsigned int v234; // [esp+D8Ch] [ebp-9E0h]
  int v235; // [esp+D90h] [ebp-9DCh]
  int v236; // [esp+D94h] [ebp-9D8h]
  unsigned int v237; // [esp+D98h] [ebp-9D4h]
  int v238; // [esp+D9Ch] [ebp-9D0h]
  int v239; // [esp+DA0h] [ebp-9CCh]
  int v240; // [esp+DA4h] [ebp-9C8h]
  int v241; // [esp+DA8h] [ebp-9C4h]
  exception *v242; // [esp+DACh] [ebp-9C0h]
  DWORD v243; // [esp+DB0h] [ebp-9BCh]
  LPVOID v244; // [esp+DB4h] [ebp-9B8h]
  int v245; // [esp+DB8h] [ebp-9B4h]
  LPVOID lpMem; // [esp+DBCh] [ebp-9B0h]
  int v247; // [esp+DC0h] [ebp-9ACh]
  int v248; // [esp+DC4h] [ebp-9A8h]
  BYTE *v249; // [esp+DC8h] [ebp-9A4h]
  BYTE *v250; // [esp+DCCh] [ebp-9A0h]
  BYTE *v251; // [esp+DD0h] [ebp-99Ch]
  BYTE *v252; // [esp+DD4h] [ebp-998h]
  BYTE *v253; // [esp+DD8h] [ebp-994h]
  int v254; // [esp+DDCh] [ebp-990h]
  BYTE *v255; // [esp+DE0h] [ebp-98Ch]
  int v256; // [esp+DE4h] [ebp-988h]
  void *(__cdecl **v257)(std::locale::facet *__hidden, unsigned int); // [esp+DE8h] [ebp-984h]
  WORD v258[514]; // [esp+DECh] [ebp-980h] BYREF
  int v259; // [esp+11F0h] [ebp-57Ch]
  int i; // [esp+11F4h] [ebp-578h]
  DEVMODEA DevMode[2]; // [esp+11F8h] [ebp-574h] BYREF
  char v262[4]; // [esp+1394h] [ebp-3D8h] BYREF
  char *Str; // [esp+1398h] [ebp-3D4h]
  char ER_SystemInfo[388]; // [esp+139Ch] [ebp-3D0h] BYREF
  CHAR tstrFilename[4]; // [esp+1520h] [ebp-24Ch] BYREF
  char Buffer[256]; // [esp+1624h] [ebp-148h] BYREF
  WORD pwVersion[3]; // [esp+1724h] [ebp-48h] BYREF
  WORD hWnd[3]; // [esp+172Ah] [ebp-42h]
  WORD wPortNumber; // [esp+1730h] [ebp-3Ch] BYREF
  int j; // [esp+1734h] [ebp-38h]
  char pvParam[4]; // [esp+1738h] [ebp-34h] BYREF
  int iFontSize; // [esp+173Ch] [ebp-30h]
  struct tagMSG msg; // [esp+1740h] [ebp-2Ch] BYREF
  int *v274; // [esp+175Ch] [ebp-10h]
  int v275; // [esp+1768h] [ebp-4h]

  v4 = alloca(5980);
  v274 = &v8;
  v275 = 0;
  FUN_00406af0();
  v103 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
  if ( v103 == -1 )
  {
    v101 = operator_new(5u);
    v239 = v101;
    v240 = v101;
    *(BYTE *)(v101 + 4) = 1;
    HashTable_Insert(&MAIN_HASH_CLASS, v240, (int)&DAT_055ca01c);
  }
  else
  {
    v102 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
    v237 = v102;
    if ( v102 == -1 )
    {
      v238 = 0;
      v241 = 0;
    }
    else
    {
      v241 = *(DWORD *)(DAT_055c9bcc + 4 * v237);
    }
    if ( (unsigned char)++*(BYTE *)(v241 + 4) < 2u )
    {
      Packet_DecryptDword(&DAT_055ca01c, v241);
    }
  }
  v100 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
  if ( v100 != -1 )
  {
    v99 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
    v234 = v99;
    if ( v99 == -1 )
    {
      v235 = 0;
      v236 = 0;
    }
    else
    {
      v236 = *(DWORD *)(DAT_055c9bcc + 4 * v234);
    }
    if ( !--*(BYTE *)(v236 + 4) )
    {
      Packet_EncryptDword(v236, &DAT_055ca01c);
    }
  }
  OpenTextData();
  if ( !UpdateMuExe() )
  {
    return 0;
  }
  strcpy(Buffer, "unknown");
  memset(&Buffer[8], 0, 0xF8u);
  Str = GetCommandLineA();
  memset(pwVersion, 0, sizeof(pwVersion));
  hWnd[0] = 0;
  if ( GetFileNameOfFilePath(tstrFilename, Str) )
  {
    if ( GetFileVersion(tstrFilename, pwVersion) )
    {
      sprintf(Buffer, "%d.%02.d", pwVersion[0], pwVersion[1]);
      if ( pwVersion[2] )
      {
        v262[1] = (BYTE)((DAT_00559470) >> 8);
        v262[0] = (BYTE)(pwVersion[2]) - 1 + DAT_00559470;
        strcat(Buffer, v262);
      }
    }
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, "\r\n");
  CErrorReport::WriteLogBegin((DWORD)&g_ErrorReport);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  CErrorReport::Write(
    (DWORD)&g_ErrorReport,
    "Mu online %s (%s) executed. (%d.%d.%d.%d)\r\n",
    Buffer,
    (const char *)DAT_055c9e50,
    pwVersion[0],
    pwVersion[1],
    pwVersion[2],
    hWnd[0]);
  CErrorReport::WriteCurrentTime((DWORD)&g_ErrorReport, 1);
  memset(ER_SystemInfo, 0, sizeof(ER_SystemInfo));
  GetSystemInfo((DWORD)ER_SystemInfo);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  CErrorReport::WriteSystemInfo((DWORD)&g_ErrorReport, (DWORD)ER_SystemInfo);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  if ( GetConnectServerInfo(szCmdLine, g_lpszCmdURL, &wPortNumber) )
  {
    szServerIpAddress = g_lpszCmdURL;
    g_ServerPort = wPortNumber;
  }
  if ( !strlen(szCmdLine) )
  {
    strcpy((char *)&DevMode[0].dmPanningHeight, TextMu);
    WinExec((LPCSTR)&DevMode[0].dmPanningHeight, 5u);
    return 0;
  }
  *(DWORD *)&hWnd[1] = FindWindowA(aDialog, aMu);
  if ( *(DWORD *)&hWnd[1] )
  {
    SendMessageA(*(HWND *)&hWnd[1], 0x10u, 0, 0);
  }
  if ( !OpenMainExe() )
  {
    return 0;
  }
  CSimpleModulus::LoadEncryptionKey((DWORD)&g_SimpleModulusCS, aDataEnc1Dat);
  CSimpleModulus::LoadDecryptionKey((DWORD)&g_SimpleModulusSC, szFileName);
  if ( !Config_Load() )
  {
    CErrorReport::Write((DWORD)&g_ErrorReport, aConfigIniReadE);
    return 0;
  }
  for ( DevMode[0].dmPanningWidth = 0;
        EnumDisplaySettingsA(0, DevMode[0].dmPanningWidth, DevMode);
        ++DevMode[0].dmPanningWidth )
  {
    ;
  }
  v95 = operator_new(148 * DevMode[0].dmPanningWidth);
  v256 = v95;
  v259 = v95;
  for ( DevMode[0].dmPanningWidth = 0;
        EnumDisplaySettingsA(0, DevMode[0].dmPanningWidth, (DEVMODEA *)(148 * DevMode[0].dmPanningWidth + v259));
        ++DevMode[0].dmPanningWidth )
  {
    ;
  }
  for ( i = 0; i < (int)DevMode[0].dmPanningWidth; ++i )
  {
    if ( *(DWORD *)(v259 + 148 * i + 108) == WindowWidth
      && *(DWORD *)(v259 + 148 * i + 112) == WindowHeight
      && *(DWORD *)(v259 + 148 * i + 104) == 16 )
    {
      ChangeDisplaySettingsA((DEVMODEA *)(148 * i + v259), 0);
      break;
    }
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, "> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);
  g_hInst = hInstance;
  g_hWnd = StartWindow(hInstance, nCmdShow);
  CErrorReport::Write((DWORD)&g_ErrorReport, aStartWindowSuc);
  (BYTE)(v94) = CreateOpenglWindow();
  if ( !(BYTE)v94 )
  {
    return 0;
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, aOpenglInitSucc);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  CErrorReport::WriteOpenGLInfo((DWORD)&g_ErrorReport);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  FUN_004058b0(&g_ErrorReport);
  ShowWindow(g_hWnd, nCmdShow);
  UpdateWindow(g_hWnd);
  (BYTE)(v93) = npGameGuard::init((int)g_hWnd);
  if ( !(BYTE)v93 )
  {
    CErrorReport::Write((DWORD)&g_ErrorReport, aGgInitError);
    return 0;
  }
  CErrorReport::Write((DWORD)&g_ErrorReport, aGgInitSuccess);
  CErrorReport::WriteImeInfo((DWORD)&g_ErrorReport, g_hWnd);
  CErrorReport::AddSeparator((DWORD)&g_ErrorReport);
  FUN_00406db0(&MAIN_HASH_CLASS, (int)g_hWnd, 1025);// Esta función no aparece en el S5
  Width = WindowWidth;
  if ( WindowWidth > 1024 )
  {
    if ( Width == 1280 )
    {
      FontHeight = 15;
    }
  }
  else
  {
    switch ( Width )
    {
      case 1024u:
        FontHeight = 14;
        break;
      case 640u:
        FontHeight = 12;
        break;
      case 800u:
        FontHeight = 13;
        break;
    }
  }
  iFontSize = FontHeight - 1;
  g_hFont = CreateFontA(
              FontHeight - 1,
              0,
              0,
              0,
              400,
              0,
              0,
              0,
              g_dwCharSet[0],
              0,
              0,
              NONANTIALIASED_QUALITY,   /* FIX 2026-07-25: sin esto la fuente sale
                 con AA/ClearType; el compose de burbujas (FUN_0047f360) hace
                 threshold (todo pixel != 0 -> color de texto), asi que el fringe
                 AA se ve como el nombre dibujado 2 veces. 1-bit = crisp como el
                 MU original. */
              0,
              GlobalText[0][0] != 0 ? GlobalText[0] : 0);
  g_hFontBold = CreateFontA(
                  iFontSize,
                  0,
                  0,
                  0,
                  700,
                  0,
                  0,
                  0,
                  g_dwCharSet[0],
                  0,
                  0,
                  0,
                  0,
                  GlobalText[0][0] != 0 ? GlobalText[0] : 0);
  g_hFontBig = CreateFontA(
                 2 * iFontSize,
                 0,
                 0,
                 0,
                 700,
                 0,
                 0,
                 0,
                 g_dwCharSet[0],
                 0,
                 0,
                 NONANTIALIASED_QUALITY,   /* ver nota en g_hFont */
                 0,
                 GlobalText[0][0] != 0 ? GlobalText[0] : 0);
  setlocale(0, lpszLocale);
  if ( m_SoundOnOff )
  {
    InitDirectSound(g_hWnd);
  }
  SetTimer(g_hWnd, 0x3E8u, 20000u, 0);
  v6 = time(0);
  srand(v6);
  for ( j = 0; j < 100; ++j )
  {
    RandomTable[j] = rand() % 360;
  }
  v7 = rand();
  v91 = (BYTE *)operator_new(v7 % 100 + 1);
  v255 = v91;
  RendomMemoryDump = v91;
  v90 = operator_new(0x384u);
  v254 = v90;
  GateAttribute = v90;
  v89 = (BYTE *)operator_new(0xA00u);
  v253 = v89;
  SkillAttribute = v89;
  v88 = (BYTE *)operator_new(0xA00u);
  v252 = v88;
  SkillAttribute2 = v88;
  v87 = (BYTE *)operator_new(98304u);
  v251 = v87;
  ItemAttRibuteMemoryDump = v87;
  ItemAttribute = (ITEM_ATTRIBUTE *)&ItemAttRibuteMemoryDump[64 * (rand() % 1024)];
  v86 = (BYTE *)operator_new(0x8000u);
  v250 = v86;
  ItemAttribute2 = v86;
  v85 = (BYTE *)operator_new(0x764D4u);
  v249 = v85;
  CharacterMemoryDump = v85;
  CharactersClient = (DWORD)&CharacterMemoryDump[916 * (rand() % 128)];
  v84 = operator_new(1412u);
  v248 = v84;
  CharacterMachine = v84;
  memset((void *)GateAttribute, 0, 0x384u);
  memset(SkillAttribute, 0, 0xA00u);
  memset(SkillAttribute2, 0, 0xA00u);
  memset(ItemAttribute, 0, 0x8000u);
  memset(ItemAttribute2, 0, 0x8000u);
  memset((void *)CharactersClient, 0, 0x59AD4u);
  memset((void *)CharacterMachine, 0, 0x584u);
  CharacterAttribute = CharacterMachine;
  CHARACTER_MACHINE::Init(CharacterMachine);
  Hero = CharactersClient;
  v83 = (void *)operator_new(0x5C8u);
  lpMem = v83;
  (BYTE)(v275) = 1;
  if ( v83 )
  {
    v82 = FUN_0040c7d0((int)lpMem);
    v81 = v82;
  }
  else
  {
    v81 = 0;
  }
  v247 = v81;
  (BYTE)(v275) = 0;
  DAT_055c9ff0 = v81;
  v80 = (void *)operator_new(0xBCu);
  v244 = v80;
  (BYTE)(v275) = 2;
  if ( v80 )
  {
    v79 = FUN_0040e990((int)v244);
    v78 = v79;
  }
  else
  {
    v78 = 0;
  }
  v245 = v78;
  (BYTE)(v275) = 0;
  DAT_055c9ff4 = v78;
  v77 = (exception *)operator_new(0xCu);
  v242 = v77;
  (BYTE)(v275) = 3;
  if ( v77 )
  {
    v76 = exception::exception(v242);
    v75 = (DWORD)v76;
  }
  else
  {
    v75 = 0;
  }
  v243 = v75;
  (BYTE)(v275) = 0;
  g_pRenderText = v75;
  SystemParametersInfoA(0x61u, 1u, pvParam, 0);
  SystemParametersInfoA(0xEu, 0, &g_iScreenSaverOldValue, 0);
  SystemParametersInfoA(0xFu, 18000u, 0, 0);
  RegisterHotKey(g_hWnd, 0, 1u, 9u);
  while ( 1 )
  {
    while ( !PeekMessageA(&msg, 0, 0, 0, 0) )
    {
      v74 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_iNoMouseTime);
      if...
      if ( ++g_iNoMouseTime > 30 )
      {
        v71 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_iNoMouseTime);
        if...
        v69 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca024);
        if...
        if...
        if...
        v31 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_iNoMouseTime);
        if...
      }
      v28 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_iNoMouseTime);
      if...
      v26 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&g_bWndActive);
      if...
      (BYTE)(v127) = g_bWndActive;
      PACKET_ENCRYPT(&MAIN_HASH_CLASS, &g_bWndActive);
      if ( (BYTE)v127 )
      {
        Scene_Dispatch(g_hDC);
      }
      else
      {
        SetForegroundWindow(g_hWnd);
        SetFocus(g_hWnd);
        v23 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca02c);
        if ( v23 == -1 )
        {
          v21 = operator_new(5u);
          v121 = v21;
          v122 = v21;
          *(BYTE *)(v21 + 4) = 1;
          HashTable_Insert(&MAIN_HASH_CLASS, v122, (int)&DAT_055ca02c);
        }
        else
        {
          v22 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca02c);
          v119 = v22;
          if ( v22 == -1 )
          {
            v120 = 0;
            v123 = 0;
          }
          else
          {
            v123 = *(DWORD *)(DAT_055c9bcc + 4 * v119);
          }
          if ( (unsigned char)++*(BYTE *)(v123 + 4) < 2u )
          {
            Packet_DecryptDword(&DAT_055ca02c, v123);
          }
        }
        if ( DAT_055ca02c <= 1 )
        {
          ++DAT_055ca02c;
          v18 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca02c);
          if ( v18 != -1 )
          {
            v17 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca02c);
            v113 = v17;
            if ( v17 == -1 )
            {
              v114 = 0;
              v115 = 0;
            }
            else
            {
              v115 = *(DWORD *)(DAT_055c9bcc + 4 * v113);
            }
            if ( !--*(BYTE *)(v115 + 4) )
            {
              Packet_EncryptDword(v115, &DAT_055ca02c);
            }
          }
          v16 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
          if ( v16 == -1 )
          {
            v14 = operator_new(5u);
            v110 = v14;
            v111 = v14;
            *(BYTE *)(v14 + 4) = 1;
            HashTable_Insert(&MAIN_HASH_CLASS, v111, (int)&DAT_055ca01c);
          }
          else
          {
            v15 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
            v108 = v15;
            if ( v15 == -1 )
            {
              v109 = 0;
              v112 = 0;
            }
            else
            {
              v112 = *(DWORD *)(DAT_055c9bcc + 4 * v108);
            }
            if ( (unsigned char)++*(BYTE *)(v112 + 4) < 2u )
            {
              Packet_DecryptDword(&DAT_055ca01c, v112);
            }
          }
          DAT_055ca01c = 1;
          v13 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
          if ( v13 != -1 )
          {
            v12 = HashTable_GetNode(&MAIN_HASH_CLASS, &DAT_055ca01c);
            v107 = v12;
            if ( !--*(BYTE *)(v107 + 4) )
            {
              Packet_EncryptDword(v107, &DAT_055ca01c);
            }
          }
          ShowWindow(g_hWnd, 6);
          v11 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca01c);
          if ( v11 == -1 )
          {
            v9 = operator_new(5u);
            v104 = v9;
            v105 = v9;
            *(BYTE *)(v9 + 4) = 1;
            HashTable_Insert(&MAIN_HASH_CLASS, v105, (int)&DAT_055ca01c);
          }
          else
          {
            v10 = HashTable_GetNode(&MAIN_HASH_CLASS, &DAT_055ca01c);
            v106 = v10;
            ++*(BYTE *)(v10 + 4);
            if ( *(unsigned char *)(v106 + 4) < 2u )
            {
              Packet_DecryptDword(&DAT_055ca01c, v106);
            }
          }
          DAT_055ca01c = 0;
          FUN_004233e0(&MAIN_HASH_CLASS, &DAT_055ca01c);
          ShowWindow(g_hWnd, 3);
        }
        else
        {
          v20 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca02c);
          if ( v20 != -1 )
          {
            v19 = FUN_004041e0(&MAIN_HASH_CLASS, (int)&DAT_055ca02c);
            v116 = v19;
            if ( v19 == -1 )
            {
              v117 = 0;
              v118 = 0;
            }
            else
            {
              v118 = *(DWORD *)(DAT_055c9bcc + 4 * v116);
            }
            if ( !--*(BYTE *)(v118 + 4) )
            {
              Packet_EncryptDword(v118, &DAT_055ca02c);
            }
          }
          SetTimer(g_hWnd, 0x3E9u, 0x3E8u, 0);
          PostMessageA(g_hWnd, 0x10u, 0, 0);
        }
      }
LABEL_306:
      ProtocolCore();
    }
    if ( !GetMessageA(&msg, 0, 0, 0) )
    {
      break;
    }
    TranslateMessage(&msg);
    if ( msg.message != 260 && msg.message != 261 )
    {
      DispatchMessageA(&msg);
      goto LABEL_306;
    }
  }
  v275 = -1;
  DestroyWindow();
  return msg.wParam;
}
#endif
