#include "stdafx.h"
#include "Item/ContentCatalog.h"
#include "Item/ItemDefines.h"
#include "Net/Protocol/GameServerProtocol.h"
#include <map>
#include <string>
#include <vector>

extern "C" void DbgLogPublic(const char* msg);

CContentCatalog gContentCatalog;

namespace {
constexpr int MAX_CATALOG_MONSTER = 512;
constexpr int MAX_CATALOG_MAP = 64;

struct PendingSection
{
    std::vector<BYTE> records;
    int recordSize = 0;
    int received = 0;
    int total = -1;
};

PendingSection s_PendingItems;
PendingSection s_PendingMonsters;
PendingSection s_PendingFog;
PendingSection s_PendingEffects;
PendingSection s_PendingPets;
PendingSection s_PendingTooltips;
std::map<int, std::vector<Proto::CATALOG_TOOLTIP>> s_Tooltips;   // item -> líneas
std::map<int, Proto::CATALOG_PET> s_Pets;           // item -> pet

// Efecto ya resuelto (textura cargada) de un item.
struct ItemEffect
{
    Proto::CATALOG_EFFECT Data;
    int Bitmap;          // slot de textura a usar
};
std::map<int, std::vector<ItemEffect>> s_Effects;   // item -> efectos
std::map<std::string, int> s_LoadedTextures;        // ruta -> slot

std::vector<BYTE> s_VanillaAttributes;   // item.bmd, para volver a él en Clear()

struct ItemExtraData
{
    bool  Present = false;
    WORD  Behavior = 0;
    bool  HasGlow = false;
    float Glow[3] = {};
    bool  HasPose = false;
    CatalogItemPose Pose = {};
    int   Model = -1;
    bool  HasWing = false;
    CatalogWing Wing = {};
    int   EntityModel = -1;
    int   Slot = -1;
};

// Piezas puestas: por entidad, modelo lógico -> modelo propio.
constexpr int MAX_ENTITY = 400;
constexpr int ENTITY_STRIDE = 0x394;
constexpr int ENTITY_PARTS = 9;
struct EntityPart { short Logic; short Own; };
EntityPart s_EntityParts[MAX_ENTITY][ENTITY_PARTS];
bool s_EntityPartsInit = false;

int EntityIndex(const void* c)
{
    const uintptr_t base = (uintptr_t)CharactersClient;
    const uintptr_t ptr = (uintptr_t)c;
    if (!base || ptr < base) return -1;
    const uintptr_t off = ptr - base;
    if (off % ENTITY_STRIDE != 0 || off / ENTITY_STRIDE >= MAX_ENTITY) return -1;
    return (int)(off / ENTITY_STRIDE);
}

void InitEntityParts()
{
    if (s_EntityPartsInit) return;
    for (EntityPart (&row)[ENTITY_PARTS] : s_EntityParts)
        for (EntityPart& part : row) part = { -1, -1 };
    s_EntityPartsInit = true;
}

std::vector<ItemExtraData> s_Items(ITEM_MAX_EX);
CatalogMonster s_Monsters[MAX_CATALOG_MONSTER] = {};
struct FogEntry { bool Present; float Day[3]; float Night[3]; };
FogEntry s_Fog[MAX_CATALOG_MAP] = {};
std::map<std::string, int> s_LoadedModels;   // ruta -> slot
std::map<int, int> s_DynamicModelItem;        // slot dinámico -> primer item que lo usa

ITEM_ATTRIBUTE* Attributes()
{
    const uintptr_t base = (uintptr_t)DAT_07d78068;
    return (base >= 0x100000u && base < 0x80000000u) ? (ITEM_ATTRIBUTE*)base : nullptr;
}

BYTE ClampByte(int value)
{
    return (BYTE)(value < 0 ? 0 : (value > 255 ? 255 : value));
}

void Log(const char* fmt, ...)
{
    char text[256];
    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(text, sizeof(text), _TRUNCATE, fmt, args);
    va_end(args);
    DbgLogPublic(text);
}

// Una ruta del catálogo: relativa a Data, sin "..", sin unidad ni raíz.  Se
// normalizan las barras dobles que traen los txt del Encoder.
bool NormalizeFolder(const char* in, char* out, size_t size)
{
    size_t n = 0;
    for (const char* p = in; *p && n + 2 < size; ++p) {
        const char ch = (*p == '/') ? '\\' : *p;
        if (ch == '\\' && n > 0 && out[n - 1] == '\\') continue;
        if (!isalnum((unsigned char)ch) && ch != '\\' && ch != '_' && ch != '-' && ch != '.' && ch != ' ')
            return false;
        out[n++] = ch;
    }
    if (n > 0 && out[n - 1] != '\\') out[n++] = '\\';
    out[n] = 0;
    return n > 1 && out[0] != '\\' && strstr(out, "..") == nullptr;
}

bool ValidName(const char* name)
{
    if (!name[0]) return false;
    for (const char* p = name; *p; ++p)
        if (!isalnum((unsigned char)*p) && *p != '_' && *p != '-' && *p != '.') return false;
    return strstr(name, "..") == nullptr;
}

bool ReadSection(PendingSection& section, const BYTE* msg, int size)
{
    if (size < (int)sizeof(Proto::PMSG_CATALOG_HEAD)) return false;
    Proto::PMSG_CATALOG_HEAD head;
    memcpy(&head, msg, sizeof(head));
    if (head.version != Proto::CATALOG_VERSION || head.recordSize == 0) return false;
    const int bytes = head.count * head.recordSize;
    if ((int)sizeof(head) + bytes > size) return false;
    if (head.chunk == 0) {
        section.records.clear();
        section.received = 0;
        section.recordSize = head.recordSize;
        section.total = head.totalChunks;
    }
    if (head.recordSize != section.recordSize || head.chunk != section.received) return false;
    section.records.insert(section.records.end(), msg + sizeof(head), msg + sizeof(head) + bytes);
    ++section.received;
    return true;
}

// Velocidades de animación de un modelo de monstruo del catálogo (DLL
// CustomMonster: OpenMonsterModel / OpenNpcModel).
void SetMonsterSpeeds(int slot, bool npc)
{
    if (slot < 0 || !DAT_05828d58) return;
    BYTE* model = (BYTE*)(uintptr_t)DAT_05828d58 + slot * 0xbc;
    BYTE* actions = *(BYTE**)(model + 0x30);
    const short count = *(short*)(model + 0x26);
    if (!actions || count <= 0) return;
    if (npc) {
        for (int i = 0; i < count; ++i) *(float*)(actions + 16 * i + 4) = 0.25f;
        return;
    }
    static const float speeds[7] = { 0.25f, 0.2f, 0.34f, 0.33f, 0.33f, 0.5f, 0.55f };
    for (int i = 0; i < count && i < 7; ++i) *(float*)(actions + 16 * i + 4) = speeds[i];
    if (count > 6) *(bool*)(actions + 16 * 6) = true;   // MONSTER01_DIE: Loop
}

bool SectionComplete(const PendingSection& section)
{
    return section.total >= 0 && section.received == section.total;
}
} // namespace

void CContentCatalog::ReceiveItems(const BYTE* msg, int size)
{
    if (!ReadSection(s_PendingItems, msg, size)) Log("ContentCatalog: E7 descartado (size=%d)", size);
}

void CContentCatalog::ReceiveMonsters(const BYTE* msg, int size)
{
    if (!ReadSection(s_PendingMonsters, msg, size)) Log("ContentCatalog: E8 descartado (size=%d)", size);
}

void CContentCatalog::ReceiveMapFog(const BYTE* msg, int size)
{
    if (!ReadSection(s_PendingFog, msg, size)) Log("ContentCatalog: E9 descartado (size=%d)", size);
}

void CContentCatalog::ReceiveEffects(const BYTE* msg, int size)
{
    if (!ReadSection(s_PendingEffects, msg, size)) Log("ContentCatalog: EC descartado (size=%d)", size);
}

void CContentCatalog::ReceivePets(const BYTE* msg, int size)
{
    if (!ReadSection(s_PendingPets, msg, size)) Log("ContentCatalog: EE descartado (size=%d)", size);
}

void CContentCatalog::ReceiveTooltips(const BYTE* msg, int size)
{
    if (!ReadSection(s_PendingTooltips, msg, size)) Log("ContentCatalog: EF descartado (size=%d)", size);
}

void CContentCatalog::ReceiveEnd(const BYTE* msg, int size)
{
    if (size < (int)sizeof(Proto::PMSG_CATALOG_END_SEND)) return;
    Proto::PMSG_CATALOG_END_SEND end;
    memcpy(&end, msg, sizeof(end));
    const int items = SectionComplete(s_PendingItems) ? (int)(s_PendingItems.records.size() / s_PendingItems.recordSize) : -1;
    const int monsters = SectionComplete(s_PendingMonsters) ? (int)(s_PendingMonsters.records.size() / s_PendingMonsters.recordSize) : -1;
    if (items != end.itemCount || monsters != end.monsterCount || !SectionComplete(s_PendingFog)) {
        Log("ContentCatalog: catálogo incompleto (items %d/%d, monstruos %d/%d); se sigue con item.bmd",
            items, end.itemCount, monsters, end.monsterCount);
        return;
    }
    Publish();
    Log("ContentCatalog: %d items, %d monstruos, hash %08X", items, monsters, end.hash);
}

void CContentCatalog::Clear()
{
    ITEM_ATTRIBUTE* attr = Attributes();
    if (attr && !s_VanillaAttributes.empty()) {
        memcpy(attr, s_VanillaAttributes.data(), s_VanillaAttributes.size());
        memset(attr + ITEM_MAX_VANILLA, 0, (ITEM_MAX_EX - ITEM_MAX_VANILLA) * sizeof(ITEM_ATTRIBUTE));
    }
    for (ItemExtraData& item : s_Items) item = ItemExtraData();
    memset(s_Monsters, 0, sizeof(s_Monsters));
    memset(s_Fog, 0, sizeof(s_Fog));
    s_PendingItems = PendingSection();
    s_PendingMonsters = PendingSection();
    s_PendingFog = PendingSection();
    s_PendingEffects = PendingSection();
    s_Effects.clear();
    s_PendingPets = PendingSection();
    s_Pets.clear();
    s_PendingTooltips = PendingSection();
    s_Tooltips.clear();
    // Los modelos y texturas ya cargados se conservan: si el server vuelve a
    // mandar las mismas rutas se reusan sin volver a leer el disco.
    m_Loaded = false;
    m_HasFog = false;
}

void CContentCatalog::Publish()
{
    ITEM_ATTRIBUTE* attr = Attributes();
    if (!attr) return;
    if (s_VanillaAttributes.empty()) {
        s_VanillaAttributes.resize(ITEM_MAX_VANILLA * sizeof(ITEM_ATTRIBUTE));
        memcpy(s_VanillaAttributes.data(), attr, s_VanillaAttributes.size());
    }
    for (ItemExtraData& item : s_Items) item = ItemExtraData();
    s_DynamicModelItem.clear();

    const int itemSize = s_PendingItems.recordSize;
    for (size_t off = 0; off + itemSize <= s_PendingItems.records.size(); off += itemSize)
        ApplyItem(&s_PendingItems.records[off], itemSize);

    memset(s_Monsters, 0, sizeof(s_Monsters));
    const int monsterSize = s_PendingMonsters.recordSize;
    for (size_t off = 0; off + monsterSize <= s_PendingMonsters.records.size(); off += monsterSize) {
        Proto::CATALOG_MONSTER row = {};
        memcpy(&row, &s_PendingMonsters.records[off], min((int)sizeof(row), monsterSize));
        if (row.Index >= MAX_CATALOG_MONSTER) continue;
        CatalogMonster& monster = s_Monsters[row.Index];
        monster.Present = true;
        monster.Kind = row.Kind;
        monster.Golden = row.Golden != 0;
        monster.Scale = row.Scale;
        monster.Model = -1;
        memcpy(monster.Name, row.Name, sizeof(monster.Name) - 1);
        if (row.Flags & Proto::CATALOG_MONSTER_HAS_MODEL) {
            row.ModelFolder[sizeof(row.ModelFolder) - 1] = 0;
            row.ModelName[sizeof(row.ModelName) - 1] = 0;
            monster.Model = LoadModel(row.ModelFolder, row.ModelName, -1);
            SetMonsterSpeeds(monster.Model, row.Kind == 0);
        }
    }

    memset(s_Fog, 0, sizeof(s_Fog));
    const int fogSize = s_PendingFog.recordSize;
    for (size_t off = 0; fogSize > 0 && off + fogSize <= s_PendingFog.records.size(); off += fogSize) {
        Proto::CATALOG_MAP_FOG row = {};
        memcpy(&row, &s_PendingFog.records[off], min((int)sizeof(row), fogSize));
        if (row.Map >= MAX_CATALOG_MAP) continue;
        FogEntry& fog = s_Fog[row.Map];
        fog.Present = true;
        for (int c = 0; c < 3; ++c) {
            fog.Day[c] = row.DayRGB[c] / 255.0f;
            fog.Night[c] = row.NightRGB[c] / 255.0f;
        }
    }
    m_HasFog = false;
    for (const FogEntry& fog : s_Fog) m_HasFog = m_HasFog || fog.Present;

    // Tooltips: sección opcional.
    s_Tooltips.clear();
    const int tooltipSize = s_PendingTooltips.recordSize;
    if (SectionComplete(s_PendingTooltips) && tooltipSize > 0) {
        for (size_t off = 0; off + tooltipSize <= s_PendingTooltips.records.size(); off += tooltipSize) {
            Proto::CATALOG_TOOLTIP line = {};
            memcpy(&line, &s_PendingTooltips.records[off], min((int)sizeof(line), tooltipSize));
            line.Text[sizeof(line.Text) - 1] = 0;
            if (line.Item < ITEM_MAX_EX && line.Color <= 8) s_Tooltips[line.Item].push_back(line);
        }
    }

    // Pets: sección opcional, igual que los efectos.
    s_Pets.clear();
    const int petSize = s_PendingPets.recordSize;
    if (SectionComplete(s_PendingPets) && petSize > 0) {
        for (size_t off = 0; off + petSize <= s_PendingPets.records.size(); off += petSize) {
            Proto::CATALOG_PET pet = {};
            memcpy(&pet, &s_PendingPets.records[off], min((int)sizeof(pet), petSize));
            if (pet.Item < ITEM_MAX_EX) s_Pets[pet.Item] = pet;
        }
    }

    // Efectos: sección opcional (un server sin Data/Custom/Items no la manda
    // completa y el resto del catálogo vale igual).
    s_Effects.clear();
    const int effectSize = s_PendingEffects.recordSize;
    if (SectionComplete(s_PendingEffects) && effectSize > 0) {
        for (size_t off = 0; off + effectSize <= s_PendingEffects.records.size(); off += effectSize) {
            ItemEffect effect = {};
            memcpy(&effect.Data, &s_PendingEffects.records[off], min((int)sizeof(effect.Data), effectSize));
            Proto::CATALOG_EFFECT& data = effect.Data;
            if (data.Item >= ITEM_MAX_EX || data.BoneCount == 0 || data.BoneCount > Proto::CATALOG_EFFECT_MAX_BONES) continue;
            data.Texture[sizeof(data.Texture) - 1] = 0;
            effect.Bitmap = (data.Texture[0] != 0) ? LoadTexture(data.Texture) : (data.Bitmap != 0xFFFF ? (int)data.Bitmap : -1);
            if (data.Type == Proto::CATALOG_EFFECT_SPRITE && effect.Bitmap < 0) continue;
            s_Effects[data.Item].push_back(effect);
        }
    }
    m_FogMap = -1;
    m_Loaded = true;
}

void CContentCatalog::ApplyItem(const BYTE* record, int recordSize)
{
    // Registros más largos: se ignora lo que no se conoce.  Más cortos: el
    // resto queda en 0 (formato tolerante, ver el diseño).
    Proto::CATALOG_ITEM row = {};
    memcpy(&row, record, min((int)sizeof(row), recordSize));
    if (row.Index >= ITEM_MAX_EX) return;

    ITEM_ATTRIBUTE* attr = Attributes() + row.Index;
    // El nombre de un vanilla sale del item.bmd del idioma del cliente; el
    // del server se usa sólo para los items que el .bmd no tiene.
    if (row.Index >= ITEM_MAX_VANILLA || attr->Name[0] == 0) {
        memset(attr->Name, 0, sizeof(attr->Name));
        memcpy(attr->Name, row.Name, sizeof(attr->Name) - 1);
    }
    attr->TwoHand = row.TwoHand != 0;
    attr->Level = ClampByte(row.Level);
    attr->Width = row.Width;
    attr->Height = row.Height;
    attr->DamageMin = ClampByte(row.DamageMin);
    attr->DamageMax = ClampByte(row.DamageMax);
    attr->DefenseRate = ClampByte(row.DefenseSuccessRate);
    attr->Defense = ClampByte(row.Defense);
    attr->MagicDefense = ClampByte(row.MagicDefense);
    attr->AttackSpeed = ClampByte(row.AttackSpeed);
    attr->WalkSpeed = ClampByte(row.WalkSpeed);
    attr->Durability = row.Durability;
    attr->MagicDurability = row.MagicDurability;
    attr->MagicPower = ClampByte(row.MagicDamageRate);
    attr->RequireStrength = row.RequireStrength;
    attr->RequireAgility = row.RequireDexterity;
    attr->RequireEnergy = ClampByte(row.RequireEnergy);
    attr->RequireLevel = ClampByte(row.RequireLevel);
    attr->Value = ClampByte((int)row.Value);
    attr->Money = (int)row.BuyMoney;
    memcpy(attr->RequireClass, row.RequireClass, sizeof(attr->RequireClass));
    memcpy(attr->Resistance, row.Resistance, sizeof(attr->Resistance));

    ItemExtraData& extra = s_Items[row.Index];
    extra.Present = true;
    extra.Behavior = (row.Behavior < ITEM_MAX_EX) ? row.Behavior : row.Index;
    extra.Slot = (row.Slot < 12) ? row.Slot : -1;
    if (row.Flags & Proto::CATALOG_ITEM_HAS_GLOW) {
        extra.HasGlow = true;
        extra.Glow[0] = ((row.GlowColor >> 16) & 0xFF) / 255.0f;
        extra.Glow[1] = ((row.GlowColor >> 8) & 0xFF) / 255.0f;
        extra.Glow[2] = (row.GlowColor & 0xFF) / 255.0f;
    }
    if (row.Flags & Proto::CATALOG_ITEM_HAS_POSE) {
        extra.HasPose = true;
        extra.Pose = { row.PositionX, row.PositionY, row.RotationX, row.RotationY, row.RotationZ, row.Scale };
    }
    if (row.CustomWing != 0) {
        extra.HasWing = true;
        extra.Wing = { (BYTE)(row.CustomWing - 1), row.WingDefenseConstA, row.WingIncDamageConstA,
                       row.WingIncDamageConstB, row.WingDecDamageConstA, row.WingDecDamageConstB };
    }
    if (row.Flags & Proto::CATALOG_ITEM_HAS_ENTITY) {
        row.EntityFolder[sizeof(row.EntityFolder) - 1] = 0;
        row.EntityName[sizeof(row.EntityName) - 1] = 0;
        extra.EntityModel = LoadModel(row.EntityFolder, row.EntityName, -1);
        if (extra.EntityModel >= MODEL_MAX_VANILLA && !s_DynamicModelItem.count(extra.EntityModel))
            s_DynamicModelItem[extra.EntityModel] = row.Index;
    }
    if (row.Flags & Proto::CATALOG_ITEM_HAS_MODEL) {
        row.ModelFolder[sizeof(row.ModelFolder) - 1] = 0;
        row.ModelName[sizeof(row.ModelName) - 1] = 0;
        // Un vanilla (o un índice libre por debajo de 512) usa su slot de
        // siempre, type + 400: así lo dibujan todos los caminos del binario.
        const int fixedSlot = (row.Index < ITEM_MAX_VANILLA) ? row.Index + ITEM_MODEL_BASE : -1;
        extra.Model = LoadModel(row.ModelFolder, row.ModelName, fixedSlot);
        if (extra.Model >= MODEL_MAX_VANILLA && !s_DynamicModelItem.count(extra.Model))
            s_DynamicModelItem[extra.Model] = row.Index;
    }
}

// Carga un BMD del catálogo.  `fixedSlot` >= 0 lo pone en ese slot; si no, va
// a un slot dinámico (una sola vez por ruta).  Las texturas van a la región
// del catálogo (>= BITMAP_MAX_VANILLA), que los mapas no reciclan.
int CContentCatalog::LoadModel(const char* folder, const char* name, int fixedSlot)
{
    char sub[96];
    if (!DAT_05828d58 || !NormalizeFolder(folder, sub, sizeof(sub)) || !ValidName(name)) {
        Log("ContentCatalog: ruta de modelo inválida '%s' '%s'", folder, name);
        return -1;
    }
    const std::string key = std::string(sub) + name + "#" + std::to_string(fixedSlot);
    std::map<std::string, int>::iterator it = s_LoadedModels.find(key);
    if (it != s_LoadedModels.end()) return it->second;

    int slot = fixedSlot;
    if (slot < 0) {
        if (m_NextDynamicModel >= MODEL_MAX_DYNAMIC) {
            Log("ContentCatalog: sin slots de modelo para '%s%s'", sub, name);
            return -1;
        }
        slot = MODEL_MAX_VANILLA + m_NextDynamicModel++;
    }

    char dir[128];
    _snprintf_s(dir, sizeof(dir), _TRUNCATE, "Data\\%s", sub);
    AccessModel(slot, dir, name, -1);
    const BYTE* model = (const BYTE*)(uintptr_t)DAT_05828d58 + slot * 0xbc;
    if (*(const short*)(model + 0x24) <= 0) {
        Log("ContentCatalog: no se pudo cargar %s%s.bmd", dir, name);
        s_LoadedModels[key] = -1;
        return -1;
    }

    if (m_NextTexture == 0) m_NextTexture = BITMAP_MAX_VANILLA;
    const DWORD savedBegin = TextureBegin;
    const DWORD savedCurrent = TextureCurrent;
    if (m_NextTexture + 32 <= BITMAP_MAX_TOTAL) {
        TextureBegin = BITMAP_MAX_VANILLA;
        TextureCurrent = m_NextTexture;
        OpenTexture(slot, sub, 0x2600, '\x01');
        m_NextTexture = (int)TextureCurrent;
    } else {
        Log("ContentCatalog: sin espacio de texturas para %s%s", sub, name);
    }
    TextureBegin = savedBegin;
    TextureCurrent = savedCurrent;

    s_LoadedModels[key] = slot;
    return slot;
}

// Textura suelta de un efecto, en la región del catálogo.  Una vez por ruta.
int CContentCatalog::LoadTexture(const char* path)
{
    char sub[64];
    size_t n = 0;
    for (const char* p = path; *p && n + 1 < sizeof(sub); ++p) sub[n++] = (*p == '/') ? '\\' : *p;
    sub[n] = 0;
    if (n < 5 || sub[0] == '\\' || strstr(sub, "..") || strchr(sub, ':')) {
        Log("ContentCatalog: ruta de textura inválida '%s'", path);
        return -1;
    }
    std::map<std::string, int>::iterator it = s_LoadedTextures.find(sub);
    if (it != s_LoadedTextures.end()) return it->second;

    if (m_NextTexture == 0) m_NextTexture = BITMAP_MAX_VANILLA;
    if (m_NextTexture >= BITMAP_MAX_TOTAL) {
        Log("ContentCatalog: sin espacio de texturas para %s", sub);
        return -1;
    }
    const int slot = m_NextTexture++;
    const char* ext = strrchr(sub, '.');
    const bool tga = ext && (_stricmp(ext, ".tga") == 0);
    if (tga) OpenTGA(sub, slot, 0x2601, 0x2900, 0, '\x01');
    else     OpenJPG(sub, slot, 0x2601, 0x2900, 0, '\x01');
    s_LoadedTextures[sub] = slot;
    return slot;
}

void CContentCatalog::RunEquippedEffects(const void* c, int model, void* modelPtr) const
{
    if (s_Effects.empty() || !modelPtr) return;
    const int itemType = GetModelItemType(model);
    if (itemType < 0) return;
    std::map<int, std::vector<ItemEffect>>::const_iterator it = s_Effects.find(itemType);
    if (it == s_Effects.end()) return;

    for (const ItemEffect& effect : it->second) {
        const Proto::CATALOG_EFFECT& data = effect.Data;
        if (!(data.On & Proto::CATALOG_EFFECT_ON_EQUIPPED)) continue;
        const float pulse = (data.PulseSpeed != 0.0f) ? fabsf(sinf(DAT_05826e08 * data.PulseSpeed)) : 0.0f;
        const float scale = data.Scale + pulse * data.PulseScale;
        float light[3] = { data.Color[0] + pulse * data.PulseColor,
                           data.Color[1] + pulse * data.PulseColor,
                           data.Color[2] + pulse * data.PulseColor };
        float offset[3] = { data.Offset[0], data.Offset[1], data.Offset[2] };
        for (int b = 0; b < data.BoneCount; ++b) {
            if (data.Chance > 1 && rand() % data.Chance != 0) continue;
            float position[3];
            BMD_TransformPosition(modelPtr, (float*)(g_BoneScratch + data.Bones[b] * 0x30), offset, position, '\x01');
            if (data.Type == Proto::CATALOG_EFFECT_SPRITE) {
                CreateSprite((unsigned short)effect.Bitmap, position, scale, light, (int)(uintptr_t)c, 0.0f, 0);
            } else if (data.Type == Proto::CATALOG_EFFECT_PARTICLE) {
                Particle_Spawn(data.Particle, position, (float*)((BYTE*)c + 28), light, data.SubType, scale, 0);
            }
        }
    }
}

int CContentCatalog::GetItemBehavior(int type) const
{
    if (type < 0 || type >= ITEM_MAX_EX || !s_Items[type].Present) return type;
    return s_Items[type].Behavior;
}

int CContentCatalog::GetItemModel(int type) const
{
    if (type < 0 || type >= ITEM_MAX_EX) return type + ITEM_MODEL_BASE;
    const ItemExtraData& extra = s_Items[type];
    if (extra.Model >= 0) return extra.Model;
    if (type < ITEM_MAX_VANILLA) return type + ITEM_MODEL_BASE;
    // Agregado sin modelo propio: el de su comportamiento, o el primero de
    // su sección.
    const int behavior = extra.Present ? extra.Behavior : type;
    if (behavior != type && behavior >= 0 && behavior < ITEM_MAX_EX) return GetItemModel(behavior);
    return GetItemSection(type) * ITEM_MAX_TYPE_VANILLA + ITEM_MODEL_BASE;
}

int CContentCatalog::GetModelItemType(int model) const
{
    if (model >= ITEM_MODEL_BASE && model < ITEM_MODEL_BASE + ITEM_MAX_VANILLA) return model - ITEM_MODEL_BASE;
    std::map<int, int>::const_iterator it = s_DynamicModelItem.find(model);
    return (it == s_DynamicModelItem.end()) ? -1 : it->second;
}

bool CContentCatalog::GetItemGlow(int type, float rgb[3]) const
{
    if (type < 0 || type >= ITEM_MAX_EX || !s_Items[type].HasGlow) return false;
    memcpy(rgb, s_Items[type].Glow, sizeof(float) * 3);
    return true;
}

const CatalogItemPose* CContentCatalog::GetItemPose(int type) const
{
    if (type < 0 || type >= ITEM_MAX_EX || !s_Items[type].HasPose) return nullptr;
    return &s_Items[type].Pose;
}

const CatalogMonster* CContentCatalog::GetMonster(int index) const
{
    if (index < 0 || index >= MAX_CATALOG_MONSTER || !s_Monsters[index].Present) return nullptr;
    return &s_Monsters[index];
}

bool CContentCatalog::GetMapFog(int map, float day[3], float night[3]) const
{
    if (map < 0 || map >= MAX_CATALOG_MAP || !s_Fog[map].Present) return false;
    memcpy(day, s_Fog[map].Day, sizeof(float) * 3);
    memcpy(night, s_Fog[map].Night, sizeof(float) * 3);
    return true;
}

void CContentCatalog::GetFogColor(int map, float rgba[4])
{
    const DWORD now = GetTickCount();
    if (map != m_FogMap || now - m_FogTick >= 10000) {
        m_FogMap = map;
        m_FogTick = now;
        float day[3] = { 127 / 255.0f, 178 / 255.0f, 1.0f };   // MapFog.h del DLL
        float night[3] = { 51 / 255.0f, 51 / 255.0f, 51 / 255.0f };
        GetMapFog(map, day, night);
        SYSTEMTIME time;
        GetLocalTime(&time);
        const float minute = time.wMinute / 60.0f;
        float t = 0.0f;
        if (time.wHour == 7) t = minute;
        else if (time.wHour > 7 && time.wHour < 20) t = 1.0f;
        else if (time.wHour == 20) t = 1.0f - minute;
        for (int i = 0; i < 3; ++i) m_FogColor[i] = (1.0f - t) * night[i] + t * day[i];
        m_FogColor[3] = 1.0f;
    }
    memcpy(rgba, m_FogColor, sizeof(m_FogColor));
}

const CatalogWing* CContentCatalog::GetItemWing(int type) const
{
    if (type < 0 || type >= ITEM_MAX_EX || !s_Items[type].HasWing) return nullptr;
    return &s_Items[type].Wing;
}

int CContentCatalog::GetWingItem(int index) const
{
    for (int type = 0; type < ITEM_MAX_EX; ++type)
        if (s_Items[type].HasWing && s_Items[type].Wing.Index == index) return type;
    return -1;
}

bool CContentCatalog::HasOwnModel(int type) const
{
    return type >= 0 && type < ITEM_MAX_EX && s_Items[type].Model >= 0;
}

void CContentCatalog::SetEntityPart(const void* c, int part, int itemType)
{
    InitEntityParts();
    const int idx = EntityIndex(c);
    if (idx < 0 || part < 0 || part >= ENTITY_PARTS) return;
    EntityPart& entry = s_EntityParts[idx][part];
    entry = { -1, -1 };
    if (itemType < 0 || itemType >= ITEM_MAX_EX) return;
    const ItemExtraData& extra = s_Items[itemType];
    const int own = (extra.EntityModel >= 0) ? extra.EntityModel : extra.Model;
    if (own < 0) return;
    // El helper guarda el pet vanilla (816..819); el resto, el modelo lógico.
    const int logic = (part == 8) ? ItemModel(GetItemBehavior(itemType)) : ItemEntityModel(itemType);
    if (logic == own) return;
    entry = { (short)logic, (short)own };
}

void CContentCatalog::ClearEntityParts(const void* c)
{
    InitEntityParts();
    const int idx = EntityIndex(c);
    if (idx < 0) return;
    for (EntityPart& part : s_EntityParts[idx]) part = { -1, -1 };
}

int CContentCatalog::EntityDrawModel(const void* c, int model) const
{
    if (!s_EntityPartsInit || model < 0) return model;
    const int idx = EntityIndex(c);
    if (idx < 0) return model;
    for (const EntityPart& part : s_EntityParts[idx])
        if (part.Logic == model && part.Own >= 0) return part.Own;
    return model;
}

int CContentCatalog::EntityHelperModel(const void* c) const
{
    if (!s_EntityPartsInit) return -1;
    const int idx = EntityIndex(c);
    return (idx < 0) ? -1 : s_EntityParts[idx][8].Own;
}

int CContentCatalog::LogicModel(int model) const
{
    if (model < MODEL_MAX_VANILLA) return model;
    const int itemType = GetModelItemType(model);
    if (itemType < 0) return model;
    const int behavior = GetItemBehavior(itemType);
    return (behavior >= 0 && behavior < ITEM_MAX_VANILLA) ? ItemModel(behavior) : model;
}

int CContentCatalog::GetItemSlot(int type) const
{
    if (type < 0 || type >= ITEM_MAX_EX || !s_Items[type].Present) return -1;
    return s_Items[type].Slot;
}

const Proto::CATALOG_PET* CContentCatalog::GetPet(int itemType) const
{
    std::map<int, Proto::CATALOG_PET>::const_iterator it = s_Pets.find(itemType);
    return (it == s_Pets.end()) ? nullptr : &it->second;
}

const Proto::CATALOG_PET* CContentCatalog::GetPetByModel(int model) const
{
    if (s_Pets.empty()) return nullptr;
    const int itemType = GetModelItemType(model);
    return (itemType < 0) ? nullptr : GetPet(itemType);
}

int CContentCatalog::GetTooltipCount(int itemType) const
{
    std::map<int, std::vector<Proto::CATALOG_TOOLTIP>>::const_iterator it = s_Tooltips.find(itemType);
    return (it == s_Tooltips.end()) ? 0 : (int)it->second.size();
}

const Proto::CATALOG_TOOLTIP* CContentCatalog::GetTooltipLine(int itemType, int index) const
{
    std::map<int, std::vector<Proto::CATALOG_TOOLTIP>>::const_iterator it = s_Tooltips.find(itemType);
    if (it == s_Tooltips.end() || index < 0 || index >= (int)it->second.size()) return nullptr;
    return &it->second[index];
}
