#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Registro runtime ITEM: mismo contrato de 68 bytes en x86 y x64.
// No es el registro de item.bmd ni un paquete de red.
constexpr std::size_t kItemSpecialOptionCount = 9;

struct ITEM  // size = 68
{
    /*+00*/ std::int16_t  Type;
    /*+04*/ std::int32_t    Level;
    /*+08*/ std::uint8_t   Part;
    /*+09*/ std::uint8_t   Class;
    /*+10*/ std::uint8_t   TwoHand;
    /*+12*/ std::uint16_t   DamageMin;
    /*+14*/ std::uint16_t   DamageMax;
    /*+16*/ std::uint8_t   SuccessfulBlocking;
    /*+18*/ std::uint16_t   Defense;
    /*+20*/ std::uint16_t   MagicDefense;
    /*+22*/ std::uint8_t   WeaponSpeed;
    /*+24*/ std::uint16_t   WalkSpeed;
    /*+26*/ std::uint8_t   Durability;
    /*+27*/ std::uint8_t   Option1;
    /*+28*/ std::uint16_t   RequireStrength;
    /*+30*/ std::uint16_t   RequireDexterity;
    /*+32*/ std::uint16_t   RequireEnergy;
    /*+34*/ std::uint16_t   RequireLevel;
    /*+36*/ std::uint8_t   SpecialNum;
    /*+37*/ std::uint8_t   Special[kItemSpecialOptionCount];
    /*+46*/ std::uint8_t   SpecialValue[kItemSpecialOptionCount];
    /*+56*/ std::uint32_t  Key;
    /*+60*/ std::uint8_t   Unknown;
    /*+61*/ std::uint8_t   byColorState;
    /*+62*/ std::uint8_t   x;
    /*+63*/ std::uint8_t   y;
    /*+64*/ std::uint8_t   Color;
};
static_assert(sizeof(ITEM) == 68, "ITEM");
static_assert(offsetof(ITEM, Type) == 0, "ITEM::Type");
static_assert(offsetof(ITEM, Level) == 4, "ITEM::Level");
static_assert(offsetof(ITEM, Part) == 8, "ITEM::Part");
static_assert(offsetof(ITEM, Class) == 9, "ITEM::Class");
static_assert(offsetof(ITEM, TwoHand) == 10, "ITEM::TwoHand");
static_assert(offsetof(ITEM, DamageMin) == 12, "ITEM::DamageMin");
static_assert(offsetof(ITEM, DamageMax) == 14, "ITEM::DamageMax");
static_assert(offsetof(ITEM, SuccessfulBlocking) == 16, "ITEM::SuccessfulBlocking");
static_assert(offsetof(ITEM, Defense) == 18, "ITEM::Defense");
static_assert(offsetof(ITEM, MagicDefense) == 20, "ITEM::MagicDefense");
static_assert(offsetof(ITEM, WeaponSpeed) == 22, "ITEM::WeaponSpeed");
static_assert(offsetof(ITEM, WalkSpeed) == 24, "ITEM::WalkSpeed");
static_assert(offsetof(ITEM, Durability) == 26, "ITEM::Durability");
static_assert(offsetof(ITEM, Option1) == 27, "ITEM::Option1");
static_assert(offsetof(ITEM, RequireStrength) == 28, "ITEM::RequireStrength");
static_assert(offsetof(ITEM, RequireDexterity) == 30, "ITEM::RequireDexterity");
static_assert(offsetof(ITEM, RequireEnergy) == 32, "ITEM::RequireEnergy");
static_assert(offsetof(ITEM, RequireLevel) == 34, "ITEM::RequireLevel");
static_assert(offsetof(ITEM, SpecialNum) == 36, "ITEM::SpecialNum");
static_assert(offsetof(ITEM, Special) == 37, "ITEM::Special");
static_assert(offsetof(ITEM, SpecialValue) == 46, "ITEM::SpecialValue");
static_assert(offsetof(ITEM, Key) == 56, "ITEM::Key");
static_assert(offsetof(ITEM, Unknown) == 60, "ITEM::Unknown");
static_assert(offsetof(ITEM, byColorState) == 61, "ITEM::byColorState");
static_assert(offsetof(ITEM, x) == 62, "ITEM::x");
static_assert(offsetof(ITEM, y) == 63, "ITEM::y");
static_assert(offsetof(ITEM, Color) == 64, "ITEM::Color");
static_assert(alignof(ITEM) == 4, "Alineacion de ITEM");
static_assert(std::is_standard_layout<ITEM>::value, "Layout estandar de ITEM");
static_assert(std::is_trivially_copyable<ITEM>::value, "ITEM copiable por bytes");
