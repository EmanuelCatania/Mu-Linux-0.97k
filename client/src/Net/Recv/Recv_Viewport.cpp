// Recv_Viewport.cpp — paquetes del server: entidades del viewport: aparición, desaparición, movimiento y cambio de equipo.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/NetText.h"
#include "Entity/EntityName.h"
#include "Game/RenderSnapshots.h"
#include "Item/ContentCatalog.h"
#include "Item/ItemDefines.h"
#include "Net/Recv/NetRecv.h"

// ---------------------------------------------------------------------------
// 0x25 — ReceiveChangePlayer  (@ 0x00429230)
//
// Cambio de UNA pieza de equipo de un jugador del viewport (el propio incluido).
// Es el que faltaba para que se vea en vivo cuando otro se pone o se saca algo:
// el F3/13 solo lo manda el server al abrir un trade (Trade.cpp:51), asi que
// cubre el estado inicial, no los cambios.
//
// PMSG_ITEM_CHANGE_SEND (ItemManager.h:108, `header.set` -> C1 con PBMSG_HEAD de
// 3 bytes):
//    +0..2  {C1, size, 0x25}
//    +3,+4  BYTE index[2]      (key de la entidad, big-endian)
//    +5..   BYTE ItemInfo[]
// El server empaqueta el slot y el nivel juntos en ItemInfo[1] (= +6):
//    `ItemInfo[1] = slot * 16 | (LevelSmallConvert(level) & 0x0F)`
// o sea `Msg[6] >> 4` = slot de equipo (0..8) y `Msg[6] & 0xF` = nivel.
// Coincide exactamente con como lo lee IDA.
//
// Trampa del decompile: Hex-Rays reusa la variable `Type` para dos cosas —
// primero `Type = (int)(ReceiveBuffer + 5)` (un PUNTERO) y despues
// `Type = ConvertItemType(ReceiveBuffer + 5)` (el tipo).  Las comparaciones
// `*(_BYTE *)Type == 0xFF` son del PRIMER uso, o sea `Msg[5] == 0xFF`
// (= slot vacio); los `Type + 400` son del segundo.
//
// IDA: ReceiveChangePlayer (0x00429230).
// DESVIACION (fix del DLL, Patchs.cpp 0x004292BC): convertir también el nivel del arma del slot 0.
// ---------------------------------------------------------------------------

// LevelConvert (0x0045C850) — mapea el nibble de nivel del paquete al +N real.
int Net_LevelConvert(BYTE Level)
{
    switch (Level) {
        case 1: return 3;
        case 2: return 5;
        case 3: return 7;
        case 4: return 8;
        case 5: return 9;
        case 6: return 10;
        case 7: return 11;
        default: return 0;
    }
}

// 0.97.20: C1:F3:EB — equipo agregado (>= 512) de un jugador del viewport.
// Llega después del 0x12, que sólo trae vanilla en el CharSet: deja dicho con
// qué modelo propio se dibuja cada pieza.  La entidad conserva el modelo del
// vanilla que imita (pet y montura incluidos), así que el bug ya creado por el
// CharSet se dibuja con el modelo del helper custom.
static void ApplyCustomEquipment(BYTE* c, const BYTE* items)
{
    static const int kPart[9] = { 5, 6, 0, 1, 2, 3, 4, 7, 8 };
    for (int slot = 0; slot < 9; ++slot) {
        const WORD item = *(const WORD*)(items + slot * 2);
        if (item != 0xFFFF) gContentCatalog.SetEntityPart(c, kPart[slot], item);
    }
    // CustomPet: el CharSet lo trajo como "sin helper"; acá se crea su bug.
    const WORD helper = *(const WORD*)(items + 8 * 2);
    if (helper != 0xFFFF && gContentCatalog.GetPet(helper)) {
        *(WORD*)(c + 696) = (WORD)ItemModel(helper);
        DeleteBug((DWORD)(uintptr_t)c);
        CreateBug(ItemModel(helper), (void*)(c + 16), (void*)c, 0);
    }
}

void Recv_CustomEquipment(const BYTE* Msg, int Size)
{
    if (Size < 24 || !DAT_07abf5d0) return;
    const int key = ((Msg[4] & 0x7F) << 8) | Msg[5];
    const int idx = FindCharacterIndex(key);
    if (idx < 0 || idx >= 400) return;
    ApplyCustomEquipment((BYTE*)(uintptr_t)DAT_07abf5d0 + (size_t)idx * 0x394, Msg + 6);
}

// 0.97.20: C1:F3:ED — lo mismo para un personaje de la lista del
// char-select.  La entidad es la del slot (ChangeCharacterExt(slot, ...)),
// y este paquete llega justo después del F3/00.
void Recv_CharacterListCustom(const BYTE* Msg, int Size)
{
    if (Size < 24 || !DAT_07abf5d0) return;
    const int slot = Msg[4];
    if (slot >= 400) return;
    ApplyCustomEquipment((BYTE*)(uintptr_t)DAT_07abf5d0 + (size_t)slot * 0x394, Msg + 6);
}

void Recv_ChangePlayer(const BYTE* Msg, int Size)
{
    if (Size < 5 + ITEM_INFO_SIZE || !DAT_07abf5d0) return;

    const int key = Msg[4] + (Msg[3] << 8);
    const int idx = FindCharacterIndex(key);
    if (idx < 0 || idx >= 400) {
        NetLog("NET:  -> 0x25 ChangePlayer key=%d (entidad ausente)", key);
        return;
    }
    BYTE* c = (BYTE*)(uintptr_t)DAT_07abf5d0 + (size_t)idx * 0x394;

    const bool  empty  = (Msg[5] == 0xFF);
    const int   type   = ConvertItemType((BYTE*)Msg + 5);   // 0x0047B110
    const BYTE  level  = (BYTE)(Msg[6] & 0x0F);
    const BYTE  option = (BYTE)(Msg[8] & 0x3F);
    const int   slot   = Msg[6] >> 4;
    // Modelo por defecto de la clase cuando la pieza se saca:
    //   912/919/926/933/940 + (skin & 7) + 4 * (skin >> 3)
    const BYTE  skin   = AsCharacter(c)->Class;
    const int   klass  = (skin & 7) + 4 * (skin >> 3);

    NetLog("NET:  -> 0x25 ChangePlayer key=%d idx=%d slot=%d type=%d lvl=%u%s",
           key, idx, slot, type, (unsigned)level, empty ? " (vacio)" : "");

    // 0.97.20: el modelo propio de la pieza (catálogo).  slot del paquete ->
    // pieza de la entidad.
    {
        static const int kPart[9] = { 5, 6, 0, 1, 2, 3, 4, 7, 8 };
        if (slot >= 0 && slot < 9) gContentCatalog.SetEntityPart(c, kPart[slot], empty ? -1 : type);
    }

    switch (slot) {
        case 0:   // mano izquierda
            if (empty) { *(WORD *)&AsCharacter(c)->Weapon[0].Type = (WORD)-1; AsCharacter(c)->Weapon[0].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->Weapon[0].Type = (WORD)ItemEntityModel(type); AsCharacter(c)->Weapon[0].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->Weapon[0].Option1 = option; }
            break;
        case 1:   // mano derecha
            if (empty) { *(WORD *)&AsCharacter(c)->Weapon[1].Type = (WORD)-1; AsCharacter(c)->Weapon[1].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->Weapon[1].Type = (WORD)ItemEntityModel(type);
                         AsCharacter(c)->Weapon[1].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->Weapon[1].Option1 = option; }
            break;
        case 2:   // casco
            if (empty) { *(WORD *)&AsCharacter(c)->BodyPart[1].Type = (WORD)(klass + 912); AsCharacter(c)->BodyPart[1].Level = 0; AsCharacter(c)->BodyPart[1].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->BodyPart[1].Type = (WORD)ItemEntityModel(type);
                         AsCharacter(c)->BodyPart[1].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->BodyPart[1].Option1 = option; }
            break;
        case 3:   // armadura
            if (empty) { *(WORD *)&AsCharacter(c)->BodyPart[2].Type = (WORD)(klass + 919); AsCharacter(c)->BodyPart[2].Level = 0; AsCharacter(c)->BodyPart[2].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->BodyPart[2].Type = (WORD)ItemEntityModel(type);
                         AsCharacter(c)->BodyPart[2].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->BodyPart[2].Option1 = option; }
            break;
        case 4:   // pantalones
            if (empty) { *(WORD *)&AsCharacter(c)->BodyPart[3].Type = (WORD)(klass + 926); AsCharacter(c)->BodyPart[3].Level = 0; AsCharacter(c)->BodyPart[3].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->BodyPart[3].Type = (WORD)ItemEntityModel(type);
                         AsCharacter(c)->BodyPart[3].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->BodyPart[3].Option1 = option; }
            break;
        case 5:   // guantes
            if (empty) { *(WORD *)&AsCharacter(c)->BodyPart[4].Type = (WORD)(klass + 933); AsCharacter(c)->BodyPart[4].Level = 0; AsCharacter(c)->BodyPart[4].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->BodyPart[4].Type = (WORD)ItemEntityModel(type);
                         AsCharacter(c)->BodyPart[4].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->BodyPart[4].Option1 = option; }
            break;
        case 6:   // botas
            if (empty) { *(WORD *)&AsCharacter(c)->BodyPart[5].Type = (WORD)(klass + 940); AsCharacter(c)->BodyPart[5].Level = 0; AsCharacter(c)->BodyPart[5].Option1 = 0; }
            else       { *(WORD *)&AsCharacter(c)->BodyPart[5].Type = (WORD)ItemEntityModel(type);
                         AsCharacter(c)->BodyPart[5].Level = (BYTE)Net_LevelConvert(level); AsCharacter(c)->BodyPart[5].Option1 = option; }
            break;
        case 7:   // alas
            if (empty) { *(WORD *)&AsCharacter(c)->Wing.Type = (WORD)-1; }
            else       { *(WORD *)&AsCharacter(c)->Wing.Type = (WORD)ItemEntityModel(type); AsCharacter(c)->Wing.Level = 0; }
            break;
        case 8: {  // helper / mascota
            if (empty) {
                *(WORD *)&AsCharacter(c)->Helper.Type = (WORD)-1;
                DeleteBug((DWORD)(uintptr_t)c);        // DeleteBug
            } else {
                *(WORD *)&AsCharacter(c)->Helper.Type = (WORD)ItemEntityModel(type);
                AsCharacter(c)->Helper.Level = 0;
                float* pos = (float*)(c + 16);
                // 0.97.20: un agregado crea el bug del pet que imita.
                const int pet = ItemBehaviorType(type);
                DeleteBug((DWORD)(uintptr_t)c);
                if (pet == 416)      CreateBug(816, (void*)pos, (void*)c, 0);
                else if (pet == 418) CreateBug(195, (void*)pos, (void*)c, 0);
                else if (pet == 419) CreateBug(267, (void*)pos, (void*)c, 0);
                else if (gContentCatalog.GetPet(type)) CreateBug(ItemModel(type), (void*)pos, (void*)c, 0);   // CustomPet
            }
            break;
        }
        default:
            break;
    }

    SetCharacterScale(AsCharacter(c));   // SetCharacterScale
}

// 0x10
void NetRecv_10(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PacketHandler_0x10: move-confirmed
    // [C1][len][0x10][id_hi][id_lo][gx][gy][speed_flags]
    if (Size < 8) return;
    WORD entityId = ((Msg[3] & 0x7F) << 8) | Msg[4];
    BYTE gx = Msg[5], gy = Msg[6];
    NetLog("NET:  → 0x10 Move id=%d to (%d,%d)", entityId, gx, gy);
    // Actualizar la entidad sin importar si es hero u otra.
    BYTE* h = nullptr;
    if (entityId == g_HeroKey && DAT_07abf5d8) {
        h = (BYTE*)DAT_07abf5d8;
    } else {
        BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
        for (int s = 0; s < 400; ++s) {
            BYTE* slot = basePtr + s * 0x394;
            if (slot[0] && *(WORD*)(slot + 0x1dc) == entityId) {
                h = slot; break;
            }
        }
    }
    if (h && AsCharacter(h)->Dead == 0) {
        // ReceiveMoveCharacter @ 00427B90: speed is always updated.
        h[0x2fc] = (BYTE)(Msg[7] >> 4);

        if (h == (BYTE*)DAT_07abf5d8) {
            // IDA no pisa acá la ruta local del héroe ni su posición
            // en el mundo. Sólo refresca este cache de respaldo
            // while no locally interpolated path is active.
            if (AsCharacter(h)->Movement == 0) {
                AsCharacter(h)->PositionX = gx;
                AsCharacter(h)->PositionY = gy;
            }
        } else {
            // ReceiveMoveCharacter 00427B90: remote entities store
            // el destino del server en +774/+775, calcula la
            // ruta desde las coordenadas de grilla cacheadas (+904/+908), y después
            // deja que el tick normal de movimiento consuma esa ruta.
            AsCharacter(h)->TargetX = gx;
            AsCharacter(h)->TargetY = gy;
            if (h[848] == 0) {
                const int srcX = AsCharacter(h)->PositionX;
                const int srcY = AsCharacter(h)->PositionY;
                if (Path_FindRoute(srcX, srcY, gx, gy, h + 852, 0.0f)) {
                    AsCharacter(h)->Movement = 1;
                    // 00427B90 invokes SetPlayerWalk immediately
                    // para el tipo 322. Al resto de las entidades remotas las
                    // cambia MoveMonsterClient antes de MovePath.
                    if (*(WORD *)&AsCharacter(h)->Object.Type == 322) {
                        SetPlayerWalk(AsCharacter(h));
                        AsCharacter(h)->JumpTime = 1;
                    }
                } else {
                    AsCharacter(h)->Movement = 0;
                    SetPlayerStop(AsCharacter(h));
                }
            }
        }
    }
}

// 0x11
void NetRecv_11(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PacketHandler_0x11: position-set / teleport
    // [C1][len][0x11][id_hi][id_lo][gx][gy]
    if (Size < 7) return;
    WORD entityId = ((Msg[3] & 0x7F) << 8) | Msg[4];
    BYTE gx = Msg[5], gy = Msg[6];
    NetLog("NET:  → 0x11 Position id=%d to (%d,%d)", entityId, gx, gy);
    BYTE* h = nullptr;
    if (entityId == g_HeroKey && DAT_07abf5d8) {
        h = (BYTE*)DAT_07abf5d8;
    } else {
        BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
        for (int s = 0; s < 400; ++s) {
            BYTE* slot = basePtr + s * 0x394;
            if (slot[0] && *(WORD*)(slot + 0x1dc) == entityId) {
                h = slot; break;
            }
        }
    }
    if (h) {
        // ReceiveMovePosition 00427F40 sólo refresca el estado de
        // grilla y encola la actualización de posición. Las coordenadas
        // de mundo las avanza el tick normal de la entidad.
        AsCharacter(h)->PositionX = gx;
        AsCharacter(h)->PositionY = gy;
        AsCharacter(h)->TargetX = gx;
        AsCharacter(h)->TargetY = gy;
        AsCharacter(h)->JumpTime = 1;
    }
}

// 0x12
void NetRecv_12(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // El server-emu manda ViewportPlayer como C2 (largo de 2 bytes): ahí Msg[3]
    // es el byte de OPCODE, no el count.
    //
    // Layout per framing:
    //   C1: [C1][len][op=12][count][entries...]   data@Msg+3
    //   C2: [C2][len_hi][len_lo][op=12][count][entries...] data@Msg+4
    int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
    NetLog("NET:  → 0x12 ViewportPlayer count=%d size=%d hdr=%02X",
           Msg[3 + hdrOff], Size, Msg[0]);
    // No descartar el viewport si el heroe todavia no esta creado: al entrar al
    // mundo el server manda el viewport junto con el spawn del heroe.
    //
    // IDA `Combat_PacketDispatch` (0x429690) NO tiene ese guard:
    // crea las entidades con `CreateCharacter` sin mirar al heroe.
    // Lo unico que hace falta es el ARRAY de entidades. El filtro de
    // mas abajo ya tolera `DAT_07abf5d8 == 0` (`heroEnt ? ... :
    // g_HeroKey`) y el bloque del heroe tiene su propio `if (hero)`.
    if (!DAT_07abf5d0) {
        NetLog("NET:    0x12 SKIP - entity array no alocado");
        return;
    }
    int count = Msg[3 + hdrOff];
    // IDA (`ReceiveCreateMonsterViewport` 0x42A230, `Combat_PacketDispatch`
    // 0x429690) NO tiene limite: itera `if (ReceiveBuffer[4]) do {...} while`
    // por el count crudo, que es un BYTE (el viewport inicial de Lost Tower
    // trae count=41). El tope real es 255 y la cota util es la validacion por
    // `Size` que esta abajo.
    if (count <= 0 || count > 255) {
        NetLog("NET:    0x12 SKIP - count=%d out of range", count);
        return;
    }
    // PMSG_VIEWPORT_PLAYER (0.97.40): index[2], x, y, CharSet[11], padding,
    // ViewSkillState, name[40], tx, ty, DirAndPkLevel, padding = 62 bytes.
    const int NAME = (int)NetText::CharacterNameSize;
    int entryStart = 4 + hdrOff;
    const int entryStride = 22 + NAME;
    if ((entryStart + count * entryStride) > Size) {
        NetLog("NET:    0x12 SKIP - payload corto (count=%d, size=%d)", count, Size);
        return;
    }
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int i = 0; i < count && (entryStart + (i + 1) * entryStride) <= Size; ++i) {
        const BYTE* e = Msg + entryStart + i*entryStride;
        // Layout decompilado del binario MuEmu
        // server (CViewport::GCViewportPlayerSend, asm a1590).
        // Stride 62 bytes (0.97.40):
        //   e[0..1]=Key BE, e[2]=PosX, e[3]=PosY,
        //   e[4..14]=CharSet(11B), e[15]=padding,
        //   e[16..17]=ViewSkillState (LE), e[18..57]=Name(40B),
        //   e[58]=TargetX, e[59]=TargetY, e[60]=Path|Dir.
        WORD entityId = ((WORD)(e[0] & 0x7F) << 8) | e[1];   // strip CREATE bit
        BYTE x = e[2], y = e[3];
        char name[NetText::CharacterNameSize + 1] = {0};
        NetText::FromServer(name, sizeof(name), e + 0x12, NAME);
        BYTE tx = e[0x12 + NAME], ty = e[0x13 + NAME];
        BYTE dirpk = e[0x14 + NAME];
        BYTE dir = (dirpk >> 4) & 0x0F;
        const WORD viewSkillState = (WORD)(e[0x10] | (e[0x11] << 8));
        // Un refresco de viewport por gate de mapa de MuEmu puede devolver
        // como eco al personaje local. El original mantiene al Hero como único objeto
        // para HeroKey; pasarlo por CreateCharacter crea
        // una segunda copia del jugador, renderizada por separado.
        // Se compara contra `g_HeroKey` Y contra el Key real de la entidad del heroe:
        // si `g_HeroKey` quedara desfasado, `CreateCharacter` reusaria el slot del
        // heroe por Key y lo re-inicializaria SIN pasar por la restauracion de abajo
        // (se perderia, entre otras cosas, el byte de clase +0x1BC).
        BYTE* heroEnt = (BYTE*)(uintptr_t)DAT_07abf5d8;
        const WORD heroKeyReal = heroEnt ? *(WORD*)(heroEnt + 0x1DC) : g_HeroKey;
        if (entityId == g_HeroKey || entityId == heroKeyReal) {
            BYTE* hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
            if (hero) {
                AsCharacter(hero)->PositionX = x;
                AsCharacter(hero)->PositionY = y;
                AsCharacter(hero)->TargetX = tx;
                AsCharacter(hero)->TargetY = ty;

                // Vuelta de una transformacion (anillo).
                // El 0x45 (GCViewportSimpleChangeSend) convierte al
                // heroe en monstruo llamando CreateMonster con SU
                // key; al sacarse el anillo el server manda este 0x12
                // con el player normal.  IDA no filtra por HeroKey:
                // `Combat_PacketDispatch` L86 llama
                // `CreateCharacter(key, 390, x, y, 0.0)` para TODAS
                // las entradas, y eso reusa el slot por key y lo
                // recrea como player — ese es el camino de vuelta.
                // Nosotros salteamos la entrada propia para no
                // clonar al heroe (nuestro scan por key excluye su
                // slot), asi que replicamos solo la restauracion.
                if (AsCharacter(hero)->Object.Type != 390) {
                    const float rot = ((float)dir - 1.0f) * 45.0f;
                    CreateCharacterPointer(hero, 390, x, y, rot);
                    AsCharacter(hero)->Key = g_HeroKey;
                    if (CharacterAttribute)
                        AsCharacter(hero)->Class = *((const BYTE*)CharacterAttribute + 11);
                    AsCharacter(hero)->Skin   = 0;
                    AsCharacter(hero)->PK = (BYTE)(dirpk & 0x0F);   // PKLevel
                    AsCharacter(hero)->Object.Kind   = 1;
                    AsCharacter(hero)->TargetX = tx;
                    AsCharacter(hero)->TargetY = ty;
                    SetCharacterClass((int)(uintptr_t)hero);   // SetCharacterClass
                    NetLog("NET:    0x12 heroe restaurado de transformacion");
                }

                // Combat_PacketDispatch (IDA 00429690) no excluye
                // la fila del jugador local: CreateCharacterPointer
                // reinicia Character+120, ChangeCharacterExt aplica
                // CharSet[1..10] y luego InsertBuffPhysicalEffect
                // consume ViewSkillState. Nuestro atajo para evitar
                // un clon conservaba la posición, pero omitía esas
                // tres consecuencias. OpenWorld borra los pools de
                // bugs/effects/joints, por lo que el resultado era
                // exactamente un Angel/Imp o Mana Shield ausente al
                // cruzar de mapa.
                //
                // No recreamos el slot completo: el héroe local
                // conserva estado que no pertenece al viewport. Sí
                // reproducimos los dos resets visuales que necesita
                // esta fila del protocolo.
                *(DWORD*)(hero + 120) = 0;       // CreateCharacterPointer
                ChangeCharacterExt((int)HeroIndex, (BYTE*)e + 5);
                Hero_ApplyCatalogEquipment((int)(uintptr_t)hero);
                ApplyPersistentSkillEffect97k(hero, viewSkillState, 1);
            }
            NetLog("NET:    0x12 own HeroKey=%u synchronized, no viewport clone",
                   (unsigned)entityId);
            continue;
        }
        // Se delega en `CreateCharacter` (port fiel, Monster/Monster.cpp): busca el
        // slot por Key sin excluir a nadie —IDA `Combat_PacketDispatch` L86 llama
        // `CreateCharacter(key, 390, x, y, 0.0)` para TODAS las entradas— y si no
        // lo encuentra reusa uno inactivo llamando antes a `DeleteCloth`.  No
        // reimplementar ese scan acá.
        //
        // Su centinela de pool lleno (`base + 366400` = slot 400)
        // cae dentro del buffer: WinMain aloca 0x764D4 = 529 slots
        // y el offset random inicial se come a lo sumo 127, o sea
        // quedan 402 utiles.
        float rot = ((float)dir - 1.0f) * 45.0f;
        BYTE* slot = (BYTE*)(uintptr_t)CreateCharacter(entityId, 390, x, y, rot);
        int spawnSlot = (int)(((uintptr_t)slot - (uintptr_t)basePtr) / 0x394);
        if (slot) {
            AsCharacter(slot)->Key = entityId;
            // Combat_PacketDispatch (00429690) applies the packed
            // la clase/estado del jugador y después ChangeCharacterExt con
            // CharSet[1..10]. Sin esto la entidad existe pero
            // has no faithful class/equipment visual setup.
            const BYTE charSet0 = e[4];
            AsCharacter(slot)->Class = (BYTE)(((charSet0 >> 4) | (charSet0 & 0x10)) >> 1);
            AsCharacter(slot)->Skin = 0;
            AsCharacter(slot)->PK = dirpk & 0x0F;
            switch (charSet0 & 0x0F) {
            case 1:
                CreateTeleportEnd(AsObject(slot));
                break;
            case 2:
                SetAction(AsObject(slot),
                             (AsCharacter(slot)->Class & 7) == 2 ? 135 : 133);
                break;
            case 3:
                SetAction(AsObject(slot),
                             (AsCharacter(slot)->Class & 7) == 2 ? 140 : 139);
                break;
            case 4:
                SetAction(AsObject(slot),
                             (AsCharacter(slot)->Class & 7) == 2 ? 138 : 137);
                break;
            default:
                break;
            }
            ChangeCharacterExt(spawnSlot, (BYTE*)e + 5);
            EntityName::Set(slot, name);
            AsCharacter(slot)->Object.Kind = 1;     // type: player (1)
            AsCharacter(slot)->Object.Visible = 1;    // visible flag — sin esto Entity_RenderAll_3D skip
            AsCharacter(slot)->Object.LightEnable = 1;     // also "is rendered" sub-flag
            AsCharacter(slot)->JumpTime = 0;
            AsCharacter(slot)->TargetX = tx;
            AsCharacter(slot)->TargetY = ty;
            AsCharacter(slot)->PathNum = 0;    // path_wp_count
            AsCharacter(slot)->PositionX = x;
            AsCharacter(slot)->PositionY = y;
            AsCharacter(slot)->Object.Live = 1;        // active flag
            if ((e[0] & 0x80) != 0) {
                // CreateFlag branch in Combat_PacketDispatch:
                // la posición llegue de forma autoritativa, y recién ahí emitir el
                // standard player teleport-in effect (1265).
                CreateEffect(1265, (float*)(slot + 16), (float*)(slot + 28),
                             (float*)(slot + 232), nullptr, (float*)slot,
                             (float*)-1, nullptr, 0);
                *(DWORD *)&AsCharacter(slot)->Object.Alpha = 0;
            }
            // El viewport trae efectos que ya estaban activos
            // antes de que esta entidad se hiciera visible. Le pasamos el
            // bitfield through InsertBuffPhysicalEffect semantics
            // para que Mana Shield y el resto de los buffs persistentes
            // appear without waiting for another 0x07 transition.
            ApplyPersistentSkillEffect97k(slot, viewSkillState, 1);
            NetLog("NET:    0x12 spawn player slot=%d id=%d name=%s pos=(%d,%d)",
                   spawnSlot, entityId, name, x, y);
        }
    }
    // 0x42 can precede this viewport batch after a relog or map
    // transition. Rebuild only Party's runtime name links now
    // that entities exist; membership remains server-authoritative.
    Party_RefreshViewportLinks();
}

// 0x45
void NetRecv_45(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ViewportChange lo emite MuEmu cuando una entidad ya visible
    // player changes skin.  Unlike the old client packet, MuEmu's
    // el PMSG_VIEWPORT_CHANGE nativo (Viewport.h, 0.97.40) son 62 bytes:
    //   id(2), x, y, skin, pad, ViewSkillState(LE), name(40),
    //   tx, ty, dir|pk, charset(11).
    // ReceiveCreateTransformViewport (00429C50) creates the
    // la forma de monstruo transformada para esta clave; no es apenas
    // un flag de skin sobre la entidad de jugador existente.
    const int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
    if (Size <= 3 + hdrOff) return;

    const int count = Msg[3 + hdrOff];
    const int entryStart = 4 + hdrOff;
    constexpr int NAME = (int)NetText::CharacterNameSize;
    constexpr int entryStride = 22 + NAME;
    if (count <= 0 || count > 255 || entryStart + count * entryStride > Size) {
        NetLog("NET:    0x45 SKIP count=%d size=%d", count, Size);
        return;
    }

    int transformed = 0;
    for (int i = 0; i < count; ++i) {
        const BYTE* const e = Msg + entryStart + i * entryStride;
        const WORD entityId = (WORD)(((e[0] & 0x7F) << 8) | e[1]);
        const BYTE skin = e[4];
        const WORD viewSkillState = (WORD)(e[6] | (e[7] << 8));

        BYTE* const entity = (BYTE*)CreateMonster(skin, e[2], e[3], entityId, 0);
        if (!entity) continue;

        // Campos de estado exactos de 00429C50, adaptados sólo para el
        // MuEmu record offsets above.
        if (entity[747] == 7) *(float*)(entity + 12) = 0.8f;
        if (DAT_07abf5d8) entity[444] = ((BYTE*)DAT_07abf5d8)[444];
        entity[746] = e[10 + NAME] & 0x0F;
        entity[132] = 1;
        entity[847] = 1;
        entity[0x160] = 1; // required by this renderer's active pass
        entity[0xdc] = 1;
        entity[0x305] = 0;
        entity[0x306] = e[8 + NAME];
        entity[0x307] = e[9 + NAME];
        *(float*)(entity + 36) = ((float)(e[10 + NAME] >> 4) - 1.0f) * 45.0f;
        *(int*)(entity + 0x388) = e[2];
        *(int*)(entity + 0x38c) = e[3];
        {
            char name[NetText::CharacterNameSize + 1];
            NetText::FromServer(name, sizeof(name), e + 8, NAME);
            EntityName::Set(entity, name);
        }
        ApplyPersistentSkillEffect97k(entity, viewSkillState, 1);

        if ((e[0] & 0x80) != 0) {
            // Rama CreateFlag: efecto de transformación 233 más su
            // partícula 1191, exactamente como el handler original.
            CreateEffect(233, (float*)(entity + 16), (float*)(entity + 28),
                         (float*)(entity + 232), nullptr, (float*)entity,
                         (float*)-1, nullptr, 0);
            CreateSprite(1191, (float*)(entity + 16), 1.0f,
                         (float*)(entity + 232), (int)(uintptr_t)entity,
                         0.0f, 0);
            *(DWORD*)(entity + 360) = 0;
        }
        ++transformed;
    }
    NetLog("NET: -> 0x45 ViewportChange count=%d transformed=%d", count, transformed);
}

// 0x13
void NetRecv_13(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Mismo enmarcado C1/C2 que en 0x12.
    int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
    NetLog("NET:  → 0x13 ViewportMonster count=%d size=%d hdr=%02X",
           Msg[3 + hdrOff], Size, Msg[0]);
    // No descartar el viewport si el heroe aun no esta creado (al entrar al mapa
    // el server lo manda junto con el spawn del heroe).
    //
    // IDA `ReceiveCreateMonsterViewport` (0x42A230) no tiene ese
    // guard, y este handler no usa `DAT_07abf5d8` en ningun lado de
    // su cuerpo: lo unico que necesita es el ARRAY de entidades.
    if (!DAT_07abf5d0) {
        NetLog("NET:    0x13 SKIP - entity array no alocado");
        return;
    }
    int count = Msg[3 + hdrOff];
    if (count <= 0 || count > 255) {
        NetLog("NET:    0x13 SKIP - count=%d out of range", count);
        return;
    }
    int entryStart = 4 + hdrOff;
    // El stride es FIJO 12, no derivado del tamano.
    // IDA `ReceiveCreateMonsterViewport` (0x0042A230): `Data2 =
    // ReceiveBuffer + 7;` y al final del cuerpo del do-while
    // `Data2 += 12;`.  Derivarlo de `(Size - entryStart) / count` desalinea
    // TODAS las entradas desde la segunda ante cualquier desajuste de `Size`
    // (el `type` sale de un byte que no es el suyo → monstruos fantasma).
    const int entryStride = 12;
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int i = 0; i < count && (entryStart + i*entryStride + 11) <= Size; ++i) {
        const BYTE* e = Msg + entryStart + i*entryStride;
        // Layout decompilado del binario MuEmu
        // Linux server (CViewport::GCViewportMonsterSend, asm
        // a17c0). Confirmado disasm:
        //   e[0]=KeyH|CREATE, e[1]=KeyL, e[2]=Class,
        //   e[3]=padding, e[4..5]=ViewSkillState (LE),
        //   e[6]=PosX, e[7]=PosY, e[8]=TargetX, e[9]=TargetY,
        //   e[10]=Path|Dir, e[11]=padding.
        WORD entityId = ((WORD)(e[0] & 0x7F) << 8) | e[1];
        BYTE type = e[2];
        BYTE x = e[6], y = e[7];
        BYTE tx = e[8], ty = e[9];
        BYTE dirpk = e[10];
        BYTE dir = (dirpk >> 4) & 0x0F;
        const WORD viewSkillState = (WORD)(e[4] | (e[5] << 8));
        // CreateMonster (no CreateCharacterPointer): ADEMÁS carga el BMD model via
        // OpenMonsterModel/OpenNpc; sin modelo el slot no se dibuja.
        extern char* __cdecl CreateMonster(unsigned int Type, int PosX,
                                         int PosY, int Key, int);
        float rot = ((float)dir - 1.0f) * 45.0f;
        char* slotChar = CreateMonster((unsigned int)type, x, y,
                                      (int)entityId, 0);
        if (slotChar) {
            BYTE* slot = (BYTE*)slotChar;
            // El facing va por separado — CreateMonster no lo setea.
            AsCharacter(slot)->Object.Angle[2] = rot;
            AsCharacter(slot)->Key = entityId;
            // NO sobrescribir +0x84 (kind).  CreateMonster ya lo setea correcto por
            // Type: 2=monster, 4=NPC (type>200), 8=ground-item.  Forzarlo a 2 haría
            // que Target_Render muestre a los NPCs con el banner de monstruo
            // (RenderCenteredText arriba) en vez del chat flotante de NPC (CreateChat).
            AsCharacter(slot)->Object.Visible = 1;    // visible flag
            AsCharacter(slot)->Object.LightEnable = 1;     // is_rendered sub-flag
            AsCharacter(slot)->JumpTime = 0;
            AsCharacter(slot)->TargetX = tx;
            AsCharacter(slot)->TargetY = ty;
            AsCharacter(slot)->PathNum = 0;
            AsCharacter(slot)->PositionX = x;
            AsCharacter(slot)->PositionY = y;
            AsCharacter(slot)->Object.Live = 1;
            // Init +0x168 (screen distance) a 1.0f para
            // que mob sea targetable INMEDIATAMENTE (antes del primer
            // frame de render). Sin esto, FUN_004afdc0 (hover detect)
            // filtra mob por `_DAT_00552580 < ent[+0x168]` (= 0
            // por default) → mob no es hovered hasta que sea
            // rendered (1+ frame later).
            AsCharacter(slot)->Object.Alpha = 1.0f;
            // Init move speed (+0x2FA = +762 word).
            // CreateMonster solo setea esto para case 11. Resto de
            // los tipos quedan en 0 → CharacterMoveSpeed retorna 0 →
            // sin movimiento. O queda en basura → mobs acelerados.
            // 4 = "slow walk" baseline para mobs/NPCs (vs player ~12).
            if (AsCharacter(slot)->MoveSpeed == 0) {
                AsCharacter(slot)->MoveSpeed = 4;
            }
            ApplyPersistentSkillEffect97k(slot, viewSkillState, 1);
            int spawnSlot = (int)((slot - basePtr) / 0x394);
            NetLog("NET:    0x13 spawn monster slot=%d id=%d type=%d pos=(%d,%d) rot=%.0f",
                   spawnSlot, entityId, type, x, y, rot);
        } else {
            NetLog("NET:    0x13 SKIP - CreateMonster returned NULL (type=%d)",
                   type);
        }
    }
}

// 0x14
void NetRecv_14(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PMSG_VIEWPORT_DESTROY_SEND (MuEmu):
    //   [C1][size][14][count][index_hi][index_lo]...
    // Fiel a ProtocolCore 004389A0:606-618: cada índice de
    // 15 bits se manda a DeleteCharacter (0045AC20), que es dueña
    // the slot/butterfly/cloth teardown.
    const int count = Msg[3];
    const int available = (Size >= 4) ? (Size - 4) / 2 : 0;
    const int entries = (count < available) ? count : available;
    for (int i = 0; i < entries; ++i) {
        const int offset = 4 + i * 2;
        const int entityId = ((Msg[offset] << 8) | Msg[offset + 1]) & 0x7FFF;
        DeleteCharacter(entityId);
    }
    NetLog("NET:  -> 0x14 ViewportDestroy count=%d processed=%d size=%d",
           count, entries, Size);
}

// 0x1F
void NetRecv_1F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveCreateSummonViewport @ 0042A530.  MuEmu emits this
    // como C2:1F, seguido de un contador y registros alineados de 52 bytes
    // (0.97.40):
    //   KeyH|CREATE, KeyL, Type, pad, ViewSkillState(WORD),
    //   X, Y, TargetX, TargetY, Dir|PK, OwnerName[40], pad.
    // Padding en los offsets 3 y 51; OwnerName arranca en +11.
    // IDA 0x42A530 copia desde ReceiveBuffer+16 (la entrada arranca en +5).
    const int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
    const int countOff = 3 + hdrOff;
    const int entryStart = 4 + hdrOff;
    const int NAME = (int)NetText::CharacterNameSize;
    const int entryStride = 12 + NAME;
    // Idem 0x12/0x13: no exigir el heroe (`DAT_07abf5d8`) para procesar el
    // paquete al entrar al mapa. Este handler tampoco usa el heroe.
    if (Size <= countOff || !DAT_07abf5d0) return;

    const int count = Msg[countOff];
    if (count <= 0 || count > 255 ||
        entryStart + count * entryStride > Size) {
        NetLog("NET:    0x1F SKIP - malformed summon viewport count=%d size=%d",
               count, Size);
        return;
    }

    BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int i = 0; i < count; ++i) {
        const BYTE* e = Msg + entryStart + i * entryStride;
        const bool create = (e[0] & 0x80) != 0;
        const WORD key = (WORD)(((e[0] & 0x7F) << 8) | e[1]);
        const BYTE type = e[2];
        const WORD viewSkillState = (WORD)(e[4] | (e[5] << 8));
        const BYTE x = e[6], y = e[7];
        const BYTE tx = e[8], ty = e[9];
        const BYTE dirPk = e[10];
        BYTE* summon = (BYTE*)CreateMonster(type, x, y, key, 0);
        if (!summon) {
            NetLog("NET:    0x1F SKIP - CreateMonster NULL type=%d", type);
            continue;
        }

        // Mantener esto sincronizado con el setup normal del viewport de
        // monstruos 0x13, y después aplicar la etiqueta/estado propios del invocado.
        AsCharacter(summon)->Object.Angle[2] = ((float)((dirPk >> 4) & 0x0F) - 1.0f) * 45.0f;
        AsCharacter(summon)->Key = key;
        AsCharacter(summon)->Object.Visible = 1;
        AsCharacter(summon)->Object.LightEnable = 1;
        AsCharacter(summon)->JumpTime = 0;
        AsCharacter(summon)->TargetX = tx;
        AsCharacter(summon)->TargetY = ty;
        AsCharacter(summon)->PathNum = 0;
        AsCharacter(summon)->PositionX = x;
        AsCharacter(summon)->PositionY = y;
        AsCharacter(summon)->Object.Alpha = 1.0f;
        if (AsCharacter(summon)->MoveSpeed == 0) AsCharacter(summon)->MoveSpeed = 4;
        AsCharacter(summon)->Object.Kind = 1;
        AsCharacter(summon)->PK = dirPk & 0x0F;
        if ((dirPk & 0x0F) >= 6) AsCharacter(summon)->Level = 1;

        // El cliente original reemplaza la etiqueta de la criatura con
        // "<owner name>'s <old monster name>".  Preserve the
        // important owner identity without relying on localized
        // el storage de GlobalText mientras se portea el subsistema de texto.
        // El nombre compuesto entra entero en EntityName (antes se cortaba en
        // los 25 bytes de CHARACTER::ID).
        const std::string oldName = EntityName::Get(summon);
        char owner[NetText::CharacterNameSize + 1] = {};
        NetText::FromServer(owner, sizeof(owner), e + 11, NAME);
        const std::string label = std::string(owner) + "'s " + oldName;
        EntityName::Set(summon, label.c_str());
        AsCharacter(summon)->Object.Live = 1;
        ApplyPersistentSkillEffect97k(summon, viewSkillState, 1);

        if (create) AppearMonster((DWORD)(uintptr_t)summon);
        NetLog("NET:    0x1F summon id=%d type=%d owner=%s pos=(%d,%d)",
               key, type, owner, x, y);
    }
}

// 0x25
void NetRecv_25(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Cambio de una pieza de equipo de un jugador del viewport.
    Recv_ChangePlayer(Msg, Size);
}

// ── 0x20 ViewportItem (ground items) ────────────────────────────
// Port FIEL del IDA ReceiveCreateItemViewport @ 0x0042F240.
// Spawnea cada item en DAT_07e12840 pool; el render lo hace
// Entity_Render (Entity_Render) que itera el pool por slots activos.
// 0x20
void NetRecv_20(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
    int count = Msg[3 + hdrOff];
    NetLog("NET:  → 0x20 ViewportItem count=%d size=%d", count, Size);
    int cursor = 4 + hdrOff;
    // Cada entrada del pool es un ITEM_t (structs.h): Key, Item y Object.
    BYTE* itemPool = (BYTE*)&DAT_07e12840[0];
    extern int  __cdecl ItemObjectAttribute(int);   // ItemObjectAttribute
    extern void __cdecl ItemAngle(int);   // ItemAngle
    // DESVIACION: el server manda PMSG_VIEWPORT_ITEM (Viewport.h) con
    // stride fijo index[2] + x + y + ItemInfo[MAX_ITEM_INFO + 1] = 12,
    // no el stride variable del 0.97k. Bound y avance usan el mismo valor.
    const int entrySize = 4 + ITEM_INFO_SIZE + 1;
    for (int i = 0; i < count && cursor + entrySize <= Size; ++i) {
        const BYTE* e = Msg + cursor;
        WORD raw = (e[0] << 8) | e[1];
        WORD key = raw & 0x7FFF;
        bool createFlag = (raw & 0x8000) != 0;
        BYTE gx = e[2], gy = e[3];
        const BYTE* itemInfo = e + 4;
        int itemType = ItemWire_GetType(itemInfo);

        if (key >= 1000) key = 0;  // safety clamp per IDA

        BYTE* ip = itemPool + (size_t)key * sizeof(ITEM_t);
        RenderSnapshots_NewItem(key);
        ITEM_t* gi = AsGroundItem(ip);
        OBJECT* o = &gi->Object;

        *(WORD *)&gi->Item.Type = (WORD)itemType;   // sin el +400 del modelo
        if (itemType == 463) {
            // Zen (GET_ITEM(14,15)): la CANTIDAD viaja en 24 bits
            // repartidos en info[1] (bits 16-23), info[2] (8-15) e
            // info[4] (0-7) — ver Viewport.cpp:926 del server.
            // Queda en Item.Level, que es de donde la leen Entity_Render
            // (para el tamaño del montón de monedas) y RenderItemName
            // (para el texto "Zen <cantidad>").
            int v5 = (int)itemInfo[4] + (((int)itemInfo[2] + ((int)itemInfo[1] << 8)) << 8);
            gi->Item.Level = v5;
            gi->Item.Durability = 0;
            gi->Item.Option1 = 0;
            if (createFlag) PlayBuffer(31, 0, 0);   // pDropMoney.wav
        } else {
            gi->Item.Level = (int)itemInfo[1];
            gi->Item.Durability = itemInfo[2];
            gi->Item.Option1 = itemInfo[3];
            if (createFlag) {
                // IDA CreateItem L38-46: joyas y pergaminos suenan
                // distinto que el resto.
                if (itemType == 461 || itemType == 462 || itemType == 464 ||
                    itemType == 399 || itemType == 470)
                    PlayBuffer(49, AsObject(o), 0);
                else
                    PlayBuffer(30, AsObject(o), 0);
            }
        }
        o->Live = 1;
        *(WORD *)&o->Type = (WORD)ItemModel(itemType);   // modelo (0.97.20: catálogo)
        o->SubType = 1;

        // Model overrides (CreateItem switch L58-126): arrows/fruit/etc
        // cuyo modelo NO es type+400.
        {
            int lvl = gi->Item.Level >> 3;
            if (itemType == 459) {                     // arrows
                switch (lvl) {
                    case 1: *(WORD*)(ip + 74) = 951; break;
                    case 2: *(WORD*)(ip + 74) = 952; break;
                    case 3: *(WORD*)(ip + 74) = 953; break;
                    case 5: *(WORD*)(ip + 74) = 955; break;
                    case 6: *(WORD*)(ip + 74) = 956; break;
                    case 8: case 9: case 10: case 11: case 12:
                            *(WORD*)(ip + 74) = 957; break;
                }
            } else if (itemType == 469) {
                if (lvl == 1) *(WORD*)(ip + 74) = 958;
            } else if (itemType == 435) {              // fruit
                if (lvl == 0)      { *(WORD*)(ip + 74) = 570; *(int*)(ip + 8) = 0; }
                else if (lvl == 1) { *(WORD*)(ip + 74) = 419; *(int*)(ip + 8) = 0; }
                else if (lvl == 2) { *(WORD*)(ip + 74) = 546; *(int*)(ip + 8) = 0; }
            } else if (itemType == 457 && lvl == 1) {
                *(WORD*)(ip + 74) = 954;
            }
        }

        // ItemObjectAttribute (CreateItem LABEL_33): setea atributos
        // de render del objeto. Sin esto el modelo puede
        // quedar sin scale/flags → invisible.
        ItemObjectAttribute((int)(uintptr_t)o);

        // BoundingBox del objeto (IDA CreateItem L129-134):
        // min = (-30,-30,-30), max = (30,30,30).
        o->BoundingBoxMin[0] = -30.0f;
        o->BoundingBoxMin[1] = -30.0f;
        o->BoundingBoxMin[2] = -30.0f;
        o->BoundingBoxMax[0] = 30.0f;
        o->BoundingBoxMax[1] = 30.0f;
        o->BoundingBoxMax[2] = 30.0f;

        // World position: ((grid + 0.5) * 100.0)
        o->Position[0] = ((float)gx + 0.5f) * 100.0f;
        o->Position[1] = ((float)gy + 0.5f) * 100.0f;
        // Z: terrain height (RequestTerrainHeight 0x004F7500).
        extern float __cdecl RequestTerrainHeight(float, float);
        float terrainH = RequestTerrainHeight(o->Position[0], o->Position[1]);

        // Caída del item recién dropeado (IDA CreateItem L139-172):
        // nace por encima del suelo con velocidad Z en Gravity y
        // MoveItems (0x503760) lo hace caer y rebotar.  Sin esto el
        // item aparecía clavado en el suelo, sin animación de drop.
        // Omitido: los CreateEffect(250)/CreateEffect(248) de las
        // flechas/bolts (modelos 955/956), que necesitan los args
        // vec3 de CreateEffect.
        if (createFlag) {
            WORD model = *(WORD *)&o->Type;
            if (model == 955 || model == 956) {
                o->Gravity     = 50.0f;
                o->Position[2] = terrainH + 3.0f;
            } else {
                o->Gravity     = 20.0f;
                o->Position[2] = terrainH + 180.0f;
            }
        } else {
            o->Position[2] = terrainH;
        }

        // ItemAngle (CreateItem final): setea rotación del item en
        // el suelo según el terreno.
        ItemAngle((int)(uintptr_t)o);

        NetLog("NET:    0x20 item[%d] key=%u type=%d mine=%d pos=(%d,%d)",
               i, key, itemType, createFlag, gx, gy);

        cursor += entrySize;
    }
}

// ── 0x21 ViewportItemDestroy (item picked up / disappeared) ─────
// 0x21
void NetRecv_21(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
    int count = Msg[3 + hdrOff];
    NetLog("NET:  → 0x21 ViewportItemDestroy count=%d", count);
    // Limpiar slots en DAT_07e12840 pool. Per-entry
    // 2 bytes (key WORD, big-endian).
    int entryStart = 4 + hdrOff;
    BYTE* itemPool = (BYTE*)&DAT_07e12840;
    for (int i = 0; i < count && entryStart + i*2 + 1 < Size; ++i) {
        WORD raw = (Msg[entryStart + i*2] << 8) | Msg[entryStart + i*2 + 1];
        WORD key = raw & 0x7FFF;
        if (key < 1000) {
            itemPool[(size_t)key * 0x204 + 72] = 0;  // active flag = 0
        }
    }
}
