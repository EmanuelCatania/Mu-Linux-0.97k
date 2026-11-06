#include <cstdint>
// SecondPassword.cpp
// SecondPassword_Handler @ 0x004E93A0  (1542 lines, 287 basic blocks)
//
// Maneja el ingreso del PIN / segunda contraseña y manda el paquete 0xC1/0x83 al server.
// Se llama en cada frame mientras el diálogo de segunda contraseña está activo.

//
// Similitud según las métricas de Ghidra: 81.6% con Game_SceneUpdate — los mismos bloques
// anti-tamper de hash-table y el XOR son la mayoría de las "líneas".
//
// ── GLOBALS ──────────────────────────────────────────────────────────────────
//
//   DAT_07eaa14c  — PIN mode / input state:
//                     0 = inactive (return 0 immediately)
//                     2 = enter new PIN
//                     3 = confirm PIN (re-enter)
//                     6 = — (mapeado a DAT_083a7acc para la altura de la UI)
//   DAT_07ea9810  — PIN input buffer base (4 bytes padding before actual data)
//   DAT_07ea9814  — PIN buffer start: bytes [0-3] of 10-byte PIN area
//                   (_DAT_07ea9814 = 4 bytes as uint)
//   DAT_07ea9818  — PIN buffer bytes [4-7] (uint)
//   DAT_07ea981c  — PIN buffer bytes [8-9] (ushort)
//   DAT_07eaa150  — ID de contexto del server, 2 bytes (va con el PIN en los casos 2 y 3)
//   DAT_07eaa179  — "input active / submitted" flag:
//                     1 = se setea al hacer click en el diálogo del PIN
//                     0 = se limpia cuando se setea DAT_083a413c (diálogo cerrado)
//   DAT_07eaa119  — anti-tamper gate byte (0 = proceed with hash-table block)
//   DAT_07eaa11a  — anti-tamper check byte
//   DAT_07eaa11c  — anti-tamper check byte
//   DAT_07e91394  — character name table (short[N]): indexed by iVar5 (keycode?)
//                   se usa en wsprintfA para formatear el nombre en el buffer del PIN
//   DAT_083a4124  — flag de click del mouse (se setea con cualquier click)
//   DAT_083a413c  — flag de "cerrar diálogo" (si está seteado → DAT_07eaa179 = 0)
//   DAT_083a42c4  — byte de estado de la UI (queda en 0 si el envío salió bien)
//   DAT_083a7acc  — parámetro de altura de la UI (se usa cuando DAT_07eaa14c == 6)
//
// ── ENTRY GUARD ──────────────────────────────────────────────────────────────
//
//   if (DAT_07eaa14c == 0) return 0;   // el diálogo del PIN no está activo
//
// ── KEY INPUT PROCESSING ──────────────────────────────────────────────────────
//
//   FUN_004e9300()  → Keyboard_GetLastKey()
//     Devuelve el código de tecla (iVar5). También setea DAT_083a4124 = flag de click.
//
//   if (DAT_083a4124 != '\0'):  // a click or keypress happened
//     if (DAT_07eaa14c in {2, 3, 6}): uVar14 = DAT_083a7acc (largo de la UI)
//     else: uVar14 = 4 (default PIN length)
//     PlayBuffer(0x19, 0, 0)  → UI_SetScene(0x19 = ServerSelect clear?)
//
//   Key code 10 (Enter / \n):
//     Chequea si DAT_07ea9814 tiene contenido:
//       if empty (iVar5 == -2 after strlen): SVar6 = 0 (no action)
//       else:
//         Si strlen == uVar14 Y los 4 bytes son iguales
//            (DAT_07ea9814 == DAT_07ea9815 == DAT_07ea9816 == DAT_07ea9817)
//            AND uVar14 == 4:
//           _DAT_07ea9814 = 0; DAT_07ea9818 = 0; DAT_07ea981c = 0
//           SetErrorMessage(0x88)  → ShowErrorDialog(0x88)  "All digits same"
//         else:
//           DAT_07eaa14c = 0
//           → cae al switch (arma y manda el paquete)
//
//   Key code 0xb (Tab? Backspace+confirm?):
//     Similar all-same-digits check → SetErrorMessage(0x88) or proceed
//
//   Other keys / character input:
//     Platform::TextFormat(&local_d20, format, (&DAT_07e91394)[iVar5])
//     Append formatted char to DAT_07ea9814 buffer (strlen-based)
//     Trimmed to uVar14 max length
//
//   Key 10 (backspace in buffer mode):
//     *(uint8_t*)(&DAT_07ea9810 + current_len + 1) = 0   (delete last char)
//
// ── 0xC1/0x83 SECOND PASSWORD PACKET ─────────────────────────────────────────
//
//   Opcode: 0x83 (second password / PIN verification)
//   Todos los casos usan la misma clave XOR de 32 bytes:
//     {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,
//      0x49,0x5d,0x39,0x5d,0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
//      0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56}
//
//   Switch on DAT_07eaa14c:
//
//   case 2 (enter new PIN):
//     Payload (16 bytes):
//       [0x00][0x01]                     — 2-byte header extension
//       DAT_07eaa150 (2 bytes LE)        — server context ID
//       _DAT_07ea9814 (4 bytes)          — PIN bytes 0-3
//       DAT_07ea9818  (4 bytes)          — PIN bytes 4-7
//       DAT_07ea981c  (2 bytes)          — PIN bytes 8-9
//     local_4 = 0
//
//   case 3 (confirm PIN re-entry):
//     Misma estructura que el caso 2.
//     local_4 = 3
//
//   default (other modes):
//     str_to_ushort(&DAT_07ea9814) → 2-byte derived value
//     Payload: [2 bytes context] [10 bytes PIN buffer]
//     local_4 = 4
//
//   Camino de envío (igual que todos los otros paquetes):
//     CRC = CSimpleModulus_Encode(0, payload, len)
//     if len < 0x100: header [0xC3][len+2], CSimpleModulus_Encode(out+2, payload, len)
//     else:           header [0xC4][hi][lo+3], CSimpleModulus_Encode(out+3, payload, len)
//     send(SocketClientSocket, ...) con WSAEWOULDBLOCK → encola en SocketClientSendBuffer
//
// ── POST-SEND ─────────────────────────────────────────────────────────────────
//
//   DAT_083a4124 = '\0'   (clear click flag)
//   DAT_083a42c4 = 0      (clear UI state)
//   if (DAT_083a4124 != '\0'): DAT_07eaa179 = 1
//   if (DAT_083a413c != '\0'): DAT_07eaa179 = 0
//   return 1
//
// ── ANTI-TAMPER BLOCK (g_bUseChatListBox == 1 gate) ────────────────────────────────
//
//   If DAT_07eaa119 == '\0':
//     HashTable operations on DAT_07eaa11b (ref-count maintain)
//     If cStack_83d=='\0' && DAT_07eaa119=='\0' && DAT_07eaa11a=='\0' && DAT_07eaa11c=='\0':
//       HashTable operations on DAT_07eaa118
//   Este bloque es puro anti-tamper (el mismo patrón que Game_SceneUpdate).
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_004e9300   → Keyboard_GetLastKey()  — reads pending keystroke
//   SetErrorMessage   → ShowErrorDialog(id)
//                     id 0x88 = "Todos los dígitos iguales, el PIN es inválido"
//   CSimpleModulus_Encode   → Packet_Encode / CRC_Compute
//   PlayBuffer   → UI_SetScene(id, 0, 0)
//   HashTable_Insert   → HashTable_Insert
//   Packet_DecryptByte   → HashTable_Remove
//   PACKET_DECRYPT  → HashTable_GetOrInsert (packet seq tracking)
//   PACKET_ENCRYPT   → HashTable_Decrement
//   str_to_ushort  → parse 2 ASCII digits to ushort (Ghidra name retained)

#include "stdafx.h"
#include "UI/ChatListBox.h"
extern "C" const char* Guild_GetMarkName(int row);
#include "Text/TextCore.h"
#include "Party/Party.h"
#include "Entity/EntityName.h"
#include "Net/NetText.h"
#include "Render/TerrainDraw.h"
void Terrain_BeginBufferedPass();
#include "Render/BufferedDraw.h"
#include "Game/RenderFxSnapshots.h"
#include "Entity/ObjectBounds.h"
#include "Platform/Text.h"
#include "Platform/ISocket.h"
#include "Platform/ITimer.h"
#include "Platform/IInput.h"
#include "Item/ContentCatalog.h"
#include "Item/ItemDefines.h"
#include "Item/RightClickMove.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"
#include "structs.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// El rango de guild pide C1 plano (Encrypt=0).
extern "C" void GuildCreator_CloseFromResult(void);

// Inventory pool bases — defined in src/Render/HUD_Pass3.cpp.
// Lo usan los llamadores de FUN_004d23b0 de abajo para recorrer los arrays de grilla correctos.
extern "C" BYTE OffsetInventoryItems[];
extern "C" BOOL ChaosBoxRequestClose(void);
extern "C" BYTE Inventory[];
extern "C" BYTE ShopItems[];   // pool dedicado de la tienda
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];

// IDA: SecondPassword_Handler (0x004E93A0)
// Teclado numerico del PIN del baul.  Lo llama UpdateWindowsMouse (0x4ECB00)
// antes que el resto de los hit-tests, y lo dibuja sub_4EB070.  El modo lo fija
// dword_7EAA14C (nuestro DAT_07eaa14c):
//   1 = abrir el candado para sacar un item (lo arma el drop del baul)
//   2 = escribir el codigo personal para PONER el candado
//   3 = escribir el codigo personal para SACAR el candado
//   4 = escribir el PIN nuevo   ->   5 = repetirlo
//   6 = cerrar la ventana del NPC
// Los modos 1, 2 y 3 mandan PMSG_WAREHOUSE_PASSWORD_RECV con type = modo - 1
// (0 abrir, 1 poner, 2 sacar), igual que IDA (case 2, case 3 y el default).
//
// Ruido anti-tamper omitido por policy (la clave XOR y la hash-table que
// envuelven cada envio).
extern "C" int __cdecl sub_4E9300_(void);   // hit-test del teclado (0x004E9300)

unsigned int __cdecl SecondPassword_Handler(void)
{
    if (!DAT_07eaa14c) return 0;

    const int btn = sub_4E9300_();
    if (IsClickPushed() && btn >= 0) {
        // Largo del campo: 4 para el PIN, 7 para el codigo personal.  (IDA lee
        // LODWORD(flt_83A7ACC[0]), que es el arreglo de la camara; ver la misma
        // desviacion en el render, HUD_Pass3.)
        const int maxLen = (DAT_07eaa14c == 2 || DAT_07eaa14c == 3 || DAT_07eaa14c == 6) ? 7 : 4;
        PlayBuffer(25, 0, 0);

        const int len = (int)strlen(DAT_07ea9814);
        if (btn == 10) {                       // borrar
            if (len > 0) DAT_07ea9814[len - 1] = '\0';
        } else if (btn != 11) {
            if (btn == 12) {                   // cancelar
                DAT_07eaa14c = 0;
            } else if (len < maxLen) {         // digito (el teclado esta barajado)
                char digit[8];
                Platform::TextFormat(digit, "%d", (int)DAT_07e91394[btn]);
                strcat(DAT_07ea9814, digit);
            }
        } else if (len == maxLen || (maxLen != 4 && len > 0)) {   // aceptar
            // DESVIACION: IDA exige `len == v2`, pero v2 sale de
            // LODWORD(flt_83A7ACC[0]) — el arreglo de la camara — asi que no hay
            // un largo fiel que copiar.  Para el codigo personal aceptamos lo
            // que se haya tipeado (el server compara 7 caracteres, y con
            // PersonalCodeCheck=0 ni los mira).  El PIN sigue pidiendo 4.
            // IDA L211-217: un PIN de 4 digitos todos iguales no se acepta.
            if (maxLen == 4 && DAT_07ea9814[0] == DAT_07ea9814[1]
                            && DAT_07ea9814[1] == DAT_07ea9814[2]
                            && DAT_07ea9814[2] == DAT_07ea9814[3]) {
                memset(DAT_07ea9814, 0, sizeof(DAT_07ea9814));
                SetErrorMessage(136);
            } else {
                const int mode = (int)DAT_07eaa14c;
                DAT_07eaa14c = 0;

                if (mode == 4) {
                    // Guarda el PIN y pide repetirlo con el teclado rebarajado.
                    const unsigned int typed = *(unsigned int*)&DAT_07ea9814[0];
                    SecondPassword_Shuffle(5);                  // baraja + DAT_07eaa14c = 5
                    memset(DAT_07ea9814, 0, sizeof(DAT_07ea9814));
                    DAT_07ea981f = typed;
                } else if (mode == 5) {
                    if (*(unsigned int*)&DAT_07ea9814[0] == DAT_07ea981f) {
                        const unsigned short pw = (unsigned short)atoi(DAT_07ea9814);
                        memset(DAT_07ea9814, 0, sizeof(DAT_07ea9814));
                        *(unsigned short*)&DAT_07eaa150 = pw;   // conserva el BYTE 2 (reparar)
                        DAT_07eaa14c = 2;                       // ahora el codigo personal
                    } else {
                        SetErrorMessage(137);                   // no coinciden
                    }
                } else if (mode == 6) {
                    const BYTE closePkt[3] = { 0xC1, 0x03, 0x31 };
                    gNetwork.SendC1(closePkt, sizeof(closePkt));
                } else {
                    // Modos 1, 2 y 3: PMSG_WAREHOUSE_PASSWORD_RECV
                    // [C1][10][83][type][WORD password][PersonalCode:10]
                    BYTE pkt[16];
                    memset(pkt, 0, sizeof(pkt));
                    pkt[0] = 0xC1;
                    pkt[1] = 16;
                    pkt[2] = 0x83;
                    pkt[3] = (BYTE)(mode - 1);
                    unsigned short pw = 0;
                    if (mode == 1) {
                        // Abrir: el PIN se acaba de tipear y el codigo personal
                        // sale del campo de texto.
                        pw = (unsigned short)atoi(DAT_07ea9814);
                        NetText::ToServer(pkt + 6, 10, (const char*)DAT_07db8710);
                    } else {
                        // Poner (2): la contrasena la dejo el modo 5.
                        // Sacar (3): el server no la mira.
                        if (mode == 2) pw = *(unsigned short*)&DAT_07eaa150;
                        NetText::ToServer(pkt + 6, 10, DAT_07ea9814);
                    }
                    memcpy(pkt + 4, &pw, 2);
                    gNetwork.SendC1(pkt, sizeof(pkt));
                    memset(DAT_07ea9814, 0, sizeof(DAT_07ea9814));
                }
            }
        }
    }

    // IDA LABEL_203: byte_7EAA179 marca el boton apretado (para el bitmap
    // "pressed" del render) y el click se consume aca.
    if (IsClickPushed()) DAT_07eaa179 = 1;
    if (DAT_083a413c)    DAT_07eaa179 = 0;
    DAT_083a4124 = 0;
    DAT_083a42c4 = 0;
    return 1;
}
