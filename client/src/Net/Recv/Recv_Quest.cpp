// Recv_Quest.cpp — paquetes del server: quests.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

// ── 0xA0..0xA3 — sistema de quests ──────────────────────────────
// ProtocolCore (0x4389A0 L1376-1387) los manda a ReceiveQuestHistory /
// State / Result / Prize.
// 0xA0
void NetRecv_A0(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveQuestHistory @ 0x00437450
    if (Size < 4 || g_csQuest == 0) return;
    NetLog("NET:  → 0xA0 QuestHistory num=%d", Msg[3]);
    // IDA lee `*(BYTE*)(Hero + 444)` directo, pero en nuestro
    // build este paquete llega ANTES de que exista la entidad del
    // heroe (en el mismo tick se ve "0x12 SKIP - hero not yet
    // allocated"), asi que Hero es 0.  Con Class = -1
    // setQuestLists NO escribe This+4, que queda en el 0xFF que le
    // pone el ctor — y FindQuestContext lo usa como indice de clase
    // para leer `pQuest + 44 + clase`, o sea se iba 255 bytes fuera
    // de la entrada, no encontraba contexto y caia en el LABEL_5
    // que decrementa el indice de quest (0 -> 255).  Ese era el
    // qIdx=255 del panel.
    // Fallback: CharacterAttribute+11, que el char-select ya dejo
    // seteado y es de donde Recv_JoinMapServer copia hero+444.
    int heroClass = -1;
    if (DAT_07abf5d8 != 0)
        heroClass = *(BYTE*)((BYTE*)(uintptr_t)DAT_07abf5d8 + 444);
    else if (CharacterAttribute != nullptr)
        heroClass = *(BYTE*)((BYTE*)CharacterAttribute + 11);
    // El destino (CSQuest + 0x1C848) son 0x32 bytes: IDA hace
    // memset de 48 + el WORD de +0x1C878.  El server manda
    // QuestInfo[50] con count = MAX_QUEST_LIST/4 = 48 (cada byte
    // lleva 4 quests de 2 bits, de ahi el `index >> 2` de
    // setQuestList).
    int num = Msg[3];
    if (num > 0x32) num = 0x32;
    if (Size < 4 + num) num = Size - 4;
    if (num > 0)
        CSQuest__setQuestLists((int)g_csQuest, 0, (BYTE*)Msg + 4, num, heroClass);
}

// 0xA1
void NetRecv_A1(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveQuestState @ 0x00437480
    if (Size < 5 || g_csQuest == 0) return;
    NetLog("NET:  → 0xA1 QuestState index=%d result=%d", Msg[3], Msg[4]);
    CSQuest__setQuestList((int)g_csQuest, 0, Msg[3], Msg[4]);
    CSQuest__ShowQuestNpcWindow((void*)(uintptr_t)g_csQuest, 0, -1);
}

// 0xA2
void NetRecv_A2(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveQuestResult @ 0x004374B0
    if (Size < 6 || g_csQuest == 0) return;
    NetLog("NET:  → 0xA2 QuestResult index=%d err=%d state=%d",
           Msg[3], Msg[4], Msg[5]);
    if (Msg[4] == 0) {          // IDA: sólo si el byte 4 es 0
        CSQuest__setQuestList((int)g_csQuest, 0, Msg[3], Msg[5]);
        CSQuest__ShowQuestNpcWindow((void*)(uintptr_t)g_csQuest, 0, -1);
    }
}

// ReceiveQuestPrize @ 0x004374E0 — premio de quest.
// El decompile son 2579 bytes pero ~70% es ruido de hash-table
// (anti-tamper) alrededor de CharacterMachine; omitido por policy.
// Lógica real, con Msg[5] como sub-tipo:
//   0xC8  puntos de stat  (CharacterAttribute+84 += Msg[6])
//   0xC9  cambio de clase (entidad+444 y CharacterAttribute+11)
//   0xCA  clase + puntos
//   0xCB  clase, sin puntos
// 0xA3
void NetRecv_A3(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    if (Size < 7) return;
    int key = ((Msg[4] + (Msg[3] << 8)) & 0x7FFF);
    int idx = FindCharacterIndex(key);
    NetLog("NET:  → 0xA3 QuestPrize sub=0x%02X key=%d idx=%d val=%d",
           Msg[5], key, idx, Msg[6]);
    if (idx < 0 || idx >= 400) return;
    if (DAT_07abf5d0 == 0) return;

    BYTE *c    = (BYTE*)(uintptr_t)DAT_07abf5d0 + (size_t)idx * 916;
    BYTE *hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
    BYTE *attr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    // Clase codificada igual que en la lista de personajes:
    //   ((b >> 4) | (b & 0x10)) >> 1
    BYTE klass = (BYTE)((((Msg[6] >> 4) | (Msg[6] & 0x10))) >> 1);

    if (Msg[5] == 0xC8) {
        if (c == hero && attr) {
            // Con GAMESERVER_EXTRA el server manda el LevelUpPoint
            // ya resuelto en ViewPoint (+8, sizeof = 12).  Igual que
            // con el F3/06 de stats y el 0x15 de damage, ese valor
            // es el autoritativo; el `+=` es el camino del 0.97k.
            if (Size >= 12) *(WORD*)(attr + 84) = (WORD)*(const DWORD*)(Msg + 8);
            else            *(WORD*)(attr + 84) += Msg[6];
        }
        // 15 chispas + destello, sin rebuild del personaje
        for (int n = 0; n < 15; ++n)
            Joint_Create(1249, (float*)(c + 16), (float*)(c + 16),
                         (float*)(c + 28), 0, (int)(uintptr_t)c,
                         40.0f, 2, 0);
        CreateEffect(1264, (float*)(c + 16), (float*)(c + 28),
                     (float*)(c + 232), nullptr, (float*)c,
                     (float*)-1, nullptr, 0);
        PlayBuffer(71, 0, 0);
        return;
    }

    if (Msg[5] == 0xC9 || Msg[5] == 0xCA || Msg[5] == 0xCB) {
        *(BYTE*)(c + 444) = klass;
        if (c == hero && attr) {
            *(BYTE*)(attr + 11) = klass;
            // Sólo el sub-tipo 0xCA suma puntos, y usa EL MISMO
            // byte que la clase.  Verificado a nivel instrucción
            // (0x437B9x: `mov [eax+0Bh], cl` seguido de
            // `movzx cx, [edx+6]` / `add [eax+54h], cx`), no sólo
            // en el decompile.  Raro, pero es lo que hace el
            // binario; el server es autoritativo igual y reenvía
            // el LevelUpPoint real.
            if (Msg[5] == 0xCA) {
                if (Size >= 12) *(WORD*)(attr + 84) = (WORD)*(const DWORD*)(Msg + 8);
                else            *(WORD*)(attr + 84) += Msg[6];
            }
        }
        float up[3] = { *(float*)(c + 16), *(float*)(c + 20),
                        *(float*)(c + 24) + 200.0f };
        for (int n = 0; n < 15; ++n) {
            Joint_Create(1249, (float*)(c + 16), (float*)(c + 16),
                         (float*)(c + 28), 0, (int)(uintptr_t)c,
                         40.0f, 2, 0);
            Joint_Create(1249, up, up,
                         (float*)(c + 28), 10, (int)(uintptr_t)c,
                         40.0f, 2, 0);
        }
        CreateEffect(1264, (float*)(c + 16), (float*)(c + 28),
                     (float*)(c + 232), nullptr, (float*)c,
                     (float*)-1, nullptr, 0);
        Character_UpdateEquipSlotAnimations((int)(uintptr_t)c);   // rebuild de body-parts
        SetAction((int)(uintptr_t)c, 124);
        PlayBuffer(72, 0, 0);
    }
}
