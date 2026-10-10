#pragma once
// ContentCatalog.h — definiciones de contenido que manda el server (0.97.20).
//
// DESVIACION: en el 0.97k las definiciones de items vivían en item.bmd y las
// de los "customs" en ClientInfo.bmd del DLL.  Acá el server es la autoridad:
// manda la tabla completa de items, monstruos y fog por mapa antes de la
// respuesta del login (F3/E7..EA, ver Net/Protocol/GameServerProtocol.h).
// El cliente aplica la tabla sobre la de item.bmd (que queda como respaldo si
// el server no la manda) y sólo resuelve los assets locales.

#include <windows.h>
#include "Item/ItemDefines.h"
#include "Net/Protocol/GameServerProtocol.h"

struct CatalogItemPose
{
    float PositionX;
    float PositionY;
    float RotationX;
    float RotationY;
    float RotationZ;
    float Scale;
};

// Ala custom (CustomWing.txt del server): índice que viaja en el CharSet y
// constantes de defensa y daño.
struct CatalogWing
{
    BYTE Index;          // 0..6
    WORD DefenseConstA;
    WORD IncDamageConstA;
    WORD IncDamageConstB;
    WORD DecDamageConstA;
    WORD DecDamageConstB;
};

struct CatalogMonster
{
    bool  Present;
    BYTE  Kind;          // 0 NPC, 1 monstruo, 0xFF = el vanilla
    bool  Golden;
    float Scale;         // 0 = la vanilla
    int   Model;         // slot de modelo, -1 = el vanilla
    char  Name[32];
};

class CContentCatalog
{
public:
    // Paquetes del server.
    void ReceiveItems(const BYTE* msg, int size);      // F3/E7
    void ReceiveMonsters(const BYTE* msg, int size);   // F3/E8
    void ReceiveMapFog(const BYTE* msg, int size);     // F3/E9
    void ReceiveEffects(const BYTE* msg, int size);    // F3/EC
    void ReceivePets(const BYTE* msg, int size);       // F3/EE
    void ReceiveTooltips(const BYTE* msg, int size);   // F3/EF
    void ReceiveEnd(const BYTE* msg, int size);        // F3/EA: valida y publica

    // Al desconectar: la tabla vuelve a la de item.bmd.
    void Clear();

    bool IsLoaded() const { return m_Loaded; }

    // Vanilla cuyo comportamiento hardcodeado usa el item (el propio si no
    // tiene columna Behavior).
    int GetItemBehavior(int type) const;

    // Slot de modelo para dibujar el item: type + 400 para los vanilla,
    // el slot dinámico o el del comportamiento para los agregados.
    int GetItemModel(int type) const;
    // true si el catálogo le da al item un modelo propio.
    bool HasOwnModel(int type) const;

    // Item que se dibuja con ese slot de modelo, o -1.
    int GetModelItemType(int model) const;

    bool GetItemGlow(int type, float rgb[3]) const;
    const CatalogItemPose* GetItemPose(int type) const;

    const CatalogWing* GetItemWing(int type) const;
    // Item del ala custom con ese índice del CharSet (0..6), o -1.
    int GetWingItem(int index) const;

    // Piezas puestas en un personaje.  La entidad guarda el modelo del
    // vanilla que imita el item (la lógica decide por esos rangos); acá se
    // recuerda con qué modelo propio se dibuja cada pieza.  part: 0 casco,
    // 1 armadura, 2 pantalones, 3 guantes, 4 botas, 5 mano izq., 6 mano der.,
    // 7 alas, 8 helper.
    void SetEntityPart(const void* c, int part, int itemType);
    void ClearEntityParts(const void* c);
    // Modelo a dibujar para una pieza cuyo modelo lógico es `model`.
    int  EntityDrawModel(const void* c, int model) const;
    // Modelo propio del helper puesto (pet o montura), o -1.
    int  EntityHelperModel(const void* c) const;
    // Modelo vanilla que imita un modelo dinámico (para la lógica de brillo
    // por nivel y los efectos por tipo); el mismo si no es del catálogo.
    int  LogicModel(int model) const;

    // Efectos del item dibujado con `model` sobre la entidad `c`, en los huesos
    // que dejó el último render del modelo (g_BoneScratch).  Se llama desde
    // RenderLinkObject después de dibujar la pieza.
    void RunEquippedEffects(const void* c, int model, void* modelPtr) const;

    // Wear slot (0..11) de un agregado sin comportamiento vanilla (columna
    // Slot de Item.txt), o -1.
    int GetItemSlot(int type) const;

    // Pet custom del item, o nullptr.  Para un bug, por su modelo.
    const Proto::CATALOG_PET* GetPet(int itemType) const;

    // Líneas propias del tooltip del item.
    int GetTooltipCount(int itemType) const;
    const Proto::CATALOG_TOOLTIP* GetTooltipLine(int itemType, int index) const;
    const Proto::CATALOG_PET* GetPetByModel(int model) const;

    const CatalogMonster* GetMonster(int index) const;

    // Niebla del mapa: colores de día y de noche (0..1).  false = sin fila.
    bool GetMapFog(int map, float day[3], float night[3]) const;

    // DLL MapFog: hay niebla si el server mandó alguna fila.  El color del
    // mapa mezcla día y noche según la hora local; los mapas sin fila usan los
    // colores por defecto del DLL.  Se recalcula al cambiar de mapa o cada 10 s.
    bool HasMapFog() const { return m_HasFog; }
    void GetFogColor(int map, float rgba[4]);

private:
    void Publish();
    void ApplyItem(const BYTE* record, int recordSize);
    int  LoadModel(const char* folder, const char* name, int fixedSlot);
    int  LoadTexture(const char* path);

    bool m_Loaded = false;
    bool m_HasFog = false;
    int  m_FogMap = -1;
    DWORD m_FogTick = 0;
    float m_FogColor[4] = {};
    int  m_NextDynamicModel = 0;
    int  m_NextTexture = 0;
};

extern CContentCatalog gContentCatalog;

// Slot de modelo de un item: type + 400 para los vanilla (el `Type + 400` del
// binario), el del catálogo para los agregados.
inline int ItemModel(int type) { return gContentCatalog.GetItemModel(type); }

// Tipo con el que la lógica hardcodeada (rangos de arco, ballesta, flechas…)
// trata al item: el vanilla que imita un agregado.  -1 sigue siendo -1.
inline int ItemBehaviorType(int type) { return (type < 0) ? type : gContentCatalog.GetItemBehavior(type); }

// Modelo LÓGICO de un item puesto en un personaje: el del vanilla que imita un
// agregado (>= 512).  Es el que guarda la entidad, así la lógica que mira el
// tipo de arma, ala o helper por rangos de modelo sigue igual.  El modelo
// propio se dibuja aparte (SetEntityPart / EntityDrawModel).
inline int ItemEntityModel(int type)
{
    if (type < ITEM_MAX_VANILLA) return ItemModel(type);
    const int behavior = gContentCatalog.GetItemBehavior(type);
    return (behavior >= 0 && behavior < ITEM_MAX_VANILLA) ? ItemModel(behavior)
                                                          : GetItemSection(type) * ITEM_MAX_TYPE_VANILLA + ITEM_MODEL_BASE;
}
