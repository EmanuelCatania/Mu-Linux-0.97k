// Recv_Chat.cpp — paquetes del server: chat, susurros y avisos.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

// ── CHAT ─────────────────────────────────────────────────────────
// Layout autoritativo del server MuEmu (Protocol.h) + confirmado 1:1 con
// IDA ReceiveChat (0x427630), que lee name en +3 y el mensaje en +13.
//   C1:00  PMSG_CHAT_SEND         name[10]@+3  message[60]@+13
//   C1:01  PMSG_CHAT_TARGET_SEND  index[2]@+3  message[60]@+5
//   C1:02  PMSG_CHAT_WHISPER_SEND name[10]@+3  message[60]@+13
// PMSG_CHAT_TARGET_SEND: el server manda por acá el mensaje de un NPC
// "hablado" (GCChatTargetSend — NpcTalk::NpcGuard lo usa para el guardia).
// Layout: index[2]@+3 (BE), message[60]@+5. Se muestra como burbuja de chat
// sobre la entidad.
// 0x01
void NetRecv_01(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    if (Size < 6) return;
    int entIdx = Entity_FindById((Msg[3] << 8) | Msg[4]);
    char cmsg[61] = {0};
    int clen = Size - 5;
    if (clen > 60) clen = 60;
    memcpy(cmsg, Msg + 5, clen);
    NetLog("NET:  → 0x01 ChatTarget ent=%d msg='%s'", entIdx, cmsg);
    if (entIdx >= 0 && entIdx < 400 && cmsg[0]) {
        BYTE* ent = (BYTE*)(uintptr_t)DAT_07abf5d0 + (uintptr_t)entIdx * 0x394;
        if (ent[0] != 0) {
            // CreateChat(nombre, texto, entidad, 0, -1) — igual que el
            // path de NPC hover (RenderMonsterName).
            CreateChat((char*)(ent + 0x1C1), cmsg, (DWORD)(uintptr_t)ent, 0, -1);
        }
    }
}

// 0x02
void NetRecv_02(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Whisper — mismo layout que chat normal; canal 3 (whisper) en el log.
    NetLog("NET:  → 0x02 ChatWhisper size=%d", Size);
    // PMSG_CHAT_WHISPER_SEND es de longitud VARIABLE (`14 + strlen`): no exigir
    // el tamaño MÁXIMO (73) — los `/post` dorados de
    // CommandManager::GCPostMessageGold llegan por este opcode con Size ~30.
    // PORT FIEL de IDA `ReceiveWhisper` @ 0x4278F0 (identificada por
    // disasm: la función sin nombre entre ReceiveChat y ReceiveNotice):
    //     a0 ac 1d e1 07  mov al, byte_7E11DAC   ; m_bBlockWhisper
    //     84 c0 / 0f 85   test+jnz → descarta el mensaje
    //     8d 4a 03        lea ecx, [edx+3]       ; name  = buf+3
    //     8d 72 0d        lea esi, [edx+0Dh]     ; msg   = buf+13
    //     6a 26 ...       PlayBuffer(0x26, 0, 0) ; sonido de whisper
    //     6a 00 ...       UIChatLogWindow_AddText(name, msg, 0)
    //
    // Canal **0** (como IDA) → negro sobre fondo celeste (0x9632C8FF), el look
    // clásico de whisper.
    if (Size >= 14 && DAT_07e11dac == 0) {   // m_bBlockWhisper (toggle F3)
        char wname[11] = {0};
        char wmsg[61]  = {0};
        memcpy(wname, Msg + 3, 10);
        // Copiar solo los bytes que el paquete realmente trae.
        int wlen = Size - 13;
        if (wlen > 60) wlen = 60;
        memcpy(wmsg, Msg + 13, wlen);
        // IDA ProtocolCore case 2: RegistWhisperID(10, strID) — con el
        // heroe de nivel < 10 anota al remitente para que despues se le
        // pueda contestar (ver FUN_0047fed0) — y el sonido SOLO con
        // m_bWhisperSound (0x07E11D80).
        RegistWhisperID(10, wname);
        if (m_bWhisperSound)
            PlayBuffer(0x26, 0, 0);
        UIChatLogWindow_AddText(wname, wmsg, 0);
    }
}

// 0x0C
void NetRecv_0C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA ProtocolCore case 0xC: PMSG_SERVER_MSG_SEND (C1:0C, MsgNumber).
    // MsgNumber 0 = el destinatario del susurro no esta conectado
    // (MuEmu: DGGlobalWhisperRecv -> GCServerMsgSend(index, 0)).
    //     if (!ReceiveBuffer[3]) UIChatLogWindow_AddText(ChatWhisperID, GlobalText[482], 2);
    if (Size >= 4 && Msg[3] == 0)
        UIChatLogWindow_AddText(ChatWhisperID, GlobalText[482], 2);
}

// 0x0D
void NetRecv_0D(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Port FIEL desde IDA ReceiveNotice (0x00427A00).
    //
    // PMSG_NOTICE_SEND server layout (server source
    // Mu-linux-97K/Source/MuServer/GameServer/Notice.cpp):
    //   struct { PBMSG_HEAD header; BYTE type; char message[256]; }
    //   = [C1][size][0x0D][type][message null-terminated]
    //
    // type 0 → CreateNotice(msg, 0) — CENTERED blue/cyan banner con
    //          blink (UI_AddNotice). Eventos del servidor (Blood
    //          Castle, Happy Hour, server close, etc).
    // type 1 → UIChatLogWindow_AddText — BLUE chat log local
    //          (mensajes personales: login welcome, errors, etc).
    // type 2 → guild notice (sprintf "Guild: %s" + CreateNotice
    //          gold). Deferred — guild stack no portado.
    //
    // El mensaje arranca en Msg+4 (no en Msg+7, que es el formato 5.2 con
    // campos extra).
    if (Size < 5) {
        NetLog("NET:  → 0x0D Notice size=%d (too small)", Size);
        return;
    }
    BYTE type = Msg[3];
    // El mensaje arranca en el offset 4 (después de C1, size, 0x0D, type).
    // El server termina el buffer con NUL, pero copiamos con
    // length cap as a safety net.
    char text[256] = {0};
    int textLen = Size - 4;
    if (textLen <= 0) {
        NetLog("NET:  → 0x0D Notice type=%d empty", type);
        return;
    }
    if (textLen > (int)sizeof(text) - 1) textLen = sizeof(text) - 1;
    memcpy(text, Msg + 4, textLen);
    text[textLen] = 0;
    NetLog("NET:  → 0x0D Notice type=%d msg='%s'", type, text);

    if (type == 0) {
        // Centered banner (blink cyan). Color flag stored at
        // slot[0x104]; el renderer UI_RenderNotices lo lee para cambiar
        // between blink-cyan (flag=0) and gold (flag=1).
        extern void __cdecl UI_AddNotice(char*, unsigned char);
        UI_AddNotice(text, 0);
    } else if (type == 1) {
        // Personal blue chat log entry.
        extern void UIChatLogWindow_AddText(const char* strID,
                                           const char* msg,
                                           int color);
        // IDA pasa una cadena vacía como strID, no NULL.
        UIChatLogWindow_AddText("", text, 1);
    } else if (type == 2) {
        // Guild notice — gold centered banner. Original IDA
        // formats with GlobalText[483] ("Guild Notice: %s").
        extern void __cdecl UI_AddNotice(char*, unsigned char);
        char guildText[320] = {0};
        SetGuildNoticeText(text);
        if (GlobalText[483] && GlobalText[483][0] != 0) {
            wsprintfA(guildText, GlobalText[483], text);
            UI_AddNotice(guildText, 1);
        } else {
            UI_AddNotice(text, 1);
        }
    }
}

// 0x00
void NetRecv_00(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // PMSG_CHAT_SEND server→client — [C1][size][0x00][name 10B][msg 60B]
    // Delega al port FIEL `ReceiveChat` (IDA 0x427630): nombre y mensaje van
    // SEPARADOS (el renderLine del ChatListBox compone "name: text"), canal 3,
    // dispatch de prefijos ('~'=party/4, '@'=guild/5, '#'=solo burbuja) y
    // burbuja sobre el personaje (AssignChat).
    NetLog("NET:  → 0x00 Chat size=%d", Size);
    // GUARDA: el server manda `C1 04 00 xx` (4 bytes) como
    // ping/handshake. ReceiveChat lee name@+3 y el mensaje desde +13, así que
    // con 4 bytes sobre-lee. El IDA solo ACKea esos en estado login
    // (SceneFlag==2) y cae al parseo de chat en el resto → misma
    // sobre-lectura. Procesamos como chat solo si el paquete tiene el
    // tamaño mínimo de PMSG_CHAT_SEND (ver abajo) o estamos en el login.
    extern void __cdecl ReceiveChat(BYTE* ReceiveBuffer);
    // PMSG_CHAT_SEND es de longitud VARIABLE — el server hace
    //   header.set(0x00, sizeof(pMsg) - (sizeof(pMsg.message) - (size+1)))
    //   = 14 + strlen(mensaje)
    // así que no se puede exigir el tamaño MÁXIMO de la struct (73): se
    // perderían los mensajes cortos y los `/post` azul (`~`) y verde (`@`) de
    // CommandManager::GCPostMessageBlue/Green.
    // Mínimo real = 3 (hdr) + 10 (name) + 1 (al menos un char) = 14.
    // El server null-termina el mensaje, así que la lectura de 60 bytes
    // que hace ReceiveChat (fiel a IDA) se corta sola en el NUL.
    if (Size >= 14 || SceneFlag == 2) {
        ReceiveChat(Msg);
    } else {
        NetLog("NET:    0x00 too short (%d) — ping/handshake, no chat parse", Size);
    }
}
