#include <cstdint>
// Scene_ObjectUpdate.cpp
//
// Update/render por frame de los objetos de la escena y de los bugs ambientales.

#include "stdafx.h"
#include "Scene/ObjectRegistry.h"
#include "Platform/GuardedOperation.h"
#include "Platform/ITimer.h"
#include <array>
#include "Item/ContentCatalog.h"
#include "globals.h"
#include "functions.h"

extern void __cdecl Effect_PhysicsTick(std::uintptr_t Object);
// MoveObject_PerWorld @ 0x004FDC00 (~608 lines) — SUMMARY STUB, sin llamadores:
// la copia viva es el port de FUN_004fdc00 más abajo en este archivo.
// Per-world object animation. Per-frame for each visible scene object.
// World 9: random terrain lights. World 0: toggle objects by HeroTile.
// Then: Alpha(), BMD setup, animate, render via RenderPartObject.
float* __cdecl MoveObject_PerWorld(OBJECT* object) {
    // 0x004FDC00 — Per-world object animation (608 lines decompiled).
    // IDA 0x004FDD00 usa el argumento como dirección de OBJECT, sin conversión numérica.
    // Per-frame update for visible scene objects: toggle visibility by HeroTile,
    // call Alpha(), set up BMD animation, then per-world switch for special effects.
    //
    // Ghidra uses unaff_EBX/EBP/ESI/EDI phantom register params throughout;
    // these carry context from the MoveObjects caller loop. We implement only
    // the real logic paths that don't depend on phantom registers.

    const std::uintptr_t objPtr = reinterpret_cast<std::uintptr_t>(object);
    if (objPtr == 0) return (float*)0;
    extern void __cdecl AddTerrainLight(float x, float y, float* light, int range, float* buffer);

    short objType = AsObject(objPtr)->Type;

    // ── World 9: random terrain lights ──
    if (gMapManager.GetCurrentMap() == 9) {
        // IDA 0x004FDC00: the storm flash is independent from the object type.
        // It happens during the first quarter of the 4-second cycle, one time
        // in 100, around the local player.  The former port kept only sound.
        if (((std::int64_t)WorldTime % 4000) < 1000 && !(rand() % 100) && Hero) {
            const float intensity = (float)(rand() % 12 + 4) * 0.1f;
            float light[3] = { intensity * 0.2f, intensity * 0.3f, intensity * 0.5f };
            const float x = (float)(rand() % 1200) + *(float*)(Hero + 16) - 600.0f;
            const float y = (float)(rand() % 1200) + *(float*)(Hero + 20) - 600.0f;
            AddTerrainLight(x, y, (float*)light, 12, (float*)PrimaryTerrainLight[0]);
        }
        PlayBuffer(1, 0, 1);
    }

    // ── World 0: toggle torch/fire objects by HeroTile ──
    if (gMapManager.GetCurrentMap() == 0) {
        if (objType == 0x7d || objType == 0x7e) {
            if (HeroTile == 4) {
                *(DWORD *)&AsObject(objPtr)->AlphaTarget = 0;        // hide (alpha=0)
            } else {
                *(DWORD *)&AsObject(objPtr)->AlphaTarget = 0x3f800000; // show (alpha=1.0f)
            }
        }
    }

    // ── World 2: toggle water objects by HeroTile ──
    if (gMapManager.GetCurrentMap() == 2) {
        short s = AsObject(objPtr)->Type;
        if (s == 0x51 || s == 0x52 || s == 0x60 || s == 0x62 || s == 99) {
            if (HeroTile == 3 || HeroTile > 9) {
                *(DWORD *)&AsObject(objPtr)->AlphaTarget = 0;
            } else {
                *(DWORD *)&AsObject(objPtr)->AlphaTarget = 0x3f800000;
            }
        }
    }

    // Alpha fade — Alpha(entity_ptr)
    Alpha(AsObject(objPtr));

    // Check alpha > 0
    float alpha = AsObject(objPtr)->Alpha;
    if (alpha < _DAT_005524f8) {
        return (float*)0;  // invisible, skip
    }

    // Setup BMD model: Models base = DAT_05828d58, stride = sizeof(BMD) = 188 (0xBC)
    int modelIdx = (int)AsObject(objPtr)->Type;
    char* model = (char*)(DAT_05828d58 + modelIdx * 0xBC);
    *(char *)&AsModel(model)->CurrentAction = *(char *)&AsObject(objPtr)->CurrentAction;  // set current action

    float animSpeed = AsObject(objPtr)->Velocity;
    if (gMapManager.GetCurrentMap() == 8 && AsObject(objPtr)->Type == 8) {
        animSpeed = animSpeed * _DAT_00552650;  // slow down lava objects
    }

    // Play animation — BMD__PlayAnimation(model, frame*, scale*, extra, speed)
    // Ghidra sig: BMD::PlayAnimation(this, frame*, priorFrame*, priorAction*, speed, pos*, angle*)
    // Our declaration has 5 params; pass what fits
    BMD__PlayAnimation((void*)model, (float*)(objPtr + 0x108), (float*)(objPtr + 0x10c),
                 (void*)(objPtr + 0x106), animSpeed);

    // ── Escena de login / char-select (IDA sub_4FDC00, bloque previo al switch)
    // En el orden del binario: después de PlayAnimation, antes del switch.
    //   160 = Logo01 (cielo) y 161 = Logo02 (olas): scroll de la V de textura.
    //   162 = Logo03 (banner MU): rampa de Light + alpha-scalar. Sin esto el
    //         banner queda con bodyLight=(0,0,0) → rectángulo negro.
    if (SceneFlag == 2 || SceneFlag == 4) {
        short t = AsObject(objPtr)->Type;
        if (t == 160 || t == 161) {
            AsObject(objPtr)->BlendMeshTexCoordV = -((float)((std::int64_t)WorldTime % 4000) * 0.00025f);
        } else if (t == 162) {
            // CameraWalkCut → over-bright final; si no, rampa 0..0.08.
            float v8 = (DAT_083a7af4 != 0) ? 1.5f
                                           : (float)(int)DAT_005615e8 * 0.002f;
            AsObject(objPtr)->Light[0] = v8;   // Light[0]
            AsObject(objPtr)->Light[1] = v8;   // Light[1]
            AsObject(objPtr)->Light[2] = v8;   // Light[2]
            AsObject(objPtr)->BlendMeshLight = v8;   // alpha-scalar (+0x68)
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
    //          types 0x26,0x27 → Effect_PhysicsTick
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
    switch (gMapManager.GetCurrentMap()) {
    case 0:
        switch (objType) {
        case 0x32: FUN_0046c7f0(0, AsObject(objPtr), 0.0f, 0.0f, 200.0f); return (float*)0;       // 0x43480000
        case 0x33: FUN_0046c7f0(0, AsObject(objPtr), 0.0f, -30.0f, 60.0f); return (float*)0;      // 0xc1f00000, 0x42700000
        case 0x34:
            FUN_0046c7f0(0, AsObject(objPtr), 0.0f, 0.0f, 60.0f);
            AsObject(objPtr)->BlendMeshLight = (float)(rand() % 6 + 4) * _DAT_005524f4;
            return (float*)0;
        case 0x37:
            FUN_0046c7f0(0, AsObject(objPtr), -150.0f, -150.0f, 140.0f);   // 0xc3160000, 0x430c0000
            FUN_0046c7f0(0, AsObject(objPtr), 150.0f, -150.0f, 140.0f);
            return (float*)0;
        case 0x50:
            FUN_0046c7f0(0, AsObject(objPtr), 90.0f, -200.0f, 30.0f);     // 0xc3480000, 0x41f00000
            FUN_0046c7f0(0, AsObject(objPtr), 90.0f, 200.0f, 30.0f);       // 0x43480000, 0x41f00000
            return (float*)0;
        case 0x5A: {
            float light = (float)(rand() % 2 + 6) * 0.1f;
            float terrainLight[3];
            terrainLight[0] = light;
            terrainLight[1] = light * 0.8f;
            terrainLight[2] = light * 0.6f;
            AddTerrainLight(AsObject(objPtr)->Position[0], AsObject(objPtr)->Position[1], (float*)terrainLight, 3, (float*)PrimaryTerrainLight[0]);
            return (float*)0;
        }
        case 0x75:
        case 0x7A:
            AsObject(objPtr)->BlendMeshLight = (float)(rand() % 4 + 4) * 0.1f;
            break;
        case 0x76:
        case 0x77:
            AsObject(objPtr)->BlendMeshTexCoordV = (float)(-((std::int64_t)WorldTime % 1000)) * 0.001f;
            break;
        case 0x82:
        case 0x83:
        case 0x84: {
            // Light01/02/03: marcadores de humo. `HiddenMesh = -2` oculta la caja
            // (8 vértices, textura dummy `ston03` 2x2 negra) y sub_46C7F0 emite
            // el fuego/humo. IDA sub_4FDC00, World 0.
            int kind = objType - 0x82;
            FUN_0046c7f0(kind, AsObject(objPtr), 0.0f, 0.0f, 0.0f);
            AsObject(objPtr)->HiddenMesh = -2;
            return (float*)0;
        }
        case 0x96: {
            float light = (float)(rand() % 4 + 3) * 0.1f;
            float terrainLight[3];
            terrainLight[0] = light;
            terrainLight[1] = light * 0.6f;
            terrainLight[2] = light * 0.2f;
            AddTerrainLight(AsObject(objPtr)->Position[0], AsObject(objPtr)->Position[1], (float*)terrainLight, 3, (float*)PrimaryTerrainLight[0]);
            return (float*)0;
        }
        }
        break;

    case 1:
        switch (objType) {
        case 0x16:
        case 0x17:
        case 0x18:
            // IDA writes the model loop flag (+136) and scrolls texture V.
            AsModel(model)->StreamMesh = 1;
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 1000) * 0.001f;
            break;
        case 0x27:
        case 0x28:
        case 0x33:
            AsObject(objPtr)->HiddenMesh = -2;
            break;
        case 0x29: FUN_0046c7f0(0, AsObject(objPtr), 0.0f, -30.0f, 240.0f); return (float*)0;     // 0xc1f00000, 0x43700000
        case 0x2a: FUN_0046c7f0(0, AsObject(objPtr), 0.0f, 0.0f, 190.0f); return (float*)0;       // 0x433e0000
        case 0x34:
            if (!(rand() % 3)) {
                CreateEffect(215, (float*)(objPtr + 16), (float*)(objPtr + 28),
                              (float*)(objPtr + 232), 0, 0,
                              (float*)(uintptr_t)0xffffffffu, 0, 0);
                AsObject(objPtr)->HiddenMesh = -2;
            }
            break;
        }
        break;

    case 2:
        switch (objType) {
        case 20:
        case 65:
        case 86:
        case 88:
            // IDA's moving gate/bridge objects chase their target coordinate
            // until they are close enough, then select the correct turn arc.
            // This is visual state only; it must not synthesize a gate packet.
            if (!DAT_07e11d30 && Hero) {
                const float dx = *(float*)(Hero + 16) - AsObject(objPtr)->HeadTargetAngle[0];
                const float dy = *(float*)(Hero + 20) - AsObject(objPtr)->HeadTargetAngle[1];
                const float distance = sqrtf(dx * dx + dy * dy);
                if (distance >= 200.0f) {
                    AsObject(objPtr)->Angle[2] = TurnAngle2(AsObject(objPtr)->Angle[2], AsObject(objPtr)->HeadAngle[2], 10.0f);
                    AsObject(objPtr)->Position[0] += (AsObject(objPtr)->HeadTargetAngle[0] - AsObject(objPtr)->Position[0]) * 0.2f;
                    AsObject(objPtr)->Position[1] += (AsObject(objPtr)->HeadTargetAngle[1] - AsObject(objPtr)->Position[1]) * 0.2f;
                } else if (objType == 86) {
                    const float heading = AsObject(objPtr)->Angle[2];
                    if (heading == 90.0f)  AsObject(objPtr)->Position[1] = AsObject(objPtr)->HeadTargetAngle[1] + 2.0f * (200.0f - distance);
                    if (heading == 270.0f) AsObject(objPtr)->Position[1] = AsObject(objPtr)->HeadTargetAngle[1] - 2.0f * (200.0f - distance);
                    if (heading == 0.0f)   AsObject(objPtr)->Position[0] = AsObject(objPtr)->HeadTargetAngle[0] + 2.0f * (200.0f - distance);
                    if (heading == 180.0f) AsObject(objPtr)->Position[0] = AsObject(objPtr)->HeadTargetAngle[0] - 2.0f * (200.0f - distance);
                    PlayBuffer(18, 0, 0);
                } else {
                    const float targetHeading = AsObject(objPtr)->HeadAngle[2];
                    if (targetHeading == 90.0f)  AsObject(objPtr)->Angle[2] = 30.0f - (200.0f - distance) * 0.5f;
                    if (targetHeading == 270.0f) AsObject(objPtr)->Angle[2] = 330.0f + (200.0f - distance) * 0.5f;
                    if (targetHeading == 0.0f)   AsObject(objPtr)->Angle[2] = 300.0f - (200.0f - distance) * 0.5f;
                    if (targetHeading == 180.0f) AsObject(objPtr)->Angle[2] = 240.0f + (200.0f - distance) * 0.5f;
                    PlayBuffer(17, 0, 0);
                }
            }
            break;
        case 0x1e:
        case 0x42: FUN_0046c7f0(0, AsObject(objPtr), 0.0f, 0.0f, 50.0f); return (float*)0;        // 0x42480000
        }
        break;

    case 3:
        switch (objType) {
        case 0x12:
            AsObject(objPtr)->BlendMeshTexCoordV = (float)((std::int64_t)WorldTime % 1000) * 0.001f;
            break;
        case 0x27:
            AsObject(objPtr)->BlendMesh = 1;
            break;
        case 0x29:
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshTexCoordV = (float)((std::int64_t)WorldTime % 2000) * 0.0005f;
            break;
        case 0x2a:
            AsModel(model)->StreamMesh = 0;
            AsObject(objPtr)->BlendMeshTexCoordU = (float)((std::int64_t)WorldTime % 500) * -0.002f;
            break;
        case 0x2b:
            AsModel(model)->StreamMesh = 0;
            AsObject(objPtr)->BlendMeshTexCoordU = (float)((std::int64_t)WorldTime % 500) * 0.002f;
            break;
        }
        break;

    case 4:
        switch (objType) {
        case 3:
        case 4:
            AsObject(objPtr)->BlendMeshTexCoordU = -(float)((std::int64_t)WorldTime % 1000) * 0.001f;
            break;
        case 0x12:
        case 0x17:
            AsObject(objPtr)->BlendMesh = 1;
            break;
        case 0x13:
        case 0x14:
            AsObject(objPtr)->BlendMesh = 4;
            AsObject(objPtr)->BlendMeshTexCoordU = -(float)((std::int64_t)WorldTime % 1000) * 0.001f;
            break;
        case 0x18:
            AsObject(objPtr)->HiddenMesh = -2;
            if (!(rand() % 64))
                CreateEffect(1200, (float*)(objPtr + 16), (float*)(objPtr + 28),
                              (float*)(objPtr + 232), 0, 0,
                              (float*)(uintptr_t)0xffffffffu, 0, 0);
            break;
        case 0x19:
            AsObject(objPtr)->HiddenMesh = -2;
            break;
        case 0x26:
        case 0x27: Effect_PhysicsTick(objPtr); return (float*)0;
        }
        break;

    case 5:
        if (objType == 2) {
            AsObject(objPtr)->BlendMesh = 0;
        } else if (objType == 3) {
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshLight = (float)(rand() % 4 + 6) * 0.1f;
        }
        break;

    case 6:
        if (objType == 0x15) {
            AsObject(objPtr)->BlendMesh = 3;
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 1000) * 0.001f;
        } else if (objType == 0x26) {
            AsObject(objPtr)->HiddenMesh = -2;  // SubType = -2
        }
        break;

    case 7:
        switch (objType) {
        case 0x16:
            AsObject(objPtr)->Timer += 0.1f;
            AsObject(objPtr)->HiddenMesh = -2;
            if (AsObject(objPtr)->Timer > 10.0f) AsObject(objPtr)->Timer = 0.0f;
            if (AsObject(objPtr)->Timer > 5.0f)
                Particle_Spawn(1241, (float*)(objPtr + 16), (float*)(objPtr + 28),
                               (float*)(objPtr + 232), 0, 1.0f, 0);
            break;
        case 0x17:
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshLight = sinf((float)WorldTime * 0.002f) * 0.3f + 0.5f;
            break;
        case 0x20:
        case 0x22:
            AsObject(objPtr)->BlendMesh = 1;
            AsObject(objPtr)->BlendMeshLight = (sinf((float)WorldTime * 0.004f) + 1.0f) * 0.5f;
            break;
        case 0x26:
            AsObject(objPtr)->BlendMesh = 0;
            break;
        case 0x28:
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->Velocity = 0.03f;
            AsObject(objPtr)->BlendMeshLight = sinf((float)WorldTime * 0.004f) * 0.3f + 0.5f;
            break;
        }
        break;

    case 8:
        if (objType == 2) {
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshTexCoordU = -(float)((std::int64_t)WorldTime % 1000) * 0.001f;
            return (float*)0;
        }
        if (objType == 4) {
            const float wave = sinf((float)WorldTime * 0.002f) * 0.35f + 0.65f;
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshLight = wave;
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 10000) * 0.0001f;
        } else if (objType == 7) {
            const float wave = sinf((AsObject(objPtr)->Angle[2] * 100.0f + (float)WorldTime) * 0.002f) * 0.35f + 0.65f;
            float light[3] = { wave, wave * 0.6f, wave * 0.2f };
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshLight = wave;
            AddTerrainLight(AsObject(objPtr)->Position[0], AsObject(objPtr)->Position[1], (float*)light, 3, (float*)PrimaryTerrainLight[0]);
            return (float*)0;
        } else if (objType == 0x0b) {
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 10000) * 0.0002f;
            return (float*)0;
        } else if (objType == 0x0c) {
            const float scroll = -(float)((std::int64_t)WorldTime % 50000) * 0.00005f;
            AsObject(objPtr)->BlendMeshTexCoordU = scroll;
            AsObject(objPtr)->BlendMeshTexCoordV = scroll;
            return (float*)0;
        } else if (objType == 0x0d) {
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 10000) * 0.0002f;
            return (float*)0;
        } else if (objType == 0x3d || objType == 0x41 || objType == 0x42) {
            const float scroll = -(float)((std::int64_t)WorldTime % 1000) * 0.001f;
            const float wave = sinf((float)WorldTime * 0.002f) * 0.35f + 0.65f;
            float light[3] = { wave, wave * 0.6f, wave * 0.2f };
            AsObject(objPtr)->BlendMesh = 1;
            AsObject(objPtr)->BlendMeshTexCoordV = scroll;
            AddTerrainLight(AsObject(objPtr)->Position[0], AsObject(objPtr)->Position[1], (float*)light, 2, (float*)PrimaryTerrainLight[0]);
        } else if (objType == 0x3f || objType == 0x40) {
            AsObject(objPtr)->HiddenMesh = -2;
        } else if (objType == 0x48) {
            AsObject(objPtr)->BlendMesh = 0;
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 10000) * 0.0002f;
        } else if (objType == 0x49 || objType == 0x4b || objType == 0x4f) {
            AsObject(objPtr)->BlendMeshTexCoordV = -(float)((std::int64_t)WorldTime % 10000) * 0.0002f;
        } else if (objType == 0x52) {
            *(DWORD *)&AsObject(objPtr)->BlendMesh = 0;
            *(DWORD *)&AsObject(objPtr)->Light[0] = 0x3f800000;  // light R = 1.0
            *(DWORD *)&AsObject(objPtr)->Light[1] = 0x3f800000;  // light G = 1.0
            *(DWORD *)&AsObject(objPtr)->Light[2] = 0x3f800000;  // light B = 1.0
        }
        break;
    }

    // For worlds 0xb..0x10: check object type 9/10 visibility
    if (gMapManager.GetCurrentMap() >= 0xb && gMapManager.GetCurrentMap() <= 0x10) {
        int t = (int)AsObject(objPtr)->Type;
        if (t >= 9 && t <= 10) {
            if (AsObject(objPtr)->PKKey == 4) {
                return (float*)0;
            }
            AsObject(objPtr)->HiddenMesh = -2;  // SubType = -2
        }
    }

    return (float*)0;
}

// MoveHeavenThunder @ 0x004FED90 (~472 lines) — SUMMARY STUB
// Lightning storm for World 10 (Icarus). Random bolts via CreateEffect(0xb6).
// Adds terrain light flash, returns nonzero when strike occurs.
int __stdcall MoveHeavenThunder(void) {
    // Port fiel de IDA MoveHeavenThunder (0x004FED90).
    //
    // `CreateEffect(182, ...)`: el tipo 182 es el modelo `cloud`
    // (`OpenWorldModels` case 10 hace `AccessModelWithTextures(182, "Data\Object11", "cloud", -1)`
    // + `OpenJPG("Effect\clouds.jpg", 1268)`), y `RenderEffects` lo dibuja por su
    // `case 182:`. O sea ESTE es el generador de las nubes de Icarus.
    //
    // Devuelve `objectCount` — un indice de objeto al azar que MoveObjects usa
    // para elegir a cual colgarle el rayo.
    int objectCount = 0;

    if (rand() % 50) return 0;

    float Position[3], Light[3], Angle[3];

    // Flash de luz sobre el terreno.
    Position[0] = (float)(rand() % 300) + *(float *)(Hero + 16) - 150.0f;
    Position[1] = *(float *)(Hero + 20);
    Position[2] = 0.0f;
    rand();                                    // IDA descarta este rand()
    {
        const float lum = (float)(rand() % 4 + 4) * 0.050000001f;
        Light[0] = lum * 0.30000001f;
        Light[1] = Light[0];
        Light[2] = lum * 0.081f;
    }
    AddTerrainLight(Position[0], Position[1], (float*)Light, 2, (float*)PrimaryTerrainLight[0]);

    // La NUBE: efecto 182 en la posicion del heroe (no en Position).
    memset(Angle, 0, sizeof(Angle));
    CreateEffect(182, (float *)(Hero + 16), Angle, Light,
                  (float *)0, (float *)0, (float *)0xffffffff, (float *)0, 0);

    if (DAT_083a3fec) objectCount = rand() % (int)DAT_083a3fec;

    // Segunda fase (20%): rayo bifurcado.
    if (rand() % 5) return objectCount;

    float angle[3] = { 0.0f, 0.0f, -45.0f };
    float Matrix1[12], Matrix2[12];
    float position[3], pos[3];
    Matrix_BuildFromEuler(angle, Matrix1);

    switch (rand() % 4) {
    case 0:
        position[0] = -400.0f; position[1] = -1000.0f; position[2] = 0.0f;
        Vector_Rotate(position, Matrix1, position);
        pos[0] = position[0] + *(float *)(Hero + 16);
        pos[1] = position[1] + *(float *)(Hero + 20);
        pos[2] = position[2] + *(float *)(Hero + 24);
        angle[0] = 0.0f; angle[1] = 0.0f; angle[2] = 240.0f;
        Matrix_BuildFromEuler(angle, Matrix2);
        position[0] = -200.0f; position[1] = -1000.0f;
        break;
    case 1:
        position[0] = -300.0f; position[1] = -400.0f; position[2] = 0.0f;
        Vector_Rotate(position, Matrix1, position);
        // OJO: este caso RESTA (los otros tres suman). Es asi en IDA.
        pos[0] = *(float *)(Hero + 16) - position[0];
        pos[1] = *(float *)(Hero + 20) - position[1];
        pos[2] = *(float *)(Hero + 24) - position[2];
        angle[0] = 0.0f; angle[1] = 0.0f; angle[2] = 210.0f;
        Matrix_BuildFromEuler(angle, Matrix2);
        position[0] = -500.0f; position[1] = -1000.0f;
        break;
    case 2:
        position[0] = -200.0f; position[1] = -400.0f; position[2] = 0.0f;
        Vector_Rotate(position, Matrix1, position);
        pos[0] = position[0] + *(float *)(Hero + 16);
        pos[1] = position[1] + *(float *)(Hero + 20);
        pos[2] = position[2] + *(float *)(Hero + 24);
        angle[0] = 0.0f; angle[1] = 0.0f; angle[2] = 235.0f;
        Matrix_BuildFromEuler(angle, Matrix2);
        position[0] = -1000.0f; position[1] = -1500.0f;
        break;
    default:
        position[0] = -200.0f; position[1] = 400.0f; position[2] = 0.0f;
        Vector_Rotate(position, Matrix1, position);
        pos[0] = position[0] + *(float *)(Hero + 16);
        pos[1] = position[1] + *(float *)(Hero + 20);
        pos[2] = position[2] + *(float *)(Hero + 24);
        angle[0] = 0.0f; angle[1] = 0.0f; angle[2] = 200.0f;
        Matrix_BuildFromEuler(angle, Matrix2);
        position[0] = -600.0f; position[1] = -1200.0f;
        break;
    }
    position[2] = 0.0f;

    Vector_Rotate(position, Matrix2, position);
    angle[2] = 0.0f;
    Position[0] = pos[0] - position[0];
    Position[1] = pos[1] - position[1];
    Position[2] = pos[2] - position[2] - 300.0f;
    position[0] = pos[0] + position[0];
    position[1] = pos[1] + position[1];
    position[2] = pos[2] + position[2] - 300.0f;

    for (int n = 0; n < 2; ++n) {
        const float scale = (float)(rand() % 10) + 40.0f;
        Joint_Create(1255, position, Position, angle, 9, 0, scale, -1, 0);
    }
    return objectCount;
}

// MoveObjects @ 0x004FF260 (~169 lines) — per-frame object update dispatcher
// World 10: MoveHeavenThunder. World 11..16: ambient particles.
// Iterates all object lists calling MoveObject_Special or FUN_004fdc00 (el tick por objeto).
void __stdcall MoveObjects(void) {
    // 0x004FF260 — Per-frame object update dispatcher.
    // World 10: calls MoveHeavenThunder. World 11..16: spawn ambient particles.
    // Then iterates all object bucket lists (16 buckets per block, from DAT_083a021c)
    // calling MoveObject_Special (Object_AnimUpdate) or FUN_004fdc00 (Object_RenderUpdate).
    // In World 10 with thunder active, spawns lightning joints on random objects.

    float Scale = 0.0f;
    if (gMapManager.GetCurrentMap() == 10) {
        Scale = (float)MoveHeavenThunder();
    }
    else if (gMapManager.GetCurrentMap() > 10 && gMapManager.GetCurrentMap() < 0x11) {
        // Worlds 11..16: spawn ambient particle near hero
        // IDA 004FF260: Angle is cleared and Light is full white.
        float light[3] = { 1.0f, 1.0f, 1.0f };
        float angle[3] = { 0.0f, 0.0f, 0.0f };
        float pos[3];
        pos[0] = (float)(rand() % 900 - 300) + *(float*)((char*)Hero + 0x10);
        pos[1] = (float)(rand() % 900 - 300) + *(float*)((char*)Hero + 0x14);
        pos[2] = (float)(rand() % 50) + *(float*)((char*)Hero + 0x18) + _DAT_00552994;
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

    // IDA 0x004FF260: recorrer las 256 celdas en bloques de 16.
    SceneObjectCell* puVar4 = g_ObjectBucketGrid;
    do {
        int bucketsLeft = 0x10;
        do {
            char* pcVar6;
            char bucketFlag = static_cast<char>(puVar4->Visible);

            // Cada enlace debe identificar un nodo aun vivo de CreateObject.
            int bucketIter = 0;
            const int kBucketIterMax = 4096;

            Platform::RunValidatedOperation([&]() {
            if (bucketFlag == '\0') {
                // Static objects: just animate, don't render-update
                pcVar6 = static_cast<char*>(puVar4->Head);
                while (pcVar6 != NULL && SceneObjects::Contains(pcVar6) &&
                       ++bucketIter < kBucketIterMax) {
                    if (*pcVar6 != '\0') {
                        MoveObject_Special(AsObject(pcVar6));
                    }
                    pcVar6[0x160] = '\0';
                    pcVar6 = *(char**)(pcVar6 + 0x1B8);
                }
            }
            else {
                // Dynamic objects: render-update + animate
                pcVar6 = static_cast<char*>(puVar4->Head);
                while (pcVar6 != NULL && SceneObjects::Contains(pcVar6) &&
                       ++bucketIter < kBucketIterMax) {
                    if (*pcVar6 != '\0' && pcVar6[0x160] != '\0') {
                        // IDA 0x004FF3B7: push edi pasa la dirección del objeto, no un float.
                        FUN_004fdc00(AsObject(pcVar6));
                        DAT_083a3fec++;

                        // Este bloque vive en **MoveObjects (0x004FF260)**, la funcion que contiene
                        // este mismo loop (no en `sub_4FDC00`, el tick por objeto), literal:
                        //
                        //   if ( World == 10 && objCount ) {
                        //     if ( rand() % 10 || (v5 = *(_WORD *)(v4 + 2), v5 < 0) || v5 > 5 )
                        //       --unk_83A3FEC;
                        //     else {
                        //       Light[i] = (double)(rand() % 10) * 0.02;
                        //       CreateSprite(1269, (float *)(v4 + 16), 0.5, Light, Hero, 0.0, 0);
                        //       Scale  = (double)(rand() % 20) + 10.0;
                        //       CreateJoint(1254, v4+16, v4+16, v4+28, 6, v4, Scale,  -1, 0);
                        //       Scalea = (double)(rand() % 20) + 10.0;
                        //       CreateJoint(1254, v4+16, v4+16, v4+28, 6, v4, Scalea, -1, 0);
                        //     }
                        //   }
                        //
                        // Son los rayos de tormenta que caen sobre los objetos del mapa en Icarus.
                        if (gMapManager.GetCurrentMap() == 10 && Scale != 0.0f) {
                            int r2 = rand();
                            if (r2 % 10 == 0 &&
                                *(short*)(pcVar6 + 2) >= 0 &&
                                *(short*)(pcVar6 + 2) < 6)
                            {
                                float light[3];
                                light[0] = (float)(rand() % 10) * 0.02f;
                                light[1] = (float)(rand() % 10) * 0.02f;
                                light[2] = (float)(rand() % 10) * 0.02f;
                                CreateSprite(1269, (float*)(pcVar6 + 16), 0.5f,
                                             light, (int)Hero, 0.0f, 0);
                                Scale = (float)(rand() % 20) + 10.0f;
                                Joint_Create(1254, (float*)(pcVar6 + 16),
                                             (float*)(pcVar6 + 16), (float*)(pcVar6 + 28),
                                             6, (int)pcVar6, Scale, -1, 0);
                                Scale = (float)(rand() % 20) + 10.0f;
                                Joint_Create(1254, (float*)(pcVar6 + 16),
                                             (float*)(pcVar6 + 16), (float*)(pcVar6 + 28),
                                             6, (int)pcVar6, Scale, -1, 0);
                            }
                            else {
                                DAT_083a3fec--;
                            }
                        }
                        MoveObject_Special(AsObject(pcVar6));
                    }
                    pcVar6 = *(char**)(pcVar6 + 0x1B8);
                } // end while linked list
            }
            }); // Windows conserva la proteccion SEH del recorrido.
            ++puVar4;
            bucketsLeft--;
        } while (bucketsLeft != 0);

        // El límite es el final real de la grilla, independiente de sizeof(void*).
        if (puVar4 >= g_ObjectBucketGrid + 256) {
            return;
        }
    } while (true);
}

// MoveBugs @ 0x005001F0 — PORT FIEL 1:1 desde IDA (sub_5001F0).
// Butterfly/mount/ambient update: chequea owner vivo, fade de
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
// CustomPet: movimiento del bug de un pet del catálogo.  `orbit` da vueltas
// alrededor del dueño (el "stand" del Collecter del 5.2: radio, período y
// altura); `follow` se queda detrás del dueño.  En los dos acerca la posición
// al objetivo y mira hacia donde va.
static void CustomPet_Move(char* e, const BYTE* owner, const Proto::CATALOG_PET& pet)
{
    if (!owner) return;
    float target[3];
    const float* ownerPos = (const float*)(owner + 0x10);

    // Collecter del 5.2: si hay zen cerca del dueño, va hasta el más cercano y
    // da vueltas cortas sobre él hasta que el server lo levanta (y el zen
    // desaparece del suelo).  El server decide; esto es sólo lo que se ve.
    const float* zen = nullptr;
    if ((pet.Flags & Proto::CATALOG_PET_FLAG_COLLECT) && DAT_07e12840) {
        const float range = (float)pet.CollectRange * 100.0f + 50.0f;
        float best = range * range;
        for (int i = 0; i < 1000; ++i) {
            const BYTE* ip = (const BYTE*)DAT_07e12840 + i * 0x204;
            if (ip[72] == 0 || *(const short*)(ip + 74) != 863) continue;   // zen
            const float* p = (const float*)(ip + 88);
            const float dx = p[0] - ownerPos[0], dy = p[1] - ownerPos[1];
            const float d = dx * dx + dy * dy;
            if (d < best) { best = d; zen = p; }
        }
    }

    if (zen) {
        const float t = (float)(Platform::GetTimer().Ticks() % 2000) / 2000.0f * 6.2831853f;
        target[0] = zen[0] + sinf(t) * 20.0f;
        target[1] = zen[1] + cosf(t) * 20.0f;
        target[2] = zen[2] + pet.Height;
    } else if (pet.Movement == Proto::CATALOG_PET_MOVE_ORBIT) {
        const float t = (float)(Platform::GetTimer().Ticks() % (DWORD)pet.Period) / pet.Period * 6.2831853f;
        target[0] = ownerPos[0] + sinf(t) * pet.Radius;
        target[1] = ownerPos[1] + cosf(t) * pet.Radius;
    } else {
        const float a = *(const float*)(owner + 0x24) * 0.017453292f;
        target[0] = ownerPos[0] + sinf(a) * pet.Radius;
        target[1] = ownerPos[1] - cosf(a) * pet.Radius;
    }
    if (!zen) target[2] = ownerPos[2] + pet.Height;

    float* pos = (float*)(e + 0x10);
    const float dx = target[0] - pos[0], dy = target[1] - pos[1];
    if (dx * dx + dy * dy > 900.0f * 900.0f) {
        pos[0] = target[0]; pos[1] = target[1];   // teleport o muy lejos
    } else {
        pos[0] += dx * 0.2f;
        pos[1] += dy * 0.2f;
    }
    pos[2] += (target[2] - pos[2]) * 0.2f;
    if (dx * dx + dy * dy > 4.0f)
        *(float*)(e + 0x24) = TurnAngle2(*(float*)(e + 0x24), CreateAngle(pos[0], pos[1], target[0], target[1]), 20.0f);

    *(float*)(e + 0x0C) = pet.Scale;                // Scale
    *(float*)(e + 0x168) = 1.0f;                    // Alpha
    if (pet.BlendMesh != 0xFF) {
        *(int*)(e + 100) = pet.BlendMesh;           // BlendMesh
        *(float*)(e + 104) = 1.0f;                  // BlendMeshLight
    }
    SetAction(AsObject(e), pet.Action);
    if (*(float*)(e + 0xCC) <= 0.0f) *(float*)(e + 0xCC) = 0.25f;
}

void __stdcall MoveBugs(void) {
    extern unsigned char* TerrainWall;

    char*  base = (char*)DAT_083a1218;
    DWORD* v0   = (DWORD*)(base + 0xFC);      // = &unk_83A1314 (owner field de entry 0)
    for (int slot = 0; slot < 10; ++slot) {
        char* e = (char*)v0 - 0xFC;           // v1 — entry base
        if (*(BYTE*)e == 0)                    // !Live
            goto next_bug;

    { // El salto de salida no entra en el ámbito de estas variables.
        DWORD owner = *v0;                     // *v0 — Owner ptr
        if (SceneFlag == 5) {               // SceneFlag == InGame
            if (owner == 0 || *(BYTE*)owner == 0 || *(BYTE*)(owner + 132) != 1) {
                *(BYTE*)e = 0;                 // kill: owner muerto / Kind != 1
                goto next_bug;
            }
        }
        Alpha(AsObject(e));              // Actualiza el alpha sin truncar la dirección.

        {
        int   v2 = (int)owner;
        float x2 = *(float*)(v2 + 16);         // owner posX
        float y2 = *(float*)(v2 + 20);         // owner posY
        int   v4 = AsObject(e)->Type;           // type
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
                    v45[0] = v45[0] + AsObject(e)->Position[0];
                    v45[1] = v45[1] + AsObject(e)->Position[1];
                    v45[2] = (float)(v27 - 8) + AsObject(e)->Position[2];
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
                *(DWORD *)&AsObject(e)->Alpha = 0;      // en muro: apaga alpha, no copia pos
            } else {
                int  v8 = *v0;
                char v9 = *(BYTE*)(v8 + 124);
                if (v9 == 1 || v9 == 2) {
                    float v10 = AsObject(e)->Alpha - 0.1f;
                    AsObject(e)->Alpha = v10;
                    if (v10 < 0.0f) *(DWORD *)&AsObject(e)->Alpha = 0;
                } else {
                    *(DWORD *)&AsObject(e)->Alpha = 0x3F800000;   // 1.0f
                }
                *(DWORD *)&AsObject(e)->Position[0] = *(DWORD*)(v8 + 16);   // posX
                short v11 = AsObject(e)->Type;               // type
                *(DWORD *)&AsObject(e)->Position[1] = *(DWORD*)(v8 + 20);   // posY
                int   v12 = gMapManager.GetCurrentMap();
                float v37 = *(float*)(v8 + 24);
                AsObject(e)->Position[2] = v37;                  // posZ
                if (v11 == 267) {
                    if (v12 == 8 || v12 == 10)   AsObject(e)->Position[2] = v37 - 10.0f;
                    else if (v12 != -1)          AsObject(e)->Position[2] = v37 - 30.0f;
                }
                *(DWORD *)&AsObject(e)->Angle[0] = *(DWORD*)(v8 + 28);   // angleX
                *(DWORD *)&AsObject(e)->Angle[1] = *(DWORD*)(v8 + 32);   // angleY
                *(DWORD *)&AsObject(e)->Angle[2] = *(DWORD*)(v8 + 36);   // angleZ
                BYTE v14 = *(BYTE*)(v8 + 261);              // owner CurrentAction
                if ((v14 >= 13 && v14 <= 33) || v14 == 76 || v14 == 77) {
                    if (v11 == 267 && (v12 == 8 || v12 == 10)) SetAction(AsObject(e), 3);
                    else                                       SetAction(AsObject(e), 2);
                    if ((rand() & 1) == 0 && gMapManager.GetCurrentMap() != 10) {
                        float Light[3] = { 1.0f, 1.0f, 1.0f };
                        float Position[3];
                        Position[0] = (float)(rand() % 64 - 32) + AsObject(e)->Position[0];
                        Position[1] = (float)(rand() % 64 - 32) + AsObject(e)->Position[1];
                        Position[2] = (float)(rand() % 32 - 16) + AsObject(e)->Position[2];
                        if (gMapManager.GetCurrentMap() == 2) Particle_Spawn(1220, Position, (float*)(e + 0x1c), Light, 0, 1.0f, 0);
                        else            Particle_Spawn(1221, Position, (float*)(e + 0x1c), Light, 0, 1.0f, 0);
                    }
                    *(DWORD *)&AsObject(e)->Velocity = 0x3EAE147B;        // animSpeed = 0.34f
                    *(BYTE*)e = *(BYTE*)(*v0);               // Live = owner.Live
                } else if (v14 == 64 || v14 == 65) {
                    if (v12 == 8 || v12 == 10) SetAction(AsObject(e), 7);
                    else                       SetAction(AsObject(e), 6);
                    *(DWORD *)&AsObject(e)->Velocity = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                } else if (v14 < 0x22 || v14 > 0x37) {
                    if (v11 == 267 && (v12 == 8 || v12 == 10)) SetAction(AsObject(e), 1);
                    else                                       SetAction(AsObject(e), 0);
                    *(DWORD *)&AsObject(e)->Velocity = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                } else if (v11 == 267) {
                    if (v12 == 8 || v12 == 10) SetAction(AsObject(e), 5);
                    else                       SetAction(AsObject(e), 4);
                    *(DWORD *)&AsObject(e)->Velocity = 0x3EAE147B;
                    *(BYTE*)e = *(BYTE*)(*v0);
                } else {
                    SetAction(AsObject(e), 3);
                    *(DWORD *)&AsObject(e)->Velocity = 0x3EAE147B;
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
        *(BYTE*)(v40 + 160) = AsObject(e)->CurrentAction;          // model.CurrentAction
        BMD__PlayAnimation((void*)v40, (float*)(e + 0x108), (float*)(e + 0x10C),
                     (void*)(e + 0x106), AsObject(e)->Velocity);   // sub_440AA0 (a6/a7 unused)

        short v29 = AsObject(e)->Type;
        // 0.97.20: pet custom (Data/Custom/Pets del server).
        if (const Proto::CATALOG_PET* pet = gContentCatalog.GetPetByModel(v29)) {
            CustomPet_Move(e, (BYTE*)(uintptr_t)*v0, *pet);
        }
        if (v29 == 816 || v29 == 817) {
            float v30 = AsObject(e)->Position[1];                // posY
            float x1  = *v28;                                // posX
            float v31 = (y2 - v30) * (y2 - v30) + (x2 - x1) * (x2 - x1);
            float v38 = v36 * v36;
            if (v31 >= v38) {
                float v41 = CreateAngle(x1, v30, x2, y2);   // Movement_Tick (CreateAngle)
                AsObject(e)->Angle[2] = TurnAngle2(AsObject(e)->Angle[2], v41, 20.0f);
            }
            // PORT FIEL 1:1: el ASM (0x5007DB) escribe la matriz de
            // AngleMatrix en `[esi+0x90]` (campo scratch), NO en 0x24. Hex-Rays lo
            // decompiló como `(float*)v1+9`=0x24 pero es un artefacto: el disasm real
            // es `lea ebx,[esi+90h]`. angleZ (0x24) queda intacto → TurnAngle acumula
            // el giro y el hada ORBITA al char (movimiento tangencial cuando está lejos).
            Matrix_BuildFromEuler((float*)(e + 0x1c), (float*)(e + 0x90));   // AngleMatrix → scratch 0x90
            float out[3];
            Vector_Rotate((float*)(e + 0xC0), (float*)(e + 0x90), out);   // VectorRotate(vel@0xC0, mat@0x90)
            *v28 = out[0] + *v28;
            AsObject(e)->Position[1] = out[1] + AsObject(e)->Position[1];
            AsObject(e)->Position[2] = out[2] + AsObject(e)->Position[2];
            AsObject(e)->Position[2] = (float)(rand() % 16 - 8) + AsObject(e)->Position[2];
            if ((rand() & 0x1F) == 0) {
                if (v31 < v38) {
                    AsObject(e)->Direction[1] = (float)(rand() % 64 + 16) * -0.1f;
                    AsObject(e)->Angle[2] = (float)(rand() % 360);
                } else {
                    AsObject(e)->Direction[1] = (float)(rand() % 64 + 128) * -0.1f;
                }
                *(DWORD *)&AsObject(e)->Direction[0] = 0;
                AsObject(e)->Direction[2] = (float)(rand() % 64 - 32) * 0.1f;
            }
            int v35 = *v0;
            if (*(float*)(*v0 + 24) + 100.0f > AsObject(e)->Position[2])
                AsObject(e)->Direction[2] = AsObject(e)->Direction[2] + 1.5f;
            if (*(float*)(v35 + 24) + 200.0f < AsObject(e)->Position[2])
                AsObject(e)->Direction[2] = AsObject(e)->Direction[2] - 1.5f;
        }
        }
        }

    }
next_bug:
        v0 += 111;                              // +0x1BC bytes
    }
}

namespace {
// IDA: sub_4FDC00 (0x004FDC00), emisores fijos de Lorencia, Dungeon y Devias.
struct SceneFlamePreset {
    int Variant;
    int Count;
    float Offset[2][3];
    bool RandomAlpha;
    bool HideMesh;
};
constexpr SceneFlamePreset SceneFlames[] = {
    { 0, 1, {{0, 0, 200}}, false, false },
    { 0, 1, {{0, -30, 60}}, false, false },
    { 0, 1, {{0, 0, 60}}, true, false },
    { 0, 2, {{-150, -150, 140}, {150, -150, 140}}, false, false },
    { 0, 2, {{90, -200, 30}, {90, 200, 30}}, false, false },
    { 0, 1, {{0, 0, 0}}, false, true },
    { 1, 1, {{0, 0, 0}}, false, true },
    { 2, 1, {{0, 0, 0}}, false, true },
    { 0, 1, {{0, -30, 240}}, false, false },
    { 0, 1, {{0, 0, 190}}, false, false },
    { 0, 1, {{0, 0, 50}}, false, false },
};
constexpr auto MakeLorenciaFlameIndices()
{
    std::array<unsigned char, 133> indices{};
    indices[50] = 0; indices[51] = 1; indices[52] = 2;
    indices[55] = 3; indices[80] = 4;
    indices[130] = 5; indices[131] = 6; indices[132] = 7;
    return indices;
}
constexpr auto LorenciaFlameIndices = MakeLorenciaFlameIndices();

void EmitSceneFlames(char* object, const SceneFlamePreset& preset)
{
    for (int i = 0; i < preset.Count; ++i) {
        const auto& offset = preset.Offset[i];
        FUN_0046c7f0(preset.Variant, AsObject(object), offset[0], offset[1], offset[2]);
    }
    if (preset.RandomAlpha)
        *(float*)(object + 104) = (double)(rand() % 6 + 4) * 0.1;
    if (preset.HideMesh)
        *(int*)(object + 88) = -2;
}
} // namespace

// === FUN_004fdc00 / MoveObjects (0x004FDC00) ===
// Gateada por IDA_PORT_004FDC00, que esta definida.
// Es la copia VIVA: el `MoveObject_PerWorld` de mas arriba en este archivo es
// el resumen viejo y no tiene llamadores (su unico call site, en
// Scene_ObjectLegacy.cpp, esta bajo `#ifndef IDA_PORT_004FDC00`).
// Alias locales para este port. #undef al final del bloque.
#define Models      DAT_05828d58
#define EditFlag    DAT_07e11d30
extern void __cdecl Effect_PhysicsTick(std::uintptr_t Object); // Efectos del mapa 4
extern "C" void DbgLogPublic(const char*);        // [DIAG activación temporal]
void __cdecl FUN_004fdc00(OBJECT* object)
{
  // IDA 0x004FDD00/0x004FDD0D: carga la dirección y lee Type en +2.
  // Se conserva la aritmética en bytes hasta migrar los layouts en su propia capa.
  const std::uintptr_t o = reinterpret_cast<std::uintptr_t>(object);
  double v1; // st7
  short v3; // ax
  short v4; // ax
  std::uintptr_t v5; // Dirección del modelo, IDA 0x004FDC00.
  float *v6; // ebp
  float *v7; // edi
  double v8; // st7
  double v9; // st7
  double v10; // st7
  long double v11; // st7
  long double v12; // st7
  short v13; // cx
  double v14; // st6
  double v15; // st4
  long double v16; // st7
  int v17; // eax
  bool v18; // zf
  signed int v19; // eax
  int v20; // eax
  double v21; // st7
  long double v22; // st7
  long double v23; // st7
  double v24; // st7
  double v25; // st7
  long double v26; // st7
  int v27; // eax
  float xf; // [esp+0h] [ebp-40h]
  float yf; // [esp+4h] [ebp-3Ch]
  float v30[3]; // [esp+28h] [ebp-18h] BYREF
  float Light[3]; // [esp+34h] [ebp-Ch] BYREF
  float oa; // [esp+44h] [ebp+4h]
  float ob; // [esp+44h] [ebp+4h]

  if ( gMapManager.GetCurrentMap() == 9 )
  {
    if ( (std::int64_t)WorldTime % 4000 < 1000 && !(rand() % 100) )
    {
      v1 = (double)(rand() % 12 + 4) * 0.1;
      Light[0] = v1 * 0.2;
      Light[1] = v1 * 0.30000001;
      Light[2] = v1 * 0.5;
      yf = (double)(rand() % 1200) + *(float *)(Hero + 20) - 600.0;
      xf = (double)(rand() % 1200) + *(float *)(Hero + 16) - 600.0;
      AddTerrainLight(xf, yf, (float*)Light, 12, (float*)PrimaryTerrainLight[0]);
    }
    PlayBuffer(1, 0, 1);
  }
  if ( !gMapManager.GetCurrentMap() )
  {
    v3 = *(WORD *)(o + 2);
    if ( v3 != 125 && v3 != 126 )
    {
      goto LABEL_22;
    }
    if ( HeroTile == 4 )
    {
      *(DWORD *)(o + 356) = 0;
    }
    else
    {
      *(DWORD *)(o + 356) = 1065353216;
    }
  }
  if ( gMapManager.GetCurrentMap() == 2 )
  {
    v4 = *(WORD *)(o + 2);
    if ( v4 == 81 || v4 == 82 || v4 == 96 || v4 == 98 || v4 == 99 )
    {
      if ( HeroTile == 3 || HeroTile >= 10 )
      {
        *(DWORD *)(o + 356) = 0;
      }
      else
      {
        *(DWORD *)(o + 356) = 1065353216;
      }
    }
  }
LABEL_22:
  Alpha(AsObject(o));
  if ( *(float *)(o + 360) < 0.0099999998 )
  {
    return;
  }
  v5 = Models + 188 * *(short *)(o + 2);
  *(BYTE *)(v5 + 160) = *(BYTE *)(o + 261);
  oa = *(float *)(o + 204);
  if ( gMapManager.GetCurrentMap() == 8 && *(WORD *)(o + 2) == 8 )
  {
    oa = oa * 4.0;
  }
  v6 = (float *)(o + 28);
  v7 = (float *)(o + 16);
  // NOTA: nuestro BMD__PlayAnimation (BMD_Anim.cpp) es la variante de 5 args (avanza
  // el frame). El IDA sub_440AA0 toma 7 (los 2 últimos = pos/vel para root-motion
  // de la animación). Los omitimos: el avance de frame —lo que faltaba— funciona.
  BMD__PlayAnimation((void*)v5, (float*)(o + 264), (float*)(o + 268),
               (void*)(o + 262), oa);
  if ( SceneFlag == 2 || SceneFlag == 4 )
  {
    if ( *(WORD *)(o + 2) == 160 )
    {
      v9 = (double)((std::int64_t)WorldTime % 4000);
    }
    else
    {
      if ( *(WORD *)(o + 2) != 161 )
      {
        if ( *(WORD *)(o + 2) == 162 )
        {
          if ( CameraWalkCut )
          {
            v8 = 1.5;
          }
          else
          {
            v8 = (double)CurrentCameraCount * 0.0020000001;
          }
          *(float *)(o + 232) = v8;
          *(float *)(o + 236) = v8;
          *(float *)(o + 240) = v8;
          *(float *)(o + 104) = v8;
        }
        goto LABEL_38;
      }
      v9 = (double)((std::int64_t)WorldTime % 4000);
    }
    *(float *)(o + 112) = -(v9 * 0.00025000001);
  }
LABEL_38:
  switch ( gMapManager.GetCurrentMap() )
  {
    case 0:
      switch ( *(WORD *)(o + 2) )
      {
        case 0x32: case 0x33: case 0x34: case 0x37: case 0x50:
        case 0x82: case 0x83: case 0x84:
          EmitSceneFlames((char*)o, SceneFlames[LorenciaFlameIndices[*(WORD*)(o + 2)]]);
          break;
        case 0x5A:
          v10 = (double)(rand() % 2 + 6) * 0.1;
          v30[0] = v10;
          v30[1] = v10 * 0.80000001;
          v11 = v10 * 0.60000002;
          goto LABEL_116;
        case 0x75:
        case 0x7A:
          goto LABEL_59;
        case 0x76:
        case 0x77:
          *(float *)(o + 112) = (double)(-(std::int64_t)WorldTime % 1000) * 0.001;
          break;
        case 0x96:
          v12 = (double)(rand() % 4 + 3) * 0.1;
          goto LABEL_51;
        default:
          return;
      }
      break;
    case 1:
      switch ( *(WORD *)(o + 2) )
      {
        case 0x16:
        case 0x17:
        case 0x18:
          *(BYTE *)(Models + 188 * *(short *)(o + 2) + 136) = 1;
          *(float *)(o + 112) = (double)((std::int64_t)WorldTime % 1000) * -0.001;
          break;
        case 0x27:
        case 0x28:
        case 0x33:
          goto LABEL_132;
        case 0x29:
          EmitSceneFlames((char*)o, SceneFlames[8]);
          break;
        case 0x2A:
          EmitSceneFlames((char*)o, SceneFlames[9]);
          break;
        case 0x34:
          if ( rand() % 3 )
          {
            goto LABEL_132;
          }
          CreateEffect(
            215,
            (float *)(o + 16),
            (float *)(o + 28),
            (float *)(o + 232),
            0,
            0,
            (float *)-1,
            0,
            0);
          *(DWORD *)(o + 88) = -2;
          break;
        default:
          return;
      }
      break;
    case 2:
      v13 = *(WORD *)(o + 2);
      switch ( v13 )
      {
        case 20:
        case 65:
        case 86:
        case 88:
          if ( !EditFlag )
          {
            v14 = *(float *)(Hero + 20) - *(float *)(o + 56);
            v15 = *(float *)(Hero + 16) - *(float *)(o + 52);
            v16 = sqrt(v14 * v14 + v15 * v15);
            ob = v16;
            if ( v16 >= 200.0 )
            {
              *(float *)(o + 36) = TurnAngle2(*(float *)(o + 36), *(float *)(o + 48), 10.0);
              *v7 = (*(float *)(o + 52) - *v7) * 0.2 + *v7;
              *(float *)(o + 20) = (*(float *)(o + 56) - *(float *)(o + 20)) * 0.2
                                          + *(float *)(o + 20);
            }
            else if ( v13 == 86 )
            {
              if ( *(DWORD *)(o + 36) == 1119092736 )
              {
                *(float *)(o + 20) = 200.0 - ob + 200.0 - ob + *(float *)(o + 56);
              }
              if ( *(DWORD *)(o + 36) == 1132920832 )
              {
                *(float *)(o + 20) = *(float *)(o + 56) - (200.0 - ob + 200.0 - ob);
              }
              if ( *(float *)(o + 36) == 0.0 )
              {
                *v7 = 200.0 - ob + 200.0 - ob + *(float *)(o + 52);
              }
              if ( *(DWORD *)(o + 36) == 1127481344 )
              {
                *v7 = *(float *)(o + 52) - (200.0 - ob + 200.0 - ob);
              }
              PlayBuffer(18, 0, 0);
            }
            else
            {
              if ( *(DWORD *)(o + 48) == 1119092736 )
              {
                *(float *)(o + 36) = 30.0 - (200.0 - ob) * 0.5;
              }
              if ( *(DWORD *)(o + 48) == 1132920832 )
              {
                *(float *)(o + 36) = (200.0 - ob) * 0.5 + 330.0;
              }
              if ( *(float *)(o + 48) == 0.0 )
              {
                *(float *)(o + 36) = 300.0 - (200.0 - ob) * 0.5;
              }
              if ( *(DWORD *)(o + 48) == 1127481344 )
              {
                *(float *)(o + 36) = (200.0 - ob) * 0.5 + 240.0;
              }
              PlayBuffer(17, 0, 0);
            }
          }
          break;
        case 30:
        case 66:
          EmitSceneFlames((char*)o, SceneFlames[10]);
          break;
        case 78:
LABEL_59:
          *(float *)(o + 104) = (double)(rand() % 4 + 4) * 0.1;
          break;
        default:
          return;
      }
      break;
    case 3:
      v17 = *(short *)(o + 2);
      switch ( *(WORD *)(o + 2) )
      {
        case 0x12:
          *(float *)(o + 112) = (double)((std::int64_t)WorldTime % 1000) * 0.001;
          break;
        case 0x27:
          goto LABEL_92;
        case 0x29:
          *(DWORD *)(o + 100) = 0;
          *(float *)(o + 112) = (double)((std::int64_t)WorldTime % 2000) * 0.00050000002;
          break;
        case 0x2A:
          *(BYTE *)(Models + 188 * v17 + 136) = 0;
          *(float *)(o + 108) = (double)((std::int64_t)WorldTime % 500) * -0.0020000001;
          break;
        case 0x2B:
          *(BYTE *)(Models + 188 * v17 + 136) = 0;
          *(float *)(o + 108) = (double)((std::int64_t)WorldTime % 500) * 0.0020000001;
          break;
        default:
          return;
      }
      break;
    case 4:
      switch ( *(WORD *)(o + 2) )
      {
        case 3:
        case 4:
          *(float *)(o + 108) = (double)(-(std::int64_t)WorldTime % 1000) * 0.001;
          break;
        case 0x12:
        case 0x17:
LABEL_92:
          *(DWORD *)(o + 100) = 1;
          break;
        case 0x13:
        case 0x14:
          *(DWORD *)(o + 100) = 4;
          *(float *)(o + 108) = (double)(-(std::int64_t)WorldTime % 1000) * 0.001;
          break;
        case 0x18:
          *(DWORD *)(o + 88) = -2;
          v19 = rand() & 0x8000003F;
          v18 = v19 == 0;
          if ( v19 < 0 )
          {
            v18 = (((BYTE)v19 - 1) | 0xFFFFFFC0) == -1;
          }
          if ( v18 )
          {
            CreateEffect(1200, v7, v6, (float *)(o + 232), 0, 0, (float *)-1, 0, 0);
          }
          break;
        case 0x19:
          goto LABEL_132;
        case 0x26:
        case 0x27:
          Effect_PhysicsTick(o);
          break;
        default:
          return;
      }
      break;
    case 5:
      if ( *(WORD *)(o + 2) == 2 )
      {
LABEL_111:
        *(DWORD *)(o + 100) = 0;
      }
      else if ( *(WORD *)(o + 2) == 3 )
      {
        *(DWORD *)(o + 100) = 0;
        *(float *)(o + 104) = (double)(rand() % 4 + 6) * 0.1;
      }
      break;
    case 6:
      v20 = *(short *)(o + 2);
      if ( v20 == 21 )
      {
        *(DWORD *)(o + 100) = 3;
        *(float *)(o + 112) = (double)(-(std::int64_t)WorldTime % 1000) * 0.001;
      }
      else if ( v20 == 38 )
      {
        *(DWORD *)(o + 88) = -2;
      }
      break;
    case 7:
      switch ( *(WORD *)(o + 2) )
      {
        case 0x16:
          v21 = *(float *)(o + 128) + 0.1;
          *(DWORD *)(o + 88) = -2;
          *(float *)(o + 128) = v21;
          if ( v21 > 10.0 )
          {
            *(DWORD *)(o + 128) = 0;
          }
          if ( *(float *)(o + 128) > 5.0 )
          {
            Particle_Spawn(1241, v7, v6, (float *)(o + 232), 0, 1.0, 0);
          }
          break;
        case 0x17:
          *(DWORD *)(o + 100) = 0;
          *(float *)(o + 104) = sin(WorldTime * 0.0020000001) * 0.30000001 + 0.5;
          break;
        case 0x20:
        case 0x22:
          *(DWORD *)(o + 100) = 1;
          *(float *)(o + 104) = (sin(WorldTime * 0.0040000002) + 1.0) * 0.5;
          break;
        case 0x26:
          goto LABEL_111;
        case 0x28:
          *(DWORD *)(o + 100) = 0;
          v22 = WorldTime * 0.0040000002;
          *(DWORD *)(o + 204) = 1028443341;
          *(float *)(o + 104) = sin(v22) * 0.30000001 + 0.5;
          break;
        default:
          return;
      }
      break;
    case 8:
      switch ( *(WORD *)(o + 2) )
      {
        case 2:
          *(DWORD *)(o + 100) = 0;
          *(float *)(o + 108) = (double)(-(std::int64_t)WorldTime % 1000) * 0.001;
          return;
        case 4:
          v23 = WorldTime * 0.0020000001;
          *(DWORD *)(o + 100) = 0;
          v11 = sin(v23) * 0.34999999 + 0.64999998;
          *(float *)(o + 104) = v11;
          *(float *)(o + 112) = (double)(-(std::int64_t)WorldTime % 10000) * 0.000099999997;
          v30[0] = v11;
          v30[1] = v11;
          goto LABEL_116;
        case 7:
          v24 = *(float *)(o + 36) * 100.0 + WorldTime;
          *(DWORD *)(o + 100) = 0;
          v12 = sin(v24 * 0.0020000001) * 0.34999999 + 0.64999998;
          *(float *)(o + 104) = v12;
LABEL_51:
          v30[0] = v12;
          v30[1] = v12 * 0.60000002;
          v11 = v12 * 0.2;
LABEL_116:
          v30[2] = v11;
          AddTerrainLight(*v7, *(float *)(o + 20), v30, 3, PrimaryTerrainLight[0]);
          return;
        case 0xB: case 0xD: case 0x49: case 0x4B: case 0x4F:
          *(float *)(o + 112) = (double)(-(std::int64_t)WorldTime % 10000) * 0.00019999999;
          return;
        case 0xC:
          *(float *)(o + 108) = (double)(-(std::int64_t)WorldTime % 50000) * 0.000049999999;
          *(float *)(o + 112) = (double)(-(std::int64_t)WorldTime % 50000) * 0.000049999999;
          return;
        case 0x3D:
          *(DWORD *)(o + 100) = 1;
          v25 = (double)(-(std::int64_t)WorldTime % 1000);
          goto LABEL_123;
        case 0x3F:
        case 0x40:
          goto LABEL_132;
        case 0x41:
        case 0x42:
          *(DWORD *)(o + 100) = 1;
          v25 = (double)(-(std::int64_t)WorldTime % 1000);
LABEL_123:
          *(float *)(o + 112) = v25 * 0.001;
          v26 = sin(WorldTime * 0.0020000001) * 0.34999999 + 0.64999998;
          v30[0] = v26;
          v30[1] = v26 * 0.60000002;
          v30[2] = v26 * 0.2;
          AddTerrainLight(*v7, *(float *)(o + 20), v30, 2, PrimaryTerrainLight[0]);
          break;
        case 0x48:
          *(DWORD *)(o + 100) = 0;
          *(float *)(o + 112) = (double)(-(std::int64_t)WorldTime % 10000) * 0.00019999999;
          break;
        case 0x52:
          *(DWORD *)(o + 100) = 0;
          *(DWORD *)(o + 232) = 1065353216;
          *(DWORD *)(o + 236) = 1065353216;
          *(DWORD *)(o + 240) = 1065353216;
          break;
        default:
          return;
      }
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      v27 = *(short *)(o + 2);
      if ( v27 >= 9 && v27 <= 10 && *(WORD *)(o + 134) != 4 )
      {
LABEL_132:
        *(DWORD *)(o + 88) = -2;
      }
      break;
    default:
      return;
  }
}
#undef Models
#undef EditFlag
