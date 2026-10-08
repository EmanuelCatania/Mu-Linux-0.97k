// Recv_Guild.cpp — paquetes del server: guild, marcas de guild y guerra de guilds.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Protocol/GameServerProtocol.h"
#include "Net/Recv/NetRecv.h"

int  s_GuildRecordKey[kGuildMarkRecordCount] = { 0 };

BYTE* GuildMark_Record(int row)
{
    return (BYTE*)DAT_07e919bc + row * 80;
}

int GuildMark_UpsertRecord(int key, const BYTE* name8, const BYTE* mark32)
{
    int row = -1;
    for (int i = 0; i < kGuildMarkRecordCount; ++i) {
        if (GuildMark_Record(i)[0] != 0 &&
            memcmp(GuildMark_Record(i), name8, 8) == 0) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        for (int i = 0; i < kGuildMarkRecordCount; ++i) {
            if (GuildMark_Record(i)[0] == 0) { row = i; break; }
        }
    }
    if (row < 0) row = 0; // misma alternativa de tabla acotada que 00434DC0.

    BYTE* dst = GuildMark_Record(row);
    s_GuildRecordKey[row] = key;
    memset(dst, 0, 80);
    memcpy(dst, name8, 8);
    if (mark32) {
        for (int i = 0; i < 64; ++i)
            dst[9 + i] = (i & 1) ? (mark32[i / 2] & 0x0F)
                                  : (mark32[i / 2] >> 4);
    }
    return row;
}

// IDA: FUN_00434DC0/RenderTrade — búsqueda compartida por la clave de guild.
extern "C" int GuildMark_FindRecordByKey(int key)
{
    for (int i = 0; i < kGuildMarkRecordCount; ++i)
        if (GuildMark_Record(i)[0] != 0 && s_GuildRecordKey[i] == key)
            return i;
    return -1;
}

extern "C" const char* Guild_GetMarkName(int row)
{
    return (row >= 0 && row < kGuildMarkRecordCount)
        ? (const char*)GuildMark_Record(row) : "";
}

// sub_434DC0 guarda las celdas 8x8 decodificadas de la marca de guild en record+9. Dejamos
// el renderer detrás de este accesor para que su índice de marca siga siendo el mismo índice
// de registro que escriben los paquetes de viewport 5B/5C en Character+474.
extern "C" const BYTE* Guild_GetMarkPixels(int row)
{
    return (row >= 0 && row < kGuildMarkRecordCount)
        ? GuildMark_Record(row) + 9 : nullptr;
}

// ── Guerra de guild (IDA: FUN_00435390 / E0 / 4F0 / AA0) ────────────────────
// Estos controladores cubren deliberadamente sólo el estado de paquetes y el ciclo
// de relaciones demostrados. Las acciones de celebración y el resto de la UI
// de guerra pertenecen a una unidad funcional posterior.
void GuildWar_CopyOpponentName(const BYTE* packet)
{
    memcpy(GuildWarName, packet + 3, 8);
    GuildWarName[8] = '\0';
}

int GuildMark_FindRecordByName(const char* name)
{
    for (int i = 0; i < kGuildMarkRecordCount; ++i)
        if (GuildMark_Record(i)[0] != 0 && strcmp((const char*)GuildMark_Record(i), name) == 0)
            return i;
    return -1;
}

// IDA: FUN_00423CE0 — recalcula la relación local, aliada o enemiga de una entidad.
// IDA: FUN_00423CE0 (0x00423CE0)
void __cdecl GuildWar_UpdateEntityRelation(int entityAddress, int, int, int)
{
    BYTE* entity = (BYTE*)(uintptr_t)entityAddress;
    BYTE* hero = (BYTE*)(uintptr_t)Hero;
    if (!entity || !entity[0] || !hero) return;

    entity[745] = 0;
    const short heroGuild = *(short*)(hero + 474);
    if (heroGuild != -1 && *(short*)(entity + 474) == heroGuild)
        entity[745] = 1;

    if (!EnableGuildWar) return;
    if (GuildWarIndex == -1) {
        if (!GuildWarName[0]) return;
        GuildWarIndex = GuildMark_FindRecordByName(GuildWarName);
    }
    if (GuildWarIndex >= 0 && *(short*)(entity + 474) == GuildWarIndex)
        entity[745] = 2;
}

void GuildWar_RefreshEntityRelations()
{
    BYTE* entities = (BYTE*)(uintptr_t)CharactersClient;
    if (!entities) return;

    for (int i = 0; i < 400; ++i) {
        BYTE* entity = entities + i * 916;
        GuildWar_UpdateEntityRelation((int)(uintptr_t)entity, 0, 0, 0);
    }
}

// IDA: FUN_00423DB0.  The client keeps the normal guild relation (1) while
// clearing the active-war relation (2) from every visible entity.  This is
// intentionally separate from the legacy function with the same old label in
// Crypto.cpp, which belongs to the already-validated Trade cleanup path.
extern "C" void GuildWar_ResetClientState()
{
    EnableGuildWar = 0;
    GuildWarIndex = -1;
    GuildWarName[0] = '\0';
    GuildWar_RefreshEntityRelations();
}

void ReceiveDeclareWar97k(const BYTE* packet, int size)
{
    if (size < (int)sizeof(Proto::PMSG_GUILD_WAR_DECLARE_SEND)) return;
    Proto::PMSG_GUILD_WAR_DECLARE_SEND message;
    memcpy(&message, packet, sizeof(message));
    memcpy(GuildWarName, message.GuildName, sizeof(message.GuildName));
    GuildWarName[sizeof(message.GuildName)] = 0;
    // DESVIACION DLL: GCGuildWarDeclareRecv asigna ambos tipos, sin heredar soccer.
    EnableSoccer = (message.type == 1);
    SetErrorMessage(128);
}

void ReceiveDeclareWarResult97k(const BYTE* packet, int size)
{
    if (size < 4) return;
    static const int kText[] = { 519, 520, 521, 522, 523, 524, 525 };
    const BYTE result = packet[3];
    if (result < _countof(kText))
        UIChatLogWindow_AddText(nullptr, GlobalText[kText[result]], 2);

    // IDA limpia el estado de declaración pendiente sólo cuando la guerra no está activa.
    if (result != 1 && !EnableGuildWar) {
        GuildWar_ResetClientState();
    }
}

void ReceiveGuildBeginWar97k(const BYTE* packet, int size)
{
    if (size < 13) return;
    EnableGuildWar = 1;
    GuildWar_CopyOpponentName(packet);
    const bool soccer = packet[11] != 0;
    if (soccer) EnableSoccer = 1;   // IDA no lo apaga en la otra rama
    HeroSoccerTeam = packet[12];
    GuildWarIndex = GuildMark_FindRecordByName(GuildWarName);

    char notice[300] = {};
    _snprintf_s(notice, sizeof(notice), _TRUNCATE,
                GlobalText[soccer ? 533 : 526], GuildWarName);
    UI_AddNotice(notice, 1);
    GuildWar_RefreshEntityRelations();

    // IDA: SetActionClass(Hero, Hero, 128, 128) y además manda el 0x18 por su
    // cuenta (doble envío, igual que el binario).
    SetActionClass((int)(uintptr_t)Hero, (int)(uintptr_t)Hero, 128, 128);
    SendRequestAction(128);
}

void ReceiveGuildEndWar97k(const BYTE* packet, int size)
{
    if (size < 4) return;
    static const int kText[] = { 527, 528, 529, 530, 531, 532, 480 };
    const BYTE result = packet[3];
    if (result < _countof(kText)) {
        char notice[300] = {};
        _snprintf_s(notice, sizeof(notice), _TRUNCATE, "%s", GlobalText[kText[result]]);
        UI_AddNotice(notice, 1);
    }

    EnableSoccer = 0;
    GuildWar_ResetClientState();

    // IDA: resultados 1, 2 y 4 -> Win = 2 -> (113, 121); 0, 3 y 5 -> Win = 0 ->
    // (107, 117); el 6 (Win = 1) no anima ni manda nada. Cada rama llama a
    // SetActionClass y ademas manda el 0x18 por su cuenta (doble envio fiel).
    if (result == 6) return;
    const bool victoryAction = result == 1 || result == 2 || result == 4;
    SetActionClass((int)(uintptr_t)Hero, (int)(uintptr_t)Hero, victoryAction ? 113 : 107, victoryAction ? 121 : 117);
    SendRequestAction(victoryAction ? 121 : 117);
}

// 0x53
void NetRecv_53(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveGuildLeave @ 00434950.
    if (Size < 4) return;
    static const int textIndex[] = { 511, 512, 513, 514, 515 };
    const BYTE result = Msg[3];
    if (result < _countof(textIndex))
        UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[result]], 2);
    if (result == 1 || result == 4) {
        if (result == 4 && Hero) {
            const short row = *(short*)((BYTE*)Hero + 474);
            if (row >= 0 && row < kGuildMarkRecordCount) {
                s_GuildRecordKey[row] = -1;
                GuildMark_Record(row)[0] = 0;
            }
            *(short*)((BYTE*)Hero + 474) = -1;
        }
        g_nGuildMemberCount = -1;
        GuildOpened = 0;
    }
    NetLog("NET: GuildLeave result=%u", (unsigned)result);
}

// 0x54
void NetRecv_54(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // El server manda `[C1][03][54]` (`GCGuildMasterQuestionSend`,
    // Protocol.cpp:1795) al hablar con el Guild Master cumpliendo los
    // requisitos.
    //
    // Si NO se cumplen, el server ni siquiera manda esto: contesta un
    // chat o un notice (NpcTalk.cpp:197-220 — ya estas en un guild,
    // nivel insuficiente, resets insuficientes).
    //
    // Mismo criterio que el 0x55 de abajo: se distingue por tamaño.
    if (Size == 3) {
        GuildCreator_OpenQuestionFromServer();
        NetLog("NET: GuildCreatorQuestion (dialogo previo)");
        return;
    }
    // 0x54 — server requests "close inventory" (port 0.52 sub_407BA0).
    NetLog("NET:  → 0x54 InventoryClose");
    Recv_InventoryClose(Msg);
}

// 0x55
void NetRecv_55(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // MuEmu CGGuildMasterOpenRecv authorizes the native guild
    // creador con el frame de tres bytes C1:03:55.
    if (Size == 3) {
        GuildCreator_OpenFromServer();
        NetLog("NET: GuildCreatorOpen");
        return;
    }
    // 0x55 — server requests "open inventory" (port 0.52 sub_407BD0).
    NetLog("NET:  → 0x55 InventoryOpen");
    Recv_InventoryOpen(Msg);
}

// 0x56
void NetRecv_56(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveCreateGuildResult @ 00435280.
    if (Size < 4) return;
    static const int textIndex[] = { 516, -1, 517, 518, 940, 941, 942 };
    const BYTE result = Msg[3];
    if (result == 1) {
        GuildCreator_CloseFromResult();
        PlayBuffer(28, 0, 0);
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    } else if (result < _countof(textIndex) && textIndex[result] >= 0) {
        UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[result]], 2);
    }
    NetLog("NET: GuildCreateResult result=%u", (unsigned)result);
}

// 0x50
void NetRecv_50(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Guild invitation. ProtocolCore stores the inviter viewport
    // clave y después abre el diálogo 119; la respuesta es C1:51, no el
    // C1:41 de party. Mantener este estado aislado de los pedidos de party.
    if (Size < 5) return;
    DAT_07eaa0d8 = (DWORD)((Msg[3] << 8) | Msg[4]);
    SetErrorMessage(119);
    NetLog("NET: -> 0x50 GuildRequest inviter=%u",
           (unsigned)DAT_07eaa0d8);
}

// 0x51
void NetRecv_51(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveGuildResult @ 00434780: outcome text + dismiss.
    if (Size < 4) return;
    static const int textIndex[] = { 503, 504, 505, 506,
                                     507, 508, 509, 510 };
    const BYTE result = Msg[3];
    if (result < _countof(textIndex))
        UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[result]], 2);
    SetErrorMessage(0);
    NetLog("NET: -> 0x51 GuildResult result=%u", (unsigned)result);
}

// 0x52
void NetRecv_52(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveGuildList @ 004348B0. MuEmu Guild.h defines the
    // native long frame as:
    // [C2][sizeHi][sizeLo][52][result][count][TotalScore:DWORD]
    // [score] followed by count * { name[10], number, connected }.
    // La rama 0x65 existente es de otro protocolo y
    // no se puede usar acá porque los offsets de sus campos difieren.
    // Ojo con el PADDING de la struct del server (MuEmu Guild.h):
    //     struct PMSG_GUILD_LIST_SEND {
    //         PWMSG_HEAD header;   // C2:52   +0..3
    //         BYTE  result;        //         +4
    //         BYTE  count;         //         +5
    //                              //         +6,+7  PADDING
    //         DWORD TotalScore;    //         +8..11   (alineado a 4)
    //         BYTE  score;         //         +12
    //     };                       // sizeof = 16
    // Los miembros (`PMSG_GUILD_LIST`, 12 bytes: name[10], number,
    // connected) arrancan en +16, no en +11.
    const int kHeaderSize = 16;
    if (Size < kHeaderSize) return;
    const BYTE count = Msg[5];
    // IDA: FUN_004348B0 avanza el staging DAT_07e91790 de a 13
    // bytes. El espacio real hasta DAT_07e919bc es 0x22C: admite
    // 42 registros, aunque MuEmu hoy envíe como máximo 40.
    const int maxMembers = 42;
    const int memberCount = (count < maxMembers) ? count : maxMembers;
    if (Size < kHeaderSize + (int)count * 12) return;

    g_nGuildMemberCount = memberCount;
    GuildTotalScore = *(const int*)(Msg + 8);
    if (GuildTotalScore < 0) GuildTotalScore = 0;
    // Se alimenta el WIDGET de lista (dword_55C9FF4), que es de donde
    // `RenderGuildList` saca las filas (`byte_7E919BC` el render lo usa nada más
    // que para el nombre del guild en el título).
    // Fiel a IDA ReceiveGuildList @0x4348B0: vtable[10] para limpiar
    // y vtable[28] por cada miembro, con el registro de 13 bytes
    //   +0..9 name · +10 NUL · +11 connected · +12 party (o -1).
    GuildList_Clear();
    memset(byte_7E91790, 0, 0x22C);
    for (int i = 0; i < memberCount; ++i) {
        const BYTE* src = Msg + kHeaderSize + i * 12;
        // El original arma cada registro EN `byte_7E91790` (stride
        // 13) y desde ahi se lo pasa a vtable[28]; el estado 126 de
        // UI_InGameMenu lo re-lee para el paquete de expulsar, asi
        // que hay que dejarlo escrito, no solo en un local.
        char* rec = &byte_7E91790[i * 13];
        memcpy(rec, src, 10);
        rec[10] = 0;
        // IDA: FUN_004348B0 lee `number` en +10 y lo entrega al
        // widget como estado visual; el byte +11 determina el
        // número de party: negativo → byte & 0x7F, no negativo → -1.
        // MuEmu conserva ese orden en PMSG_GUILD_LIST.
        char connected = (char)src[10];
        char partyByte = (char)src[11];
        char party     = (partyByte >= 0) ? (char)-1
                                           : (char)(partyByte & 0x7F);
        rec[11] = connected;
        rec[12] = party;
        GuildList_AddMember(rec, connected, party);
    }
    NetLog("NET: -> 0x52 GuildList result=%u members=%d score=%d",
           (unsigned)Msg[4], memberCount, GuildTotalScore);
}

// 0x5A
void NetRecv_5A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ProtocolCore → FUN_00434DC0 una vez por registro. El
    // contador es pkt[4]; cada registro de 42 bytes es [key:2][name:8][mark:32].
    if (Size < 5) return;
    const BYTE count = Msg[4];
    if (Size < 5 + (int)count * 42) return;
    for (int i = 0; i < count; ++i) {
        const BYTE* entry = Msg + 5 + i * 42;
        GuildMark_UpsertRecord((entry[0] << 8) | entry[1],
                               entry + 2, entry + 10);
    }
    NetLog("NET: GuildMark batch count=%u", (unsigned)count);
}

// 0x5C
void NetRecv_5C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: clave de entidad + registro independiente de nombre/marca;
    // asocia la fila devuelta a Character+474 y luego recalcula la relación.
    if (Size < 45) return;
    const WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);
    const int row = GuildMark_UpsertRecord(-1, Msg + 5, Msg + 13);
    if (s_IsRebuildingHero && entityKey == g_HeroKey) {
        // Ver Recv_JoinMapServer: no asociar al slot viejo que va
        // a ser borrado; la marca queda pendiente para el héroe nuevo.
        s_PendingHeroGuildMarkRow = (short)row;
        s_HasPendingHeroGuildMark = true;
        NetLog("NET: GuildMark hero=%u row=%d diferido por F3/03",
               (unsigned)entityKey, row);
        return;
    }
    const int entitySlot = FindCharacterIndex(entityKey);
    BYTE* base = (BYTE*)(uintptr_t)DAT_07abf5d0;
    if (base && entitySlot >= 0 && entitySlot < 400) {
        BYTE* entity = base + entitySlot * 916;
        *(short*)(entity + 474) = (short)row;
        // IDA: FUN_00423CE0 recibe la entidad después de actualizar Character+474.
        GuildWar_UpdateEntityRelation((int)(uintptr_t)entity, 0, 0, 0);
        GuildWar_RefreshEntityRelations();
    }
    NetLog("NET: GuildMark entity=%u row=%d", (unsigned)entityKey, row);
}

// 0x5D
void NetRecv_5D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: limpia la fila de guild de esta entidad y cierra/reinicia
    // el estado de la lista de miembros de Guild. No es un paquete Trade.
    if (Size < 5) return;
    const WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);
    const int entitySlot = FindCharacterIndex(entityKey);
    BYTE* base = (BYTE*)(uintptr_t)DAT_07abf5d0;
    if (base && entitySlot >= 0 && entitySlot < 400)
        *(short*)(base + entitySlot * 916 + 474) = -1;
    GuildOpened = 0;
    g_nGuildMemberCount = -1;
    NetLog("NET: GuildMark clear entity=%u", (unsigned)entityKey);
}

// 0x60
void NetRecv_60(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: FUN_004353E0 ReceiveDeclareWarResult.
    NetLog("NET: GuildWar 0x60 DeclareResult");
    ReceiveDeclareWarResult97k(Msg, Size);
}

// 0x61
void NetRecv_61(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: FUN_00435390 ReceiveDeclareWar.
    NetLog("NET: GuildWar 0x61 Declare");
    ReceiveDeclareWar97k(Msg, Size);
}

// 0x62
void NetRecv_62(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: FUN_004354F0 ReceiveGuildBeginWar.
    NetLog("NET: GuildWar 0x62 Begin");
    ReceiveGuildBeginWar97k(Msg, Size);
}

// 0x63
void NetRecv_63(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: FUN_00435AA0 ReceiveGuildEndWar.
    NetLog("NET: GuildWar 0x63 End");
    ReceiveGuildEndWar97k(Msg, Size);
}

// 0x64
void NetRecv_64(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ProtocolCore activa directamente el HUD de guerra y actualiza
    // sus dos puntajes de un byte.
    if (Size < 5) return;
    EnableGuildWar = 1;
    GuildWarScore[0] = Msg[3];
    GuildWarScore[1] = Msg[4];
    NetLog("NET: GuildWar 0x64 score=%d-%d",
           GuildWarScore[0], GuildWarScore[1]);
}

// 0x5B
void NetRecv_5B(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // GuildMark_AssociateEntities — el server reporta la clave de guild de cada
    // entidad del viewport. IDA: sub_435110 @ 0x00435110.
    //   [C1][size][0x5B][count][stride 4: id_hi id_lo guild_hi guild_lo]
    //
    // Por cada entidad:
    //   guildKey se busca en la tabla de GuildMark_UpsertRecord.
    //   entity+474 recibe la fila encontrada (o -1 si no existe).
    //   entity+745 queda en 0, 1 (misma guild) o 2 (guerra activa).
    if (Size < 5) return;
    BYTE count = Msg[4];
    NetLog("NET:  → 0x5B EntityGuildList count=%d", count);
    if (Size < 5 + count * 4) return;
    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int i = 0; i < count; ++i) {
        const BYTE* e = Msg + 5 + i * 4;
        WORD entityId = (WORD)((e[0] << 8) | e[1]) & 0x7FFF;
        const int guildKey = (e[2] << 8) | e[3];
        const int entitySlot = FindCharacterIndex(entityId);
        BYTE* slot = (basePtr && entitySlot >= 0 && entitySlot < 400)
            ? basePtr + entitySlot * 916 : nullptr;
        if (slot) {
            const int guildRow = GuildMark_FindRecordByKey(guildKey);
            // IDA sub_435110: si la clave no esta en la tabla de marcas,
            // +474 queda como estaba (el bucle sale sin escribir).
            if (guildRow >= 0)
                *(short*)(slot + 474) = (short)guildRow;
            if (!slot[0]) continue;          // entidad inactiva
            slot[745] = 0;
            // Mismo guild que el heroe.  IDA no excluye al heroe: el
            // propio personaje tambien queda marcado con 1.
            if (DAT_07abf5d8) {
                const short heroGuild = *(short*)((BYTE*)DAT_07abf5d8 + 474);
                if (heroGuild != -1 && *(short*)(slot + 474) == heroGuild)
                    slot[745] = 1;
            }
            if (!EnableGuildWar) continue;
            // Guerra: si la fila enemiga todavia no se resolvio, se busca
            // por nombre (el paquete de la guerra pudo llegar antes).
            if (GuildWarIndex == -1) {
                if (!GuildWarName[0]) continue;
                GuildWarIndex = GuildMark_FindRecordByName(GuildWarName);
            }
            if (GuildWarIndex >= 0 && *(short*)(slot + 474) == GuildWarIndex)
                slot[745] = 2;  // guild enemiga en la guerra activa
        }
    }
}

// 0x65
void NetRecv_65(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ProtocolCore no deriva 0x65 a ReceiveGuildList; el
    // listado nativo es exclusivamente C2:52 → FUN_004348B0.
    // MuEmu tampoco emite 0x65 para Guild. Se conserva el caso
    // aislado para no reinterpretar un paquete ajeno, pero no puede
    // modificar ni el staging de miembros ni la tabla de marks.
    NetLog("NET:  → 0x65 sin asociación a Guild, size=%d", Size);
}
