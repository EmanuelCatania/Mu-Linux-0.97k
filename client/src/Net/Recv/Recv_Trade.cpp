// Recv_Trade.cpp — paquetes del server: trade entre jugadores.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

void ReceiveTradeExit97k(const BYTE* Msg, int Size)
{
    if (DAT_07eaa165) {                              // EquipmentItem
        if (!s_bPacketAfterEquipmentItem) {
            s_bPacketAfterEquipmentItem = true;
            memcpy(s_byPacketAfterEquipmentItem, Msg, 4);
        }
        return;
    }
    NetLog("NET:  -> 0x3D TradeExit state=%d size=%d", Msg[3], Size);
    BYTE state = Msg[3];

    if (state == 0) {
        UIChatLogWindow_AddText(nullptr, GlobalText[494], 2);   // IDA: 494
        DAT_07eaa0e8 = 0;

        // IDA L29-35: cada item del trade del otro jugador (Type y Key en +0x38)
        // pasa por sub_4CC530, que recuerda los valiosos.  El port llamaba UI_Main
        // y leia la Key en +4 (Level).
        for (int slot = 0; slot < 32; ++slot) {
            BYTE* item = Inventory + slot * 0x44;
            if (*(short*)item != (short)0xFFFF && *(DWORD*)(item + 0x38) != 0)
                Item_TradeHistoryAdd(slot, Inventory);
        }
    } else if (state == 2) {
        UIChatLogWindow_AddText(nullptr, GlobalText[495], 2);
    } else if (state == 3) {
        UIChatLogWindow_AddText(nullptr, GlobalText[496], 2);
        SetErrorMessage(0);
    }
    // ReceiveTradeExit (IDA 0x4337F0) solo maneja los estados 0, 2 y 3.  Un
    // `state == 4` con GlobalText[2108] seria de una version posterior (indice
    // FUERA del Text.bmd del 0.97k, que tiene 1000 filas).

    DAT_07eaa11b = 0;
    DAT_05826d30 = 0;
    DAT_07e91388 = 0;
    DAT_07eaa165 = 0;
    DAT_07eaa0f0 = 0;
    DAT_07eaa0f4 = 0; // IDA: m_nMyTradeGold
    m_bYourConfirm = 0;
    m_bMyConfirm = 0;
    TradeYourWait = 0;
    TradeMyWait = 0;
    TradeRemoteGuildKey = 0;
    TradeRemoteLevel = 0;
    DAT_07ea9834[0] = '\0';
    EnableUse = 0;
    g_ItemMoveSourcePool = 0;
    g_ItemMoveTargetPool = 0;
    InitGuildWar();
    DAT_07eaa117 = 0;   // InventoryOpened (IDA ReceiveTradeExit: cierra el inventario)
    CloseInventoryRelatedWindows();

    if (DAT_083a7c24 == 116) {
        SetErrorMessage(0);
        ClearInput(0);
        _InputTextMaxArr[0] = 42;
        InputNumber = 2;
        InputEnable = 0;
    }
}

// 0x36
void NetRecv_36(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ProtocolCore 0x43B32D — C1:36 copia el nombre de diez
    // bytes en DAT_07EA9834 y abre ErrorMessage 121. MuEmu conserva
    // ese nombre, aunque use C3 en la notificación al destinatario.
    if (Size < 13) return;
    memcpy(DAT_07ea9834, Msg + 3, 10);
    DAT_07ea9834[10] = '\0';
    SetErrorMessage(121);
    NetLog("NET: -> 0x36 TradeRequest name=%s", DAT_07ea9834);
}

// 0x37
void NetRecv_37(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    NetLog("NET:  -> 0x37 TradeResult sub=%d size=%d", Msg[3], Size);
    sub = Msg[3];
    if (sub == 0) {
        UIChatLogWindow_AddText(nullptr, GlobalText[492], 2);
    } else if (sub == 2) {
        UIChatLogWindow_AddText(nullptr, GlobalText[493], 2);
    } else if (sub == 1) {
        // IDA: FUN_004332E0 ReceiveTradeResult. La apertura limpia
        // paneles incompatibles y reinicia ambos lados del intercambio.
        PartyOpened = 0;
        CharacterOpened = 0;
        ShopOpened = 0;
        WarehouseOpened = 0;
        DAT_07eaa11b = 1;
        DAT_05826d30 = 1;
        DAT_07eaa0e8 = 0;
        DAT_07eaa0f0 = 0;
        DAT_07eaa0f4 = 0; // IDA: m_nMyTradeGold
        m_bYourConfirm = 0;
        m_bMyConfirm = 0;
        TradeYourWait = 0;
        TradeMyWait = 0;
        DAT_00559684 = 0xFFFFFFFF;
        ItemMove_ClearPickedState();
        InventoryOpened = 1;
        if (Size >= 20) {
            memcpy(DAT_07ea9834, Msg + 4, 10);
            DAT_07ea9834[10] = '\0';
            TradeRemoteGuildKey = *(DWORD*)(Msg + 16);
            TradeRemoteLevel = *(WORD*)(Msg + 14);
        }
    }
}

// 0x38
void NetRecv_38(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // UI_Main slot-clear notification. Per IDA L1126-1128.
    // Server tells client to clear inventory slot pkt[3].
    NetLog("NET:  → 0x38 SlotClear slot=%d", Msg[3]);
    // IDA 004389A0: C1:38 limpia `Inventory`, la grilla superior
    // de sólo lectura que representa la oferta del otro comerciante.
    UI_Main((int)Msg[3], (short*)Inventory, 8u);
    PlayBuffer(29, 0, 0);
}

// 0x39
void NetRecv_39(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PMSG_TRADE_ITEM_ADD_SEND (MuEmu Trade.h): slot + 4-byte
    // ItemInfo. El guard anterior de >=16 venía de otro
    // packet family and discarded every valid MuEmu trade item.
    NetLog("NET:  → 0x39 TradeWarehouseSlot slot=%d size=%d",
           Msg[3], Size);
    if (Size >= 8) {
        // IDA 004389A0: C1:39 inserta en la misma grilla remota.
        InsertInventoryItem(Inventory, 8, 4,
                     (int)Msg[3], (BYTE*)Msg + 4, 1);
        PlayBuffer(29, 0, 0);
    }
}

// 0x3A
void NetRecv_3A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ProtocolCore 0x43B3D0 — ACK del importe propio. El
    // servidor sólo devuelve un resultado; el valor está en el
    // temporal m_nTempMyTradeGold que UI_InGameMenu guardó antes del envío.
    DAT_07eaa0f4 = (Msg[3] != 0) ? m_nTempMyTradeGold : 0;
}

// 0x3B
void NetRecv_3B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ProtocolCore 0x43B40C lee *((DWORD*)ReceiveBuffer + 1),
    // es decir dinero en +4. El byte +3 es el relleno de alineación
    // de PMSG_TRADE_MONEY_SEND (PBMSG_HEAD + DWORD).
    if (Size >= 8) {
        DAT_07eaa0f0 = *(DWORD*)(Msg + 4);
    }
}

// 0x3C
void NetRecv_3C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PMSG_TRADE_OK_BUTTON_SEND. Estas son las dos lámparas de
    // confirmación, no estado de la segunda contraseña.
    sub = Msg[3];
    if (sub == 0) {
        m_bYourConfirm = 0;
    } else if (sub == 1) {
        m_bYourConfirm = 1;
        PlayBuffer(0x19, 0, 0);
    } else {
        if (sub == 2) {
            m_bMyConfirm = 0;
        }
        PlayBuffer(0x19, 0, 0);
    }
}

// 0x3D
void NetRecv_3D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    ReceiveTradeExit97k(Msg, Size);
}
