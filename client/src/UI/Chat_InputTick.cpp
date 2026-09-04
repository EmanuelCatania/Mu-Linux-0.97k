// Chat_InputTick.cpp — FUN_004b14f0 @ 0x004b14f0
// Per-frame chat input + hotkey + character-preview tick.
// Called from Game_MainLoop every frame.
//
// Responsibilities:
//   1. NPC/entity name tab-complete hover (stride-0x46 entity table)
//   2. Chat history navigation (PageUp/Down vtable scroll; Up/Down arrow ring buffer)
//   3. Class-tab buttons (4 tabs) — char preview select + 3-byte packet send
//   4. Hotkey assignment grid (chardata+0xd7, 20 slots, keys 1-9)
//   5. Hotkey trigger via numpad/number 1-9 keys → FUN_004b0e80()
//   6. Chat input for 9 channels (FUN_00494520 IME → validate → XOR-encode → send)
//   7. Whisper-target channel (DAT_07e108c8) — same pipeline as above
//   8. 'B' key toggle (DAT_07eaa134 / DAT_07eaa150 byte 2)
//   9. 'R' key toggle (chardata XOR-encode/decode cycle)
//
// Anti-tamper artefacts removed:
//   • Phantom stack params (in_stack_0x28..0x10b20) — SEH obfuscation, ignored
//   • XOR key double-init (forward+reverse) — compiler artifact, same result
//   • HashTable ops (FUN_00403f80/FUN_00404280/FUN_00404330/FUN_00423710) — ref-count noise
//   • 70+ unreachable blocks stripped
//
// Packet format reminder: [0xC1][len][opcode][sub][payload…]
// All chat packets XOR-encrypted with same 32-byte key as login.

#include "stdafx.h"
#pragma warning(disable: 4102)
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

// Inventory grid base — used by section 11 (Q/W/E quick-use sound feedback).
// Defined in src/Render/HUD_Pass3.cpp.
extern "C" BYTE OffsetInventoryItems[];

// ---------------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------------

// XOR key (32 bytes) — same as login/logout key
static const BYTE s_xorKey[32] = {
    0xe7, 0x6d, 0x3a, 0x89, 0xbc, 0xb2, 0x9f, 0x73,
    0x23, 0xa8, 0xfe, 0xb6, 0x49, 0x5d, 0x39, 0x5d,
    0x8a, 0xcb, 0x63, 0x8d, 0xea, 0x7d, 0x2b, 0x5f,
    0xc3, 0xb1, 0xe9, 0x83, 0x29, 0x51, 0xe8, 0x56
};

// Send a packet to the server.  Applies the XOR cipher (offset by hdr_skip bytes
// to skip the C1/len header), encodes via FUN_0053cc30, then send()s.
// If WSAEWOULDBLOCK, queues into DAT_055cc16c / 0x55ca16c overflow buffer.
static void Chat_SendPacket(BYTE *pkt, int len, int hdr_skip = 0)
{
    // XOR payload bytes starting at hdr_skip
    for (int i = hdr_skip; i < len; ++i)
        pkt[i] ^= s_xorKey[i & 0x1f];

    FUN_0053cc30(0, pkt, len);

    if (DAT_055ca168 == 0xffffffff)
        return;

    int sent = 0;
    int rem  = len;
    do {
        int n = send(DAT_055ca168, (char *)pkt + sent, rem, 0);
        if (n == -1) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                if ((int)(DAT_055cc16c + len) < 0x2001) {
                    // BUG-FIX 2026-05-03: was `(BYTE*)0x055ca16c + 4` — literal
                    // source-binary address (unmapped in our build → AV on first
                    // WSAEWOULDBLOCK retry). The other 5 sites of this same
                    // pattern (Game_*Tick, Party, Player_InputTick) all use the
                    // DAT_055ca16c macro (= DAT_055ca160 + 0xC). Match them.
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, len);
                    DAT_055cc16c += len;
                } else {
                    Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                }
            } else {
                Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
            }
            break;
        }
        if (n == 0) break;
        if (DAT_055ce174 != 0)
            FUN_0043de60();
        sent += n;
        rem  -= n;
    } while (rem > 0);
}

// Send a raw 3-byte packet (no XOR, no encode — these class-select packets are
// pre-encoded in the original code via FUN_0053cc30 before this call site).
static void SendRaw3(BYTE b0, BYTE b1, BYTE b2)
{
    BYTE pkt[3] = { b0, b1, b2 };
    if (DAT_055ca168 == 0xffffffff) return;
    int rem = 3, off = 0;
    do {
        int n = send(DAT_055ca168, (char *)pkt + off, rem, 0);
        if (n == -1) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                if ((int)(DAT_055cc16c + 3) < 0x2001) {
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, 3);  // BUG-FIX 2026-05-03: was literal 0x055ca16c
                    DAT_055cc16c += 3;
                } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
            } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
            break;
        }
        if (n == 0) break;
        if (DAT_055ce174) FUN_0043de60();
        off += n; rem -= n;
    } while (rem > 0);
}

// Resolves explicit target commands used by Trade and Guild.  Party has its own
// original interaction below: it does not accept a target name.
static char* Chat_FindPlayerInteractionTarget(const char* argument)
{
    char* entity = nullptr;
    const char* requestedName = argument ? argument : "";
    while (*requestedName == ' ')
        ++requestedName;

    if (*requestedName != '\0' && DAT_07abf5d0) {
        char* candidate = (char*)(uintptr_t)DAT_07abf5d0;
        for (int i = 0; i < 400; ++i, candidate += 916) {
            if (candidate[0] && *(short*)(candidate + 2) == 390 &&
                candidate[132] == 1 &&
                _stricmp(candidate + 449, requestedName) == 0) {
                entity = candidate;
                break;
            }
        }
    } else if (SelectedCharacter >= 0 && SelectedCharacter < 400 && DAT_07abf5d0) {
        char* candidate = (char*)(uintptr_t)DAT_07abf5d0 + SelectedCharacter * 916;
        if (candidate[0] && *(short*)(candidate + 2) == 390 && candidate[132] == 1)
            entity = candidate;
    }

    if (entity && entity != DAT_07abf5d8) {
        return entity;
    }
    return nullptr;
}

static bool Chat_TrySendTargetRequest(const char* text, const char* command, BYTE head,
                                      bool plainC1)
{
    const size_t commandLen = strlen(command);
    if (!text || _strnicmp(text, command, commandLen) != 0 ||
        (text[commandLen] != '\0' && text[commandLen] != ' '))
        return false;

    char* entity = Chat_FindPlayerInteractionTarget(text + commandLen);
    if (entity) {
        const WORD key = *(WORD*)(entity + 476);
        BYTE packet[5] = { 0xC1, 0x05, head, (BYTE)(key >> 8), (BYTE)key };
        // El frame lo decide `Data\Hack\HackPacketCheck.txt` (columna Encrypt):
        //   0x40 party y 0x36 trade → Encrypt=1 → C3
        //   0x50 guild             → Encrypt=0 → C1 plano
        // Mandarlo con el frame equivocado = "Packet encryption error" y
        // CloseClient del lado del server.
        if (plainC1)
            Net_SendC1Packet(packet, sizeof(packet));
        else
            Net_SendSmallPacket(packet, sizeof(packet));
    }
    return true; // Native command: never forward it as an unknown text command.
}

// Original Party request path (Chat command parser, before its generic slash
// command branch).  It accepts only the exact native command, requires the
// local player to lead any existing party, and chooses either the selected
// player or an adjacent, facing player.  The packet payload remains the
// viewport key required by MuEmu's PMSG_PARTY_REQUEST_RECV.
static bool Chat_TrySendPartyRequest(const char* text)
{
    if (!text || _stricmp(text, "/party") != 0)
        return false;

    char* const hero = DAT_07abf5d8;
    char* const entities = (char*)(uintptr_t)DAT_07abf5d0;
    if (!hero || !entities)
        return true;

    // IDA: PartyNumber > 0 && strcmp(Party[0].name, Hero->name).
    // Only the first listed member may initiate another invitation.
    if (PartyNumber > 0 && strcmp((const char*)Party, hero + 449) != 0) {
        UIChatLogWindow_AddText(nullptr, GlobalText[257], 1);
        return true;
    }

    auto IsPlayerTarget = [hero](char* entity) {
        return entity && entity != hero && entity[0] && entity[132] == 1 &&
               (*(short*)(entity + 2) == 390 || entity[847] != 0);
    };
    auto IsAdjacent = [hero](const char* entity) {
        return abs(*(const int*)(entity + 904) - *(const int*)(hero + 904)) <= 1 &&
               abs(*(const int*)(entity + 908) - *(const int*)(hero + 908)) <= 1;
    };

    char* target = nullptr;
    if (SelectedCharacter >= 0 && SelectedCharacter < 400) {
        char* candidate = entities + SelectedCharacter * 916;
        if (IsPlayerTarget(candidate) && IsAdjacent(candidate))
            target = candidate;
    } else {
        const int heroDirection = ((int)((*(float*)(hero + 36) + 22.5f) *
                                         0.022222223f + 1.0f)) & 7;
        for (int i = 0; i < 400; ++i) {
            char* candidate = entities + i * 916;
            if (!IsPlayerTarget(candidate) || !IsAdjacent(candidate))
                continue;

            const int targetDirection = ((int)((*(float*)(candidate + 36) + 22.5f) *
                                                 0.022222223f + 1.0f)) & 7;
            if (abs(targetDirection - heroDirection) == 4) {
                target = candidate;
                break;
            }
        }
    }

    if (!target)
        return true;

    const WORD key = *(WORD*)(target + 476);
    PartyKey = key;
    const BYTE packet[5] = { 0xC1, 0x05, 0x40, (BYTE)(key >> 8), (BYTE)key };
    Net_SendSmallPacket(packet, sizeof(packet));

    const int targetIndex = FUN_0045ac80((int)(short)key);
    if (targetIndex >= 0 && targetIndex < 400) {
        char message[300] = {};
        sprintf(message, GlobalText[476], entities + targetIndex * 916 + 449);
        UIChatLogWindow_AddText(nullptr, message, 1);
    }
    return true;
}

static bool Chat_TrySendTradeRequest(const char* text)
{
    // PMSG_TRADE_REQUEST_RECV in MuEmu Trade.h: C1:36 + viewport key.
    return Chat_TrySendTargetRequest(text, "/trade", 0x36, /*plainC1=*/false);
}

static bool Chat_TrySendGuildRequest(const char* text)
{
    // PMSG_GUILD_REQUEST_RECV in MuEmu Guild.h: C1:50 + viewport key.
    // The server itself validates that the sender is a guild master.
    return Chat_TrySendTargetRequest(text, "/guild", 0x50, /*plainC1=*/true);
}

// 2026-05-04 — Public helper: send a chat line typed into InputText[0]
// (DAT_07db8710 slot 0) by the WM_CHAR handler. Mirrors the inline
// build/send logic at Chat_InputTick lines 580-614 (channel-0 path)
// without depending on the per-frame FUN_00494520 polling. Packet:
//   [0xC1][len][..XOR-encoded text..]
// `text` must be NUL-terminated, length capped at 0x3c chars (matching
// the IDA original's truncation).
extern "C" void Chat_SendChatLine(const char* text)
{
    if (!text || !*text) return;
    if (Chat_TrySendPartyRequest(text)) return;


    if (Chat_TrySendTradeRequest(text)) return;
    if (Chat_TrySendGuildRequest(text)) return;

    // General chat is exactly SendChat @ 0x004C1B90.  Keep the whisper
    // builder below as a separate branch: IDA places it inline in this UI
    // tick and it uses the recipient name in the C1:02 header.
    if (((const char*)&DAT_07db8810)[0] == '\0') {
        SendChat((char*)text);
        return;
    }

    // This is intentionally not a client-side command whitelist.  MuEmu's
    // CommandManager parses slash commands from CGChatRecv, which keeps this
    // client compatible with every command enabled in Command.txt (including
    // future server-side additions) without another client patch.
    size_t tlen = strlen(text);
    if (tlen > 0x3c) tlen = 0x3c;

    // ── PMSG_CHAT_RECV (server MuEmu Protocol.h, header C1:00) ───────────────
    //   +0..2   PBMSG_HEAD  { C1, size, headcode=0x00 }
    //   +3..12  char name[10]
    //   +13..72 char message[60]
    // IDA, SendChat @ 0x004C1B90: el C1 se construye con tamaño variable.
    // Primero agrega los diez bytes del nombre y después `strlen(texto)+1`;
    // para una entrada de 59+ caracteres limita ese último bloque a 60.
    // No se debe rellenar a sizeof(PMSG_CHAT_RECV): el ejecutable original
    // cifra y envía únicamente los bytes que realmente construyó.
    const size_t msgBytes = (tlen + 1 >= 0x3c) ? 0x3c : (tlen + 1);
    const int pktLen = 3 + 10 + (int)msgBytes;

    BYTE pkt[0x410];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;
    pkt[1] = (BYTE)pktLen;

    // ── WHISPER vs CHAT ──────────────────────────────────────────────────────
    // `InputText[1]` (= DAT_07db8810, g_TextBuf base + 1*0x100) es el campo de
    // NOMBRE del destinatario. Confirmado en IDA `MoveChat` @0x4821A0, que al
    // hacer click derecho sobre la burbuja de un personaje hace:
    //     strcpy(InputText[1], <nombre del hablante>);
    //     InputLength[1] = strlen(InputText[1]);
    //     PlayBuffer(25, 0, 0);
    //
    // Formato del wire (server MuEmu Protocol.h) — las dos structs son
    // idénticas salvo el headcode y QUIÉN va en name[10]:
    //   C1:00 PMSG_CHAT_RECV          name = el PROPIO personaje
    //         CGChatRecv: `if (strcmp(name, lpObj->Name) != 0) return;`
    //   C1:02 PMSG_CHAT_WHISPER_RECV  name = el DESTINATARIO
    //         CGChatWhisperRecv: no exige que sea el propio.
    //
    // NOTA DE FIDELIDAD: el bloque de envío de whisper del IDA está inline en
    // Chat_InputTick (~0x4B1760-0x4B18C0), entrelazado con las llamadas
    // anti-tamper de hash-table, y NO llegué a transcribirlo completo. El
    // formato del paquete sí es autoritativo (struct del server) y reusa el
    // mismo path C1 + chain-XOR que el chat normal, que ya está verificado en
    // runtime. Lo único inferido es la condición de rama (campo de nombre no
    // vacío ⇒ whisper), que es coherente con la semántica de InputText[1].
    const char* whisperTarget = (const char*)&DAT_07db8810;
    if (whisperTarget[0] != '\0') {
        pkt[2] = 0x02;                        // headcode = whisper
        memcpy(pkt + 3, whisperTarget, 10);   // name[10] = DESTINATARIO
    } else {
        pkt[2] = 0x00;                        // headcode = chat normal
        // BUG-FIX 2026-07-19 (nuestros mensajes no llegaban): el campo name[10]
        // quedaba en CEROS y el server los descartaba en silencio.
        if (DAT_07abf5d8) {
            memcpy(pkt + 3, (const char*)DAT_07abf5d8 + 0x1C1, 10);
        }
    }

    memcpy(pkt + 13, text, msgBytes);

    // BUG-FIX: era un XOR simple `pkt[i] ^= key[i]`. El server (XorData en
    // PacketManager.cpp) reversa el CHAIN-XOR, así que el cliente debe usar
    // `pkt[i] ^= pkt[i-1] ^ key[i]` — igual que el resto de los C1 (movimiento,
    // enter-world, char-select). Con el XOR simple el texto llegaba ilegible.
    for (int xi = 3; xi < pktLen; ++xi)
        pkt[xi] ^= pkt[xi - 1] ^ s_xorKey[xi & 0x1f];

    if (DAT_055ca168 == 0xffffffff) return;
    int rem = pktLen, off = 0;
    do {
        int n = send(DAT_055ca168, (char *)pkt + off, rem, 0);
        if (n == -1) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                if ((int)(DAT_055cc16c + pktLen) < 0x2001) {
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, pktLen);
                    DAT_055cc16c += pktLen;
                } else Net_Disconnect((int)(uintptr_t)DAT_055ca160);
            } else Net_Disconnect((int)(uintptr_t)DAT_055ca160);
            break;
        }
        if (n == 0) break;
        if (DAT_055ce174) FUN_0043de60();
        off += n; rem -= n;
    } while (rem > 0);

    // Update last-sent-cmp buffer + reset rate-limit (matches IDA).
    memcpy(&DAT_05826adc[0], text, tlen + 1);
    DAT_05826d08 = 0x46;
}

// Sequence called on every class-tab click.
// After the first 3-byte packet, does the hash-table check then optionally
// calls FUN_004cba60 or FUN_004e3d60 + sends a second 3-byte packet.
// pkt0/pkt1/pkt2 = first 3-byte packet bytes.
// pkt_b0/b1/b2 = second 3-byte packet (FUN_004e3d60 branch).
// pkt_c0/c1/c2 = second 3-byte packet (DAT_07eaa165 branch).
static void ClassTab_HandleClick(BYTE p0, BYTE p1, BYTE p2,
                                 BYTE q0, BYTE q1, BYTE q2,
                                 BYTE r0, BYTE r1, BYTE r2)
{
    SendRaw3(p0, p1, p2);

    DAT_07eaa0d0 = 0xffffffff;
    DAT_07eaa114 = '\x01';

    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b);
    char bVar4 = DAT_07eaa11b;
    FUN_00404040(&DAT_055c9bc8, (char *)&DAT_07eaa11b);

    if (bVar4 == 0) {
        if (DAT_07eaa119 == '\0') {
            if (DAT_07eaa11a == '\0') {
                DAT_07eaa117 = '\0';
                DAT_07eaa116 = '\0';
                FUN_004cba60();
            } else {
                char cVar3 = FUN_004e3d60(&DAT_07ea9848, 8, 4);
                if ((cVar3 == '\0') || (0 < (int)DAT_07e91388)) {
                    FUN_00480620((const char*)&lpDefault_00583d88,&DAT_07d55410, 2);
                } else {
                    SendRaw3(q0, q1, q2);
                }
            }
        } else if (DAT_07eaa165 == '\0') {
            DAT_07eaa117 = '\0';
            FUN_004cba60();
            if (0 < (int)DAT_07e91388)
                FUN_004cd3b0();
            SendRaw3(r0, r1, r2);
        }
    } else {
        // Hash-table encode path (obfuscation) + encoded packet send
        rand();
        FUN_00422df0(&DAT_055c9bc8, &DAT_05826ceb);
        DAT_05826ceb++;
        FUN_00404040(&DAT_055c9bc8, &DAT_05826ceb);
        // Variable-length encode+send path (obfuscation noise — exact packet bytes
        // come from FUN_0053cc30 internal state; not recoverable without full trace)
        (void)FUN_0053cc30(0, (BYTE *)&DAT_05826ceb, 0);
    }
}

// ---------------------------------------------------------------------------
// FUN_004b14f0 — Chat_InputTick
// ---------------------------------------------------------------------------
void __cdecl FUN_004b14f0(void)
{
    int mouseX = (int)DAT_083a427c;
    int mouseY = (int)DAT_083a4278;

    // ── 1. Entity tab-complete hover loop ──────────────────────────────────
    // Iterates up to 0x4e entity slots, stride 0x46.
    // Each slot: first byte = active, name at DAT_07df9380+slot*0x46,
    //            type at DAT_07df948c+slot*0x46, posX at DAT_07df9494+slot*0x118.
    // Compares entity's name at entity+0x1c1 against the table.
    // On match: copies table name to DAT_07db8810, updates DAT_07d780ac.
    {
        int  rowY  = 0;
        char *base = (char *)&DAT_07abf5d8;   // local player entity ptr (as char*)
        for (int slot = 0; slot < 0x4e; ++slot)
        {
            // Con los aliases DAT_07df938b/948c/9494 apuntando al buffer real,
            // &DAT_07df948c y &DAT_07df9494 son int*, asi que la aritmetica se
            // hace casteando a char* antes de sumar el stride en bytes.
            //
            // 2026-09-03 FIX: tres de los cuatro punteros usaban `slot * 0x46`
            // sobre char*, o sea un paso de 0x46 BYTES.  El stride real de la
            // tabla es **0x118** -- lo dice su propia declaracion
            // (`char DAT_07df9380[0x77 * 0x118]`) y lo usan todos los accesos de
            // Chat.cpp.  El 0x46 viene de los sitios donde el indice se aplica a
            // un `int*` (`(&DAT_07df948c)[i * 0x46]`, y 0x46*4 == 0x118): al
            // copiar el multiplicador a un contexto de bytes el paso quedaba 4x
            // corto y el bucle releia las primeras ~20 filas en vez de recorrer
            // las 78.  Misma familia que el bug del pool de clima, pero sin
            // salirse del buffer: no corrompe, devuelve la fila equivocada.
            const char *tableName = DAT_07df9380 + slot * 0x118;
            const char *activePtr = (const char *)&DAT_07df938b + slot * 0x118;
            const char *typePtr   = (const char *)&DAT_07df948c + slot * 0x118;
            const int  *posXPtr   = (const int *)((const char *)&DAT_07df9494 + slot * 0x118);
            int         posX_scaled = (int)((unsigned int)(*posXPtr * 0x280) / DAT_0056156c);

            if (*activePtr != '\0' &&
                (DAT_00559bf1 != '\0' || *typePtr != 3) &&
                (-1 < mouseX) &&
                (mouseX < posX_scaled) &&
                ((int)rowY <= mouseY) && (mouseY < (int)(rowY + 0xd)) &&
                (DAT_00559c84 != '\0') &&
                (*(char *)(DAT_07abf5d8 + 0x34e) != '\0'))
            {
                // Compare entity name at +0x1c1 against table name
                const char *entityName = DAT_07abf5d8 + 0x1c1;
                if (strcmp(tableName, entityName) != 0 && DAT_083a42d0 != '\0')
                {
                    DAT_083a42d0 = '\0';
                    size_t nameLen = strlen(tableName);
                    memcpy(DAT_07db8810, tableName, nameLen + 1);
                    // Update input length counter
                    DAT_07d780ac = (DWORD)strlen((const char *)DAT_07db8810);
                    FUN_00404bc0(0x19, 0, 0);
                    // Re-read (loop continues from updated mouseX)
                    mouseX = (int)DAT_083a427c;
                }
            }
            rowY += 0xd;
        }
    }

    // ── 2. PageUp / PageDown (vtable scroll) ───────────────────────────────
    {
        SHORT sv = GetAsyncKeyState(0x21); // VK_PRIOR = PageUp
        if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
            if (DAT_07e11970 == 0) {
                DAT_07e11970 = 1;
                // Same structural split as newer clean sources:
                // guild list open -> guild widget, else main chat widget.
                DWORD* obj = (DWORD*)(GuildOpened ? DAT_055c9ff4
                                                  : DAT_055c9ff0);
                if (obj && *(int*)obj) {
                    void** vt = (void**)*obj;
                    typedef int (__fastcall *FnScroll)(DWORD*, int, int);
                    int rows = (int)obj[35];     // visible row count
                    if (rows <= 0) rows = 6;
                    ((FnScroll)vt[12])(obj, 0, -rows);
                }
            }
        } else {
            DAT_07e11970 = 0;
        }

        sv = GetAsyncKeyState(0x22); // VK_NEXT = PageDown
        if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
            if (DAT_07e11974 == 0) {
                DAT_07e11974 = 1;
                DWORD* obj = (DWORD*)(GuildOpened ? DAT_055c9ff4
                                                  : DAT_055c9ff0);
                if (obj && *(int*)obj) {
                    void** vt = (void**)*obj;
                    typedef int (__fastcall *FnScroll)(DWORD*, int, int);
                    int rows = (int)obj[35];
                    if (rows <= 0) rows = 6;
                    ((FnScroll)vt[12])(obj, 0, rows);
                }
            }
        } else {
            DAT_07e11974 = 0;
        }
    }

    // ── 3. Up/Down arrows — chat history ring buffer ────────────────────────
    // Ring buffer: 5 slots × 0x100 bytes each at DAT_07e113e4.
    // Index in DAT_00559cc4 (0..4). Up = older entry (decrement), Down = newer (increment).
    {
        SHORT sv = GetAsyncKeyState(0x26); // VK_UP
        if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
            if (DAT_07e11984 == 0 && 0 < (int)DAT_00559cc4) {
                DAT_07e11984 = 1;
                // Save current input to history[current_index]
                char *histSlot = (char *)&DAT_07e113e4 + DAT_00559cc4 * 0x100;
                size_t curLen = strlen((const char *)DAT_07db8810);
                memcpy(histSlot, DAT_07db8810, curLen + 1);
                DAT_00559cc4--;
                // Load history[new_index] into input buffer
                const char *newHist = (const char *)&DAT_07e113e4 + DAT_00559cc4 * 0x100;
                size_t newLen = strlen(newHist);
                memset(DAT_07db8810, 0, 0x100);
                memcpy(DAT_07db8810, newHist, newLen + 1);
                DAT_07d780ac = (DWORD)strlen((const char *)DAT_07db8810);
                FUN_00404bc0(0x19, 0, 0);
            }
        } else {
            DAT_07e11984 = 0;
        }

        sv = GetAsyncKeyState(0x28); // VK_DOWN
        if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
            if (DAT_07e1198c == 0 && (int)DAT_00559cc4 < 4) {
                DAT_07e1198c = 1;
                // Save current input to history[current_index]
                char *histSlot = (char *)&DAT_07e113e4 + DAT_00559cc4 * 0x100;
                size_t curLen = strlen((const char *)DAT_07db8810);
                memcpy(histSlot, DAT_07db8810, curLen + 1);
                DAT_00559cc4++;
                // Load history[new_index] into input buffer
                const char *newHist = (const char *)&DAT_07e113e4 + DAT_00559cc4 * 0x100;
                size_t newLen = strlen(newHist);
                memset(DAT_07db8810, 0, 0x100);
                memcpy(DAT_07db8810, newHist, newLen + 1);
                DAT_07d780ac = (DWORD)strlen((const char *)DAT_07db8810);
                FUN_00404bc0(0x19, 0, 0);
            }
        } else {
            DAT_07e1198c = 0;
        }
    }

    // ── 4. Class-tab buttons + hot gate ─────────────────────────────────────
    // Guard condition: all of these must be true to process tab/hotkey/chat input:
    //   bVar4(hash)==0, DAT_07eaa124==0, DAT_07e11d70==0,
    //   DAT_083a7c24 not 0x7e/0x98, DAT_07eaa128==0,
    //   *(DAT_00583d8c+0x1c87f)==0, DAT_07eaa130==0
    {
        FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b);
        char bVar4 = DAT_07eaa11b;
        FUN_00404040(&DAT_055c9bc8, (char *)&DAT_07eaa11b);

        if ((bVar4 == 0) &&
            (DAT_07eaa124 == '\0') &&
            (DAT_07e11d70 == '\0') &&
            (DAT_083a7c24 != 0x7e) && (DAT_083a7c24 != 0x98) &&
            (DAT_07eaa128 == 0) &&
            (*(char *)((uintptr_t)DAT_00583d8c + 0x1c87f) == '\0') &&
            (DAT_07eaa130 == '\0'))
        {
            // Tab 0 — X=[0x246..0x279], Y=[0x1cb..0x1dc]: select first class group
            if ((0x245 < mouseX) && (mouseX < 0x27a) &&
                (0x1ca < mouseY) && (mouseY < 0x1dd) &&
                IsClickPushed())
            {
                DAT_083a4124 = '\0';
                DAT_07eaa115 = '\0';
                if (DAT_07eaa114 == '\0') {
                    // Packet: class-select tab 0 (3 bytes, pre-encoded)
                    ClassTab_HandleClick(
                        0xC1, 0x03, 0xF3,  // send pkt 0 (class-select tab 0)
                        0xC1, 0x03, 0xF4,  // second pkt branch A
                        0xC1, 0x03, 0xF5); // second pkt branch B
                } else {
                    DAT_07eaa114 = '\0';
                }
                FUN_00404bc0(0x19, 0, 0);
                FUN_00404bc0(0x1c, 0, 0);
            }

            // Tab 1 — X=[0x15c..0x173], Y=[0x1c3..0x1db]
            if ((0x15b < mouseX) && (mouseX < 0x174) &&
                (0x1c3 < mouseY) && (mouseY < 0x1dc) &&
                IsClickPushed())
            {
                DAT_083a4124 = '\0';
                DAT_07eaa114 = '\0';
                if (DAT_07eaa115 == '\0') {
                    DAT_07eaa0e0 = 0;
                    ClassTab_HandleClick(
                        0xC1, 0x03, 0xF6,
                        0xC1, 0x03, 0xF7,
                        0xC1, 0x03, 0xF8);
                } else {
                    DAT_07eaa115 = '\0';
                }
                FUN_00404bc0(0x19, 0, 0);
                FUN_00404bc0(0x1c, 0, 0);
            }

            // Tab 2 — X=[0x17b..0x192], Y=[0x1c3..0x1db]
            if ((0x17a < mouseX) && (mouseX < 0x193) &&
                (0x1c3 < mouseY) && (mouseY < 0x1dc) &&
                IsClickPushed())
            {
                DAT_083a4124 = '\0';
                if (DAT_07eaa116 == '\0') {
                    DAT_07eaa116 = '\x01';
                    ClassTab_HandleClick(
                        0xC1, 0x03, 0xF9,
                        0xC1, 0x03, 0xFA,
                        0xC1, 0x03, 0xFB);
                }
                FUN_00404bc0(0x19, 0, 0);
                FUN_00404bc0(0x1c, 0, 0);
            }

            // ── 5. Hotkey assignment grid ────────────────────────────────────
            // chardata+0x56 != 0: item grid is visible.
            // Items: 20 slots at chardata+0x57+slot (one byte each).
            // Hotkey area: X=[0x12f..0x150], Y=[0x1bb..0x1e0] — click → open grid.
            // Item cells: stride 0x20, hit Y=[0x171..0x19c].
            // Keys 1-9: write to chardata+0xd7+slot (value 1..9); clear old.
            if (*(char *)((char *)DAT_07cf1ff4 + 0x56) != '\0')
            {
                // Outer click area (open toggle)
                if ((0x12f < mouseX) && (mouseX < 0x150) &&
                    (0x1bb < mouseY) && (mouseY < 0x1e0))
                {
                    DAT_0055a3e4 = (DWORD)*(BYTE *)(DAT_07abf5d8 + 0x391);
                    DAT_07ea840c = 0x140;
                    DAT_07ea8408 = 0x1bc;
                    DAT_07d78094 = 1;
                    DAT_07e11d28 = 0;
                    if (IsClickPushed()) {
                        DAT_07db870c = (DAT_07db870c == '\0') ? '\x01' : '\0';
                        DAT_083a4124 = '\0';
                        FUN_00404bc0(0x19, 0, 0);
                    }
                }

                // Item cell grid (visible when DAT_07db870c != 0)
                if (DAT_07db870c != '\0')
                {
                    DWORD numSlots = (DWORD)*(BYTE *)((char *)DAT_07cf1ff4 + 0x56);
                    int   cellOff  = 0;
                    for (DWORD uVar14 = 0; (int)uVar14 < 0x14; ++uVar14)
                    {
                        if (*(char *)((char *)DAT_07cf1ff4 + 0x57 + uVar14) != '\0')
                        {
                            int cellX0 = cellOff - (int)(numSlots * 0x20) / 2;
                            int cellX1 = cellX0 + 0x20;
                            if (((cellX0 + 0x140 <= mouseX) && (mouseX < cellX1 + 0x140)) &&
                                (0x171 < mouseY) && (mouseY < 0x19c))
                            {
                                DAT_07ea840c = cellX0 + 0x150;
                                DAT_07ea8408 = 0x172;
                                DAT_07d78094 = 1;
                                DAT_0055a3e4 = uVar14;
                                if (IsClickPushed()) {
                                    DAT_083a4124 = '\0';
                                    *(char *)((char *)DAT_07abf5d8 + 0x391) = (char)uVar14;
                                    DAT_07db870c = '\0';
                                    FUN_00404bc0(0x19, 0, 0);
                                    DAT_07e11d28 = 0;
                                    DAT_00559bec = 6;
                                }
                                // Hotkey assignment: keys 1-9 via GetAsyncKeyState
                                // extraout_AH from GetAsyncKeyState indicates key was pressed this frame
                                for (int key = 1; key <= 9; ++key)
                                {
                                    SHORT sv2 = GetAsyncKeyState(0x30 + key); // VK '1'..'9'
                                    if ((char)((unsigned short)sv2 >> 8) != '\0')
                                    {
                                        // Assign hotkey slot: clear previous mapping, set new
                                        FUN_00423040(&DAT_055c9bc8, DAT_07cf1ffc);
                                        int charRow = (int)DAT_005616ac;
                                        for (int j = 0; j < 0x14; ++j) {
                                            char *slot_ptr = (char *)DAT_07cf1ff4 + charRow * 0x40 + 0xd7 + j;
                                            if (*slot_ptr == (char)key) {
                                                *slot_ptr = (char)0xff;
                                                charRow = (int)DAT_005616ac;
                                            }
                                        }
                                        *((char *)DAT_07cf1ff4 + charRow * 0x40 + 0xd7 + (int)uVar14) = (char)key;
                                        FUN_0043d1d0(&DAT_055c9bc8, (void *)DAT_07cf1ffc);
                                    }
                                }
                                break;
                            }
                            cellOff += 0x20;
                        }
                    }
                }
            }

            // ── 6. Numpad / number 1-9 hotkey trigger ────────────────────────
            // Only when not in a chat UI mode (DAT_00559c84==0, DAT_07e11d71==0)
            // and number-key VK_NUMPAD0 (0x60) not held.
            if ((DAT_00559c84 == '\0') && (DAT_07e11d71 == '\0'))
            {
                SHORT sv0 = GetAsyncKeyState(0x60); // VK_NUMPAD0
                if ((char)((unsigned short)sv0 >> 8) != (char)(-0x80))
                {
                    for (int i = 1; i <= 9; ++i) {
                        SHORT sv2 = GetAsyncKeyState(0x30 + i); // '1'..'9'
                        if ((char)((unsigned short)sv2 >> 8) != '\0')
                            FUN_004b0e80();
                    }
                    SHORT sv2 = GetAsyncKeyState(0x30); // '0'
                    if ((char)((unsigned short)sv2 >> 8) != '\0')
                        FUN_004b0e80();
                }
            }

            // ── 7. Chat input — 9 channels ───────────────────────────────────
            // Channels 0-8 each have a 0x100-byte input buffer at DAT_07e0ffc8+ch*0x100.
            // FUN_00494520 reads a key press into the buffer, returns ch != '\0' if Enter.
            // FUN_00513440 validates the text (profanity/length); '\0' = ok.
            // Rate-limit: DAT_05826d08 starts at 0x46, counts down each frame.
            //             If > 0x32, reject (too fast). Resets to 0x46 on send.
            // Duplicate check: compare buffer vs DAT_05826adc (last sent).
            // Command parsing (prefix '/'):
            //   /whisper → DAT_07e11dac=1, FUN_00480620 with DAT_07d3d608
            //   /pvp     → DAT_07e11dac=0, FUN_00480620 with DAT_07d3d734
            //   GM cmd   → if +0x2fd!=0 and strncmp to DAT_07d3cdd4 → discard
            // Normal send:
            //   Build C1 packet: [0xC1][len][opcode][sub][player_name][chat_text][0x00]
            //   XOR-encrypt payload from byte offset 3 onward, FUN_0053cc30 + send.
            if (DAT_07e11d7c == 0)
            {
                SHORT svEnter = GetAsyncKeyState(0x0D); // VK_RETURN
                if ((char)((unsigned short)svEnter >> 8) != '\0')
                {
                    for (int ch = 0; ch < 9; ++ch)
                    {
                        BYTE *chBuf = (BYTE *)&DAT_07e0ffc8 + ch * 0x100;
                        DWORD keyVal = (DWORD)((unsigned short)GetAsyncKeyState(0x0D) >> 8);
                        DWORD uVar9  = FUN_00494520((void *)keyVal, chBuf, '\x01');
                        if ((char)uVar9 != '\0')
                            continue;  // not Enter for this channel

                        // Validate input
                        if (((*(short *)(DAT_07abf5d8 + 0x2b8) != 0x332) &&
                             (*(short *)(DAT_07abf5d8 + 0x2b8) != 0x333)) ||
                             (*(char *)(DAT_07abf5d8 + 0x34e) != '\0'))
                            FUN_00497c70();

                        if ((char)FUN_00513440((char *)chBuf) != '\0')
                            continue;  // invalid text

                        // Rate-limit and duplicate check
                        if (DAT_05826d08 >= 0x33) {
                            DAT_07e11d7c = 100;
                            continue;
                        }

                        if (DAT_05826d08 > 0) {
                            if (strcmp((char *)chBuf, &DAT_05826adc[0]) == 0)
                                goto chat_done;
                        }

                        // Copy to last-sent buffer, reset rate-limit
                        {
                            size_t tlen = strlen((char *)chBuf);
                            if (tlen > 0x3c) tlen = 0x3c;
                            memcpy(&DAT_05826adc[0], chBuf, tlen + 1);
                            DAT_05826d08 = 0x46;
                        }

                        // Player name length for name-match check
                        {
                            int pnLen = (int)strlen(&DAT_005592dc);
                            int enLen = (int)strlen(DAT_07abf5d8 + 0x1c1);
                            // Try to find player name in entity name substring
                            // (tab-complete name match — sets up struct at DAT_005592d4/d8)
                            // Simplified: set name match refs
                        }

                        // Command parsing
                        if (*(char *)(DAT_07abf5d8 + 0x2fd) == '\0') {
                            if (chBuf[0] == '/') {
                                if (Chat_TrySendPartyRequest((const char*)chBuf)) {
                                    goto chat_done;
                                }
                                if (strlen(&DAT_07d3d284) > 0 &&
                                    strncmp((char *)chBuf, &DAT_07d3d284,
                                            strlen(&DAT_07d3d284)) == 0) {
                                    // /whisper command
                                    DAT_07e11dac = 1;
                                    FUN_00480620((const char*)&lpDefault_00583d88,&DAT_07d3d608, 1);
                                    goto chat_done;
                                }
                                if (strlen(&DAT_07d3d3b0) > 0 &&
                                    strncmp((char *)chBuf, &DAT_07d3d3b0,
                                            strlen(&DAT_07d3d3b0)) == 0) {
                                    // /pvp command
                                    DAT_07e11dac = 0;
                                    FUN_00480620((const char*)&lpDefault_00583d88,&DAT_07d3d734, 1);
                                    goto chat_done;
                                }
                            }
                        } else {
                            // GM mode: check GM command prefix
                            if (strlen(&DAT_07d3cdd4) > 0 &&
                                strncmp((char *)chBuf, &DAT_07d3cdd4,
                                        strlen(&DAT_07d3cdd4)) == 0) {
                                goto chat_done;
                            }
                        }

                        Chat_SendChatLine((const char*)chBuf);
                        chat_done:
                        DAT_07e11d7c = 100;
                    }
                }

                // ── 8. Whisper-target channel (channel 9) ────────────────────
                // Uses DAT_07e108c8 buffer (offset 0x900 from DAT_07e0ffc8).
                {
                    DWORD keyVal = (DWORD)((unsigned short)GetAsyncKeyState(0x0D) >> 8);
                    DWORD uVar9  = FUN_00494520((void *)keyVal, (BYTE *)&DAT_07e108c8, '\x01');
                    if ((char)uVar9 == '\0') {
                        // SendChat@004C1B90 never emits a C1:00 chat shorter
                        // than 14 bytes (the text terminator is part of it).
                        // This polling branch can be reached with its whisper
                        // buffer empty; its 13-byte frame makes MuEmu read the
                        // fixed PMSG_CHAT_RECV past the received packet.
                        if (DAT_07e108c8 == '\0')
                            goto whisper_done;
                        if (((*(short *)(DAT_07abf5d8 + 0x2b8) != 0x332) &&
                             (*(short *)(DAT_07abf5d8 + 0x2b8) != 0x333)) ||
                             (*(char *)(DAT_07abf5d8 + 0x34e) != '\0'))
                            FUN_00497c70();

                        if ((char)FUN_00513440(&DAT_07e108c8) == '\0') {
                            if (DAT_05826d08 < 0x33) {
                                bool bDupWhisper = (DAT_05826d08 > 0) &&
                                                   (strcmp(&DAT_07e108c8, &DAT_05826adc[0]) == 0);
                                if (!bDupWhisper) {
                                size_t tlen = strlen(&DAT_07e108c8);
                                if (tlen > 0x3c) tlen = 0x3c;
                                memcpy(&DAT_05826adc[0], &DAT_07e108c8, tlen + 1);
                                DAT_05826d08 = 0x46;

                                // Build and send whisper packet (same XOR+encode pattern)
                                {
                                    int pktLen = (int)(tlen + 0x0d);
                                    if ((DWORD)pktLen < 0x401) {
                                        BYTE pkt[0x410];
                                        memset(pkt, 0, sizeof(pkt));
                                        pkt[0] = 0xC1;
                                        pkt[1] = (BYTE)pktLen;
                                        memcpy(pkt + 0x0d, &DAT_07e108c8, tlen);
                                        for (int xi = 3; xi < pktLen; ++xi)
                                            pkt[xi] ^= s_xorKey[xi & 0x1f];
                                        FUN_0053cc30(0, pkt, pktLen);
                                        if (DAT_055ca168 != 0xffffffff) {
                                            int rem = pktLen, off = 0;
                                            do {
                                                int n = send(DAT_055ca168, (char *)pkt + off, rem, 0);
                                                if (n == -1) {
                                                    if (WSAGetLastError() == WSAEWOULDBLOCK) {
                                                        if ((int)(DAT_055cc16c + pktLen) < 0x2001) {
                                                            memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, pktLen);  // BUG-FIX 2026-05-03: was literal 0x055ca16c
                                                            DAT_055cc16c += pktLen;
                                                        } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                                                    } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                                                    break;
                                                }
                                                if (n == 0) break;
                                                if (DAT_055ce174) FUN_0043de60();
                                                off += n; rem -= n;
                                            } while (rem > 0);
                                        }
                                    }
                                }
                            } // end if (!bDupWhisper)
                        } else {
                            DAT_07e11d7c = 100;
                        }
                        }
                        whisper_done:;
                    }
                }
            }
        } // end main guard
    }

    // ── GATE: hotkeys de letra solo con el input de chat CERRADO ────────────
    // PORT FIEL de IDA Chat_InputTick @ 0x4B6575..0x4B65DB — son CINCO tests
    // encadenados, cada uno con `jnz` al epílogo (0x4BB807/0x4BB80D/0x4BB924/
    // 0x4BB928), no uno solo:
    //     8a 15 84 9c 55 00   mov  dl, byte_559C84    ; g_TextMode
    //     8a 15 71 1d e1 07   mov  dl, byte_7E11D71   ; g_IME_Mode
    //        ... test byte_7EAA11B                    ; TradeOpened
    //     a0 24 a1 ea 07      mov  al, byte_7EAA124   ; GuildCreatorOpened
    //     a0 70 1d e1 07      mov  al, byte_7E11D70   ; g_ChatMode
    // Todo lo que sigue (B @0x4BAD60, R, y las llamadas a sub_482BE0 de Q/W/E
    // en 0x4B666B/0x4B6CE8/0x4B739C/0x4B7A8D) está DESPUÉS de esos saltos.
    // (Las llamadas sub_43D8A0/sub_4041E0 intercaladas son ruido anti-tamper
    // de hash-table — omitidas per policy del proyecto.)
    //
    // BUG QUE ARREGLA (2026-07-19): en teclado español/latino `@` es AltGr+Q, y
    // `GetAsyncKeyState` ve la Q presionada → tipear `@` (o 'q'/'w'/'e', 'b',
    // 'r') en el chat disparaba el quick-use del hotbar y mandaba un
    // PMSG_ITEM_USE_RECV → el server cerraba la conexión.
    //
    // 1er intento porteó SOLO `g_ChatMode` y no alcanzó: al abrir el chat el
    // que se pone en 1 es **`g_TextMode` (DAT_00559c84)** — es el `InputEnable`
    // del log de WM_CHAR. Es también el flag que ya usaba `HUD_HotkeyTick`
    // (Player_InputTick.cpp), por eso las teclas de inventario SÍ quedaban
    // bloqueadas y estas no.
    if (DAT_00559c84 != 0) return;   // g_TextMode  (input de texto activo)
    if (DAT_07e11d71 != 0) return;   // g_IME_Mode  (composición DBCS/coreano)
    if (DAT_07eaa11b != 0) return;   // TradeOpened
    if (DAT_07eaa124 != 0) return;   // GuildCreatorOpened
    if (DAT_07e11d70 != 0) return;   // g_ChatMode

    // ── 9. 'B' key — toggle body rendering / battle mode ────────────────────
    // HashTable ops are obfuscation noise around the real toggle.
    {
        SHORT sv = GetAsyncKeyState(0x42); // 'B'
        if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
            if (DAT_07e119f4 == 0) {
                DAT_07e119f4 = 1;
                // HashTable ref-count noise (FUN_00404280/FUN_00403f80 etc.) — skipped
                // Core toggle:
                char cVar3 = DAT_07eaa118;
                if (cVar3 == '\0') {
                    // B key didn't toggle — early exit conditions
                    return;
                }
                if (DAT_07eaa132 == '\0') {
                    return;
                }
                if (DAT_07e91388 != 0) {
                    return;
                }
                if (DAT_07eaa150 == 0) {
                    DAT_07eaa134 = 1;
                    DAT_07eaa150 = 2;   // byte 2 of the DWORD
                    return;
                }
                DAT_07eaa134 = 0;
                DAT_07eaa150 = 0;
                return;
            }
        } else {
            DAT_07e119f4 = 0;
        }
    }

    // ── 10. 'R' key — chardata re-encode / refresh ──────────────────────────
    // Decodes, transforms, re-encodes 0x584-byte chardata block.
    // Transform: buf[i] = (DAT_00559050[i&0xf] ^ (buf[i]+0x47)) + 0x23, XOR next
    // Inverse:   buf[i] = (buf[i]+0x47 XOR ...) + 0x23 reversed
    // This implements the XOR-encoded chardata refresh / view toggle.
    {
        SHORT sv = GetAsyncKeyState(0x52); // 'R'
        if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
            if (DAT_07e11a34 == 0) {
                DAT_07e11a34 = 1;

                // Look up chardata in hash table — if found, decode + transform + re-encode
                if (DAT_07cf1ffc != nullptr) {
                    // Get hash-table entry for chardata
                    unsigned int idx = HashTable_GetIndex(&DAT_055c9bc8, DAT_07cf1ffc);
                    if (idx != 0xffffffff) {
                        void **ppEntry = (void **)(DAT_055c9bcc + idx * 4);
                        char *entry = (char *)*ppEntry;
                        char ref = *(entry + 0x161 * 4);
                        *(entry + 0x161 * 4) = ref + 1;
                        if ((BYTE)(ref + 1) < 2) {
                            // Allocate copy, decode (reverse XOR transform), write back
                            BYTE *buf = (BYTE *)operator_new(0x584);
                            memcpy(buf, DAT_07cf1ffc, 0x584);
                            // Reverse decode loop (0x583 downto 0)
                            for (int i = 0x583; i >= 0; --i) {
                                BYTE b   = buf[i];
                                DWORD ki = (DWORD)i & 0x8000000fu;
                                if ((int)ki < 0) ki = (ki - 1 | 0xfffffff0u) + 1;
                                b = (BYTE)(((BYTE *)&PacketXorKey16)[ki] ^ b + 0x47) + 0x23;
                                buf[i] = b;
                                if (i < 0x583) buf[i] ^= buf[i + 1];
                            }
                            // Write back to chardata
                            memcpy(DAT_07cf1ffc, buf, 0x584);
                            operator_delete(buf);
                        }
                    }
                }

                // Locate current DAT_07cf1ffc in hash and re-encode
                if (DAT_07cf1ffc != nullptr) {
                    unsigned int idx = HashTable_GetIndex(&DAT_055c9bc8, DAT_07cf1ffc);
                    if (idx != 0xffffffff) {
                        void **ppEntry = (void **)(DAT_055c9bcc + idx * 4);
                        char *entry = (char *)*ppEntry;
                        char ref = *(entry + 0x161 * 4);
                        *(entry + 0x161 * 4) = ref - 1;
                        if (*(entry + 0x161 * 4) == '\0') {
                            // Re-encode loop (0 to 0x583)
                            BYTE *buf = (BYTE *)operator_new(0x584);
                            memcpy(buf, DAT_07cf1ffc, 0x584);
                            for (DWORD i = 0; i < 0x584; ++i) {
                                BYTE b   = buf[i];
                                DWORD ki = i & 0x8000000fu;
                                if ((int)ki < 0) ki = (ki - 1 | 0xfffffff0u) + 1;
                                b = (BYTE)(b + 0x47);
                                b = (BYTE)(((BYTE *)&PacketXorKey16)[ki] ^ b) + 0x23;
                                buf[i] = b;
                                if (i < 0x583) buf[i] ^= buf[i + 1];
                                buf[i] ^= (BYTE)rand();
                            }
                            memcpy(DAT_07cf1ffc, buf, 0x584);
                            operator_delete(buf);
                        }
                    }
                }

                // Toggle DAT_07eaa118 (B-key correlation)
                {
                    char old118 = DAT_07eaa118;
                    if (((BYTE)old118 == 0) || (DAT_07eaa132 == '\0') || (DAT_07e91388 != 0)) {
                        // no toggle
                    } else {
                        if (DAT_07eaa150 == 0) {
                            DAT_07eaa134 = 1;
                            DAT_07eaa150 = 2;
                        } else {
                            DAT_07eaa134 = 0;
                            DAT_07eaa150 = 0;
                        }
                    }
                }
            }
        } else {
            DAT_07e11a34 = 0;
        }
    }

    // ── 11. Quick-use hotbar keys Q (slot 0), W (slot 1), E (slot 2) ─────────
    // Per IDA L4219-4914. Each key on edge-down triggers an item-use packet
    // for the item in hotbar slot N: `[0xC1, 0x04, 0x26, slot_index]`.
    // Plays sound 33 for type 448 (potion of mana), 32 for types 449-457
    // (other potions/scrolls). Skipped if WarehouseOpened or TradeOpened.
    // Rate-limited by EnableUse: the server acknowledges the item use with
    // 0x26/0xFD and clears this exact latch.
    {
        struct { int vk; int slotIdx; } qwe[] = {
            { 'Q', 0 },
            { 'W', 1 },
            { 'E', 2 },
        };
        // KeyState locals for Q=81, W=87, E=69 — using existing edge-tracking globals
        static BYTE s_qweEdge[3] = {0,0,0};
        for (int i = 0; i < 3; ++i) {
            SHORT sv = GetAsyncKeyState(qwe[i].vk);
            if ((char)((unsigned short)sv >> 8) == (char)(-0x80)) {
                if (s_qweEdge[i] == 0) {
                    s_qweEdge[i] = 1;
                    int slot = (int)FUN_00482be0(qwe[i].slotIdx);
                    if (slot != -1 &&
                        WarehouseOpened == '\0' &&
                        DAT_07eaa11b == '\0')   // !TradeOpened
                    {
                        if ((int)EnableUse <= 0) {
                            EnableUse = 10;
                            // The deployed server expects item use through the
                            // C3/serial path.  Chat_SendPacket only XORs the raw
                            // C1 payload, corrupting the slot on the wire and
                            // causing an immediate disconnect.
                            BYTE pkt[5] = { 0xC1, 0x05, 0x26, (BYTE)(slot + 12), 0x00 };
                            Net_SendSmallPacket(pkt, 5);
                            // Sound feedback by item type
                            int itemType = *(int*)(OffsetInventoryItems + slot * 0x44);
                            if (itemType == 448) {
                                FUN_00404bc0(33, 0, 0);   // potion mana sound
                            } else if (itemType >= 449 && itemType <= 457) {
                                FUN_00404bc0(32, 0, 0);   // potion HP / scroll sound
                            }
                        }
                    }
                }
            } else {
                s_qweEdge[i] = 0;
            }
        }
    }

    // ── 12-15. C/V/I/G/P key handlers — REMOVED ────────────────────────────
    // 2026-05-08 (b): These keys are already handled by Player_InputTick
    // (`HUD_HotkeyTick` in src/Game/Player_InputTick.cpp:251-287) using the
    // edge-triggered helper Input_IsKeyJustPressed. Adding duplicate handlers here
    // caused a DOUBLE-TOGGLE bug: pressing C played sound (Chat_InputTick set
    // CharacterOpened=1, played sound) but Player_InputTick toggled it back
    // to 0 in the same frame → net result = closed.
    //
    // Player_InputTick's pure-toggle handler is sufficient for visible-panel
    // gameplay. The packet-send side effects (guild/party 0x52/0x42 list
    // request, warehouse close 0x82) of the IDA Chat_InputTick path require
    // server context which is not yet wired.
}
