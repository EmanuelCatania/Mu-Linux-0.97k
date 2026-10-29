#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

// IDA: CreatePoint 0x004792C0, MovePoints y RenderPoints 0x00479330.
// El pool no contiene punteros: conserva el mismo contrato en x86 y x64.
struct DamagePoint {
    std::uint8_t Active;
    std::uint8_t Reserved001[3];
    std::int32_t Value;
    std::uint8_t Reserved008[4];
    float Scale;
    float Position[3];
    float Color[3];
    std::uint8_t Reserved028[16];
    std::int32_t Delay;
    std::uint8_t Reserved03C[12];
    float LifeTime;
    std::uint8_t Reserved04C[36];
};

constexpr std::size_t kDamagePointCount = 100;
static_assert(sizeof(void*) == 4 || sizeof(void*) == 8, "Arquitectura de punteros admitida");
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559, "Float IEEE de32 bits");
static_assert(std::is_standard_layout<DamagePoint>::value, "Layout estandar de numeros de dano");
static_assert(std::is_trivially_copyable<DamagePoint>::value, "Pool copiable por bytes");
static_assert(alignof(DamagePoint) == 4, "Alineacion de numeros de dano");
static_assert(sizeof(DamagePoint) == 0x70, "Stride comun x86/x64");
static_assert(offsetof(DamagePoint, Active) == 0x00, "Active");
static_assert(offsetof(DamagePoint, Value) == 0x04, "Value");
static_assert(offsetof(DamagePoint, Scale) == 0x0C, "Scale");
static_assert(offsetof(DamagePoint, Position) == 0x10, "Position");
static_assert(offsetof(DamagePoint, Color) == 0x1C, "Color");
static_assert(offsetof(DamagePoint, Delay) == 0x38, "Delay entero");
static_assert(offsetof(DamagePoint, LifeTime) == 0x48, "LifeTime");
static_assert(sizeof(DamagePoint[kDamagePointCount]) == 0x2BC0, "Reserva comun x86/x64");
