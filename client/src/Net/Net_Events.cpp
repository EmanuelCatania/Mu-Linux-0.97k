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

// C1:8E / C1:8F are extensions emitted by the in-tree GameServer before its
// normal C3:30 event-NPC result.  Their destination arrays are exactly the
// ones read by the original 0.97K RenderEventWindow @ 0x004F3C50.
void Recv_DevilSquareRequiredLevels(BYTE* Msg, int Size)
{
    // The in-tree GameServer declares PBMSG_HEAD (3 bytes) immediately before
    // an int array without #pragma pack(1).  MSVC therefore inserts one byte
    // of alignment padding and sends 36 bytes, with the first int at +4.
    // A packed sender (the wire layout expected by a stock client) is 35
    // bytes and starts at +3.  Accept both forms; reading the padded packet at
    // +3 turned a level 15 with padding 0x06 into 0x00000F06 = 3846.
    const int valueBytes = 4 * 2 * (int)sizeof(int);
    const int payload = (Size >= 4 + valueBytes) ? 4 : 3;
    if (Size < payload + valueBytes) return;
    for (int level = 0; level < 4; ++level) {
        m_iDevilSquareLimitLevel[level][0] = *(const int*)(Msg + payload + (level * 2 + 0) * 4);
        m_iDevilSquareLimitLevel[level][1] = *(const int*)(Msg + payload + (level * 2 + 1) * 4);
    }
}

void Recv_BloodCastleRequiredLevels(BYTE* Msg, int Size)
{
    // Same ABI padding as C1:8E above.  The active server emits 100 bytes
    // (header 3 + padding 1 + 6*4 ints), whereas a packed implementation is
    // 99 bytes.  Keep compatibility with both on the client boundary.
    const int valueBytes = 6 * 4 * (int)sizeof(int);
    const int payload = (Size >= 4 + valueBytes) ? 4 : 3;
    if (Size < payload + valueBytes) return;
    for (int level = 0; level < 6; ++level) {
        const int* source = (const int*)(Msg + payload + level * 4 * sizeof(int));
        m_iBloodCastleLimitLevel[level][0]     = source[0];
        m_iBloodCastleLimitLevel[level][1]     = source[1];
        m_iBloodCastleLimitLevel[level + 6][0] = source[2];
        m_iBloodCastleLimitLevel[level + 6][1] = source[3];
    }
}

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

// 0x9A — ReceiveMoveToEventMatchResult @ 0x00436AC0 (Blood Castle).
void Recv_MoveToBloodCastleResult(BYTE* Msg, int Size)
{
    InventoryOpened = 0;
    CloseInventoryRelatedWindows();

    const BYTE ack[3] = { 0xC1, 0x03, 0x31 };
    Net_SendC1Packet(ack, 3);
    if (Size < 4) return;

    static const int kText[5] = { 854, 852, 686, 687, 853 };
    const int index = (int)Msg[3] - 1;
    if (index >= 0 && index < 5)
        CreateOkMessageBox(GlobalText[kText[index]]);
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
// 0x92 — StartMatchCountDown (0x0047EC00)
// Arranca la cuenta regresiva de 30 s que dibuja `sub_4BF090` abajo a la
// izquierda ("Infiltracion al Blood Castle (en %d segundos)" y equivalentes de
// Devil Square).  El tipo llega en Msg[3] y el original le suma 1.
// ─────────────────────────────────────────────────────────────────────────────
void Recv_StartMatchCountDown(BYTE* Msg, int Size)
{
    if (Size < 4) return;
    m_iMatchCountDownType   = (int)Msg[3] + 1;
    m_dwMatchCountDownStart = GetTickCount();
}

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
// 0x99 — ReceiveServerImmigration (0x004373D0)
// Resultado del pedido de mudanza de server. 0 = OK (144), 1 = rechazado (145).
// ─────────────────────────────────────────────────────────────────────────────
void Recv_ServerImmigration(BYTE* Msg, int Size)
{
    if (Size < 4) return;
    if (Msg[3] == 0)      SetErrorMessage(144);
    else if (Msg[3] == 1) SetErrorMessage(145);
}
