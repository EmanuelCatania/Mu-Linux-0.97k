#pragma once
// NetRecv.h — handlers de los paquetes que manda el server.
//
// IDA: ProtocolCore (0x004389A0) despacha por opcode a una función ReceiveXxx
// por paquete. Acá el dispatcher es Net_ProcessPacket (Net/Net_Process.cpp) y
// cada opcode tiene su NetRecv_XX, agrupado por tema en Net/Recv/Recv_*.cpp.
// Todos reciben el paquete ya desencriptado y re-enmarcado como C1/C2:
//   Msg = [C1][len][opcode]...  (o C2 con el tamaño en dos bytes)
//   Size = tamaño real, hdr = C1/C2, sub = el byte que sigue al opcode,
//   bEncrypted = llegó como C3/C4.

#include "Net/Net.h"
#include "Net/HWID.h"
#include "Net/MuEmu.h"
#include "Party/Party.h"

// ── Símbolos de otros módulos que usan los handlers ─────────────────────────

// Definida en Item/Item_ClickHandler.cpp — devuelve el item agarrado a su slot
// de origen. La usa el rechazo de venta del handler 0x33.
void RestorePickedItemToSource(void);

// 0x00474310. Queda acá porque el paquete 0x1C es el fin autoritativo de un
// Teleport local, después de que el paquete de skill 0x19 arrancó su animación.
extern "C" void __cdecl CreateTeleportEnd(unsigned int entity);

extern "C" void __cdecl CreatePoint(float Position[3], int Value,
                                    float Color[3], float scale);

// ============================================================================
// Net_ProcessPacket @ 0x004389A0 — server→client opcode dispatcher
// ============================================================================
//
// Estructura portada desde IDA (ProtocolCore, 1824 líneas, 489 basic blocks):
//   while ((Msg = CWsctlc::GetReadMsg(&SocketClient)) != NULL) {
//       bEncrypted = 0;
//       if (Msg[0] == 0xC1)  { HeadCode = Msg[2]; Size = Msg[1]; }
//       else if (Msg[0]==0xC2) { HeadCode = Msg[3]; Size = Msg[1]*256+Msg[2]; }
//       else if (Msg[0]==0xC3 o 0xC4) { /* desencripta in situ */ ... }
//       dispatch on HeadCode...
//   }
// ============================================================================

extern int __fastcall CWsctlc_GetReadMsg(int poolBase);   // GetReadMsg (Scene/Scene_CharSelect_Nav.cpp) — returns ptr as int

// Forward decls for inventory packet handlers (Item/Item_Inventory.cpp).
extern "C" void __cdecl Recv_Inventory     (const BYTE* Msg);   // F3/10
extern "C" void __cdecl Recv_InventoryOpen (const BYTE* Msg);   // 0x55
extern "C" void __cdecl Recv_InventoryClose(const BYTE* Msg);   // 0x54
extern "C" void HeroEquipWatchdog(int c);
extern "C" void __cdecl SeedQuickPotionTypesFromInventory(void);
extern "C" void SetGuildNoticeText(const char* text);

// Inventory/warehouse pool symbols (defined in HUD_Pass3.cpp).
extern "C" BYTE OffsetInventoryItems[];
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];
extern "C" BYTE Inventory[];
extern "C" BYTE ShopItems[];   // pool dedicado de la tienda (120 slots)
extern "C" void DbgLogPublic(const char* msg);
int __cdecl Entity_FindById(int entity_id);   // Entity/Entity_Lookup.cpp
extern "C" void __cdecl UI_Main(int slot_idx, short* inv_base,
                                 unsigned int gridW);  // Item_ClickHandler.cpp
extern "C" int  pPickedItem;
extern "C" int  Level;
extern "C" BYTE byte_7E9136B;
extern "C" int __cdecl ConvertItemType(BYTE* Item);
extern "C" void ChaosBoxCloseAck(void);

extern "C" int  g_nGuildMemberCount;
extern "C" int  GuildTotalScore;
extern "C" char byte_7E91790[];   // tabla de miembros, stride 13
extern "C" void GuildList_Clear(void);
extern "C" void GuildList_AddMember(const char* name, char connected, char partyNumber);
#define byte_7E919BC  DAT_07e919bc   // tabla global stride 80 (ver globals.h)
extern "C" void GuildCreator_OpenFromServer(void);
extern "C" void GuildCreator_OpenQuestionFromServer(void);
extern "C" void GuildCreator_CloseFromResult(void);
constexpr int kGuildMarkRecordCount = 1000;
extern "C" void DbgLogPublic(const char* msg);
static inline WORD ClampToWord(DWORD v) { return v >= 0xFFFF ? (WORD)0xFFFF : (WORD)v; }
extern "C" void DbgLogPublic(const char* msg);
extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType);
extern "C" void __cdecl DeleteEffect(int Type, DWORD Owner, int iSubType);
extern "C" void __cdecl DeleteCharacter(int Key);  // 0045AC20
extern "C" void __cdecl Item_TradeHistoryAdd(int slot, BYTE* pool);

// ── Helpers compartidos entre handlers ──────────────────────────────────────
void NetLog(const char* fmt, ...);
// Recv_Account.cpp
void ReceiveGGAuth97k(BYTE* packet, int size, bool encrypted);
void Recv_JoinServer(const BYTE* Msg);
void Recv_LoginResult(const BYTE* Msg);
void Recv_LogOut(const BYTE* Msg);
void Recv_CustomServerList(const BYTE* Msg);
void Recv_ServerList(const BYTE* Msg);
void Recv_Redirect(const BYTE* Msg);
void Recv_BackToConnecting(void);
// Recv_Character.cpp
void Recv_NewCharacterInfo(const BYTE* Msg);
extern BYTE s_PendingSkillKey[10];
extern bool s_HasPendingSkillKey;
void ApplySkillKeyMap(void);
void Recv_NewCharacterCalc(const BYTE* Msg, int Size);
extern unsigned short g_HeroKey;
extern bool s_IsRebuildingHero;
extern bool s_HasPendingHeroGuildMark;
extern short s_PendingHeroGuildMarkRow;
void Recv_CharList(const BYTE* Msg, int Size);
void Recv_CreateChar(const BYTE* Msg);
void Recv_DeleteChar(const BYTE* Msg);
void Recv_JoinMapServer(const BYTE* Msg, int bEncrypted);
void Recv_Revival(const BYTE* Msg, int Size);
void Recv_LevelUp(const BYTE* Msg, int Size);
// Recv_Viewport.cpp
int Net_LevelConvert(BYTE Level);
void Recv_ChangePlayer(const BYTE* Msg, int Size);
// Recv_Combat.cpp
void ApplyPersistentSkillEffect97k(BYTE* entity, WORD effect, BYTE state);
void CreateMagicShiny97k(BYTE* entity, int hand);
// Recv_Item.cpp
void ShopInsertItem(int slot, const BYTE* Item);
BYTE* ItemMove_GetPool(DWORD pool);
int ItemMove_GetGridH(BYTE* pool);
int ItemMove_ToGridSlot(BYTE* pool, int slot);
BYTE* ItemMove_GetEquipSlotPtr(int slot);
bool ItemMove_IsInventoryEquipSlot(BYTE* pool, int slot);
void ItemMove_RestoreSlot(BYTE* pool, int slot, const BYTE* item68);
void ItemMove_ApplyServerSlot(BYTE* pool, int slot, const BYTE* itemWire12, const BYTE* item68);
BYTE* ItemMove_GetActiveSecondaryPool();
void ItemMove_ClearPoolPreview(BYTE* pool, int slots);
void ItemMove_UpdateInventoryDurability(int slot, BYTE durability);
bool ItemMove_LooksLikeStackMerge(BYTE* targetPool, int targetSlot, const BYTE* sourceItem68);
void ItemMove_ClearPickedState();
extern bool s_bPacketAfterEquipmentItem;
extern BYTE s_byPacketAfterEquipmentItem[4];
// Recv_Trade.cpp
void ReceiveTradeExit97k(const BYTE* Msg, int Size);
// Recv_Guild.cpp
extern int  s_GuildRecordKey[kGuildMarkRecordCount];
BYTE* GuildMark_Record(int row);
int GuildMark_UpsertRecord(int key, const BYTE* name8, const BYTE* mark32);
extern "C" int GuildMark_FindRecordByKey(int key);
extern "C" const char* Guild_GetMarkName(int row);
extern "C" const BYTE* Guild_GetMarkPixels(int row);
void GuildWar_CopyOpponentName(const BYTE* packet);
int GuildMark_FindRecordByName(const char* name);
void __cdecl GuildWar_UpdateEntityRelation(int entityAddress, int, int, int);
void GuildWar_RefreshEntityRelations();
extern "C" void GuildWar_ResetClientState();
void ReceiveDeclareWar97k(const BYTE* packet, int size);
void ReceiveDeclareWarResult97k(const BYTE* packet, int size);
void ReceiveGuildBeginWar97k(const BYTE* packet, int size);
void ReceiveGuildEndWar97k(const BYTE* packet, int size);

// ── Handlers por opcode ─────────────────────────────────────────────────────
// Recv_Account.cpp
void NetRecv_F1(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_F4(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_0E(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_03(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_73(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Character.cpp
void NetRecv_F3(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_2C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_1C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_26(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_27(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_DD(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_DE(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_DF(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Viewport.cpp
void NetRecv_10(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_11(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_12(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_45(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_13(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_14(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_1F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_25(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_20(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_21(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Combat.cpp
void NetRecv_15(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_18(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_07(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_19(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_1E(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_1B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_16(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_17(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_9C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_1A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Item.cpp
void NetRecv_24(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_30(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_31(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_32(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_33(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_34(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_81(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_82(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_83(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_86(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_87(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_23(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_22(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_28(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_29(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_2A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_2F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Trade.cpp
void NetRecv_36(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_37(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_38(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_39(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_3A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_3B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_3C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_3D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Party.cpp
void NetRecv_44(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_40(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_41(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_42(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_43(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_71(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Guild.cpp
void NetRecv_53(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_54(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_55(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_56(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_50(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_51(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_52(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_5A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_5C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_5D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_60(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_61(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_62(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_63(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_64(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_5B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_65(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Chat.cpp
void NetRecv_01(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_02(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_0C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_0D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_00(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Event.cpp
void NetRecv_8E(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_8F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_90(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_91(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_92(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_93(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_94(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_95(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_96(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_97(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_9D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_99(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_9A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_9B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_0B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_Quest.cpp
void NetRecv_A0(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_A1(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_A2(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_A3(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
// Recv_World.cpp
void NetRecv_46(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
void NetRecv_0F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted);
