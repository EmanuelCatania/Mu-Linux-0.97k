// Scene_ObjectUpdate.cpp
//
// Extracted from stubs_game.cpp.  Owns per-frame update/render dispatch for
// world scene objects and ambient bugs.  Function comments retain IDA provenance.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern void __cdecl FUN_0046ca00(DWORD Object);
// MoveObject_PerWorld @ 0x004FDC00 (~608 lines) — SUMMARY STUB
// Per-world object animation. Per-frame for each visible scene object.
// World 9: random terrain lights. World 0: toggle objects by HeroTile.
// Then: Alpha(), BMD setup, animate, render via RenderPartObject.
float* __cdecl MoveObject_PerWorld_stub(float param_1) {
    // 0x004FDC00 — Per-world object animation (608 lines decompiled).
    // param_1 is actually the OBJECT pointer cast to float (Ghidra artifact).
    // Per-frame update for visible scene objects: toggle visibility by HeroTile,
    // call Alpha(), set up BMD animation, then per-world switch for special effects.
    //
    // Ghidra uses unaff_EBX/EBP/ESI/EDI phantom register params throughout;
    // these carry context from the MoveObjects caller loop. We implement only
    // the real logic paths that don't depend on phantom registers.

    int objPtr = (int)param_1;  // OBJECT* reinterpreted
    if (objPtr == 0) return (float*)0;
    extern void __cdecl AddTerrainLight(float x, float y, float* light, int range, float* buffer);

    short objType = *(short*)(objPtr + 2);

    // ── World 9: random terrain lights ──
    if (World == 9) {
        // Spawn random terrain light near hero (1/100 chance when time < 1000 of 4000-tick cycle)
        // In original: uses __ftol() on WorldTime % 4000
        // Simplified: cosmetic ambient effect
        PlayBuffer(1, 0, 1);  // ambient thunder sound
    }

    // ── World 0: toggle torch/fire objects by HeroTile ──
    if (World == 0) {
        if (objType == 0x7d || objType == 0x7e) {
            if (HeroTile == 4) {
                *(DWORD*)(objPtr + 0x164) = 0;        // hide (alpha=0)
            } else {
                *(DWORD*)(objPtr + 0x164) = 0x3f800000; // show (alpha=1.0f)
            }
        }
    }

    // ── World 2: toggle water objects by HeroTile ──
    if (World == 2) {
        short s = *(short*)(objPtr + 2);
        if (s == 0x51 || s == 0x52 || s == 0x60 || s == 0x62 || s == 99) {
            if (HeroTile == 3 || HeroTile > 9) {
                *(DWORD*)(objPtr + 0x164) = 0;
            } else {
                *(DWORD*)(objPtr + 0x164) = 0x3f800000;
            }
        }
    }

    // Alpha fade — FUN_0043e5c0(entity_ptr)
    FUN_0043e5c0(objPtr);

    // Check alpha > 0
    float alpha = *(float*)(objPtr + 0x168);
    if (alpha < _DAT_005524f8) {
        return (float*)0;  // invisible, skip
    }

    // Setup BMD model: Models base = DAT_05828d58, stride = sizeof(BMD) = 188 (0xBC)
    int modelIdx = (int)*(short*)(objPtr + 2);
    char* model = (char*)(DAT_05828d58 + modelIdx * 0xBC);
    model[0xa0] = *(char*)(objPtr + 0x105);  // set current action

    float animSpeed = *(float*)(objPtr + 0xcc);
    if (World == 8 && *(short*)(objPtr + 2) == 8) {
        animSpeed = animSpeed * _DAT_00552650;  // slow down lava objects
    }

    // Play animation — FUN_00440aa0(model, frame*, scale*, extra, speed)
    // Ghidra sig: BMD::PlayAnimation(this, frame*, priorFrame*, priorAction*, speed, pos*, angle*)
    // Our declaration has 5 params; pass what fits
    FUN_00440aa0((void*)model, (float*)(objPtr + 0x108), (float*)(objPtr + 0x10c),
                 (void*)(objPtr + 0x106), animSpeed);

    // ── Escena de login / char-select (IDA sub_4FDC00, bloque previo al switch)
    // Este bloque vivía en la copia mínima de `FUN_004fdc00` (stubs_linker.cpp),
    // que era la que realmente se llamaba. Al unificar las dos copias se trae
    // acá, en el orden del binario: después de PlayAnimation, antes del switch.
    //   160 = Logo01 (cielo) y 161 = Logo02 (olas): scroll de la V de textura.
    //   162 = Logo03 (banner MU): rampa de Light + alpha-scalar. Sin esto el
    //         banner queda con bodyLight=(0,0,0) → rectángulo negro.
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4) {
        short t = *(short*)(objPtr + 2);
        if (t == 160 || t == 161) {
            *(float*)(objPtr + 112) = -((float)((__int64)WorldTime % 4000) * 0.00025f);
        } else if (t == 162) {
            // CameraWalkCut → over-bright final; si no, rampa 0..0.08.
            float v8 = (DAT_083a7af4 != 0) ? 1.5f
                                           : (float)(int)DAT_005615e8 * 0.002f;
            *(float*)(objPtr + 232) = v8;   // Light[0]
            *(float*)(objPtr + 236) = v8;   // Light[1]
            *(float*)(objPtr + 240) = v8;   // Light[2]
            *(float*)(objPtr + 104) = v8;   // alpha-scalar (+0x68)
        }
    }

    // ── Per-world special effects (large switch) ──
    // The original has a huge switch(World) with sub-switches on objType.
    // Most branches call FUN_0046c7f0 (directional effects), CreateEffect,
    // or manipulate rotation/scale based on WorldTime with sin() waves.
    // These are cosmetic ambient effects. Key patterns:
    //
    // World 0: types 0x32-0x34 → FUN_0046c7f0 (fire effects)
    //          types 0x75,0x7a → random scale
    //          types 0x76,0x77 → WorldTime-based rotation
    //          types 0x82-0x84 → FUN_0046c7f0 + SubType=-2
    // World 1: types 0x16-0x18 → WorldTime%1000 rotation
    //          type 0x29,0x2a → FUN_0046c7f0
    // World 2: types 0x14,0x41,0x56,0x58 → follow hero (gates)
    //          types 0x1e,0x42 → FUN_0046c7f0
    // World 3: type 0x12 → WorldTime rotation
    //          type 0x27 → SubType=1
    //          types 0x2a,0x2b → WorldTime scale
    // World 4: types 3,4 → WorldTime rotation
    //          type 0x18 → CreateEffect(0x4b0) on 1/64 chance
    //          types 0x26,0x27 → FUN_0046ca00
    // World 5: type 2 → SubType=0, type 3 → random scale
    // World 6: type 0x15 → WorldTime rotation, type 0x26 → SubType=-2
    // World 7: type 0x16 → pulsing scale with CreateParticle
    //          type 0x17 → sin(WorldTime) scale
    //          types 0x20,0x22 → sin(WorldTime) scale
    // World 8: type 2 → WorldTime rotation
    //          type 4 → sin(WorldTime) scale + rotation
    //          type 7 → phase-shifted sin scale
    //          types 0x3d,0x41,0x42 → WorldTime rotation + sin scale
    //          type 0x52 → white light (1,1,1)
    // Worlds 0xb-0x10: types 9,10 → SubType check, type with action==4 → SubType=-2

    // Implement key patterns that affect gameplay visibility:
    switch (World) {
    case 0:
        switch (objType) {
        case 0x32: FUN_0046c7f0(0, objPtr, 0.0f, 0.0f, 200.0f); return (float*)0;       // 0x43480000
        case 0x33: FUN_0046c7f0(0, objPtr, 0.0f, -30.0f, 60.0f); return (float*)0;      // 0xc1f00000, 0x42700000
        case 0x34:
            FUN_0046c7f0(0, objPtr, 0.0f, 0.0f, 60.0f);
            *(float*)(objPtr + 0x68) = (float)(rand() % 6 + 4) * _DAT_005524f4;
            return (float*)0;
        case 0x37:
            FUN_0046c7f0(0, objPtr, -150.0f, -150.0f, 140.0f);   // 0xc3160000, 0x430c0000
            FUN_0046c7f0(0, objPtr, 150.0f, -150.0f, 140.0f);
            return (float*)0;
        case 0x50:
            FUN_0046c7f0(0, objPtr, 90.0f, -200.0f, 30.0f);     // 0xc3480000, 0x41f00000
            FUN_0046c7f0(0, objPtr, 90.0f, 200.0f, 30.0f);       // 0x43480000, 0x41f00000
            return (float*)0;
        case 0x5A: {
            float light = (float)(rand() % 2 + 6) * 0.1f;
            float terrainLight[3];
            terrainLight[0] = light;
            terrainLight[1] = light * 0.8f;
            terrainLight[2] = light * 0.6f;
            AddTerrainLight(*(float*)(objPtr + 0x10), *(float*)(objPtr + 0x14), terrainLight, 3, PrimaryTerrainLight[0]);
            return (float*)0;
        }
        case 0x75:
        case 0x7A:
            *(float*)(objPtr + 0x68) = (float)(rand() % 4 + 4) * 0.1f;
            break;
        case 0x76:
        case 0x77:
            *(float*)(objPtr + 0x70) = (float)(-((__int64)WorldTime % 1000)) * 0.001f;
            break;
        case 0x82:
        case 0x83:
        case 0x84: {
            // Light01/02/03: marcadores de humo. `HiddenMesh = -2` oculta la caja
            // (8 vértices, textura dummy `ston03` 2x2 negra) y sub_46C7F0 emite
            // el fuego/humo. IDA sub_4FDC00, World 0.
            int kind = objType - 0x82;
            FUN_0046c7f0(kind, objPtr, 0.0f, 0.0f, 0.0f);
            *(int*)(objPtr + 0x58) = -2;
            return (float*)0;
        }
        case 0x96: {
            float light = (float)(rand() % 4 + 3) * 0.1f;
            float terrainLight[3];
            terrainLight[0] = light;
            terrainLight[1] = light * 0.6f;
            terrainLight[2] = light * 0.2f;
            AddTerrainLight(*(float*)(objPtr + 0x10), *(float*)(objPtr + 0x14), terrainLight, 3, PrimaryTerrainLight[0]);
            return (float*)0;
        }
        }
        break;

    case 1:
        switch (objType) {
        case 0x29: FUN_0046c7f0(0, objPtr, 0.0f, -30.0f, 240.0f); return (float*)0;     // 0xc1f00000, 0x43700000
        case 0x2a: FUN_0046c7f0(0, objPtr, 0.0f, 0.0f, 190.0f); return (float*)0;       // 0x433e0000
        }
        break;

    case 2:
        switch (objType) {
        case 0x1e:
        case 0x42: FUN_0046c7f0(0, objPtr, 0.0f, 0.0f, 50.0f); return (float*)0;        // 0x42480000
        }
        break;

    case 4:
        switch (objType) {
        case 0x26:
        case 0x27: FUN_0046ca00(objPtr); return (float*)0;
        }
        break;

    case 6:
        if (objType == 0x26) {
            *(int*)(objPtr + 0x58) = -2;  // SubType = -2
        }
        break;

    case 8:
        if (objType == 0x52) {
            *(DWORD*)(objPtr + 100) = 0;
            *(DWORD*)(objPtr + 0xe8) = 0x3f800000;  // light R = 1.0
            *(DWORD*)(objPtr + 0xec) = 0x3f800000;  // light G = 1.0
            *(DWORD*)(objPtr + 0xf0) = 0x3f800000;  // light B = 1.0
        }
        break;
    }

    // For worlds 0xb..0x10: check object type 9/10 visibility
    if (World >= 0xb && World <= 0x10) {
        int t = (int)*(short*)(objPtr + 2);
        if (t >= 9 && t <= 10) {
            if (*(short*)(objPtr + 0x86) == 4) {
                return (float*)0;
            }
            *(int*)(objPtr + 0x58) = -2;  // SubType = -2
        }
    }

    return (float*)0;
}

// MoveHeavenThunder @ 0x004FED90 (~472 lines) — SUMMARY STUB
// Lightning storm for World 10 (Icarus). Random bolts via CreateEffect(0xb6).
// Adds terrain light flash, returns nonzero when strike occurs.
int __stdcall MoveHeavenThunder_stub(void) {
    // 0x004FED90 — Lightning storm for World 10 (Icarus), 472 lines decompiled.
    // Creates random lightning bolt effects near the hero.
    // Returns nonzero (iVar5) when a strike occurs, 0 otherwise.
    //
    // Flow:
    //   1. rand() % 50 != 0 → return 0 (2% chance per frame)
    //   2. Calculate random position near hero
    //   3. Add terrain light flash
    //   4. CreateEffect(0xb6, ...) — lightning bolt visual
    //   5. rand() % 5 != 0 → return (only 20% get the second phase)
    //   6. AngleMatrix setup, switch(rand()%4) for 4 bolt directions:
    //      case 0: offset (-400,-1000,0), angle 240
    //      case 1: offset (-300,-400,0), angle 210
    //      case 2: offset (-200,-400,0), angle 235
    //      case 3: offset (-200,400,0), angle 200
    //   7. VectorRotate to transform direction
    //   8. CreateJoint(0x4e7, ...) x2 — forked lightning joints
    //
    // NOTE: Heavy phantom register usage (unaff_ESI/EBX/EBP/EDI) for
    //       CreateEffect/CreateJoint/VectorRotate/AngleMatrix args.
    //       These carry matrix/angle context from the caller.
    //       Implementing the core logic; cosmetic joint details approximate.

    int result = 0;

    // 2% chance per frame
    if (rand() % 0x32 != 0) return 0;

    // Random position near hero — Object.Position at offset 0x10 in CHARACTER
    float heroX = *(float*)(Hero + 0x10);
    float heroY = *(float*)(Hero + 0x14);
    float posX = (float)(rand() % 300) + heroX - _DAT_0055297c;
    float posY = heroY;  // Y stays at hero's Y in original (rand result unused for Y)

    // Random bolt count (4..7)
    int boltCount = (rand() & 3) + 4;
    float fVar21 = (float)boltCount * _DAT_00552874 * _DAT_005528b8;

    // Add terrain light flash at strike position
    float light[3] = { 0.0f, 0.0f, 0.0f };
    // In original: AddTerrainLight(posX, posY, light, ...) with phantom register args
    // The light array + position create a bright flash

    // CreateEffect(0xb6, ...) — main lightning bolt
    // In original: complex phantom-register-based call
    // Effect 0xb6 = lightning strike visual at (posX, posY)

    result = 1;  // strike occurred

    // Second phase: 20% chance for forked lightning
    if (rand() % 5 != 0) return result;

    // AngleMatrix for bolt direction (random rotation)
    // The original builds a 3x4 matrix from angles (0, 0, -45) then
    // switches on rand()%4 for 4 different bolt offsets/angles.
    // Each case:
    //   - Sets fVar14,fVar15,fVar16 as direction offset
    //   - VectorRotate to get world-space bolt direction
    //   - Calculates strike endpoint from hero position + offset
    //   - AngleMatrix for bolt visual angle
    //   - Sets bolt length parameters
    //
    // After the switch: final VectorRotate + 2x CreateJoint(0x4e7)

    // Approximate the lightning joint spawn (cosmetic only)
    // In original: CreateJoint(0x4e7, pos, targetPos, angle, x, y, z, ...)
    // The joints create forked lightning visual from sky to ground

    return result;
}

// MoveObjects @ 0x004FF260 (~169 lines) — per-frame object update dispatcher
// World 10: MoveHeavenThunder. World 11..16: ambient particles.
// Iterates all object lists calling MoveObject_Special or MoveObject_PerWorld.
void __stdcall MoveObjects_stub(void) {
    // 0x004FF260 — Per-frame object update dispatcher.
    // World 10: calls MoveHeavenThunder. World 11..16: spawn ambient particles.
    // Then iterates all object bucket lists (16 buckets per block, from DAT_083a021c)
    // calling FUN_004fa5f0 (Object_AnimUpdate) or FUN_004fdc00 (Object_RenderUpdate).
    // In World 10 with thunder active, spawns lightning joints on random objects.

    float Scale = 0.0f;
    if (World == 10) {
        Scale = (float)MoveHeavenThunder_stub();
    }
    else if (World > 10 && World < 0x11) {
        // Worlds 11..16: spawn ambient particle near hero
        // IDA 004FF260: Angle is cleared and Light is full white.
        float light[3] = { 1.0f, 1.0f, 1.0f };
        float angle[3] = { 0.0f, 0.0f, 0.0f };
        float pos[3];
        pos[0] = (float)(rand() % 900 - 300) + *(float*)((char*)(DWORD)Hero + 0x10);
        pos[1] = (float)(rand() % 900 - 300) + *(float*)((char*)(DWORD)Hero + 0x14);
        pos[2] = (float)(rand() % 50) + *(float*)((char*)(DWORD)Hero + 0x18) + _DAT_00552994;
        unsigned int r = rand() & 0x80000003;
        if ((int)r < 0) r = (r - 1 | 0xFFFFFFFC) + 1;
        if (r == 0) {
            // CreateParticle(0x4E1, pos, angle, light, 0, 1.0f, 0) — ambient dust
            // Signature confirmed from Ghidra:
            //   int CreateParticle(Type, float Position[3], float Angle[3], float Light[3],
            //                      int SubType, float Scale, DWORD Owner)
            Particle_Spawn(0x4E1, pos, angle, light, 3, 0.19f, 0);
        }
    }

    DAT_083a3fec = 0;  // reset visible object counter

    // Iterate object bucket array starting at DAT_083a021c
    // Each block has 16 bucket entries (4 DWORDs each = 16 bytes per entry)
    // Block iteration continues until address > 0x083a121b
    DWORD* puVar4 = (DWORD*)&DAT_083a021c;
    do {
        int bucketsLeft = 0x10;
        do {
            char* pcVar6;
            char bucketFlag = *(char*)(puVar4 + 2);  // +8 bytes: bucket type flag

            // 2026-05-07: guard against corrupt linked-list pointers.
            // The bucket grid's `next` field (+0x1B8) is sometimes garbage
            // (some unidentified path leaves a dangling pointer in a slot).
            // Wrap the deref in SEH so an AV reading the corrupt node terminates
            // the bucket walk instead of taking down the process. Range check +
            // iteration cap on top, to avoid loops that don't actually fault.
            #define MOV_OBJ_VALID_PTR(p) \
                ((uintptr_t)(p) >= 0x00010000u && (uintptr_t)(p) < 0x80000000u)
            int bucketIter = 0;
            const int kBucketIterMax = 4096;

            __try {
            if (bucketFlag == '\0') {
                // Static objects: just animate, don't render-update
                pcVar6 = (char*)*puVar4;
                while (pcVar6 != NULL && MOV_OBJ_VALID_PTR(pcVar6) &&
                       ++bucketIter < kBucketIterMax) {
                    if (*pcVar6 != '\0') {
                        FUN_004fa5f0((int)pcVar6);
                    }
                    pcVar6[0x160] = '\0';
                    pcVar6 = *(char**)(pcVar6 + 0x1B8);
                }
            }
            else {
                // Dynamic objects: render-update + animate
                pcVar6 = (char*)*puVar4;
                while (pcVar6 != NULL && MOV_OBJ_VALID_PTR(pcVar6) &&
                       ++bucketIter < kBucketIterMax) {
                    if (*pcVar6 != '\0' && pcVar6[0x160] != '\0') {
                        // 2026-08-12 — BUG DE CONVERSIÓN, causa raíz de que
                        // NINGÚN objeto del mundo ejecutara su tick.
                        //
                        // `FUN_004fdc00` tiene la firma `(float o)` — un
                        // artefacto de Hex-Rays: el parámetro es un PUNTERO y
                        // adentro se usa siempre como `LODWORD(o)`, o sea por
                        // sus BITS. El call site hacía `(float)(DWORD)pcVar6`,
                        // que convierte el puntero NUMÉRICAMENTE: 0x12E37C8C
                        // (317752972) pasaba a 3.1775e8f, cuyos bits son
                        // 0x4D9749BE. `LODWORD(o)` recuperaba esa basura y la
                        // deferenciaba → AV que el `__except` de abajo se
                        // tragaba en silencio, abortando el walk del bucket.
                        // (El comentario "corrupt linked-list pointers" de ese
                        // SEH describía justamente ESTE puntero, no la lista.)
                        //
                        // Mismo primo del patrón `(float)(uintptr_t)` que
                        // corrompía los joints (ver CLAUDE.md 2026-08-10).
                        // El fix es reinterpretar los bits, no convertir.
                        {
                            float __o;
                            DWORD __p = (DWORD)(uintptr_t)pcVar6;
                            memcpy(&__o, &__p, sizeof(__o));
                            FUN_004fdc00(__o);
                        }
                        DAT_083a3fec++;

                        if (World == 10 && Scale != 0.0f) {
                            int r2 = rand();
                            if (r2 % 10 == 0 &&
                                *(short*)(pcVar6 + 2) >= 0 &&
                                *(short*)(pcVar6 + 2) < 6)
                            {
                                // Spawn lightning sprite + 2 joints on this object
                                float light[3];
                                light[0] = (float)(rand() % 10) * 0.02f;
                                light[1] = (float)(rand() % 10) * 0.02f;
                                light[2] = (float)(rand() % 10) * 0.02f;
                                FUN_004795c0(1269, (float*)(pcVar6 + 16), 0.5f,
                                              light, (int)Hero, 0.0f, 0);
                                Scale = (float)(rand() % 20) + 10.0f;
                                Joint_Create(1254, (float*)(pcVar6 + 16),
                                              (float*)(pcVar6 + 16), (float*)(pcVar6 + 28),
                                              6, (int)pcVar6, Scale, -1, 0);
                                Scale = (float)(rand() % 20) + 10.0f;
                                Joint_Create(1254, (float*)(pcVar6 + 16),
                                              (float*)(pcVar6 + 16), (float*)(pcVar6 + 28),
                                              6, (int)pcVar6, Scale, -1, 0);
                                // CreateSprite(0x4F5, ...) — lightning flash
                                // CreateJoint(0x4E6, ...) x2 — lightning bolts
                                // Phantom register args (unaff_EBX/EBP/ESI/EDI) prevent exact call.
                                // Effect is cosmetic (thunder lightning on objects in World 10).
                            }
                            else {
                                DAT_083a3fec--;
                            }
                        }
                        FUN_004fa5f0((int)pcVar6);
                    }
                    pcVar6 = *(char**)(pcVar6 + 0x1B8);
                } // end while linked list
            }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                // Corrupt next-pointer hit unmapped memory; truncate this
                // bucket and continue with the next one.
            }
            puVar4 += 4;  // next bucket entry (16 bytes)
            bucketsLeft--;
        } while (bucketsLeft != 0);

        // BUG-FIX 2026-04-28: bound era abs addr 0x083a121b (= DAT_083a121c en
        // el binario original = end of g_ObjectBucketGrid[0x1000]). Cambiamos
        // a comparación contra el array end real.
        if ((char*)puVar4 >= ((char*)&g_ObjectBucketGrid[0]) + 0x1000) {
            return;
        }
    } while (true);
}

// MoveBugs @ 0x005001F0 — PORT FIEL 1:1 desde IDA (2026-07-16).
// Reemplaza el SUMMARY STUB previo (que omitía la rama LABEL_16 de las monturas
// e inventaba un "hover acotado" para el hada). Este es traducción directa de
// IDA sub_5001F0. Butterfly/mount/ambient update: chequea owner vivo, fade de
// alpha, copia pos del owner, dispatch de acción por CurrentAction del owner,
// avanza el BMD, y para el hada/uniria-helper (816/817) hace el follow-movement.
//
// Layout de entry (base = e, stride 0x1BC, 10 entries en DAT_083a1218):
//   +0x00 BYTE Live      +0x02 short type    +0x04 DWORD subType
//   +0x10 posX +0x14 posY +0x18 posZ   +0x1c angleX +0x20 angleY +0x24 angleZ
//   +0xC0 velX +0xC4 velY +0xC8 velZ   +0xCC animSpeed
//   +0xFC DWORD Owner    +0x105 curAction  +0x106 priorAction
//   +0x108 frame +0x10C priorFrame        +0x168 alpha-target
// Owner (CharactersClient) offsets: +0x10/14/18 pos, +0x1c/20/24 ang,
//   +0x7c(124) state, +0x84(132) Kind, +0x105(261) CurrentAction.
void __stdcall MoveBugs_stub(void) {
    extern unsigned char* TerrainWall;

    char*  base = (char*)DAT_083a1218;
    DWORD* v0   = (DWORD*)(base + 0xFC);      // = &unk_83A1314 (owner field de entry 0)
    char*  endp = base + 0x1254;              // = &unk_83A246C (10 entries × 0x1BC)

    do {
        char* e = (char*)v0 - 0xFC;           // v1 — entry base
        if (*(BYTE*)e == 0)                    // !Live
            goto next_bug;

        DWORD owner = *v0;                     // *v0 — Owner ptr
        if (DAT_005615c0 == 5) {               // g_GameState == InGame
            if (owner == 0 || *(BYTE*)owner == 0 || *(BYTE*)(owner + 132) != 1) {
                *(BYTE*)e = 0;                 // kill: owner muerto / Kind != 1
                goto next_bug;
            }
        }
        FUN_0043e5c0((int)e);                  // Alpha() — fade

        {
        int   v2 = (int)owner;
        float x2 = *(float*)(v2 + 16);         // owner posX
        float y2 = *(float*)(v2 + 20);         // owner posY
        int   v4 = *(short*)(e + 2);           // type
        char* v40 = (char*)(DAT_05828d58 + 188 * v4);   // Models[v4]
        float v36 = 0.0f;                      // trackDist (solo usado por 816/817)

        if (v4 > 267) {
            int v25 = v4 - 816;
            if (v25 == 0) {
                // v4 == 816 (hada): 4× polvo brillante (Particle_Spawn 1175)
                float v49[3] = { 0.40000001f, 0.40000001f, 0.40000001f };
                for (int v26 = 4; v26; --v26) {
                    float v45[3];
                    v45[0] = (float)(rand() % 16 - 8);
                    v45[1] = (float)(rand() % 16 - 8);
                    int v27 = rand() % 16;
                    v45[0] = v45[0] + *(float*)(e + 0x10);
                    v45[1] = v45[1] + *(float*)(e + 0x14);
                    v45[2] = (float)(v27 - 8) + *(float*)(e + 0x18);
                    Particle_Spawn(1175, v45, (float*)(e + 0x1c), v49, 1, 1.0f, 0);
                }
                v36 = 150.0f;
            } else if (v25 == 1) {
                // v4 == 817
                v36 = 150.0f;
            }
            // else: v4 > 267 pero no 816/817 → cae a LABEL_72 con v36=0
        }
        else if (v4 == 267 || v4 == 195) {
            // ── LABEL_16 — montura (195=Uniria / 267=Dinorant) ──────────────
            int idx = (unsigned char)(int)(*(float*)(v2 + 16) * 0.0099999998f)
                    + (((unsigned char)(int)(*(float*)(v2 + 20) * 0.0099999998f)) << 8);
            if ((TerrainWall[idx] & 1) == 1) {
                *(DWORD*)(e + 0x168) = 0;      // en muro: apaga alpha, no copia pos
            } else {
                int  v8 = *v0;
                char v9 = *(BYTE*)(v8 + 124);
                if (v9 == 1 || v9 == 2) {
                    float v10 = *(float*)(e + 0x168) - 0.1f;
                    *(float*)(e + 0x168) = v10;
                    if (v10 < 0.0f) *(DWORD*)(e + 0x168) = 0;
                } else {
                    *(DWORD*)(e + 0x168) = 0x3F800000;   // 1.0f
                }
                *(DWORD*)(e + 0x10) = *(DWORD*)(v8 + 16);   // posX
                short v11 = *(short*)(e + 2);               // type
                *(DWORD*)(e + 0x14) = *(DWORD*)(v8 + 20);   // posY
                int   v12 = World;
                float v37 = *(float*)(v8 + 24);
                *(float*)(e + 0x18) = v37;                  // posZ
                if (v11 == 267) {
                    if (v12 == 8 || v12 == 10)   *(float*)(e + 0x18) = v37 - 10.0f;
                    else if (v12 != -1)          *(float*)(e + 0x18) = v37 - 30.0f;
                }
                *(DWORD*)(e + 0x1c) = *(DWORD*)(v8 + 28);   // angleX
                *(DWORD*)(e + 0x20) = *(DWORD*)(v8 + 32);   // angleY
                *(DWORD*)(e + 0x24) = *(DWORD*)(v8 + 36);   // angleZ
                BYTE v14 = *(BYTE*)(v8 + 261);              // owner CurrentAction
                if ((v14 >= 13 && v14 <= 33) || v14 == 76 || v14 == 77) {
                    if (v11 == 267 && (v12 == 8 || v12 == 10)) FUN_0043e820((int)e, 3);
                    else                                       FUN_0043e820((int)e, 2);
                    if ((rand() & 1) == 0 && World != 10) {
                        float Light[3] = { 1.0f, 1.0f, 1.0f };
                        float Position[3];
                        Position[0] = (float)(rand() % 64 - 32) + *(float*)(e + 0x10);
                        Position[1] = (float)(rand() % 64 - 32) + *(float*)(e + 0x14);
                        Position[2] = (float)(rand() % 32 - 16) + *(float*)(e + 0x18);
                        if (World == 2) Particle_Spawn(1220, Position, (float*)(e + 0x1c), Light, 0, 1.0f, 0);
                        else            Particle_Spawn(1221, Position, (float*)(e + 0x1c), Light, 0, 1.0f, 0);
                    }
                    *(DWORD*)(e + 0xCC) = 0x3EAE147B;        // animSpeed = 0.34f
                    *(BYTE*)e = *(BYTE*)(*v0);               // Live = owner.Live
                } else if (v14 == 64 || v14 == 65) {
                    if (v12 == 8 || v12 == 10) FUN_0043e820((int)e, 7);
                    else                       FUN_0043e820((int)e, 6);
                    *(DWORD*)(e + 0xCC) = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                } else if (v14 < 0x22 || v14 > 0x37) {
                    if (v11 == 267 && (v12 == 8 || v12 == 10)) FUN_0043e820((int)e, 1);
                    else                                       FUN_0043e820((int)e, 0);
                    *(DWORD*)(e + 0xCC) = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                } else if (v11 == 267) {
                    if (v12 == 8 || v12 == 10) FUN_0043e820((int)e, 5);
                    else                       FUN_0043e820((int)e, 4);
                    *(DWORD*)(e + 0xCC) = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                } else {
                    FUN_0043e820((int)e, 3);
                    *(DWORD*)(e + 0xCC) = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                }
            }
        }
        else {
            int v5 = v4 - 175;
            if (v5 == 0) {
                // v4 == 175 (criatura ambiental)
                v36 = 100.0f;
                float Light[3] = { 0.40000001f, 0.60000002f, 1.0f };
                if ((rand() & 1) == 0) {
                    Particle_Spawn(1220, (float*)(e + 0x10), (float*)(e + 0x1c), Light, 1, 1.0f, 0);
                }
            }
            // else: v4 no es 175/195/267 → cae a LABEL_72 con v36=0
        }

        // ── LABEL_72 — avance BMD común + follow (solo 816/817) ─────────────
        {
        float* v28 = (float*)(e + 0x10);
        *(BYTE*)(v40 + 160) = *(BYTE*)(e + 0x105);          // model.CurrentAction
        FUN_00440aa0((void*)v40, (float*)(e + 0x108), (float*)(e + 0x10C),
                     (void*)(e + 0x106), *(float*)(e + 0xCC));   // sub_440AA0 (a6/a7 unused)

        short v29 = *(short*)(e + 2);
        if (v29 == 816 || v29 == 817) {
            float v30 = *(float*)(e + 0x14);                // posY
            float x1  = *v28;                                // posX
            float v31 = (y2 - v30) * (y2 - v30) + (x2 - x1) * (x2 - x1);
            float v38 = v36 * v36;
            if (v31 >= v38) {
                float v41 = FUN_0043e050(x1, v30, x2, y2);   // Movement_Tick (CreateAngle)
                *(float*)(e + 0x24) = FUN_0043e1b0(*(float*)(e + 0x24), v41, 20.0f);
            }
            // PORT FIEL 1:1 (2026-07-18): el ASM (0x5007DB) escribe la matriz de
            // AngleMatrix en `[esi+0x90]` (campo scratch), NO en 0x24. Hex-Rays lo
            // decompiló como `(float*)v1+9`=0x24 pero es un artefacto: el disasm real
            // es `lea ebx,[esi+90h]`. angleZ (0x24) queda intacto → TurnAngle acumula
            // el giro y el hada ORBITA al char (movimiento tangencial cuando está lejos).
            Matrix_BuildFromEuler((float*)(e + 0x1c), (float*)(e + 0x90));   // AngleMatrix → scratch 0x90
            float out[3];
            Vector_Rotate((float*)(e + 0xC0), (float*)(e + 0x90), out);   // VectorRotate(vel@0xC0, mat@0x90)
            *v28 = out[0] + *v28;
            *(float*)(e + 0x14) = out[1] + *(float*)(e + 0x14);
            *(float*)(e + 0x18) = out[2] + *(float*)(e + 0x18);
            *(float*)(e + 0x18) = (float)(rand() % 16 - 8) + *(float*)(e + 0x18);
            if ((rand() & 0x1F) == 0) {
                if (v31 < v38) {
                    *(float*)(e + 0xC4) = (float)(rand() % 64 + 16) * -0.1f;
                    *(float*)(e + 0x24) = (float)(rand() % 360);
                } else {
                    *(float*)(e + 0xC4) = (float)(rand() % 64 + 128) * -0.1f;
                }
                *(DWORD*)(e + 0xC0) = 0;
                *(float*)(e + 0xC8) = (float)(rand() % 64 - 32) * 0.1f;
            }
            int v35 = *v0;
            if (*(float*)(*v0 + 24) + 100.0f > *(float*)(e + 0x18))
                *(float*)(e + 0xC8) = *(float*)(e + 0xC8) + 1.5f;
            if (*(float*)(v35 + 24) + 200.0f < *(float*)(e + 0x18))
                *(float*)(e + 0xC8) = *(float*)(e + 0xC8) - 1.5f;
        }
        }
        }

next_bug:
        v0 += 111;                              // +0x1BC bytes
    } while ((int)(uintptr_t)v0 < (int)(uintptr_t)endp);
}
