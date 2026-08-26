// Net_Events.cpp
// Handlers de los opcodes 0x90-0x99 — eventos (Devil Square, Blood Castle,
// Golden Archer / Event Chip) y migracion de server.
//
// Por que existe este archivo
// ---------------------------
// Hasta 2026-08-26 estos opcodes estaban despachados en Net_Process.cpp a
// handlers de GUILD inventados por el port (Guild_CreateOk,
// Guild_AddMemberResult, Guild_MemberList, ...). Los nombres que IDA le da a
// las funciones del binario original desmienten esa etiqueta una por una:
//
//   opcode | funcion real en el binario           | etiqueta que tenia el port
//   -------|--------------------------------------|---------------------------
//   0x90   | ReceiveMoveToDevilSquareResult 436820 | "Guild create result"
//   0x91   | ReceiveEventZoneOpenTime       436CB0 | "Guild add member result"
//   0x92   | StartMatchCountDown            47EC00 | (no tenia case)
//   0x93   | ReceiveDevilSquareRank         436A80 | "Guild member list"
//   0x94   | ReceiveEventChipInfomation     4372C0 | "Guild char-select result"
//   0x95   | ReceiveEventChip               437380 | "Guild update pos"
//   0x96   | ReceiveMutoNumber              4373A0 | "Guild set target pos"
//   0x99   | ReceiveServerImmigration       4373D0 | "Guild join toggle"
//
// Y MuEmu coincide con IDA en los ocho: manda 0x90 desde DevilSquare.cpp, 0x91
// desde Protocol.cpp (PMSG_EVENT_REMAIN_TIME_SEND), 0x92/0x93 desde
// DevilSquare.cpp y BloodCastle.cpp, y 0x94-0x97 desde GoldenArcher.cpp. El
// guild real vive en 0x50-0x62 (Guild.cpp del server), que el cliente ya
// atiende aparte y correctamente.
//
// O sea no habia que elegir entre fidelidad a IDA y fidelidad a MuEmu: las dos
// fuentes dicen lo mismo y el port estaba mal. Los Guild_* de Party.cpp quedan
// sin callers (ver la nota alli).
//
// Hasta que se abra algun evento del lado del server esto no cambia nada
// visible, salvo el 0x91, que es la respuesta al click derecho sobre las
// entradas de evento.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

// ─────────────────────────────────────────────────────────────────────────────
// 0x90 — ReceiveMoveToDevilSquareResult (0x00436820)
// Resultado del intento de entrar a Devil Square desde el NPC.
// ─────────────────────────────────────────────────────────────────────────────
void Recv_MoveToDevilSquareResult(BYTE* Msg, int Size)
{
    InventoryOpened = 0;
    CloseInventoryRelatedWindows();

    // El original manda [C1][03][31] antes de mostrar el cartel.
    const BYTE ack[3] = { 0xC1, 0x03, 0x31 };
    Net_SendC1Packet(ack, 3);

    if (Size < 4) return;

    // GlobalText 677/678/686/687/679 segun el resultado (IDA L60-77).
    static const int kText[5] = { 677, 678, 686, 687, 679 };
    const int idx = (int)Msg[3] - 1;
    if (idx >= 0 && idx < 5)
        CreateOkMessageBox(GlobalText[kText[idx]]);
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x91 — ReceiveEventZoneOpenTime (0x00436CB0)
// Respuesta al click derecho sobre una entrada de evento: cuanto falta para
// que abra. Server: PMSG_EVENT_REMAIN_TIME_SEND (Protocol.h:296)
//     [C1][05][91][EventType][RemainTime]
// EventType 1 = Devil Square, 2 = Blood Castle. RemainTime 0 = ya esta abierto.
// ─────────────────────────────────────────────────────────────────────────────
void Recv_EventZoneOpenTime(BYTE* Msg, int Size)
{
    if (Size < 5) return;

    const BYTE eventType = Msg[3];
    const BYTE remain    = Msg[4];
    char strMsg[256];

    if (eventType == 1) {           // Devil Square
        if (remain) { wsprintfA(strMsg, GlobalText[644], remain); CreateOkMessageBox(strMsg); }
        else        { CreateOkMessageBox(GlobalText[643]); }
    }
    else if (eventType == 2) {      // Blood Castle
        if (remain) { wsprintfA(strMsg, GlobalText[851], remain); CreateOkMessageBox(strMsg); }
        else        { CreateOkMessageBox(GlobalText[850]); }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x92 — StartMatchCountDown (0x0047EC00)  — SIN PORTAR
//
// El cuerpo del original son dos lineas:
//     m_iMatchCountDownType  = iType;          // = ReceiveBuffer[3] + 1
//     m_dwMatchCountDownStart = GetTickCount();
//
// No se porta todavia porque esas dos variables viven como estaticas dentro de
// `Render/HUD_Pass4.cpp` (lineas 41-42) y exponerlas es un cambio de ese
// modulo, no de este. `FUN_0047ec00` tampoco sirve: no tiene definicion en
// ningun .cpp y en functions.h esta declarada `(int,int,int)` con la etiqueta
// "CharSelect_SetSlotCount", que es otra misidentificacion de la misma familia.
//
// Efecto de que falte: no se dibuja la cuenta regresiva al empezar un evento.
// El resto del evento funciona.
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// 0x93 — ReceiveDevilSquareRank (0x00436A80)
// Tabla de posiciones de Devil Square. Msg[4] == 0xFF trae UN registro;
// si no, la lista entera.
//
// Nota: FUN_0051da80 / FUN_0051d9e0 figuran en functions.h como
// "GuildMemberList_*". Es otra etiqueta heredada del mismo error: su unico
// caller en el binario es esta funcion, o sea son el render del ranking.
// ─────────────────────────────────────────────────────────────────────────────
void Recv_DevilSquareRank(BYTE* Msg, int Size)
{
    if (Size < 5) return;

    if (Msg[4] == 0xFF) FUN_0051da80((int)Msg[3], Msg + 5);
    else                FUN_0051d9e0((int)Msg[4], (int)Msg[3], Msg + 5);
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x94 — ReceiveEventChipInfomation (0x004372C0)
// Estado del dialogo del Golden Archer. Msg[3] + 1 elige el modo del panel
// (1 = normal, 3 = scratch ticket / regalo).
// ─────────────────────────────────────────────────────────────────────────────
void Recv_EventChipInfomation(BYTE* Msg, int Size)
{
    if (Size < 6) return;

    CloseInventoryRelatedWindows();

    const int mode = (int)Msg[3] + 1;
    DAT_07e11e1c = *(short*)(Msg + 4);      // g_shEventChipCount
    _g_bEventChipDialogEnable = mode;

    // mode == 3 (scratch ticket / regalo) — SIN PORTAR. El original prepara el
    // campo de texto del nombre del regalo:
    //     memset(g_strGiftName, 0, sizeof(g_strGiftName));
    //     ClearInput(0); InputTextMax[0] = 12; InputNumber = 1;
    //     InputEnable = 0; GoldInputEnable = 0; InputGold = 0;
    //     StorageGoldFlag = 0; g_bScratchTicket = 1;
    // De esos, `g_strGiftName`, `g_bScratchTicket`, `InputNumber` y `ClearInput`
    // no existen todavia en el arbol, y los de zen aparecen con nombre DAT_
    // (GoldInputEnable = DAT_07e11d72). Es una tanda propia del Golden Archer.
    // El panel igual abre; lo que falta es el modo de escribir el nombre.

    InventoryOpened = 0;
    CharacterOpened = 0;

    // Los tres shorts de g_shMutoNumber solo se pisan si ninguno viene en -1.
    if (mode == 1 && Size >= 12) {
        const short a = *(short*)(Msg + 6);
        const short b = *(short*)(Msg + 8);
        const short c = *(short*)(Msg + 10);
        if (a != -1 && b != -1 && c != -1) {
            // El original hace un store de 4 bytes sobre g_shMutoNumber[0..1];
            // aca se escribe short por short porque los tres son globals
            // separados y su contiguidad no esta garantizada.
            DAT_07e11e20 = a;
            DAT_07e11e22 = b;
            DAT_07e11e24 = c;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x95 — ReceiveEventChip (0x00437380)
// Actualiza el contador de chips. -1 = sin dato, no se toca.
// ─────────────────────────────────────────────────────────────────────────────
void Recv_EventChip(BYTE* Msg, int Size)
{
    if (Size < 6) return;
    const short count = *(short*)(Msg + 4);
    if (count != -1) DAT_07e11e1c = count;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x96 — ReceiveMutoNumber (0x004373A0)
// Los tres numeros del scratch ticket. Se aplican solo si ninguno viene en -1.
// ─────────────────────────────────────────────────────────────────────────────
void Recv_MutoNumber(BYTE* Msg, int Size)
{
    if (Size < 10) return;
    const short a = *(short*)(Msg + 4);
    const short b = *(short*)(Msg + 6);
    const short c = *(short*)(Msg + 8);
    if (a != -1 && b != -1 && c != -1) {
        DAT_07e11e20 = a;
        DAT_07e11e22 = b;
        DAT_07e11e24 = c;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x99 — ReceiveServerImmigration (0x004373D0)
// Resultado del pedido de mudanza de server. 0 = OK (144), 1 = rechazado (145).
// ─────────────────────────────────────────────────────────────────────────────
void Recv_ServerImmigration(BYTE* Msg, int Size)
{
    if (Size < 4) return;
    if (Msg[3] == 0)      SetErrorMessage(144);
    else if (Msg[3] == 1) SetErrorMessage(145);
}
