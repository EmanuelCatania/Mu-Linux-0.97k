// Recv_Event.cpp — paquetes del server: eventos: Devil Square, Blood Castle y Golden Archer.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

// ── 0x8E-0x99 — EVENTOS, no guild ─────────────────────────────
// IDA y MuEmu coinciden en que son eventos: ver la tabla
// completa en la cabecera de `src/Net/Net_Events.cpp`.
// El guild de verdad esta en 0x50-0x56, mas arriba en este mismo
// switch.
// 0x8E
void NetRecv_8E(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Devil Square admission levels (GameServer extension)
    extern void Recv_DevilSquareRequiredLevels(BYTE* Msg, int Size);
    Recv_DevilSquareRequiredLevels((BYTE*)Msg, Size);
}

// 0x8F
void NetRecv_8F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Blood Castle admission levels (GameServer extension)
    extern void Recv_BloodCastleRequiredLevels(BYTE* Msg, int Size);
    Recv_BloodCastleRequiredLevels((BYTE*)Msg, Size);
}

// 0x90
void NetRecv_90(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMoveToDevilSquareResult @ 0x00436820
    NetLog("NET:  -> 0x90 MoveToDevilSquareResult");
    extern void Recv_MoveToDevilSquareResult(BYTE* Msg, int Size);
    Recv_MoveToDevilSquareResult((BYTE*)Msg, Size);
}

// 0x91
void NetRecv_91(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveEventZoneOpenTime @ 0x00436CB0
    NetLog("NET:  -> 0x91 EventZoneOpenTime type=%u remain=%u",
           (unsigned)(Size > 3 ? Msg[3] : 0),
           (unsigned)(Size > 4 ? Msg[4] : 0));
    extern void Recv_EventZoneOpenTime(BYTE* Msg, int Size);
    Recv_EventZoneOpenTime((BYTE*)Msg, Size);
}

// 0x92
void NetRecv_92(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // StartMatchCountDown @ 0x0047EC00
    NetLog("NET:  -> 0x92 StartMatchCountDown type=%d", Size >= 4 ? Msg[3] + 1 : -1);
    extern void Recv_StartMatchCountDown(BYTE* Msg, int Size);
    Recv_StartMatchCountDown((BYTE*)Msg, Size);
}

// 0x93
void NetRecv_93(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveDevilSquareRank @ 0x00436A80
    NetLog("NET:  -> 0x93 DevilSquareRank");
    extern void Recv_DevilSquareRank(BYTE* Msg, int Size);
    Recv_DevilSquareRank((BYTE*)Msg, Size);
}

// 0x94
void NetRecv_94(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveEventChipInfomation @ 0x004372C0
    NetLog("NET:  -> 0x94 EventChipInfomation");
    extern void GoldenArcher_Recv94(BYTE* Msg, int Size);
    GoldenArcher_Recv94((BYTE*)Msg, Size);
}

// 0x95
void NetRecv_95(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveEventChip @ 0x00437380
    NetLog("NET:  -> 0x95 EventChip");
    extern void GoldenArcher_Recv95(BYTE* Msg, int Size);
    GoldenArcher_Recv95((BYTE*)Msg, Size);
}

// 0x96
void NetRecv_96(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMutoNumber @ 0x004373A0
    NetLog("NET:  -> 0x96 MutoNumber");
    extern void GoldenArcher_Recv96(BYTE* Msg, int Size);
    GoldenArcher_Recv96((BYTE*)Msg, Size);
}

// 0x97
void NetRecv_97(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Golden Archer del evento propio (C1:97:00..04)
    extern void GoldenArcher_Recv97(BYTE* Msg, int Size);
    GoldenArcher_Recv97((BYTE*)Msg, Size);
}

// 0x9D
void NetRecv_9D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveScratchResult @ 0x00437400 / resultado del evento propio
    extern void GoldenArcher_Recv9D(BYTE* Msg, int Size);
    GoldenArcher_Recv9D((BYTE*)Msg, Size);
}

// 0x99
void NetRecv_99(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveServerImmigration @ 0x004373D0
    NetLog("NET:  -> 0x99 ServerImmigration");
    extern void Recv_ServerImmigration(BYTE* Msg, int Size);
    Recv_ServerImmigration((BYTE*)Msg, Size);
}

// 0x9A
void NetRecv_9A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMoveToEventMatchResult @ 0x00436AC0
    extern void Recv_MoveToBloodCastleResult(BYTE* Msg, int Size);
    Recv_MoveToBloodCastleResult((BYTE*)Msg, Size);
}

// 0x9B
void NetRecv_9B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMatchGameCommand @ 0x00436E40 -- estado del evento
    // (Blood Castle / Devil Square).  Es lo que alimenta el cartel
    // lateral con el tiempo restante y el contador de monstruos.
    //
    // MuEmu: CBloodCastle::GCBloodCastleStateSend,
    //        PMSG_BLOOD_CASTLE_STATE_SEND (BloodCastle.h:65)
    //   +3     BYTE state
    //   +4,+5  WORD time
    //   +6,+7  WORD MaxMonster
    //   +8,+9  WORD CurMonster
    //   +10,11 WORD EventItemOwner
    //   +12    BYTE EventItemLevel
    // El struct no lleva padding (state en +3 y el primer WORD en
    // +4, que ya esta alineado), asi que coincide exacto con los
    // indices de palabra del decompile.
    if (Size < 13) { NetLog("NET:  -> 0x9B MatchState size=%d (corto)", Size); return; }
    {
        const BYTE state     = Msg[3];
        const WORD tRemain   = *(WORD*)(Msg + 4);
        const WORD maxMon    = *(WORD*)(Msg + 6);
        const WORD curMon    = *(WORD*)(Msg + 8);
        const short itemOwner= *(short*)(Msg + 10);
        const BYTE itemLevel = Msg[12];

        NetLog("NET:  -> 0x9B MatchState state=%u t=%u mon=%u/%u owner=%d lvl=%u",
               state, tRemain, curMon, maxMon, itemOwner, itemLevel);

        switch (state) {
        case 0:
            // Arranca el evento: todos los jugadores a la anim 128
            // y BGM del castillo en loop.
            Characters_SetActionAll(128);
            PlayBuffer(110, 0, 1);
            // fallthrough  (IDA: `goto LABEL_3`)
        case 1:
        case 4: {
            SetMatchInfo((BYTE)(state + 1), 900, tRemain, maxMon, curMon);
            // Marca quien lleva el arma del evento.  sub_45ACC0 ademas
            // LIMPIA el flag +744 en todas las entidades antes de
            // devolver el indice, o sea el portador es unico.
            if (itemOwner != -1 && itemLevel != 0xFF && itemLevel != 0) {
                const int idx = Character_FindByKey_WithClear(itemOwner & 0x7FFF);
                if (DAT_07abf5d0 && idx >= 0 && idx < 400) {
                    BYTE* c = (BYTE*)(uintptr_t)DAT_07abf5d0 + 916 * idx;
                    *(BYTE*)(c + 744) = itemLevel;
                }
            } else if (itemOwner == -1 || itemLevel == 0) {
                // DESVIACION DEL PORT.  El gate de arriba es fiel a IDA
                // (`if (owner != -1) if (lvl != 0xFF) if (lvl)`), pero con
                // ese gate NADIE limpia +744 cuando se devuelve el arma:
                // MuEmu manda `owner=-1 lvl=0` y acto seguido
                // `owner=0 lvl=255`, y los dos caen fuera del if.  El arma
                // quedaba colgada de la espalda hasta cambiar de mapa.
                // `sub_45ACC0` limpia el flag en TODAS las entidades antes
                // de buscar, asi que llamarla con una key imposible es
                // exactamente "que no lo lleve nadie".
                Character_FindByKey_WithClear(0xFFFF);
            }
            break;
        }
        case 2:
            clearMatchInfo();          // clearMatchInfo
            StopBuffer(110, 1);
            break;
        case 3:
            // Puerta destruida: dispara la animacion de derrumbe
            // sobre el objeto tipo 36 del mapa actual
            // (la consume MoveObject_Special / sub_4FA5F0).
            SetActionObject((int)gMapManager.GetCurrentMap(), 36, 20, 1);
            break;
        default:
            break;
        }
    }
}

// 0x0B
void NetRecv_0B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA ProtocolCore case 0xB (inline).  MuEmu: GCEventStateSend,
    // PMSG_EVENT_STATE_SEND [C1][05][0B][state][event].
    //   event 1 -> EnableEvent = (state != 0)
    //   event 3 -> EnableEvent = state ? 3 : 0
    // y en todos los casos DeleteBoids() (0x500A80): apaga los 40
    // slots de Boids (= g_WeatherSlotPool).  La cola DebugText que
    // IDA llena antes no tiene lectores en el binario; se omite.
    if (Size < 5) return;
    NetLog("NET:  → 0x0B EventState state=%d event=%d", Msg[3], Msg[4]);
    if (Msg[4] == 1)      DAT_083a3ff0 = (Msg[3] != 0) ? 1 : 0;   // EnableEvent
    else if (Msg[4] == 3) DAT_083a3ff0 = (Msg[3] != 0) ? 3 : 0;
    // event 2 = Tamachan.  No existe en el 0.97k; viene del 0.98j
    // (ProtocolCore 0x4437A0, bloque en 0x444299):
    //     if (state) { sub_46C220(); sub_46C190(); }   // limpiar + aparecer
    //     else         sub_46C250();                   // despedir
    // y despues DeleteBoids(), como los otros tipos.
    else if (Msg[4] == 2) {
        if (Msg[3]) { Tamachan_Clear(); Tamachan_Spawn(); }
        else        Tamachan_Dismiss();
    }
    for (int i = 0; i < 40; ++i)
        g_WeatherSlotPool[i * 0x1bc] = 0;
}
