// Item_Move.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);

// SendRequestEquipmentItem @ 0x0043C250 — Equipment move / item drag
//
// 2026-05-08 BUG-FIX MAYÚSCULO (round 2): la versión anterior mandaba C1
// plano. El servidor exige Encrypt=1 para opcode 0x24 (HackPacketCheck.txt
// línea 38: "36 * 1 0 0 0"), así que cualquier packet 0x24 que llegue como
// C1 es kickeado por `CHackPacketCheck::CheckPacketHack` → CloseClient →
// "Has sido desconectado del servidor".
//
// La solución correcta es usar `Net_SendSmallPacket` (Game_SceneUpdate.cpp:112),
// que es el mismo helper que login (F1/01) y combat usan. Hace:
//   1. Chain XOR con s_LoginKey (i=3..len)
//   2. Stomp pkt[1] = DAT_05826ceb++ (serial counter — server valida que sea
//      monotónico vía CSerialCheck::CheckSerial)
//   3. CSimpleModulus encrypt vía FUN_0053cc30
//   4. C3 wrap: [C3][outerLen][encryptedBlob]
//   5. Send vía socket con WSAEWOULDBLOCK queue
//
// Layout plaintext esperado por Net_SendSmallPacket: [C1][size][head][payload].
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
// los callers en stubs_game.cpp:1296/1320 ya pasan el slot encoded.
// C++-linkage forward decl matching Net.h:89.
void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

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

void __cdecl SendRequestEquipmentItem_stub(int srcFlag, int iSrcIndex, ITEM* pItem,
                                           int dstFlag, int iDstIndex) {
    if (!pItem) return;

    // Rebuild the 4-byte wire item info from the ITEM struct.
    BYTE itemBytes[4];
    itemBytes[0] = (BYTE)(pItem->Type & 0xFF);
    itemBytes[1] = pItem->Option1;
    itemBytes[2] = pItem->Durability;
    itemBytes[3] = pItem->Unknown;

    // Build plaintext packet: [C1][len=11][0x24][srcF][srcS][i0..i3][tgtF][tgtS].
    // Net_SendSmallPacket will overwrite pkt[1] with the serial byte, do the
    // chain-XOR + CSM encrypt, and emit the final C3 frame.
    BYTE pkt[16];
    memset(pkt, 0, sizeof(pkt));
    pkt[0]  = 0xC1;
    pkt[1]  = 11;          // size (will be stomped with serial)
    pkt[2]  = 0x24;        // head
    pkt[3]  = (BYTE)srcFlag;
    pkt[4]  = (BYTE)iSrcIndex;
    pkt[5]  = itemBytes[0];   // type lo
    pkt[6]  = itemBytes[1];   // optByte (level<<3 | luck | options)
    pkt[7]  = itemBytes[2];   // durability
    pkt[8]  = itemBytes[3];   // type hi | excellent
    pkt[9]  = (BYTE)dstFlag;
    pkt[10] = (BYTE)iDstIndex;

    // Diagnostic — log every move attempt so we can see what the user is
    // doing if disconnect persists.
    {
        char dbg[160];
        wsprintfA(dbg,
            "ITEM_MOVE C3 send: srcF=%d srcIdx=%d dstF=%d dstIdx=%d "
            "item=[%02X %02X %02X %02X]",
            srcFlag, iSrcIndex, dstFlag, iDstIndex,
            itemBytes[0], itemBytes[1], itemBytes[2], itemBytes[3]);
        DbgLogPublic(dbg);
    }

    Net_SendSmallPacket(pkt, 11);
}
