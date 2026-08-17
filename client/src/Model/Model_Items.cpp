// Model_LoadItems @ 0x00506170
// Allocates the model data pool, zeroes it, then loads the player body mesh
// (model slot 0x186) with all its animations (Korean-locale paths when
// DAT_0055a7c4 == 0), plus all equipment piece slots (Helm/Armor/Pant/Glove/Boot
// for every class/tier), item slots, and shadow model.  Ends by calling
// FUN_00443e70 (Entity_LockAnim) and setting scale tables in the model block.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// Model_LoadItems — loads player body + all equipment model slots.
// Called once from FUN_0050e5a0 (Map_LoadResources) during startup.
void __cdecl FUN_00506170(void)
{
    // Allocate raw model pool (0x5b334 bytes) and align the base pointer
    // with a random offset to frustrate memory-address fingerprinting.
    DAT_06f42a58 = operator new(0x5b334);
    unsigned int rnd = (unsigned int)rand() & 0x800003ff;
    if ((int)rnd < 0) rnd = (rnd - 1 | 0xfffffc00) + 1;
    DAT_05828d58 = (DWORD)((char*)DAT_06f42a58 + rnd * 0xbc);

    // Zero-fill 0xb0cd DWORDs (= the entire model table region)
    DWORD* p = (DWORD*)DAT_05828d58;
    for (int i = 0xb0cd; i != 0; i--) *p++ = 0;

    // ── Korean-locale SMD animations (skipped in compressed-asset mode) ──────
    if (DAT_0055a7c4 == '\0') {
        // Player body (female base mesh, slot 0x186, 0x8d = 141 animation slots)
        FUN_0040b280((int)0x186, "Data2\\Player\\BodyFemale.smd", 0x8d, '\0');

        // Idle / stand animations
        FUN_0040b310(0x186, "Data2\\Player\\set_01.smd",            1);
        FUN_0040b310(0x186, "Data2\\Player\\\xB3\xB2\xC0\xDA\xB8\xDE\xBC\xCB\xB5\xBF\xC0\xDB.smd", 0); // ³²ÀڸžÛ·Á
        FUN_0040b310(0x186, "Data2\\Player\\stop_female_01.smd",    0);
        FUN_0040b310(0x186, "Data2\\Player\\stop_sword_01.smd",     0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBB\xC7\xB0\xED\xB5\xBF\xC0\xDB.smd", 0); // »ÇGoµ¿ÀÛ (shield idle)
        FUN_0040b310(0x186, "Data2\\Player\\\xC1\xA4\xB8\xED\xB5\xBF\xC0\xDB.smd", 0); // Á¤¸éµ¿ÀÛ
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xC7\xAE\xBD\xC3\xBD\xBA.smd", 0); // ½ºÅÀÀ§½º
        FUN_0040b310(0x186, "Data2\\Player\\stop_bow_01.smd",       0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBF\xC0\xBD\xBA\xC5\xC0\xC7\xAE.smd", 0); // ¿À½ºÅÀÀ®
        FUN_0040b310(0x186, "Data2\\Player\\stop_fly_01.smd",       0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBF\xC0\xC7\xAE\xC0\xDA\xBB\xF3.smd", 0); // ¿ÀÀ®ÀÚ»ó
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_stop.smd",      0);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_stop_weapon.smd", 0);

        // Run animations
        FUN_0040b310(0x186, "Data2\\Player\\run_male_01.smd",       1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB5\xB5.smd", 1); // ½ºÅÀ·Î (run to stop?)
        FUN_0040b310(0x186, "Data2\\Player\\run_sword_01.smd",      1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xC7\xAE\xC0\xDA\xBB\xF3.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xC1\xA4\xB8\xED.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB8\xDE\xC0\xCC\xBD\xBA.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\run_bow_01.smd",        1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xBD\xBA.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC0\xDA.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xB5\xAF\xAE\xEC.smd", 1);  // µ¿Àۯ
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA.smd", 1);
        FUN_0040b310(0x186, "Data2\\Player\\run_fly_01.smd",        0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_run.smd",       1);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_run_weapon.smd", 1);

        // Attack animations
        FUN_0040b310(0x186, "Data2\\Player\\attack_fist.smd",       0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xC1\xF6\xC8\xF4\xB0\xF8\xB0\xDD_01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xC1\xF6\xC8\xF4\xB0\xF8\xB0\xDD_02.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xC7\xD8\xB0\xF8\xB0\xDD_01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xC7\xD8\xB0\xF8\xB0\xDD_02.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xBD\xCD\xB0\xEE\xB0\xF8\xB0" "\xDD" "01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xBD\xCD\xB0\xEE\xB0\xF8\xB0" "\xDD" "02.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xBD\xCD\xB0\xEE\xB0\xF8\xB0" "\xDD" "03.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xC7\xD8\xC1\xD6\xB0\xEE_\xC7\xEC" "01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xC7\xD8\xC1\xD6\xB0\xEE_\xB3\xB2" "01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0); // 6F4=Hwaro
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xBD\xBA\xBD\xBA\xB0\xF8\xB0" "\xDD" "01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xB0\xF8\xB0\xDD_\xBD\xBA\xBD\xBA\xB0\xF8\xB0" "\xDD" "02.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0); // 7F8=Hwalro
        FUN_0040b310(0x186, "Data2\\Player\\attack_bow_fly_01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\attack_crossbow_fly_01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_attack_sword.smd",   0);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_attack_twohands.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_attack_spear.smd",   0);
        FUN_0040b310(0x186, "Data2\\Player\\uniconp_attack_sickle.smd",  0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);

        // (8 padding idle anims follow from Ghidra loop: iVar4=8 times &DAT_0055bd4c)
        for (int i = 0; i < 8; i++)
            FUN_0040b310(0x186, "Data2\\Player\\\xB3\xB2\xC0\xDA\xB8\xDE\xBC\xCB\xB5\xBF\xC0\xDB.smd", 0);

        // Pegasus / TwoHand Two / Skill animations
        FUN_0040b310(0x186, "Data2\\Player\\Pegasus_fly.smd",              0);
        FUN_0040b310(0x186, "Data2\\Player\\Pegasus_fly_weapon.smd",       0);
        FUN_0040b310(0x186, "Data2\\Player\\stop_TwoHandTwo.smd",          0);
        FUN_0040b310(0x186, "Data2\\Player\\walk_TwoHandTwo.smd",          0);
        FUN_0040b310(0x186, "Data2\\Player\\run_TwoHandTwo.smd",           0);
        FUN_0040b310(0x186, "Data2\\Player\\attack_TwoHandTwo.smd",        0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBF\xEC\xC1\xF6\xB0\xF8\xB0" "\xDD" "01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBF\xEC\xC1\xF6\xB0\xF8\xB0" "\xDD" "02.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xBD" "\xBA" "01.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xBD" "\xBA" "02.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xBD\xBA\xBD" "\xBA" "03.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\Skill_02.smd",                 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\defense_01.smd",               0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\shock_01.smd",                 0);
        FUN_0040b310(0x186, "Data2\\Player\\die_01.smd",                   0);
        FUN_0040b310(0x186, "Data2\\Player\\die_02.smd",                   0);
        FUN_0040b310(0x186, "Data2\\Player\\action_sit1.smd",              0);
        FUN_0040b310(0x186, "Data2\\Player\\action_sit2.smd",              0);
        FUN_0040b310(0x186, "Data2\\Player\\action_sit_f_01.smd",          0);
        FUN_0040b310(0x186, "Data2\\Player\\action_sit_f_02.smd",          0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);
        FUN_0040b310(0x186, "Data2\\Player\\action_pose_01.smd",           0);
        FUN_0040b310(0x186, "Data2\\Player\\\xBD\xBA\xC5\xC0\xB0\xFC\xC3\xA3.smd", 0);

        // Equipment piece loaders (class 1-4): each loads class_N suffix
        for (int i = 1; i <= 4; i++) {
            FUN_00506050((int)(i + 0x38f), "Data2\\Player\\Helm\\class_",  i);
            FUN_00506050((int)(i + 0x396), "Data2\\Player\\Armor\\class_", i);
            FUN_00506050((int)(i + 0x39d), "Data2\\Player\\Pant\\class_",  i);
            FUN_00506050((int)(i + 0x3a4), "Data2\\Player\\Glove\\class_", i);
            FUN_00506050((int)(i + 0x3ab), "Data2\\Player\\Boot\\class_",  i);
        }

        // Male armour tiers (1-10)
        for (int i = 1; i <= 10; i++) {
            FUN_00506050((int)(i + 0x26f), "Data2\\Player\\Helm\\helm_",        i);
            FUN_00506050((int)(i + 0x28f), "Data2\\Player\\Armor\\male_upper_", i);
            FUN_00506050((int)(i + 0x2af), "Data2\\Player\\Pant\\male_lower_",  i);
            FUN_00506050((int)(i + 0x2cf), "Data2\\Player\\Glove\\male_gloves_",i);
            FUN_00506050((int)(i + 0x2ef), "Data2\\Player\\Boot\\male_boots_",  i);
        }

        // Elf armour tiers (1-5, index offset +1 for Pant/Boot)
        for (int i = 1; i <= 5; i++) {
            FUN_00506050((int)(i + 0x279), "Data2\\Player\\Helm\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0", i);
            FUN_00506050((int)(i + 0x299), "Data2\\Player\\Armor\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0", i);
            FUN_00506050((int)(i + 0x2b9), "Data2\\Player\\Pant\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0", i+1);
            FUN_00506050((int)(i + 0x2d9), "Data2\\Player\\Glove\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0", i);
            FUN_00506050((int)(i + 0x2f9), "Data2\\Player\\Boot\\\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0", i+1);
        }

        // High-tier (slot 0x29f/0x2bf/0x2df/0x2ff/0x280 etc.) — single-slot SMD loads
        FUN_00505e90((int)0x29f, "Data2\\Player\\Armor\\", "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x2bf, "Data2\\Player\\Pant\\",  "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x2df, "Data2\\Player\\Glove\\", "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x2ff, "Data2\\Player\\Boot\\",  "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x280, "Data2\\Player\\Helm\\",  "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x2a0, "Data2\\Player\\Armor\\", "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x2c0, "Data2\\Player\\Pant\\",  "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x2e0, "Data2\\Player\\Glove\\", "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");
        FUN_00505e90((int)0x300, "Data2\\Player\\Boot\\",  "\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0\xBD\xBA\xC5\xC0.smd");

        // Shadow model
        FUN_00506050((int)0x187, "Data2\\Player\\Shadow\\Shadow", 1);
    } // end if DAT_0055a7c4 == '\0' (Korean SMD branch)

    // ── Compressed-asset branch (always runs: BMD-based loads) ───────────────
    FUN_005060b0(0x186, "Data\\Player\\", "Player", -1);
    DAT_0055a7c4 = '\x01';

    // Check that BMD loaded successfully — DESHABILITADO
    // 0x47a3*4 = 0x11E8C → dirección absoluta 0x05828d58+0x11E8C = 0x0583ABE4
    // (otro DAT_ en el binario original). Ghidra malrenderizó como base+index.
    // if (*(short*)((BYTE*)DAT_05828d58 + 0x47a3*4) > 0) {
    //     FUN_00405540(nullptr, "Player.bmd file error.\r\n");
    //     MessageBoxA((HWND)(DWORD)DAT_055c9ffc, "Player.bmd file error!!!", nullptr, 0);
    //     SendMessageA((HWND)(DWORD)DAT_055c9ffc, WM_CLOSE, 0, 0);
    // }

    // Class armour BMD (class 1-4)
    for (int i = 1; i <= 4; i++) {
        FUN_005060b0(i + 0x38f, "Data\\Player\\", "HelmClass",  i);
        FUN_005060b0(i + 0x396, "Data\\Player\\", "ArmorClass", i);
        FUN_005060b0(i + 0x39d, "Data\\Player\\", "PantClass",  i);
        FUN_005060b0(i + 0x3a4, "Data\\Player\\", "GloveClass", i);
        FUN_005060b0(i + 0x3ab, "Data\\Player\\", "BootClass",  i);
    }

    // Class2 SMD (only in Korean locale)
    // BUGFIX 2026-04-26: era `i+4 < 7` (=i<3, 2 iter); IDA 0x00506170 línea 57
    // `while (v6 + 3 < 7)` = 3 iter (v6=1,2,3) — sin i=3 nunca se carga
    // HelmClass23/ArmorClass23/etc → ME (cls=10) class-default invisible.
    if (DAT_0055a7c4 == '\0') {
        for (int i = 1; i+3 < 7; i++) {
            FUN_00506050((int)(i + 0x393), "Data2\\Player\\Helm\\class2_",  i);
            FUN_00506050((int)(i + 0x39a), "Data2\\Player\\Armor\\class2_", i);
            FUN_00506050((int)(i + 0x3a1), "Data2\\Player\\Pant\\class2_",  i);
            FUN_00506050((int)(i + 0x3a8), "Data2\\Player\\Glove\\class2_", i);
            FUN_00506050((int)(i + 0x3af), "Data2\\Player\\Boot\\class2_",  i);
        }
    }

    // Class2 BMD (classes 1-3) — same off-by-one fix as SMD path
    for (int i = 1; i+3 < 7; i++) {
        FUN_005060b0(i + 0x393, "Data\\Player\\", "HelmClass2",  i);
        FUN_005060b0(i + 0x39a, "Data\\Player\\", "ArmorClass2", i);
        FUN_005060b0(i + 0x3a1, "Data\\Player\\", "PantClass2",  i);
        FUN_005060b0(i + 0x3a8, "Data\\Player\\", "GloveClass2", i);
        FUN_005060b0(i + 0x3af, "Data\\Player\\", "BootClass2",  i);
    }

    DAT_0055a7c4 = '\x01';

    // Male armour BMD tiers (1-10)
    for (int i = 1; i <= 10; i++) {
        FUN_005060b0(i + 0x26f, "Data\\Player\\", "HelmMale",  i);
        FUN_005060b0(i + 0x28f, "Data\\Player\\", "ArmorMale", i);
        FUN_005060b0(i + 0x2af, "Data\\Player\\", "PantMale",  i);
        FUN_005060b0(i + 0x2cf, "Data\\Player\\", "GloveMale", i);
        FUN_005060b0(i + 0x2ef, "Data\\Player\\", "BootMale",  i);
    }

    // Elf armour BMD (1-5)
    for (int i = 1; i <= 5; i++) {
        FUN_005060b0(i + 0x279, "Data\\Player\\", "HelmElf",  i);
        FUN_005060b0(i + 0x299, "Data\\Player\\", "ArmorElf", i);
        FUN_005060b0(i + 0x2b9, "Data\\Player\\", "PantElf",  i);
        FUN_005060b0(i + 0x2d9, "Data\\Player\\", "GloveElf", i);
        FUN_005060b0(i + 0x2f9, "Data\\Player\\", "BootElf",  i);
    }

    // Extra slots (0x10/0x11 index range)
    FUN_005060b0(0x29f, "Data\\Player\\", "ArmorMale", 0x10);
    FUN_005060b0(0x2bf, "Data\\Player\\", "PantMale",  0x10);
    FUN_005060b0(0x2df, "Data\\Player\\", "GloveMale", 0x10);
    FUN_005060b0(0x2ff, "Data\\Player\\", "BootMale",  0x10);
    FUN_005060b0(0x280, "Data\\Player\\", "HelmMale",  0x11);
    FUN_005060b0(0x2a0, "Data\\Player\\", "ArmorMale", 0x11);
    FUN_005060b0(0x2c0, "Data\\Player\\", "PantMale",  0x11);
    FUN_005060b0(0x2e0, "Data\\Player\\", "GloveMale", 0x11);
    FUN_005060b0(0x300, "Data\\Player\\", "BootMale",  0x11);

    // Dark/Soul/Muz/Ma unique armour (Korean SMD only)
    if (DAT_0055a7c4 == '\0') {
        FUN_00505e90((int)0x281, "Data2\\Player\\Helm\\",  "DarkP_Head.smd");
        FUN_00505e90((int)0x2a1, "Data2\\Player\\Armor\\", "DarkP_Armor.smd");
        FUN_00505e90((int)0x2c1, "Data2\\Player\\Pant\\",  "DarkP_Pant.smd");
        FUN_00505e90((int)0x2e1, "Data2\\Player\\Glove\\", "DarkP_Glove.smd");
        FUN_00505e90((int)0x301, "Data2\\Player\\Boot\\",  "DarkP_Boot.smd");
        FUN_00505e90((int)0x282, "Data2\\Player\\Helm\\",  "Soul_Helm.smd");
        FUN_00505e90((int)0x2a2, "Data2\\Player\\Armor\\", "Soul_Armor.smd");
        FUN_00505e90((int)0x2c2, "Data2\\Player\\Pant\\",  "Soul_Pants.smd");
        FUN_00505e90((int)0x2e2, "Data2\\Player\\Glove\\", "Soul_Gloves.smd");
        FUN_00505e90((int)0x302, "Data2\\Player\\Boot\\",  "Soul_Boots.smd");
        FUN_00505e90((int)0x283, "Data2\\Player\\Helm\\",  "Muz_Helm.smd");
        FUN_00505e90((int)0x2a3, "Data2\\Player\\Armor\\", "Muz_Armor.smd");
        FUN_00505e90((int)0x2c3, "Data2\\Player\\Pant\\",  "Muz_Pants.smd");
        FUN_00505e90((int)0x2e3, "Data2\\Player\\Glove\\", "Muz_Gloves.smd");
        FUN_00505e90((int)0x303, "Data2\\Player\\Boot\\",  "Muz_Boots.smd");
        FUN_00505e90((int)0x2a4, "Data2\\Player\\Armor\\", "Ma_Armor.smd");
        FUN_00505e90((int)0x2c4, "Data2\\Player\\Pant\\",  "Ma_Pant.smd");
        FUN_00505e90((int)0x2e4, "Data2\\Player\\Glove\\", "Ma_Gloves.smd");
        FUN_00505e90((int)0x304, "Data2\\Player\\Boot\\",  "Ma_Boots.smd");
    }

    // Final slot range (0x12-0x15 suffix) — includes helm only for first 3
    // BUGFIX 2026-04-26: el guard era `i-0x11 < 4` (= i < 0x15), perdía la
    // iteración i=0x15 → ArmorMale21/PantMale21/GloveMale21/BootMale21 (idx
    // 0x2A4/0x2C4/0x2E4/0x304) NO se registraban → MG (class=3) con armor
    // tier=20 (Equipment[2]&0xF=4 + bit40 → +16 = 20) renderizaba sólo la
    // cabeza porque los cinco body slots apuntaban a model_idx vacío. IDA
    // 0x00506170 línea 104: `while (v9 - 18 < 4)` = 4 iteraciones (v9=18..21).
    for (int i = 0x12; i-0x12 < 4; i++) {
        if (i - 0x12 < 3)
            FUN_005060b0(i + 0x26f, "Data\\Player\\", "HelmMale",  i);
        FUN_005060b0(i + 0x28f, "Data\\Player\\", "ArmorMale", i);
        FUN_005060b0(i + 0x2af, "Data\\Player\\", "PantMale",  i);
        FUN_005060b0(i + 0x2cf, "Data\\Player\\", "GloveMale", i);
        FUN_005060b0(i + 0x2ef, "Data\\Player\\", "BootMale",  i);
    }

    DAT_0055a7c4 = 1;

    // Shadow BMD
    FUN_005060b0(0x187, "Data\\Player\\", "Shadow", 1);

    // ── Player action PlaySpeed table (IDA OpenPlayers @ 0x00506170 lines 107-154) ──
    // BMD::Open NO carga el campo action+4 (PlaySpeed) del archivo BMD. Quedan en 0
    // (operator_new no inicializa). Sin esta tabla todas las acciones del player
    // tienen speed=0 y CharacterAnimation no avanza ningún frame → personajes
    // estáticos en login y char-select.
    //
    // `Models + 73368` = (BYTE*)DAT_05828d58 + 73320 + 48 = player_model[390].Actions
    // (188*390=73320 base + 48 offset al campo Actions del struct BMD).
    // Cada acción es 16 bytes; offset +4 dentro de la acción es PlaySpeed (float).
    // (offset 20 = action[1].speed, 36 = action[2].speed, ...)
    {
        BYTE* playerModelBase = (BYTE*)(uintptr_t)DAT_05828d58;
        // Aux fields del struct player (no son acciones; offsets internos del modelo)
        *(DWORD*)(playerModelBase + 73404) = 20;
        *(DWORD*)(playerModelBase + 73408) = 6;
        *(DWORD*)(playerModelBase + 73412) = 13;

        DWORD actions = *(DWORD*)(playerModelBase + 73368);
        if (actions) {
            BYTE* A = (BYTE*)(uintptr_t)actions;

            // action[1..12].speed = 0.28f  (idle/stop / walk variants)
            for (int v10 = 32; v10 <= 192; v10 += 16) {
                *(DWORD*)(A + v10 - 12) = 0x3E8F5C29;  // 0.28f
            }
            // Spot speeds for stop/walk hands variants
            *(DWORD*)(A +  52) = 0x3E851EB8;  // 0.26f  action[3]
            *(DWORD*)(A +  68) = 0x3E75C28F;  // 0.24f  action[4]
            *(DWORD*)(A +  84) = 0x3E75C28F;  // 0.24f  action[5]
            *(DWORD*)(A + 116) = 0x3E6147AE;  // 0.22f  action[7]
            *(DWORD*)(A + 132) = 0x3E6147AE;  // 0.22f  action[8]

            // action[12..32].speed = 0.30f (run variants)
            for (int i = 208; i <= 528; i += 16) {
                *(DWORD*)(A + i - 12) = 0x3E99999A;   // 0.30f
            }
            *(DWORD*)(A + 340) = 0x3EB33333;  // 0.35f
            *(DWORD*)(A + 468) = 0x3EB33333;  // 0.35f

            // action[91..129].speed = 0.32f (skill/magic anims)
            for (int j = 1472; j <= 2080; j += 16) {
                *(DWORD*)(A + j - 12) = 0x3EA3D70A;   // 0.32f
            }
            // action[130..131].speed = 0.45f
            for (int k = 2096; k <= 2112; k += 16) {
                *(DWORD*)(A + k - 12) = 0x3EE66666;   // 0.45f
            }
            // action[132..139].speed = 0.40f (sit/die)
            for (int m = 2128; m < 2256; m += 16) {
                *(DWORD*)(A + m - 12) = 0x3ECCCCCD;   // 0.40f
            }

            // Per-action overrides
            *(DWORD*)(A + 2084) = 0x3ECCCCCD;  // 0.40f
            *(DWORD*)(A + 1780) = 0x3E8F5C29;  // 0.28f
            *(DWORD*)(A + 1796) = 0x3E8F5C29;  // 0.28f
            *(DWORD*)(A + 2196) = 0x3E4CCCCD;  // 0.20f
            *(DWORD*)(A + 2212) = 0x3E4CCCCD;  // 0.20f
            *(DWORD*)(A + 1028) = 0x3E4CCCCD;  // 0.20f
            *(DWORD*)(A + 1044) = 0x3E4CCCCD;  // 0.20f
            *(DWORD*)(A + 1252) = 0x3E75C28F;  // 0.24f  PLAYER_STOP_TWO_HAND_SWORD_TWO
            *(DWORD*)(A + 1268) = 0x3E99999A;  // 0.30f  PLAYER_WALK_TWO_HAND_SWORD_TWO
            *(DWORD*)(A + 1284) = 0x3E99999A;  // 0.30f  PLAYER_RUN_TWO_HAND_SWORD_TWO
            *(DWORD*)(A + 1300) = 0x3E75C28F;  // 0.24f  PLAYER_ATTACK_TWO_HAND_SWORD_TWO
            *(DWORD*)(A + 1076) = 0x3EE66666;  // 0.45f

            // Loop flags (action+0 = loop). 1 = loop enabled.
            *(BYTE*)(A + 2096) = 1;
            *(BYTE*)(A + 2112) = 1;
            *(BYTE*)(A + 2064) = 1;
        }
    }

    // Lock animation state machine
    FUN_00443e70();
}
