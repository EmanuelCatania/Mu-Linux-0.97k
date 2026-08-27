// IDA: FUN_0050f030 @ 0x0050F030 — Sound_LoadGameSamples
//
// Precarga los sonidos globales del juego (ids 0..109) via FUN_00404a10
// (LoadWaveFile).  El nombre del archivo viene del port anterior; a pesar de
// llamarse "Gates", esta funcion carga audio, no geometria de puertas.
//
// Firma: LoadWaveFile(id, path, nChannels, b3D)
//
// DESVIACION DELIBERADA (2026-08-17) — nombres de archivo.
// En el binario la mayoria de estos paths estan en coreano (cp949):
// "Data\\Sound\\p걷기(땅).wav", "Data\\Sound\\e타격1.wav", etc.  Nuestro pack de
// assets viene renombrado al ingles (pWalk(Soil).wav, eBlow1.wav, ...), igual
// que los .mp3 — asi que se apunta a los archivos reales.  El comentario al
// final de cada linea conserva el nombre coreano original del binario para
// poder re-cruzarlo.
//
// El mapeo se reconstruyo parseando OpenSounds directamente del `.text` del
// binario original (MD5 eb95ac0785e40a7ad60c9ddb5d8bef34): los literales que
// tenia el port estaban CORRUPTOS — casi todos eran "a\xBD\xBA.wav" /
// "p\xBD\xBA.wav" / "e\xBD\xBA.wav" ("스" repetido como placeholder), o sea
// archivos inexistentes.  Por eso no sonaban pasos, golpes, armas, skills de
// guerrero, magias, gritos, beber pocion ni levantar items: solo funcionaban
// los ~20 ids cuyo nombre ya estaba en ingles en el binario.
//
// NOTA: el orden 82..85 -> sKnightSkill1..4 es la correspondencia secuencial
// entre los 4 ataques de guerrero del binario (내려찍기 / 찌르기 / 올려치기 /
// 돌려치기) y los 4 archivos renombrados.  Lo respalda que el Power Slash
// (case 0x38 de Skills_PacketHandler) usa el id 85 = 돌려치기 = golpe giratorio.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: FUN_0050f030
void __cdecl Sound_LoadGameSamples(void)
{
    FUN_00404a10(0,    "Data\\Sound\\aWind.wav", 1, '\0');   // a바람.wav
    FUN_00404a10(1,    "Data\\Sound\\aRain.wav", 1, '\0');   // a비.wav
    FUN_00404a10(3,    "Data\\Sound\\aDungeon.wav", 1, '\0');   // a던젼.wav
    FUN_00404a10(2,    "Data\\Sound\\aForest.wav", 1, '\0');   // a숲.wav
    FUN_00404a10(5,    "Data\\Sound\\aTower.wav", 1, '\0');   // a탑.wav
    FUN_00404a10(6,    "Data\\Sound\\aWater.wav", 1, '\0');   // a물.wav
    FUN_00404a10(7,    "Data\\Sound\\desert.wav", 1, '\0');   // desert.wav
    FUN_00404a10(8,    "Data\\Sound\\pWalk(Soil).wav", 2, '\0');   // p걷기(땅).wav
    FUN_00404a10(9,    "Data\\Sound\\pWalk(Grass).wav", 2, '\0');   // p걷기(풀).wav
    FUN_00404a10(0xa,  "Data\\Sound\\pWalk(Snow).wav", 2, '\0');   // p걷기(눈).wav
    FUN_00404a10(0xb,  "Data\\Sound\\pSwim.wav", 2, '\0');   // p수영.wav
    FUN_00404a10(0xc,  "Data\\Sound\\aBird1.WAV", 1, '\x01');   // a새1.wav
    FUN_00404a10(0xd,  "Data\\Sound\\aBird2.WAV", 1, '\x01');   // a새2.wav
    FUN_00404a10(0xe,  "Data\\Sound\\aBat.wav", 1, '\x01');   // a박쥐.wav
    FUN_00404a10(0xf,  "Data\\Sound\\aMouse.WAV", 1, '\x01');   // a쥐.wav
    FUN_00404a10(0x10, "Data\\Sound\\aGrate.wav", 1, '\x01');   // a쇠창살.wav
    FUN_00404a10(0x11, "Data\\Sound\\aDoor.wav", 1, '\0');   // a문.wav
    FUN_00404a10(0x12, "Data\\Sound\\aCastleDoor.wav", 1, '\0');   // a성문.wav
    FUN_00404a10(0x14, "Data\\Sound\\aHeaven.wav", 1, '\0');   // aHeaven.wav
    FUN_00404a10(0x15, "Data\\Sound\\aThunder01.wav", 1, '\0');   // aThunder01.wav
    FUN_00404a10(0x16, "Data\\Sound\\aThunder02.wav", 1, '\0');   // aThunder02.wav
    FUN_00404a10(0x17, "Data\\Sound\\aThunder03.wav", 1, '\0');   // aThunder03.wav
    FUN_00404a10(0x32, "Data\\Sound\\eShortBlow1.wav", 2, '\0');   // e짧은타격1.wav
    FUN_00404a10(0x33, "Data\\Sound\\eShortBlow2.wav", 2, '\0');   // e짧은타격2.wav
    FUN_00404a10(0x34, "Data\\Sound\\eShortBlow3.wav", 2, '\0');   // e짧은타격3.wav
    FUN_00404a10(0x35, "Data\\Sound\\eBlow1.wav", 2, '\0');   // e타격1.wav
    FUN_00404a10(0x36, "Data\\Sound\\eBlow2.wav", 2, '\0');   // e타격2.wav
    FUN_00404a10(0x37, "Data\\Sound\\eBlow3.wav", 2, '\0');   // e타격3.wav
    FUN_00404a10(0x38, "Data\\Sound\\eBlow4.wav", 2, '\0');   // e타격4.wav
    FUN_00404a10(0x28, "Data\\Sound\\eSwingWeapon1.wav", 2, '\0');   // e무기휘두르기1.wav
    FUN_00404a10(0x29, "Data\\Sound\\eSwingWeapon2.wav", 2, '\0');   // e무기휘두르기2.wav
    FUN_00404a10(0x2a, "Data\\Sound\\eSwingLightSword.wav", 2, '\0');   // e광선검휘두르기.wav
    FUN_00404a10(0x41, "Data\\Sound\\eBow.wav", 2, '\0');   // e활.wav
    FUN_00404a10(0x42, "Data\\Sound\\eCrossbow.wav", 2, '\0');   // e석궁.wav
    FUN_00404a10(0x43, "Data\\Sound\\eMix.wav", 2, '\0');   // e믹스.wav
    FUN_00404a10(0x20, "Data\\Sound\\pDrink.wav", 1, '\0');   // p마시기.wav
    FUN_00404a10(0x21, "Data\\Sound\\pEatApple.wav", 1, '\0');   // p사과먹기.wav
    FUN_00404a10(0x22, "Data\\Sound\\pHeartBeat.wav", 1, '\0');   // p심장소리.wav
    FUN_00404a10(0x23, "Data\\Sound\\pEnergy.wav", 1, '\0');   // p에너지.wav
    FUN_00404a10(0x4b, "Data\\Sound\\pMaleScream1.wav", 2, '\0');   // p남자비명1.wav
    FUN_00404a10(0x4c, "Data\\Sound\\pMaleScream2.wav", 2, '\0');   // p남자비명2.wav
    FUN_00404a10(0x4d, "Data\\Sound\\pMaleScream3.wav", 2, '\0');   // p남자비명3.wav
    FUN_00404a10(0x4e, "Data\\Sound\\pMaleDie.wav", 2, '\0');   // p남자죽기.wav
    FUN_00404a10(0x4f, "Data\\Sound\\pFemaleScream1.wav", 2, '\0');   // p여자비명1.wav
    FUN_00404a10(0x50, "Data\\Sound\\pFemaleScream2.wav", 2, '\0');   // p여자비명2.wav
    FUN_00404a10(0x1e, "Data\\Sound\\pDropItem.wav", 1, '\0');   // p아이템떨어트리기.wav
    FUN_00404a10(0x1f, "Data\\Sound\\pDropMoney.wav", 1, '\0');   // p금화떨어트리기.wav
    FUN_00404a10(0x31, "Data\\Sound\\eGem.wav", 1, '\0');   // e보석.wav
    FUN_00404a10(0x1d, "Data\\Sound\\pGetItem.wav", 1, '\0');   // p아이템가지기.wav
    FUN_00404a10(0x51, "Data\\Sound\\sKnightDefense.wav", 1, '\0');   // s전사방어.wav
    FUN_00404a10(0x52, "Data\\Sound\\sKnightSkill1.wav", 1, '\0');   // s전사내려찍기.wav
    FUN_00404a10(0x53, "Data\\Sound\\sKnightSkill2.wav", 1, '\0');   // s전사찌르기.wav
    FUN_00404a10(0x54, "Data\\Sound\\sKnightSkill3.wav", 1, '\0');   // s전사올려치기.wav
    FUN_00404a10(0x55, "Data\\Sound\\sKnightSkill4.wav", 1, '\0');   // s전사돌려치기.wav
    FUN_00404a10(0x11e, "Data\\Sound\\mShadowAttack1.wav", 1, '\0');   // m쉐도우공격1.wav
    FUN_00404a10(0x56, "Data\\Sound\\sTornado.wav", 2, '\x01');   // s토네이도.wav
    FUN_00404a10(0x57, "Data\\Sound\\sEvil.wav", 2, '\x01');   // s영혼.wav
    FUN_00404a10(0x58, "Data\\Sound\\sMagic.wav", 2, '\x01');   // s마법.wav
    FUN_00404a10(0x59, "Data\\Sound\\sHellFire.wav", 2, '\x01');   // s헬파이어.wav
    FUN_00404a10(0x5a, "Data\\Sound\\sIce.wav", 2, '\x01');   // s얼음.wav
    FUN_00404a10(0x5b, "Data\\Sound\\sFlame.wav", 2, '\x01');   // s불기둥.wav
    FUN_00404a10(0x5c, "Data\\Sound\\sAquaFlash.wav", 2, '\x01');   // s아쿠아플래쉬.wav
    FUN_00404a10(0x30, "Data\\Sound\\eBreak.wav", 1, '\x01');   // e부서짐.wav
    FUN_00404a10(0x2c, "Data\\Sound\\eExplosion.wav", 1, '\x01');   // e폭파.wav
    FUN_00404a10(0x2e, "Data\\Sound\\eMeteorite.wav", 2, '\x01');   // e운석떨어지기.wav
    FUN_00404a10(0x3c, "Data\\Sound\\eThunder.wav", 1, '\x01');   // e번개.wav
    FUN_00404a10(0x5d, "Data\\Sound\\mBone1.WAV", 2, '\x01');   // m해골1.wav
    FUN_00404a10(0x5e, "Data\\Sound\\mBone2.wav", 2, '\x01');   // m해골2.wav
    FUN_00404a10(0x13, "Data\\Sound\\mAssassin1.wav", 1, '\x01');   // m암살자1.wav
    FUN_00404a10(0x44, "Data\\Sound\\eFirecracker1.wav", 1, '\x01');   // eFirecracker1.wav
    FUN_00404a10(0x45, "Data\\Sound\\eFirecracker2.wav", 1, '\x01');   // eFirecracker2.wav
    FUN_00404a10(0x46, "Data\\Sound\\eMedal.wav", 1, '\x01');   // eMedal.wav
    FUN_00404a10(0x49, "Data\\Sound\\ePhoenixExp.wav", 1, '\x01');   // ePhoenixExp.wav
    FUN_00404a10(0x5f, "Data\\Sound\\eRidingSpear.wav", 1, '\0');   // eRidingSpear.wav
    FUN_00404a10(0x60, "Data\\Sound\\eRaidShoot.wav", 1, '\0');   // eRaidShoot.wav
    FUN_00404a10(0x61, "Data\\Sound\\eSwellLife.wav", 1, '\0');   // eSwellLife.wav
    FUN_00404a10(0x62, "Data\\Sound\\eBlow.wav", 1, '\0');   // eBlow.wav
    FUN_00404a10(0x64, "Data\\Sound\\ePiercing.wav", 1, '\0');   // ePiercing.wav
    FUN_00404a10(0x65, "Data\\Sound\\eIceArrow.wav", 1, '\0');   // eIceArrow.wav
    FUN_00404a10(0x66, "Data\\Sound\\eTelekinesis.wav", 1, '\0');   // eTelekinesis.wav
    FUN_00404a10(0x67, "Data\\Sound\\eSoulBarrier.wav", 1, '\0');   // eSoulBarrier.wav
    FUN_00404a10(0x68, "Data\\Sound\\eBloodAttack.wav", 1, '\0');   // eBloodAttack.wav
    FUN_00404a10(0x69, "Data\\Sound\\eHitGate.wav", 1, '\0');   // eHitGate.wav
    FUN_00404a10(0x6a, "Data\\Sound\\eHitGate2.wav", 1, '\0');   // eHitGate2.wav
    FUN_00404a10(0x6b, "Data\\Sound\\eHitCristal.wav", 1, '\0');   // eHitCristal.wav
    FUN_00404a10(0x6c, "Data\\Sound\\eDownGate.wav", 1, '\0');   // eDownGate.wav
    FUN_00404a10(0x6d, "Data\\Sound\\eCrow.wav", 1, '\0');   // eCrow.wav
}
