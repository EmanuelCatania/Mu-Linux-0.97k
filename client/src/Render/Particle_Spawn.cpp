#include <cstdint>
// Particle_Spawn.cpp — IDA: FUN_00475220 @ 0x00475220
// Allocates a free slot in the particle pool and initialises it.
// The pool starts at DAT_07abf5f0; each slot is 0x70 bytes.
// Overflow guard: if the slot pointer exceeds 0x7b1166f, returns 0.
//
// Slot layout (offsets within each 0x70 block):
//   +0x00  char     active flag (0=free, 1=alive)
//   +0x04  int      effect_type  (param_1)
//   +0x08  int      sub_type     (param_5)
//   +0x0c  float    scale        (param_6)
//   +0x10  float[3] world_xyz    (param_2)
//   +0x1c  float[3] dir_xyz      (param_3)
//   +0x28  float[3] color_rgb    (param_4)
//   +0x38  int      lifetime     (default 2)
//   +0x3c  int      entity_ptr   (param_7)
//   +0x40  float    rotation
//   +0x44  float    (clear)
//   +0x48  float    extra2
//   +0x4c  float    extra3
//   +0x50  float[3] vel_xyz
//   +0x5c  float[3] color_copy   (set to color_rgb for some types)
//   +0x64  float[3] prev_xyz     (copy of world_xyz for some types)

#include "stdafx.h"
#include "Game/RenderFxSnapshots.h"
#include <array>
#include "globals.h"
#include "functions.h"


namespace {
// IDA: Particle_Spawn (0x00475220). Presets sin azar; el switch limita
// la consulta a estos siete tipos. Los demás conservan su inicialización.
enum class ParticleRenderSetup : unsigned char { None, Always, ExceptSubtype99 };
struct ParticlePreset {
    int Lifetime;
    bool Angle45;
    bool UnitScale;
    bool ImpactSound;
    ParticleRenderSetup RenderSetup;
};

constexpr int FirstPresetType = 0x4bf;
constexpr auto MakeParticlePresets()
{
    std::array<ParticlePreset, 0x596 - FirstPresetType + 1> presets{};
    presets[0x4bf - FirstPresetType] = { 20, false, false, true, ParticleRenderSetup::None };
    presets[0x4c0 - FirstPresetType] = { 12, false, false, true, ParticleRenderSetup::None };
    presets[0x4ce - FirstPresetType] = { 18, true, false, false, ParticleRenderSetup::None };
    presets[0x4cf - FirstPresetType] = { 36, true, false, false, ParticleRenderSetup::ExceptSubtype99 };
    presets[0x4d0 - FirstPresetType] = { 18, false, false, false, ParticleRenderSetup::Always };
    presets[0x4d2 - FirstPresetType] = { 20, false, false, false, ParticleRenderSetup::None };
    presets[0x596 - FirstPresetType] = { 1, false, true, false, ParticleRenderSetup::None };
    return presets;
}
constexpr auto ParticlePresets = MakeParticlePresets();
constexpr int ParticleScaleOffset = 0x0c;
constexpr int ParticleAngleOffset = 0x1c;
constexpr int ParticleLifetimeOffset = 0x38;

void ApplyParticlePreset(char* particle, int type, int subtype)
{
    const auto& preset = ParticlePresets[type - FirstPresetType];
    *(int*)(particle + ParticleLifetimeOffset) = preset.Lifetime;
    if (preset.Angle45) *(float*)(particle + ParticleAngleOffset) = 45.0f;
    if (preset.UnitScale) *(float*)(particle + ParticleScaleOffset) = 1.0f;
    if (preset.ImpactSound) PlayBuffer(0x2c, 0, 0);
    if (preset.RenderSetup == ParticleRenderSetup::Always ||
        (preset.RenderSetup == ParticleRenderSetup::ExceptSubtype99 && subtype != 99))
        ItemDrop_SetupRenderRef(reinterpret_cast<BYTE*>(particle));
}
// IDA: Particle_Spawn (0x00475220), tipos 1241..1244.
// rand() es no negativo: los restos conservan el orden y número de llamadas.
struct ParticleDispersion {
    int MinimumLifetime;
    int ScaleVariants;
    bool HasVelocity;
};
constexpr ParticleDispersion ParticleDispersions[] = {
    { 30, 6, false },
    { 15, 8, true },
    { 15, 8, true },
    { 15, 8, true },
};
constexpr int ParticleColorOffset = 0x28;
constexpr int ParticleDriftOffset = 0x4c;

void ApplyParticleDispersion(char* particle, int type)
{
    const auto& definition = ParticleDispersions[type - 1241];
    *(int*)(particle + ParticleLifetimeOffset) = _rand() % 10 + definition.MinimumLifetime;
    const int scaleStep = _rand() % definition.ScaleVariants;
    float* color = (float*)(particle + ParticleColorOffset);
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    *(float*)(particle + ParticleScaleOffset) = (float)(scaleStep + 4) * _DAT_00552910;
    if (definition.HasVelocity) {
        float* drift = (float*)(particle + ParticleDriftOffset);
        drift[0] = (float)(_rand() % 32 - 16) * _DAT_005524f4;
        drift[1] = (float)(_rand() % 32 - 16) * _DAT_005524f4;
        drift[2] = (float)(_rand() % 32 - 32) * _DAT_005524f4;
    }
}
// IDA: Particle_Spawn (0x00475220), tipos 1221/1222. Cada trayectoria
// conserva sus matrices, destino y orden de rand; los vectores son contiguos.
constexpr int ParticlePositionOffset = 0x10;
constexpr int ParticlePreviousPositionOffset = 0x64;

void InitializeDirectedParticle(char* particle, int subtype, float* sourceAngle)
{
    float* position = (float*)(particle + ParticlePositionOffset);
    float* angle = (float*)(particle + ParticleAngleOffset);
    *(float*)(particle + ParticleScaleOffset) = (float)(_rand() % 32 + 32) * _DAT_005524f8;
    position[0] = (float)(_rand() % 16 - 8) + position[0];
    position[1] = (float)(_rand() % 16 - 8) + position[1];
    float matrix[12];
    float direction[3] = { 0.0f, 3.0f, 0.0f };
    if (subtype == 0) {
        Matrix_BuildFromEuler(sourceAngle, matrix);
    } else {
        angle[2] = (float)(_rand() % 360);
        Matrix_BuildFromEuler(angle, matrix);
        direction[1] = 15.0f;
    }
    Vector_Rotate(direction, matrix, (float*)(particle + ParticleDriftOffset));
    angle[0] = (float)(_rand() % 360);
}

void InitializeRadialParticle(char* particle, int, float*)
{
    float* angle = (float*)(particle + ParticleAngleOffset);
    const int scaleStep = _rand() % 32;
    angle[0] = 0.0f;
    angle[1] = 0.0f;
    *(float*)(particle + ParticleScaleOffset) = (float)(scaleStep + 64) * _DAT_005524f8;
    angle[2] = (float)(_rand() % 360);
    float matrix[12];
    Matrix_BuildFromEuler(angle, matrix);
    float direction[3];
    direction[0] = 0.0f;
    direction[1] = (float)(_rand() % 64);
    direction[2] = (float)(_rand() % 16 + 16);
    Vector_Rotate(direction, matrix, (float*)(particle + ParticlePreviousPositionOffset));
    angle[0] = 0.0f;
    angle[1] = 0.0f;
    angle[2] = 0.0f;
}

struct ParticleTrajectory {
    int Lifetime;
    void (*Initialize)(char*, int, float*);
};
constexpr ParticleTrajectory ParticleTrajectories[] = {
    { 32, InitializeDirectedParticle }, // 1221: dirección de entrada o giro aleatorio.
    { 50, InitializeRadialParticle },   // 1222: offset rotado, ángulos finales a cero.
};
// IDA: Particle_Spawn (0x00475220), tipos 1196/1383.
// Desplazamiento en Y: se conserva el orden del azar y la escala de cada tipo.
struct ParticleLinearMotion {
    int Lifetime;
    int SpeedVariants;
    const float* SpeedFactor;
    bool RandomScaleAndGray;
};
constexpr ParticleLinearMotion ParticleLinearMotions[] = {
    { 12, 8, &_DAT_005529c0, false }, // 1196: escala fija, color de entrada.
    { 24, 16, &_DAT_005529c8, true }, // 1383: escala aleatoria, gris.
};
constexpr int ParticleRotationOffset = 0x40;

void ApplyParticleLinearMotion(char* particle, const ParticleLinearMotion& definition)
{
    *(int*)(particle + ParticleLifetimeOffset) = definition.Lifetime;
    float* drift = (float*)(particle + ParticleDriftOffset);
    drift[0] = 0.0f;
    const int speedStep = _rand() % definition.SpeedVariants;
    drift[2] = 0.0f;
    if (!definition.RandomScaleAndGray)
        *(float*)(particle + ParticleScaleOffset) = 0.8f;
    drift[1] = (float)(speedStep + 32) * *definition.SpeedFactor;
    if (definition.RandomScaleAndGray)
        *(float*)(particle + ParticleScaleOffset) = (float)(_rand() % 128 + 128) * _DAT_005524f8;
    const int rotation = _rand() % 360;
    if (definition.RandomScaleAndGray) {
        float* color = (float*)(particle + ParticleColorOffset);
        color[0] = 0.5f;
        color[1] = 0.5f;
        color[2] = 0.5f;
    }
    *(float*)(particle + ParticleRotationOffset) = (float)rotation;
}
// IDA: Particle_Spawn (0x00475220), tipos 1268/subtipo 6 y 1382.
void InitializeRotatingImpulse(char* particle, float scale)
{
    *(int*)(particle + ParticleLifetimeOffset) = _rand() % 5 + 25;
    const int rotation = _rand() % 360;
    *(float*)(particle + ParticleScaleOffset) = scale;
    *(float*)(particle + ParticleRotationOffset) = (float)rotation;
    const int speedStep = _rand() % 10;
    float* drift = (float*)(particle + ParticleDriftOffset);
    drift[1] = 0.0f;
    drift[0] = (float)(speedStep + 5) * _DAT_005528b4;
    drift[2] = (float)(_rand() % 10 + 5) * _DAT_005526e4;
}

// IDA: Particle_Spawn (0x00475220), tipo 1381: cero y no cero son variantes distintas.
struct ParticleImpulse {
    const float* ScaleFactor;
    const float* ScaleBase;
    const float* HorizontalFactor;
    const float* VerticalFactor;
};
constexpr ParticleImpulse ParticleImpulses[] = {
    { &_DAT_00552914, &_DAT_00552504, &_DAT_005528b4, &_DAT_005526e4 },
    { &_DAT_005524f4, &_DAT_0055256c, &_DAT_005526e4, &_DAT_005524f4 },
};

void InitializeParticleImpulse(char* particle, int subtype)
{
    const int lifetime = _rand() % 5 + 20;
    *(float*)(particle + ParticleRotationOffset) = 0.0f;
    *(int*)(particle + ParticleLifetimeOffset) = lifetime;
    const auto& definition = ParticleImpulses[subtype != 0 ? 1 : 0];
    *(float*)(particle + ParticleScaleOffset) =
        (float)(_rand() % 10) * *definition.ScaleFactor + *definition.ScaleBase;
    float* drift = (float*)(particle + ParticleDriftOffset);
    drift[0] = (float)(_rand() % 10 + 5) * *definition.HorizontalFactor;
    drift[1] = (float)(_rand() % 10 - 5) * *definition.HorizontalFactor;
    drift[2] = (float)(_rand() % 10 + 5) * *definition.VerticalFactor;
}
// IDA: Particle_Spawn (0x00475220), tipo 1191: sólo subtipos 0..3.
struct ParticleScalePreset { int Lifetime; float Scale; };
constexpr ParticleScalePreset ParticleScalePresets[] = {
    { -1, 0.15f }, { 10, 0.0f }, { 20, 1.0f }, { 1, -1.0f },
};
// IDA: Particle_Spawn (0x00475220), fuego 1220/subtipos 1, 3 y 11.
struct ParticleFirePreset {
    int Lifetime;
    bool RandomScale;
    bool OffsetPosition;
    bool CopyColor;
};
constexpr ParticleFirePreset ParticleFires[] = {
    { 50, true, true, false }, { 10, true, false, false }, { 50, false, true, true },
};
constexpr int ParticleSecondaryColorOffset = 0x58;
constexpr float ParticleFixedRotations[] = { 0.0f, 45.0f, 315.0f };

void InitializeFireParticle(char* particle, const ParticleFirePreset& preset)
{
    *(int*)(particle + ParticleLifetimeOffset) = preset.Lifetime;
    if (preset.RandomScale)
        *(float*)(particle + ParticleScaleOffset) = (float)(_rand() % 32 + 80) * _DAT_005524f8;
    if (preset.OffsetPosition) {
        float* position = (float*)(particle + ParticlePositionOffset);
        position[0] = (float)(_rand() % 64 - 32) + position[0];
        position[1] = (float)(_rand() % 64 - 32) + position[1];
        position[2] = (float)(_rand() % 64 + 32) + position[2];
    }
    float* angle = (float*)(particle + ParticleAngleOffset);
    angle[0] = (float)(_rand() % 90 - 45);
    const int rotation = _rand() % 360;
    float* drift = (float*)(particle + ParticleDriftOffset);
    drift[0] = 0.0f;
    if (preset.CopyColor) {
        auto* color = (unsigned int*)(particle + ParticleColorOffset);
        auto* copy = (unsigned int*)(particle + ParticleSecondaryColorOffset);
        copy[1] = color[1]; copy[2] = color[2]; copy[0] = color[0];
    }
    angle[2] = (float)rotation;
    const int speedStep = _rand() % 8;
    drift[2] = 0.0f;
    drift[1] = -(float)(speedStep + 40);
}
} // namespace

// IDA: Particle_Spawn (0x00475220)
int __cdecl Particle_Spawn(int param_1, float *param_2, float *param_3, float *param_4,
                         int param_5, float param_6, int param_7)
{
    float *pfVar1;
    float *pfVar2;
    float *pfVar3;
    float fVar4;
    float fVar5;
    int   iVar6;
    int   iVar7;
    unsigned int uVar8;
    int   iVar9;
    int   iVar10;
    char *pcVar11;
    long double fVar12;
    long double fVar13;
    // Locales contiguos: IDA Particle_Spawn (0x00475220) declara `float in1[3]`
    // en [ebp-3Ch] y lo pasa entero a VectorRotate.  Ghidra lo emitio como TRES
    // escalares sueltos (local_3c/38/34 = ebp-0x3C/-0x38/-0x34) y los seis call
    // sites de esta funcion hacen `Vector_Rotate(&local_3c, ...)`, o sea leen 3
    // floats contiguos desde el primero.  MSVC no garantiza ese layout, asi que
    // se usa un array real.
    float in1_3c[3];
    float &local_3c = in1_3c[0];
    float &local_38 = in1_3c[1];
    float &local_34 = in1_3c[2];
    float local_30[12];

    // ── scan pool for free slot ────────────────────────────────────────────────
    // Pool real de 3000 slots: se itera por índice, no por el end-bound absoluto
    // 0x7b1166f del binario.
    iVar6 = 0;
    pcVar11 = DAT_07abf5f0;
    while (*pcVar11 != '\0') {
        pcVar11 = pcVar11 + 0x70;
        iVar6 = iVar6 + 1;
        if (iVar6 >= 3000)
            return 0;
    }

    RenderFxSnapshots_New(pcVar11);

    // ── base initialisation ────────────────────────────────────────────────────
    *(int *)(pcVar11 + 8)  = param_5;
    *pcVar11               = '\x01';               // mark active
    *(int *)(pcVar11 + 4)  = param_1;              // effect_type
    pfVar1 = (float *)(pcVar11 + 0x10);
    pfVar2 = (float *)(pcVar11 + 0x1c);
    *pfVar1                    = *param_2;
    *(float *)(pcVar11 + 0x14) = param_2[1];
    *(float *)(pcVar11 + 0x18) = param_2[2];
    // prev_xyz = world_xyz
    *(float *)(pcVar11 + 100)  = *param_2;          // +0x64
    *(float *)(pcVar11 + 0x68) = param_2[1];
    *(float *)(pcVar11 + 0x6c) = param_2[2];
    // color
    *(float *)(pcVar11 + 0x28) = *param_4;
    *(float *)(pcVar11 + 0x2c) = param_4[1];
    *(float *)(pcVar11 + 0x30) = param_4[2];
    // scale
    *(float *)(pcVar11 + 0x0c) = param_6;
    // clear extra / rotation fields
    *(float*)(pcVar11 + 0x48) = 0.0f;
    *(float*)(pcVar11 + 0x44) = 0.0f;
    *(float*)(pcVar11 + ParticleRotationOffset) = 0.0f;
    // lifetime default = 2
    *(int*)(pcVar11 + ParticleLifetimeOffset) = 2;
    *(int *)(pcVar11 + 0x3c) = param_7;            // entity_ptr
    // dir
    *pfVar2                    = *param_3;
    *(float *)(pcVar11 + 0x20) = param_3[1];
    *(float *)(pcVar11 + 0x24) = param_3[2];
    pfVar3 = (float *)(pcVar11 + 0x4c);
    *pfVar3 = 0.0f;
    // clear vel
    *(float*)(pcVar11 + ParticleDriftOffset + 4) = 0.0f;
    *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;

    // IDA: Particle_Spawn (0x00475220). Despacho directo por tipo;
    // cada familia conserva sus subtipos y el orden de inicialización.
    switch (param_1) {
    case 1220: {
        // 0x4c4 — fire/ember type; sub_type selects variant
        switch (param_5) {
        case 0: case 4: case 9:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 16;
            uVar8 = _rand() % 32;
            *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x30) * _DAT_005524f8;
            iVar7 = _rand();
            *pfVar2 = (float)(iVar7 % 0x168);
            // IDA: `*((float *)v8 + 16) = (float)((std::int64_t)WorldTime % 360);`
            *(float *)(pcVar11 + 0x40) = (float)(int)((std::int64_t)WorldTime % 360);
            return iVar6;
        case 1: case 3: case 11:
            InitializeFireParticle(pcVar11, ParticleFires[param_5 == 1 ? 0 : (param_5 == 3 ? 1 : 2)]);
            return iVar6;
        case 2: case 5:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = param_5 == 2 ? 50 : 20;
            uVar8 = _rand() % 64;
            *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + (param_5 == 2 ? 0x40 : 0x62)) * _DAT_005524f8;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
            uVar8 = _rand() % 32;
            *(float *)(pcVar11 + 0x48) = (float)(int)(uVar8 + 0x3c) * _DAT_005524f4;
            return iVar6;
        case 6:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 30;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 1000);
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 0x14 + 0xb4) * _DAT_005524f8;
            iVar7 = _rand();
            *pfVar1 = (float)(iVar7 % 200 + -100) * param_6 + *pfVar1;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x14) = (float)(iVar7 % 200 + -100) * param_6 + *(float *)(pcVar11 + 0x14);
            iVar7 = _rand();
            *pfVar3 = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 4) = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            fVar4 = (float)(iVar7 % 0x14 + 0x14) + *(float *)(pcVar11 + 0x18);
            *(float *)(pcVar11 + 0x18) = fVar4;
            *(float *)(pcVar11 + 0x40) = fVar4;
            return iVar6;
        case 7:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 30;
            *pfVar3 = 0.0f;
            uVar8 = _rand() % 4;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 6) * *(float *)(pcVar11 + 0x0c);
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 200) * _DAT_00552abc;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x0c) =
                (float)(iVar7 % 0x14 + 0x78) * *(float *)(pcVar11 + 0x0c) * _DAT_005524f8;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
            return iVar6;
        case 8:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 24;
            *pfVar3 = 0.0f;
            uVar8 = _rand() % 8;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 0x20) * _DAT_005529c0;
            *(float *)(pcVar11 + 0x0c) = *(float *)(pcVar11 + 0x0c) * _DAT_00552530;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
            return iVar6;
        case 10:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 16;
            return iVar6;
        default:
            return iVar6;
        }
    }
    case 1190: {
        // 0x4a6 — up-burst with fixed scale
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 10;
        *(float*)(pcVar11 + ParticleScaleOffset) = 1.8f;
        *pfVar2 = 30.0f;
        uVar8 = _rand() % 4;
        *(float *)(pcVar11 + 0x28) = (float)(int)(uVar8 + 6) * _DAT_005524f4;
        uVar8 = _rand() % 4;
        *(float *)(pcVar11 + 0x2c) = (float)(int)(uVar8 + 6) * _DAT_005524f4;
        uVar8 = _rand() % 4;
        *(float *)(pcVar11 + 0x30) = (float)(int)(uVar8 + 6) * _DAT_005524f4;
        *(float *)(pcVar11 + 0x18) = *(float *)(pcVar11 + 0x18) + _DAT_005529e0;
        return iVar6;
    }
    case 1175: {
        // 0x497 — matrix-transformed directional burst
        uVar8 = _rand() % 4;
        iVar7 = *(int *)(pcVar11 + 8);
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 4) * _DAT_005524f4;
        if (((iVar7 != 0) && (iVar7 != 2)) && (iVar7 != 3)) {
            uVar8 = _rand() % 8;
            *(unsigned int *)(pcVar11 + 0x38) = uVar8 + 8;
            return iVar6;
        }
        uVar8 = _rand() % 16;
        *(unsigned int *)(pcVar11 + 0x38) = uVar8 + 0x18;
        iVar7 = _rand();
        *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
        Matrix_BuildFromEuler(pfVar2, local_30);
        uVar8 = _rand() % 16;
        local_3c = 0.0f;
        *(float *)(pcVar11 + 0x48) = (float)(int)(uVar8 + 6);
        iVar7 = _rand();
        local_38 = (float)(iVar7 % 0x14 + 0x14) * _DAT_005524f4;
        if (*(int *)(pcVar11 + 8) == 2) {
            *(float *)(pcVar11 + 0x0c) = *(float *)(pcVar11 + 0x0c) + *(float *)(pcVar11 + 0x0c);
            local_3c = local_3c * _DAT_00552540;
            local_38 = local_38 * _DAT_00552540;
        }
        local_34 = 0.0f;
        Vector_Rotate(&local_3c, local_30, pfVar3);
        return iVar6;
    }
    case 1168: {
        // 0x490 — glow with RGBA
        fVar4 = *(float *)(pcVar11 + 0x24) + _DAT_0055293c;
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 10;
        *(float*)(pcVar11 + 0x48) = 0.1f;
        *(float *)(pcVar11 + 0x40) = fVar4;
        return iVar6;
    }
    case 1151: {
        // IDA 0x475220 L810-814: `if (Type == 1151) { life =
        // rand() % 8 + 20; return; }`.  El port lo mandaba con un
        // goto al codigo de 0x4ab (vida 24), y el cuerpo real de
        // 1151 habia quedado suelto mas abajo (ver la nota del
        // bloque `< 0x4a7`).
        *(int *)(pcVar11 + 0x38) = _rand() % 8 + 20;
        return iVar6;
    }
    // IDA 0.98j: polvo de Tamachan, tipos 103/104; se conserva el camino activo.
    case 103: case 104: {
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 20;
        iVar7 = _rand();
        *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 6 + 8) * _DAT_005524f4;
        if ((*(int *)(pcVar11 + 4) != 0x68) && (*(int *)(pcVar11 + 8) != 1))
            return iVar6;
        iVar7 = _rand(); *pfVar1 = (float)(iVar7 % 10 + -5) + *pfVar1;
        iVar7 = _rand(); *(float *)(pcVar11 + 0x14) = (float)(iVar7 % 10 + -5) + *(float *)(pcVar11 + 0x14);
        iVar7 = _rand(); *(float *)(pcVar11 + 0x18) = (float)(iVar7 % 10 + -5) + *(float *)(pcVar11 + 0x18);
        if (*(int *)(pcVar11 + 8) != 1) return iVar6;
        *(float *)(pcVar11 + 0x0c) = param_6;
        iVar7 = _rand();
        *(float*)(pcVar11 + ParticleDriftOffset + 4) = 1.0f;
        *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 100) * _DAT_00552890;
        return iVar6;
    }
    case 1150: {
        // 0x47e — misc bubble/spark
        if (param_5 == 0 || param_5 == 2) {
            iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 10 + 10;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 10) + _DAT_00552488;
            iVar7 = _rand();
        } else if (param_5 == 1) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 50;
            return iVar6;
        } else if (param_5 == 3) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 10;
            return iVar6;
        } else if (param_5 == 4) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 10;
            *(float*)(pcVar11 + 0x48) = 0.0f;
            *(float *)(pcVar11 + 0x40) = param_6;
            *(float*)(pcVar11 + ParticleScaleOffset) = 2.0f;
            return iVar6;
        } else if (param_5 == 5) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 25;
            return iVar6;
        } else {
            return iVar6;
        }
        *(float *)(pcVar11 + 0x0c) =
            ((float)(iVar7 % 0x32) + _DAT_00552598) * param_6 * _DAT_00552594;
        return iVar6;
    }
    case 1176: {
        // 0x498 — directional smoke/mist
        if (param_5 == 1) {
            iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x168);
            iVar7 = _rand(); *(float *)(pcVar11 + 0x20) = (float)(iVar7 % 0x168);
            iVar7 = _rand(); *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
            Matrix_BuildFromEuler(pfVar2, local_30);
            local_3c = 0.0f; local_38 = -50.0f; local_34 = 0.0f;
            Vector_Rotate(&local_3c, local_30, pfVar3);
            return iVar6;
        }
        if (param_5 == 3) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 5;
            return iVar6;
        }
        if (param_5 == 2) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 15;
            *pfVar2 = 0.0f;
            *(float*)(pcVar11 + ParticleAngleOffset + 4) = 0.0f;
            *(float*)(pcVar11 + ParticleAngleOffset + 8) = 0.0f;
            *(unsigned int *)(pcVar11 + 0x68) = *(unsigned int *)(pcVar11 + 0x14);
            *(float *)(pcVar11 + 100) = *pfVar1;
            *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(pcVar11 + 0x18);
            *pfVar3 = (*(float *)(param_7 + 0x10) - *pfVar1) * _DAT_00552a00;
            *(float *)(pcVar11 + 0x50) =
                (*(float *)(param_7 + 0x14) - *(float *)(pcVar11 + 0x14)) * _DAT_00552a00;
            *(float *)(pcVar11 + 0x54) =
                (*(float *)(param_7 + 0x18) - *(float *)(pcVar11 + 0x18)) * _DAT_00552a00;
            return iVar6;
        }
        return iVar6;
    }
    case 1177: case 1205: case 1206: {
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 12;
        uVar8 = _rand() % 4;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 8) * _DAT_00552874;
        iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x168);
        Matrix_BuildFromEuler(param_3, local_30);
        local_3c = 0.0f;
        uVar8 = _rand() % 16;
        local_38 = -(float)(int)(uVar8 + 8);
        iVar7 = _rand(); local_34 = (float)(iVar7 % 6 + -3);
        Vector_Rotate(&local_3c, local_30, pfVar3);
        if (*(int *)(pcVar11 + 4) == 0x4b6) {
            *(float*)(pcVar11 + ParticleColorOffset) = 0.1f;
            *(float*)(pcVar11 + ParticleColorOffset + 4) = 0.0f;
            *(float*)(pcVar11 + ParticleColorOffset + 8) = 0.0f;
        }
        return iVar6;
    }
    case 1180: {
        uVar8 = _rand() % 8;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 6) * param_6 * _DAT_005524f4;
        iVar7 = _rand();
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;
    }
    case 0x4a7: {
        if (param_5 >= 0 && param_5 < 4) {
            const auto& preset = ParticleScalePresets[param_5];
            if (preset.Lifetime >= 0)
                *(int*)(pcVar11 + ParticleLifetimeOffset) = preset.Lifetime;
            *(float*)(pcVar11 + ParticleScaleOffset) = param_5 == 3 ? param_6 : preset.Scale;
        }
        *(float *)(pcVar11 + 0x28) = *param_4    + _DAT_00552504;
        *(float *)(pcVar11 + 0x2c) = param_4[1]  + _DAT_00552504;
        *(float *)(pcVar11 + 0x30) = param_4[2]  + _DAT_00552504;
        return iVar6;
    }

    case 0x4ab:
    case 0x4ad:
        goto switchD_4ab_impl;

    case 0x4ac:
    case 0x567:
        ApplyParticleLinearMotion(pcVar11, ParticleLinearMotions[param_1 == 0x4ac ? 0 : 1]);
        return iVar6;

    case 0x4b0: {
        switch (param_5) {
        case 0:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 20;
            *pfVar3 = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 4) = 0.0f;
            uVar8 = _rand() % 128;
            *(float *)(pcVar11 + 0x54) = (float)(int)(uVar8 + 0x80) * _DAT_00552934;
            uVar8 = _rand() % 64;
            *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x40) * _DAT_005524f8;
            return iVar6;
        case 1:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 15;
            uVar8 = _rand() % 32;
            *pfVar3 = 0.0f;
            *(float *)(pcVar11 + 0x0c) =
                (float)(int)(uVar8 + 0x20) * _DAT_005524f8 + *(float *)(pcVar11 + 0x0c);
            uVar8 = _rand() % 4;
            break;
        case 2:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 10;
            uVar8 = _rand() % 32;
            *(float *)(pcVar11 + 0x0c) =
                (float)(int)(uVar8 + 0x20) * _DAT_005524f8 + *(float *)(pcVar11 + 0x0c);
            iVar7 = _rand();
            fVar4 = *param_4; *pfVar3 = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            fVar5 = (float)(iVar7 % 0x50) * fVar4 * _DAT_00552594;
            *(float *)(pcVar11 + 0x50) = fVar5;
            Vector_AddRotated(pfVar1, pfVar2, pfVar3);
            fVar4 = *param_4; *pfVar3 = 0.0f;
            fVar4 = (fVar4 - fVar5) * _DAT_00552a48;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            *(float *)(pcVar11 + 0x50) = fVar4;
            fVar13 = (long double)fsin((long double)WorldTime * (long double)_DAT_005528e0);
            fVar13 = fVar13 * (long double)_DAT_005528b8 + (long double)_DAT_00552928;
            *(float *)(pcVar11 + 0x28) = (float)fVar13;
            fVar13 = fVar13 * (long double)_DAT_00552504;
            *(float *)(pcVar11 + 0x2c) = (float)fVar13;
            *(float *)(pcVar11 + 0x30) = (float)fVar13;
            return iVar6;
        case 3:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 10;
            uVar8 = _rand() % 32;
            *pfVar3 = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 4) = 0.0f;
            fVar4 = (float)(int)(uVar8 + 0x20) * _DAT_005524f8;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            *(float *)(pcVar11 + 0x0c) = fVar4 + *(float *)(pcVar11 + 0x0c);
            fVar13 = (long double)fsin((long double)WorldTime * (long double)_DAT_005528e0);
            fVar13 = fVar13 * (long double)_DAT_005528b8 + (long double)_DAT_00552928;
            *(float *)(pcVar11 + 0x28) = (float)fVar13;
            fVar13 = fVar13 * (long double)_DAT_00552504;
            *(float *)(pcVar11 + 0x2c) = (float)fVar13;
            *(float *)(pcVar11 + 0x30) = (float)fVar13;
            return iVar6;
        case 4:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 5;
            uVar8 = _rand() % 32;
            *(float*)(pcVar11 + 0x48) = 10.0f;
            iVar7 = *(int *)(pcVar11 + 0x3c);
            *(float *)(pcVar11 + 0x0c) =
                (float)(int)(uVar8 + 0x20) * _DAT_005524f8 + *(float *)(pcVar11 + 0x0c);
            fVar12 = (long double)WorldTime;
            fVar13 = (long double)_DAT_005528e0;
            *(unsigned int *)(pcVar11 + 100) = *(unsigned int *)(iVar7 + 0x10);
            fVar13 = (long double)fsin(fVar12 * fVar13);
            *(unsigned int *)(pcVar11 + 0x68) = *(unsigned int *)(iVar7 + 0x14);
            *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(iVar7 + 0x18);
            fVar13 = fVar13 * (long double)_DAT_005528b8 + (long double)_DAT_00552928;
            *(float *)(pcVar11 + 0x28) = (float)fVar13;
            fVar13 = fVar13 * (long double)_DAT_00552504;
            *(float *)(pcVar11 + 0x2c) = (float)fVar13;
            *(float *)(pcVar11 + 0x30) = (float)fVar13;
            return iVar6;
        case 5:
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 4;
            *pfVar3 = 0.0f;
            uVar8 = _rand() % 4;
            break;
        default:
            return iVar6;
        }
        fVar4 = (float)(int)(uVar8 + 4) * _DAT_00552934;
        *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
        *(float *)(pcVar11 + 0x50) = fVar4;
        return iVar6;
    }

    case 0x4bf: case 0x4c0: case 0x4ce: case 0x4cf:
    case 0x4d0: case 0x4d2: case 0x596:
        ApplyParticlePreset(pcVar11, param_1, param_5);
        return iVar6;

    case 0x4c5: case 0x4c6: {
        const auto& trajectory = ParticleTrajectories[param_1 - 1221];
        *(int*)(pcVar11 + ParticleLifetimeOffset) = trajectory.Lifetime;
        trajectory.Initialize(pcVar11, param_5, param_3);
        return iVar6;
    }

    case 0x4d9: case 0x4da: case 0x4db: case 0x4dc:
        ApplyParticleDispersion(pcVar11, param_1);
        return iVar6;

    case 0x4df:
        *(float*)(pcVar11 + ParticleAngleOffset + 8) = 0.0f;
        *(float*)(pcVar11 + ParticleAngleOffset + 4) = 0.0f;
        *pfVar2 = 0.0f;
        iVar7 = _rand(); *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        if (param_5 != -1) {
            iVar7 = param_5 + 0xf;
            iVar10 = iVar7 / 2;
            iVar9 = _rand(); *pfVar3                  = (float)(iVar9 % iVar7 - iVar10) * _DAT_00552934;
            iVar9 = _rand(); *(float *)(pcVar11 + 0x50) = (float)(iVar9 % iVar7 - iVar10) * _DAT_00552934;
            iVar9 = _rand();
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 50;
            *(float*)(pcVar11 + ParticleScaleOffset) = 0.1f;
            *(float *)(pcVar11 + 0x54) = (float)(iVar9 % iVar7 - iVar10) * _DAT_00552934 + _DAT_00552488;
            return iVar6;
        }
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 2;
        *(float*)(pcVar11 + ParticleScaleOffset) = 0.25f;
        return iVar6;

    case 0x4e1: {
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 60;
        if ((param_5 == 0) || (param_5 == 3)) {
            iVar7 = _rand();
            fVar4 = (float)(iVar7 % 100) * _DAT_00552594 * _DAT_00552650;
            fVar4 = fVar4 + _DAT_0055256c;
        } else if (param_5 == 4) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 40;
            iVar7 = _rand();
            fVar4 = (float)(iVar7 % 100) * _DAT_00552594 * _DAT_00552660;
            fVar4 = fVar4 + _DAT_0055256c;
        } else if (param_5 == 2) {
            iVar7 = _rand();
            fVar4 = ((float)(iVar7 % 100) * _DAT_00552594 + _DAT_0055256c) * _DAT_00552660;
        } else {
            goto lab_4e1_skip_size;
        }
        *(float *)(pcVar11 + 0x48) = fVar4;
lab_4e1_skip_size:
        iVar7 = _rand(); *pfVar3 = (float)(iVar7 % 300 + -0x96);
        iVar7 = _rand(); *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 6) * _DAT_005524f8 + param_6;
        iVar7 = _rand();
        *pfVar2 = 0.0f;
        *(float*)(pcVar11 + ParticleAngleOffset + 4) = 0.0f;
        *(float*)(pcVar11 + ParticleAngleOffset + 8) = 0.0f;
        *(unsigned int *)(pcVar11 + 0x68) = *(unsigned int *)(pcVar11 + 0x14);
        *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(pcVar11 + 0x18);
        *(float *)(pcVar11 + 100) = *pfVar1;
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;
    }

    case 0x4f4:
        if (param_5 != 6) {
            *(int*)(pcVar11 + ParticleLifetimeOffset) = 30;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 1000);
            iVar7 = _rand(); *pfVar1 = (float)(iVar7 % 500 + -0xfa) + *pfVar1;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x14) = (float)(iVar7 % 500 + -0xfa) + *(float *)(pcVar11 + 0x14);
            iVar7 = _rand(); *(float *)(pcVar11 + 0x18) = (float)(iVar7 % 0x14 + 0x14) + *(float *)(pcVar11 + 0x18);
            iVar7 = _rand();
            *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(pcVar11 + 0x18);
            *(float *)(pcVar11 + 0x68) = (float)(iVar7 % 100) * _DAT_00552594;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 0x14 + 0xb4) * _DAT_005524f8;
            iVar7 = _rand();
            *pfVar3 = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 4) = 0.0f;
            *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
            *(float *)(pcVar11 + 0x58) = (float)(iVar7 % 0x1e) * _DAT_00552594 + param_6;
            return iVar6;
        }
        InitializeRotatingImpulse(pcVar11, 0.2f);
        return iVar6;

    case 0x565:
        InitializeParticleImpulse(pcVar11, param_5);
        return iVar6;

    case 0x566:
        InitializeRotatingImpulse(pcVar11, 0.5f);
        return iVar6;

    case 0x598:
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 5;
        *(float*)(pcVar11 + 0x48) = 0.0f;
        *(float*)(pcVar11 + ParticleScaleOffset) = 4.0f;
        if (param_5 >= 0 && param_5 < 3)
            *(float*)(pcVar11 + ParticleRotationOffset) = ParticleFixedRotations[param_5];
        return iVar6;

    case 0x599:
        *(int*)(pcVar11 + ParticleLifetimeOffset) = 7;
        iVar7 = _rand();
        *(float*)(pcVar11 + ParticleScaleOffset) = 1.2f;
        *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 0x32 + 0x96) * _DAT_00552594;
        iVar7 = _rand();
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;

    default:
        return iVar6;
    }

    // ── 0x4ab/0x4ad shared implementation ──────────────────────────────
switchD_4ab_impl:
    *(int*)(pcVar11 + ParticleLifetimeOffset) = 24;
    *pfVar3 = 0.0f;
    uVar8 = _rand() % 16;
    *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
    iVar7 = *(int *)(pcVar11 + 8);
    *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 0x20) * _DAT_005529c8;
    if ((iVar7 == 0) || (iVar7 == 5)) {
        uVar8 = _rand() % 64;
        fVar4 = (float)(int)(uVar8 + 0x80) * _DAT_005524f8;
    } else if (iVar7 == 7) {
        *pfVar3 = 0.0f;
        uVar8 = _rand() % 32;
        *(float*)(pcVar11 + ParticleDriftOffset + 8) = 0.0f;
        *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 - 0x10) * _DAT_005529c8;
        uVar8 = _rand() % 64;
        fVar4 = (float)(int)(uVar8 + 0x80) * _DAT_00552ac0;
    } else if (iVar7 == 9) {
        iVar7 = _rand();
        int s = iVar7 % 0x3c - 0x1e;
        *(float *)(pcVar11 + 100) = (float)(int)s;
        *(float *)(pcVar11 + 0x68) = (float)(int)s;
        *(float *)(pcVar11 + 0x6c) =
            _DAT_00552974 -
            (float)(int)((s ^ (s >> 0x1f)) - (s >> 0x1f)) * _DAT_005528f0;
        goto lab_4ab_setscale;
    } else {
        goto lab_4ab_setscale;
    }
    *(float *)(pcVar11 + 0x0c) = fVar4;
lab_4ab_setscale:
    if (*(int *)(pcVar11 + 8) == 1) {
        uVar8 = _rand() % 4;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 10) * _DAT_005524f8;
    }
    iVar7 = _rand();
    *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
    return iVar6;
}
