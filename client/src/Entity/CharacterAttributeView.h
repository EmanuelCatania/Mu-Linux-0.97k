#pragma once
#include <windows.h>
#include <cstring>

// IDA: ReceiveJoinMapServer (0x00425840), atributos del personaje local.
// Adaptador mínimo: el paquete conserva DWORD; sólo el layout legado usa WORD.
class CharacterAttributeView {
public:
    explicit CharacterAttributeView(void* data) : m_Data((BYTE*)data) {}
    const char* Name() const { return (const char*)m_Data; }
    WORD Level() const { return ReadWord(0x0E); }
    WORD PhysicalSpeed() const { return ReadWord(0x38); }
    WORD MagicSpeed() const { return ReadWord(0x44); }
    BYTE Effects() const { return m_Data[0x28]; }
    void SetPhysicalSpeed(DWORD value) { WriteWord(0x38, value); }
    void SetMagicSpeed(DWORD value) { WriteWord(0x44, value); }
    void SetAttackSuccessRate(DWORD value) { WriteWord(0x3A, value); }
    void SetDefense(DWORD value) { WriteWord(0x4E, value); }
    void SetDefenseSuccessRate(DWORD value) { WriteWord(0x4C, value); }
    void SetMagicDamageMin(DWORD value) { WriteWord(0x46, value); }
    void SetMagicDamageMax(DWORD value) { WriteWord(0x48, value); }
private:
    WORD ReadWord(size_t offset) const {
        WORD value; memcpy(&value, m_Data + offset, sizeof(value)); return value;
    }
    void WriteWord(size_t offset, DWORD value) {
        const WORD bounded = value > 0xFFFFu ? 0xFFFFu : (WORD)value;
        memcpy(m_Data + offset, &bounded, sizeof(bounded));
    }
    BYTE* m_Data;
};
