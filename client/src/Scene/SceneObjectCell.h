#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

// IDA 0x004FF5A0/0x004FFCC0: celda de escena, fuera de disco y protocolo.
// El DWORD inicial no se reutiliza como puntero. Visible ocupa un solo byte.
#pragma pack(push, 1)
struct SceneObjectCell {
    std::uint32_t Reserved;
    void* Head;
    void* Tail;
    std::uint8_t Visible;
    std::uint8_t Padding[3];
};
#pragma pack(pop)

static_assert(std::is_standard_layout_v<SceneObjectCell>);
static_assert(std::is_trivially_copyable_v<SceneObjectCell>);
static_assert(alignof(SceneObjectCell) == 1);
static_assert(offsetof(SceneObjectCell, Reserved) == 0);
static_assert(offsetof(SceneObjectCell, Head) == 4);
static_assert(offsetof(SceneObjectCell, Tail) == 4 + sizeof(void*));
static_assert(offsetof(SceneObjectCell, Visible) == 4 + 2 * sizeof(void*));
static_assert(sizeof(SceneObjectCell) == 8 + 2 * sizeof(void*));
static_assert(sizeof(void*) != 4 || sizeof(SceneObjectCell) == 16);
static_assert(sizeof(void*) != 8 || sizeof(SceneObjectCell) == 24);
