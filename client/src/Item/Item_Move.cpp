// Item_Move.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// SendRequestEquipmentItem @ 0x0043C250 — Equipment move / item drag
//
// El servidor exige Encrypt=1 para el opcode 0x24 (HackPacketCheck.txt:
// "36 * 1 0 0 0"); como C1 plano lo desconecta. Por eso va por
// `gNetwork.Send` (Game_SceneUpdate.cpp), el mismo helper que login
// (F1/01) y combat. Hace:
//   1. Chain XOR con s_LoginKey (i=3..len)
//   2. Stomp pkt[1] = DAT_05826ceb++ (serial counter — server valida que sea
//      monotónico vía CSerialCheck::CheckSerial)
//   3. CSimpleModulus encrypt vía CSimpleModulus_Encode
//   4. C3 wrap: [C3][outerLen][encryptedBlob]
//   5. Send vía socket con WSAEWOULDBLOCK queue
//
// Layout plaintext esperado por gNetwork.Send: [C1][size][head][payload].
// Para 0x24 PMSG_ITEM_MOVE_RECV (ItemManager.h:39):
//   struct {
//     PBMSG_HEAD header;        // C1 : len=11 : 0x24
//     BYTE SourceFlag;          // 0=inventory, 1=trade, 2=warehouse, 3=chaos-box
//     BYTE SourceSlot;
//     BYTE ItemInfo[4];         // [type, optByte, dur, typeHi|exc]
//     BYTE TargetFlag;
//     BYTE TargetSlot;
//   };
// Total: 3 + 1 + 1 + 4 + 1 + 1 = 11 bytes.
//
// `pItem` (= pPickedItem = DAT_07e91350) es un buffer ITEM 0x44 bytes; los
// primeros 4 son el wire format del server (per Recv_Inventory):
//   pItem[0] = type byte 0
//   pItem[1] = optByte (level<<3 | skill | luck | options)
//   pItem[2] = durability
//   pItem[3] = type hi nibble | excellent
//
// `iSrcType` mapeo: 0=inventory, 1=trade, 2=warehouse, 3=equipment-direct-equip.
// `iDstIndex` codifica destino: para inventario es slot index puro (0..63);
// los callers ya pasan el slot encoded.
// C++-linkage forward decl matching Net.h:89.

static BYTE InventoryPoolToMoveFlag(const BYTE* poolBase) {
    if (poolBase == &OffsetTradeItems[0] || poolBase == &Inventory[0]) {
        return 1;
    }
    if (poolBase == &OffsetWarehouseItems[0]) {
        return 2;
    }
    if (poolBase == &OffsetMixItems[0]) {
        return 3;
    }
    return 0;
}

static void InventoryMove_SetPendingPools(const BYTE* sourcePoolBase,
                                          const BYTE* targetPoolBase) {
    g_ItemMoveSourcePool = (DWORD)(uintptr_t)sourcePoolBase;
    g_ItemMoveTargetPool = (DWORD)(uintptr_t)targetPoolBase;
}

int g_ItemMoveTargetDurBefore = -1;

void __cdecl SendRequestEquipmentItem(int srcFlag, int iSrcIndex, ITEM* pItem,
                                           int dstFlag, int iDstIndex) {
    if (!pItem) return;

    // [C1][size][0x24][srcF][srcS][item: 7 bytes][tgtF][tgtS].  0.97.20: el
    // item viaja en 7 bytes (PMSG_ITEM_MOVE_RECV del server con
    // MAX_ITEM_INFO = 7); antes eran 4 y el paquete medía 11.
    // gNetwork.Send pisa pkt[1] con el serial, aplica el chain-XOR y CSM y
    // emite el frame C3.
    BYTE pkt[16];
    memset(pkt, 0, sizeof(pkt));
    pkt[0]  = 0xC1;
    pkt[1]  = (BYTE)(7 + ITEM_INFO_SIZE);
    pkt[2]  = 0x24;
    pkt[3]  = (BYTE)srcFlag;
    pkt[4]  = (BYTE)iSrcIndex;
    ItemWire_FromItem((const BYTE*)pItem, pkt + 5);
    pkt[5 + ITEM_INFO_SIZE] = (BYTE)dstFlag;
    pkt[6 + ITEM_INFO_SIZE] = (BYTE)iDstIndex;

    // Para el merge de stacks: el server responde 0xFF y sólo manda la
    // durabilidad nueva del destino (0x2A), así que el sobrante del origen se
    // deduce comparando con la que tenía antes (Recv_Item.cpp NetRecv_24).
    g_ItemMoveTargetDurBefore = -1;
    if (srcFlag == 0 && dstFlag == 0 && iDstIndex >= 12 && iDstIndex < 76) {
        const ITEM* target = (const ITEM*)OffsetInventoryItems + (iDstIndex - 12);
        if (target->Type == pItem->Type) g_ItemMoveTargetDurBefore = target->Durability;
    }

    gNetwork.Send(pkt, 7 + ITEM_INFO_SIZE);

    // IDA: sub_4CDC70 y FUN_004D6470 — toda modificación de la oferta local
    // (inventario <-> Trade o Trade <-> Trade) bloquea la confirmación durante
    // 150 ticks cuando todavía no estaba confirmada. FUN_004EB7F0 descuenta
    // m_nMyTradeWait y RenderTrade tiñe la lámpara de rojo mientras dure.
    if ((srcFlag == 1 || dstFlag == 1) && m_bMyConfirm == 0) {
        TradeMyWait = 150;
    }
}
