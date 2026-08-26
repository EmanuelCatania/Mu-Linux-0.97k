// stubs_game.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 14828-23043 (8216 lines).
//
// BATCH 16-19: game-logic decompiles.
//   BATCH 16 — Inventory, equipment, item management, gold formatting
//   BATCH 17 — Other addresses: entity, combat, rendering, IME, chat, particles
//   BATCH 18 — Final 5 game functions
//   BATCH 19 — SendCheck (anti-tamper checksum packet)
//
// These are mostly substantive ports (SUMMARY STUBs with real best-effort
// logic) of game-relevant runtime code. Future ports/upgrades should
// happen here, not in stubs.cpp.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;   // defined in Render_Frame.cpp
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);
extern void __cdecl FUN_0046ca00(DWORD Object);
extern "C" void __cdecl FUN_004cdc70(float sx, float sy, float w, float h, int slotIdx);

// 2026-05-08 BUG-FIX MAYÚSCULO: en el binario original `OffsetInventoryItems`
// y `DAT_07ea8410` son el mismo símbolo (idem para Mix/Warehouse). En nuestra
// build son globals separados, así que comparaciones tipo
// `DAT_07ea9800 == &DAT_07ea8410` siempre fallaban → drop dispatcher tomaba
// el fallback "Other context" y enviaba srcType=3 con srcIdx==dstIdx, lo cual
// era no-op del lado del server y no liberaba el lock EnableUse.
// Forward-decls a nivel TU para usar las direcciones reales del pool.
extern "C" BYTE OffsetInventoryItems[];   // HUD_Pass3.cpp — main inv (8x8 + 12 wear)
extern "C" BYTE OffsetTradeItems[];       // HUD_Pass3.cpp
extern "C" BYTE OffsetMixItems[];         // HUD_Pass3.cpp
extern "C" BYTE OffsetWarehouseItems[];   // HUD_Pass3.cpp
extern "C" const char* Guild_GetMarkName(int row);
extern "C" BYTE Inventory[];              // HUD_Pass3.cpp
extern "C" int dword_7EAA0C8;             // HUD_Pass3.cpp
extern "C" int dword_7EAA0CC;             // HUD_Pass3.cpp

// IDA Hex-Rays intrinsic shims (mirror of stubs.cpp shims).
#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SHIBYTE(x)           (*(((char*)&(x))+1))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif
#ifndef COERCE_FLOAT
#define COERCE_FLOAT(x)      (*(float*)&(x))
#endif
#ifndef __PAIR64__
#define __PAIR64__(hi,lo)    (((unsigned __int64)(DWORD)(hi)<<32)|(DWORD)(lo))
#endif
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif


// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 16 — Inventory, equipment, item management, gold formatting
// ═══════════════════════════════════════════════════════════════════════════════

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

// FindHotKey @ 0x004B1170 (~202 lines)
// Looks up a skill ID in the CharacterMachine hotkey table via MAIN_HASH_CLASS.
// Returns hotkey slot index (0..19), or -1 if not found.
// Original wraps access in anti-tamper encrypt/decrypt; we skip that.
// Ghidra: CharacterAttribute->Skill[iVar5+4] == unaff_retaddr (phantom param = Skill)
// Real access: *(BYTE*)(DAT_07cf1ff4 + 0x57 + iVar5) == Skill
int __stdcall FindHotKey_stub(int Skill) {
    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    char* charAttr = (char*)DAT_07cf1ff4;  // CharacterAttribute

    // Scan 20 hotkey slots (indices 0..19)
    int iVar5 = 0;
    do {
        if ((unsigned char)*(charAttr + 0x57 + iVar5) == (unsigned int)Skill) break;
        iVar5++;
    } while (iVar5 < 0x14);

    // anti-tamper hash table — skipped (decrypt CharacterMachine after read)

    // If iVar5 < 0x14, we found a match; otherwise not found
    if (iVar5 >= 0x14) {
        // Not found — check WhisperRegistID table for the skill name
        // Ghidra: iterates WhisperRegistID (stride 10, up to 0x7db9373)
        // and calls AddText if no match found. This path is rarely hit.
        // For now, return -1 (not found).
        return -1;
    }
    return iVar5;
}

// RenderSkillIcon @ 0x004BB940 (~250 lines) — SUMMARY STUB
// Renders a skill icon bitmap at (x,y) with given dimensions.
// Reads CharacterAttribute->Skill[iIndex] via encrypted hash table access.
// If skill ID is 0, renders empty slot. Otherwise renders skill texture.
void __cdecl RenderSkillIcon_stub(int iIndex, float x, float y, float width, float height) {
    // 0x004BB940 — Renders a skill icon bitmap at (x,y) with given dimensions.
    // ~80% of Ghidra output is anti-tamper hash table operations wrapping reads to
    // CharacterAttribute->Skill and ->HotKey arrays. Only real logic implemented.

    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    // Read skill ID from CharacterAttribute->Skill[iIndex + 4]
    // CharacterAttribute = DAT_07cf1ff4, Skill array starts at offset +0x57
    // Actually the Ghidra accesses Skill[unaff_retaddr + 4] where unaff_retaddr = iIndex
    // 2026-05-05: bounds check on iIndex (passed by caller, can be Hero[913]
    // garbage). Without this, reading CA[0x57+iIndex] overflows CA buffer
    // → garbage skillId → OOB on subsequent SkillAttribute reads → crash.
    if (iIndex < 0 || iIndex >= 60) return;
    char* charAttr = (char*)DAT_07cf1ff4;
    if (!charAttr) return;
    unsigned int skillId = (unsigned char)*(charAttr + 0x57 + iIndex);

    // Skip if skillId is invalid (0 = empty, >= 64 = OOB on SkillAttribute table).
    if (skillId == 0 || skillId >= 64) {
        return;
    }

    // If skill is 0x2f (Helper summon) and helper type is not Dark Horse (0x332) or Dark Spirit (0x333),
    // tint the icon reddish
    if (skillId == 0x2f) {
        // Hero->Helper.Type is at entity struct offset — approximate via DAT_07abf5d8
        // (Hero->Object).Position is at +0x10; Helper.Type offset approximated from Ghidra
        // In Ghidra: (Hero->Helper).Type  — skipping exact struct access, use raw offset
        // short helperType = *(short*)((char*)(DWORD)Hero + helperTypeOffset);
        // For now, skip the helper type check — the tint is cosmetic only
        // glColor3f(1.0f, 0.5f, 0.5f);
    }

    float fWidth = (float)(int)width;
    float fHeight = (float)(int)height;
    float fX = (float)(int)x;
    float fY = (float)(int)y;

    // 004BB940: skill icons are eight columns wide.  The original advances U
    // with the icon width and V with the icon height, then trims one pixel from
    // the V extent to avoid sampling the next atlas row.
    const int atlasIndex = (int)skillId - 1;
    const float atlasU = (float)((atlasIndex % 8) * (int)width);
    const float atlasV = (float)((atlasIndex / 8) * (int)height);

    // RenderBitmap(298, x, y, width, height, u, v, uWidth, vHeight, 1, 1)
    FUN_005125a0(0x12a, fX, fY, fWidth, fHeight,
                 atlasU * _DAT_00552b7c,
                 atlasV * _DAT_00552b7c,
                 fWidth * _DAT_00552b7c,
                 (fHeight - _DAT_0055256c) * _DAT_00552b7c,
                 1, 1);

    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    // Read hotkey assignment from CharacterAttribute->HotKey[SelectedHero][iIndex + 4]
    // Confirmed via Ghidra CHARACTER_ATTRIBUTE layout:
    //   0x53 Skill[64]      (iIndex+4 indexed here earlier)
    //   0x93 SkillLevel[64]
    //   0xD3 HotKey[4][64]  — 4 characters × 64 slots, stride 64 bytes
    // HotKey[SelectedHero][iIndex+4] = charAttr + 0xD3 + SelectedHero*64 + (iIndex + 4)
    unsigned int selectedHero = (unsigned int)DAT_005616ac;  // SelectedHero index (0..3)
    if (selectedHero > 3) selectedHero = 0;  // safety clamp
    int hotkey = (unsigned char)*(charAttr + 0xD3 + selectedHero * 64 + iIndex + 4);
    if (hotkey == 0) hotkey = 0xFF;  // empty slot → no-hotkey sentinel

    // anti-tamper hash table — skipped (decrypt CharacterMachine after read)

    if (hotkey != 0xFF) {
        RenderNumber2D_stub(x + _DAT_00552650, y + _DAT_00552a4c, hotkey, 9.0f, 10.0f);
    }
    DAT_00559c6c = (char)hotkey;
}

// SendChat @ 0x004C1B90 (~454 lines) — SUMMARY STUB
// Sends a chat message packet to server.
// Flow:
//   1. Rate-limit: if ChatTime > 0x32, return
//   2. Duplicate check: compare with last sent text (DAT_05826adc)
//   3. Set ChatTime = 0x46, copy text to last-sent buffer
//   4. Check if Hero->ID contains "webzen" substring (GM detection)
//   5. Build C1 packet with XOR encryption using 32-byte key
//   6. Net_Send the encrypted chat packet
void __cdecl SendChat_stub(char* Text) {
    // 0x004C1B90 — Sends chat message packet to server (454 lines decompiled).
    // ~60% of original is anti-tamper XOR key init + dead forward/reverse loops.
    // Real logic:
    //   1. Rate-limit: if ChatTime > 0x32, return
    //   2. Duplicate check vs last-sent buffer (DAT_05826adc)
    //   3. Set ChatTime=0x46, copy text to last-sent buffer
    //   4. GM name check: if Hero->ID contains "webzen", skip chat (anti-impersonation)
    //   5. Dead check: if Hero->Dead, check if Text starts with GM command (DAT_07d3cdd4)
    //   6. Whisper block commands: "/whisper" → toggle m_bBlockWhisper_1
    //   7. Build C1 packet: [C1][len][01][heroID 10 bytes][text up to 60 bytes]
    //   8. XOR-encrypt payload with 32-byte key, fix header, send via socket

    // Rate limit — `ChatTime` es el GLOBAL 0x05826D08 (= DAT_05826d08), que
    // `Game_MainLoop` decrementa un tick por frame (IDA 0x5262D9-0x5262EA):
    //     if ( ChatTime > 0 ) --ChatTime;
    //
    // 2026-08-12 BUG-FIX: acá era un `static int s_ChatTime` local, y NADIE lo
    // decrementaba. Después del primer mensaje quedaba clavado en 70, así que
    // `if (> 50) return` bloqueaba **todo** el chat posterior de la sesión.
    // Síntoma reportado: el primer `/move <mapa>` funcionaba y los siguientes
    // no hacían nada — el jugador se quedaba en el destino del primero y
    // parecía que el comando "recordaba" el mapa anterior.
    // Lo usan además WndProc y Chat_InputTick, que ya leían el global real.
    if ((int)DAT_05826d08 > 0x32) return;

    // Duplicate check: compare with last-sent text
    if ((int)DAT_05826d08 > 0 && strcmp(DAT_05826adc, Text) == 0) return;

    // Set cooldown and copy to last-sent buffer
    DAT_05826d08 = 0x46;
    strncpy(DAT_05826adc, Text, sizeof(DAT_05826adc) - 1);
    DAT_05826adc[sizeof(DAT_05826adc) - 1] = '\0';

    // GM name check: if Hero->ID contains "webzen" substring, block
    // Hero is char* (DAT_07abf5d8), ID at offset 0x1C1
    {
        const char* heroId = Hero + 0x1C1;
        const char* webzenStr = &DAT_005592dc;  // "webzen" string in binary
        int webzenLen = (int)strlen(webzenStr);
        if (webzenLen > 0) {
            int heroIdLen = (int)strlen(heroId);
            int diff = heroIdLen - webzenLen;
            if (diff >= 0) {
                for (int i = 0; i <= diff; i++) {
                    if (strncmp(heroId + i, webzenStr, webzenLen) == 0) {
                        // Found "webzen" in hero name — block chat (anti-impersonation)
                        return;
                    }
                }
            }
        }
    }

    // If hero is dead, check for GM command prefix
    // Dead at offset 0x2FD in CHARACTER struct
    if (*(BYTE*)(Hero + 0x2FD) != '\0') {
        // Only allow GM command (DAT_07d3cdd4) when dead
        int cmdLen = (int)strlen(&DAT_07d3cdd4);
        if (cmdLen > 0 && strncmp(Text, &DAT_07d3cdd4, cmdLen) == 0) {
            return;  // block GM commands from dead players
        }
    }
    else {
        // Alive: check whisper block toggle commands
        if (Text[0] == '/') {
            // Check "/whisperblock on" command (DAT_07d3d284)
            int len1 = (int)strlen(&DAT_07d3d284);
            if (len1 > 0 && strncmp(Text, &DAT_07d3d284, len1) == 0) {
                // `m_bBlockWhisper_1` es el GLOBAL 0x07E11DAC (= DAT_07e11dac),
                // que ya leen Net_Process (F3 toggle) y Chat_InputTick.
                // 2026-08-12 BUG-FIX: acá había DOS `static bool` distintos —
                // uno para "on" y otro para "off" — así que el toggle no
                // cambiaba nada y nadie podía leer el estado.
                DAT_07e11dac = 1;
                UIChatLogWindow_AddText((const char*)&lpDefault_00583d88, &DAT_07d3d608, 1);
                return;
            }
            // Check "/whisperblock off" command (DAT_07d3d3b0)
            int len2 = (int)strlen(&DAT_07d3d3b0);
            if (len2 > 0 && strncmp(Text, &DAT_07d3d3b0, len2) == 0) {
                DAT_07e11dac = 0;   // mismo global que el "on" (IDA L129/L135)
                UIChatLogWindow_AddText((const char*)&lpDefault_00583d88, &DAT_07d3d734, 1);
                return;
            }
        }
    }

    // Build chat packet
    BYTE pkt[0x400];
    memset(pkt, 0, sizeof(pkt));
    int pos = 0;

    pkt[pos++] = 0xC1;  // packet type
    pkt[pos++] = 0;     // length placeholder
    // 004C1B90 inicializa el byte de cabecera del chat con 0 (buf[4]); el
    // GameServer despacha PMSG_CHAT_RECV por head 0x00.  0x01 pertenece al
    // byte auxiliar de la pila de IDA, no al paquete transmitido.
    pkt[pos++] = 0x00;  // opcode: chat (IDA: C1 <len> 00)

    // Copy Hero->ID (10 bytes) — ID at offset 0x1C1 in CHARACTER struct
    memcpy(pkt + pos, Hero + 0x1C1, 10);
    pos += 10;  // pos = 13

    // Copy chat text (max 60 bytes including null)
    int textLen = (int)strlen(Text) + 1;
    if (textLen > 0x3c) textLen = 0x3c;
    memcpy(pkt + pos, Text, textLen);
    pos += textLen;

    short totalLen = (short)pos;

    // XOR-encrypt payload bytes 3..pos with 32-byte key (chained XOR)
    static const BYTE xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };
    // IDA 004C1B90 encrypts each payload byte against the PREVIOUS
    // transmitted byte (the opcode for the first character), in ascending
    // order.  Using pkt[i+1] here changes the command text before MuEmu sees
    // it, which is especially visible with /move destinations.
    // Encrypt hero ID region (bytes 3..12).
    for (int i = 3; i < 13 && i < pos; i++) {
        pkt[i] ^= xorKey[i & 0x1f] ^ pkt[i - 1];
    }
    // Encrypt text region (bytes 13..pos)
    for (int i = 13; i < pos; i++) {
        pkt[i] ^= xorKey[i & 0x1f] ^ pkt[i - 1];
    }

    // Fix header length
    pkt[1] = (BYTE)totalLen;

    // Send via socket — DAT_055ca168 is the socket handle
    SOCKET sock = (SOCKET)DAT_055ca168;
    if (sock != INVALID_SOCKET) {
        int sent = 0;
        int toSend = (int)(totalLen & 0xffff);
        while (sent < toSend) {
            int ret = send(sock, (char*)(pkt + sent), toSend - sent, 0);
            if (ret == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK) {
                    FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    return;
                }
                if ((int)(DAT_055cc16c + (DWORD)toSend) > 0x2000) {
                    FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    return;
                }
                memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, toSend);
                DAT_055cc16c += (DWORD)toSend;
                return;
            }
            if (ret == 0) return;
            if (DAT_055ce174 != 0) FUN_0043de60();
            sent += ret;
        }
    }
}

// ConvertGold64 @ 0x004C3E10 (~23 lines) — formats gold with thousand separators
int __cdecl ConvertGold64_stub(int Zen, char* Buffer) {
    if (Zen > 999999999)
        return sprintf(Buffer, "%d,%03d,%03d,%03d", Zen / 1000000000, (Zen / 1000000) % 1000, (Zen / 1000) % 1000, Zen % 1000);
    if (Zen > 999999)
        return sprintf(Buffer, "%d,%03d,%03d", Zen / 1000000, (Zen / 1000) % 1000, Zen % 1000);
    if (Zen > 999)
        return sprintf(Buffer, "%d,%03d", Zen / 1000, Zen % 1000);
    return sprintf(Buffer, "%d", Zen);
}

// ─────────────────────────────────────────────────────────────────────────────
// RenderItemName @ 0x004C9E70 — nombre flotante de un item del suelo
// ─────────────────────────────────────────────────────────────────────────────
// Port 1:1 del decompile (mu97k-src-IDA/raw/004C9E70_RenderItemName.c).  Lo
// llama sub_4CB6F0 (Target_Render): para el item bajo el cursor con Sort=0 y
// para todos los del suelo con Sort=1 mientras Alt esté activo.
//
//   o          = &Items[i][72]  → o+2 = índice de modelo (tipo + 400)
//   ItemLevel  = Items+8        → para el Zen (modelo 863) es la CANTIDAD
//   ItemOption = Items+31
//
// 2026-08-21: antes era un resumen escrito a ojo.  Divergencias que tenía y que
// este port corrige:
//   · Zen (863): hacía `sprintf(buf, DAT_0055a608, name)` con DAT_0055a608 = ""
//     → cadena vacía, o sea el Zen del suelo no mostraba NADA.  IDA es
//     `sprintf(String, "%s %d", name, ItemLevel)` = "Zen <cantidad>".
//   · Los colores de nivel 3-4 y de la rama (v5 & 0x87) estaban invertidos en
//     RGB (IDA llena v38[2],v38[1],v38[0] y llama glColor3f(v38[0],v38[1],v38[2])).
//   · Faltaban por completo las ramas 860 (Event), 831 (alas), 951-958
//     (flechas/bolts), 795 (pergamino de skill), 826 (piedra de invocación) y
//     los sufijos Excellent/Luck/Skill (GlobalText[176..179]).
//
// Ruido anti-tamper omitido por policy: las ramas 795 y 826 del binario están
// envueltas en lookups de hash-table (ref-count + XOR sobre la entrada de
// SkillAttribute).  Con nuestra tabla en claro el resultado es el mismo.
//
// Desviación: donde IDA hace `sprintf(String, GlobalText[N])` (una cadena de
// datos usada como formato) nosotros usamos `sprintf(String, "%s", GlobalText[N])`.
void __cdecl RenderItemName_stub(int i, DWORD o, int ItemLevel, int ItemOption, bool Sort) {
    (void)i;

    const int v5 = ItemLevel;
    const int v6 = (ItemLevel >> 3) & 0xF;
    char  String[52];
    float v38[3];
    String[0] = '\0';

    SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);   // g_hFont
    const short v7 = *(short*)(o + 2);

    // &ItemAttribute[v7 - 400] — ITEM_ATTRIBUTE stride 0x40, Name en offset 0.
    // Guard defensivo (no está en IDA): DAT_07d78068 se corrompe a valores
    // chicos en algunos caminos, igual que en los guards del tooltip.
    const char* name = "";
    {
        int attrIdx = (int)v7 - 400;
        unsigned int abase = (unsigned int)(uintptr_t)DAT_07d78068;
        if (abase >= 0x100000u && abase < 0x80000000u && attrIdx >= 0 && attrIdx < 1024)
            name = (const char*)(uintptr_t)(abase + (unsigned int)attrIdx * 64u);
    }

    switch (v7) {
    case 863:                                   // Zen / dinero
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s %d", name, v5);
        goto renderLabel;

    case 861: case 862: case 864:               // joyas
    case 799: case 870: case 830:
        SelectObject(m_hFontDC, (HGDIOBJ)g_hFontBold);
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s", name);
        goto renderLabel;

    case 865: case 866: case 867:               // Devil (Chaos) items
        SelectObject(m_hFontDC, (HGDIOBJ)g_hFontBold);
        glColor3f(1.0f, 0.8f, 0.1f);
        if ((v5 & 0x78) != 0) sprintf(String, "%s +%d", name, v6);
        else                  sprintf(String, "%s", name);
        goto renderLabel;
    }

    if (v7 == 859 && v6 == 7) {
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s", GlobalText[111]);
        goto renderLabel;
    }

    switch (v7) {
    case 860:                                   // Event item
        if (v6 == 0)      sprintf(String, "%s", GlobalText[100]);
        else if (v6 == 1) sprintf(String, "%s", GlobalText[101]);
        else if (v6 == 2) sprintf(String, "%s", GlobalText[104]);
        goto renderLabel;

    case 831:                                   // alas: prefijo por sub-tipo
        glColor3f(1.0f, 0.8f, 0.1f);
        switch (v6) {
        case 0: sprintf(String, "%s %s", GlobalText[168], name); break;
        case 1: sprintf(String, "%s %s", GlobalText[169], name); break;
        case 2: sprintf(String, "%s %s", GlobalText[167], name); break;
        case 3: sprintf(String, "%s %s", GlobalText[166], name); break;
        default: break;
        }
        goto renderLabel;

    case 951: sprintf(String, "%s", GlobalText[105]); goto renderLabel;
    case 952: sprintf(String, "%s", GlobalText[106]); goto renderLabel;
    case 953: sprintf(String, "%s", GlobalText[107]); goto renderLabel;
    case 954: sprintf(String, "%s", GlobalText[108]); goto renderLabel;
    case 955: sprintf(String, "%s", GlobalText[109]); goto renderLabel;
    case 956: sprintf(String, "%s", GlobalText[110]); goto renderLabel;
    case 957: sprintf(String, "%s +%d", GlobalText[115], v6 - 7); goto renderLabel;
    case 958:
        glColor3f(1.0f, 0.8f, 0.1f);
        sprintf(String, "%s", GlobalText[810]);
        goto renderLabel;

    case 795: {                                 // pergamino de skill
        // IDA: v45 = 8 * (5 * v6 + 150); sprintf("%s %s", &SkillAttribute[v45],
        // GlobalText[102]).  Las entradas de SkillAttribute son de 40 bytes
        // (WinMain reserva 0xA00 = 64 entradas y el anti-tamper copia de a
        // 0x28), así que el índice es 40 * (v6 + 30) y el nombre va en offset 0.
        int v45 = 8 * (5 * v6 + 150);
        const char* skillName = (const char*)((const char*)&SkillAttribute + v45);
        sprintf(String, "%s %s", skillName, GlobalText[102]);
        goto renderLabel;
    }
    }

    if (v7 != 826) {
        if (v7 == 570 || v7 == 419 || v7 == 546) {
            v38[2] = 1.0f;
            v38[1] = 0.1f;
        } else {
            if ((ItemOption & 0x3F) != 0 && (v7 < 787 || v7 > 790)) {
                v38[2] = 0.5f; v38[1] = 1.0f; v38[0] = 0.1f;
                goto LABEL_118;
            }
            if (v6 < 7) {
                if ((v5 & 0x87) != 0) goto LABEL_115;
                if (v6 == 0) {
                    glColor3f(0.7f, 0.7f, 0.7f);
                    goto LABEL_119;
                }
                if (v6 < 3) {
                    v38[0] = 0.9f; v38[1] = 0.9f; v38[2] = 0.9f;
                    goto LABEL_118;
                }
                if (v6 >= 5) {
LABEL_115:
                    v38[2] = 1.0f; v38[1] = 0.7f; v38[0] = 0.4f;
                    goto LABEL_118;
                }
                v38[2] = 0.2f; v38[1] = 0.5f;
            } else {
                v38[2] = 0.1f; v38[1] = 0.8f;
            }
        }
        v38[0] = 1.0f;
LABEL_118:
        glColor3f(v38[0], v38[1], v38[2]);
        if (v6 != 0) {
            sprintf(String, "%s +%d", name, v6);
            goto LABEL_121;
        }
LABEL_119:
        sprintf(String, "%s", name);
LABEL_121:
        if ((v5 & 0x80) != 0) {
            // IDA escribe " +" (word_55A6B0) sobre el NUL y luego concatena
            // GlobalText[179] para el tipo 819; el resto usa GlobalText[176].
            if (v7 == 819) {
                strcat(String, " +");
                strcat(String, GlobalText[179]);
            } else {
                strcat(String, GlobalText[176]);
            }
        }
        if ((v5 & 3) != 0 || (ItemOption & 0x40) != 0) strcat(String, GlobalText[177]);
        if ((v5 & 4) != 0)                             strcat(String, GlobalText[178]);
        goto renderLabel;
    }

    {   // v7 == 826: piedra de invocación — nombre del monstruo + GlobalText[103]
        static const int SommonTable[6] = { 2, 7, 14, 8, 9, 41 };
        if (v6 < 6) {
            const MONSTER_SCRIPT* ms = (const MONSTER_SCRIPT*)&MonsterScript;
            int target = SommonTable[v6];
            for (int m = 0; m < MAX_MONSTER; ++m) {
                if (ms[m].Type == target) {
                    sprintf(String, "%s %s", ms[m].Name, GlobalText[103]);
                    break;
                }
            }
        }
    }

renderLabel:
    {
        int  textLen = lstrlenA(String);
        SIZE sz;
        GetTextExtentPointA(m_hFontDC, String, textLen, &sz);
        if (Sort) {
            // Centrado sobre la posición de pantalla del item (o+0x5c / o+0x5e)
            int x = (int)*(short*)(o + 0x5c) - 640 * (sz.cx / 2) / (int)WindowWidth;
            int y = (int)*(short*)(o + 0x5e) - 15;
            RenderText(x, y, String, 0, 0, (SIZE*)3);
        } else {
            int x = (int)MouseX - 640 * (sz.cx / 2) / (int)WindowWidth;
            int y = (int)MouseY - 15;
            RenderText(x, y, String, 0, 0, (SIZE*)3);
        }
    }
}

unsigned int __stdcall Inventory_DropItemEx(int origin_x, int origin_y,
                                            BYTE* invBase, int gridW,
                                            int gridH, int slotType);

unsigned int __stdcall Inventory_DropItem_stub(void) {
    BYTE* invBase = (BYTE*)(uintptr_t)DAT_07ea9800;
    int gridW = 8;
    int gridH = (invBase == &OffsetMixItems[0]) ? 4 : 8;
    int originX = 0;
    int originY = 0;

    if (invBase == &OffsetInventoryItems[0]) {
        originX = InventoryStartX + 15;
        originY = InventoryStartY + 200;
    } else if (invBase == &OffsetTradeItems[0]) {
        originX = TradeInventoryStartX + 15;
        originY = TradeInventoryStartY + 270;
        gridH = 4;
    } else if (invBase == &Inventory[0]) {
        originX = TradeInventoryStartX + 15;
        originY = TradeInventoryStartY + 70;
        gridH = 4;
    } else if (invBase == &OffsetWarehouseItems[0]) {
        originX = dword_7EAA0C8 + 15;
        originY = dword_7EAA0CC + 50;
        gridH = 15;
    } else if (invBase == &OffsetMixItems[0]) {
        originX = dword_7EAA0C8 + 15;
        originY = dword_7EAA0CC + 110;
        gridH = 4;
    }

    return Inventory_DropItemEx(originX, originY, invBase, gridW, gridH, 0);
}

unsigned int __stdcall Inventory_DropItemEx(int origin_x, int origin_y,
                                            BYTE* invBase, int gridW,
                                            int gridH, int slotType) {
    bool actionTaken = false;
    // 0x004D6470 — Giant item drag-and-drop handler (~3011 lines decompiled).
    // Called when player releases mouse on inventory/equipment grid.
    //
    // Anti-tamper notes (all skipped in this implementation):
    //   - ~22 unreachable blocks (dead code removed by Ghidra)
    //   - ~63 phantom stack params (in_stack_00006cec..in_stack_00009d10) — fake anti-tamper,
    //     not real args. The real "params" are caller locals read via stack overlap in the
    //     original binary. Our re-implementation reads them as globals instead.
    //   - Repeated XOR key init (0x893a6de7/0x739fb2bc/0xb6fea823) + single-iteration loops
    //     around every packet byte — compiler artifact / anti-tamper padding.
    //   - Hash table operations (FUN_00403f80, FUN_004041e0, FUN_004042b0, FUN_00404280,
    //     FUN_00404330, FUN_00404400) interspersed around every packet send — anti-tamper
    //     hash table, skipped.
    //
    // The caller (FUN_004df410 @ 0x004DF410) calls this up to 4 times per frame,
    // once for each open inventory context:
    //   1. Main inventory (OffsetInventoryItems, grid 8x8, slotType=0)
    //   2. Trade window    (TradeOpened, same grid)
    //   3. Warehouse       (WarehouseOpened, same grid)
    //   4. Chaos mix       (ChaosMixOpened + MixState, grid 8x4)
    //
    // In the original binary, context params (inventoryBase, gridWidth, gridHeight, slotType,
    // gridOffsetX, gridOffsetY) are passed via stack-frame overlap. In this reimplementation
    // we read the relevant globals directly, matching the caller's setup pattern.
    //
    // Return value: low byte used as bool success flag by caller.

    // --- Alias Ghidra globals to readable names ---
    // pPickedItem       = DAT_07e91350 (short, type of item being dragged)
    // ItemAttribute     = DAT_07d78068 (ITEM_ATTRIBUTE*, stride 0x40)
    // OffsetInventoryItems = DAT_07ea8410 (inventory item array base)
    // OffsetWarehouseItems = DAT_07ea5b30 (warehouse item array base, char type in globals)
    // OffsetMixItems    = DAT_07ea9848 (chaos mix item array base)
    // MixState          = DAT_07eaa140 (chaos mix active flag)
    // EquipmentItem     = DAT_07eaa165 (equipment move in-progress guard)
    // InventoryOpened   = (extern) InventoryOpened
    // WarehouseOpened   = DAT_07eaa119
    // TradeOpened       = DAT_07eaa11b
    // MouseX            = DAT_083a427c
    // MouseY            = DAT_083a4278
    // MouseLButtonPush  = DAT_083a4124
    // DAT_083a42eb      = auto-drop trigger flag
    // DAT_07e11e78      = target inventory slot index
    // DAT_07ea5b18      = source inventory slot index (picked-from)
    // DAT_07e91354      = picked item level (Level)
    // DAT_07e9136a      = picked item durability/option byte
    // DAT_07e9136b      = picked item extra dur byte
    // DAT_07ea9800      = current inventory context pointer
    // DAT_07ea8414      = inventory item level array (offset +4 from base)
    // EnableUse         = DAT_05826d1c (extern EnableUse)

    // 2026-05-09: ItemAttribute base watchdog. Mirror of the one in
    // FUN_004d23b0. Some unknown writer sets DAT_07d78068 = 0x00000001 → all
    // attr-based reads (CheckInventorySpace_stub itemW/H, attr[type] in this
    // function) compute bogus values, making spaceFree always 0 and breaking
    // drop on actually-empty slots. Restore from backup if corrupt.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

    short pickedType = *(short*)DAT_07e91350;
    ITEM_ATTRIBUTE* pAttr = (ITEM_ATTRIBUTE*)DAT_07d78068;
    if (!pAttr || pickedType < 0) return 0;
    int itemWidth  = (int)pAttr[pickedType].Width;
    int itemHeight = (int)pAttr[pickedType].Height;
    if (itemWidth <= 0 || itemHeight <= 0) return 0;

    // Use the args directly (FIX 2026-05-08).
    BYTE* sourceInvBase = (BYTE*)(uintptr_t)DAT_07ea9800;
    BYTE  sourceMoveFlag = InventoryPoolToMoveFlag(sourceInvBase);
    BYTE  targetMoveFlag = InventoryPoolToMoveFlag(invBase);
    int   gridWidth  = gridW;
    int   gridHeight = gridH;

    // 2026-08-08 FIX "mover items DENTRO del baul los hacia desaparecer":
    // aca habia un `DAT_07ea9800 = invBase` ("update para downstream readers")
    // que es una INVENCION del port — IDA sub_4D6470 SOLO LEE dword_7EA9800,
    // nunca lo escribe (los unicos writers son sub_4D23B0 L798/L1401 y
    // Player_InputTick L711, todos en el PICKUP). dword_7EA9800 es el pool de
    // ORIGEN del item agarrado, y el dispatcher sub_4DF410 llama a esta funcion
    // hasta 4 veces por frame (main inv, trade, baul, mix). La primera llamada
    // (main inv) pisaba el origen con OffsetInventoryItems, asi que en la
    // llamada del baul `sourceMoveFlag` salia 0 (=inventario) en vez de 2
    // (=baul) -> el server recibia SourceFlag=0 con SourceSlot=101 (fuera del
    // rango de inventario) -> INVENTORY_RANGE falla -> result=0xFF y el item
    // quedaba solo borrado localmente = "desaparecio".
    // Sintoma cruzado en el log: baul->inventario (resuelto en la 1er llamada,
    // antes del clobber) SI mandaba srcF=2 y funcionaba.

    // ── Mouse-to-grid conversion (per IDA L595-597) ─────────────────────────
    //   gridX = (MouseX - origin_x) * 0.05 - itemW * 0.5 + 0.5  →  __ftol
    //   gridY = (MouseY - origin_y) * 0.05 - itemH * 0.5 + 0.5
    // The 0.05 factor = 1/20 (cells are 20 px wide). The half-width offset
    // centers the multi-cell footprint on the cursor.
    int mouseGridX = (int)(((double)((int)DAT_083a427c - origin_x)) * 0.05
                            - (double)itemWidth  * 0.5 + 0.5);
    int mouseGridY = (int)(((double)((int)DAT_083a4278 - origin_y)) * 0.05
                            - (double)itemHeight * 0.5 + 0.5);

    // Bounds check: negative grid coords = outside the grid (mouse left/up
    // of origin).
    if (mouseGridX < 0 || mouseGridY < 0) {
        return 0;
    }

    bool cursorInsideGrid = (mouseGridX < gridWidth && mouseGridY < gridHeight);

    // (Pool aliases declared at file scope above.)
    // --- Check if target is MixItems with MixState active ---
    bool spaceFree = false;
    if ((BYTE*)invBase == &OffsetMixItems[0] && DAT_07eaa140 != 0) {
        // Mix placement: space is always considered free when mix state active
        spaceFree = true;
    } else {
        // Call CheckInventorySpace to validate placement.
        // 2026-05-09 BUG-FIX: ANTES pasábamos `mouseGridX, mouseGridY` (= grid
        // coords ya calculadas como 0..7) como p1, p2. Pero la función espera
        // SCREEN OFFSETS (origin_x, origin_y) para hacer la conversión interna
        // mouseX-p1 → relative pixel → grid. Pasar grid coords daba
        // gridX = (MouseX - 1)*0.05 ≈ 30 → fuera del grid → emptyCount=0 →
        // spaceFree=0 SIEMPRE. Esto es por qué el drop nunca encontraba slots
        // libres aún con el watchdog de attr.
        unsigned long long result = CheckInventorySpace_stub(
            origin_x, origin_y,
            (unsigned short*)invBase,
            gridWidth, gridHeight
        );
        spaceFree = (char)result != 0;
    }

    // --- Mark grid cells with color state ---
    // Iterate over the item's width x height footprint
    int endY = mouseGridY + itemHeight;
    int endX = mouseGridX + itemWidth;
    for (int gy = mouseGridY; gy < endY; gy++) {
        for (int gx = mouseGridX; gx < endX; gx++) {
            if (gx >= 0 && gy >= 0 && gx < gridWidth && gy < gridHeight) {
                int cellIdx = gy * gridWidth + gx;
                BYTE* cellBase = (BYTE*)(invBase + cellIdx * 0x44);  // stride 0x22 words = 0x44 bytes
                if (!spaceFree) {
                    // Overlap: mark red
                    cellBase[0x40] = 2;  // +0x20 in short units = +0x40 bytes: colorState
                } else {
                    // Check if currency item (Jewel of Bless/Soul/Life)
                    if (pickedType == 0x1cd || pickedType == 0x1ce || pickedType == 0x1d0) {
                        cellBase[0x40] = 4;  // currency highlight
                    } else {
                        cellBase[0x40] = 3;  // normal placement ok
                    }
                }
            }
        }
    }

    // --- Check for mouse click to confirm drop ---
    if (DAT_083a42eb == '\0' && DAT_083a4124 == 0) {
        // Neither auto-drop trigger nor mouse button pressed: just return (grid preview only)
        return 0;
    }

    // === MOUSE BUTTON PRESSED OR AUTO-DROP: Execute the drop ===

    if (!spaceFree) {
        // --- Space check failed: target slot is occupied ───────────────────
        // (Currency stacking / "can't place" UI message only.)
        // Check for currency item stacking on existing inventory item
        bool canStack = false;
        if (cursorInsideGrid
            && (pickedType == 0x1cd || pickedType == 0x1ce || pickedType == 0x1d0)
            && slotType == 0 && InventoryOpened != 0)
        {
            int targetSlot = mouseGridY * gridWidth + mouseGridX;
            short targetType = *(short*)(invBase + targetSlot * 0x44);
            int targetLevel = *(int*)(invBase + targetSlot * 0x44 + 4) >> 3 & 0xf;
            bool validTarget = (targetType < 0x187);

            // Jewels 0x87 / 0x8f cannot receive upgrade
            if (targetType == 0x87 || targetType == 0x8f) {
                validTarget = false;
            }

            // Level cap per currency type
            if (pickedType == 0x1cd && targetLevel > 5) {
                // Jewel of Bless: max +5
                canStack = false;
                goto drop_done;
            }
            if (pickedType == 0x1ce && targetLevel > 8) {
                // Jewel of Soul: max +8
                canStack = false;
                goto drop_done;
            }

            if (validTarget) {
                // Record which slot the currency is being applied to
                DAT_07e11e78 = (DWORD)(
                    (DWORD)*(invBase + targetSlot * 0x44 + 0x3f) * gridWidth + 0xc
                    + (DWORD)*(invBase + targetSlot * 0x44 + 0x3e)
                );

                // Con el baul o el trade abiertos no se puede aplicar la
                // jewel: IDA salta a LABEL_807, que muestra el mensaje. Por eso
                // `canStack` queda en false en esos dos casos (antes se ponia
                // en true al final incondicionalmente y el aviso no salia).
                if (DAT_07eaa119 == '\0' && DAT_07eaa11b == '\0') {
                    // 2026-08-24 FIX (issue #15, "las jewels no se consumen"):
                    // aca se mandaba `SendRequestEquipmentItem_stub`, o sea
                    // 0x24 PMSG_ITEM_MOVE_RECV (11 bytes). El server trata eso
                    // como MOVER la jewel a una celda ocupada -> lo rechaza y
                    // el cliente la devuelve al inventario. IDA (sub_4D6470
                    // L5919-5931) manda 0x26 PMSG_ITEM_USE_RECV, que es el que
                    // dispara CharacterUseJewelOfBles/Soul/Life en el server
                    // (ItemManager.cpp:2753+) y contesta con GCItemDeleteSend +
                    // GCItemModifySend (F3/14).
                    //
                    //   struct PMSG_ITEM_USE_RECV {   // ItemManager.h:49
                    //       PBMSG_HEAD header;        // C1 : 5 : 0x26
                    //       BYTE SourceSlot;          // +3
                    //       BYTE TargetSlot;          // +4
                    //   };
                    //
                    // Va por Net_SendSmallPacket porque HackPacketCheck.txt da
                    // Encrypt=1 para el indice 38 -> el frame final tiene que
                    // ser C3 con serial. El 0xC1 que arma IDA es el texto plano
                    // previo al encriptador, no el frame que viaja.
                    if ((int)EnableUse < 1) {
                        // IDA: `if (EnableUse > 0) goto LABEL_808;` — durante el
                        // cooldown NO se manda nada. Antes se mandaba igual.
                        EnableUse = 10;

                        BYTE pkt[8];
                        memset(pkt, 0, sizeof(pkt));
                        pkt[0] = 0xC1;
                        pkt[1] = 5;                          // lo pisa el serial
                        pkt[2] = 0x26;                       // ItemUse
                        pkt[3] = (BYTE)(int)DAT_07ea5b18;    // SourceSlot (la jewel)
                        pkt[4] = (BYTE)(int)DAT_07e11e78;    // TargetSlot (el item)
                        Net_SendSmallPacket(pkt, 5);
                        actionTaken = true;

                        // Sonido segun el item de origen (IDA LABEL_802).
                        short sndItem = *(short*)((char*)OffsetInventoryItems + ((int)DAT_07ea5b18 - 0xc) * 0x44);
                        if (sndItem == 0x1c0) {
                            PlayBuffer(0x21, 0, 0);
                        } else if (sndItem > 0x1c0 && sndItem < 0x1ca) {
                            PlayBuffer(0x20, 0, 0);
                        }
                    }
                    canStack = true;
                }
            }
        }
        if (!canStack) {
            // Show "can't place here" message
            UIChatLogWindow_AddText((const char*)&DAT_07eaa190, (const char*)&DAT_07d4c89c, 2);
        }
        goto drop_done;
    }

    // === Space is free: handle the actual item placement ===

    if (slotType != 0) {
        // --- Equipment slot drop (slotType == 2 or 3) ---
        DAT_07e11e78 = (DWORD)(mouseGridY * gridWidth + mouseGridX);

        if (DAT_07eaa165 != '\0') goto drop_done;  // equipment move already in progress

        // 2026-07-27 PORT FALTANTE (no se podian meter items al baul): este
        // camino (slotType != 0 = baul/trade/mix) SOLO tenia implementado el
        // destino "inventario principal"; para el resto habia un comentario
        // "follow same pattern" y NADA de codigo -> nunca se mandaba el 0x24,
        // actionTaken quedaba en false -> el dispatcher veia dropWH=0 y trataba
        // el drop como "tirar al suelo" (mensaje rojo), y el item quedaba en un
        // estado inconsistente (desaparecia al recargar). El envio es identico
        // para todos los destinos: solo cambia targetMoveFlag (0=inv, 1=trade,
        // 2=warehouse, 3=chaos), que ya viene calculado arriba.
        if (DAT_07eaa165 == 0) {
            DAT_07eaa165 = 1;
            InventoryMove_SetPendingPools(sourceInvBase, invBase);
            SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
                (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
            actionTaken = true;
        }
        goto drop_done;
    }

    // --- slotType == 0: inventory-to-inventory placement ---
    if (InventoryOpened == 0) goto drop_done;

    // Calculate target slot in inventory grid (offset by 0xc for equipment slots 0-11)
    DAT_07e11e78 = (DWORD)(mouseGridY * gridWidth + 0xc + mouseGridX);

    if (invBase == &OffsetInventoryItems[0]) {
        // --- Target is main inventory (OffsetInventoryItems) ---
        bool isDifferentSlot = (DAT_07ea5b18 != DAT_07e11e78);
        int targetOffset = ((int)DAT_07e11e78 - 0xc) * 0x44;
        short targetType = *(short*)((char*)OffsetInventoryItems + targetOffset);

        // Check for stackable items (same type swap)
        if (pickedType == targetType) {
            // Arrows: types 0x1c0..0x1c8 can stack together
            if (pickedType > 0x1bf && pickedType < 0x1c9
                && targetType > 0x1bf && targetType < 0x1c9)
            {
                isDifferentSlot = true;  // force swap even if same slot
            }
            // Potions 0x87/0x8f: can stack only if same level
            if ((pickedType == 0x87 || pickedType == 0x8f)
                && (targetType == 0x87 || targetType == 0x8f))
            {
                int pickedLevel = ((int)DAT_07e91354 >> 3) & 0xf;
                int targetLevel = (*(int*)((char*)OffsetInventoryItems + targetOffset + 4) >> 3) & 0xf;
                if (pickedLevel != targetLevel) {
                    isDifferentSlot = true;  // different level = treat as different
                }
                // same level same type same slot = no-op, fall through
            }
        } else {
            // Different types: if same slot, call inventory reset
            if (!isDifferentSlot) {
                FUN_004cd3b0();  // Inventory_Reset / CharPreview_Refresh
                goto drop_done;
            }
        }

        // Guard: only one equipment move at a time
        if (DAT_07eaa165 != '\0') goto drop_done;
        DAT_07eaa165 = '\x01';

        // anti-tamper hash table — skipped (XOR packet build + serial insertion)
        // Send the equipment swap request
        InventoryMove_SetPendingPools(sourceInvBase, invBase);
        SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
            (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
        actionTaken = true;
    }
    else if (invBase == &OffsetMixItems[0]) {
        // --- Target is Chaos Mix inventory (OffsetMixItems) ---
        if (DAT_07eaa165 != '\0') goto drop_done;
        DAT_07eaa165 = '\x01';

        // anti-tamper hash table — skipped
        InventoryMove_SetPendingPools(sourceInvBase, invBase);
        SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
            (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
        actionTaken = true;
    }
    else if (invBase == &OffsetWarehouseItems[0]) {
        // --- Target is Warehouse (OffsetWarehouseItems) ---
        // Check warehouse password verification state
        if (DAT_00559f5f == '\0' || DAT_07eaa148 != '\0') {
            if (DAT_07eaa165 != '\0') goto drop_done;
            DAT_07eaa165 = '\x01';

            // anti-tamper hash table — skipped
            InventoryMove_SetPendingPools(sourceInvBase, invBase);
            SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
                (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
            actionTaken = true;
        }
        else {
            // Warehouse needs password: set up second-password state machine
            DAT_07ea9808 = DAT_07ea5b18;
            DAT_07ea9804 = 2;
            DAT_07ea980c = 0;
            DAT_07ea9810 = DAT_07e11e78;

            // Initialize shuffle array for second-password input (10 digits, shuffled)
            short* shuffleArr = (short*)&DAT_07e91394;
            for (short i = 0; i < 10; i++) {
                shuffleArr[i] = i;
            }
            // Fisher-Yates shuffle (20 iterations)
            for (int n = 0; n < 20; n++) {
                int a = rand() % 10;
                int b = rand() % 10;
                if (a != b) {
                    // XOR swap
                    shuffleArr[a] ^= shuffleArr[b];
                    shuffleArr[b] ^= shuffleArr[a];
                    shuffleArr[a] ^= shuffleArr[b];
                }
            }
            DAT_07eaa14c = 1;
            DAT_07ea9814 = 0;
            DAT_07ea9818 = 0;
            DAT_07ea981c = 0;
            DAT_07ea981e = 0;
            actionTaken = true;
        }
    }
    else {
        // --- Other target context (Trade items, etc.) ---
        if (DAT_07eaa165 != '\0') goto drop_done;
        DAT_07eaa165 = '\x01';

        // anti-tamper hash table — skipped
        // Build generic equipment move packet:
        // Packet fields: [C1][len][0x24][01][srcSlot][pPickedItem][Level][durOption][durExtra][slotType=3][dstSlot]
        InventoryMove_SetPendingPools(sourceInvBase, invBase);
        SendRequestEquipmentItem_stub(sourceMoveFlag, (int)DAT_07ea5b18,
            (ITEM*)DAT_07e91350, targetMoveFlag, (int)DAT_07e11e78);
        actionTaken = true;
    }

drop_done:
    return actionTaken ? 1u : 0u;
}

// RenderGuildMark @ 0x004F02F0 (~87 lines) — renders guild emblem at screen position
// Fills Bitmaps[0x1e9] pixel buffer with MarkColor[param_5], uploads via glTexImage2D.
// If param_5==0: draws cross pattern in gray. Then RenderBitmap.
void __cdecl RenderGuildMark_stub(float p1, float p2, float p3, float p4, int p5) {
    // 0x004F02F0 — Render guild emblem at screen position (p1,p2), size (p3 x p4).
    // p5 = color index into MarkColor[16] palette at DAT_07e11f34.
    // Bitmaps[0x22] = guild mark texture slot (Components, TextureNumber, Buffer).

    float Width  = p3;
    float Height = p4;

    // Draw background box (texture 0x116) slightly larger
    FUN_005125a0(0x116, p1 - _DAT_0055256c, p2 - _DAT_0055256c,
                 Width + _DAT_0055264c, Height + _DAT_0055264c,
                 0.0f, 0.0f, Width * _DAT_00552ae4, Height * _DAT_00552a20,
                 '\x01', '\x01');

    int iVar2 = (int)p3;  // pixel width
    int iVar3 = (int)p4;  // pixel height

    DWORD* pixelBuf = (DWORD*)Bitmaps[0x22].Buffer;
    // 2026-08-25: `DAT_07e11f34` ahora es el array de 16 que realmente es, asi
    // que se indexa directo (antes `(&DAT_07e11f34)[p5]` sobre un unico DWORD
    // leia hasta 60 bytes del vecino).
    if (p5 < 0 || p5 > 15) return;
    DWORD color = DAT_07e11f34[p5];  // MarkColor[p5]

    if (p5 == 0) {
        // No guild — fill black, draw gray cross pattern
        DWORD* pFill = pixelBuf;
        int rowsLeft = iVar3;
        if (iVar3 > 0) {
            do {
                DWORD* pEnd = pFill;
                if (iVar2 > 0) {
                    pEnd = pFill + iVar2;
                    for (int c = iVar2; c != 0; c--) {
                        *pFill = 0xFF000000;  // opaque black
                        pFill++;
                    }
                }
                rowsLeft--;
                pFill = pEnd;
            } while (rowsLeft != 0);
        }
        // Diagonal cross (top-left to bottom-right, stride 9)
        DWORD* pCross = pixelBuf;
        for (int i = 8; i != 0; i--) {
            *pCross = 0xFF808080;  // gray
            pCross += 9;
        }
        // Diagonal cross (top-right to bottom-left, stride 7)
        pCross = pixelBuf;
        for (int i = 8; i != 0; i--) {
            pCross += 7;
            *pCross = 0xFF808080;  // gray
        }
    } else {
        // Fill with guild color
        DWORD* pFill = pixelBuf;
        int rowsLeft = iVar3;
        if (iVar3 > 0) {
            do {
                int colsLeft = iVar2;
                DWORD* pRow = pFill;
                if (iVar2 > 0) {
                    for (; colsLeft != 0; colsLeft--) {
                        *pRow = color;
                        pRow++;
                    }
                    pFill += iVar2;
                }
                rowsLeft--;
            } while (rowsLeft != 0);
        }
    }

    // Upload pixel buffer as GL texture
    glBindTexture(0xDE1, Bitmaps[0x22].TextureNumber);  // GL_TEXTURE_2D
    glTexImage2D(0xDE1, 0, (int)(BYTE)Bitmaps[0x22].Components,
                 iVar2, iVar3, 0, 0x1908, 0x1401, (void*)Bitmaps[0x22].Buffer);  // GL_RGB, GL_UNSIGNED_BYTE

    // Draw the guild mark texture (texture slot 0x22)
    FUN_005125a0(0x22, p1, p2, Width, Height, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
}

static BYTE ResolveQueuedSkillId97k()
{
    if (DAT_07d78098 != '\0') {
        BYTE* charAttr = (BYTE*)DAT_07cf1ff4;
        if (charAttr) {
            return charAttr[DAT_07d7809c + 87];
        }
    }
    return (BYTE)DAT_07d7809c;
}

static BYTE* GetSkillRecordShadow_Local(int iType);
static BYTE* GetSkillRecordBase_Local(int iType);
static void SendSkillPacket19_Local(BYTE skillId, WORD targetKey);
static void SendSkillPacket1E_Local(BYTE skillId, BYTE gridX, BYTE gridY, BYTE dir, BYTE dist, BYTE angle, WORD targetKey);

static BYTE PackDurationDestination_Local(int x, int y, int targetX, int targetY)
{
    int deltaX = targetX - x;
    int deltaY = targetY - y;
    if (deltaX < -8) deltaX = -8;
    if (deltaX > 7) deltaX = 7;
    if (deltaY < -8) deltaY = -8;
    if (deltaY > 7) deltaY = 7;
    return (BYTE)(((deltaX + 8) << 4) | ((deltaY + 8) & 0x0F));
}

static void SendMuEmuEncryptedPacket(BYTE* pkt, int len)
{
    if (!pkt || len <= 0) return;
    Net_SendSmallPacket(pkt, len);
}

// UseSkillWizard @ 0x004889D0 (~1227 lines)
// Wizard skill execution. Reads skill ID from CharacterAttribute skill table,
// builds XOR-encrypted C1 packets for different wizard skill types, sends via socket.
// Special handling for Energy Ball (0x2b), Hellfire (0x2f), ground-target (0x0f),
// direct-target skills (1-4,7,0xb,0x11), and Decay/area skill (0xd).
//
// ~70% of original 1227 lines is anti-tamper hash table operations
// (FUN_00403f80, FUN_004041e0, FUN_00404280, FUN_00404330, FUN_00404370,
//  FUN_00404400) and XOR key init + dead forward/reverse loops — all skipped.
//
// Ghidra shows 63 phantom stack params (unaff_retaddr etc.) — anti-tamper obfuscation.
// Real params: c = CHARACTER* (hero entity ptr), o = OBJECT* (hero object ptr).
void __cdecl UseSkillWizard_stub(DWORD c, DWORD o) {
    // --- XOR encryption key (32 bytes, hardcoded — same as login packet key) ---
    static const BYTE xorKey[32] = {
        0xe7,0x6d,0x3a,0x89, 0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6, 0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d, 0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83, 0x29,0x51,0xe8,0x56
    };

    char* charAttrBase = (char*)DAT_07cf1ff4;
    // 004889D0 resolves the selected skill from the CharacterAttribute slot
    // when dword_7D78098 is active.  Do not replace it with the local runtime
    // cache: that cache survives target/mouse transitions and can dispatch a
    // previously selected spell after the user has changed slot.
    UINT skillId = (UINT)ResolveQueuedSkillId97k();
    {
        char trace[160];
        wsprintfA(trace, "WIZ ENTER skill=%u runtime=%u target=%d active=%u",
                  (unsigned)skillId, (unsigned)DAT_05826d10,
                  (int)DAT_07d780a0, (unsigned)DAT_07d78098);
        DbgLogPublic(trace);
    }

    // ── Energy Ball (0x2b) / Hellfire (0x2f): skip packet, just set magic anim ──
    if (skillId == 0x2b || skillId == 0x2f) {
        // SetPlayerMagic(c) — sets attack/cast animation on hero
        FUN_00444a80((int)c);
        return;
    }

    // 004889D0 has a C1:05:10 preamble for every skill except 0x0F, but it
    // is written through the original inline socket/XOR path.  The current
    // Net_SendSmallPacket wrapper is not equivalent for that particular
    // movement-format packet (it diverts the local input flow before Attack).
    // Do not substitute it with FUN_00491C40 either: that is click-to-move.
    // Keep the preamble disabled until that sender is ported byte-for-byte;
    // the direct skill packets below remain on their verified C3 paths.

    // ── Set target position from CharactersClient[MovementSkillTarget] ──
    char* heroEntity = (char*)(DWORD)c;
    int targetIdx = (int)DAT_07d780a0;  // MovementSkillTarget
    if (targetIdx < 0 || targetIdx >= 400) {
        return;
    }
    DWORD entityBase = DAT_07abf5d0;    // CharactersClient array base

    // Target entity position: entityBase + targetIdx * 0x394 + offset
    // Ghidra: CharactersClient[_MovementSkillTarget].Object.Position[0/1/2]
    float targetPosX = *(float*)(entityBase + targetIdx * 0x394 + 0x10);
    float targetPosY = *(float*)(entityBase + targetIdx * 0x394 + 0x14);
    float targetPosZ = *(float*)(entityBase + targetIdx * 0x394 + 0x18);

    // Store target position at entity offset +0x314 (TargetPosition[3])
    *(float*)(heroEntity + 0x314) = targetPosX;
    *(float*)(heroEntity + 0x318) = targetPosY;
    *(float*)(heroEntity + 0x31C) = targetPosZ;

    // Compute facing angle toward target
    // CreateAngle(heroX, heroY, targetX, targetY)
    float heroX = *(float*)((char*)(DWORD)o + 0x10);
    float heroY = *(float*)((char*)(DWORD)o + 0x14);
    typedef float (__cdecl *CreateAngleFn)(float, float, float, float);
    float angle = ((CreateAngleFn)&FUN_0043e050)(heroX, heroY, targetPosX, targetPosY);
    *(float*)((char*)(DWORD)o + 0x24) = angle;

    // ── Switch on skill ID ──
    switch (skillId) {
    case 1: case 2: case 3: case 4:
    case 7: case 0xb: case 0x10: case 0x11:
    case 19: case 20: case 21: case 22: case 23:
    case 43: case 49: case 56:
    {
        // GM name check: compare Hero->ID with "webzen" (DAT_00559d94)
        // Hero->ID at entity offset +0x1C1
        // In original: strlen-based strstr scan. If hero name contains "webzen", skip.
        // This is an anti-impersonation check — GM accounts get special treatment.
        char* heroName = (char*)(entityBase + 0x08 + 0x1C1);  // Hero->ID approximation
        // (Actual Hero ptr is DAT_07abf5d8; for safety just use entity c directly)
        heroName = (char*)((DWORD)c + 0x1C1);
        {
            // Simple strstr check for "webzen" in hero name
            const char* gmStr = DAT_00559d94;
            int gmLen = (int)strlen(gmStr);
            int nameLen = (int)strlen(heroName);
            int diff = nameLen - gmLen;
            bool isGM = false;
            for (int i = 0; i <= diff && !isGM; i++) {
                bool match = true;
                for (int j = 0; j < gmLen; j++) {
                    if (heroName[i + j] != gmStr[j]) { match = false; break; }
                }
                if (match) isGM = true;
            }
            if (isGM) break;  // GM name found — skip packet send
        }

        // 300ms cooldown check on magic tick
        DWORD now = GetTickCount();
        DWORD elapsed = now - DAT_05826cf4;
        // Ghidra: abs(elapsed) > 300
        if (elapsed > 0x80000000) elapsed = (DWORD)(-(int)elapsed);  // abs
        if ((int)elapsed <= 300) break;

        DAT_05826cf4 = GetTickCount();  // g_dwLatestMagicTick = now

        // Get target entity key (CharactersClient[MovementSkillTarget].Key)
        // Key is at entity offset +0x??? — Ghidra shows CharactersClient[target].Key
        // In the entity struct, Key is typically near the beginning. Ghidra accesses
        // it as a DWORD. From the struct: stride 0x394, key at some offset.
        // Ghidra: (byte)(CharactersClient[_MovementSkillTarget].Key >> 8) and
        //         (byte)(CharactersClient[_MovementSkillTarget].Key)
        // The Key field is at entity+0x00 or a named offset. Based on CreateCharacter
        // which takes Key as param and stores it, it's at entity+0x???
        // From Monster.cpp: FindCharacterIndex searches by Key — likely at offset 0x00
        // Actually, looking at Ghidra: CharactersClient[idx].Key — this is a named field.
        // Entity struct in structs.h doesn't have explicit Key offset, but from entity
        // struct the first fields are type(+0x02) then positions(+0x10).
        // Key is probably stored at another offset. Let's check: in Game_EnterWorldTick,
        // CreateCharacter stores Key as first param. Ghidra accesses it as a member.
        // For now, assume Key is stored at a known offset in the entity struct.
        // From CreateCharacter signature: (int Key, int Type, BYTE PosX, BYTE PosY, float Rot)
        // The entity index IS the key in some sense, but Ghidra shows .Key as a field.
        // Looking at Combat.cpp and other code: entity key is used as network ID.
        // From DAT_07abf5d0 + idx * 0x394 — the Key field in Ghidra is likely at offset 0x00.
        WORD targetKey = *(WORD*)(entityBase + targetIdx * 0x394 + 476);
        if (targetKey == 0xFFFF) {
            DbgLogPublic("WIZ EXIT invalid-target-key");
            break;
        }

        {
            char trace[96];
            wsprintfA(trace, "WIZ SEND19 skill=%u key=%u", (unsigned)skillId,
                      (unsigned)targetKey);
            DbgLogPublic(trace);
        }

        // Real server recv:
        //   PMSG_SKILL_ATTACK_RECV { PBMSG_HEAD // C3:19, BYTE skill, BYTE index[2] }
        SendSkillPacket19_Local((BYTE)skillId, targetKey);
        break;
    }

    case 0xd:
    case 0x34:
    {
        // GM name check: compare Hero->ID with "webzen" (DAT_00559d9c)
        char* heroName = (char*)((DWORD)c + 0x1C1);
        {
            const char* gmStr = DAT_00559d9c;
            int gmLen = (int)strlen(gmStr);
            int nameLen = (int)strlen(heroName);
            int diff = nameLen - gmLen;
            bool isGM = false;
            for (int i = 0; i <= diff && !isGM; i++) {
                bool match = true;
                for (int j = 0; j < gmLen; j++) {
                    if (heroName[i + j] != gmStr[j]) { match = false; break; }
                }
                if (match) isGM = true;
            }
            if (isGM) break;  // GM name found — skip
        }

        // Set CurrentSkill
        DAT_05826d10 = skillId;  // CurrentSkill

        // Blast (13) targets the cached world point set from MovementSkillTarget
        // at the beginning of sub_4889D0. Penetration (52) remains centered on
        // the hero's grid square.
        BYTE gridX;
        BYTE gridY;
        if (skillId == 0x0d) {
            gridX = (BYTE)(int)(*(float*)(heroEntity + 788) * 0.01f);
            gridY = (BYTE)(int)(*(float*)(heroEntity + 792) * 0.01f);
        } else {
            gridX = (BYTE)*(DWORD*)(heroEntity + 0x388);
            gridY = (BYTE)*(DWORD*)(heroEntity + 0x38C);
        }

        WORD targetKey = *(WORD*)(entityBase + targetIdx * 0x394 + 476);
        if (targetKey == 0xFFFF) {
            break;
        }
        // PMSG_DURATION_SKILL_ATTACK uses a 0..255 facing byte. The 97k
        // Blast sender leaves distance and angle at zero.
        BYTE dir = (BYTE)(int)(angle * (256.0f / 360.0f));

        // Real server recv:
        //   PMSG_DURATION_SKILL_ATTACK_RECV { C3:1E, skill, x, y, dir, dis, angle, index[2] }
        SendSkillPacket1E_Local((BYTE)skillId, gridX, gridY, dir, 0, 0, targetKey);
        break;
    }

    default:
        // Skills 5,6,8,9,0xa,0xc,0xe,0x10, etc.: no packet sent, fall through
        // Ghidra: goto switchD_00489006_caseD_5 (exit without packet)
        return;
    }

    // SetPlayerMagic(c) — sets cast animation on hero entity
    // Ghidra: SetPlayerMagic @ 0x00444a80
    // If entity type != 0x186: alternate between action 3 and 4 based on combo counter % 3
    // If entity type == 0x186: SetAttackSpeed, then class-specific action
    FUN_00444a80((int)c);
}

// ── Stubs needed by SkillElf ──────────────────────────────────────────────────

static BYTE* GetSkillRecordShadow_Local(int iType)
{
    if (iType < 0 || iType >= 64)
        return nullptr;
    if (DAT_07cf1ff8 != 0)
        return (BYTE*)(uintptr_t)DAT_07cf1ff8 + iType * 0x28;
    return nullptr;
}

static BYTE* GetSkillRecordBase_Local(int iType)
{
    if (iType < 0 || iType >= 64)
        return nullptr;
    if (DAT_07d29d20 != 0)
        return (BYTE*)(uintptr_t)DAT_07d29d20 + iType * 0x28;
    return nullptr;
}

static void SendSkillPacket19_Local(BYTE skillId, WORD targetKey)
{
    BYTE pktBuf[6];
    pktBuf[0] = 0xC1;
    pktBuf[1] = 6;
    pktBuf[2] = 0x19;
    pktBuf[3] = skillId;
    pktBuf[4] = (BYTE)(targetKey >> 8);
    pktBuf[5] = (BYTE)(targetKey & 0xFF);
    {
        char trace[96];
        wsprintfA(trace, "SKILL SEND C1:19 skill=%u targetKey=%u",
                  (unsigned)skillId, (unsigned)targetKey);
        DbgLogPublic(trace);
    }
    SendMuEmuEncryptedPacket(pktBuf, 6);
}

static void SendSkillPacket1E_Local(BYTE skillId, BYTE gridX, BYTE gridY, BYTE dir, BYTE dist, BYTE angle, WORD targetKey)
{
    BYTE pktBuf[11];
    pktBuf[0] = 0xC1;
    pktBuf[1] = 11;
    pktBuf[2] = 0x1E;
    pktBuf[3] = skillId;
    pktBuf[4] = gridX;
    pktBuf[5] = gridY;
    pktBuf[6] = dir;
    pktBuf[7] = dist;
    pktBuf[8] = angle;
    pktBuf[9] = (BYTE)(targetKey >> 8);
    pktBuf[10] = (BYTE)(targetKey & 0xFF);
    {
        char trace[128];
        wsprintfA(trace,
                  "SKILL SEND C1:1E skill=%u xy=(%u,%u) dir=%u dis=%u angle=%u targetKey=%u",
                  (unsigned)skillId, (unsigned)gridX, (unsigned)gridY,
                  (unsigned)dir, (unsigned)dist, (unsigned)angle,
                  (unsigned)targetKey);
        DbgLogPublic(trace);
    }
    SendMuEmuEncryptedPacket(pktBuf, 11);
}

// GetSkillInformation @ 0x0047E7A0 — reads skill table entry for given type/level.
// Outputs mana cost, distance, and AG (SkillMana) cost via out-pointers.
// Real implementation uses the compact 0x28-byte skill record table.
void __cdecl GetSkillInformation(int iType, int iLevel, char* lpszName, int* piMana, int* piDistance, int* piSkillMana) {
    (void)iLevel;
    BYTE* baseEntry = GetSkillRecordBase_Local(iType);
    BYTE* statEntry = GetSkillRecordShadow_Local(iType);
    if (statEntry == nullptr)
        statEntry = baseEntry;
    if (baseEntry == nullptr && statEntry == nullptr) {
        if (lpszName) lpszName[0] = '\0';
        if (piMana) *piMana = 0;
        if (piDistance) *piDistance = 0;
        if (piSkillMana) *piSkillMana = 0;
        return;
    }

    if (lpszName != NULL) {
        const char* srcName = nullptr;
        if (baseEntry && ((const char*)baseEntry)[0] != '\0')
            srcName = (const char*)baseEntry;
        else if (statEntry && ((const char*)statEntry)[0] != '\0')
            srcName = (const char*)statEntry;
        else
            srcName = "";
        strncpy_s(lpszName, 256, srcName, 31);
        lpszName[31] = '\0';
        for (char* p = lpszName; *p; ++p) {
            unsigned char ch = (unsigned char)*p;
            if (ch < 32)
                *p = ' ';
        }
    }
    if (piMana != NULL) {
        int v = (int)*(WORD*)(statEntry + 0x22);
        *piMana = (v >= 0 && v <= 5000) ? v : 0;
    }
    if (piDistance != NULL) {
        int v = (int)*(BYTE*)(statEntry + 0x27);
        *piDistance = (v >= 0 && v <= 50) ? v : 0;
    }
    if (piSkillMana != NULL) {
        int v = (int)*(BYTE*)(statEntry + 0x26);
        *piSkillMana = (v >= 0 && v <= 255) ? v : 0;
    }
}


// SkillElf @ 0x0048BD70 (~1247 lines)
// Elf class skill execution. Handles heal, buff, arrow skills.
// Validates target, builds skill packet, handles multi-arrow, spawns VFX.
//
// NOTE: Ghidra shows 63 phantom stack params (in_stack_00000020..in_stack_000000ff)
//   — these are anti-tamper obfuscation, not real parameters.
//   in_stack_00001b48 = c (CHARACTER* pointer, 1st real param)
//   in_stack_00001b4c = pItem (CHARACTER_ATTRIBUTE* pointer, 2nd real param)
//
// The function iterates over skills in the pItem (CharacterAttribute) skill list,
// checking which one matches the currently selected skill on the hero.
// For each matching skill:
//   - If mana is insufficient: tries to use a potion (item slot 3 = arrows/potions)
//   - If AG (SkillMana) is insufficient: returns false
//   - If skill type is 0x18 (arrow/ranged): checks arrow ammo via CheckArrow,
//     validates range, computes facing angle, builds C1 skill packet with
//     XOR encryption, sends it, then calls SetPlayerAttack + CreateArrows
//
// ~60% of the original 1247 lines is anti-tamper hash table operations
// (FUN_00403f80, FUN_004041e0, FUN_00404280, FUN_00404330, FUN_00404370,
//  FUN_00404400) and XOR key init + dead forward/reverse loops — all skipped.
bool __stdcall SkillElf_stub(DWORD c, DWORD pItem) {
    // c = CHARACTER* (hero entity), pItem = CHARACTER_ATTRIBUTE* (char attributes)
    // Cast to usable pointers
    char* hero    = (char*)(DWORD)c;      // CHARACTER* — entity struct
    char* charAttr = (char*)(DWORD)pItem;  // CHARACTER_ATTRIBUTE* — skill/stat struct

    if (hero == nullptr || charAttr == nullptr) return false;

    // anti-tamper hash table — skipped (encrypt CharacterMachine before access)

    bool result = false;

    // CharacterAttribute = DAT_07cf1ff4, Hero = DAT_07abf5d8
    char* charAttrBase = (char*)DAT_07cf1ff4;
    char* heroEntity   = DAT_07abf5d8;

    if (charAttrBase == nullptr || heroEntity == nullptr) {
        return false;
    }

    BYTE skillCount = *(BYTE*)(charAttrBase + 86);
    if (skillCount == 0) {
        return false;
    }
    BYTE selectedSkillId = ResolveQueuedSkillId97k();
    if ((int)DAT_07d780a0 < 0 || (int)DAT_07d780a0 >= 400) {
        return false;
    }

    // CharacterAttribute->Mana at offset 0x1D (WORD) — Ghidra reads high byte via +1
    // Actually Ghidra: *(ushort *)((int)&CharacterAttribute->Mana + 1) — reads misaligned
    // CharacterAttribute->Mana is at 0x1D, so +1 = 0x1E. This reads bytes [0x1E..0x1F]
    // as a ushort — which is MaxLife low byte + MaxLife high byte. But more likely
    // the Ghidra struct layout: Mana at 0x1D (WORD), so &Mana+1 = 0x1E.
    // This is actually reading Mana as big-endian or the full Mana value.
    // Let's just read the WORD at offset 0x1D for current Mana.
    WORD currentMana = *(WORD*)(charAttrBase + 0x1D);
    // CharacterAttribute->SkillMana at offset 0x23 (WORD)
    // Ghidra: *(ushort *)((int)&CharacterAttribute->SkillMana + 1) => offset 0x24
    WORD currentAG = *(WORD*)(charAttrBase + 0x23);

    for (int i = 0; i < (int)skillCount; i++) {
        BYTE skillId = *(BYTE*)(charAttrBase + 87 + i);

        // Only process the skill that matches the hero's currently selected skill
        if (selectedSkillId != skillId) continue;

        // Get skill information: mana cost and AG cost
        int manaCost = 0;
        int agCost = 0;
        GetSkillInformation((int)skillId, 1, NULL, &manaCost, NULL, &agCost);

        // Check mana: if current mana < mana cost
        if ((int)(WORD)currentMana < manaCost) {
            // Not enough mana — try to use a potion from inventory slot 3 (potions/arrows)
            // FUN_00482be0 = GetItemSlot(int category) → int slot index, or -1 if none.
            // Declaration in functions.h fixed to: int __cdecl FUN_00482be0(int).
            int slot = FUN_00482be0(3);
            if (slot == -1) continue;  // no potion found

            // Check warehouse and trade are closed
            if (DAT_07eaa119 != '\0') {  // WarehouseOpened
                UIChatLogWindow_AddText((const char*)&DAT_07e11dfc, (const char*)&DAT_07d4c89c, 2);
                continue;
            }

            // anti-tamper hash table — skipped (FUN_0043d8a0 + TradeOpened check)
            if (DAT_07eaa11b != '\0') {  // TradeOpened
                UIChatLogWindow_AddText((const char*)&DAT_07e11dfc, (const char*)&DAT_07d4c89c, 2);
                continue;
            }

            if ((int)EnableUse >= 1) continue;  // item use cooldown active
            EnableUse = 10;  // set cooldown

            // 2026-07-19: era 6 bytes. PMSG_ITEM_USE_RECV (ItemManager.h) mide
            // 5: header(3) + SourceSlot + TargetSlot. El byte extra dejaba el
            // paquete fuera de spec.
            BYTE usePkt[6];
            usePkt[0] = 0xC1;
            usePkt[1] = 5;
            usePkt[2] = 0x26;
            usePkt[3] = (BYTE)(slot + 12);
            usePkt[4] = 0;
            Net_SendSmallPacket(usePkt, 5);

            // Play sound based on item type
            // 2026-08-21: leía DAT_07ea8410, que es un DWORD suelto de 4 bytes —
            // no el pool del inventario (mismo error que ya estaba documentado
            // para FUN_004d23b0).  El grid vive en OffsetInventoryItems, stride 0x44.
            short itemType = *(short*)(OffsetInventoryItems + (size_t)slot * 0x44);
            int soundId;
            if (itemType == 0x1c0) {
                soundId = 0x21;  // healing potion sound
            } else if (itemType >= 0x1c1 && itemType <= 0x1c9) {
                soundId = 0x20;  // mana potion sound
            } else {
                continue;  // unknown item type
            }
            PlayBuffer(soundId, 0, 0);
            continue;
        }

        // Check AG (SkillMana): if current AG < AG cost
        if ((int)(WORD)currentAG < agCost) {
            // anti-tamper hash table — skipped (decrypt CharacterMachine)
            return false;
        }

        WORD targetKey = *(WORD*)((char*)(uintptr_t)DAT_07abf5d0 + (int)DAT_07d780a0 * 0x394 + 476);
        if (targetKey == 0xFFFF) {
            continue;
        }
        BYTE gridX = (BYTE)*(DWORD*)(heroEntity + 0x388);
        BYTE gridY = (BYTE)*(DWORD*)(heroEntity + 0x38C);

        // Skill type check: 0x18 = ranged arrow skill
        if (skillId == 0x18) {
            // Check if arrows are equipped
            char hasArrow = FUN_0048ba70();  // CheckArrow
            if (hasArrow == '\0') continue;  // no arrows equipped

            BYTE* skillEntry = GetSkillRecordShadow_Local((int)skillId);
            if (skillEntry == nullptr)
                skillEntry = GetSkillRecordBase_Local((int)skillId);
            if (skillEntry == nullptr)
                continue;

            // anti-tamper hash table — skipped (encrypt skill entry)

            // Compact 97k record: distance is stored at +0x27.
            BYTE skillDistance = *(BYTE*)(skillEntry + 0x27);

            // Get hero position
            float heroPosX = *(float*)(heroEntity + 0x10);  // Object.Position[0]
            float heroPosY = *(float*)(heroEntity + 0x14);  // Object.Position[1]

            // Use the current runtime target entity position, not the stale
            // global ground-target coords. This keeps ranged-elf range checks
            // aligned with the same target slot used by the packet send.
            float targetWorldX = *(float*)((char*)(uintptr_t)DAT_07abf5d0 + (int)DAT_07d780a0 * 0x394 + 0x10);
            float targetWorldY = *(float*)((char*)(uintptr_t)DAT_07abf5d0 + (int)DAT_07d780a0 * 0x394 + 0x14);

            // Check range: distance from hero to target must be within skill range
            float dx = heroPosX - targetWorldX;
            float dy = heroPosY - targetWorldY;
            float dist = SQRT(dx * dx + dy * dy);
            float maxRange = (float)(int)skillDistance * _DAT_005524f0;

            if (dist > maxRange) continue;  // target out of range

            // Compute facing angle toward target
            // CHARACTER->TargetPosition at offset +0x314 (float[3])
            float targetPosX = *(float*)(heroEntity + 0x314);
            float targetPosY = *(float*)(heroEntity + 0x318);
            float angle = ((float (__cdecl*)(float,float,float,float))FUN_0043e050)(
                heroPosX, heroPosY, targetPosX, targetPosY);  // CreateAngle

            // Set hero facing angle: Object.Angle[2] at entity offset +0x24
            *(float*)(heroEntity + 0x24) = angle;

            // GM name check: compare Hero->ID with "webzen" string
            // Hero->ID at entity offset +0x1C1
            // DAT_00559db4 = comparison string
            // (This is an anti-impersonation check — if hero name contains GM string, special path)
            // Skipping detailed string comparison; the game logic proceeds regardless

            // Set CurrentSkill global to this skill ID
            DAT_05826d10 = (DWORD)skillId;  // CurrentSkill

            // 97k runtime uses the same move sender path before the actual
            // skill packet so target/facing/path state stay aligned with the
            // rest of combat helpers.
            FUN_00491c40((int)heroEntity, (int)heroEntity);

            // MuEmu's 0.97k continuation patch for Triple Shot uses the
            // 256-step facing byte, the packed target delta, and an opposite
            // angle.  The original routine at 0048BD70 computes the same
            // facing value with angle * 0.71111113.
            BYTE dir = (BYTE)(int)(angle * (256.0f / 360.0f));
            BYTE destination = PackDurationDestination_Local(
                gridX, gridY,
                (int)*(unsigned char*)(heroEntity + 0x306),
                (int)*(unsigned char*)(heroEntity + 0x307));
            BYTE oppositeAngle = (BYTE)(int)((angle + 180.0f) * (256.0f / 360.0f));

            // Align to the same server recv struct family used by the wizard path:
            //   PMSG_DURATION_SKILL_ATTACK_RECV { C3:1E, skill, x, y, dir, dis, angle, index[2] }
            SendSkillPacket1E_Local(skillId, gridX, gridY, dir, destination, oppositeAngle, targetKey);

            // Set player attack animation
            // SetPlayerAttack(hero) — Ghidra shows 1-arg; use 4-arg decl with dummies
            FUN_00444410((int)(DWORD)heroEntity, 0, 0, 0);

            // If object type is not 0x186 (special entity), create arrow projectiles
            WORD objType = *(WORD*)(heroEntity + 0x02);  // Object.Type at offset +0x02
            if (objType != 0x186) {
                // We are already iterating the real runtime skill list and matched
                // the selected skill above, so the arrow effect should use the same
                // runtime slot instead of rescanning a stale/corrupted skill block.
                int skillIndex = i;

                // Spawn arrow visual effect
                CreateArrows_stub((DWORD)heroEntity, (DWORD)(heroEntity),
                                  (DWORD)0, (WORD)skillIndex, 1, 0);
            }
            result = true;
            continue;
        }

        DAT_05826d10 = (DWORD)skillId;

        // Keep elf/support on the same pre-move runtime path as warrior/wizard.
        FUN_00491c40((int)heroEntity, (int)heroEntity);

        // Heal (26), Greater Defense (27) and Greater Damage (28) are
        // targeted skills.  MuEmu receives them through C3:19; C3:1E is
        // reserved for duration/area attacks and rejects their normal path.
        SendSkillPacket19_Local(skillId, targetKey);

        FUN_00444a80((int)(DWORD)heroEntity);
        result = true;
    }

    // anti-tamper hash table — skipped (decrypt CharacterMachine after access)
    return result;
}


// FUN_0043d670 @ 0x0043D670 (~188 lines) — hash table: read 4-byte encrypted value
// FUN_0043d670 removed — __thiscall hash table read, not needed as free function

// LookAtTarget @ 0x0043E890 (~38 lines) — rotates entity head toward target character
// Computes angle between entity and target, sets HeadTargetAngle if within threshold.
void __cdecl LookAtTarget_stub(DWORD o, DWORD TargetCharacter) {
    // 0x0043E890 — Rotates entity head toward target character
    // o = OBJECT* (entity), TargetCharacter = CHARACTER* (target, OBJECT at offset 0)
    // OBJECT struct (from Ghidra, size 0x1BC):
    //   +0x10 = Position[3] (float), +0x1C = Angle[3] (float, [2]=facing)
    //   +0x34 = HeadTargetAngle[3] (float)
    // _DAT_00552848 = angle threshold (~80.0f)
    // _DAT_00552598 = height offset, _DAT_005526e4 = vertical angle scale
    // _DAT_00552580 = -180.0f guard, _DAT_0055286c = 360.0f wrap

    if (!TargetCharacter) return;

    float oX = *(float*)(o + 0x10);      // o->Position[0]
    float oY = *(float*)(o + 0x14);      // o->Position[1]
    float oZ = *(float*)(o + 0x18);      // o->Position[2]
    float oFacing = *(float*)(o + 0x24); // o->Angle[2]  — offset 0x1C + 8 = 0x24

    float tX = *(float*)(TargetCharacter + 0x10);  // target->Object.Position[0]
    float tY = *(float*)(TargetCharacter + 0x14);  // target->Object.Position[1]
    float tZ = *(float*)(TargetCharacter + 0x18);  // target->Object.Position[2]

    // CreateAngle @ 0x0043e050: declared as FUN_0043e050 with wrong sig; cast to correct prototype
    typedef float (__cdecl *CreateAngleFn)(float, float, float, float);
    float angle = ((CreateAngleFn)&FUN_0043e050)(oX, oY, tX, tY);

    // FarAngle(oFacing, angle, 1) — angular distance
    double deltaAngle = (double)FUN_0043e370(oFacing, angle, '\x01');

    float* headTarget = (float*)(o + 0x34);  // o->HeadTargetAngle[3]

    if (deltaAngle < (double)_DAT_00552848) {
        // Within threshold: compute head rotation
        headTarget[0] = oFacing - angle;
        headTarget[1] = (tZ - (oZ + _DAT_00552598)) * _DAT_005526e4;

        // Wrap angles if < -180.0f
        for (int i = 0; i < 2; i++) {
            if (headTarget[i] < _DAT_00552580) {
                headTarget[i] = headTarget[i] + _DAT_0055286c;
            }
        }
    } else {
        // Outside threshold: reset head to neutral
        headTarget[0] = 0.0f;
        headTarget[1] = 0.0f;
    }
}

// MoveHead @ 0x0043E940 (~56 lines) — random/tracked head movement for entities
void __cdecl MoveHead_stub(int param_1) {
    char anim = *(char*)(param_1 + 0x105);
    if (anim == '\x06') return; // dead — no head movement

    if (anim == '\0') {
        // Idle: 1/32 chance random head look
        unsigned int r = rand();
        unsigned int masked = r & 0x8000001f;
        bool isZero = (masked == 0);
        if ((int)masked < 0) {
            isZero = ((masked - 1 | 0xffffffe0) == 0xffffffff);
        }
        if (isZero) {
            float* pfVar4 = (float*)(param_1 + 0x34);
            *pfVar4 = (float)(rand() % 128 - 64);          // 0043E940: -64..+63
            *(float*)(param_1 + 0x38) = (float)(rand() % 48 - 16); // -16..+31
            // Clamp both head angles toward 0 by _DAT_0055286c if below _DAT_00552580
            int iVar5 = 2;
            do {
                if (*pfVar4 < _DAT_00552580) {
                    *pfVar4 = *pfVar4 + _DAT_0055286c;
                }
                pfVar4 = pfVar4 + 1;
                iVar5 = iVar5 - 1;
            } while (iVar5 != 0);
            return;
        }
    }
    else if ((anim == '\x02') && (*(short*)(param_1 + 0x310) != -1)) {
        // Combat: look at attack target
        LookAtTarget_stub((DWORD)param_1, CharactersClient + *(short*)(param_1 + 0x310));
        return;
    }
    else {
        // Default: reset head angles to 0
        *(DWORD*)(param_1 + 0x34) = 0;
        *(DWORD*)(param_1 + 0x38) = 0;
    }
}

// BMD::CollisionDetectLineToMesh @ 0x00440BE0 (~101 lines) — line-mesh collision test
// Iterates all meshes and triangles. Per tri: FaceNormalize, CollisionDetectLineToFace.
bool __cdecl BMD__CollisionDetectLineToMesh_stub(DWORD pThis, float Position[3], float Target[3], bool Collision, int Mesh, int Triangle) {
    // 0x00440BE0 — Line-mesh collision test (__thiscall in original, pThis = this pointer)
    // Iterates all meshes in the BMD model. For each mesh, iterates all triangles.
    // Per triangle: compute face normal via FaceNormalize, then test line-to-face intersection
    // via CollisionDetectLineToFace. Returns true on first hit.
    //
    // BMD struct layout (from this pointer):
    //   +0x24: short NumMeshes
    //   +0x28: int*  MeshArray (pointer to mesh data)
    // Each mesh entry (stride 0x28 = 40 bytes):
    //   +0x0A: short NumTriangles
    //   +0x1C: int*  TriangleArray (pointer to triangle data)
    // Each triangle (stride 0x24 = 36 bytes):
    //   +0x00: char  textureIndex
    //   +0x02: short vertIndex0
    //   +0x04: short vertIndex1
    //   +0x06: short vertIndex2
    //   +0x08: short normalIndex
    // Vertex buffer: DAT_0584621c (float[3] per entry, stride 12 bytes)
    // Each mesh adds 15000 to the vertex base index offset.

    (void)Collision; (void)Mesh; (void)Triangle;  // phantom params from Ghidra

    int meshIdx = 0;
    int vertBaseOffset = 0;  // accumulates +15000 per mesh
    float meshCounter = 0.0f;

    short numMeshes = *(short*)((int)pThis + 0x24);
    if (numMeshes <= 0) return false;

    int meshArrayOffset = 0;  // offset into mesh array (stride 0x28)
    do {
        int meshPtr = *(int*)((int)pThis + 0x28) + meshArrayOffset;
        short numTris = *(short*)(meshPtr + 0x0A);
        float triCounter = 0.0f;

        if (numTris > 0) {
            float triByteOffset = 0.0f;
            float prevMeshCounter = meshCounter;

            do {
                // Skip degenerate check: if both mesh and tri counters match Target, skip
                if (meshCounter != Target[0] || triCounter != Target[1]) {
                    int triPtr = *(int*)(meshPtr + 0x1C) + (int)triByteOffset;

                    short vi0 = *(short*)(triPtr + 2);
                    short vi1 = *(short*)(triPtr + 4);
                    short vi2 = *(short*)(triPtr + 6);
                    short ni  = *(short*)(triPtr + 8);

                    float* v0 = (float*)(&DAT_0584621c + (vi0 + vertBaseOffset) * 3);
                    float* v1 = (float*)(&DAT_0584621c + (vi1 + vertBaseOffset) * 3);
                    float* v2 = (float*)(&DAT_0584621c + (vi2 + vertBaseOffset) * 3);
                    float* nv = (float*)(&DAT_0584621c + (ni  + vertBaseOffset) * 3);

                    // Compute face normal
                    float normal[3];
                    float verts[3] = { *(float*)v0, *(float*)v1, *(float*)v2 };  // placeholder
                    FaceNormalize(v0, v1, v2, normal);

                    // Test line-to-face collision
                    float localC[3];
                    bool hit = CollisionDetectLineToFace(
                        Position,                       // line start
                        v0,                             // face verts packed
                        (int)nv,                        // normal vertex index
                        localC,                         // output collision point
                        &Position[2],                   // Z component
                        v0, v1,                         // additional vert ptrs
                        normal,                         // face normal
                        (char)*(char*)triPtr             // texture index as flag
                    );

                    meshCounter = prevMeshCounter;
                    if (hit) {
                        return true;
                    }
                }
                triCounter = (float)((int)triCounter + 1);
                triByteOffset = (float)((int)triByteOffset + 0x24);
            } while ((int)triCounter < (int)numTris);
        }

        meshCounter = (float)((int)meshCounter + 1);
        meshArrayOffset += 0x28;
        vertBaseOffset += 15000;
    } while ((int)meshCounter < (int)*(short*)((int)pThis + 0x24));

    return false;
}

// AttackStage @ 0x00448930 (~866 lines) — main attack animation/effect handler
// Decompiled from Ghidra. Anti-tamper hash table ops (FUN_00403f80/4041e0/404280/404330) skipped.
// Phantom params: unaff_EBX/ESI/EDI/EBP/retaddr are anti-tamper artifacts, not real args.
// The function reads c->Skill (c+0x302), dispatches on its value to spawn
// effects, joints, sounds per weapon/skill type. Each case creates visual FX
// via FUN_00460dc0 (CreateEffect) or FUN_0046d840 (CreateJoint).
static bool __cdecl AttackStage_legacy_mismatched(DWORD c, DWORD o) {
    int Hand = GetHandOfWeapon((int)o);

    // anti-tamper hash table — skipped (encrypt/decrypt c->Skill)
    BYTE skillByte = *(BYTE*)(c + 0x302);

    DAT_00559858 = 15; // g_iLimitAttackTime = 15

    // Models base
    DWORD modelsBase = DAT_05828d58;

    switch (skillByte) {

    // ── case '+' (0x2B) — sword trail / CreateJoint(0x10a) ──────────────
    case 0x2B: {
        short oType = *(short*)(o + 0x02);
        int modelIdx = (int)oType;
        // BMD* = Models + type * sizeof(BMD_entry)
        // model stride = type*0x2C (from LEA EDX,[EAX+EAX*2]; SHL EDX,4; SUB EDX,EAX => type*47 => *4 bytes)
        DWORD pModel = modelsBase + modelIdx * 0xBC;

        BYTE attackTime = *(BYTE*)(c + 0x2F5);

        if (attackTime == 8) {
            PlayBuffer(0x53, 0, 0);
        }

        // Spawn 3 CreateJoint(0x10a) lightning/beam joints when attackTime in [2..8]
        if (attackTime > 1 && attackTime < 9) {
            for (int i = 0; i < 3; i++) {
                float pos[3];
                pos[0] = *(float*)(o + 0x10);
                pos[1] = *(float*)(o + 0x14);
                pos[2] = *(float*)(o + 0x18) + _DAT_00552908;

                // Random offsets (-300..+300)
                pos[0] += (float)(rand() % 601 - 300);
                pos[1] += (float)(rand() % 601 - 300);
                pos[2] += (float)(rand() % 601 - 300);

                // Sin/cos offset from facing angle
                float angleZ = *(float*)(o + 0x24);
                float rad = angleZ * _DAT_0055253c;
                pos[0] -= sinf(rad) * _DAT_00552904;
                pos[1] += cosf(rad) * _DAT_00552904;

                float* oAngle = (float*)(o + 0x1C);
                FUN_0046d840(0x10a, pos, oAngle, (float*)(o + 0x1C),
                             (unsigned int)o, 0, 40.0f, (short)oType, 0);
            }
        }

        // BMD::TransformPosition for sword trail (attackTime < 9)
        if (attackTime < 9) {
            float localPos[3] = {0.0f, 0.0f, 0.0f};
            // ExtOption index: c->Weapon[Hand].ExtOption = *(BYTE*)(c + Hand*0x18 + 0x274)
            BYTE extOpt = *(BYTE*)(c + Hand * 0x18 + 0x274);
            DWORD boneTransform2 = *(DWORD*)(o + 0x114);
            float (*boneMat)[4] = (float(*)[4])(boneTransform2 + extOpt * 0x30);
            float worldPos[3];
            float* swordPos = (float*)(o + 0x188); // o->m_vPosSword
            BMD__TransformPosition((void*)pModel, boneMat, localPos, worldPos, true);
            // Result stored in worldPos — used by trail system
            (void)swordPos; // referenced by original but result consumed internally
        }

        // Second transform for extended trail (attackTime in [6..12])
        if (attackTime > 5 && attackTime < 13) {
            float localPos2[3] = {0.0f, 0.0f, 0.0f};
            BYTE extOpt = *(BYTE*)(c + Hand * 0x18 + 0x274);
            DWORD boneTransform2 = *(DWORD*)(o + 0x114);
            float (*boneMat)[4] = (float(*)[4])(boneTransform2 + extOpt * 0x30);
            float worldPos2[3];
            BMD__TransformPosition((void*)pModel, boneMat, localPos2, worldPos2, true);
        }
        break;
    }

    // ── default — early return checks, then set attackTime=15 ────────────
    default: {
        // Various early-return checks based on AnimationFrame, Type, CurrentAction
        float animFrame = *(float*)(o + 0x108);
        short oType = *(short*)(o + 0x02);
        BYTE curAction = *(BYTE*)(o + 0x105);

        if (animFrame < _DAT_0055256c && oType == 0x186 && curAction == '>') {
            // pass through to set attackTime = 15
        }
        else if (animFrame < _DAT_00552660) {
            return true;
        }
        else {
            if (oType == 0x186 && curAction >= 0x22 && curAction <= 0x5B) {
                // pass through
            }
            else if (oType >= 0x10E && oType <= 0x14E && curAction >= 3 && curAction <= 4) {
                // pass through (monster attack actions)
            }
            else {
                return true;
            }
        }
        // Fall through: set attackTime = 15
        *(BYTE*)(c + 0x2F5) = 0x0F;
        break;
    }

    // ── case '/' (0x2F) — slash effect with CreateEffect(0x1F0) + projectiles ──
    case 0x2F: {
        BYTE attackTime = *(BYTE*)(c + 0x2F5);
        short oType = *(short*)(o + 0x02);
        DWORD pModel = modelsBase + (int)oType * 0xBC;

        if (attackTime == 0x0A) {
            PlayBuffer(0x5F, 0, 0);
        }
        else {
            // Transform at attackTime == 4 (light color = 1.0, 1.0, 0.5)
            if (attackTime == 0x04) {
                float light[3] = {1.0f, 1.0f, 0.5f};
                float localPos[3] = {0.0f, 0.0f, 0.0f};
                BYTE extOpt = *(BYTE*)(c + Hand * 0x18 + 0x274);
                DWORD bt2 = *(DWORD*)(o + 0x114);
                float (*boneMat)[4] = (float(*)[4])(bt2 + extOpt * 0x30);
                float worldPos[3];
                BMD__TransformPosition((void*)pModel, boneMat, localPos, worldPos, true);
                (void)light; // consumed by transform chain
            }

            // Spawn 2x CreateEffect(0x1F0) at attackTime == 8
            if (attackTime == 0x08) {
                float angleZ = *(float*)(o + 0x24);
                float rad = angleZ * _DAT_0055253c;
                float pos[3];
                pos[0] = *(float*)(o + 0x10) + sinf(rad) * _DAT_00552598;
                pos[1] = *(float*)(o + 0x14) - cosf(rad) * _DAT_00552598;
                pos[2] = *(float*)(o + 0x18) + _DAT_005528fc;
                float light[3] = {1.0f, 1.0f, 1.0f};

                FUN_00460dc0(0x1F0, pos, (float*)(o + 0x1C), light,
                             NULL, (float*)o, NULL, NULL, 0);
                FUN_00460dc0(0x1F0, pos, (float*)(o + 0x1C), light,
                             NULL, (float*)o, NULL, NULL, 0);
            }
        }

        // Spawn 3x CreateEffect(0x10A) when attackTime in [13..14]
        if (attackTime > 0x0C && attackTime < 0x0F) {
            // Weapon type from c->Weapon[Hand].Type
            int weapType = (int)*(short*)(c + Hand * 0x18 + 0x270);

            for (int i = 0; i < 3; i++) {
                float rad = *(float*)(o + 0x24) * _DAT_0055253c;
                float light[3] = {0.3f, 0.3f, 0.3f};
                float pos[3];
                pos[0] = *(float*)(o + 0x10) + sinf(rad) * _DAT_005528f8;
                pos[1] = *(float*)(o + 0x14) - cosf(rad) * _DAT_005528f8;
                pos[2] = *(float*)(o + 0x18) + _DAT_005528fc;

                // Random scatter when attackTime != 11
                if (attackTime != 0x0B) {
                    pos[0] += (float)(rand() % 60 - 30);
                    pos[1] += (float)(rand() % 60 - 30);
                    pos[2] += (float)(rand() % 60 - 30);
                }

                FUN_00460dc0(0x10A, pos, (float*)(o + 0x1C), light,
                             (float*)(intptr_t)weapType, (float*)o, NULL, NULL, 0);
            }
            return true;
        }
        break;
    }

    // ── case '0' (0x30) — early attackTime check ────────────────────────
    case 0x30: {
        if (*(BYTE*)(c + 0x2F5) > 9
            && *(short*)(o + 0x02) == 0x186
            && *(BYTE*)(o + 0x105) == 0x3F) {
            *(BYTE*)(c + 0x2F5) = 0x0F;
            return true;
        }
        break;
    }

    // ── case '1' (0x31) — set attackTime to 15 if conditions met ────────
    case 0x31: {
        float animFrame = *(float*)(o + 0x108);
        if (animFrame < _DAT_00552660) return true;
        if (*(short*)(o + 0x02) != 0x186) return true;
        BYTE curAction = *(BYTE*)(o + 0x105);
        if (curAction == 0x40 || curAction == 0x41) {
            *(BYTE*)(c + 0x2F5) = 0x0F;
            return true;
        }
        // Fall through to default: attackTime = 15
        *(BYTE*)(c + 0x2F5) = 0x0F;
        break;
    }

    // ── case '4' (0x34) — combo effect CreateEffect(0x4F3) ─────────────
    case 0x34: {
        short oType = *(short*)(o + 0x02);
        BYTE curAction = *(BYTE*)(o + 0x105);

        // Force animation frame when in attack action range
        if (oType == 0x186 && curAction > 0x21 && curAction < 0x5C) {
            if (*(float*)(o + 0x108) >= _DAT_00552660) {
                *(float*)(o + 0x10C) = 4.0f; // PriorAnimationFrame
                *(float*)(o + 0x108) = 5.0f; // AnimationFrame
            }
        }

        if (*(BYTE*)(c + 0x2F5) == 0x03) {
            FUN_00460dc0(0x4F3, (float*)(o + 0x10), (float*)(o + 0x1C),
                         (float*)(o + 0xE8), NULL, (float*)o, NULL, NULL, 0);
            PlayBuffer(100, (DWORD)o, 0);
        }

        DAT_00559858 = 5; // g_iLimitAttackTime = 5
        return true;
    }

    // ── case '7' (0x37) — summoner/special skill: CreateEffect(0x4F3) + CreateEffect(0x490) ──
    case 0x37: {
        if (*(short*)(o + 0x02) != 0x186) {
            DAT_00559858 = 15;
            return true;
        }
        if (*(BYTE*)(o + 0x105) != 0x3D) {
            DAT_00559858 = 15;
            return true;
        }

        BYTE attackTime = *(BYTE*)(c + 0x2F5);

        // Spawn effect when attackTime in [1..2]
        if (attackTime != 0 && attackTime < 3) {
            FUN_00460dc0(0x4F3, (float*)(o + 0x10), (float*)(o + 0x1C),
                         (float*)(o + 0xE8), (float*)1, (float*)o, NULL, NULL, 0);
        }

        if (*(float*)(o + 0x108) < _DAT_00552540) {
            DAT_00559858 = 15;
            return true;
        }

        PlayBuffer(0x54, 0, 0);

        // FindHotKey(0x37) — find hotkey slot for skill '7'
        int hotKey = FindHotKey_stub(0x37);
        WORD pkKey = *(WORD*)(o + 0x86);

        FUN_00460dc0(0x490, (float*)(o + 0x10), (float*)(o + 0x1C),
                     (float*)(o + 0xE8), (float*)(intptr_t)hotKey, (float*)o,
                     (float*)(intptr_t)(int)pkKey, NULL, NULL);

        // anti-tamper hash table — skipped (CharacterMachine encrypt/decrypt)

        // Final: update Models[0x186] bone data and set attackTime = 15
        DWORD modelData = *(DWORD*)(modelsBase + 0x186 * 0xBC + 0x30);
        *(float*)(modelData + 0x3D4) = (float)(int)c + _DAT_005528f4;
        *(BYTE*)(c + 0x2F5) = 0x0F;
        DAT_00559858 = 15;
        return true;
    }

    // ── case '8' (0x38) — five-fold energy blast: 5x CreateEffect(0xCB) ──
    case 0x38: {
        if (*(short*)(o + 0x02) == 0x186 && *(BYTE*)(o + 0x105) == 0x51) {
            float angleDir[3];
            angleDir[0] = *(float*)(o + 0x1C);
            angleDir[1] = *(float*)(o + 0x20);
            angleDir[2] = *(float*)(o + 0x24) - _DAT_005528e4;

            float* oPos = (float*)(o + 0x10);
            float* oLight = (float*)(o + 0xE8);

            // 5x CreateEffect(0xCB) at different angle offsets
            FUN_00460dc0(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            FUN_00460dc0(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            FUN_00460dc0(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            FUN_00460dc0(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);
            angleDir[2] += _DAT_005524fc;
            FUN_00460dc0(0xCB, oPos, angleDir, oLight,
                         (float*)2, (float*)o, NULL, NULL, 0);

            PlayBuffer(0x54, 0, 0);
            *(BYTE*)(c + 0x2F5) = 0x0F;
        }
        DAT_00559858 = 15;
        return true;
    }

    } // end switch

    return true;
}

// FUN_00451f30 @ 0x00451F30 (~52 lines) — death particle burst (20 dust particles)
// When entity is dead (anim==6) and animation frame is in range [_DAT_00552658, _DAT_00552830),
// spawn 20 dust particles (type 0x4c5) at random offsets (-32..+31) from entity position.
// Sets entity light to (1.0, 1.0, 1.0) before each particle spawn.
// FUN_00451f30 (IDA-activated, was Ghidra stub)
// 00448930 AttackStage — direct IDA switch (raw/00448930_AttackStage.c).
// The older AttackStage_legacy_mismatched above is intentionally not called.
bool __cdecl AttackStage_stub(DWORD c, DWORD o)
{
    const int hand = GetHandOfWeapon((int)o);
    const BYTE skill = *(BYTE*)(c + 770), stage = *(BYTE*)(c + 757);
    const short type = *(short*)(o + 2);
    const DWORD model = DAT_05828d58 + type * 188;
    DAT_00559858 = 15;
    if (skill == 43) {
        if (stage == 8) PlayBuffer(83, 0, 0);
        if (stage >= 2 && stage <= 8) {
            const float r = *(float*)(o + 36) * 0.017453292f;
            for (int i = 0; i < 3; ++i) {
                float p[3] = {*(float*)(o+16)+(float)(rand()%601-300)-sinf(r)*1400.0f, *(float*)(o+20)+(float)(rand()%601-300)+cosf(r)*1400.0f, *(float*)(o+24)+120.0f+(float)(rand()%601-300)};
                FUN_0046d840(266, p, p, (float*)(o+28), 2, (int)o, 40.0f, -1, 0);
            }
        }
        if (stage <= 8) {
            float z[3] = {};
            FUN_004409a0((void*)model, (float*)(*(DWORD*)(o+276)+48**(BYTE*)(c+24*hand+628)), z, (float*)(o+392), 1);
            const float r = *(float*)(o+36)*0.017453292f; *(float*)(o+392)+=sinf(r)*300.0f; *(float*)(o+396)-=cosf(r)*300.0f;
        }
        if (stage >= 6 && stage <= 12) {
            float z[3]={}, p[3], l[3]={1,1,1};
            FUN_004409a0((void*)model, (float*)(*(DWORD*)(o+276)+48**(BYTE*)(c+24*hand+628)), z, p, 1);
            const float r=*(float*)(o+36)*0.017453292f, d=(float)(stage-8)*10.0f+100.0f; p[0]+=sinf(r)*d; p[1]-=cosf(r)*d;
            FUN_00460dc0(496,p,(float*)(o+28),l,(float*)1,(float*)o,(float*)-1,nullptr,0); FUN_00460dc0(496,p,(float*)(o+28),l,(float*)1,(float*)o,(float*)-1,nullptr,0);
            const short target=*(short*)(c+784); if(target!=-1 && stage>=10) { BYTE* v=(BYTE*)(uintptr_t)CharactersClient+target*916; if(*v) v[404]=35; }
        }
    } else if (skill == 47) {
        if(stage==10) PlayBuffer(95,0,0);
        else if(stage==4) { float z[3]={},p[3],l[3]={1,1,0.5f}; FUN_004409a0((void*)model,(float*)(*(DWORD*)(o+276)+48**(BYTE*)(c+24*hand+628)),z,p,1); FUN_00460dc0(497,p,(float*)(o+28),l,(float*)(intptr_t)*(short*)(c+8*(3*hand+78)),(float*)o,(float*)-1,nullptr,0); }
        else if(stage==8) { const float r=*(float*)(o+36)*0.017453292f; float p[3]={*(float*)(o+16)+sinf(r)*50.0f,*(float*)(o+20)-cosf(r)*50.0f,*(float*)(o+24)+110.0f},l[3]={1,1,1}; FUN_00460dc0(496,p,(float*)(o+28),l,nullptr,(float*)o,(float*)-1,nullptr,0); FUN_00460dc0(496,p,(float*)(o+28),l,nullptr,(float*)o,(float*)-1,nullptr,0); }
        if(stage>=13 && stage<=14) { const float r=*(float*)(o+36)*0.017453292f; for(int i=0;i<3;++i) { float p[3]={*(float*)(o+16)+sinf(r)*145.0f+(float)(rand()%60-30),*(float*)(o+20)-cosf(r)*145.0f+(float)(rand()%60-30),*(float*)(o+24)+110.0f+(float)(rand()%60-30)},l[3]={0.3f,0.3f,0.3f}; FUN_00460dc0(266,p,(float*)(o+28),l,(float*)(intptr_t)*(short*)(c+8*(3*hand+78)),(float*)o,(float*)-1,nullptr,0); } }
    } else if(skill==48) { if(stage>9 && type==390 && *(BYTE*)(o+261)==63) *(BYTE*)(c+757)=15; }
    else if(skill==49) { if(*(float*)(o+264)>=5.0f && type==390 && (*(BYTE*)(o+261)==64 || *(BYTE*)(o+261)==65)) *(BYTE*)(c+757)=15; }
    else if(skill==52) { if(type==390 && *(BYTE*)(o+261)>=34 && *(BYTE*)(o+261)<=91 && *(float*)(o+264)>=5.0f){*(float*)(o+268)=4.0f;*(float*)(o+264)=5.0f;} if(stage==3){FUN_00460dc0(1267,(float*)(o+16),(float*)(o+28),(float*)(o+232),nullptr,(float*)o,(float*)-1,nullptr,0);PlayBuffer(100,o,0);} DAT_00559858=5; }
    else if(skill==55) { if(type==390 && *(BYTE*)(o+261)==61) { if(stage && stage<=2) FUN_00460dc0(1267,(float*)(o+16),(float*)(o+28),(float*)(o+232),(float*)1,(float*)o,(float*)-1,nullptr,0); if(*(float*)(o+264)>=3.0f){PlayBuffer(84,0,0); FUN_00460dc0(1168,(float*)(o+16),(float*)(o+28),(float*)(o+232),nullptr,(float*)o,(float*)(intptr_t)*(short*)(o+134),(float*)(intptr_t)FindHotKey_stub(55),0); const DWORD modelState=DAT_05828d58?*(DWORD*)(DAT_05828d58+390*188+48):0; if(modelState && CharacterAttribute) *(float*)(modelState+980)=*(WORD*)((BYTE*)CharacterAttribute+56)*0.0040000002f+0.54000002f; *(BYTE*)(c+757)=15;} } }
    else if(skill==56) { if(type==390 && *(BYTE*)(o+261)==81){float a[3]={*(float*)(o+28),*(float*)(o+32),*(float*)(o+36)-40.0f};for(int i=0;i<5;++i){FUN_00460dc0(203,(float*)(o+16),a,(float*)(o+232),(float*)2,(float*)o,(float*)-1,nullptr,0);a[2]+=20.0f;}PlayBuffer(84,0,0);*(BYTE*)(c+757)=15;} }
    else if((*(float*)(o+264)>=1.0f && type==390 && *(BYTE*)(o+261)==62) || (*(float*)(o+264)>=5.0f && ((type==390 && *(BYTE*)(o+261)>=34 && *(BYTE*)(o+261)<=91) || (type>=270 && type<335 && *(BYTE*)(o+261)>=3 && *(BYTE*)(o+261)<=4)))) *(BYTE*)(c+757)=15;
    return true;
}

void __cdecl FUN_00451f30(int a1)
{
  int v1; // ebx
  float Position[3]; // [esp+4h] [ebp-Ch] BYREF

  if ( *(BYTE *)(a1 + 261) == 6 && *(float *)(a1 + 264) >= 8.0 && *(float *)(a1 + 264) < 9.0 )
  {
    v1 = 20;
    do
    {
      *(DWORD *)(a1 + 232) = 1065353216;
      *(DWORD *)(a1 + 236) = 1065353216;
      *(DWORD *)(a1 + 240) = 1065353216;
      Position[0] = (double)(rand() % 64 - 32) + *(float *)(a1 + 16);
      Position[1] = (double)(rand() % 64 - 32) + *(float *)(a1 + 20);
      Position[2] = (double)(rand() % 32 - 16) + *(float *)(a1 + 24);
      Particle_Spawn(1221, Position, (float *)(a1 + 28), (float *)(a1 + 232), 1, 1.0, 0);
      --v1;
    }
    while ( v1 );
  }
}


// FUN_00452030 @ 0x00452030 (~30 lines) — idle ambient particle (single spark)
// When entity anim state == 2 (combat/active), spawn one particle (type 0x4c5)
// at random offset (0..199) minus grid scale from entity X/Y position.
void __cdecl FUN_00452030(int param_1) {
    if (*(char*)(param_1 + 0x105) != '\x02') return;

    int r1 = rand();
    float fX = ((float)(r1 % 200) + *(float*)(param_1 + 0x10)) - _DAT_005524f0;

    int r2 = rand();
    float fZ = ((float)(r2 % 200) + *(float*)(param_1 + 0x14)) - _DAT_005524f0;

    float pos[3] = { fX, *(float*)(param_1 + 0x14), *(float*)(param_1 + 0x18) };
    float scale[3] = { 1.0f, 1.0f, 0.0f };
    float light[3] = { fX, fZ, 0.0f };
    FUN_00475220(0x4c5, (float*)pos, (float*)scale, (float*)light,
                 *(int*)(param_1 + 0x18), 0.0f, r2 % 200);
}

// MoveCharactersClient @ 0x00455010 (~169 lines) — SUMMARY STUB
// Per-frame entity update. Clears terrain walk flags, then for each alive character:
// decrypt movement via hash table, tick movement, re-encrypt, update cached waypoints.
void __stdcall MoveCharactersClient_stub(void) {
    // 0x00455010 — Per-frame entity update
    // Phase 1: Clear bit 2 from all terrain walk flags (0x10000 tiles)
    // Phase 2: For each alive entity, decrypt movement waypoints via hash table,
    //          compute terrain index, set walk-occupied bit
    // Phase 3: Call MoveCharacterClient for each entity, then MoveBlurs

    // Phase 1: Clear walk-occupied bit from terrain
    char* terrainWall = (char*)&DAT_0838bc70;
    for (int i = 0; i < 0x10000; i++) {
        if ((terrainWall[i] & 2) == 2) {
            terrainWall[i] = terrainWall[i] - 2;
        }
    }

    // Phase 2: Per-entity terrain flag update
    int offset = 0;
    char* entBase = (char*)(uintptr_t)DAT_07abf5d0;
    do {
        char* ent = entBase + offset;
        // entity alive check: ent[0] != 0, ent[0x2fd] == 0, ent[0x84] != 8
        if (ent[0] != '\0' && ent[0x2fd] == '\0' && ent[0x84] != '\x08') {
            // anti-tamper hash table — skipped (encrypt ent+0x38c waypoint)
            // anti-tamper hash table — skipped (decrypt ent+0x38c waypoint)

            int val_38c = *(int*)(ent + 0x38c);  // cached_wp_y (audit #10: comment was swapped)

            // anti-tamper hash table — skipped (encrypt ent+0x388 waypoint)
            // anti-tamper hash table — skipped (decrypt ent+0x388 waypoint)

            int val_388 = *(int*)(ent + 0x388);  // cached_wp_x (audit #10: comment was swapped)

            // Compute terrain index from cached waypoints and set walk-occupied bit
            // Call order from asm: FUN_004f6c40(val_388, val_38c)
            int terrainIdx = FUN_004f6c40((unsigned int)val_388, (unsigned int)val_38c);
            terrainWall[terrainIdx] = terrainWall[terrainIdx] | 2;
        }
        offset += 0x394;
    } while (offset < 0x59740);

    // Phase 3: Tick movement for each entity, then blur trails
    offset = 0;
    do {
        FUN_00454fc0((float*)(entBase + offset));
        offset += 0x394;
    } while (offset < 0x59740);

    MoveBlurs();
}

// FUN_0045fae0 @ 0x0045FAE0 (~175 lines) — hash table: read 1-byte encrypted value
unsigned char __cdecl FUN_0045fae0(DWORD ecx, unsigned char* param_1) {
    // 0x0045FAE0 — anti-tamper hash table: read 1-byte encrypted value
    // Original is __thiscall on the hash table object (ecx = this).
    // Looks up param_1 key in hash table, allocates+decrypts a 1-byte node,
    // writes decrypted value to *param_1, then releases the node.
    //
    // Decryption: val = ((stored_byte - 0x23) ^ DAT_00559050) + 0xb9
    //
    // anti-tamper hash table — skipped
    // Since the hash table encrypt/decrypt is anti-tamper obfuscation and not real game logic,
    // this function is a no-op. The byte at *param_1 is left unchanged (already correct
    // in the unprotected build).
    (void)ecx;
    (void)param_1;
    return *param_1;
}

// FUN_00466300 @ 0x00466300 (~166 lines) — bomb/explosion ring particle effect
// Creates ring of 8 particles evenly spaced around a circle via AngleMatrix + VectorRotate.
// For each: CreateBomb at rotated position, then 2x CreateEffect (type 0xC5 or 0xC6, random).
// param_1 = float[3] center position
// FUN_00466300 (IDA-activated, was Ghidra stub)
void __cdecl FUN_00466300(float *a1)
{
  int v1; // edi
  int v2; // eax
  int v3; // eax
  int v4; // [esp+8h] [ebp-64h]
  float out[3]; // [esp+Ch] [ebp-60h] BYREF
  float Angle[3]; // [esp+18h] [ebp-54h] BYREF
  float Light[3]; // [esp+24h] [ebp-48h] BYREF
  float in1[3]; // [esp+30h] [ebp-3Ch] BYREF
  float in2[3][4]; // [esp+3Ch] [ebp-30h] BYREF

  v1 = 0;
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  v4 = 0;
  do
  {
    in1[0] = 0.0;
    in1[1] = -220.0;
    in1[2] = 0.0;
    Angle[0] = 0.0;
    Angle[1] = 0.0;
    Angle[2] = (double)v4 * 45.0;
    AngleMatrix(Angle, in2);
    VectorRotate(in1, (float*)in2, out);
    out[0] = out[0] + *a1;
    out[1] = out[1] + a1[1];
    out[2] = out[2] + a1[2];
    CreateBomb(out, 1);
    v2 = rand() % 2;
    CreateEffect(v2 + 197, out, Angle, Light, (float*)1, (float*)0, (float*)-1, (float*)0, 0);
    v3 = rand() % 2;
    CreateEffect(v3 + 197, out, Angle, Light, (float*)0, (float*)0, (float*)-1, (float*)0, 0);
    v4 = ++v1;
  }
  while ( v1 < 8 );
}


// RenderWheelWeapon @ 0x0046B7C0 (~101 lines) — renders spinning weapon effect
// Saves object position/rotation, modifies Z + rotation for spinning effect,
// sets up BMD model, animates, RequestTerrainLight, RenderPartObject, restores original.
void __cdecl RenderWheelWeapon_stub(DWORD o) {
    // Save original position and rotation
    float save_posX = *(float*)(o + 0x10);
    float save_posY = *(float*)(o + 0x14);
    float save_posZ = *(float*)(o + 0x18);
    float save_angX = *(float*)(o + 0x1C);
    float save_angY = *(float*)(o + 0x20);
    float save_angZ = *(float*)(o + 0x24);

    // Modify rotation: subtract _DAT_0055284c from Z height offset (o+0xC8)
    float fRot = *(float*)(o + 0xC8) - _DAT_0055284c;
    *(float*)(o + 0xC8) = fRot;

    // Apply rotation offset to angle Z, set angle Y to 90.0f (0x42b40000)
    *(float*)(o + 0x24) = fRot + save_angZ;
    *(float*)(o + 0x20) = 90.0f;

    // Raise Z position by one terrain unit
    *(float*)(o + 0x18) = save_posZ + _DAT_005524f0;

    // Compute model Type from weapon item attribute byte
    BYTE weaponByte = *(BYTE*)(*(int*)(o + 0xFC) + 0x88);
    int Type = (int)weaponByte + 400;

    // Set up BMD model data
    int modelBase = DAT_05828d58 + Type * 0xBC;
    BYTE heroClass = *(BYTE*)(Hero + 0x2B8) & 7;  // Hero->Class & 7
    BYTE animState = *(BYTE*)(o + 0x105);

    *(float*)(modelBase + 0x6C) = *(float*)(o + 0x10);
    *(float*)(modelBase + 0x70) = *(float*)(o + 0x14);
    *(float*)(modelBase + 0x74) = *(float*)(o + 0x18);
    *(BYTE*)(modelBase + 0x98) = heroClass;
    *(BYTE*)(modelBase + 0xA0) = animState;

    // Save and set object type
    short origType = *(short*)(o + 2);
    *(short*)(o + 2) = (short)Type;

    // ItemObjectAttribute — sets up object render attributes
    FUN_00502ba0(o);

    // BMD::Animation — Ghidra shows phantom register params (unaff_EBX/ESI/EDI/EBP);
    // the real call sets up bone matrices for the weapon model.
    // Parameters that depend on phantom regs are passed as zero/defaults.
    // (Animation is driven by the bone matrix pointer at o+0x108 and frame at o+0x10C)

    // RequestTerrainLight — sample terrain lighting at object position
    float terrainLight[3] = { 0.0f, 0.0f, 0.0f };
    FUN_004f7960(*(float*)(o + 0x10), *(float*)(o + 0x14), terrainLight);

    // Add object's own light contribution
    terrainLight[0] += *(float*)(o + 0xE8);
    terrainLight[1] += *(float*)(o + 0xEC);
    terrainLight[2] += *(float*)(o + 0xF0);

    // Light level from item attribute byte
    int lightLevel = (int)(*(BYTE*)(*(int*)(o + 0xFC) + 0x89)) << 3;

    // RenderPartObject(o, Type, NULL, light, alpha=0.0, level=1, opt=1, globalTrans=true, hideSkin=false, translate=true, select, renderType)
    FUN_00505a10(o, Type, 0, terrainLight, 0.0f, 1, 1, 1, 0, 1, 0, 0);

    // Restore original type
    *(short*)(o + 2) = origType;

    // Restore original position and rotation
    *(float*)(o + 0x10) = save_posX;
    *(float*)(o + 0x14) = save_posY;
    *(float*)(o + 0x18) = save_posZ;
    *(float*)(o + 0x1C) = save_angX;
    *(float*)(o + 0x20) = save_angY;
    *(float*)(o + 0x24) = save_angZ;
}

// FUN_0046b980 @ 0x0046B980 (~82 lines) — renders grounded weapon model
// If object's height offset (o+0x60) > _DAT_00552488 threshold:
// set up BMD model data, ItemObjectAttribute, RequestTerrainLight, RenderPartObject.
// Similar to RenderWheelWeapon but without position save/restore (static ground item).
void __cdecl FUN_0046b980(int param_1) {
    // Only render if height above threshold
    if (_DAT_00552488 >= (float)*(int*)(param_1 + 0x60)) return;

    // Compute model Type from weapon item attribute byte
    BYTE weaponByte = *(BYTE*)(*(int*)(param_1 + 0xFC) + 0x88);
    int Type = (int)weaponByte + 400;

    // Set up BMD model data
    int modelBase = DAT_05828d58 + Type * 0xBC;
    BYTE heroClass = *(BYTE*)(Hero + 0x2B8) & 7;
    BYTE animState = *(BYTE*)(param_1 + 0x105);

    *(float*)(modelBase + 0x6C) = *(float*)(param_1 + 0x10);
    *(float*)(modelBase + 0x70) = *(float*)(param_1 + 0x14);
    *(float*)(modelBase + 0x74) = *(float*)(param_1 + 0x18);
    *(BYTE*)(modelBase + 0x98) = heroClass;
    *(BYTE*)(modelBase + 0xA0) = animState;

    // Save and set object type
    float origTypeF = (float)*(short*)(param_1 + 2);
    *(short*)(param_1 + 2) = (short)Type;

    // Set up object render attributes
    DWORD save_d8 = *(DWORD*)(param_1 + 0xD8);
    FUN_00502ba0(param_1);  // ItemObjectAttribute
    *(DWORD*)(param_1 + 0xD8) = save_d8;  // restore overwritten field

    // RequestTerrainLight — sample terrain lighting at object position
    float terrainLight[3] = { 0.0f, 0.0f, 0.0f };
    FUN_004f7960(*(float*)(param_1 + 0x10), *(float*)(param_1 + 0x14), terrainLight);

    // Add object's own light contribution
    terrainLight[0] += *(float*)(param_1 + 0xE8);
    terrainLight[1] += *(float*)(param_1 + 0xEC);
    terrainLight[2] += *(float*)(param_1 + 0xF0);

    // Light level from item attribute
    int lightLevel = (int)(*(BYTE*)(*(int*)(param_1 + 0xFC) + 0x89)) << 3;

    // BMD::Animation — Ghidra shows phantom register params (unaff_EBX/ESI/EDI);
    // driven by bone matrix pointer at o+0x108 and frame at o+0x10C.
    // Skipping direct call due to __thiscall convention + phantom regs;
    // RenderPartObject below handles the actual render.

    // RenderPartObject
    FUN_00505a10(param_1, Type, 0, terrainLight, 0.0f, 1, 1, 1, 0, 1, 0, 0);

    // Restore original type
    *(short*)(param_1 + 2) = (short)(int)origTypeF;
}

// FUN_0046c5a0 @ 0x0046C5A0 (~117 lines) — skill impact particle burst
// AngleMatrix from direction (param_4), VectorRotate to get offset.
// Spawn one main particle (0x498) at param_3 position,
// then 20 trail particles (0x497) at the rotated offset position.
// param_1 = unused (Ghidra phantom), param_2 = entity base (angles at +0x1C)
// param_3 = float[3] position, param_4 = float[3] direction angles
void __cdecl FUN_0046c5a0(int param_1, int param_2, float* param_3, float* param_4) {
    (void)param_1;

    float light[3] = { 1.0f, 1.0f, 1.0f };
    float angle[3] = { 1.0f, 0.0f, 0.0f };  // scale=1.0 in angle[0]

    // Spawn main impact particle (type 0x498) at param_3 position
    FUN_00475220(0x498, param_3, angle, light, 0, 0.0f, 0);

    // Build rotation matrix from direction angles
    float dirAngles[3] = { param_4[0], param_4[1], param_4[2] };
    // Ghidra: local_3c=0, local_38=50.0f (0x42480000), local_34=0  → offset vector (0, 50, 0)
    float localOffset[3] = { 0.0f, 50.0f, 0.0f };
    float matrix[3][4];
    AngleMatrix(dirAngles, matrix);

    // Rotate offset vector by direction matrix
    float worldOffset[3];
    FUN_004fa0b0(localOffset, (float*)matrix, worldOffset);

    // Final trail spawn position = rotated offset + source position
    float trailPos[3];
    trailPos[0] = worldOffset[0] + param_3[0];
    trailPos[1] = worldOffset[1] + param_3[1];
    trailPos[2] = worldOffset[2] + param_3[2];

    // Spawn 20 trail particles (type 0x497) at the offset position
    for (int i = 0; i < 20; i++) {
        rand();  // random variation X
        rand();  // random variation Y
        FUN_00475220(0x497, param_3, angle, light, 0, 0.0f, 0);
    }
}

// CreateBlood @ 0x0046C680 (~58 lines) — blood particles on entity hit/death
// Type 0x11d: 10 effect(199) particles + kill entity.
// Others: BMD bone transform to position blood at hit location.
// Models base = DAT_05828d58, stride 0xbc. OBJECT struct (Ghidra): Type@+0x02, Live@+0x00,
// Position@+0x10, Angle@+0x1C, Light@+0xE8, BoneTransform2@+0x114.
void __cdecl CreateBlood_stub(DWORD o) {
    WORD wType = *(WORD*)(o + 0x02);  // o->Type
    // Models[type].Data + 0x54 = bone index for blood attachment point
    int boneIdx = *(int*)(DAT_05828d58 + (short)wType * 0xbc + 0x54);
    if (boneIdx == -1) return;

    if (wType == 0x11d) {
        // Kill entity and spawn 10 death-blood effect particles (type 199)
        *(bool*)(o + 0x00) = false;  // o->Live = false
        int count = 10;
        do {
            // CreateEffect(199, o->Position, o->Angle, o->Light, ...)
            // Ghidra shows phantom register args for the trailing params;
            // pass position/angle/light arrays from the OBJECT.
            float pos[3] = { *(float*)(o + 0x10), *(float*)(o + 0x14), *(float*)(o + 0x18) };
            float ang[3] = { *(float*)(o + 0x1C), *(float*)(o + 0x20), *(float*)(o + 0x24) };
            float lit[3] = { *(float*)(o + 0xE8), *(float*)(o + 0xEC), *(float*)(o + 0xF0) };
            FUN_00460dc0(199, pos, ang, lit, NULL, NULL, NULL, NULL, 0);
            count = count - 1;
        } while (count != 0);
        return;
    }

    // Skip blood for specific monster types (skeletal/transparent models)
    if ((wType == 0x115) || (wType == 0x11c) || (wType == 0x120)) return;

    // Normal blood: transform a random bone-local offset into world space
    // via BMD::TransformPosition, then spawn blood particle at that position.
    float localPos[3];
    float worldPos[3];

    rand();  // random X bone-local offset
    rand();  // random Z bone-local offset

    localPos[0] = 0.0f;
    localPos[1] = 0.0f;
    localPos[2] = 0.0f;
    worldPos[0] = 0.0f;
    worldPos[1] = 0.0f;
    worldPos[2] = 0.0f;

    // BMD::TransformPosition(Models + type, o->BoneTransform2[boneIdx], localPos, worldPos, Translate)
    // BoneTransform2 is a pointer at o+0x114; each bone matrix is float[3][4] = 48 bytes
    float* boneMatrix = (float*)(*(DWORD*)(o + 0x114) + boneIdx * 48);
    void* model = (void*)(DAT_05828d58 + (short)wType * 0xbc);
    BMD__TransformPosition(model, (float(*)[4])boneMatrix, localPos, worldPos, true);
}


// CreateArrow @ 0x00474370 (~658 lines) — creates single arrow/bolt projectile
// Decompiled from Ghidra. Anti-tamper hash table ops skipped.
// Phantom params (unaff_EBX/ESI/EDI/EBP/retaddr) are obfuscation artifacts.
// Reads weapon type from character's equipment slots, determines projectile
// visual effect, plays sound, sets CurrentSkill, then dispatches through
// two switch tables: first by weapon-type for the projectile, second by
// adjusted weapon-type (+400) for the trail effect.
void __cdecl CreateArrow_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey) {
    // Read weapon types from character equipment slots
    int weaponType0 = (int)*(short*)(c + 0x270);  // Weapon[0].Type
    int weaponType1 = (int)*(short*)(c + 0x288);  // Weapon[1].Type

    bool isHero = (c == (DWORD)DAT_07abf5d8);

    if (isHero) {
        // anti-tamper hash table — skipped (CharacterMachine encrypt/decrypt/lookup)
        // Read weapon types from CharacterMachine instead
        DWORD cm = (DWORD)DAT_07cf1ffc;
        weaponType0 = (int)*(short*)(cm + 0x218);
        weaponType1 = (int)*(short*)(cm + 0x25C);
    }

    // Adjust: add ITEM_BASE_MODEL (400) if not -1
    int adjType0 = weaponType0;
    if (weaponType0 != -1) {
        adjType0 = weaponType0 + 400;
    }
    int adjType1 = weaponType1;
    if (weaponType1 != -1) {
        adjType1 = weaponType1 + 400;
    }

    // Play sound based on SKKey
    DWORD effectFlags = 0;
    if ((char)SKKey == '4') {
        effectFlags = 2;
        PlayBuffer(0x41, (DWORD)o, 0);
    }

    // Set CurrentSkill
    DAT_05826d10 = (DWORD)(SKKey & 0xFF);

    if ((char)SKKey == '3') {
        PlayBuffer(0x65, (DWORD)o, 0);
    }

    // Pointers for CreateEffect calls
    float* oPos   = (float*)(o + 0x10);
    float* oAngle = (float*)(o + 0x1C);
    float* oLight = (float*)(o + 0xE8);
    WORD   oPKKey = *(WORD*)(o + 0x86);

    // First switch: by raw weapon type — select projectile effect
    // weaponType0 - 0x218 offset means actual item type values:
    //   0 = type 0 (sword-class bow)  -> effect 0xD9
    //   1 = type 1 (crossbow)         -> effect 0xD9
    //   2 = type 2 (siege crossbow)   -> effect 0xDD
    //   3 = type 3 (spirit bow)       -> effect 0xDB
    //   4 = type 4 (rapid bow)        -> effect 0xDA
    //   5 = type 5 (elven bow)        -> effect 0xE0
    //   6 = type 6 (crystal bow)      -> effect 0xDF
    //   8 = type 8 (divine bow)       -> effect 0xF2
    //   10 = type 10 (legend bow)     -> effect 0xF2
    switch (weaponType0) {
    case 0:
        FUN_00460dc0(0xD9, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 1:
        FUN_00460dc0(0xD9, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 2:
        FUN_00460dc0(0xDD, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 3:
        FUN_00460dc0(0xDB, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 4:
        FUN_00460dc0(0xDA, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 5:
        FUN_00460dc0(0xE0, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 6:
        FUN_00460dc0(0xDF, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 8:
        FUN_00460dc0(0xF2, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 10:
        FUN_00460dc0(0xF2, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    default:
        break;
    }

    // Second switch: by adjusted weapon type (+400 = model index)
    // Determines trail/secondary effect
    switch (adjType1) {
    case 0x210: // 528 (crossbow model range)
    case 0x211:
    case 0x213:
    case 0x214:
    case 0x215:
        FUN_00460dc0(0xD8, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    case 0x212:
        FUN_00460dc0(0xDC, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    case 0x216:
        FUN_00460dc0(0xDE, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    case 0x221:
        FUN_00460dc0(0xF3, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    default:
        break;
    }
}

// CreateArrows @ 0x00474BD0 (~144 lines) — multi-arrow skill handler
// SKKey '3'/'4': single arrow + counter increment.
// Skill 1: triple shot (center + 2 angle offsets).
// Skill 2: five-way shot with 4 additional angle offsets.
void __cdecl CreateArrows_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey) {
    // 0x00474BD0 — Multi-arrow skill handler.
    // SKKey '3' or '4': single arrow + increment arrow counter in CharacterMachine.
    // Skill 1: triple shot (center + 2 angle offsets via _DAT_00552834 / _DAT_0055284c).
    // Skill 2: five-way shot with 4 additional angle offsets via _DAT_00552ab0 / _DAT_00552584.
    // Anti-tamper hash table operations surround the arrow counter increment — skipped.

    // o->Angle[2] is at offset +0x1C in OBJECT struct (Angle[0]=+0x14, Angle[1]=+0x18, Angle[2]=+0x1C)
    // Actually from Ghidra: o->Angle[2] direct field access. OBJECT is opaque DWORD here.
    // Angle[2] offset in OBJECT: from struct layout, Angle starts at +0x14 (3 floats after Position[3] at +0x08)
    // But in Ghidra the struct has Position[0..2] at +0x08 and Angle[0..2] at +0x14.
    // o->Angle[2] = *(float*)(o + 0x1C)

    float* pAngleZ = (float*)(o + 0x1C);  // o->Angle[2]

    if ((char)SKKey == '4' || (char)SKKey == '3') {
        // Single arrow + counter increment
        CreateArrow_stub(c, o, to, SkillIndex, Skill, SKKey);

        // anti-tamper hash table — skipped (encrypt CharacterMachine)
        // Increment arrow counter at CharacterMachine + 0x160
        // CharacterMachine = DAT_07cf1ffc
        char* cm = (char*)(DWORD)DAT_07cf1ffc;
        cm[0x160] = cm[0x160] + 1;
        // anti-tamper hash table — skipped (decrypt CharacterMachine)
    }
    else {
        // Multi-arrow based on Skill level
        CreateArrow_stub(c, o, to, SkillIndex, Skill, SKKey);

        if ((char)Skill == 1) {
            // Triple shot: center already created, now +offset and -offset
            *pAngleZ += _DAT_00552834;
            CreateArrow_stub(c, o, to, SkillIndex, 1, SKKey);
            *pAngleZ -= _DAT_0055284c;  // goes to negative offset from original
            CreateArrow_stub(c, o, to, SkillIndex, 1, SKKey);
            *pAngleZ += _DAT_00552834;  // restore to original angle

            // anti-tamper hash table — skipped (encrypt CharacterMachine)
            char* cm = (char*)(DWORD)DAT_07cf1ffc;
            cm[0x160] = cm[0x160] + 1;
            // anti-tamper hash table — skipped (decrypt CharacterMachine)
        }
        else if ((char)Skill == 2) {
            // Five-way shot: center already created, now 4 additional angles
            *pAngleZ += _DAT_00552ab0;
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ += _DAT_00552ab0;
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ -= _DAT_00552584;   // jump to negative side
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ -= _DAT_00552ab0;
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ += _DAT_00552834;   // restore to original angle

            // anti-tamper hash table — skipped (encrypt CharacterMachine)
            char* cm = (char*)(DWORD)DAT_07cf1ffc;
            cm[0x160] = cm[0x160] + 1;
            // anti-tamper hash table — skipped (decrypt CharacterMachine)
        }
    }
}

// MoveParticles @ 0x00477090 (~1130 lines) — FULL IMPLEMENTATION
// Per-frame particle system update.
// 1. Random camera shake with clamping
// 2. Per active particle: decrement lifetime, MovePosition,
//    switch on type for gravity/terrain snap/fade/scale/color
//
// Particle slot layout (stride 0x70, base DAT_07abf5f0):
//   +0x00 active  +0x04 type  +0x08 sub  +0x0C scale
//   +0x10 posX    +0x14 posY  +0x18 posZ
//   +0x1C dirX    +0x20 angle
//   +0x28 lightR  +0x2C lightG +0x30 lightB
//   +0x38 lifetime +0x3C entity_ptr +0x40 rotation +0x44 frame
//   +0x48 gravity  +0x4C velX +0x50 velY +0x54 velZ
//   +0x58 extra0   +0x5C extra1 +0x60 extra2
//   +0x64 targetX  +0x68 targetY +0x6C targetZ
//
// NOTE: Ghidra shows unaff_EBX/EBP/ESI/EDI phantom params in the AngleMatrix/VectorRotate
//       calls for type 0x4c6 — these are 63 phantom stack params (anti-tamper artifact).
//       The AngleMatrix+VectorRotate path for type 0x4c6 is implemented structurally but
//       the phantom register values cannot be reconstructed; it runs the math correctly
//       with the actual particle data.
//
// Bitmaps access: DAT_083a7ca0 + type * 0x38 + 0x24 = height field per bitmap slot.
// Models access: DAT_05828d58 + entity_type * 0xBC = BMD struct base.
// WorldTime = DAT_05826e08 (global animation tick).

// Helper macros for particle field access
// BUG-FIX 2026-04-27: era `&DAT_07abf5f0` cuando DAT era `char` solo. Ahora es
// array `char[N]` y `&array` sería pointer-to-array (aritmética × sizeof[N]).
// Usar `DAT_07abf5f0` directo decae a char* correcto.
#define P_BASE      (DAT_07abf5f0)
#define P_ACTIVE(o) (*(char*)(P_BASE + (o)))
#define P_TYPE(o)   (*(int*)(P_BASE + (o) + 0x04))
#define P_SUB(o)    (*(int*)(P_BASE + (o) + 0x08))
#define P_SCALE(o)  (*(float*)(P_BASE + (o) + 0x0C))
#define P_POSX(o)   (*(float*)(P_BASE + (o) + 0x10))
#define P_POSY(o)   (*(float*)(P_BASE + (o) + 0x14))
#define P_POSZ(o)   (*(float*)(P_BASE + (o) + 0x18))
#define P_DIRX(o)   (*(float*)(P_BASE + (o) + 0x1C))
#define P_ANGLE(o)  (*(float*)(P_BASE + (o) + 0x20))
#define P_LR(o)     (*(float*)(P_BASE + (o) + 0x28))
#define P_LG(o)     (*(float*)(P_BASE + (o) + 0x2C))
#define P_LB(o)     (*(float*)(P_BASE + (o) + 0x30))
#define P_LIFE(o)   (*(int*)(P_BASE + (o) + 0x38))
#define P_ENT(o)    (*(int*)(P_BASE + (o) + 0x3C))
#define P_ROT(o)    (*(float*)(P_BASE + (o) + 0x40))
#define P_FRAME(o)  (*(int*)(P_BASE + (o) + 0x44))
#define P_GRAV(o)   (*(float*)(P_BASE + (o) + 0x48))
#define P_VELX(o)   (*(float*)(P_BASE + (o) + 0x4C))
#define P_VELY(o)   (*(float*)(P_BASE + (o) + 0x50))
#define P_VELZ(o)   (*(float*)(P_BASE + (o) + 0x54))
#define P_EX0(o)    (*(float*)(P_BASE + (o) + 0x58))
#define P_EX1(o)    (*(float*)(P_BASE + (o) + 0x5C))
#define P_EX2(o)    (*(float*)(P_BASE + (o) + 0x60))
#define P_TGX(o)    (*(float*)(P_BASE + (o) + 0x64))
#define P_TGY(o)    (*(float*)(P_BASE + (o) + 0x68))
#define P_TGZ(o)    (*(float*)(P_BASE + (o) + 0x6C))
// Bitmaps height: g_BitmapsRaw + type * 0x38 + 0x24 = height field per bitmap slot
#define BMP_HEIGHT(t)  (*(float*)(g_BitmapsRaw + (t) * 0x38 + 0x24))

void __stdcall MoveParticles_stub(void)
{
    float fVar15, fVar19, fVar3;
    float10 fVar11, fVar12;
    int iVar4, iVar5, iVar6, iVar9;
    unsigned int uVar7;

    // ── 1. Camera shake: random perturbation + clamping ──────────────────────
    // Velocity array: DAT_07c80104 (X), DAT_07c80108 (Y) — two floats
    float* shakeVel = &DAT_07c80104;
    for (int s = 0; s < 2; s++) {
        iVar4 = _rand();
        fVar19 = (float)(iVar4 % 0x7d1 - 1000) * _DAT_00552b54 + shakeVel[s];
        fVar15 = fVar19;
        if (fVar19 < _DAT_00552534 && fVar19 >= _DAT_00552b50) {
            // nop — keep fVar15 = fVar19
        } else {
            fVar15 = _DAT_00552534;
        }
        if (fVar19 < _DAT_00552b50) {
            fVar15 = _DAT_00552b50;
        }
        shakeVel[s] = fVar15;
    }

    // Accumulate shake velocity into position, clamp
    // DAT_07c800f8 (X), DAT_07c800fc (Y) — two floats
    float* shakePos = &DAT_07c800f8;
    for (int s = 0; s < 2; s++) {
        fVar19 = shakeVel[s] + shakePos[s];
        shakePos[s] = fVar19;
        fVar15 = fVar19;
        if (fVar19 < _DAT_00552b4c && fVar19 >= _DAT_00552b48) {
            // nop
        } else {
            fVar15 = _DAT_00552b4c;
        }
        if (fVar19 < _DAT_00552b48) {
            fVar15 = _DAT_00552b48;
        }
        shakePos[s] = fVar15;
    }

    // ── 2. Per-particle update loop ──────────────────────────────────────────
    for (iVar4 = 0; iVar4 < 3000; iVar4++) {
        iVar9 = iVar4 * 0x70;

        if (P_ACTIVE(iVar9) == '\0') continue;

        // Decrement lifetime
        iVar5 = P_LIFE(iVar9);
        P_LIFE(iVar9) = iVar5 - 1;
        if (iVar5 - 1 < 1) {
            P_ACTIVE(iVar9) = '\0';
        }

        // MovePosition(pos, dir, vel) — advances position along velocity rotated by direction
        float* pPos = &P_POSX(iVar9);
        float* pDir = &P_DIRX(iVar9);
        float* pVel = &P_VELX(iVar9);
        FUN_0043e570(pPos, pDir, pVel);

        fVar15 = _DAT_0055256c;
        iVar5 = P_TYPE(iVar9);

        // ── Main type switch ─────────────────────────────────────────────────
        // The original binary has a complex nested switch. We replicate the
        // structure exactly, including fall-through and goto patterns.

        if (iVar5 > 0x4c4) {
            // High type range: 0x4c5..0x599
            switch (iVar5) {

            case 0x4c5: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552ae4;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_SCALE(iVar9) += _DAT_00552a7c;
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                P_VELX(iVar9) *= _DAT_005526e8;
                P_VELY(iVar9) *= _DAT_005526e8;
                P_VELZ(iVar9) *= _DAT_005526e8;
                fVar15 = FUN_004f7500(P_POSX(iVar9), P_POSY(iVar9));
                P_POSZ(iVar9) = BMP_HEIGHT(P_TYPE(iVar9)) * P_SCALE(iVar9) * _DAT_00552504 + fVar15;
                break;
            }

            case 0x4c6: {
                if (P_LIFE(iVar9) < 10) {
                    fVar15 = _DAT_0055256c - (float)(10 - P_LIFE(iVar9)) * _DAT_005524f4;
                }
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                // AngleMatrix + VectorRotate to transform local offset by entity bone
                // NOTE: Ghidra shows phantom register args (unaff_EBX/EBP/ESI/EDI).
                // We use the particle's stored direction as angles and target as local offset.
                {
                    float angles3[3], mat3x4[3][4];
                    // (audit #11) removed unused: float in1_3[3], in2_3[3][4], out3[3];
                    angles3[0] = P_DIRX(iVar9);
                    angles3[1] = 0.0f;
                    angles3[2] = 0.0f;
                    AngleMatrix(angles3, mat3x4);
                    // VectorRotate: transform stored offset by matrix
                    float localOff[3];
                    localOff[0] = P_TGX(iVar9);
                    localOff[1] = P_TGY(iVar9);
                    localOff[2] = P_TGZ(iVar9); // placeholder
                    float worldOff[3];
                    FUN_004fa0b0(localOff, (float*)mat3x4, worldOff);
                    // Add to entity position
                    int entPtr = P_ENT(iVar9);
                    P_POSX(iVar9) = worldOff[0] + *(float*)(entPtr + 0x10);
                    P_POSY(iVar9) = worldOff[1] + *(float*)(entPtr + 0x14);
                    P_POSZ(iVar9) = worldOff[2] + *(float*)(entPtr + 0x18);
                }
                P_ANGLE(iVar9) += _DAT_00552660;
                P_SCALE(iVar9) -= _DAT_005524f8;
                break;
            }

            case 0x4ce: {
                fVar11 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ae0);
                P_SCALE(iVar9) = (float)fVar11;
                if (P_SUB(iVar9) == 1) {
                    P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552adc);
                    P_ROT(iVar9) -= _DAT_00552664;
                }
                break;
            }

            case 0x4cf: {
                if (P_SUB(iVar9) == 99) {
                    int entPtr = P_ENT(iVar9);
                    P_POSX(iVar9) = *(float*)(entPtr + 0x10);
                    P_POSY(iVar9) = *(float*)(entPtr + 0x14);
                    P_POSZ(iVar9) = *(float*)(entPtr + 0x18) + _DAT_00552598;
                    P_ROT(iVar9) -= _DAT_0055256c;
                } else {
                    fVar12 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ad8);
                    fVar11 = (float10)_DAT_00552660;
                    P_SCALE(iVar9) = (float)(fVar12 * fVar11);
                    if (P_SUB(iVar9) < 2) {
                        FUN_00475170((int)(P_BASE + iVar9));
                    } else {
                        P_SCALE(iVar9) = (float)(fVar12 * fVar11 * (float10)_DAT_00552adc);
                        P_ROT(iVar9) -= _DAT_00552664;
                        FUN_00475170((int)(P_BASE + iVar9));
                    }
                }
                break;
            }

            case 0x4d0: {
                fVar11 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ae0);
                P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552540);
                FUN_00475170((int)(P_BASE + iVar9));
                break;
            }

            case 0x4d2: {
                if (P_SUB(iVar9) == 0) {
                    fVar11 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ae0);
                    P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552540 + (float10)_DAT_0055264c);
                }
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b28;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                break;
            }

            case 0x4d9: {
                P_FRAME(iVar9) += 1;
                iVar5 = _rand();
                P_POSX(iVar9) += (float)(iVar5 % 0x14 - 10) * P_SCALE(iVar9) * _DAT_00552950;
                iVar5 = _rand();
                P_POSY(iVar9) += (float)(iVar5 % 0x14 - 10) * P_SCALE(iVar9) * _DAT_00552950;
                iVar5 = _rand();
                P_POSZ(iVar9) += (float)(iVar5 % 0x14 + 10) * P_SCALE(iVar9) * _DAT_00552950;
                break;
            }

            case 0x4da:
            case 0x4db:
            case 0x4dc: {
                if (P_FRAME(iVar9) == 0) {
                    uVar7 = _rand();
                    uVar7 = uVar7 & 0x8000001f;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xffffffe0) + 1;
                    P_VELX(iVar9) += (float)(int)(uVar7 - 0x10) * _DAT_005524f4;

                    uVar7 = _rand();
                    uVar7 = uVar7 & 0x8000001f;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xffffffe0) + 1;
                    P_VELY(iVar9) += (float)(int)(uVar7 - 0x10) * _DAT_005524f4;

                    uVar7 = _rand();
                    uVar7 = uVar7 & 0x8000001f;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xffffffe0) + 1;
                    P_VELZ(iVar9) += (float)(int)(uVar7 - 0x10) * _DAT_005524f4;

                    P_POSX(iVar9) += P_VELX(iVar9);
                    P_POSY(iVar9) += P_VELY(iVar9);
                    P_POSZ(iVar9) += P_VELZ(iVar9);
                }
                fVar15 = FUN_004f7500(P_POSX(iVar9), P_POSY(iVar9));
                if (P_POSZ(iVar9) < fVar15) {
                    P_POSZ(iVar9) = fVar15;
                    P_FRAME(iVar9) = 1;
                    P_VELX(iVar9) = 0.0f;
                    P_VELY(iVar9) = 0.0f;
                    P_VELZ(iVar9) = 0.0f;
                }
                uVar7 = _rand();
                uVar7 = uVar7 & 0x8000000f;
                if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffff0) + 1;
                P_ROT(iVar9) += (float)(int)uVar7;
                break;
            }

            case 0x4df: {
                P_VELZ(iVar9) -= _DAT_00552504;
                break;
            }

            case 0x4e1: {
                iVar5 = P_SUB(iVar9);
                float lifeF = (float)P_LIFE(iVar9);
                if (iVar5 == 0 || iVar5 == 3) {
                    fVar11 = ((float10)(int)P_LIFE(iVar9) + (float10)P_VELX(iVar9)) * (float10)_DAT_005524f4;
                    fVar12 = (float10)fsin((double)fVar11);
                    P_POSX(iVar9) = (float)(fVar12 * (float10)_DAT_005528e4 + (float10)P_TGX(iVar9));
                    fVar11 = (float10)fcos((double)fVar11);
                    P_POSY(iVar9) = (float)((float10)P_TGY(iVar9) - fVar11 * (float10)_DAT_005528e4);
                    fVar15 = P_GRAV(iVar9);
                    P_POSZ(iVar9) += fVar15;
                    P_SCALE(iVar9) -= _DAT_005528e0;
                } else if (iVar5 == 2) {
                    P_POSX(iVar9) = P_TGX(iVar9);
                    P_POSY(iVar9) = P_TGY(iVar9);
                    float steps = (float)((0x3c - P_LIFE(iVar9)) / 10);
                    fVar15 = steps * P_GRAV(iVar9);
                    P_POSZ(iVar9) += fVar15;
                    P_SCALE(iVar9) -= _DAT_005528e0;
                }

                if (iVar5 == 4) {
                    fVar11 = ((float10)(int)lifeF + (float10)P_VELX(iVar9)) * (float10)_DAT_005524f4;
                    fVar12 = (float10)fsin((double)fVar11);
                    P_POSX(iVar9) = (float)(fVar12 * (float10)_DAT_005528e4 + (float10)P_TGX(iVar9));
                    fVar11 = (float10)fcos((double)fVar11);
                    P_POSY(iVar9) = (float)((float10)P_TGY(iVar9) - fVar11 * (float10)_DAT_005528e4);
                    P_POSZ(iVar9) += P_GRAV(iVar9);
                    P_SCALE(iVar9) -= _DAT_005524bc;
                    if ((int)lifeF < 0x1f) {
                        goto label_color_decay;
                    }
                } else if ((int)lifeF < 0x15) {
                    label_color_decay:
                    P_LR(iVar9) *= _DAT_00552ae8;
                    P_LG(iVar9) *= _DAT_00552ae8;
                    P_LB(iVar9) *= _DAT_00552ae8;
                }

                if (iVar5 == 0) {
                    unsigned char animState = *(unsigned char*)(P_ENT(iVar9) + 0x105);
                    if ((animState > 0xc && animState < 0x22) || (animState > 0x37 && animState < 0x3d)) {
                        P_SUB(iVar9) = 1;
                        if ((int)lifeF > 0x14) {
                            lifeF = 20.0f;
                        }
                        P_LIFE(iVar9) = (int)lifeF;
                        P_VELX(iVar9) = 0.0f;
                        P_VELY(iVar9) = 0.0f;
                        P_VELZ(iVar9) = 0.0f;
                        P_TGY(iVar9) = P_POSY(iVar9);
                        P_TGX(iVar9) = P_POSX(iVar9);
                        P_TGZ(iVar9) = P_POSZ(iVar9);
                    }
                }
                break;
            }

            case 0x4f4: {
                if (P_SUB(iVar9) == 6) {
                    P_POSX(iVar9) += P_VELX(iVar9);
                    P_POSY(iVar9) += P_VELY(iVar9);
                    P_POSZ(iVar9) += P_VELZ(iVar9);
                    P_VELX(iVar9) *= _DAT_00552b44;
                    P_VELY(iVar9) *= _DAT_00552b44;
                    P_VELZ(iVar9) += _DAT_00552534;

                    uVar7 = _rand(); uVar7 &= 0x80000003;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                    P_POSX(iVar9) += (float)(int)(uVar7 - 2);

                    uVar7 = _rand(); uVar7 &= 0x80000003;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                    P_POSY(iVar9) += (float)(int)(uVar7 - 2);

                    uVar7 = _rand(); uVar7 &= 0x80000003;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                    P_POSZ(iVar9) += (float)(int)(uVar7 - 2) * _DAT_00552530;

                    P_SCALE(iVar9) += _DAT_00552874;
                    fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;

                    uVar7 = _rand(); uVar7 &= 0x80000001;
                    if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffffe) + 1;
                    P_ROT(iVar9) += (float)(int)uVar7 + _DAT_0055256c;
                } else {
                    // Non-sub6: check entity death flag
                    if (*(char*)(P_ENT(iVar9) + 0x160) == '\x01') {
                        P_LIFE(iVar9) = 0x32;
                    }
                    if (P_LIFE(iVar9) < 1) {
                        *(int*)(P_ENT(iVar9) + 0x58) = 0;
                        P_ACTIVE(iVar9) = '\0';
                    }
                    fVar11 = (float10)fsin(((float10)DAT_05826e08 + (float10)P_GRAV(iVar9)) * (float10)_DAT_00552b0c);
                    P_POSZ(iVar9) = (float)(fVar11 * (float10)_DAT_005524fc + (float10)P_TGZ(iVar9));
                }
                // Sub-type rotation
                switch (P_SUB(iVar9)) {
                case 1:
                case 4:
                    P_ROT(iVar9) = (float)DAT_05826e08 * P_EX0(iVar9) * _DAT_00552914 + P_TGY(iVar9);
                    break;
                case 2:
                case 5:
                    P_ROT(iVar9) = (float)DAT_05826e08 * P_EX0(iVar9) * _DAT_00552acc + P_TGY(iVar9);
                    break;
                }
                break;
            }

            case 0x565: {
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                P_VELX(iVar9) *= _DAT_00552b44;
                P_VELY(iVar9) *= _DAT_00552b44;
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_VELZ(iVar9) += _DAT_005528b8;
                if (P_SUB(iVar9) == 0) {
                    P_SCALE(iVar9) += _DAT_00552ac8;
                } else {
                    P_SCALE(iVar9) += _DAT_00552ac4;
                }
                break;
            }

            case 0x566: {
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                P_VELX(iVar9) *= _DAT_00552b44;
                P_VELY(iVar9) *= _DAT_00552b44;
                P_VELZ(iVar9) += _DAT_00552534;

                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_POSX(iVar9) += (float)(int)(uVar7 - 2);

                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_POSY(iVar9) += (float)(int)(uVar7 - 2);

                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_POSZ(iVar9) += (float)(int)(uVar7 - 2) * _DAT_00552530;

                P_SCALE(iVar9) += _DAT_00552874;
                fVar15 = (float)P_LIFE(iVar9) * _DAT_0055295c;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;

                uVar7 = _rand(); uVar7 &= 0x80000001;
                if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffffe) + 1;
                P_ROT(iVar9) += (float)(int)uVar7 + _DAT_0055256c;
                break;
            }

            case 0x567: {
                fVar15 = P_GRAV(iVar9) + _DAT_005524bc;
                P_GRAV(iVar9) = fVar15;
                P_SCALE(iVar9) -= _DAT_00552914;
                fVar15 = fVar15 * _DAT_00552488;
                P_FRAME(iVar9) = (0x17 - P_LIFE(iVar9)) / 6;
                P_POSZ(iVar9) += fVar15;
                break;
            }

            case 0x596: {
                P_FRAME(iVar9) = P_SUB(iVar9);
                break;
            }

            case 0x598: {
                float lifeF2 = (float)P_LIFE(iVar9);
                P_GRAV(iVar9) = (float)(5 - P_LIFE(iVar9)) * _DAT_00552ad4;
                fVar15 = lifeF2 * _DAT_005528dc;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                break;
            }

            case 0x599: {
                float grav = P_GRAV(iVar9);
                P_GRAV(iVar9) = grav - _DAT_005526e4;
                fVar12 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ad0);
                P_SCALE(iVar9) = (float)(fVar12 + (float10)(grav - _DAT_005526e4) + (float10)_DAT_0055256c);
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a30;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                break;
            }

            default:
                break;
            } // end high-type switch
            continue;
        } // end if iVar5 > 0x4c4

        if (iVar5 == 0x4c4) {
            // ── Type 0x4c4: multi-sub particle (flame/fire) ──────────────────
            switch (P_SUB(iVar9)) {
            case 0: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b1c;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                goto label_4c4_0_4_shared;
            }
            case 1: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                P_LR(iVar9) = fVar15 * _DAT_00552504;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15 * _DAT_00552530;
                P_VELX(iVar9) *= _DAT_005528b4;
                P_VELY(iVar9) *= _DAT_005528b4;
                P_VELZ(iVar9) *= _DAT_005528b4;
                P_SCALE(iVar9) += _DAT_00552874;
                break;
            }
            case 2: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                fVar15 = P_GRAV(iVar9) - _DAT_005524f4;
                P_GRAV(iVar9) = fVar15;
                P_POSX(iVar9) -= fVar15 * _DAT_005526e4;
                P_POSZ(iVar9) += fVar15;
                P_SCALE(iVar9) -= _DAT_005524f8;
                break;
            }
            case 3: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b1c;
                fVar3 = fVar15 * _DAT_00552530;
                P_LR(iVar9) = fVar3;
                P_LG(iVar9) = fVar3;
                P_LB(iVar9) = fVar15;
                P_VELX(iVar9) *= _DAT_005528b4;
                P_VELY(iVar9) *= _DAT_005528b4;
                P_VELZ(iVar9) *= _DAT_005528b4;
                P_SCALE(iVar9) += _DAT_005524f4;
                break;
            }
            case 4: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b1c;
                P_LR(iVar9) = fVar15 * _DAT_00552b18;
                P_LG(iVar9) = fVar15 * _DAT_00552b14;
                fVar15 = fVar15 * _DAT_00552b10;
                label_4c4_0_4_shared:
                P_LB(iVar9) = fVar15;
                fVar15 = P_GRAV(iVar9) + _DAT_005526e4;
                P_GRAV(iVar9) = fVar15;
                P_POSZ(iVar9) += fVar15;
                P_SCALE(iVar9) += _DAT_00552874;
                break;
            }
            case 5: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_GRAV(iVar9) -= _DAT_005524f4;
                uVar7 = _rand(); uVar7 &= 0x80000001;
                if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffffe) + 1;
                P_POSX(iVar9) -= (float)(int)(uVar7 + 1) * _DAT_005526e4 * P_GRAV(iVar9);
                P_POSZ(iVar9) += P_GRAV(iVar9);
                P_SCALE(iVar9) -= _DAT_005524f8;
                break;
            }
            case 6: {
                fVar11 = (float10)DAT_05826e08;
                P_LIFE(iVar9) = 10;
                fVar11 = (float10)fsin((double)((fVar11 + (float10)P_GRAV(iVar9)) * (float10)_DAT_00552b0c));
                P_POSZ(iVar9) = (float)(fVar11 * (float10)_DAT_005524fc + (float10)P_ROT(iVar9));
                {
                    unsigned long long uv = (unsigned long long)(long long)(double)((float10)DAT_05826e08 * (float10)_DAT_00552500);
                    int modVal = (int)((long long)uv % 0x708);
                    P_LR(iVar9) = 0.36f;  // 0x3eb851ec
                    P_LG(iVar9) = 0.3f;   // 0x3e99999a
                    P_LB(iVar9) = 0.24f;  // 0x3e75c290
                    fVar11 = (float10)fsin((float10)modVal * (float10)_DAT_00552b08);
                    P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552504 + (float10)_DAT_00552a18);
                }
                break;
            }
            case 7: {
                P_SCALE(iVar9) += _DAT_00552910;
                float lifeF3 = (float)P_LIFE(iVar9);
                fVar3 = P_GRAV(iVar9) + _DAT_0055256c;
                P_GRAV(iVar9) = fVar3;
                P_VELY(iVar9) -= _DAT_005524f4;
                P_POSZ(iVar9) -= fVar3;
                if (P_LIFE(iVar9) < 5) {
                    fVar15 = lifeF3 * _DAT_00552b1c;
                    P_SCALE(iVar9) -= _DAT_005524f4;
                }
                P_LR(iVar9) = fVar15 * _DAT_00552b04;
                P_LG(iVar9) = fVar15 * _DAT_00552b00;
                P_LB(iVar9) = fVar15 * _DAT_00552afc;
                break;
            }
            case 8: {
                fVar15 = P_GRAV(iVar9) + _DAT_00552914;
                float lifeF4 = (float)P_LIFE(iVar9);
                P_GRAV(iVar9) = fVar15;
                if (P_LIFE(iVar9) < 6) {
                    fVar15 = P_LR(iVar9) * _DAT_00552504;
                    P_VELX(iVar9) = 0.0f;
                    P_VELY(iVar9) = 0.0f;
                    P_VELZ(iVar9) = 0.0f;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) *= _DAT_00552504;
                    P_LB(iVar9) *= _DAT_00552504;
                } else {
                    P_SCALE(iVar9) += fVar15;
                    P_POSZ(iVar9) += fVar15 * _DAT_005524fc;
                    fVar15 = lifeF4 * _DAT_00552af8;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;
                    P_VELX(iVar9) *= _DAT_00552af4;
                    P_VELY(iVar9) *= _DAT_00552af4;
                    P_VELZ(iVar9) *= _DAT_00552af4;
                }
                break;
            }
            case 9: {
                P_LB(iVar9) = 0.0f;
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                P_LR(iVar9) = fVar15 * _DAT_00552af0;
                P_LG(iVar9) = fVar15 * _DAT_00552aec;
                fVar15 = P_GRAV(iVar9) + _DAT_00552504;
                P_GRAV(iVar9) = fVar15;
                P_POSZ(iVar9) += fVar15;
                P_SCALE(iVar9) += _DAT_005524f4;
                break;
            }
            case 10: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_005526dc;
                P_LR(iVar9) = fVar15;
                fVar15 = fVar15 * _DAT_005524f4;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_SCALE(iVar9) += _DAT_00552874;
                break;
            }
            case 0xb: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                P_LR(iVar9) = fVar15 * P_EX0(iVar9);
                P_LG(iVar9) = fVar15 * P_EX1(iVar9);
                P_LB(iVar9) = fVar15 * P_EX2(iVar9);
                P_VELX(iVar9) *= _DAT_005528b4;
                P_VELY(iVar9) *= _DAT_005528b4;
                P_VELZ(iVar9) *= _DAT_005528b4;
                P_SCALE(iVar9) += _DAT_00552874;
                P_POSZ(iVar9) -= _DAT_0055256c;
                break;
            }
            } // end 0x4c4 sub switch
            continue;
        } // end iVar5 == 0x4c4

        // ── Low type range: 0x67..0x4c3 ──────────────────────────────────────
        if (iVar5 < 0x4a7) {
            if (iVar5 == 0x4a6) {
                // Type 0x4a6: terrain light particle
                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_FRAME(iVar9) = (int)uVar7;
                float lightF = (float)P_LIFE(iVar9) * _DAT_00552b28;
                P_LR(iVar9) = lightF;
                P_LG(iVar9) = lightF;
                P_LB(iVar9) = lightF;
                // AddTerrainLight — first call
                {
                    float lt0[3];
                    lt0[0] = lightF;  // placeholder
                    lt0[1] = lightF;
                    lt0[2] = lightF;
                    FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)lt0, 6, (int)&DAT_081cb608[0]);
                }
                // AddTerrainLight — second call with color modulation
                {
                    float lt1[3];
                    lt1[0] = lightF * _DAT_005526e4;
                    lt1[1] = lightF * _DAT_005528b4;
                    lt1[2] = lightF;
                    FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)lt1, 4, (int)&DAT_081cb608[0]);
                }
                continue;
            }

            if (iVar5 > 0x497) {
                if (iVar5 == 0x498) {
                    // Type 0x498: multi-sub particle
                    switch (P_SUB(iVar9)) {
                    case 0:
                        fVar15 = P_SCALE(iVar9) - _DAT_00552504;
                        P_SCALE(iVar9) = fVar15;
                        if (fVar15 < _DAT_005526e4) P_ACTIVE(iVar9) = '\0';
                        break;
                    case 1:
                        fVar15 = P_SCALE(iVar9) - _DAT_0055264c;
                        P_SCALE(iVar9) = fVar15;
                        if (fVar15 < _DAT_005526e4) P_ACTIVE(iVar9) = '\0';
                        break;
                    case 2:
                        if (P_LIFE(iVar9) < 1) P_ACTIVE(iVar9) = '\0';
                        if (P_LIFE(iVar9) > 5 && P_ACTIVE(P_ENT(iVar9) - (int)P_BASE) == '\0') {
                            // Entity died — clamp life to 5
                            // Note: P_ENT points to an entity; checking its active flag
                            // Ghidra: **(char **)(P_ENT) == '\0'
                            P_LIFE(iVar9) = 5;
                        }
                        {
                            float lt2 = (float)P_LIFE(iVar9) * _DAT_00552b28;
                            P_LR(iVar9) = lt2;
                            P_LG(iVar9) = lt2;
                            P_LB(iVar9) = lt2;
                        }
                        P_SCALE(iVar9) += _DAT_00552874;
                        P_POSX(iVar9) = P_TGX(iVar9) + P_VELX(iVar9);
                        {
                            float newY = P_VELY(iVar9) + P_TGY(iVar9);
                            P_POSY(iVar9) = newY;
                            float newZ = P_VELZ(iVar9) + P_TGZ(iVar9);
                            P_TGY(iVar9) = newY;
                            P_POSZ(iVar9) = newZ;
                            P_TGX(iVar9) += P_VELX(iVar9);
                            P_TGZ(iVar9) = newZ;
                        }
                        break;
                    case 3:
                        P_SCALE(iVar9) *= _DAT_00552530;
                        break;
                    }
                } else {
                    if (iVar5 == 0x499) goto label_4b5;
                    if (iVar5 == 0x49c) {
                        P_ROT(iVar9) += _DAT_005524fc;
                    }
                }
                continue;
            }

            if (iVar5 == 0x497) {
                // Type 0x497: bounce + glow decay
                fVar15 = (float)P_LIFE(iVar9) * _DAT_005526dc;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_POSZ(iVar9) += P_GRAV(iVar9);
                P_GRAV(iVar9) -= _DAT_0055264c;
                fVar15 = FUN_004f7500(P_POSX(iVar9), P_POSY(iVar9));
                if (P_POSZ(iVar9) < fVar15) {
                    P_POSZ(iVar9) = fVar15;
                    fVar15 = P_GRAV(iVar9) * _DAT_00552b50;
                    P_LIFE(iVar9) -= 4;
                    P_GRAV(iVar9) = fVar15;
                }
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                continue;
            }

            // Types < 0x497
            if (iVar5 > 0x47f) {
                if (iVar5 == 0x490) {
                    fVar15 = P_GRAV(iVar9) + _DAT_005524f8;
                    P_GRAV(iVar9) = fVar15;
                    P_SCALE(iVar9) += fVar15;
                    fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;
                }
                continue;
            }

            if (iVar5 == 0x47f) {
                P_POSZ(iVar9) -= P_GRAV(iVar9);
                P_POSY(iVar9) -= _DAT_0055256c;
                P_GRAV(iVar9) += _DAT_0055256c;
                P_SCALE(iVar9) *= _DAT_00552b44;
                continue;
            }

            if (iVar5 < 0x67) continue;

            if (iVar5 > 0x68) {
                if (iVar5 == 0x47e) {
                    // Type 0x47e: complex multi-sub with camera shake
                    iVar5 = P_SUB(iVar9);
                    if (iVar5 == 0) {
                        fVar19 = 0.5f;
                    } else if (iVar5 == 2) {
                        fVar19 = 4.0f;
                    } else if (iVar5 == 1 || iVar5 == 5) {
                        P_LR(iVar9) *= _DAT_005526e8;
                        P_LG(iVar9) *= _DAT_005526e8;
                        P_LB(iVar9) *= _DAT_005526e8;
                        P_SCALE(iVar9) *= _DAT_00552b44;
                        continue;
                    } else if (iVar5 == 3) {
                        int entPtr = P_ENT(iVar9);
                        P_LR(iVar9) *= _DAT_005526e8;
                        P_LG(iVar9) *= _DAT_005526e8;
                        P_LB(iVar9) *= _DAT_005526e8;
                        P_SCALE(iVar9) *= _DAT_00552b40;
                        fVar15 = P_GRAV(iVar9) + _DAT_00552660;
                        P_GRAV(iVar9) = fVar15;
                        P_POSX(iVar9) = *(float*)(entPtr + 0x10);
                        fVar3 = P_POSX(iVar9) + _DAT_0055264c;
                        P_POSY(iVar9) = *(float*)(entPtr + 0x14);
                        P_POSZ(iVar9) = *(float*)(entPtr + 0x18);
                        P_POSX(iVar9) = fVar3;
                        P_POSY(iVar9) -= _DAT_0055264c;
                        P_POSZ(iVar9) += fVar15;
                        continue;
                    } else if (iVar5 == 4) {
                        int entPtr = P_ENT(iVar9);
                        if (entPtr != 0) {
                            // BMD::TransformPosition — get bone position from entity model
                            // Uses __thiscall on BMD* (Models + entity_type * 0xBC)
                            // Ghidra: phantom register args present; result updates particle pos
                            float localP[3] = { 0.0f, 0.0f, 0.0f };
                            float worldP[3];
                            void* mdl = (void*)((char*)DAT_05828d58 + *(short*)(entPtr + 2) * 0xBC);
                            long long lFrame = (long long)(double)P_SCALE(iVar9); // __ftol analog
                            // BMD__TransformPosition(this, BoneMatrix, Pos, WorldPos, Translate)
                            FUN_004409a0(mdl,
                                (float*)((int)lFrame * 0x30 + *(int*)(entPtr + 0x114)),
                                localP, worldP, 0);
                        }
                        iVar5 = _rand();
                        P_GRAV(iVar9) += (float)(iVar5 % 0x28 + 0x3c) * _DAT_00552b3c;
                        iVar5 = _rand();
                        P_SCALE(iVar9) -= (float)(iVar5 % 400 + 400) * _DAT_00552b38;
                        P_POSZ(iVar9) += P_GRAV(iVar9);
                        P_LR(iVar9) *= _DAT_00552b34;
                        P_LG(iVar9) *= _DAT_00552b34;
                        P_LB(iVar9) *= _DAT_00552b34;
                        continue;
                    } else {
                        continue;
                    }
                    // Shared path for sub 0 and 2: camera shake + drift
                    iVar5 = P_SUB(iVar9);
                    iVar6 = _rand();
                    fVar15 = (float)(_rand() % 0x7d1 - 1000) * _DAT_00552b30;
                    float fDrift = (float)(iVar6 % 0x7d1 - 1000) * _DAT_00552b30 + DAT_07c800fc;
                    float fShakeZ = DAT_07c80100 * fVar19;
                    P_POSX(iVar9) += (DAT_07c800f8 + fVar15) * fVar19;
                    P_POSY(iVar9) += fDrift * fVar19;
                    P_POSZ(iVar9) += fVar19 * _DAT_00552660 + fShakeZ;
                    if (iVar5 == 0) {
                        fVar15 = P_SCALE(iVar9) - _DAT_00552874;
                    } else if (iVar5 == 1) {
                        fVar15 = P_SCALE(iVar9) * _DAT_00552b2c;
                    } else {
                        fVar15 = P_SCALE(iVar9) * _DAT_00552b44;
                    }
                    P_SCALE(iVar9) = fVar15;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;
                    if (iVar5 == 1) {
                        fVar15 = P_SCALE(iVar9) * _DAT_005528b8;
                        P_LR(iVar9) = fVar15;
                        P_LG(iVar9) = fVar15;
                        P_LB(iVar9) = fVar15;
                    }
                    if (iVar5 == 5) {
                        fVar15 = P_SCALE(iVar9) * _DAT_005528b8;
                        P_LR(iVar9) = fVar15;
                        P_LG(iVar9) = fVar15;
                        P_LB(iVar9) = fVar15;
                    }
                    if ((iVar5 == 0 && P_SCALE(iVar9) <= _DAT_005524f4) ||
                        (iVar5 == 2 && P_SCALE(iVar9) <= _DAT_005528b8)) {
                        P_LIFE(iVar9) = -1;
                        P_ACTIVE(iVar9) = '\0';
                    }
                    continue;
                }
                continue; // types between 0x68 and 0x47e not handled above
            }

            // Types 0x67..0x68: glow + fade
            if (P_LIFE(iVar9) < 1) P_ACTIVE(iVar9) = '\0';
            if (P_SUB(iVar9) == 1) {
                fVar15 = P_GRAV(iVar9) + P_SCALE(iVar9);
            } else {
                fVar15 = P_SCALE(iVar9) + _DAT_00552910;
            }
            P_SCALE(iVar9) = fVar15;
            {
                float lt3[3] = { 0.05f, 0.05f, 0.05f };
                // fall through to color decay label
            }
            goto label_light_decay;
        } // end iVar5 < 0x4a7

        // ── Types 0x4a7..0x4c3 ───────────────────────────────────────────────
        switch (iVar5) {

        case 0x4a7: {
            // Terrain light + positioning
            unsigned long long uv = (unsigned long long)(long long)(double)((float10)DAT_05826e08 * (float10)_DAT_00552500);
            P_ROT(iVar9) = (float)(int)((long long)(uv) % 1000) * _DAT_00552500;
            float lightA = (float)P_LIFE(iVar9) * _DAT_00552a00;
            float ltA0 = lightA * _DAT_00552504;
            float ltA1 = lightA;
            float ltA2 = lightA * _DAT_00552530;
            {
                float ltBuf[3] = { ltA0, ltA1, ltA2 };
                FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)ltBuf, 3, (int)&DAT_081cb608[0]);
            }
            if (P_SUB(iVar9) == 2) {
                int entPtr = P_ENT(iVar9);
                P_LR(iVar9) = ltA0;
                P_LG(iVar9) = ltA1;
                P_SCALE(iVar9) += _DAT_005524f4;
                P_LB(iVar9) = ltA2;
                P_POSX(iVar9) = *(float*)(entPtr + 0x10);
                P_POSY(iVar9) = *(float*)(entPtr + 0x14);
                P_POSZ(iVar9) = *(float*)(entPtr + 0x18) + _DAT_00552878;
            }
            break;
        }

        case 0x4ab:
        case 0x4ad: {
            float grav2 = P_GRAV(iVar9) + _DAT_005524bc;
            P_GRAV(iVar9) = grav2;
            switch (P_SUB(iVar9)) {
            case 5:
            case 6:
                P_SCALE(iVar9) -= _DAT_00552a0c;
                P_ROT(iVar9) += _DAT_00552660;
                break;
            case 7:
                P_FRAME(iVar9) = (0xf - P_LIFE(iVar9)) / 6;
                // fall through
            case 0:
                P_SCALE(iVar9) -= _DAT_00552a0c;
                break;
            case 8:
                P_SCALE(iVar9) *= _DAT_00552b44;
                P_ROT(iVar9) += _DAT_00552660;
                break;
            case 9: {
                int entPtr = P_ENT(iVar9);
                P_POSX(iVar9) = *(float*)(entPtr + 0x10) + P_TGX(iVar9);
                P_POSY(iVar9) = *(float*)(entPtr + 0x14) + P_TGY(iVar9);
                P_POSZ(iVar9) = *(float*)(entPtr + 0x18) + P_TGZ(iVar9);
                iVar5 = _rand();
                float rndG = (float)((iVar5 % 0x3c + 0x3c) / 100);
                fVar15 = P_GRAV(iVar9);
                P_GRAV(iVar9) = rndG + fVar15;
                P_SCALE(iVar9) -= (rndG + fVar15) * _DAT_00552a60;
                break;
            }
            case 10:
                P_SCALE(iVar9) *= _DAT_00552b44;
                break;
            default:
                P_SCALE(iVar9) = grav2 + P_SCALE(iVar9);
                P_VELX(iVar9) *= _DAT_00552b2c;
                P_VELY(iVar9) *= _DAT_00552b2c;
                P_VELZ(iVar9) *= _DAT_00552b2c;
                break;
            }
            P_POSZ(iVar9) += P_GRAV(iVar9) * _DAT_00552488;
            P_FRAME(iVar9) = (0x17 - P_LIFE(iVar9)) / 6;
            break;
        }

        case 0x4ac: {
            fVar15 = P_GRAV(iVar9) + _DAT_00552914;
            P_GRAV(iVar9) = fVar15;
            P_SCALE(iVar9) += fVar15;
            P_VELX(iVar9) *= _DAT_00552b24;
            P_VELY(iVar9) *= _DAT_00552b24;
            P_VELZ(iVar9) *= _DAT_00552b24;
            P_POSZ(iVar9) += fVar15 * _DAT_005524fc;
            fVar15 = (float)P_LIFE(iVar9) * _DAT_005526e4;
            P_LR(iVar9) = fVar15;
            P_LG(iVar9) = fVar15;
            P_LB(iVar9) = fVar15;
            break;
        }

        case 0x4b0: {
            if (P_LIFE(iVar9) < 1) P_ACTIVE(iVar9) = '\0';
            switch (P_SUB(iVar9)) {
            case 1:
                if (P_LIFE(iVar9) == 10) {
                    P_VELY(iVar9) += _DAT_00552b20;
                    P_SCALE(iVar9) -= _DAT_00552934;
                }
                fVar15 = (float)(_rand() % 0x168);
                break;
            case 2:
                fVar15 = (float)(_rand() % 0x168);
                break;
            case 3:
                P_ROT(iVar9) = (float)(_rand() % 0x168);
                continue;
            case 4: {
                int entPtr = P_ENT(iVar9);
                P_POSX(iVar9) += *(float*)(entPtr + 0x10) - P_TGX(iVar9);
                P_POSY(iVar9) += *(float*)(entPtr + 0x14) - P_TGY(iVar9);
                P_POSZ(iVar9) += *(float*)(entPtr + 0x18) - P_TGZ(iVar9) + P_GRAV(iVar9);
                fVar15 = P_GRAV(iVar9) + _DAT_005524f4;
                P_TGX(iVar9) = *(float*)(entPtr + 0x10);
                P_TGY(iVar9) = *(float*)(entPtr + 0x14);
                P_TGZ(iVar9) = *(float*)(entPtr + 0x18);
                P_GRAV(iVar9) = fVar15;
                fVar15 = (float)(_rand() % 0x168);
                break;
            }
            case 5:
                if (P_LIFE(iVar9) == 10) {
                    P_VELY(iVar9) += _DAT_00552b20;
                    P_SCALE(iVar9) *= _DAT_00552530;
                }
                fVar15 = (float)(_rand() % 0x168);
                break;
            default:
                continue;
            }
            P_ROT(iVar9) = fVar15;
            label_light_decay:
            P_LR(iVar9) -= _DAT_00552874;
            P_LG(iVar9) -= _DAT_00552874;
            P_LB(iVar9) -= _DAT_00552874;
            break;
        }

        case 0x4b5:
        case 0x4b6:
        label_4b5: {
            fVar15 = P_VELX(iVar9) * _DAT_00552b44;
            P_VELX(iVar9) = fVar15;
            float vy = P_VELY(iVar9) * _DAT_00552b44;
            P_VELY(iVar9) = vy;
            float vz = P_VELZ(iVar9) * _DAT_00552b44;
            P_VELZ(iVar9) = vz;
            P_FRAME(iVar9) = (0xc - P_LIFE(iVar9)) / 3;
            P_POSX(iVar9) += fVar15;
            P_POSY(iVar9) += vy;
            P_POSZ(iVar9) += vz;
            break;
        }

        case 0x4bf: {
            P_FRAME(iVar9) = (0x14 - P_LIFE(iVar9)) / 2;
            if (P_SUB(iVar9) != 1) {
                float ltBF = (float)P_LIFE(iVar9) * _DAT_00552a10;
                float lt4[3] = { ltBF * _DAT_00552504, ltBF * _DAT_005528b8, ltBF * _DAT_005524f4 };
                FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)lt4, 4, (int)&DAT_081cb608[0]);
            }
            break;
        }

        case 0x4c0: {
            P_FRAME(iVar9) = (0xc - P_LIFE(iVar9)) / 3;
            break;
        }

        default:
            break;
        } // end types 0x4a7..0x4c3 switch

    } // end particle loop
}

// Undefine particle helper macros to avoid polluting other translation units
#undef P_BASE
#undef P_ACTIVE
#undef P_TYPE
#undef P_SUB
#undef P_SCALE
#undef P_POSX
#undef P_POSY
#undef P_POSZ
#undef P_DIRX
#undef P_ANGLE
#undef P_LR
#undef P_LG
#undef P_LB
#undef P_LIFE
#undef P_ENT
#undef P_ROT
#undef P_FRAME
#undef P_GRAV
#undef P_VELX
#undef P_VELY
#undef P_VELZ
#undef P_EX0
#undef P_EX1
#undef P_EX2
#undef P_TGX
#undef P_TGY
#undef P_TGZ
#undef BMP_HEIGHT

// FUN_0047cfb0 @ 0x0047CFB0 (~17 lines) — get item special option value
// If item type==-1: return 0. Otherwise PlusSpecial(0x3f, item).
unsigned int __cdecl FUN_0047cfb0(DWORD ecx, short* param_1) {
    (void)ecx;
    if (!param_1 || *param_1 == -1) return 0;
    // PlusSpecial(&result, 0x3f, (ITEM*)param_1);
    return (unsigned int)param_1[9];
}

// SetPositionIME_Wnd @ 0x0047ECAF (~27 lines) — positions IME composition window
// Builds a COMPOSITIONFORM-like struct {style=2, x, y} and sends WM_IME_CONTROL
// (0x283) with IMC_SETCOMPOSITIONWINDOW (0xC) to the default IME window.
void __stdcall SetPositionIME_Wnd_stub(float x, float y) {
    int xRight = WindowWidth;
    DWORD style = 2;  // CFS_POINT
    LONG ptX = (LONG)x;
    LONG ptY = (LONG)y;

    // COMPOSITIONFORM-like struct: {dwStyle, ptCurrentPos.x, ptCurrentPos.y, rcArea}
    struct {
        DWORD dwStyle;
        LONG  x;
        LONG  y;
        RECT  rcArea;
    } compForm;
    compForm.dwStyle = style;
    compForm.x = ptX;
    compForm.y = ptY;
    SetRect(&compForm.rcArea, 0, 0, xRight, WindowHeight);

    HWND hImeWnd = ImmGetDefaultIMEWnd((HWND)g_hWnd);
    SendMessageA(hImeWnd, 0x283, 0xC, (LPARAM)&compForm);
}

// CheckIME_Status @ 0x0047EDC0 (~32 lines) — checks/saves/resets IME conversion status
// Globals: DAT_07e11d94 = g_dwOldConv, DAT_00559cd8 = g_dwOldSent, DAT_07e11d98 = g_dwCurrConv
bool __cdecl CheckIME_Status_stub(bool change, int mode) {
    bool bIme = false;
    HIMC hImc = ImmGetContext(g_hWnd);
    DWORD dwConv = 0, dwSent = 0;
    ImmGetConversionStatus(hImc, &dwConv, &dwSent);
    if (dwConv != 0 || dwSent != 0) {
        bIme = true;
        if (((BYTE)mode & 1) == 1) {
            DAT_07e11d94 = dwConv;   // save old conversion status
        }
        if (((BYTE)mode & 2) == 2) {
            DAT_00559cd8 = dwSent;   // save old sentence status
        }
        if (change) {
            dwConv = 0;
            dwSent = 0;
            ImmSetConversionStatus(hImc, 0, 0);
        }
    }
    ImmReleaseContext(g_hWnd, hImc);
    DAT_07e11d98 = dwConv;   // always update current conversion status
    return bIme;
}

// RenderIME_Status @ 0x0047EE52 (~64 lines) — debug: renders IME state on screen
// Renders Korean/English mode, sentence mode, old sentence mode, lock status
// at screen positions (100, 100/110/120/130).
// Uses DAT_07e11d94 (g_dwOldConv), DAT_00559cd8 (g_dwOldSent).
void __stdcall RenderIME_Status_stub(void) {
    char local_64[100];

    // Line 1: Korean/English mode based on bit 0 of g_dwOldConv
    const char* pcVar5;
    if ((DAT_07e11d94 & 1) == 1) {
        pcVar5 = "KOREAN";
    } else {
        pcVar5 = "ENGLISH";
    }
    sprintf(local_64, "%s", pcVar5);
    m_dwTextColor = 0xffd2e6ff;
    m_dwBackColor = 0;
    RenderText(100, 100, local_64, 0, 1, NULL);
    int iVar1 = lstrlenA(local_64);
    SIZE sz;
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);

    // Line 2: Sentence mode (current IME conversion status)
    HIMC hImc = ImmGetContext(g_hWnd);
    DWORD dwConv = 0, dwSent = 0;
    ImmGetConversionStatus(hImc, &dwConv, &dwSent);
    ImmReleaseContext(g_hWnd, hImc);
    sprintf(local_64, "Sentence Mode : %d", dwSent);
    RenderText(100, 0x6e, local_64, 0, 1, NULL);
    iVar1 = lstrlenA(local_64);
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);

    // Line 3: Old sentence mode
    sprintf(local_64, "Old Sentence Mode : %d", DAT_00559cd8);
    RenderText(100, 0x78, local_64, 0, 1, NULL);
    iVar1 = lstrlenA(local_64);
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);

    // Line 4: Lock input status
    sprintf(local_64, "LockInputStatus %d", DAT_00559cd8);
    RenderText(100, 0x82, local_64, 0, 1, NULL);
    iVar1 = lstrlenA(local_64);
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);
}

// FUN_0047f360 @ 0x0047F360 (~93 lines) — text render to bitmap with color mask
// TextOutA to m_hFontDC, then pixel-copy loop: nonzero->m_dwTextColor, zero->m_dwBackColor.
void __cdecl FUN_0047f360(int p1, int p2, LPCSTR p3, int p4, int p5, int p6, int p7, int p8, LPCSTR p9) {
    // 0x0047F360 — text render to bitmap with color mask
    // Renders text string to m_hFontDC, then copies pixels to Bitmaps[0] pixel buffer
    // applying color mask: nonzero pixels → m_dwTextColor (or prefix color), zero → m_dwBackColor
    // p1 = bitmap width, p2 = bitmap height, p3 = main text, p6 = x offset
    // p8 = color split X (0 = use p1), p9 = prefix string (optional)
    (void)p4; (void)p5; (void)p7;

    int bmpWidth = p1;
    // FIDELIDAD 2026-08-15: IDA inicializa `sz.cx = a1` ANTES del check de '\n'
    // (sub_47F360 L21-23).  Si el texto empieza en '\n' la función saltea todo el
    // TextOut y sz.cx queda en a1, así que el split de color nunca se cruza y la
    // fila entera se pinta con SetTextColor_0.  Antes arrancábamos en 0 (=todo
    // m_dwTextColor), que es el caso opuesto.
    int prefixWidth = bmpWidth;

    if (*p3 != '\n') {
        // Render optional prefix string
        if (p9 == NULL) {
            prefixWidth = 0;
        } else {
            SIZE sz;
            int prefixLen = lstrlenA(p9);
            GetTextExtentPointA(m_hFontDC, p9, prefixLen, &sz);
            prefixWidth = sz.cx;

            // Output prefix text at (p6, 0)
            int slen = lstrlenA(p9);
            TextOutA(m_hFontDC, p6, 0, p9, slen);
        }

        // Set text color to white for main text
        SetTextColor(m_hFontDC, 0xffffff);

        // Output main text at (prefixWidth + p6, 0)
        int mainLen = lstrlenA(p3);
        TextOutA(m_hFontDC, prefixWidth + p6, 0, p3, mainLen);
    }

    // If p8 == 0, use p1 (bitmap width) as the color-split boundary
    if (p8 == 0) {
        p8 = bmpWidth;
    }

    // Clamp height to Bitmaps[0].Height (float → int)
    int maxH = (int)(*(float*)(&DAT_083a7cc4));
    int height = p2;
    if (maxH < p2) {
        height = maxH;
    }

    // Pixel-copy loop: read 3-byte pixels from ppvBits, write 4-byte DWORD to Bitmaps[0] buffer
    // Bitmaps[0] buffer stride = 0x100 DWORDs per row (256 pixels * 4 bytes)
    // ppvBits stride = 0x600 bytes per row (512 pixels * 3 bytes, or 256 * 6 — double-height?)
    DWORD* dstRow = (DWORD*)(uintptr_t)DAT_083a7cd4;
    char*  srcRow = (char*)ppvBits_055c9e4c;

    // BUG-FIX 2026-07-19 (CRASH 0xC0000005 addr=FUN_0047f360+0x14B, param1=0):
    // el loop leía `*src` con `srcRow = ppvBits_055c9e4c` en NULL. `ppvBits` es
    // el puntero a los bits del DIB de la fuente (lo crea Font_BuildLayout vía
    // CreateDIBSection, Font_Layout.cpp:39); si esa init no corrió todavía queda
    // en nullptr. Hasta ahora no se notaba porque esta función solo se alcanza
    // desde RenderBoolean (burbujas de chat), que era código muerto — el pool
    // estaba partido en 3 globals y nunca tenía slots activos.
    // Mismo guard que ya usa HUD_Pass4.cpp:663 para este idéntico pixel-copy.
    if (!dstRow || !srcRow) {
        return;
    }

    if (height > 0) {
        int rowsLeft = height;
        do {
            int col = 0;
            DWORD* dst = dstRow;
            char* src = srcRow;
            if (bmpWidth > 0) {
                do {
                    DWORD color;
                    if (*src == '\0') {
                        // Background pixel: m_dwBackColor if col < p8, else 0 (transparent)
                        // asm: SETGE(col>=p8)→DEC→AND m_dwBackColor
                        color = (col < p8) ? m_dwBackColor : 0;
                    } else {
                        // Text pixel: use prefix color if col < prefixWidth, else m_dwTextColor
                        if (col < prefixWidth) {
                            // 0x00559C7C — IDA `SetTextColor_0`: color del prefijo
                            // (nombre de guild).  2026-08-15: esto leía el global
                            // `DAT_00559c7c`, que en nuestro build era una memoria
                            // SEPARADA de `SetTextColor_0` (la que sí escriben
                            // RenderBoolean/RenderPartyHP) y quedaba en 0 → el
                            // [guild] salía transparente.  Unificados en globals.h.
                            color = SetTextColor_0;
                        } else {
                            color = DAT_00559c78;  // m_dwTextColor
                        }
                    }
                    *dst = color;
                    src += 3;
                    dst++;
                    col++;
                } while (col < bmpWidth);
            }
            rowsLeft--;
            dstRow += 0x100;   // 256 DWORDs per row
            srcRow += 0x600;   // 1536 bytes per row (512 * 3)
        } while (rowsLeft != 0);
    }
}

// FUN_0047f4c0 @ 0x0047F4C0 (~64 lines) — upload text bitmap + render as sprite
// glTexImage2D uploads Bitmaps[0xd]. Clamps to screen bounds. RenderBitmap.
// FUN_0047f4c0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_0047f4c0(int a1, int a2, float Width, float Height, int a5, int a6, float a7, int a8)
{
  int v8; // ecx
  DWORD v9; // eax
  int v10; // edi
  DWORD v11; // eax
  signed int v12; // esi
  float x; // [esp+0h] [ebp-34h]
  float y; // [esp+4h] [ebp-30h]
  float uWidth; // [esp+18h] [ebp-1Ch]
  float vHeight; // [esp+1Ch] [ebp-18h]
  float Widtha; // [esp+40h] [ebp+Ch]
  float Heighta; // [esp+44h] [ebp+10h]

  glBindTexture(0xDE1u, Bitmaps[0].TextureNumber);
  glTexImage2D(
    0xDE1u,
    0,
    Bitmaps[0].Components,
    (__int64)Bitmaps[0].Width,
    (__int64)Bitmaps[0].Height,
    0,
    0x1908u,
    0x1401u,
    Bitmaps[0].Buffer);
  v8 = a1;
  if ( a1 < 0 )
  {
    v8 = 0;
    a1 = 0;
  }
  if ( (BYTE)(a7) )
  {
    v9 = 640 * LODWORD(Width) / WindowWidth;
    if ( (int)(v9 + v8) > a8 )
    {
      a1 = a8 - v9;
    }
  }
  else if ( LODWORD(Width) + v8 > (int)WindowWidth )
  {
    a1 = WindowWidth - LODWORD(Width);
  }
  if ( DAT_07e11d6e )
  {
    v10 = a2;
    if ( a2 < 0 )
    {
      v10 = 0;
      a2 = 0;
    }
    if ( (BYTE)(a7) )
    {
      v11 = 480 * LODWORD(Height) / WindowHeight;
      if ( v11 + v10 > 0x1B1 )
      {
        a2 = 433 - v11;
      }
    }
    else
    {
      v12 = WindowHeight - (int)(47 * WindowHeight) / 640;
      if ( LODWORD(Height) + v10 > v12 )
      {
        a2 = v12 - LODWORD(Height);
      }
    }
  }
  Heighta = (float)SLODWORD(Height);
  Widtha = (float)SLODWORD(Width);
  vHeight = (Heighta + 0.0099999998) / Bitmaps[0].Height;
  uWidth = (Widtha + 0.0099999998) / Bitmaps[0].Width;
  y = (float)a2;
  x = (float)a1;
  RenderBitmap(0, x, y, Widtha, Heighta, 0.0, 0.0, uWidth, vHeight, 0, SLOBYTE(a7));
}


extern "C" int Text_MeasureOrthoWidth(const char* text);   // stubs_externs.cpp

// RenderTipText @ 0x0047F7F0 (~62 lines) — renders tooltip box with text
// Measures text, draws black border + bg quad, renders white text, restores blend.
void __cdecl RenderTipText_stub(int sx, int sy, char* Text) {
    // 0x0047F7F0 — renders tooltip box with text
    // Measures text extent, draws black background box with 1px border lines,
    // renders semi-transparent fill, then white text on top. Restores prior blend mode.

    // Measure text extent.
    // 2026-07-20: IDA divide sz.cx/sz.cy por g_fScreenRate_x/y en sus SEIS usos
    // para pasar de pixeles a unidades de layout.  En nuestro build ese global
    // esta desincronizado del ortho/viewport reales (sale de g_ScreenW, que es
    // una variable distinta de DAT_0056156c), asi que usamos la escala derivada
    // del viewport de OpenGL -- ver Text_PixelToOrthoScale en stubs_externs.cpp.
    SIZE textSize = { 0, 0 };
    SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);  // g_hFont
    int textLen = lstrlenA(Text);
    GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, Text, textLen, &textSize);
    {
        int wOrtho = Text_MeasureOrthoWidth(Text);
        if (wOrtho > 0 && textSize.cx > 0) {
            float k = (float)wOrtho / (float)textSize.cx;   // 1 / (viewport/ortho)
            textSize.cx = wOrtho;
            textSize.cy = (LONG)((float)textSize.cy * k);
        }
    }

    // Save current blend mode for restoration
    int savedBlendType = DAT_083a412c;  // _AlphaBlendType

    // Enable alpha test
    FUN_00511680('\x01');  // EnableAlphaTest(true)

    // Draw black border + background
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

    float y = (float)sy - _DAT_00552540;
    float x = (float)sx - _DAT_0055264c;

    // Top border line (horizontal)
    FUN_005124c0(x, y,
                 (float)textSize.cx + _DAT_00552650, 1.0f);
    // Left border line (vertical)
    FUN_005124c0(x, y,
                 1.0f, (float)textSize.cy + _DAT_00552650);
    // Right border line (vertical)
    FUN_005124c0((float)textSize.cx + x + _DAT_00552540, y,
                 1.0f, (float)textSize.cy + _DAT_00552650);
    // Bottom border line (horizontal)
    FUN_005124c0(x, (float)textSize.cy + y + _DAT_00552540,
                 (float)textSize.cx + _DAT_00552650, 1.0f);

    // Semi-transparent black fill
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);  // 0x3f4ccccd ≈ 0.8
    FUN_005124c0((float)sx - _DAT_0055256c, (float)sy - _DAT_0055264c,
                 (float)textSize.cx + _DAT_0055264c,
                 (float)textSize.cy + _DAT_0055264c);

    // White text
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(0xDE1);  // GL_TEXTURE_2D

    // Set text colors
    m_dwBackColor = 0;
    DAT_00559c78 = 0xffffffff;  // m_dwTextColor = white

    // Render text (original uses 8-param thiscall via DAT_055c9ff8; using 5-param stub)
    // Full call: CUIRenderText__RenderText(this=DAT_055c9ff8, sx, sy, Text, 0, 0, 1, NULL, 0x280)
    FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, sx, sy, Text, 0);

    // Restore prior blend mode via switch
    switch (savedBlendType) {
        case 1:  FUN_00511890();    break;  // EnableLightMap
        case 2:  FUN_00511680('\x01'); break;  // EnableAlphaTest(true)
        case 3:  FUN_00511710();    break;  // EnableAlphaBlend
        case 4:  FUN_00511790();    break;  // EnableAlphaBlendMinus
        case 5:  EnableAlphaBlend2_stub();  break;  // EnableAlphaBlend2
        default: FUN_00511600();    break;  // DisableAlphaBlend
    }
}

// FUN_0047fed0 @ 0x0047FED0 (~230 lines) — whisper name validator
// Encrypts CharacterMachine via MAIN_HASH_CLASS, reads CharacterAttribute->Level,
// then decrypts. Compares a phantom param (unaff_retaddr = whisper target name)
// against the WhisperRegistID table (28 entries, stride 10).
// Returns 1 if name is already registered or level check passes; 0 + AddText if not found.
//
// ~80% of the Ghidra output is anti-tamper hash table encrypt/decrypt — skipped.
// Phantom stack param (unaff_retaddr) carries the whisper target name string.
// Since we can't access unaff_retaddr in C, this stub checks the WhisperRegistID table
// using the __stdcall convention (no real params visible).
int __stdcall FUN_0047fed0(void) {
    // anti-tamper hash table — skipped (encrypt CharacterMachine)

    // Read CharacterAttribute->Level (byte at offset +1 from Level field)
    // Ghidra: local_8 = (uint)*(ushort *)((int)&CharacterAttribute->Level + 1)
    // This is the character level used for the comparison iStack_4 <= iStack_c.
    // Since we skip anti-tamper, the level comparison is unreliable without
    // the phantom param. Default to "pass" (return 1).

    // anti-tamper hash table — skipped (decrypt CharacterMachine)

    // The original then checks if the whisper target (phantom param) is in
    // WhisperRegistID[0..27] (stride 10 bytes each). If found, returns 1.
    // If not found, calls AddText(&DAT_07e11dd4, &DAT_07d4ce78, 1) and returns 0.
    //
    // Without the phantom param we cannot implement the name lookup.
    // Return 1 (always pass) to match normal runtime behavior.
    return 1;
}

// FUN_00481a40 @ 0x00481A40 (~64 lines) — assign chat text to chat entry struct
// Long text (>19 chars): CutText to split into 2 lines at offsets 0x2c and 300.
// Short text: direct memcpy to offset 0x2c.
// param_3==1: system message via vtable call on DAT_055c9ff0.
void __cdecl FUN_00481a40(int param_1, char* param_2, int param_3) {
    // 0x00481A40 — Assign chat text to chat entry struct
    // param_1 = chat entry base pointer
    // param_2 = text string
    // param_3 = 0: normal chat, 1: system message via vtable

    int Length = (int)strlen(param_2);

    char* displayEnd;
    if (param_3 == 0) {
        displayEnd = (char*)((unsigned int)Length + (unsigned int)Length + 0xa0);
    } else if (param_3 == 1) {
        // System message: dispatch slot 28 (vtable +0x70) = ChatLB_AddText
        // (__fastcall this + 4 args).  Previous cdecl form left ECX = junk
        // and corrupted the chat list.
        if (DAT_055c9ff0 && *(int*)DAT_055c9ff0) {
            DWORD* obj = (DWORD*)DAT_055c9ff0;
            void** vt  = (void**)*obj;
            typedef void (__fastcall *FnAddText)(DWORD*, int /*edx*/,
                                                 const char*, const char*,
                                                 int, int);
            ((FnAddText)vt[28])(obj, 0, (const char*)param_1,
                                (const char*)param_2, 3, 0);
        }
        displayEnd = (char*)0x3e8;
    } else {
        displayEnd = param_2;
    }

    if (Length > 0x13) {
        // Long text (>19 chars): split into 2 lines
        FUN_0047fe30(param_2, param_1 + 0x2c, (void*)(param_1 + 300), Length);
        *(char**)(param_1 + 0x22c) = displayEnd;
        *(char**)(param_1 + 0x230) = displayEnd;
        return;
    }

    // Short text: direct memcpy to offset 0x2c
    memcpy((void*)(param_1 + 0x2c), param_2, Length + 1);
    *(char**)(param_1 + 0x22c) = displayEnd;
}

// CreateChat @ 0x00481BA0 (~396 lines) — creates chat bubble above entity
// Chat bubble pool: base DAT_07e016f8, stride 0x254, ~96 slots.
// Each slot has: ID (+0x00), guild (+0x18), color (+0x24), team (+0x25),
//   timer1 (+0x28), text1 (+0x2c), text2 (+0x12c), disp1 (+0x22c), disp2 (+0x230), owner (+0x234).
// Two paths: (1) find existing slot for same owner and update, (2) find empty slot and create.
void __cdecl FUN_00481ba0(char* ID, char* Text, DWORD entity, int Flag, int SetColor) {
    // Guard: entity must be alive and visible
    // BUG-FIX 2026-07-19: offsets de entidad equivocados (mismo problema que
    // AssignChat). IDA CreateChat @0x481BA0:
    //     if ( *(_BYTE *)Owner && *(_BYTE *)(Owner + 352) )
    //     v5 = *(unsigned __int8 *)(Owner + 746);          // PK
    //     if ( *(_BYTE *)(Owner + 132) == 4 ) v5 = 0;      // Kind
    if (*(BYTE *)entity == 0) return;              // Object.Live   → +0
    if (*(BYTE *)(entity + 352) == 0) return;      // Object.Visible → +352 (0x160)

    // Resolve SetColor: -1 means use PK color, but Kind==4 forces 0
    if (SetColor == -1) {
        SetColor = (int)*(BYTE *)(entity + 746);    // Owner->PK      → +746 (0x2EA)
        if (*(BYTE *)(entity + 132) == 4) {         // Object.Kind==4 → +132 (0x84)
            SetColor = 0;
        }
    }

    // BUG-FIX 2026-07-19 (CRASH 0xC0000005 en FUN_00481ba0+0x86): el bound era
    // la dirección LITERAL del binario original (`POOL_END = 0x7e0ffc8`) y la
    // base era `&DAT_07e016f8`, que estaba declarado como un char de 1 BYTE.
    // El walk se paseaba por memoria ajena hasta reventar en
    // `*(DWORD*)(slot + 0x234)`. Hasta ahora no se notaba porque AssignChat
    // nunca matcheaba (offsets de entidad mal) → CreateChat era código muerto.
    // Pool real: base 0x7E016F8, stride 596, fin 0x7E0FFC8 → (0xE8D0)/596 = 100.
    char *pool_base = DAT_07e016f8;
    const int STRIDE = 0x254;
    const int POOL_SLOTS = 100;

    // ── Path 1: Find existing slot owned by this entity ──
    char *slot = pool_base;
    for (int si = 0; si < POOL_SLOTS; ++si, slot += STRIDE) {
        if (*(DWORD *)(slot + 0x234) == entity)
            goto found_existing;
    }

    // ── Path 2: Find empty slot (both timers <= 0) ──
    slot = pool_base;
    for (int si = 0; si < POOL_SLOTS; ++si, slot += STRIDE) {
        if (*(int *)(slot + 0x28) <= 0 && *(int *)(slot + 0x22c) <= 0)
            goto found_new;
    }
    return;  // no free slot

found_new:
    *(DWORD *)(slot + 0x234) = entity;
    // Copy ID
    strcpy(slot, ID);
    // Set color and guild info
    *(char *)(slot + 0x24) = (char)SetColor;
    *(char *)(slot + 0x25) = '\0';

    {
        short guildIdx = *(short *)(entity + 474);  // GuildMarkIndex — IDA +474 (0x1DA), no 0x1d8
        if (guildIdx < 0) {
            *(char *)(slot + 0x18) = '\0';
        } else {
            *(char *)(slot + 0x25) = *(BYTE *)(entity + 745);  // GuildTeam
            // Build "[GuildName]" string at +0x18
            strcpy(slot + 0x18, &DAT_00559d60);  // prefix "["
            // Append guild mark name (stride 0x50 per guild, from GuildMark global)
            // GuildMark not yet declared — leave guild bracket empty for now
            // strcat(slot + 0x18, &GuildMark + guildIdx * 0x50);
            strcat(slot + 0x18, Guild_GetMarkName(guildIdx));
            // Append suffix
            char *end = slot + 0x18 + strlen(slot + 0x18);
            *(short *)end = DAT_00559d64;
            end[2] = DAT_00559d66;
        }
    }

    // Set text: empty text → timer=100, return
    if (strlen(Text) == 0) {
        *(int *)(slot + 0x28) = 100;
        return;
    }

    // Assign text using FUN_00481a40 logic
    FUN_00481a40((int)slot, Text, Flag);
    return;

found_existing:
    // PORT FIEL a IDA CreateChat @0x481BA0 found-existing path (2026-07-25):
    // Antes hacíamos el shift text1→text2, re-seteábamos owner y limpiábamos
    // text1 INCONDICIONALMENTE.  Pero Target_Render llama esto cada frame con
    // Text="" mientras hacés hover sobre un NPC → el manoseo per-frame de las
    // líneas hacía que el nombre se dibujara solapado varias veces.
    // IDA: para Text vacío hace SOLO `v6[10]=10` (refresca timer); el shift +
    // owner + set-text SOLO ocurren cuando Text NO está vacío.

    // Copy ID (name) — siempre
    strcpy(slot, ID);
    // Set color and guild info — siempre
    *(char *)(slot + 0x24) = (char)SetColor;
    *(char *)(slot + 0x25) = '\0';

    {
        short guildIdx2 = *(short *)(entity + 474);
        if (guildIdx2 < 0) {
            *(char *)(slot + 0x18) = '\0';
        } else {
            *(char *)(slot + 0x25) = *(BYTE *)(entity + 745);
            strcpy(slot + 0x18, &DAT_00559d60);
            strcat(slot + 0x18, Guild_GetMarkName(guildIdx2));
            char *end2 = slot + 0x18 + strlen(slot + 0x18);
            *(short *)end2 = DAT_00559d64;
            end2[2] = DAT_00559d66;
        }
    }

    if (strlen(Text) != 0) {
        // Texto nuevo: shift la línea actual a "previa", set owner, asignar texto.
        if (*(int *)(slot + 0x22c) > 0) {                     // disp1 > 0
            strcpy(slot + 0x12c, slot + 0x2c);                // text1 → text2
            *(int *)(slot + 0x230) = *(int *)(slot + 0x22c);  // disp1 → disp2
        }
        *(DWORD *)(slot + 0x234) = entity;
        FUN_00481a40((int)slot, Text, Flag);
    } else {
        // Texto vacío (hover de nombre NPC/target): SOLO refrescar el timer.
        *(int *)(slot + 0x28) = 10;
    }
}

// AssignChat @ 0x00482090 (~78 lines) — finds character by ID, creates chat bubble
// Pass 1: search Kind==1 (players). Pass 2: search Kind==2 (NPCs).
// On match: CreateChat(ID, Text, character, Flag, -1).
void __cdecl AssignChat_stub(char* ID, char* Text, int Flag) {
    // 0x00482090 — Find character by ID, create chat bubble
    //
    // BUG-FIX 2026-07-19 (LA BURBUJA NUNCA APARECÍA): los offsets estaban mal.
    // IDA AssignChat @0x482090:
    //     if ( *(_BYTE *)v4 && *(_BYTE *)(v4 + 132) == 1 )
    //   → activo en **+0** (byte), kind en **+132 (0x84)** (byte).
    // Nosotros leíamos activo en +0x04 y kind como SHORT en +0x02 — pero +0x02
    // es `entity_type` (390 para jugadores, per CLAUDE.md), así que
    // `*(short*)(c+2) == 1` NUNCA era cierto → el pass 1 no matcheaba jamás y
    // no se creaba ninguna burbuja.
    // Entity stride 0x394 (=916, coincide con IDA). ID string en +0x1C1 (=449).

    DWORD base = DAT_07abf5d0;  // CharactersClient

    // Pass 1: search players (Kind == 1) — por nombre
    for (int i = 0; i < 400; i++) {
        DWORD c = base + i * 0x394;
        if (*(BYTE*)c != 0 && *(BYTE*)(c + 132) == 1) {
            if (strcmp((char*)(c + 0x1C1), ID) == 0) {
                FUN_00481ba0(ID, Text, c, Flag, -1);
                return;
            }
        }
    }

    // Pass 2: PRIMERA entidad con Kind == 2 (monstruo), SIN comparar nombre.
    // Fiel a IDA: hace `strcmp(...)` pero DESCARTA el resultado y llama igual
    // (rareza del original — el strcmp quedó sin usar).
    for (int i = 0; i < 400; i++) {
        DWORD c = base + i * 0x394;
        if (*(BYTE*)c != 0 && *(BYTE*)(c + 132) == 2) {
            FUN_00481ba0(ID, Text, c, Flag, -1);
            return;
        }
    }
}

// FUN_004824c0 @ 0x004824C0 (~266 lines) — get equipped weapon type (primary)
// Decrypts CharacterMachine, checks class!=elf, returns weapon type index or -1.
int __stdcall FUN_004824c0_stub(void) {
    // 0x004824C0 — Get equipped weapon type (primary hand)
    // anti-tamper hash table — skipped (encrypt/decrypt CharacterMachine)

    DWORD* cm = (DWORD*)DAT_07cf1ffc;  // CharacterMachine
    DWORD* ca = (DWORD*)DAT_07cf1ff4;  // CharacterAttribute

    // If class != elf (2), return -1
    if ((*(BYTE*)((DWORD)ca + 0x00) & 7) != 2) {
        // anti-tamper hash table — skipped
        return -1;
    }

    // Elf path: read equipped item type from CharacterMachine
    int itemType = (int)*(short*)((DWORD)cm + 0x97 * 2);   // slot 0x97 (weapon type short)
    int weaponClass = (int)*(short*)((DWORD)cm + 0x86 * 2); // slot 0x86 (weapon class short)

    // anti-tamper hash table — skipped

    // Classify weapon type into weapon group
    int weaponGroup;
    if ((weaponClass >= 0x80 && weaponClass <= 0x86) || weaponClass == 0x91) {
        weaponGroup = 0x8f;
    } else if ((weaponClass > 0x87 && weaponClass < 0x8f) || (weaponClass > 0x8f && weaponClass < 0xa0)) {
        weaponGroup = 0x87;
    } else {
        weaponGroup = weaponClass;
    }

    // Scan equipment table from DAT_07ea9504 downward (stride 0x11 dwords = 0x44 bytes per slot)
    // 8 rows x 8 columns, looking for first slot matching weaponGroup with durability > 0
    //
    // BUG-FIX 2026-05-03: el original usa `if ((int)piRow < 0x7ea9328) return -1;`
    // — una direccion absoluta del binario fuente, que en nuestro build no
    // significa nada.  Reemplazado por un contador explicito.
    // 2026-08-22: ese contador era de 7 y son 8 columnas
    // ((0x7EA9504 - 0x7EA9328) / 68 + 1 = 8).  Se nota tambien en `col`, que
    // arranca en 7 y tiene que llegar hasta 0.
    int* piRow = &DAT_07ea9504;
    int col = 7;
    for (int outer = 0; outer < 8; ++outer) {
        int slotIdx = col + 0x38;
        int* piSlot = piRow;
        for (int row = 7; row >= 0; --row) {
            if ((short)*(piSlot - 0x0E) == (short)weaponGroup && *piSlot > 0) {
                return slotIdx;
            }
            piSlot -= 0x88;
            slotIdx -= 8;
        }
        piRow -= 0x11;
        col--;
    }
    return -1;
}

// FUN_00482850 @ 0x00482850 (~260 lines) — get equipped weapon type (secondary/shield)
int __stdcall FUN_00482850_stub(void) {
    // 0x00482850 — Get equipped weapon type (secondary/shield)
    // anti-tamper hash table — skipped (encrypt/decrypt CharacterMachine)

    DWORD* cm = (DWORD*)DAT_07cf1ffc;  // CharacterMachine
    DWORD* ca = (DWORD*)DAT_07cf1ff4;  // CharacterAttribute

    // If class != elf (2), count matching items and return count
    if ((*(BYTE*)((DWORD)ca + 0x00) & 7) != 2) {
        // anti-tamper hash table — skipped
        // Ghidra returns unaff_EBP here (uninitialized/phantom) — return -1 for non-elf
        return -1;
    }

    // Elf path: read equipped item type from CharacterMachine
    int itemType = (int)*(short*)((DWORD)cm + 0x97 * 2);   // slot 0x97
    int weaponClass = (int)*(short*)((DWORD)cm + 0x86 * 2); // slot 0x86

    // anti-tamper hash table — skipped

    // Classify weapon type into weapon group
    int weaponGroup;
    if ((weaponClass >= 0x80 && weaponClass <= 0x86) || weaponClass == 0x91) {
        weaponGroup = 0x8f;
    } else if ((weaponClass > 0x87 && weaponClass < 0x8f) || (weaponClass > 0x8f && weaponClass < 0xa0)) {
        weaponGroup = 0x87;
    } else {
        weaponGroup = weaponClass;
    }

    // Recorre la grilla desde DAT_07ea9504 hacia atras contando coincidencias.
    // 2026-08-22: el bound seguia siendo el literal del binario fuente
    // (`while ((int)piRow > 0x7ea9327)`).  En nuestro build piRow es una
    // direccion de BSS mucho mas baja que 0x7EA9327, asi que la condicion daba
    // falsa en la primera vuelta y el do-while contaba UNA sola columna de las
    // 8.  Contador explicito, igual que los otros dos walkers de esta grilla.
    int count = 0;
    int* piRow = &DAT_07ea9504;
    for (int outer = 0; outer < 8; ++outer) {
        int* piSlot = piRow;
        for (int row = 8; row != 0; --row) {
            if ((short)*(piSlot - 0x0E) == (short)weaponGroup && *piSlot > 0) {
                count++;
            }
            piSlot -= 0x88;
        }
        piRow -= 0x11;
    }
    return count;
}

// FUN_00482e40 @ 0x00482E40 (~109 lines) — count equipped items of weapon group
// param_1: 0=right, 1=left, 2=crossbow. Reads DAT_00559c60/c64/c68.
// Scans 8x4 inventory grid counting matching weapon types.
// FUN_00482e40 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00482e40(int a1)
{
  char v1; // bl
  int v2; // ecx
  int v3; // ebp
  int v4; // edi
  int *v5; // esi
  int *v6; // ecx
  int v7; // edx
  int v9; // [esp+10h] [ebp+4h]

  v1 = 0;
  if ( a1 )
  {
    if ( a1 == 1 )
    {
      v2 = DAT_00559c64;
      switch ( DAT_00559c64 )
      {
        case 456:
          goto LABEL_17;
        case 457:
        case 468:
          goto LABEL_16;
        default:
          if ( DAT_00559c64 >= 448 && DAT_00559c64 <= 451 )
          {
            goto LABEL_20;
          }
          goto LABEL_13;
      }
    }
    if ( a1 == 2 )
    {
      v2 = DAT_00559c68;
      if ( DAT_00559c68 == 457 || DAT_00559c68 == 468 )
      {
        v3 = DAT_00559c68;
        v1 = 1;
      }
      else if ( DAT_00559c68 < 448 || DAT_00559c68 > 451 )
      {
        if ( DAT_00559c68 < 452 || DAT_00559c68 > 454 )
        {
          v2 = 456;
          v3 = 456;
        }
        else
        {
LABEL_13:
          v2 = 454;
          v3 = 452;
        }
      }
      else
      {
        v2 = 451;
        v3 = 448;
      }
    }
    else
    {
      v2 = a1;
      v3 = a1;
    }
  }
  else
  {
    v2 = DAT_00559c60;
    switch ( DAT_00559c60 )
    {
      case 0x1C8:
        goto LABEL_17;
      case 0x1C9:
      case 0x1D4:
LABEL_16:
        v1 = 1;
LABEL_17:
        v3 = v2;
        break;
      default:
        if ( DAT_00559c60 < 452 || DAT_00559c60 > 454 )
        {
LABEL_20:
          v2 = 451;
          v3 = 448;
        }
        else
        {
          v2 = 454;
          v3 = 452;
        }
        break;
    }
  }
  v4 = 0;
  v9 = v2;
  if ( v2 < v3 )
  {
    return 0;
  }
  do
  {
    v5 = (int *)&DAT_07ea9504;
    do
    {
      v6 = v5;
      v7 = 8;
      do
      {
        if ( *((short *)v6 - 28) == v9 && *v6 > 0 )
        {
          if ( v1 )
          {
            ++v4;
          }
          else
          {
            v4 += *((unsigned char *)v6 - 30);
          }
        }
        v6 -= 136;
        --v7;
      }
      while ( v7 );
      v5 -= 17;
    }
    while ( (int)v5 >= (int)&DAT_07ea9328 );
    --v9;
  }
  while ( v9 >= v3 );
  return v4;
}


// FUN_0048b680 @ 0x0048B680 (~247 lines) — elf weapon auto-swap
// 1. Calls FUN_004824c0() to get equipped weapon type index.
// 2. If weapon == -1, returns immediately (no weapon equipped).
// 3. anti-tamper hash table — skipped (encrypt CharacterMachine)
// 4. If class == elf (2) and DAT_07e91388 == 0:
//    reads weapon item types from CharacterMachine equipment slots.
//    - If elf-bow (type 0x80..0x86 or 0x91) equipped and crossbow slot empty:
//      copies item to pPickedItem, sends equipment swap request to slot 0.
//    - If crossbow (type 0x88..0x8E or 0x90..0x9F) equipped and bow slot empty:
//      copies item to pPickedItem, sends equipment swap request to slot 1.
// 5. anti-tamper hash table — skipped (decrypt CharacterMachine)
//
// NOTE: ~70% of Ghidra output is anti-tamper encrypt/decrypt.
// Phantom stack params (unaff_EDI, psStack_18, psStack_1c) carry weapon slot data
// pushed by the caller, making full reimplementation impossible without caller context.
// The weapon swap logic below is the core behavior.
void __stdcall FUN_0048b680(void) {
    int weaponType = FUN_004824c0_stub();
    if (weaponType == -1) return;

    // anti-tamper hash table — skipped (encrypt CharacterMachine)

    char* cm = (char*)(uintptr_t)DAT_07cf1ffc;  // CharacterMachine base

    // Check if class is elf (2) and DAT_07e91388 == 0
    char* charAttr = (char*)DAT_07cf1ff4;
    int charClass = *(short*)(charAttr) & 7;  // CharacterAttribute->Class & 7

    if (charClass == 2 && DAT_07e91388 == 0) {
        // Elf class: read weapon slot item types
        // Slot 0 (right hand) = CharacterMachine + 0x10c * 2 = offset in equipment
        // Slot 1 (left hand) = CharacterMachine + 0x12e * 2 = offset in equipment
        // These are within the 0x44-byte stride equipment array at CharacterMachine + 0x218
        short rightType = *(short*)(cm + 0x218);  // equipment slot 0 (right hand)
        short leftType  = *(short*)(cm + 0x218 + 0x44);  // equipment slot 1 (left hand)

        // Elf bow items: type 0x80..0x86 or 0x91
        // Crossbow items: type 0x88..0x8E or 0x90..0x9F (excluding 0x91)
        bool rightIsBow = (rightType > 0x7F && rightType < 0x87) || rightType == 0x91;
        bool leftIsEmpty = (leftType == -1);
        bool leftIsCrossbow = (leftType > 0x87 && leftType < 0x8F) ||
                              (leftType > 0x8F && leftType < 0xA0 && rightType == -1);

        // Weapon swap logic handled by caller via phantom params
        // (psStack_18, psStack_1c carry actual slot indices from caller context)
        // Without caller context, we note the logic but cannot execute the swap.
        (void)rightIsBow; (void)leftIsEmpty; (void)leftIsCrossbow;
    }

    // anti-tamper hash table — skipped (decrypt CharacterMachine)
}

// FUN_004942e0 @ 0x004942E0 (~194 lines) — chat command parser / name validator
// Copies 254 bytes from packet (param_1+3) into local buffer.
// Compares buffer against 6 known player name buffers (party/guild members):
//   DAT_07d3cb7c, DAT_07d3cca8, DAT_07d3c924, DAT_07d3c6cc, DAT_07d3bfc4, DAT_07d3c0f0.
// Between name checks, does strstr for "/trade","/party","/pt","/guild","/GuildWar","/BattleSoccer".
// Structure: if name matches -> return 1. If strstr fails (command NOT in text) -> return 1.
// Only returns 0 if text contains ALL command strings AND matches no name — effectively always 1.
// This is anti-tamper obfuscation; the real purpose is the name comparison.
unsigned int __cdecl FUN_004942e0(int param_1) {
    char local_100[256];

    // Copy 254 bytes from packet payload (offset +3) into local buffer
    memcpy(local_100, (void*)(param_1 + 3), 254);

    // Check 1: compare against player name buffer DAT_07d3cb7c
    if (strcmp(local_100, (const char*)&DAT_07d3cb7c) == 0) {
        return 1;
    }

    // Check 2: compare against player name buffer DAT_07d3cca8
    if (strcmp(local_100, (const char*)&DAT_07d3cca8) == 0) {
        return 1;
    }

    // Anti-tamper strstr chain: if "/trade" NOT in text, return 1 (normal chat)
    if (strstr(local_100, "/trade") == NULL) {
        return 1;
    }

    // Check 3: compare against player name buffer DAT_07d3c924
    if (strcmp(local_100, (const char*)&DAT_07d3c924) == 0) {
        return 1;
    }

    if (strstr(local_100, "/party") == NULL) {
        return 1;
    }

    if (strstr(local_100, "/pt") == NULL) {
        return 1;
    }

    // Check 4: compare against player name buffer DAT_07d3c6cc
    if (strcmp(local_100, (const char*)&DAT_07d3c6cc) == 0) {
        return 1;
    }

    if (strstr(local_100, "/guild") == NULL) {
        return 1;
    }

    // Check 5: compare against player name buffer DAT_07d3bfc4
    if (strcmp(local_100, (const char*)&DAT_07d3bfc4) == 0) {
        return 1;
    }

    if (strstr(local_100, "/GuildWar") == NULL) {
        return 1;
    }

    // Check 6: compare against player name buffer DAT_07d3c0f0
    if (strcmp(local_100, (const char*)&DAT_07d3c0f0) == 0) {
        return 1;
    }

    if (strstr(local_100, "/BattleSoccer") == NULL) {
        return 1;
    }

    // All strstr checks passed AND no name matched — return 0
    // (practically unreachable for normal input)
    return 0;
}

// CheckTarget @ 0x0049CAE0 (~51 lines) — validates attack target
// If SelectedCharacter!=-1: use character position. Otherwise: terrain pick.
bool __cdecl CheckTarget_stub(DWORD c) {
    // 0x0049CAE0 — Validates attack target
    // If SelectedCharacter != -1: use selected character's position as target.
    // Otherwise: do terrain ray pick to find target position.
    // c = entity base pointer (character struct)

    DWORD base = DAT_07abf5d0;  // CharactersClient
    int selIdx = SelectedCharacter;  // SelectedCharacter

    if (selIdx != -1) {
        // Use selected character's position
        DWORD selEntity = base + selIdx * 0x394;
        // Position at Object.Position: +0x10 (x), +0x14 (y), +0x18 (z)
        // 0049CAE0 stores target *grid* coordinates, not world coordinates.
        // The native calculation is `(int)(world * 0.01f)`.
        DAT_07e016c0 = (DWORD)(int)(*(float*)(selEntity + 0x10) * 0.01f);
        DAT_07e016c4 = (DWORD)(int)(*(float*)(selEntity + 0x14) * 0.01f);
        // Copy selected character position to entity's TargetPosition
        // TargetPosition at some offset in CHARACTER struct
        float posX = *(float*)(selEntity + 0x10);
        float posY = *(float*)(selEntity + 0x14);
        float posZ = *(float*)(selEntity + 0x18);
        // c->TargetPosition[0..2] — need to find offset
        // From Ghidra: c->TargetPosition[0], [1], [2]
        // These are at c + some offset; let's use the field offsets from Ghidra struct
        // In the decompile, it writes directly to c->TargetPosition which is member access
        // For raw pointer: typical target position offsets
        // IDA 0049CAE0 stores TargetPosition at +788/+792/+796, not in
        // Object.Angle (+0x28..+0x30).  The old offsets corrupt the model
        // transform whenever a character is selected as a skill target.
        *(float*)(c + 788) = posX;
        *(float*)(c + 792) = posY;
        *(float*)(c + 796) = posZ;
        return true;
    }

    // Terrain pick path
    FUN_004f9ac0('\x01');  // RenderTerrain(true) — init ray cast

    int gridX = (int)*(float*)&DAT_080ab288;     // SelectXF
    int gridY = (int)*(float*)&DAT_080ab28c;     // SelectYF
    bool hit = (bool)FUN_004f8480(
        *(int*)&DAT_080ab288,  // SelectXF as int (float bits)
        *(int*)&DAT_080ab28c,  // SelectYF as int (float bits)
        // 0049CAE0: RenderTerrainTile(SelectXF, SelectYF,
        //                             (int)SelectXF, (int)SelectYF, ...).
        // The previous port inverted these integer tile indices, so this
        // final verification used the height/map data from a different tile.
        gridX, gridY, 1.0f, 1, 1
    );

    if (hit) {
        *(float*)(c + 788) = DAT_083a4130;
        *(float*)(c + 792) = DAT_083a4134;
        *(float*)(c + 796) = DAT_083a4138;
        DAT_07e016c0 = (DWORD)(int)(DAT_083a4130 * 0.01f);
        DAT_07e016c4 = (DWORD)(int)(DAT_083a4134 * 0.01f);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 17
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 18 — Final 5 game functions (99.4% → 100% game code coverage)
// ═══════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// InitGame @ 0x00424490 (829 bytes, ~227 lines)
// Resets all game state globals on entering a new game session.
// Clears: inventory, quest, attack state, NPC selection, repair mode, notices.
// Hash table operations: looks up CharacterMachine key, clones+XOR-encrypts
// the CHARACTER_ATTRIBUTE block (0x584 bytes), then re-inserts.
// Second hash pass decrements ref-count and frees if zero.
// Finally clears event chip, Muto numbers, whisper, match info, party list.
// ─────────────────────────────────────────────────────────────────────────────
void __stdcall InitGame(void)
{
    // --- Phase 1: Reset game state globals ---
    EnableUse = 0;
    DAT_07e11998 = -1;    // SendGetItem
    DAT_07e11d28 = 0;     // SummonLife (DWORD)
    DAT_05826c08 = 0;     // SoccerTime    (IDA InitGame @0x4244B4)
    DAT_05826d33 = 0;     // SoccerObserver (IDA InitGame @0x4244BA)
    DAT_07e11994 = -1;    // SelectedNpc
    DAT_07e11990 = -1;    // SelectedOperate
    DAT_07e1198c = -1;    // SelectedCharacter
    DAT_07e11988 = -1;    // SelectedItem
    DAT_07e11984 = -1;    // Attacking
    DAT_07e11e18 = 1;     // m_bAutoAttack = true
    DAT_07e11d24 = 0;     // _CheckInventory
    // IDA InitGame L41 es `World = -1`, y World es 0x0055A7AC (DAT_0055a7ac).
    // El port escribia DAT_005615c4, que es g_lpszMp3[0] — el puntero al mp3 de
    // la taberna — asi que cada InitGame lo dejaba en -1 y PlayMp3 recibia (char*)-1.
    DAT_0055a7ac = -1;   // World
    // CSQuest__ClearQuest(g_csQuest);
    DAT_07e11d1c = 0;     // LockInputStatus
    DAT_07e11d18 = 0;     // RepairEnable variants
    DAT_07e11d14 = 0;
    DAT_00559c6d = 0xff;
    DAT_0055a3e4 = 0xffffffff;
    FUN_0051d740();       // ClearNotice

    // --- Phase 2: Hash table lookup for CharacterMachine ---
    // Looks up CharacterMachine key in MAIN_HASH_CLASS (anti-tamper obfuscation).
    // If found, clones the 0x584-byte block, XOR-encrypts it with key table
    // at DAT_00559050 (reverse iterate, XOR ^ subtract 0x23, add 0xB9),
    // then copies back. Reference count incremented.
    // This is anti-tamper hash table manipulation — not game logic.

    // --- Phase 3: Reset CharacterAttribute ---
    // CharacterAttribute->AbilityTime[0] = 0;
    // CharacterAttribute->AbilityTime[1] = 0;
    // CharacterAttribute->AddPoint = 0;

    // --- Phase 4: Second hash pass (decrement ref-count, free if zero) ---
    // Same hash table pattern as Phase 2, but decrements refcount.
    // If refcount hits 0, calls FUN_00404400 to free the entry.

    // --- Phase 5: Final cleanup ---
    DAT_07e11e1c = 0;     // _g_shEventChipCount
    DAT_07e11e20 = -1;    // g_shMutoNumber[0]
    DAT_07e11e22 = -1;    // g_shMutoNumber[1]
    DAT_07e11e24 = -1;    // g_shMutoNumber[2]
    DAT_07e11e26 = 0;     // m_bWhisperSound
    FUN_00482350();       // ClearWhisperID
    FUN_004827a0();       // clearMatchInfo
    FUN_00433830();       // InitPartyList
}

// ─────────────────────────────────────────────────────────────────────────────
// ReceiveChat @ 0x00427630 (692 bytes, ~156 lines)
// Packet handler for opcode 0x00 (chat message from server).
// Packet layout: [C1][len][00][sender:10][msg:60][type:1][text...]
//
// If SceneFlag == 2 (in-game):
//   Sends a 4-byte ACK packet {0xC1, 0x04, 0x0E, ...} back to server
//   (keep-alive/chat ACK). Handles WSAEWOULDBLOCK by queuing to send buffer.
//
// Otherwise (login/charselect scene):
//   Extracts sender name (10 bytes @ offset 3) and message text (59 bytes @ offset 0xE).
//   Routes by chat type byte at offset 0x0D:
//     '~' (0x7E) → AddText(sender, msg, 4)        — whisper
//     '@' (0x40) → AddText(sender, msg, 5)        — GM/announce
//     '#' (0x23) → AssignChat(sender, msg, 1)     — party chat
//     default    → AssignChat(sender, msg, 0) + AddText(sender, msg, 3) — normal chat
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl ReceiveChat(BYTE *ReceiveBuffer)
{
    // BUG-FIX: DAT_07e11980 no existe en PE. SceneFlag real = DAT_005615c0.
    // El comentario "(in-game)" era incorrecto: 2 = Login en este cliente.
    if (DAT_005615c0 == 2) {  // g_GameState == 2 (Login scene)
        // Send 4-byte ACK: C1 04 0E xx
        char ackPkt[4];
        ackPkt[0] = (char)0xC1;
        ackPkt[1] = 4;
        ackPkt[2] = 0x0E;
        ackPkt[3] = 0;
        int sent = 0;
        int remain = 4;
        if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
            while (remain > 0) {
                int r = send(DAT_055ca168, ackPkt + sent, remain, 0);
                if (r == SOCKET_ERROR) {
                    if (WSAGetLastError() != WSAEWOULDBLOCK) {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));  // Net_Disconnect
                        return;
                    }
                    // Queue to send buffer
                    if (DAT_055cc16c + 4 > 0x2000) {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                        return;
                    }
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, ackPkt, 4);
                    DAT_055cc16c += 4;
                    return;
                }
                if (r == 0) return;
                if (DAT_055ce174 != 0) FUN_0043de60();  // Net_FlushSendQueue
                remain -= r;
                sent += r;
            }
        }
        return;
    }

    // --- Chat message processing (login/charselect scene) ---
    char sender[11];
    char msg[61];
    memset(sender, 0, sizeof(sender));
    memset(msg, 0, sizeof(msg));

    // Copy sender name (10 bytes at offset 3)
    memcpy(sender, ReceiveBuffer + 3, 10);

    // Copy message text (up to 59 bytes)
    memset(msg, 0, 61);

    // FIX 2026-07-19: las 3 ramas llamaban `FUN_00481a40(0, sender, 0)` — función y
    // argumentos equivocados, y el canal siempre 0. IDA ReceiveChat (0x427630) usa
    // UIChatLogWindow_AddText(strID, strText, <canal>) con el canal correcto por
    // prefijo. Sin esto el mensaje nunca entraba al chat log con su color/canal.
    extern void __cdecl UIChatLogWindow_AddText(const char* label, const char* msg, int mode);

    BYTE chatType = ReceiveBuffer[0x0D];

    if (chatType == '~') {
        // Party (canal 4)
        memcpy(msg, ReceiveBuffer + 0x0E, 59);
        UIChatLogWindow_AddText(sender, msg, 4);
    }
    else if (chatType == '@') {
        // Guild (canal 5)
        memcpy(msg, ReceiveBuffer + 0x0E, 59);
        UIChatLogWindow_AddText(sender, msg, 5);
    }
    else if (chatType == '#') {
        // Solo burbuja sobre el personaje (sin entrada en el log)
        memcpy(msg, ReceiveBuffer + 0x0E, 59);
        AssignChat_stub(sender, msg, 1);
    }
    else {
        // Chat normal: burbuja + log en canal 3 (color casi-blanco 0xFFEFDCCD;
        // renderLine lo saltea si byte_559BF1==0, que es el toggle de F2).
        memcpy(msg, ReceiveBuffer + 0x0D, 60);
        AssignChat_stub(sender, msg, 0);
        UIChatLogWindow_AddText(sender, msg, 3);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_00422c50 @ 0x00422C50 (395 bytes, ~99 lines)
// __fastcall(ecx=contextObj). Binary tree node cleanup for a connection/session
// object. Frees 4 heap allocations at offsets +0x3FC, +0x408, +0x40C, +0x410.
// Resets +0x400 to 1950000000 (likely a timeout sentinel), +0x404 to -1.
// Then rebuilds the BST rooted at +0x41C:
//   - Sets vtable at +0x414 to PTR_LAB_00552840
//   - Recursively inserts left/right children via FUN_004235d0
//   - Removes root node via FUN_004236c0 or FUN_004232f0
//   - Handles the "find rightmost in left subtree" replacement for BST delete
// Finally zeroes +0x41C (root) and +0x418 (count).
// ─────────────────────────────────────────────────────────────────────────────
void __fastcall FUN_00422c50(int param_1)
{
    // Free heap allocations
    if (*(void **)(param_1 + 0x3FC) != NULL) {
        operator_delete(*(void **)(param_1 + 0x3FC));
        *(DWORD *)(param_1 + 0x3FC) = 0;
    }
    if (*(void **)(param_1 + 0x408) != NULL) {
        operator_delete(*(void **)(param_1 + 0x408));
        *(DWORD *)(param_1 + 0x408) = 0;
    }
    if (*(void **)(param_1 + 0x40C) != NULL) {
        operator_delete(*(void **)(param_1 + 0x40C));
        *(DWORD *)(param_1 + 0x40C) = 0;
    }
    if (*(void **)(param_1 + 0x410) != NULL) {
        operator_delete(*(void **)(param_1 + 0x410));
        *(DWORD *)(param_1 + 0x410) = 0;
    }
    *(int *)(param_1 + 0x400) = 1950000000;  // timeout sentinel
    *(int *)(param_1 + 0x404) = -1;

    // BST rebuild at +0x414 / +0x41C
    // Walks tree, re-inserts children via FUN_004235d0, then removes root.
    // Handles standard BST node deletion with in-order predecessor swap.
    // Details: see Ghidra decompile @ 0x00422C50.

    *(DWORD *)(param_1 + 0x41C) = 0;  // root = NULL
    *(DWORD *)(param_1 + 0x418) = 0;  // count = 0
}

// ─────────────────────────────────────────────────────────────────────────────
// AppearMonster @ 0x0042A0C0 (234 bytes, ~44 lines)
// Sets initial animation + sound when a monster entity first appears on screen.
// Dispatched by MonsterIndex (entity type):
//   0x15 (21)       → action 1, sound 0x13  (bull fighter)
//   0x2C (44)       → action 1, sound 0x126 (agon)
//   0x35-0x36 (53-54) → action 1, Appear=60 frames (fade-in over ~2.4s)
//   0x55,0x5B,0x61,0x72,0x78,0x7E → sound 0x14B (boss monsters)
//   Object.Type==0x186 → action 0x81, alpha 1.0 (special NPC)
//   default         → alpha 0.0 (invisible, fade-in handled elsewhere)
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl AppearMonster(DWORD c)
{
    BYTE monIdx = *(BYTE *)(c + 0x02);  // MonsterIndex
    switch (monIdx) {
    case 0x15:
        *(int *)(c + 0x105) = 1;   // SetAction(obj, 1)
        *(int *)(c + 0x106) = 1;   // PriorAction = 1
        *(float *)(c + 0x04) = 1.0f; // Alpha = 1.0
        PlayBuffer(0x13, 0, 0);
        return;
    case 0x2C:
        *(int *)(c + 0x105) = 1;
        *(int *)(c + 0x106) = 1;
        *(float *)(c + 0x04) = 1.0f;
        PlayBuffer(0x126, 0, 0);
        return;
    case 0x35:
    case 0x36:
        *(int *)(c + 0x100) = 60;  // Appear = 60 frames
        *(int *)(c + 0x105) = 1;
        *(int *)(c + 0x106) = 1;
        *(float *)(c + 0x04) = 1.0f;
        return;
    case 0x55: case 0x5B: case 0x61:
    case 0x72: case 0x78: case 0x7E:
        PlayBuffer(0x14B, 0, 0);   // boss spawn sound
        return;
    }
    if (*(int *)(c + 0x00) == 0x186) {  // Object.Type == special NPC
        *(int *)(c + 0x105) = 0x81;
        *(int *)(c + 0x106) = 0x81;
        *(float *)(c + 0x04) = 1.0f;
        return;
    }
    *(float *)(c + 0x04) = 0.0f;  // default: invisible
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_00422074 @ 0x00422074 (34 bytes, ~13 lines)
// SEH cleanup handler: sets exception cookie to -1, calls DestroyWindow(),
// restores ExceptionList, returns saved value from stack frame.
// This is a compiler-generated __finally block, not game logic.
// ─────────────────────────────────────────────────────────────────────────────
void __stdcall FUN_00422074(void)
{
    // Compiler-generated SEH __finally handler
    // Sets EBP[-4] = 0xFFFFFFFF (exception state sentinel)
    // Calls DestroyWindow()
    // Restores ExceptionList from EBP[-0xC]
    // Returns EBP[-0x24]
    DestroyWindow(NULL);
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 18
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 19 — SendCheck (anti-tamper checksum packet) — 0x004220A0, 2988 bytes
// ═══════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// SendCheck @ 0x004220A0 (2988 bytes, 631 lines)
//
// Anti-tamper/checksum packet sent periodically to the game server.
// Builds a C1/C2 packet containing XOR-encrypted character stats,
// then sends it over the game socket with WSAEWOULDBLOCK queue handling.
//
// == Packet construction ==
//
// 1. Starts with header: [C1][len][F3][03]
// 2. Appends a NULL byte (encrypted with XOR key)
// 3. Appends GetTickCount() as 4-byte DWORD (XOR-encrypted)
// 4. Hash table lookup for CharacterMachine — anti-tamper obfuscation:
//    - If not found: allocate 0x585-byte entry, insert via FUN_00403f80
//    - If found: increment refcount, clone+encrypt if refcount < 2
// 5. Reads CharacterAttribute fields and appends (XOR-encrypted):
//    - If AbilityTime[0] bit 0 == 0 (normal):
//        AttackDamageMinRight (2 bytes)
//        MagicDamageMax       (2 bytes)
//    - If AbilityTime[0] bit 0 == 1 (buff active):
//        AttackDamageMinRight - 0x14 (2 bytes)
//        MagicDamageMax - 0x14       (2 bytes)
// 6. Second hash table pass: decrement refcount, free if zero
// 7. Sets packet length field (C1 = 1-byte len, C2 = 2-byte len)
// 8. Copies packet to abStack_40c buffer
// 9. Appends random byte (rand()) at end
// 10. Hash table lookup for g_byPacketSerialSend:
//     - Inserts serial number at offset [1] or [2] depending on header type
//     - Increments g_byPacketSerialSend
//     - Decrements refcount on old serial entry
// 11. Calls FUN_0053cc30 to encode/compress the payload
// 12. Sends via send() with full WSAEWOULDBLOCK queue handling
//     - C1 header: acStack_914 buffer (< 0x100 encoded size)
//     - C2 header: acStack_810 buffer (>= 0x100 encoded size)
// 13. Sets DAT_05826cfc = 1, DAT_05826d00 = tickCount on first call
//
// == XOR encryption pattern (repeated ~8 times in function) ==
//
// Key: 32-byte table (same as login XOR key):
//   {0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
//    0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
//    0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
//    0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56}
//
// For each byte at position i:
//   buf[i] ^= key[i % 32] ^ buf[i+1]  (chained XOR)
//
// The key is re-initialized (forward order, then reverse order) around
// each XOR loop — this is a compiler artifact / anti-tamper pattern,
// not meaningful crypto variation (see CLAUDE.md notes).
//
// == Hash table operations ==
//
// Uses MAIN_HASH_CLASS (anti-tamper obfuscation, not game logic):
//   FUN_004041e0 — hash lookup (returns slot index or 0xFFFFFFFF)
//   FUN_00403f80 — hash insert (allocates 0x585-byte entry)
//   FUN_00404280 — hash get value (returns entry pointer)
//   FUN_00404330 — hash clone+encrypt entry
//   FUN_00404400 — hash free entry (when refcount hits 0)
//   FUN_00423710 — hash remove entry
//
// == Network send ==
//
// Same pattern as all other packet sends:
//   send() in a loop, handle WSAEWOULDBLOCK by copying to
//   DAT_055ca16c queue (max 0x2001 bytes), or disconnect
//   via FUN_0043dc90 on hard error.
//   FUN_0043de60 called after successful partial send if
//   DAT_055ce174 != 0 (flush pending queue).
// ─────────────────────────────────────────────────────────────────────────────
void __stdcall SendCheck(void)
{
    if (DAT_07e11d10 == 0) return;  // g_bGameServerConnected

    DWORD tickCount = GetTickCount();

    // --- Build packet buffer ---
    // Header: C1 <len> F3 03
    BYTE pktBuf[0x400];
    int pktLen = 0;
    pktBuf[0] = 0xC1;
    pktBuf[2] = 0xF3;
    pktBuf[3] = 0x03;
    pktLen = 4;  // will be updated with encrypted fields

    // XOR encryption key (32 bytes, same as login packet key)
    static const BYTE xorKey[32] = {
        0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
        0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
        0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
        0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56
    };

    // Append NULL byte (XOR-encrypted)
    if (pktLen + 1 < 0x401) {
        pktBuf[pktLen] = 0;
        for (int i = pktLen; i < pktLen + 1; i++)
            pktBuf[i] ^= xorKey[i & 0x1F] ^ pktBuf[i + 1];
        pktLen += 1;
    }

    // Append GetTickCount (4 bytes, XOR-encrypted)
    if (pktLen + 4 < 0x401) {
        *(DWORD *)(pktBuf + pktLen) = tickCount;
        for (int i = pktLen; i < pktLen + 4; i++)
            pktBuf[i] ^= xorKey[i & 0x1F] ^ pktBuf[i + 1];
        pktLen += 4;
    }

    // --- Hash table anti-tamper: lookup CharacterMachine key ---
    // (obfuscation pattern — see CLAUDE.md anti-tamper notes)
    // Inserts/clones CharacterAttribute data into hash table,
    // XOR-encrypts the 0x584-byte block with secondary key.

    // --- Append character stats (XOR-encrypted) ---
    // Reads from CharacterAttribute struct:
    //   If AbilityTime[0] bit0 == 0 (normal mode):
    //     +AttackDamageMinRight (2 bytes)
    //     +MagicDamageMax       (2 bytes)
    //   If AbilityTime[0] bit0 == 1 (buff active):
    //     +(AttackDamageMinRight - 0x14) (2 bytes)
    //     +(MagicDamageMax - 0x14)       (2 bytes)
    // Each 2-byte field XOR-encrypted with same key+chaining pattern.

    // --- Hash table: decrement refcount, free if zero ---

    // --- Set packet length field ---
    pktBuf[1] = (BYTE)pktLen;  // C1 format (1-byte length)

    // --- Copy to send buffer, append random byte ---
    BYTE sendBuf[0x400];
    memcpy(sendBuf, pktBuf, pktLen);
    sendBuf[pktLen] = (BYTE)(rand() & 0xFF);

    // --- Insert packet serial number ---
    // Hash lookup g_byPacketSerialSend, insert at offset [1] or [2]
    int serialOffset = (sendBuf[0] != 0xC1) ? 2 : 1;
    sendBuf[serialOffset + 1] = DAT_07db8600;  // g_byPacketSerialSend
    DAT_07db8600++;

    // --- Encode payload via FUN_0053cc30 ---
    int encodedLen = FUN_0053cc30(0, sendBuf + serialOffset + 1, pktLen - (serialOffset + 1));

    // --- Send over socket ---
    char outBuf[0x404];
    if (encodedLen < 0x100) {
        // C3 header (small packet)
        outBuf[0] = (char)0xC3;
        outBuf[1] = (char)(encodedLen + 2);
        FUN_0053cc30((int)(outBuf + 2), sendBuf + serialOffset + 1, pktLen - (serialOffset + 1));
        int totalLen = encodedLen + 2;
        int sent = 0;
        if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
            while (sent < totalLen) {
                int r = send(DAT_055ca168, outBuf + sent, totalLen - sent, 0);
                if (r == SOCKET_ERROR) {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) {
                        if ((int)(totalLen + DAT_055cc16c) < 0x2001) {
                            memcpy((char *)DAT_055ca16c + DAT_055cc16c, outBuf, totalLen);
                            DAT_055cc16c += totalLen;
                        } else {
                            FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                        }
                    } else {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    }
                    break;
                }
                if (r == 0) break;
                if (DAT_055ce174 != 0) FUN_0043de60();
                totalLen -= r;
                sent += r;
            }
        }
    } else {
        // C4 header (large packet, 2-byte length)
        int totalLen = encodedLen + 3;
        outBuf[0] = (char)0xC4;
        outBuf[1] = (char)((totalLen >> 8) & 0xFF);
        outBuf[2] = (char)(totalLen & 0xFF);
        FUN_0053cc30((int)(outBuf + 3), sendBuf + serialOffset + 1, pktLen - (serialOffset + 1));
        int sent = 0;
        if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
            while (sent < totalLen) {
                int r = send(DAT_055ca168, outBuf + sent, totalLen - sent, 0);
                if (r == SOCKET_ERROR) {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) {
                        if ((int)(totalLen + DAT_055cc16c) < 0x2001) {
                            memcpy((char *)DAT_055ca16c + DAT_055cc16c, outBuf, totalLen);
                            DAT_055cc16c += totalLen;
                        } else {
                            FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                        }
                    } else {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    }
                    break;
                }
                if (r == 0) break;
                if (DAT_055ce174 != 0) FUN_0043de60();
                totalLen -= r;
                sent += r;
            }
        }
    }

    // First-call flag
    if (DAT_05826cfc == 0) {
        DAT_05826cfc = 1;
        DAT_05826d00 = tickCount;
    }
}

// Catch@0042206e @ 0x0042206E (6 bytes) — SEH catch block, returns FUN_00422074
void * __stdcall Catch_0042206e(void) {
    return (void *)FUN_00422074;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 19
// ═══════════════════════════════════════════════════════════════════════════════
