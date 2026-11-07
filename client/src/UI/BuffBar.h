#pragma once
#include <cstddef>

// DESVIACION: barra de buffs del personaje. El 0.97k no la tiene; el server
// manda cada efecto propio por F3/F1 (tiempo, valores y quién lo lanzó) y acá
// se dibuja arriba al centro con el ícono del skill, con el detalle al pasar
// el mouse.
class CBuffBar {
public:
    void Receive(const unsigned char* packet, int size);
    void Clear();
    void Render();

private:
    struct Buff {
        unsigned char effect = 0;
        unsigned short skill = 0;
        unsigned long seconds = 0;     // al recibirlo; 0 = sin límite
        unsigned long receivedAt = 0;
        unsigned short value[4] = {};
        char caster[11] = {};
        char name[33] = {};
    };
    static constexpr int MaxBuffs = 16;
    unsigned long Remaining(const Buff& buff) const;
    void RenderTooltip(const Buff& buff, int x, int y) const;
    Buff m_Buffs[MaxBuffs];
    int m_Count = 0;
};
extern CBuffBar gBuffBar;
