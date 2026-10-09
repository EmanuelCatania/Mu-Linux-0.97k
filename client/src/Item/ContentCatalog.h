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

struct CatalogItemPose
{
    float PositionX;
    float PositionY;
    float RotationX;
    float RotationY;
    float RotationZ;
    float Scale;
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

    // Item que se dibuja con ese slot de modelo, o -1.
    int GetModelItemType(int model) const;

    bool GetItemGlow(int type, float rgb[3]) const;
    const CatalogItemPose* GetItemPose(int type) const;

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

// Modelo de un item puesto en un personaje.  Un agregado (>= 512) se dibuja
// con el modelo de su comportamiento: es lo que ven los demás jugadores (el
// CharSet sólo lleva vanilla) y mantiene intacta la lógica que mira el tipo de
// arma por rangos de modelo.
inline int ItemEntityModel(int type)
{
    if (type < ITEM_MAX_VANILLA) return ItemModel(type);
    const int behavior = gContentCatalog.GetItemBehavior(type);
    return (behavior >= 0 && behavior < ITEM_MAX_VANILLA) ? ItemModel(behavior)
                                                          : GetItemSection(type) * ITEM_MAX_TYPE_VANILLA + ITEM_MODEL_BASE;
}
