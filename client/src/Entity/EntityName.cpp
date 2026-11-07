#include "stdafx.h"
#include "globals.h"
#include "Entity/EntityName.h"

#include "Entity/EntityTypes.h"
#include "Text/TextCore.h"

#include <cstdint>
#include <cstring>

namespace EntityName {

namespace {

constexpr std::size_t PoolSize = 400;
constexpr std::size_t Stride = sizeof(CHARACTER);
constexpr std::size_t IdOffset = 0x1C1;
constexpr std::size_t IdCapacity = 0x1DA - 0x1C1;   // hasta GuildMarkIndex

char s_Names[PoolSize][Capacity];
char s_Selected[Capacity];

int SlotOf(const void* character)
{
    if (!character || !CharactersClient) return -1;
    const std::uintptr_t base = (std::uintptr_t)CharactersClient;
    const std::uintptr_t p = (std::uintptr_t)character;
    if (p < base) return -1;
    const std::uintptr_t offset = p - base;
    if (offset % Stride != 0 || offset / Stride >= PoolSize) return -1;
    return (int)(offset / Stride);
}

}

const char* Get(const void* character)
{
    const int slot = SlotOf(character);
    if (slot >= 0 && s_Names[slot][0]) return s_Names[slot];
    return character ? (const char*)character + IdOffset : "";
}

void Set(void* character, const char* text, std::size_t length)
{
    if (!character) return;
    std::size_t used = 0;
    while (text && used < length && text[used]) ++used;
    const std::string_view value(text ? text : "", used);
    TextEngine::CopyTruncated((char*)character + IdOffset, IdCapacity, value);
    const int slot = SlotOf(character);
    if (slot >= 0) TextEngine::CopyTruncated(s_Names[slot], Capacity, value);
}

void Set(void* character, const char* text)
{
    Set(character, text, text ? std::strlen(text) : 0);
}

void Clear(void* character)
{
    if (!character) return;
    std::memset((char*)character + IdOffset, 0, IdCapacity);
    const int slot = SlotOf(character);
    if (slot >= 0) s_Names[slot][0] = 0;
}

void SetSelected(const char* name)
{
    TextEngine::CopyTruncated(s_Selected, Capacity, name ? name : "");
}

const char* Selected()
{
    return s_Selected;
}

}
