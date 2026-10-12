#pragma once
// HeroVitals.h — vida, mana y AG del héroe con 32 bits.
//
// CharacterAttribute guarda Life/MaxLife/Mana/MaxMana/BP/MaxBP como WORD (el
// 0.97k no pasaba de 65535).  MuEmu con GAMESERVER_EXTRA manda además los
// valores completos (View*) en F3/E0, F3/E1, F3/04-07, 0x15, 0x26 y 0x27.
// DESVIACION (DLL PrintPlayer.cpp): el HUD los usa para que vida, mana y AG
// grandes se vean bien.  Los handlers siguen escribiendo el WORD recortado.
//
// Si el WORD de CharacterAttribute ya no coincide con el valor guardado (lo
// cambió un camino sin View*), manda el WORD: nunca se muestra un dato viejo.

#include <windows.h>

enum eHeroVital { VITAL_LIFE, VITAL_MANA, VITAL_AG, MAX_HERO_VITAL };

class CHeroVitals {
public:
    void Reset();
    void SetCurrent(eHeroVital vital, DWORD value);
    void SetMax(eHeroVital vital, DWORD value);
    DWORD GetCurrent(eHeroVital vital) const;
    DWORD GetMax(eHeroVital vital) const;

private:
    DWORD Resolve(DWORD value, bool known, int offset) const;
    DWORD m_Current[MAX_HERO_VITAL] = {};
    DWORD m_Max[MAX_HERO_VITAL] = {};
    bool  m_HasCurrent[MAX_HERO_VITAL] = {};
    bool  m_HasMax[MAX_HERO_VITAL] = {};
};

extern CHeroVitals gHeroVitals;
