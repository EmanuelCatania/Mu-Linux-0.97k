// 0x0050C4D0 OpenWorldModels — nunca activado: IDA_PORT_0050C4D0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── OpenWorldModels (IDA-only, gated) ──
#if defined(IDA_PORT_0050C4D0)
void __cdecl OpenWorldModels()
{
  int i; // eax
  int v1; // esi
  int j; // esi
  int v3; // esi
  int v4; // ebx
  BITMAP_t *v5; // ebp
  char *v6; // edi
  int k; // esi
  int m; // esi
  int v9; // eax
  int v10; // esi
  int v11; // esi
  int v12; // esi
  int v13; // esi
  int v14; // esi
  int v15; // esi
  int v16; // esi
  int v17; // esi
  int v18; // esi
  int v19; // esi
  int v20; // esi
  int v21; // esi
  int v22; // esi
  int v23; // esi
  int v24; // esi
  int v25; // esi
  int v26; // esi
  int v27; // esi
  int v28; // esi
  int v29; // esi
  int v30; // esi
  int v31; // esi
  int ii; // esi
  int v33; // ebp
  int v34; // eax
  int v35; // ebx
  int v36; // eax
  int v37; // esi
  int n; // esi
  char v39; // [esp+3h] [ebp-385h]
  char FileName[32]; // [esp+4h] [ebp-384h] BYREF
  char Buffer[100]; // [esp+24h] [ebp-364h] BYREF
  char ModelFileName[256]; // [esp+88h] [ebp-300h] BYREF
  char v43[256]; // [esp+188h] [ebp-200h] BYREF
  char v44[256]; // [esp+288h] [ebp-100h] BYREF

  v39 = DAT_0055a7c4;
  if ( DAT_083a410c )
  {
    DAT_0055a7c4 = 0;
    World = 7;
  }
  OpenJPG(aObject8Drop01J, 0x4D9u, 0x2600u, 0x2900u, 0, 1);
  if ( !DAT_0055a7c4 )
  {
    switch ( World )
    {
      case 0:
        OpenModel(174, aData2Object1An, aBirdSmd, aBirdsFlySmd, aBirdsStopSmd, "end");
        OpenModel(181, aData2Object1An, aFishSmd, aFishsRunSmd, aFishsJumpSmd, "end");
        break;
      case 1:
      case 4:
        OpenModel(215, aData2Object2, aU, "end");
        OpenModel(176, aData2Object2, aBatSmd, aBatsSmd, "end");
        OpenModel(177, aData2Object2, aMouseSmd, aMousesSmd, "end");
        break;
      case 3:
        OpenModel(175, aData2Object1An, aButterflySmd, aButterflySSmd, "end");
        break;
      case 5:
        OpenModel(228, aData2Object6, aE_1, "end");
        OpenModel(229, aData2Object6, aE_2, "end");
        OpenModel(230, aData2Object6, aE_3, "end");
        OpenModel(231, aData2Object6, aE_4, "end");
        OpenModel(232, aData2Object6, aE_5, "end");
        OpenModel(234, aData2Monster, aAiud_0, aAiudOSSmd, aAiudOSAoSmd, "end");
        OpenModel(235, aData2Object6, aOaoSmd, aOaosAAu01Smd, aOaosAAu02Smd, "end");
        break;
      case 6:
        OpenModel(178, aData2Object7, &DAT_0055f560, &DAT_0055f56c, &DAT_0055f580, "end");
        *(BYTE *)(*(DWORD *)(Models + 33512) + 26) = 1;
        break;
      case 7:
        OpenModel(182, aData2Object8, aA_7, &DAT_0055f540, "end");
        OpenModel(183, aData2Object8, aB, &DAT_0055f514, "end");
        OpenModel(184, aData2Object8, aA_8, &DAT_0055f4f8, "end");
        OpenModel(185, aData2Object8, aB_0, &DAT_0055f4dc, "end");
        OpenModel(186, aData2Object8, aAuao, &DAT_0055f4c0, "end");
        OpenModel(187, aData2Object8, aA_9, &DAT_0055f4a0, "end");
        OpenModel(188, aData2Object8, aAo_1, &DAT_0055f480, "end");
        OpenModel(189, aData2Object8, aCoA, &DAT_0055f464, "end");
        for ( i = 188; i < 1692; *(BYTE *)(*(DWORD *)(i + Models + 33888) + 10) = 1 )
        {
          i += 188;
        }
        break;
      case 8:
        OpenModel(179, aData2Object9, aUua, &DAT_0055f444, "end");
        break;
      case 10:
        OpenJPG(aEffectCloudsJp, 0x4F4u, 0x2601u, 0x2900u, 0, 1);
        OpenModel(182, aData2Object11, aCloudSmd, "end");
        AccessModel(182, aDataObject11, aCloud, -1);
        OpenTexture(182, aObject11, 9728, 1);
        OpenJPG(aEffectCloudlig, 0x4F5u, 0x2601u, 0x2900u, 0, 1);
        break;
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
        OpenModel(184, aData2Object12, &DAT_0055f388, &DAT_0055f394, &DAT_0055f3a4, "end");
        break;
      default:
        break;
    }
  }
  SetMaxTextures(105);
  switch ( World )
  {
    case 0:
      AccessModel(174, aDataObject1, aBird, 1);
      OpenTexture(174, aObject1, 9728, 1);
      AccessModel(181, aDataObject1, aFish, 1);
      OpenTexture(181, aObject1, 9728, 1);
      break;
    case 1:
    case 4:
      AccessModel(215, aDataObject2, aDungeonstone, 1);
      OpenTexture(215, aObject2, 9728, 1);
      AccessModel(176, aDataObject2, DAT_0055f31c, 1);
      OpenTexture(176, aObject2, 9728, 1);
      AccessModel(177, aDataObject2, DAT_0055f318, 1);
      OpenTexture(177, aObject2, 9728, 1);
      break;
    case 3:
      AccessModel(175, aDataObject1, aButterfly, 1);
      OpenTexture(175, aObject1, 9728, 1);
      break;
    case 5:
      v1 = 228;
      do
      {
        AccessModel(v1, aDataObject6, aMeteo, v1 - 227);
        ++v1;
      }
      while ( v1 - 228 < 5 );
      AccessModel(234, aDataObject6, aBosshead, 1);
      AccessModel(235, aDataObject6, aPrincess, 1);
      for ( j = 228; j <= 235; ++j )
      {
        OpenTexture(j, aObject6, 9728, 1);
      }
      break;
    case 6:
      AccessModel(178, aDataObject7, DAT_0055f2cc, 1);
      OpenTexture(178, aObject7, 9728, 1);
      break;
    case 7:
      v3 = 182;
      do
      {
        AccessModel(v3, aDataObject8, aFish, v3 - 180);
        OpenTexture(v3++, aObject8, 9728, 1);
      }
      while ( v3 - 181 < 9 );
      v4 = 0;
      v5 = &Bitmaps[65];
      do
      {
        if ( (int)v5 >= (int)Bitmaps[75].FileName )
        {
          sprintf(Buffer, "Object8\\wt%d.jpg", v4);
        }
        else
        {
          sprintf(Buffer, "Object8\\wt0%d.jpg", v4);
        }
        OpenJPG(Buffer, v4 + 65, 0x2601u, 0x2901u, 0, 0);
        if ( (int)v5 >= (int)Bitmaps[75].FileName )
        {
          sprintf(Buffer, "wt%d.jpg", v4);
        }
        else
        {
          sprintf(Buffer, "wt0%d.jpg", v4);
        }
        v6 = (char *)v5++;
        strcpy(v6, Buffer);
        ++v4;
      }
      while ( (int)v5 < (int)Bitmaps[97].FileName );
      break;
    case 8:
      OpenJPG(aObject9Sand01J, 0x494u, 0x2601u, 0x2901u, 0, 1);
      OpenJPG(aObject9Sand02J, 0x495u, 0x2601u, 0x2901u, 0, 1);
      OpenJPG(aObject9Impack0, 0x597u, 0x2601u, 0x2900u, 0, 1);
      AccessModel(179, aDataObject9, DAT_0055f2cc, 2);
      OpenTexture(179, aObject9, 9728, 1);
      break;
    case 10:
      OpenJPG(aEffectCloudsJp, 0x4F4u, 0x2601u, 0x2900u, 0, 1);
      AccessModel(182, aDataObject11, aCloud, -1);
      OpenTexture(182, aObject11, 9728, 1);
      OpenJPG(aEffectCloudlig, 0x4F5u, 0x2601u, 0x2900u, 0, 1);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      if ( !DAT_0055a7c4 )
      {
        OpenModel(184, aData2Object12, &DAT_0055f388, &DAT_0055f394, &DAT_0055f3a4, "end");
        OpenModel(262, aData2Object12, &DAT_0055f1e8, "end");
        OpenModel(263, aData2Object12, &DAT_0055f1d8, "end");
        OpenModel(260, aData2Object12, &DAT_0055f1c8, "end");
        OpenModel(261, aData2Object12, &DAT_0055f1b8, "end");
        OpenModel(185, aData2Object12, aCOu2Smd, "end");
      }
      AccessModel(184, aDataObject12, aCrow, 1);
      OpenTexture(184, aObject12, 9728, 1);
      AccessModel(262, aDataObject12, aGate, 1);
      AccessModel(263, aDataObject12, aGate, 2);
      AccessModel(260, aDataObject12, aStonecoffin, 1);
      AccessModel(261, aDataObject12, aStonecoffin, 2);
      for ( k = 0; k < 2; ++k )
      {
        OpenTexture(k + 262, "Monster\\", 9728, 1);
      }
      for ( m = 0; m < 2; ++m )
      {
        OpenTexture(m + 260, "Monster\\", 9728, 1);
      }
      AccessModel(185, aDataObject12, aShine, 1);
      OpenTexture(185, aObject12, 9728, 1);
      OpenJPG(aEffectCloudsJp, 0x4F4u, 0x2601u, 0x2900u, 0, 1);
      LoadWaveFile(110, "Data\\Sound\\iBloodCastle.wav", 1, 0);
      DAT_0055a7c4 = 1;
      break;
    default:
      break;
  }
  SetMaxTextures(750);
  if ( World )
  {
    v33 = World + 1;
    if ( World >= 11 && World <= 16 )
    {
      v33 = 12;
    }
    if ( !DAT_0055a7c4 )
    {
      sprintf(FileName, "Data2\\Object%d\\_ÆÄÀÏ.txt", v33);
      ParserFileHandle = fopen(FileName, "rb");
      if ( ParserFileHandle )
      {
        while ( 1 )
        {
          v34 = ParseNextToken();
          v35 = (__int64)DAT_083a40f8;
          if ( v34 == 2 )
          {
            break;
          }
          ParseNextToken();
          strcpy(ModelFileName, (const char *)&ParserTokenString);
          ParseNextToken();
          strcpy(v43, (const char *)&ParserTokenString);
          ParseNextToken();
          strcpy(v44, (const char *)&ParserTokenString);
          sprintf(FileName, aData2ObjectD, v33);
          if ( strlen(v43) )
          {
            if ( strlen(v44) )
            {
              OpenModel(v35, FileName, ModelFileName, v43, v44, "end");
            }
            else
            {
              OpenModel(v35, FileName, ModelFileName, v43, "end");
            }
          }
          else
          {
            OpenModel(v35, FileName, ModelFileName, "end");
          }
        }
        fclose(ParserFileHandle);
      }
    }
    sprintf(FileName, "Data\\Object%d\\", v33);
    v36 = 0;
    do
    {
      v37 = v36 + 1;
      AccessModel(v36, FileName, aObject, v36 + 1);
      v36 = v37;
    }
    while ( v37 < 160 );
    SetMaxTextures(750);
    sprintf(FileName, "Object%d\\", v33);
    for ( n = 0; n < 160; ++n )
    {
      OpenTexture(n, FileName, 9728, 1);
    }
    if ( World == 1 )
    {
      *(DWORD *)(*(DWORD *)(Models + 7568) + 20) = 1053609165;
    }
    else if ( World == 8 )
    {
      *(BYTE *)(Models + 2204) = 0;
      *(BYTE *)(Models + 2392) = 0;
      *(BYTE *)(Models + 2580) = 0;
      *(BYTE *)(Models + 13860) = 0;
      *(BYTE *)(Models + 14236) = 0;
      *(BYTE *)(Models + 14988) = 0;
    }
  }
  else
  {
    if ( !DAT_0055a7c4 )
    {
      OpenModel(0, aData2Object1, aTreesmallSmd, aTreesmallsSmd, "end");
      OpenModel(1, aData2Object1, aTreebigSmd, aTreebigsSmd, "end");
      OpenModel(2, aData2Object1, aTreea01Smd, "end");
      OpenModel(3, aData2Object1, aTreea02Smd, "end");
      OpenModel(4, aData2Object1, aTreea03Smd, "end");
      OpenModel(5, aData2Object1, aTreea04Smd, "end");
      OpenModel(6, aData2Object1, aTreea05Smd, "end");
      OpenModel(7, aData2Object1, aTreea06Smd, "end");
      OpenModel(8, aData2Object1, aTreea07Smd, "end");
      OpenModel(9, aData2Object1, aTreea08Smd, "end");
      OpenModel(10, aData2Object1, aTreea09Smd, aTreeas09Smd, "end");
      OpenModel(11, aData2Object1, aTreea10Smd, aTreeas10Smd, "end");
      OpenModel(12, aData2Object1, aTreea11Smd, aTreeas11Smd, "end");
      OpenModel(20, aData2Object1, aGrass01Smd, "end");
      OpenModel(21, aData2Object1, aGrass02Smd, "end");
      OpenModel(22, aData2Object1, aGrass03Smd, "end");
      OpenModel(23, aData2Object1, aGrass04Smd, "end");
      OpenModel(24, aData2Object1, aGrass05Smd, "end");
      OpenModel(25, aData2Object1, aGrass06Smd, "end");
      OpenModel(26, aData2Object1, aMushroom01Smd, "end");
      OpenModel(27, aData2Object1, aMushroom02Smd, "end");
      OpenModel(30, aData2Object1, aSton01Smd, "end");
      OpenModel(31, aData2Object1, aSton02Smd, "end");
      OpenModel(32, aData2Object1, aSton03Smd, "end");
      OpenModel(33, aData2Object1, aSton04Smd, "end");
      OpenModel(34, aData2Object1, aSton05Smd, "end");
      OpenModel(40, aData2Object1, aStoneStatue01S, "end");
      OpenModel(41, aData2Object1, aStoneStatue02S, "end");
      OpenModel(42, aData2Object1, aAngelStoneSmd, "end");
      OpenModel(43, aData2Object1, aSteelBarredDoo, "end");
      OpenModel(44, aData2Object1, aTombArcSmd, "end");
      DAT_083a4100 = 1;
      OpenModel(45, aData2Object1, aTombCrossSmd, "end");
      OpenModel(46, aData2Object1, aTombstoneSmd, "end");
      OpenModel(50, aData2Object1, aFireLightSmd, "end");
      OpenModel(51, aData2Object1, aFireLight01Smd, "end");
      OpenModel(52, aData2Object1, aFireSmd, "end");
      OpenModel(55, aData2Object1, aDungeonGate01S, "end");
      OpenModel(58, aData2Object1, aDrumSmd, "end");
      OpenModel(59, aData2Object1, aTreasureChestS, aTreasureChests, "end");
      OpenModel(60, aData2Object1, aShipSmd, aShipsSmd, "end");
      OpenModel(69, aData2Object1, aWall01Smd, "end");
      OpenModel(70, aData2Object1, aWall02Smd, "end");
      OpenModel(71, aData2Object1, aWall03Smd, "end");
      OpenModel(72, aData2Object1, aCWall06Smd, aCWalls06Smd, "end");
      OpenModel(73, aData2Object1, aWall05Smd, "end");
      OpenModel(74, aData2Object1, aWall06Smd, aWalls06Smd, "end");
      OpenModel(75, aData2Object1, aCWall01Smd, "end");
      OpenModel(76, aData2Object1, aCWall02Smd, "end");
      OpenModel(77, aData2Object1, aCWall03Smd, "end");
      OpenModel(78, aData2Object1, aCWall05Smd, "end");
      DAT_083a4100 = 1;
      OpenModel(65, aData2Object1, aSteelBarred02S, "end");
      OpenModel(66, aData2Object1, aSteelBarred03S, "end");
      OpenModel(67, aData2Object1, aSteelBarred01S, "end");
      OpenModel(68, aData2Object1, aSteelBarredDoo_0, "end");
      OpenModel(91, aData2Object1, aGun01Smd, "end");
      OpenModel(92, aData2Object1, aGun02Smd, "end");
      OpenModel(93, aData2Object1, aGun03Smd, "end");
      OpenModel(80, aData2Object1, aBridge01Smd, "end");
      OpenModel(81, aData2Object1, aFenceSmd, "end");
      OpenModel(82, aData2Object1, aJoint01Smd, "end");
      OpenModel(83, aData2Object1, aJoint02Smd, "end");
      OpenModel(84, aData2Object1, aJoint03Smd, "end");
      OpenModel(85, aData2Object1, aBridge02Smd, "end");
      OpenModel(90, aData2Object1, aStreetlightSmd, aStreetlightsSm, "end");
      OpenModel(95, aData2Object1, aBadge01Smd, aBadges01Smd, "end");
      OpenModel(98, aData2Object1, aHorseDrawnSmd, aHorseDrawnsSmd, "end");
      OpenModel(99, aData2Object1, aCarriage01Smd, "end");
      OpenModel(100, aData2Object1, aCarriage02Smd, "end");
      OpenModel(101, aData2Object1, aCarriage03Smd, "end");
      OpenModel(102, aData2Object1, aRice01Smd, "end");
      OpenModel(103, aData2Object1, aRice02Smd, "end");
      OpenModel(96, aData2Object1, aSignboardUpSmd, aSignboardUpSSm, "end");
      OpenModel(97, aData2Object1, aSignboardDownS, "end");
      OpenModel(56, aData2Object1, aMonsterASmd, aMonsterAStop01, aMonsterAStop02, "end");
      OpenModel(57, aData2Object1, aMonsterBSmd, aMonsterBStop01, aMonsterBStop02, "end");
      OpenModel(105, aData2Object1, aWaterspoutSmd, aWaterspoutsSmd, "end");
      OpenModel(106, aData2Object1, aJar01Smd, "end");
      OpenModel(107, aData2Object1, aJar02Smd, "end");
      OpenModel(108, aData2Object1, aJar03Smd, "end");
      OpenModel(109, aData2Object1, aJar04Smd, "end");
      OpenModel(110, aData2Object1, aExecutionGroun_0, aExecutionGroun, "end");
      OpenModel(115, aData2Object1, aHouseStone01Sm, "end");
      OpenModel(116, aData2Object1, aHouseStone02Sm, "end");
      OpenModel(117, aData2Object1, aHouseSmithSmd, "end");
      OpenModel(118, aData2Object1, aHouseScienceSm, aHouseSciencesS, "end");
      OpenModel(119, aData2Object1, aHouseMillSmd, aHouseMillsSmd, "end");
      OpenModel(120, aData2Object1, aTent01Smd, aTents01Smd, "end");
      OpenModel(111, aData2Object1, aStairsSmd, "end");
      OpenModel(121, aData2Object1, aHouseJoint01Sm, "end");
      OpenModel(122, aData2Object1, aHouseJoint02Sm, "end");
      OpenModel(123, aData2Object1, aHouseJoint03Sm, "end");
      OpenModel(124, aData2Object1, aHouseJoint04Sm, "end");
      OpenModel(125, aData2Object1, aHouseJoint05Sm, "end");
      OpenModel(126, aData2Object1, aHouseJoint06Sm, "end");
      OpenModel(127, aData2Object1, aCWall07Smd, "end");
      OpenModel(128, aData2Object1, aHouseSmd, "end");
      OpenModel(129, aData2Object1, aCageSmd, "end");
      OpenModel(130, aData2Object1, aEffectSmd, "end");
      OpenModel(131, aData2Object1, aEffectSmd, "end");
      OpenModel(132, aData2Object1, aEffectSmd, "end");
      OpenModel(133, aData2Object1, aPoseBoxSmd, "end");
      OpenModel(140, aData2Object1, aHouseIn01Smd, "end");
      OpenModel(141, aData2Object1, aHouseIn02Smd, "end");
      OpenModel(142, aData2Object1, aHouseIn03Smd, "end");
      OpenModel(143, aData2Object1, aHouseIn04Smd, "end");
      OpenModel(144, aData2Object1, aHouseIn05Smd, "end");
      OpenModel(145, aData2Object1, aHouseIn06Smd, "end");
      OpenModel(146, aData2Object1, aHouseIn07Smd, "end");
      OpenModel(150, aData2Object1, aCandleSmd, aCandlesSmd, "end");
      OpenModel(151, aData2Object1, aHouseInBeer01S, "end");
      OpenModel(152, aData2Object1, aHouseInBeer02S, "end");
      OpenModel(153, aData2Object1, aHouseInBeer03S, "end");
    }
    v9 = 0;
    do
    {
      v10 = v9 + 1;
      AccessModel(v9, aDataObject1, aTree, v9 + 1);
      v9 = v10;
    }
    while ( v10 < 13 );
    v11 = 20;
    do
    {
      AccessModel(v11, aDataObject1, aGrass, v11 - 19);
      ++v11;
    }
    while ( v11 - 20 < 8 );
    v12 = 30;
    do
    {
      AccessModel(v12, aDataObject1, aStone, v12 - 29);
      ++v12;
    }
    while ( v12 - 30 < 5 );
    v13 = 40;
    do
    {
      AccessModel(v13, aDataObject1, aStonestatue, v13 - 39);
      ++v13;
    }
    while ( v13 - 40 < 3 );
    AccessModel(43, aDataObject1, aSteelstatue, 1);
    v14 = 44;
    do
    {
      AccessModel(v14, aDataObject1, aTomb, v14 - 43);
      ++v14;
    }
    while ( v14 - 44 < 3 );
    v15 = 50;
    do
    {
      AccessModel(v15, aDataObject1, aFirelight, v15 - 49);
      ++v15;
    }
    while ( v15 - 50 < 2 );
    AccessModel(52, aDataObject1, aBonfire, 1);
    AccessModel(55, aDataObject1, aDoungeongate, 1);
    AccessModel(58, aDataObject1, aTreasuredrum, 1);
    AccessModel(59, aDataObject1, aTreasurechest, 1);
    AccessModel(60, aDataObject1, aShip, 1);
    v16 = 69;
    do
    {
      AccessModel(v16, aDataObject1, aStonewall, v16 - 68);
      ++v16;
    }
    while ( v16 - 69 < 6 );
    v17 = 75;
    do
    {
      AccessModel(v17, aDataObject1, aStonemuwall, v17 - 74);
      ++v17;
    }
    while ( v17 - 75 < 4 );
    v18 = 65;
    do
    {
      AccessModel(v18, aDataObject1, aSteelwall, v18 - 64);
      ++v18;
    }
    while ( v18 - 65 < 3 );
    AccessModel(68, aDataObject1, aSteeldoor, 1);
    v19 = 91;
    do
    {
      AccessModel(v19, aDataObject1, aCannon, v19 - 90);
      ++v19;
    }
    while ( v19 - 91 < 3 );
    AccessModel(80, aDataObject1, aBridge, 1);
    v20 = 81;
    do
    {
      AccessModel(v20, aDataObject1, aFence, v20 - 80);
      ++v20;
    }
    while ( v20 - 81 < 4 );
    AccessModel(85, aDataObject1, aBridgestone, 1);
    AccessModel(90, aDataObject1, aStreetlight, 1);
    AccessModel(95, aDataObject1, aCurtain, 1);
    v21 = 98;
    do
    {
      AccessModel(v21, aDataObject1, aCarriage, v21 - 97);
      ++v21;
    }
    while ( v21 - 98 < 4 );
    v22 = 102;
    do
    {
      AccessModel(v22, aDataObject1, aStraw, v22 - 101);
      ++v22;
    }
    while ( v22 - 102 < 2 );
    v23 = 96;
    do
    {
      AccessModel(v23, aDataObject1, aSign, v23 - 95);
      ++v23;
    }
    while ( v23 - 96 < 2 );
    v24 = 56;
    do
    {
      AccessModel(v24, aDataObject1, aMerchantanimal, v24 - 55);
      ++v24;
    }
    while ( v24 - 56 < 2 );
    AccessModel(105, aDataObject1, aWaterspout, 1);
    v25 = 106;
    do
    {
      AccessModel(v25, aDataObject1, aWell, v25 - 105);
      ++v25;
    }
    while ( v25 - 106 < 4 );
    AccessModel(110, aDataObject1, aHanging, 1);
    v26 = 115;
    do
    {
      AccessModel(v26, aDataObject1, aHouse, v26 - 114);
      ++v26;
    }
    while ( v26 - 115 < 5 );
    AccessModel(120, aDataObject1, aTent, 1);
    AccessModel(111, aDataObject1, aStair, 1);
    v27 = 121;
    do
    {
      AccessModel(v27, aDataObject1, aHousewall, v27 - 120);
      ++v27;
    }
    while ( v27 - 121 < 6 );
    v28 = 127;
    do
    {
      AccessModel(v28, aDataObject1, aHouseetc, v28 - 126);
      ++v28;
    }
    while ( v28 - 127 < 3 );
    v29 = 130;
    do
    {
      AccessModel(v29, aDataObject1, aLight, v29 - 129);
      ++v29;
    }
    while ( v29 - 130 < 3 );
    AccessModel(133, aDataObject1, aPosebox, 1);
    v30 = 140;
    do
    {
      AccessModel(v30, aDataObject1, aFurniture, v30 - 139);
      ++v30;
    }
    while ( v30 - 140 < 7 );
    AccessModel(150, aDataObject1, aCandle, 1);
    v31 = 151;
    do
    {
      AccessModel(v31, aDataObject1, "Beer", v31 - 150);
      ++v31;
    }
    while ( v31 - 151 < 3 );
    for ( ii = 0; ii < 160; ++ii )
    {
      OpenTexture(ii, aObject1, 9728, 1);
    }
  }
  if ( DAT_083a410c )
  {
    DAT_0055a7c4 = v39;
  }
}
#endif
