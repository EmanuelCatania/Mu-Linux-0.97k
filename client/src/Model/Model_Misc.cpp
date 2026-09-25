// IDA: OpenSkills @ 0x0050b710 — Model_LoadSkillEffectAssets
// Loads skill effect geometry (SMD meshes for ice, fire, arrows, bone warriors,
// dragon magic, etc.) and their BMD equivalents.  Also loads skill textures
// and flower particle textures.  Covers model slots 0xbe-0x10d.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: OpenSkills
void __cdecl Model_LoadSkillEffectAssets(void)
{
    // ── Korean-locale SMD skill meshes ────────────────────────────────────────
    if (DAT_0055a7c4 == '\0') {
        OpenModel((int)0xbe, "Data2\\Skill\\", "ice.smd");
        OpenModel((int)0xbf, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");       // freeze aura
        OpenModel((int)0xc0, "Data2\\Skill\\", "wall.smd");
        OpenModel((int)0xc5, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xc6, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xc7, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xc8, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xc9, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xca, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xcb, "Data2\\Skill\\", "magic_c.smd");
        OpenModel((int)0xcc, "Data2\\Skill\\", "typhoon.smd");
        OpenModel((int)0xcd, "Data2\\Skill\\", "dargon_magic.smd");
        OpenModel((int)0xce, "Data2\\Skill\\", "bones_warrior.smd");
        OpenModel((int)0xcf, "Data2\\Skill\\", "bone_a.smd");
        OpenModel((int)0xd0, "Data2\\Skill\\", "bone_c.smd");
        OpenModel((int)0xd1, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");       // saw
        OpenModel((int)0xd2, "Data2\\Skill\\", "bons_p01.smd");
        OpenModel((int)0xd3, "Data2\\Skill\\", "bons_p02.smd");
        OpenModel((int)0xd4, "Data2\\Skill\\", "snow_p01.smd");
        OpenModel((int)0xd5, "Data2\\Skill\\", "snow_p02.smd");
        OpenModel((int)0xd6, "Data2\\Skill\\", "snow_p03.smd");
        OpenModel((int)0xc3, "Data2\\Skill\\", "unicon.smd");
        OpenModel((int)0x10c, "Data2\\Skill\\", "wall.smd");   // PhoenixShield reuse
        OpenModel((int)0xd8, "Data2\\Skill\\", "arrow.smd");
        OpenModel((int)0xd9, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xda, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xdb, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xdc, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xdd, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xde, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0x103, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xe1, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xe2, "Data2\\Skill\\", "mston_a.smd");
        OpenModel((int)0xe3, "Data2\\Skill\\", "mston_b.smd");
        OpenModel((int)0xe9, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xdf, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xe0, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xec, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf0, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf1, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf2, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0x10a, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf5, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf6, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf7, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf8, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf9, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfa, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfb, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfc, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfd, "Data2\\Skill\\", "magic2.smd");
        OpenModel((int)0xfe, "Data2\\Skill\\", "magic3.smd");
        if (DAT_0055a7c4 == '\0')
            OpenModel((int)0xff, "Data2\\Skill\\", "piercing.smd");
    }

    // ── Compressed-asset (BMD) skill loads ────────────────────────────────────
    DAT_0055a7c4 = '\x01';
    FUN_005060b0(0xbe,  "Data\\Skill\\", "Ice",   1);
    FUN_005060b0(199,   "Data\\Skill\\", "Ice",   2);
    FUN_005060b0(0xbf,  "Data\\Skill\\", "Fire",  1);
    FUN_005060b0(0xc0,  "Data\\Skill\\", "Poison",1);
    // 2026-09-02 (Inferno sin fuego / "solo un pedazo del circulo"): estos
    // bucles venian con la BASE de la condicion tomada del argumento en vez del
    // valor inicial.  IDA los escribe asi (OpenSkills 0x0050B710 L91-97):
    //     v1 = 206;
    //     do { AccessModel(v1, ..., v1 - 205); ++v1; } while ( v1 - 206 < 3 );
    // o sea la condicion usa el INICIO (206) y el argumento otra base (205).
    // El port usaba la del argumento en los dos lados, asi que cada bucle
    // cargaba (N - (inicio - base)) modelos en vez de N.
    //
    // Efecto medido con la sonda INFERNO: `mdl197=1 mdl198=0`, o sea Stone02
    // nunca se abria.  Effect_SpawnBombRing elige `rand()%2 + 197` en cada una
    // de las 8 posiciones del anillo, asi que ~la mitad de los efectos apuntaba
    // a un modelo vacio: de ahi "solo carga un pedazo del circulo".
    // Habia 17 bucles con el mismo error (ver Model_Players.cpp).
    for (int i = 0xc5; i-0xc5 < 2; i++)
        FUN_005060b0(i, "Data\\Skill\\", "Stone", i - 0xc4);
    FUN_005060b0(200,   "Data\\Skill\\", "Circle",1);
    FUN_005060b0(0xc9,  "Data\\Skill\\", "Circle",2);
    FUN_005060b0(0xca,  "Data\\Skill\\", "Magic", 1);
    FUN_005060b0(0xcb,  "Data\\Skill\\", "Magic", 2);
    FUN_005060b0(0xcc,  "Data\\Skill\\", "Storm", 1);
    FUN_005060b0(0xcd,  "Data\\Skill\\", "Laser", 1);
    for (int i = 0xce; i-0xce < 3; i++)
        FUN_005060b0(i, "Data\\Skill\\", "Skeleton", i - 0xcd);
    FUN_005060b0(0xd1,  "Data\\Skill\\", "Saw",   1);
    for (int i = 0xd2; i-0xd2 < 2; i++)
        FUN_005060b0(i, "Data\\Skill\\", "Bone",  i - 0xd1);
    for (int i = 0xd4; i-0xd4 < 3; i++)
        FUN_005060b0(i, "Data\\Skill\\", "Snow",  i - 0xd3);
    FUN_005060b0(0xc3,  "Data\\Skill\\", "Rider", 1);

    if (DAT_0055a7c4 == '\0')
        OpenModel((int)0x10b, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
    FUN_005060b0(0x10b, "Data\\Skill\\", "Rider", 2);
    DAT_0055a7c4 = '\x01';

    FUN_005060b0(0x10c, "Data\\Skill\\", "PhoenixShield", 1);
    FUN_005060b0(0xd8,  "Data\\Skill\\", "Arrow",         1);
    FUN_005060b0(0xd9,  "Data\\Skill\\", "ArrowSteel",    1);
    FUN_005060b0(0xda,  "Data\\Skill\\", "ArrowThunder",  1);
    FUN_005060b0(0xdb,  "Data\\Skill\\", "ArrowLaser",    1);
    FUN_005060b0(0xdc,  "Data\\Skill\\", "ArrowV",        1);
    FUN_005060b0(0xdd,  "Data\\Skill\\", "ArrowSaw",      1);
    FUN_005060b0(0xde,  "Data\\Skill\\", "ArrowNature",   1);

    OpenTexture(0x103, "Skill\\", 0x2600, '\x01');
    FUN_005060b0(0x103, "Data\\Skill\\", "Protect",     2);
    OpenTexture(0x10a, "Skill\\", 0x2600, '\x01');
    FUN_005060b0(0x10a, "Data\\Skill\\", "RidingSpear", 1);
    FUN_005060b0(0xe1,  "Data\\Skill\\", "Protect",     1);
    for (int i = 0xe2; i-0xe2 < 2; i++)
        FUN_005060b0(i, "Data\\Skill\\", "BigStone", i - 0xe1);
    FUN_005060b0(0xe9,  "Data\\Skill\\", "MagicCircle", 1);
    FUN_005060b0(0xe0,  "Data\\Skill\\", "ArrowWing",   1);
    FUN_005060b0(0xdf,  "Data\\Skill\\", "ArrowBomb",   1);
    FUN_005060b0(0xec,  "Data\\Skill\\", "Ball",        1);

    // Scale adjustment for slot 0xec — DESHABILITADO
    // 0xad80*4 = 0x2B600 → no es índice, sino absoluto 0x05828d58+0x2B600 = 0x05854358
    // (otro DAT_ en el binario original). Ghidra lo renderizó como base+index por error.
    // *(DWORD*)(*(DWORD*)((BYTE*)DAT_05828d58 + 0xad80*4) + 4) = 0x3f000000; // 0.5f

    FUN_005060b0(0xf0,  "Data\\Skill\\", "Blast",       1);
    FUN_005060b0(0xf1,  "Data\\Skill\\", "Inferno",     1);
    FUN_005060b0(0xf2,  "Data\\Skill\\", "ArrowDouble", 1);

    // EarthQuake variants (Korean SMD re-load before BMD)
    if (DAT_0055a7c4 == '\0') {
        OpenModel((int)0xf5, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf6, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf7, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xf9, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfb, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfc, "Data2\\Skill\\", "\xBD\xBA\xC5\xC0.smd");
        OpenModel((int)0xfd, "Data2\\Skill\\", "magic2.smd");
        OpenModel((int)0xfe, "Data2\\Skill\\", "magic3.smd");
    }
    FUN_005060b0(0xf5, "Data\\Skill\\", "EarthQuake", 1);
    FUN_005060b0(0xf6, "Data\\Skill\\", "EarthQuake", 2);
    FUN_005060b0(0xf7, "Data\\Skill\\", "EarthQuake", 3);
    FUN_005060b0(0xf9, "Data\\Skill\\", "EarthQuake", 5);
    FUN_005060b0(0xfb, "Data\\Skill\\", "EarthQuake", 7);
    FUN_005060b0(0xfc, "Data\\Skill\\", "EarthQuake", 8);
    FUN_005060b0(0xfd, "Data\\Skill\\", "flashing",  -1);
    FUN_005060b0(0xfe, "Data\\Skill\\", "tail",      -1);
    DAT_0055a7c4 = 1;
    FUN_005060b0(0xf8, "Data\\Skill\\", "EarthQuake", 4);
    FUN_005060b0(0xfa, "Data\\Skill\\", "EarthQuake", 6);
    FUN_005060b0(0xff, "Data\\Skill\\", "Piercing",  -1);

    // ── Skill texture bindings ────────────────────────────────────────────────
    SetMaxTextures(700);
    OpenTexture(0x10b, "Skill\\", 0x2600, '\x01');
    OpenTexture(0x10c, "Skill\\", 0x2600, '\x01');
    OpenTexture(0xf5,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xf6,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xf7,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xf9,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xfb,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xfc,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xfd,  "Skill\\", 0x2600, '\x01');
    OpenTexture(0xfe,  "Skill\\", 0x2600, '\x01');
    DAT_0055a7c4 = 1;

    // IDA 0.98j: OpenSkills (0x523DC0) carga el Tamachan en el slot 183:
    //     AccessModel(183, "Data\\Skill\\", "Tama", 1);
    // Desviacion: en el 0.97k el 183 lo pisan los peces de Atlans
    // (OpenWorldModels case 7), asi que va al 193 (0xC1), libre y dentro de los
    // rangos que dibuja RenderEffects (190..268) y anima MoveEffect (174..268).
    // Sus texturas las cubre el bucle de abajo.
    FUN_005060b0(0xc1,  "Data\\Skill\\", "Tama", 1);

    // Full skill texture range (0xbe-0x10c)
    for (int i = 0xbe; i < 0x10d; i++) {
        int slot = (i == 0xff) ? 0xff : i;
        OpenTexture(slot, "Skill\\", 0x2600, '\x01');
    }

    // Flower particle textures
    OpenTGA("Skill\\flower1.tga", 0x4da, 0x2600, 0x2900, 0, '\x01');
    OpenTGA("Skill\\flower2.tga", 0x4db, 0x2600, 0x2900, 0, '\x01');
    OpenTGA("Skill\\flower3.tga", 0x4dc, 0x2600, 0x2900, 0, '\x01');
    DAT_0055a7c4 = 1;
}
