// Item_ServerValue.cpp
//
// Precios de items compatibles con el server MuEmu.
//
// El 0.97k calcula precio de compra/venta y costo de reparacion en el cliente
// (ItemValue 0x0047C690, ConvertRepairGold 0x004C3EF0), pero MuEmu usa otra
// formula (GameServer/Item.cpp, CItem::Value) y ademas manda dos tablas propias:
// F3/E3 (maximo de apilado) y F3/E4 (precios fijos de ItemValue.txt).  Con la
// formula del binario el tooltip muestra un precio y el server paga otro: los
// items con opciones se venden por mucho mas de lo que dice el cartel.
//
// El DLL de inyeccion lo arregla reemplazando las dos funciones
// (Source/Client/Main/Item.cpp: SetCompleteHook en 0x0047C690 y 0x004C3EF0) y
// recibiendo las dos tablas (Protocol.cpp, F3/E3 y F3/E4).  Esto es ese fix,
// portado.  Los ports de IDA quedan en Item_Durability.cpp como
// ItemValue_Vanilla / ConvertRepairGold_Vanilla.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <vector>

int __cdecl ItemStack_GetMaxStack(int index, int level);

extern "C" void DbgLogPublic(const char* msg);

namespace {

struct ItemStackInfo { int Index; int Level; int MaxStack; };
struct ItemValueInfo { int Index; int Level; int BuyValue; int SellValue; };

std::vector<ItemStackInfo> s_ItemStack;
std::vector<ItemValueInfo> s_ItemValue;

const unsigned long long kMaxItemPrice = 2000000000ULL;   // MAX_ITEM_PRICE

inline int ItemLevelOf(const ITEM* ip) { return (ip->Level >> 3) & 0xF; }

// Encabezado PSWMSG_HEAD (C2, size[2], head, subhead) + BYTE count = 6 bytes.
template <typename T>
void ReadList(std::vector<T>& out, const BYTE* msg, int size)
{
    out.clear();
    if (size < 6) return;
    const int count = msg[5];
    for (int n = 0; n < count; ++n) {
        const int off = 6 + (int)sizeof(T) * n;
        if (off + (int)sizeof(T) > size) break;
        T info;
        memcpy(&info, msg + off, sizeof(T));
        out.push_back(info);
    }
}

// CItem::myCalcMaxDurability del DLL.
int CalcMaxDurability_MuEmu(const ITEM* ip, const ITEM_ATTRIBUTE* p, int level)
{
    if (ip->Type == 14 * 32 + 21) return 1;                          // Rena
    if (ip->Type == 13 * 32 + 18 || ip->Type == 14 * 32 + 19) return 1;  // Capa / Invitacion

    int dur = p->Durability;
    if (ip->Type >= 5 * 32 && ip->Type < 6 * 32) dur = p->MagicDurability;   // baculos

    if (level >= 5) {
        if (level == 10)      dur += level * 2 - 3;
        else if (level == 11) dur += level * 2 - 1;
        else                  dur += level * 2 - 4;
    } else {
        dur += level;
    }

    if (ip->Type != 19 && ip->Type != 4 * 32 + 18 && ip->Type != 5 * 32 + 10
        && ip->Part != 7 /* EQUIPMENT_WING */) {
        if ((ip->Option1 & 0x3F) != 0) dur += 15;
    }
    return dur > 255 ? 255 : dur;
}

bool FindValue(const ITEM* ip, bool sell, unsigned long long* value)
{
    const int level = ItemLevelOf(ip);
    for (const ItemValueInfo& it : s_ItemValue) {
        if (it.Index != ip->Type) continue;
        if (it.Level != -1 && it.Level != level) continue;
        const int v = sell ? it.SellValue : it.BuyValue;
        if (v == -1) continue;
        if (ItemStack_GetMaxStack(it.Index, level) == 0
            || it.Index == 4 * 32 + 7 || it.Index == 4 * 32 + 15) {
            *value = (unsigned long long)v;
        } else {
            *value = (unsigned long long)v * ip->Durability;
        }
        return true;
    }
    return false;
}

} // namespace

// F3/E3: lista de apilado (DLL CItemStack::GCItemStackListRecv).
void Recv_ItemStackList(const BYTE* Msg, int Size)
{
    ReadList(s_ItemStack, Msg, Size);
}

// F3/E4: precios fijos (DLL CItemValue::GCItemValueListRecv).
void Recv_ItemValueList(const BYTE* Msg, int Size)
{
    ReadList(s_ItemValue, Msg, Size);
}

int __cdecl ItemStack_GetMaxStack(int index, int level)
{
    for (const ItemStackInfo& it : s_ItemStack) {
        if (it.Index != index) continue;
        if (it.Level != -1 && it.Level != level) continue;
        return it.MaxStack;
    }
    return 0;
}

// CItem::ItemValue del DLL.  goldType 1 = precio de venta; cualquier otro valor
// devuelve el de compra (el 2 que pasa la reparacion incluido).
int __cdecl ItemValue_MuEmu(void* item, int goldType)
{
    const ITEM* ip = (const ITEM*)item;
    if (!ip || ip->Type == -1) return 0;
    const unsigned int attrBase = ItemAttribute_Base();
    if (!attrBase || ip->Type < 0 || ip->Type >= 1024) return 0;
    const ITEM_ATTRIBUTE* info = (const ITEM_ATTRIBUTE*)(uintptr_t)(attrBase + ip->Type * sizeof(ITEM_ATTRIBUTE));

    const int  itemLevel = ItemLevelOf(ip);
    const bool skill     = ((ip->Level >> 7) & 1) != 0;
    const bool luck      = ((ip->Level >> 2) & 1) != 0;
    const int  addOption = (ip->Level & 3) + ((ip->Option1 & 64) >> 4);

    if (info->Money != 0) {
        unsigned int buy = info->Money;
        buy = buy >= 100 ? buy / 10 * 10 : buy;
        buy = buy >= 1000 ? buy / 100 * 100 : buy;
        unsigned int sell = info->Money / 3;
        sell = sell >= 100 ? sell / 10 * 10 : sell;
        sell = sell >= 1000 ? sell / 100 * 100 : sell;
        return (int)(goldType == 1 ? sell : buy);
    }

    unsigned long long price = 0;
    if (FindValue(ip, false, &price)) {
    } else if (info->Value > 0) {
        price = (unsigned long long)info->Value * info->Value * 10 / 12;
        if (ip->Type >= 14 * 32 && ip->Type <= 14 * 32 + 8) {
            if (ip->Type == 14 * 32 + 3 || ip->Type == 14 * 32 + 6) price *= 2;
            price *= 1ULL << itemLevel;
            price *= ip->Durability;
            if (price > kMaxItemPrice) price = kMaxItemPrice;
            unsigned int buy = (unsigned int)price;
            buy = buy >= 10 ? buy / 10 * 10 : buy;
            unsigned int sell = (unsigned int)(price / 3);
            sell = sell >= 10 ? sell / 10 * 10 : sell;
            return (int)(goldType == 1 ? sell : buy);
        }
    } else {
        unsigned long long lvl = info->Level + itemLevel * 3;
        for (int n = 0; n < ip->SpecialNum; ++n) {
            if (ip->Special[n] != 0 && ip->Type < 12 * 32) { lvl += 25; break; }
        }

        const int group = ip->Type / 32;
        if ((group == 12 && ip->Type > 12 * 32 + 6) || group == 13 || group == 15) {
            price = lvl * lvl * lvl + 100;
            for (int n = 0; n < ip->SpecialNum; ++n) {
                if (ip->Special[n] == 65 /* ITEM_OPTION_ADD_HP_RECOVERY_RATE */) {
                    price += price * addOption;
                    break;
                }
            }
        } else {
            switch (itemLevel) {
            case 5:  lvl += 4;   break;
            case 6:  lvl += 10;  break;
            case 7:  lvl += 25;  break;
            case 8:  lvl += 45;  break;
            case 9:  lvl += 65;  break;
            case 10: lvl += 95;  break;
            case 11: lvl += 135; break;
            default: break;
            }

            if (ip->Type >= 12 * 32 && ip->Type <= 12 * 32 + 6)
                price = (lvl + 40) * lvl * lvl * 11 + 40000000;
            else
                price = (lvl + 40) * lvl * lvl / 8 + 100;

            if (ip->Type >= 0 && ip->Type < 6 * 32 && info->TwoHand == 0)
                price = price * 80 / 100;
            if (skill) price += price * 25 / 100;
            if (luck)  price += price * 25 / 100;
            if (addOption == 1) price += price * 60 / 100;
            if (addOption == 2) price += price * 140 / 100;
            if (addOption == 3) price += price * 280 / 100;
            if (addOption == 4) price += price * 560 / 100;
            for (int n = 0; n < 6; ++n) {
                if ((ip->Option1 & (1 << n)) != 0)
                    price += ip->Type < 12 * 32 ? price * 100 / 100 : price * 25 / 100;
            }
        }
    }

    unsigned int buy = (unsigned int)price;
    buy = buy >= 100 ? buy / 10 * 10 : buy;
    buy = buy >= 1000 ? buy / 100 * 100 : buy;

    const float baseDur = (float)CalcMaxDurability_MuEmu(ip, info, itemLevel);
    if (!FindValue(ip, true, &price)) price = price / 3;

    unsigned int sell = (unsigned int)price;
    if (ip->Part <= 11 /* EQUIPMENT_WEAPON_RIGHT..EQUIPMENT_RING_LEFT */ && baseDur > 0.0f)
        sell = sell - (unsigned int)((sell * 0.6) * (1 - (ip->Durability / baseDur)));
    sell = sell >= 100 ? sell / 10 * 10 : sell;
    sell = sell >= 1000 ? sell / 100 * 100 : sell;

    return (int)(goldType == 1 ? sell : buy);
}

// CItem::ConvertRepairGold del DLL.  Gold llega como precio de compra.
unsigned int __cdecl ConvertRepairGold_MuEmu(int Gold, int Durability, int MaxDurability, short Type, char* Text)
{
    (void)Type;
    const float dur     = (float)Durability;
    const float baseDur = (float)MaxDurability;

    long long repair = Gold / 3;
    if (repair > (long long)kMaxItemPrice) repair = kMaxItemPrice;
    repair = repair >= 100 ? repair / 10 * 10 : repair;
    repair = repair >= 1000 ? repair / 100 * 100 : repair;

    const float sq1 = (float)sqrt((float)repair);
    const float sq2 = (float)sqrt(sq1);
    float value = 3.0f * sq1 * sq2 * (1.0f - (baseDur > 0.0f ? dur / baseDur : 1.0f)) + 1.0f;
    if (dur <= 0) value *= 1.4f;

    int money = (int)(DAT_07eaa138 == 1 ? value * 2.5f : value);   // RepairEnable
    money = money >= 100 ? money / 10 * 10 : money;
    money = money >= 1000 ? money / 100 * 100 : money;

    if (Text) {
        if (money >= 1000000)
            sprintf(Text, "%d,%03d,%03d", money / 1000000, money / 1000 % 1000, money % 1000);
        else if (money >= 1000)
            sprintf(Text, "%d,%03d", money / 1000, money % 1000);
        else
            sprintf(Text, "%d", money);
    }
    return (unsigned int)money;
}
