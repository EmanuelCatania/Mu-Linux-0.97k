#pragma once
#include <windows.h>
#include <cstring>

// Vista mínima del layout legado (0x394); no posee ni modifica la entidad.
// IDA: RenderMonsterName (0x004CB6F0), ClearCharacters (layout del viewport).
class EntityView {
public:
    static constexpr int Capacity = 400;
    static constexpr int Stride = 0x394;
    static constexpr BYTE PlayerKind = 1;
    static constexpr BYTE MonsterKind = 2;
    static constexpr WORD SoccerBall = 200;
    explicit EntityView(const void* entity) : m_Data((const BYTE*)entity) {}
    bool IsActive() const { return m_Data && m_Data[0] != 0; }
    BYTE Kind() const { return m_Data[0x84]; }
    WORD ModelType() const { return Read<WORD>(0x2EB); }
    WORD NetworkKey() const { return Read<WORD>(0x1DC); }
    const char* Name() const { return (const char*)(m_Data + 0x1C1); }
    float X() const { return Read<float>(0x10); }
    float Y() const { return Read<float>(0x14); }
    float Z() const { return Read<float>(0x18); }
    float Height() const { return Read<float>(0x12C); }
private:
    template<typename T> T Read(size_t offset) const {
        T value; memcpy(&value, m_Data + offset, sizeof(value)); return value;
    }
    const BYTE* m_Data;
};
