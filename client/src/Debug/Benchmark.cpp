// Benchmark.cpp — modo benchmark de render.  Ver Debug/Benchmark.h.

#include "stdafx.h"
#include "Debug/Benchmark.h"

#if defined(MU_BENCHMARK)

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

#include "Entity/EntityTypes.h"
#include "Model/ModelTypes.h"
#include "Net/NetText.h"
#include "Net/Recv/NetRecv.h"
#include "Platform/IInput.h"
#include "globals.h"
#include "functions.h"
#include "Game/MapManager.h"

namespace Benchmark {
namespace {

constexpr int kKeyBase   = 0x6000;   // lejos de los índices que reparte el server
constexpr int kMaxCount  = 350;      // el pool tiene 400 slots; dejamos lugar a lo real
constexpr int kNameSize  = (int)NetText::CharacterNameSize;
constexpr int kEntrySize = 26 + kNameSize;   // PMSG_VIEWPORT_PLAYER (protocolo 3: + velocidades)

struct Bot {
    WORD key;
    int  anchorX, anchorY;   // casilla alrededor de la que camina
    int  cls;                // 0 DW, 1 DK, 2 Elf, 3 MG
    int  nextAction;         // pasos de simulación hasta la próxima acción
};

std::vector<Bot> g_bots;
bool g_active = false;
bool g_skills = true;
int  g_world  = -1;
int  g_casts  = 0;           // skills lanzados en el último segundo
int  g_castsShown = 0;
int  g_tickCount = 0;
char g_status[128];

int EnvInt(const char* name, int def, int lo, int hi)
{
    const char* v = std::getenv(name);
    if (!v || !*v) return def;
    int n = std::atoi(v);
    return n < lo ? lo : (n > hi ? hi : n);
}

int Rand(int n) { return n > 0 ? std::rand() % n : 0; }

bool ModelLoaded(int type)
{
    if (!DAT_05828d58 || type < 0) return false;
    const BMD* m = AsModel(DAT_05828d58 + (std::uintptr_t)type * BMD_STRIDE);
    return m->NumMeshs > 0;
}

// Ítem del CharSet (índice = sección*32 + número) cuyo modelo esté cargado.
int PickItem(int section, int maxIndex)
{
    for (int tries = 0; tries < 16; ++tries) {
        const int idx = section * 32 + Rand(maxIndex + 1);
        if (idx < 256 && ModelLoaded(idx + 400)) return idx;
    }
    return 0xFF;
}

// Índice de set (0..30) cuyas cinco piezas tengan modelo; 31 = el cuerpo por
// defecto de la clase.
int PickSet()
{
    static std::vector<int> valid;
    if (valid.empty()) {
        for (int i = 0; i < 31; ++i) {
            bool ok = true;
            for (int part = 0; part < 5 && ok; ++part)
                ok = ModelLoaded(624 + part * 32 + i);
            if (ok) valid.push_back(i);
        }
    }
    if (valid.empty() || Rand(10) == 0) return 31;
    return valid[Rand((int)valid.size())];
}

// Nivel en el código de 3 bits del CharSet; sesgado a +7..+11 para ver glows.
int PickLevelCode() { return Rand(3) == 0 ? Rand(8) : 5 + Rand(3); }

bool Walkable(int x, int y)
{
    if (x < 1 || y < 1 || x > 254 || y > 254 || !TerrainWall) return false;
    return (TerrainWall[x + y * 256] & 0x0C) == 0;   // sin NOMOVE ni NOGROUND
}

BYTE* FindSlot(WORD key)
{
    BYTE* base = (BYTE*)CharactersClient;
    if (!base) return nullptr;
    for (int s = 0; s < 400; ++s) {
        BYTE* c = base + s * CHARACTER_STRIDE;
        if (c[0] && AsCharacter(c)->Key == key) return c;
    }
    return nullptr;
}

void BuildCharSet(int cls, BYTE* cs /* 11 bytes */)
{
    const int evo = (cls != 3 && Rand(2)) ? 1 : 0;
    cs[0] = (BYTE)((cls << 5) | (evo << 4));
    BYTE* eq = cs + 1;                         // los 10 bytes de ChangeCharacterExt
    std::memset(eq, 0, 10);

    int right = 0xFF, left = 0xFF;
    switch (cls) {
    case 0: right = PickItem(5, 8);  if (Rand(2)) left = PickItem(6, 14); break; // bastón + escudo
    case 1: right = PickItem(0, 15); left = Rand(2) ? PickItem(6, 14) : PickItem(1, 8); break;
    case 2: left = PickItem(4, 6);   right = PickItem(4, 15);  break;           // arco + flechas
    default: right = PickItem(0, 15); if (Rand(2)) left = PickItem(5, 8); break;
    }
    eq[0] = (BYTE)right;
    eq[1] = (BYTE)left;

    // Un solo set casi siempre (para que salte el brillo de set completo) y a
    // veces piezas mezcladas.
    int parts[5];
    const int set = PickSet();
    for (int p = 0; p < 5; ++p) parts[p] = (Rand(5) == 0) ? PickSet() : set;
    if (cls == 3) parts[0] = 31;               // el MG no usa casco

    eq[2] = (BYTE)(((parts[0] & 0xF) << 4) | (parts[1] & 0xF));
    eq[3] = (BYTE)(((parts[2] & 0xF) << 4) | (parts[3] & 0xF));
    int wing   = Rand(4);                      // 0..2 alas, 3 = ninguna
    int helper = Rand(4);                      // 0 Angel, 1 Imp, 2 Uniria, 3 nada/Dinorant
    eq[4] = (BYTE)(((parts[4] & 0xF) << 4) | (wing << 2) | helper);
    eq[8] = (BYTE)(((parts[0] >> 4) << 7) | ((parts[1] >> 4) << 6) | ((parts[2] >> 4) << 5) |
                   ((parts[3] >> 4) << 4) | ((parts[4] >> 4) << 3));
    if (wing == 3 && cls == 3 && Rand(2)) eq[8] |= 4;   // ala del MG (790)
    if (helper == 3 && Rand(2)) eq[9] |= 1;             // Dinorant

    // Niveles: 3 bits por pieza, armas y luego casco..botas.
    const int setLevel = PickLevelCode();
    int level = PickLevelCode() | (PickLevelCode() << 3);
    for (int p = 0; p < 5; ++p)
        level |= ((Rand(4) == 0) ? PickLevelCode() : setLevel) << (6 + 3 * p);
    eq[5] = (BYTE)(level >> 16);
    eq[6] = (BYTE)(level >> 8);
    eq[7] = (BYTE)level;
    eq[9] |= (BYTE)(Rand(3) == 0 ? (Rand(256) & 0xFE) : 0);  // flags de excelente
}

void SendViewport(const Bot& b, int x, int y)
{
    BYTE msg[5 + kEntrySize] = {};
    const int size = (int)sizeof(msg);
    msg[0] = 0xC2;
    msg[1] = (BYTE)(size >> 8);
    msg[2] = (BYTE)size;
    msg[3] = 0x12;
    msg[4] = 1;
    BYTE* e = msg + 5;
    e[0] = (BYTE)(b.key >> 8);
    e[1] = (BYTE)b.key;
    e[2] = (BYTE)x;
    e[3] = (BYTE)y;
    BuildCharSet(b.cls, e + 4);
    char name[kNameSize + 1] = {};
    std::snprintf(name, sizeof(name), "Bench%03d", b.key - kKeyBase);
    std::memcpy(e + 18, name, std::strlen(name));
    e[18 + kNameSize] = (BYTE)x;               // TargetX
    e[19 + kNameSize] = (BYTE)y;               // TargetY
    e[20 + kNameSize] = (BYTE)((1 + Rand(8)) << 4);
    NetRecv_12(msg, size, 0xC2, 0, false);
}

void SendMove(const Bot& b, int x, int y)
{
    BYTE msg[8] = { 0xC1, 8, 0x10, (BYTE)(b.key >> 8), (BYTE)b.key, (BYTE)x, (BYTE)y, 0 };
    NetRecv_10(msg, (int)sizeof(msg), 0xC1, 0, false);
}

// Skills por clase.  Los de un objetivo van por 0x19; los de área por 0x1E.
struct Skill { BYTE id; bool area; };
const Skill kDW[]  = { {1,0},{2,0},{3,0},{4,0},{7,0},{17,0},{5,1},{8,1},{9,1},{10,1},{12,1},{13,1},{14,1} };
const Skill kDK[]  = { {19,0},{20,0},{21,0},{22,0},{23,0},{41,1},{42,1},{43,1} };
const Skill kElf[] = { {26,0},{27,0},{28,0},{51,0},{24,1},{52,1} };
const Skill kMG[]  = { {3,0},{4,0},{9,1},{10,1},{55,1},{56,1} };

void CastSkill(const Bot& b, BYTE* caster)
{
    const Skill* list; int n;
    switch (b.cls) {
    case 0:  list = kDW;  n = (int)(sizeof(kDW)  / sizeof(kDW[0]));  break;
    case 1:  list = kDK;  n = (int)(sizeof(kDK)  / sizeof(kDK[0]));  break;
    case 2:  list = kElf; n = (int)(sizeof(kElf) / sizeof(kElf[0])); break;
    default: list = kMG;  n = (int)(sizeof(kMG)  / sizeof(kMG[0]));  break;
    }
    const Skill s = list[Rand(n)];
    const int x = AsCharacter(caster)->PositionX;
    const int y = AsCharacter(caster)->PositionY;
    if (s.area) {
        const BYTE dir = (BYTE)Rand(256);
        BYTE msg[9] = { 0xC1, 9, 0x1E, s.id, (BYTE)(b.key >> 8), (BYTE)b.key, (BYTE)x, (BYTE)y, dir };
        NetRecv_1E(msg, (int)sizeof(msg), 0xC1, 0, false);
    } else {
        // Objetivo: otro personaje de prueba (o uno mismo si es un buff).
        WORD target = b.key;
        if (s.id != 26 && s.id != 27 && s.id != 28 && g_bots.size() > 1)
            target = g_bots[Rand((int)g_bots.size())].key;
        const WORD t = (WORD)(target | 0x8000);    // bit 15 = skill exitoso
        BYTE msg[10] = { 0xC1, 10, 0x19, s.id, (BYTE)(b.key >> 8), (BYTE)b.key,
                         (BYTE)(t >> 8), (BYTE)t, 0, 0 };
        NetRecv_19(msg, (int)sizeof(msg), 0xC1, 0, false);
    }
    ++g_casts;
}

void Spawn()
{
    BYTE* hero = (BYTE*)Hero;
    if (!hero || !CharactersClient) return;
    const int count  = EnvInt("MU_BENCH_COUNT", 100, 1, kMaxCount);
    const int radius = EnvInt("MU_BENCH_RADIUS", 10, 2, 60);
    const int hx = AsCharacter(hero)->PositionX;
    const int hy = AsCharacter(hero)->PositionY;

    g_bots.clear();
    for (int i = 0; i < count; ++i) {
        int x = hx, y = hy;
        for (int tries = 0; tries < 40; ++tries) {
            const int cx = hx + Rand(radius * 2 + 1) - radius;
            const int cy = hy + Rand(radius * 2 + 1) - radius;
            if (Walkable(cx, cy)) { x = cx; y = cy; break; }
        }
        Bot b{ (WORD)(kKeyBase + i), x, y, Rand(4), 10 + Rand(50) };
        g_bots.push_back(b);
        SendViewport(b, x, y);
    }
    g_world  = gMapManager.GetCurrentMap();
    g_active = true;
}

void Despawn()
{
    for (const Bot& b : g_bots) {
        BYTE msg[6] = { 0xC1, 6, 0x14, 1, (BYTE)(b.key >> 8), (BYTE)b.key };
        NetRecv_14(msg, (int)sizeof(msg), 0xC1, 0, false);
    }
    g_bots.clear();
    g_active = false;
}

} // namespace

bool Active() { return g_active; }

const char* Status()
{
    std::snprintf(g_status, sizeof(g_status), "Benchmark: %d personajes, skills %s (%d/s)",
                  (int)g_bots.size(), g_skills ? "si" : "no", g_castsShown);
    return g_status;
}

void Tick()
{
    if (PressKey(VK_F9)) {
        if (Platform::GetInput().KeyState(VK_SHIFT) & 0x8000) g_skills = !g_skills;
        else if (g_active) Despawn();
        else Spawn();
    }
    if (!g_active) return;

    // Cambio de mapa o relog: el pool de entidades ya se vació.
    if (!Hero || gMapManager.GetCurrentMap() != g_world) {
        g_bots.clear();
        g_active = false;
        return;
    }

    if (++g_tickCount >= 25) { g_tickCount = 0; g_castsShown = g_casts; g_casts = 0; }

    const int radius = EnvInt("MU_BENCH_RADIUS", 10, 2, 60);
    for (Bot& b : g_bots) {
        if (--b.nextAction > 0) continue;
        BYTE* c = FindSlot(b.key);
        if (!c) { b.nextAction = 25; continue; }
        CHARACTER* ch = AsCharacter(c);

        const bool canCast = g_skills && !ch->SafeZone && ch->Movement == 0;
        if (canCast && Rand(3) != 0) {
            CastSkill(b, c);
            b.nextAction = 20 + Rand(40);      // 0.8 a 2.4 s
            continue;
        }
        const int half = radius / 2 + 2;
        for (int tries = 0; tries < 10; ++tries) {
            const int tx = b.anchorX + Rand(half * 2 + 1) - half;
            const int ty = b.anchorY + Rand(half * 2 + 1) - half;
            if (Walkable(tx, ty)) { SendMove(b, tx, ty); break; }
        }
        b.nextAction = 40 + Rand(60);          // 1.6 a 4 s
    }
}

} // namespace Benchmark

#endif // MU_BENCHMARK
