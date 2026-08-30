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
//     FUN_00404bc0(0x19, 0, 0)  → UI_SetScene(0x19 = ServerSelect clear?)
//
//   Key code 10 (Enter / \n):
//     Chequea si DAT_07ea9814 tiene contenido:
//       if empty (iVar5 == -2 after strlen): SVar6 = 0 (no action)
//       else:
//         Si strlen == uVar14 Y los 4 bytes son iguales
//            (DAT_07ea9814 == DAT_07ea9815 == DAT_07ea9816 == DAT_07ea9817)
//            AND uVar14 == 4:
//           _DAT_07ea9814 = 0; DAT_07ea9818 = 0; DAT_07ea981c = 0
//           FUN_005142d0(0x88)  → ShowErrorDialog(0x88)  "All digits same"
//         else:
//           DAT_07eaa14c = 0
//           → cae al switch (arma y manda el paquete)
//
//   Key code 0xb (Tab? Backspace+confirm?):
//     Similar all-same-digits check → FUN_005142d0(0x88) or proceed
//
//   Other keys / character input:
//     wsprintfA(&local_d20, format, (&DAT_07e91394)[iVar5])
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
//     CRC = FUN_0053cc30(0, payload, len)
//     if len < 0x100: header [0xC3][len+2], FUN_0053cc30(out+2, payload, len)
//     else:           header [0xC4][hi][lo+3], FUN_0053cc30(out+3, payload, len)
//     send(DAT_055ca168, ...) con WSAEWOULDBLOCK → encola en DAT_055ca16c
//
// ── POST-SEND ─────────────────────────────────────────────────────────────────
//
//   DAT_083a4124 = '\0'   (clear click flag)
//   DAT_083a42c4 = 0      (clear UI state)
//   if (DAT_083a4124 != '\0'): DAT_07eaa179 = 1
//   if (DAT_083a413c != '\0'): DAT_07eaa179 = 0
//   return 1
//
// ── ANTI-TAMPER BLOCK (DAT_005590ac == 1 gate) ────────────────────────────────
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
//   FUN_005142d0   → ShowErrorDialog(id)
//                     id 0x88 = "Todos los dígitos iguales, el PIN es inválido"
//   FUN_0053cc30   → Packet_Encode / CRC_Compute
//   FUN_00404bc0   → UI_SetScene(id, 0, 0)
//   FUN_00403f80   → HashTable_Insert
//   FUN_00404330   → HashTable_Remove
//   FUN_00422df0   → HashTable_GetOrInsert (packet seq tracking)
//   FUN_00404040   → HashTable_Decrement
//   str_to_ushort  → parse 2 ASCII digits to ushort (Ghidra name retained)

#include "stdafx.h"
#include "structs.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// 2026-08-25: el rango de guild pide C1 plano (Encrypt=0).
extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);
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

// 2026-05-07: B3 refactor — SecondPassword screens (FUN_004e4760 .. FUN_004ec330)
// moved from stubs.cpp lines 6961-8495 (1535 lines). Full implementation below.

// FUN_004e93a0 @ 0x004E93A0 — SecondPassword_NetHandler(void)
// Handler de red principal de la segunda contraseña: lee los paquetes entrantes del socket
// y despacha a los sub-handlers según el opcode. SEH completo + ofuscación por HashTable.
// STUB: returns 0 (no PIN required). Full impl pending.
unsigned int __cdecl FUN_004e93a0(void) { return 0; }
// FUN_004df410 @ 0x004DF410 — Inventory drop dispatcher.
// 2026-05-08: port FIEL completo movido a `Item/Item_ClickHandler.cpp`
// (~150 líneas). Antes era stub vacío bloqueando toda la cadena drag-drop.
// Misnamed previously as "SecondPassword_NetTick" — IDA confirma que es
// el dispatcher de drop sobre las 4 inventories abiertas + sell-to-shop +
// drop-on-ground.
// Sub-handlers de SecondPassword (todos usan frame SEH de Windows — sólo stubs)
// Son las funciones de render/tick por frame del subsistema del diálogo de 2da contraseña.
// Cada una tiene entre 80 y 854 líneas decompiladas. Las implementaciones completas están en src/Net/SecondPassword_UI.cpp.
//
static bool GetGuildCreatorOrigin(int& originX, int& originY)
{
    // 2026-07-27: el scratch ya no vive en Inventory[32] (= slot 0 del pool de
    // la tienda, lo pisaba); ahora tiene globals propios.
    originX = g_GuildCreatorScratchX;
    originY = g_GuildCreatorScratchY;
    return (originX != 0 || originY != 0);
}

// FUN_004e4760 @ 0x004E4760 — SecondPassword_Screen1 (727 lines)
//   - Chequea DAT_07eaa124 (hover habilitado). Si DAT_07eaa144==1: grilla de click para 8 botones
//     (8×0x0F tiles con origen en DAT_07ea5b1c/20+100), escribe el array DAT_07ea51f5 con DAT_07eaa0dc.
//   - Segunda pasada: itera los pasos de DAT_07ea5b18 (+0x32 cada uno), los compara con la posición del mouse para encontrar
//     hovered button index, sets DAT_07ea51ed.
//   - SEH + llamadas a UI_SetScene(0x19/0x1c). Implementado en SecondPassword_UI.cpp.
void __cdecl FUN_004e4760(void) {
    // 2026-05-04: BUG-FIX del sonido de click fantasma del lado izquierdo. El sub_4E4760 de IDA es
    // el hit-test del diálogo GuildCreator (gatea con GuildCreatorOpened, usa
    // Inventory[32].Level/Part as panel origin). Our port had wrong gate
    // (DAT_07eaa124) Y con la base equivocada (DAT_07ea5b1c/20 = 0 → los tests disparan en
    // left-side x=0..190 → phantom click sfx).
    // Hasta que cableemos el storage de Inventory[32] y el camino de render correcto, gateamos
    // además con `DAT_07ea5b1c != 0` para que los hit-tests fantasma del lado izquierdo
    // no disparen nunca. El código de la fase de render en HUD_Pass6 maneja los clicks reales
    // for Char/Party/Guild panels.
    int originX = 0, originY = 0;
    if (DAT_07eaa124 == '\0' || !GetGuildCreatorOrigin(originX, originY))
        goto LAB_FUN_004e4760_end;

    // Zona de hover del mouse para todo el panel
    if ((int)DAT_083a427c >= originX &&
        (int)DAT_083a427c <  originX + 0xbe &&
        (int)DAT_083a4278 >= originY &&
        (int)DAT_083a4278 <  originY + 0x1b1) {
        DAT_07d78094 = 1;
    }

    if (DAT_07eaa144 == 1) {
        // IDA: FUN_004E4760. La grilla de edición tiene 8 filas de 8 celdas;
        // cada celda conserva un índice de color completo en DAT_07ea51f5.
        // La compresión a 32 bytes ocurre únicamente al enviar el paquete 0x55.
        *(DWORD*)(DAT_07abf5d8 + 0x24) = 0x42e10000;
        const BYTE selectedColor = (BYTE)DAT_07eaa0dc;
        BYTE* const cells = (BYTE*)&DAT_07ea51f5;
        for (int row = 0; row < 8; ++row) {
            const int cellY = originY + 100 + row * 15;
            for (int col = 0; col < 8; ++col) {
                const int cellX = originX + 50 + col * 15;
                if ((int)DAT_083a427c >= cellX && (int)DAT_083a427c < cellX + 15 &&
                    (int)DAT_083a4278 >= cellY && (int)DAT_083a4278 < cellY + 15) {
                    const int cellIndex = row * 8 + col;
                    if (DAT_083a42c4 != '\0') cells[cellIndex] = selectedColor;
                    if (DAT_083a42ac != '\0') cells[cellIndex] = 0;
                }
            }
        }

        // IDA: FUN_004E4760 — paleta 2×8, índice lineal fila*8+col.
        for (int row = 0; row < 2; ++row) {
            const int colorY = originY + 260 + row * 20;
            for (int col = 0; col < 8; ++col) {
                const int colorX = originX + 15 + col * 20;
                if ((int)DAT_083a427c >= colorX && (int)DAT_083a427c < colorX + 20 &&
                    (int)DAT_083a4278 >= colorY && (int)DAT_083a4278 < colorY + 20 &&
                    IsClickPushed()) {
                    DAT_083a4124 = '\0';
                    FUN_00404bc0(0x19, 0, 0);
                    DAT_07eaa0dc = (char)(row * 8 + col);
                }
            }
        }
    }

    {
        // "OK" button [x+0x14, x+0x5a) x [y+0x15e, y+0x173)
        int iVar8 = DAT_083a427c;
        if (originX + 0x14 <= iVar8 &&
            iVar8 < originX + 0x5a &&
            originY + 0x15e <= (int)DAT_083a4278 &&
            (int)DAT_083a4278 < originY + 0x173 &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            // 2026-08-25 PORT (no se podia crear un guild): este boton — el izquierdo,
            // rect [+20,+90) x [+350,+371) — es el de CREAR, y mandaba el opcode
            // equivocado. IDA `sub_4E4760` L210 manda aca el **0x55**
            // (PMSG_GUILD_CREATE_RECV); el 0x54 es el del boton derecho, que ya esta
            // bien abajo.
            //
            //   struct PMSG_GUILD_CREATE_RECV {   // Guild.h:218
            //       PBMSG_HEAD header;   // C1 : 43 : 0x55
            //       char GuildName[8];   // +3
            //       BYTE Mark[32];       // +11
            //   };
            //
            // `g_iKeyPadEnable` (= DAT_07eaa144, 0x7EAA144) distingue DOS MODOS del
            // mismo panel, no autoriza nada — `ProtocolCore` L1253-1258:
            //     0x54 recibido -> GuildCreatorOpened=1, g_iKeyPadEnable=0
            //                      (dialogo previo "¿crear guild?", OK/Cancel)
            //     0x55 recibido -> GuildCreatorOpened=1, g_iKeyPadEnable=1
            //                      (UI de creacion: nombre + marca)
            // y cada boton cambia de opcode segun el modo (IDA sub_4E4760):
            //     izquierdo: modo 0 -> 0x54 result=1   |  modo 1 -> 0x55 CREAR
            //     derecho:   modo 0 -> 0x54            |  modo 1 -> 0x57 cancelar
            //
            // El `== 0` de este gate era correcto: es la rama del dialogo previo,
            // que ya mandaba bien su 0x54. Lo que faltaba era la OTRA rama.
            //
            // Nota aparte: el flag estaba partido en dos — `HUD_Pass6.cpp` tenia un
            // `static int g_iKeyPadEnable` homonimo que el handler del 0x55 seteaba
            // mientras este hit-test leia el global. Unificados.
            if (DAT_07eaa144) {
                if (Chat_ValidateInputCommand()) {
                    // nombre invalido / vacio
                    FUN_005142d0(115);
                } else {
                    // Empaquetado de la marca, fiel a IDA L180-193: 64 celdas de la
                    // grilla 8x8 -> 32 bytes, celda PAR en el nibble alto e IMPAR en el
                    // bajo. `any` replica el flag `v71`: con la marca vacia el original
                    // no manda nada y muestra el error 127.
                    BYTE mark[32];
                    bool any = false;
                    for (int k = 0; k < 64; ++k) {
                        const unsigned char v = (unsigned char)DAT_07ea51f5[k];
                        if (k & 1) mark[k >> 1] = (BYTE)(mark[k >> 1] + v);
                        else       mark[k >> 1] = (BYTE)(16 * v);
                        if (v) any = true;
                    }
                    if (!any) {
                        FUN_005142d0(127);
                        FUN_00404bc0(25, 0, 0);
                    } else {
                        // IDA: strcpy(&dword_7EA51EC, InputText[0])
                        memset(DAT_07ea51ec, 0, sizeof(DAT_07ea51ec));
                        for (int c = 0; c < 8 && DAT_07db8710[0][c]; ++c)
                            DAT_07ea51ec[c] = DAT_07db8710[0][c];
            
                        // El rango de guild entero es Encrypt=0 -> C1 plano, y
                        // `Net_SendC1Packet` aplica el mismo chain-XOR sobre 3..len-1
                        // que hace el original.
                        BYTE pkt[43];
                        memset(pkt, 0, sizeof(pkt));
                        pkt[0] = 0xC1;
                        pkt[1] = 43;
                        pkt[2] = 0x55;
                        memcpy(pkt + 3,  DAT_07ea51ec, 8);
                        memcpy(pkt + 11, mark, 32);
                        Net_SendC1Packet(pkt, 43);

                        // IDA: FUN_004E4760 cae al cierre común inmediatamente
                        // después de encolar 0x55: limpia el input, cierra el
                        // editor y reinicia el intervalo de mouse 0/6.
                        GuildCreator_CloseFromResult();
                        PlayBuffer(28, 0, 0);
                        DAT_07e11d28 = 0;
                        DAT_00559bec = 6;
                    }
                }
            } else {
                // Modo 0 = dialogo previo "¿crear guild?": el boton izquierdo
                // responde 0x54 con result=1 (PMSG_GUILD_MASTER_OPEN_RECV). El
                // server valida nivel y resets (Guild.cpp:37-45) y, si pasa,
                // contesta el 0x55 que abre la UI de creacion.
                // IDA: C1:04:54:01 confirma la pregunta previa al Guild Master.
                {
                    const BYTE pkt[4] = { 0xC1, 0x04, 0x54, 0x01 };
                    Net_SendC1Packet(pkt, sizeof(pkt));
                }
            }
            FUN_00404bc0(0x19, 0, 0);
        }
    }

    // "Cancel" button [x+0x64, x+0xaa) x [y+0x15e, y+0x173)
    {
        int iVar8 = DAT_083a427c;
        if (originX + 100 <= iVar8 &&
            iVar8 < originX + 0xaa &&
            originY + 0x15e <= (int)DAT_083a4278 &&
            (int)DAT_083a4278 < originY + 0x173 &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            if (DAT_07eaa144 == 0) {
                // IDA: C1:04:54:00 cancela la pregunta previa al Guild Master.
                const BYTE pkt2[4] = { 0xC1, 0x04, 0x54, 0x00 };
                Net_SendC1Packet(pkt2, sizeof(pkt2));
            } else {
                // IDA: C1:03:57 cancela el editor y libera INTERFACE_GUILD_CREATE.
                const BYTE pkt3[3] = { 0xC1, 0x03, 0x57 };
                Net_SendC1Packet(pkt3, sizeof(pkt3));
                *(short*)(DAT_07abf5d8 + 0x1da) = (short)0xffff;
            }
            FUN_00404bc0(0x19, 0, 0);
            // 2026-05-04: DAT_07db8710 ahora es char[10][256] — escribimos el primer byte
            // del slot 0 explícitamente. (Los globals 8714/8718 son símbolos separados
            // preexistentes — los limpiamos como antes para terminar en NUL lo que haya quedado
            // username data.)
            DAT_07db8710[0][0] = 0;
            DAT_07db8714 = 0;
            DAT_07db8718 = 0;
            *(DWORD*)DAT_07d780a8 = 0;
            _DAT_00559c94 = 10;
            DAT_07e11d70 = 0;
            DAT_07eaa124 = '\0';
            DAT_07eaa144 = 0;
            FUN_00404bc0(0x1c, 0, 0);
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }

LAB_FUN_004e4760_end:
    // DAT_07eaa114 NPC-window close button
    if (DAT_07eaa114 != '\0') {
        // 2026-04-30 BUG-FIX: vtable[+0x14] = slot 5 = ChatLB_tick (__fastcall
        // toma esto en ECX + argumento). El dispatch cdecl estilo Ghidra anterior
        // con el argumento `(0)` dejaba ECX = basura → ChatLB_tick leía memoria random
        // como `self[3]` (contador de list1), entraba al loop de desencolado, y
        // FUN_0040c580 deferenciaba un puntero-atrás de nodo que eran bytes de código
        // (0x83EC8B5D = pop ebp; mov ebp, esp; ...).
        if (DAT_055c9ff4 && *(int*)DAT_055c9ff4) {
            DWORD* obj = (DWORD*)DAT_055c9ff4;
            void** vt  = (void**)*obj;
            typedef int (__fastcall *FnTick)(DWORD*, int /*edx*/, int);
            ((FnTick)vt[5])(obj, 0, 0);
        }
        if ((int)(DAT_07e91788 + 0x19) <= (int)DAT_083a427c &&
            (int)DAT_083a427c < (int)(DAT_07e91788 + 0x31) &&
            (int)(DAT_07e91784 + 0x18b) <= (int)DAT_083a4278 &&
            (int)DAT_083a4278 < (int)(DAT_07e91784 + 0x1a3) &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            DAT_07eaa114 = '\0';
            FUN_00404bc0(0x19, 0, 0);
            FUN_00404bc0(0x1c, 0, 0);
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }
}
// FUN_004e5500 @ 0x004E5500 — Party panel input.
// The Party row action is gated by leader/self name matching and emits the
// server-owned delete request.  The close button only closes the panel.
void __cdecl FUN_004e5500(void) {
    if (DAT_07eaa115 == '\0' || DAT_07ea5b24 == 0) return;

    if ((int)DAT_083a427c >= (int)DAT_07ea5b24 &&
        (int)DAT_083a427c <  (int)(DAT_07ea5b24 + 0xbe) &&
        (int)DAT_083a4278 >= (int)DAT_07ea5b28 &&
        (int)DAT_083a4278 <  (int)(DAT_07ea5b28 + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    const char* const heroName = DAT_07abf5d8 ? DAT_07abf5d8 + 449 : "";
    const bool heroLeads = PartyNumber > 0 && strcmp((const char*)Party, heroName) == 0;
    for (int row = 0; row < PartyNumber; ++row) {
        BYTE* const member = Party + row * 36;
        const bool mayRemove = heroLeads || strcmp((const char*)member, heroName) == 0;
        const int rowY = (int)DAT_07ea5b28 + 55 + row * 35;
        if (mayRemove && (int)DAT_083a427c >= (int)DAT_07ea5b24 + 140 &&
            (int)DAT_083a427c < (int)DAT_07ea5b24 + 164 &&
            (int)DAT_083a4278 >= rowY && (int)DAT_083a4278 < rowY + 24 &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            FUN_00404bc0(0x19, 0, 0);
            // The original click gate uses the visual row.  MuEmu validates
            // the wire party number, which is explicitly supplied by 0x42.
            const BYTE pkt[4] = { 0xC1, 0x04, 0x43, member[11] };
            Net_SendC1Packet(pkt, sizeof(pkt));
            return;
        }
    }

    if ((int)DAT_083a427c >= (int)(DAT_07ea5b24 + 0x19) &&
        (int)DAT_083a427c <  (int)(DAT_07ea5b24 + 0x31) &&
        (int)DAT_083a4278 >= (int)(DAT_07ea5b28 + 0x18b) &&
        (int)DAT_083a4278 <  (int)(DAT_07ea5b28 + 0x1a3) &&
        IsClickPushed()) {

        DAT_083a4124 = '\0';
        DAT_07eaa115 = '\0';
        FUN_00404bc0(0x19, 0, 0);
        FUN_00404bc0(0x1c, 0, 0);
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    }
}
// FUN_004e5de0 @ 0x004E5DE0 — SecondPassword_Screen3 (443 lines)
// Envío del paquete de selección de personaje: por cada uno de los hasta 0x10 botones de personaje de la columna,
// chequea si el mouse clickeó en el área de la fila [DAT_07ea982c+0x7d,+0x95) x [DAT_07ea9830+0x73+row*15, +0x18).
// Guard de entrada: DAT_07eaa116 tiene que ser distinto de cero. También chequea *(short*)(DAT_07cf1ff4+0x54) != 0.
// On click: sends 4-byte packet {0xC1,0x01,0x00,opcode_xored}.
//   opcode plain byte = (byte)(row_index), XOR key[3]=0x89, prev_plain_at[4]=4 → cipher = row^0x89^4
//   Después appendea el byte del índice de fila como byte 4 (si total_len+1 < 0x401).
//   El largo del paquete depende de los datos de cada fila. Clave: la misma de 32 bytes.
// After click: FUN_00404bc0(0x19,0,0).
// HashTable ref-count noise around DAT_07cf1ffc is anti-tamper, skipped.
// Back button [DAT_07ea982c+0x19,+0x31) x [DAT_07ea9830+0x18b,+0x1a3):
//   → clear DAT_07eaa116, FUN_00404bc0(0x19/0x1c), DAT_07e11d28=0, DAT_00559bec=6.
void __cdecl FUN_004e5de0(void) {
    // 2026-05-04: BUG-FIX phantom click. IDA sub_4E5DE0 = Character panel
    // hit-test, usa CharacterInfoStartX/Y como base. Nuestro port usa
    // uninitialized DAT_07ea982c/30 (=0) → fake hit-tests at left side fire
    // FUN_00404bc0 click sfx whenever Character is open. Skip until
    // CharacterInfoStartX también se cablea acá; el RenderCharacterInfoWindow de HUD_Pass6
    // already handles [+] stat-add and other panel clicks.
    if (DAT_07eaa116 == '\0' || DAT_07ea982c == 0) return;

    // Mouse hover zone
    if ((int)DAT_083a427c >= (int)DAT_07ea982c &&
        (int)DAT_083a427c <  (int)(DAT_07ea982c + 0xbe) &&
        (int)DAT_083a4278 >= (int)DAT_07ea9830 &&
        (int)DAT_083a4278 <  (int)(DAT_07ea9830 + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    // Clave XOR (32 bytes, la misma que se usa en todo el archivo)
    static const BYTE key[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,
        0x49,0x5d,0x39,0x5d,0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    int iRow     = 0;
    int iRowAccum = 0;
    int iX = DAT_083a427c;
    int iY = DAT_083a4278;

    // ── Botones [+] de stats ─────────────────────────────────────────────────
    // 2026-08-08 FIX (subir un punto DESCONECTABA): esta rama estaba mal portada
    // en tres cosas y sólo no se notaba porque el early-return por
    // `DAT_07ea982c == 0` la mantenía muerta (CharacterInfoStartX nunca se
    // escribía). Al arreglar el origen del panel, la rama despertó y mandó un
    // paquete basura → el server ve `size < 3` ("Protocol size error") y cierra
    // el socket. En el log: `AUTO-ENCRYPT C1 len=5 plain=[C1 01 00 34 03]`
    // seguido de FD_CLOSE 62 ms después.
    // Lo que estaba mal vs IDA sub_4E5DE0 L85-244:
    //   1. Paso de fila 15 px y 16 filas → son **60 px y 4 filas** (`v8 += 60;
    //      while (v8 < 240)`) = Fuerza / Agilidad / Vitalidad / Energía.
    //   2. El paquete se armaba a mano como `[C1][01][00][xor][row]`. El
    //      original arma `[C1][len][F3][06][statIdx]` (buf[2]=0xC1, buf[4]=0xF3,
    //      el subcode 6 y el índice al final) = PMSG_LEVEL_UP_POINT_RECV
    //      (Protocol.h:128) — y sale por el camino C3 con serial.
    //   3. Se mandaba con `send()` crudo, sin serial ni CSM.
    // Gate `*(short*)(CharacterAttribute + 0x54) != 0` = LevelUpPoint, fiel.
    if (*(short *)((BYTE*)DAT_07cf1ff4 + 0x54) != 0) {
        while (iRowAccum < 240) {
            int rowY = iRowAccum + (int)DAT_07ea9830 + 115;
            if (iX >= (int)(DAT_07ea982c + 125) &&
                iX <  (int)(DAT_07ea982c + 149) &&
                iY >= rowY && iY < rowY + 24 &&
                IsClickPushed()) {

                DAT_083a4124 = '\0';

                BYTE pkt[5] = { 0xC1, 5, 0xF3, 0x06, (BYTE)iRow };
                Net_SendSmallPacket(pkt, 5);

                FUN_00404bc0(0x19, 0, 0);
                iX = DAT_083a427c;
                iY = DAT_083a4278;
            }
            iRowAccum += 60;   // IDA `v8 += 60` (4 filas: Str/Agi/Vit/Ene)
            iRow++;
        }
    }

    // Back button
    if ((int)iX >= (int)(DAT_07ea982c + 0x19) &&
        (int)iX <  (int)(DAT_07ea982c + 0x31) &&
        (int)DAT_083a4278 >= (int)(DAT_07ea9830 + 0x18b) &&
        (int)DAT_083a4278 <  (int)(DAT_07ea9830 + 0x1a3) &&
        IsClickPushed()) {

        DAT_083a4124 = '\0';
        DAT_07eaa116 = '\0';
        FUN_00404bc0(0x19, 0, 0);
        FUN_00404bc0(0x1c, 0, 0);
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    }
}
// FUN_004e6550 @ 0x004E6550 — SecondPassword_Screen4 (257 lines)
//   - Wrong-password error handler: displays FUN_005142d0 error dialog for various
//     server error codes. Resets PIN buffer (DAT_07ea9814/18/1c = 0).
//   - Contiene una llamada a __ftol() (FPU ST0 → long) para el límite Y del botón, con origen float desconocido.
//     Casi toda la lógica (ruido de ref-count de HashTable + zonas de hover) está limpia, pero el origen del FPU
//     unresolvable without call-site context. STUB: __ftol() button region.
void __cdecl FUN_004e6550(void) {
    // SecondPassword_Screen4 — wrong-password error handler + char-list item visibility reset
    // Setea DAT_07d78094=1 si el mouse está sobre el panel principal, resetea los arrays de visibilidad por personaje,
    // y llama a FUN_004d1fc0 o FUN_004d23b0 para renderizar la grilla.
    // __ftol() Y-boundary → approximated as (int)DAT_07ea5284 (screen origin Y).

    if ((int)DAT_07ea5288 <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5288 + 0xbe) &&
        (int)DAT_07ea5284 <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea5284 + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa118);
    char sv1 = DAT_07eaa118;
    {
        uint uVar4 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_07eaa118);
        if (uVar4 != 0xffffffff) {
            BYTE* pbVar5 = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_07eaa118);
            BYTE bVar1 = pbVar5[1];
            pbVar5[1] = bVar1 - 1;
            if ((BYTE)(bVar1 - 1) == 0) FUN_00423710(pbVar5, &DAT_07eaa118);
        }
    }
    if ((sv1 != '\0') &&
        (int)DAT_07eaa0c8 <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07eaa0c8 + 0xbe) &&
        (int)DAT_07eaa0cc <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07eaa0cc + 0x1b1)) {
        DAT_07d78094 = 1;
    }
    if (DAT_07eaa119 != '\0' &&
        (int)DAT_07eaa0c8 <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07eaa0c8 + 0xbe) &&
        (int)DAT_07eaa0cc <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07eaa0cc + 0x1b1)) {
        DAT_07d78094 = 1;
    }
    if (DAT_07eaa11a != '\0' &&
        (int)DAT_07eaa0c8 <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07eaa0c8 + 0xbe) &&
        (int)DAT_07eaa0cc <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07eaa0cc + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b);
    char sv2 = DAT_07eaa11b;
    {
        uint uVar4 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_07eaa11b);
        if (uVar4 != 0xffffffff) {
            BYTE* pbVar5 = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_07eaa11b);
            BYTE bVar1 = pbVar5[1];
            pbVar5[1] = bVar1 - 1;
            if ((BYTE)(bVar1 - 1) == 0) FUN_00423710(pbVar5, &DAT_07eaa11b);
        }
    }
    if (sv2 != '\0' &&
        (int)DAT_07ea5290 <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5290 + 0xbe) &&
        (int)DAT_07ea528c <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea528c + 0x1b1)) {
        DAT_07d78094 = 1;
    }
    if (DAT_07eaa11c != '\0' &&
        (int)DAT_07eaa0c8 <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07eaa0c8 + 0xbe) &&
        (int)DAT_07eaa0cc <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07eaa0cc + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    // Refresh char-list visibility arrays for all CharData slots (stride 0x44, 12 slots)
    for (int i = 0; i < 0x330; i += 0x44) {
        void* puVar8 = DAT_07cf1ffc;
        uint uVar4 = HashTable_GetIndex(&DAT_055c9bc8, (void*)DAT_07cf1ffc);
        if (uVar4 == 0xffffffff) {
            void* pvVar10 = operator_new(0x585);
            *(unsigned char*)((int)pvVar10 + 0x584) = 1;
            FUN_00403f80(&DAT_055c9bc8, pvVar10, puVar8);
        } else {
            uint uVar4b = HashTable_GetIndex(&DAT_055c9bc8, puVar8);
            void* puVar9 = (uVar4b == 0xffffffff) ? nullptr : *(void**)(DAT_055c9bcc + uVar4b * 4);
            char cVar2 = *(char*)((int)puVar9 + 0x584);
            *(BYTE*)((int)puVar9 + 0x584) = (BYTE)(cVar2 + 1);
            if ((BYTE)(cVar2 + 1) < 2) FUN_00404370(puVar8, puVar9);
        }
        // Update slot visibility based on item type == -1
        if (*(short*)((int)DAT_07cf1ffc + 0x218 + i) == -1)
            *(BYTE*)((int)DAT_07cf1ffc + 0x258 + i) = 0;
        else
            *(BYTE*)((int)DAT_07cf1ffc + 0x258 + i) = 1;

        uVar4 = HashTable_GetIndex(&DAT_055c9bc8, puVar8);
        if (uVar4 != 0xffffffff) {
            uint uVar4b = HashTable_GetIndex(&DAT_055c9bc8, puVar8);
            void* puVar9 = (uVar4b == 0xffffffff) ? nullptr : *(void**)(DAT_055c9bcc + uVar4b * 4);
            char cVar2 = *(char*)((int)puVar9 + 0x584);
            *(char*)((int)puVar9 + 0x584) = cVar2 - 1;
            if ((char)(cVar2 - 1) == '\0') FUN_00404400(puVar9, puVar8);
        }
    }

    // Render de la grilla: si Y >= __ftol() (aprox. DAT_07ea5284), renderiza la grilla completa; si no, vacía
    DAT_07eaa164 = 0;
    int lVal = (int)DAT_07ea5284;
    if ((int)DAT_083a4278 < lVal) {
        FUN_004d1fc0();
    } else {
        // 2026-05-08: bug-fix — el inv_base era `&DAT_07ea8410` (DWORD de 4
        // bytes en globals.cpp) heredado del IDA. En el binario original,
        // DAT_07ea8410 ES la base del pool de inventario; en nuestro build
        // OffsetInventoryItems es ese pool. Sin este fix el click handler
        // walkeaba 4 bytes de DAT_07ea8410 + globals adyacentes random como
        // si fueran items → "inventario lleno de fantasmas" + sin tooltip.
        FUN_004d23b0((char*)(uintptr_t)(DAT_07ea5288 + 0xf),
                     (int)(DAT_07ea5284 + 200),
                     (short*)OffsetInventoryItems, 8, 8, '\0');
    }

    DAT_07eaa138 = 0;
    // Char-count check and back-button
    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b);
    char sv3 = DAT_07eaa11b;
    FUN_00404040(&DAT_055c9bc8, &DAT_07eaa11b);
    if (sv3 == '\0') {
        FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa118);
        char sv4 = DAT_07eaa118;
        FUN_00404040(&DAT_055c9bc8, &DAT_07eaa118);
        if (sv4 == '\0' && DAT_07eaa119 == '\0' && DAT_07eaa11a == '\0' && DAT_07eaa128 == 0) {
            ushort uVar3 = *(ushort*)((int)DAT_07cf1ff4 + 0xe);
            if (0x31 < uVar3) {
                DAT_07eaa138 = 1;
                if ((int)(DAT_07ea5288 + 0x3c) <= (int)DAT_083a427c &&
                    (int)DAT_083a427c < (int)(DAT_07ea5288 + 0x54) &&
                    (int)(DAT_07ea5284 + 0x18b) <= (int)DAT_083a4278 &&
                    (int)DAT_083a4278 < (int)(DAT_07ea5284 + 0x1a3) &&
                    IsClickPushed()) {
                    DAT_083a4124 = '\0';
                    DAT_07eaa134 ^= 1;
                }
            }
            // Back button
            if ((int)(DAT_07ea5288 + 0x19) <= (int)DAT_083a427c &&
                (int)DAT_083a427c < (int)(DAT_07ea5288 + 0x31) &&
                (int)(DAT_07ea5284 + 0x18b) <= (int)DAT_083a4278 &&
                (int)DAT_083a4278 < (int)(DAT_07ea5284 + 0x1a3) &&
                IsClickPushed()) {
                DAT_083a4124 = '\0';
                DAT_07eaa117 = 0;
                FUN_004cba60();
                DAT_07e11d28 = 0;
                DAT_00559bec = 6;
            }
        }
    }

    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b);
    char sv5 = DAT_07eaa11b;
    FUN_00404040(&DAT_055c9bc8, &DAT_07eaa11b);
    if (sv5 != '\0') {
        // 2026-05-08: trade — DAT_07ea5298 / DAT_07ea7b88 son DWORDs (4 bytes)
        // en globals.cpp pero en el binario original son las bases de los
        // pools de trade. Tal como 004D23B0: Inventory es la oferta remota
        // de sólo lectura (arriba) y OffsetTradeItems la oferta local
        // editable (abajo).
        FUN_004d23b0((char*)(uintptr_t)(DAT_07ea5290 + 0xf),
                     (int)(DAT_07ea528c + 0x46),
                     (short*)Inventory, 8, 4, '\0');
        FUN_004d23b0((char*)(uintptr_t)(DAT_07ea5290 + 0xf),
                     (int)(DAT_07ea528c + 0x10e),
                     (short*)OffsetTradeItems, 8, 4, '\0');
    }
    // FIX 2026-07-25: el render de shop/warehouse/chaos (RenderShopInterface
    // HUD_Pass6:1358) usa dword_7EAA0C8=260 / dword_7EAA0CC=0 (símbolo separado
    // del global DAT_07eaa0c8 en el port). El hover de abajo usa DAT_07eaa0c8,
    // que valía 0 → el tooltip caía en top-left (sx=35) en vez de sobre el item.
    // Sincronizamos el global con los valores del render para que coincidan.
    DAT_07eaa0c8 = 260;
    DAT_07eaa0cc = 0;
    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa118);
    char sv6 = DAT_07eaa118;
    FUN_00404040(&DAT_055c9bc8, &DAT_07eaa118);
    if (sv6 != '\0') {
        // FIX 2026-07-25: era copy-paste del branch de Warehouse (usaba
        // OffsetWarehouseItems → el hover leía un slot basura y el tooltip
        // mostraba un item que no está en la tienda, ej "Kris"). El pool de
        // TIENDA es el overlay &Inventory[32].WalkSpeed (offset +24), el mismo
        // que usa el render (HUD_Pass3:382 sub_4E38B0). Grid 8×15.
        FUN_004d23b0((char*)(uintptr_t)(DAT_07eaa0c8 + 0xf),
                     (int)(DAT_07eaa0cc + 0x32),
                     (short*)ShopItems, 8, 0xf, '\x01');
    }
    if (DAT_07eaa119 != '\0') {
        FUN_004d23b0((char*)(uintptr_t)(DAT_07eaa0c8 + 0xf),
                     (int)(DAT_07eaa0cc + 0x32),
                     (short*)OffsetWarehouseItems, 8, 0xf, '\0');
    }
    if (DAT_07eaa11a != '\0' && DAT_07eaa140 != 1) {
        // Chaos mix: 8x4 grid en OffsetMixItems.
        FUN_004d23b0((char*)(uintptr_t)(DAT_07eaa0c8 + 0xf),
                     (int)(DAT_07eaa0cc + 0x6e),
                     (short*)OffsetMixItems, 8, 4, '\0');
    }
}
// FUN_004e6c40 @ 0x004E6C40 — SecondPassword_Screen5 (783 lines)
//   - Main second-password entry UI: draws 10-button numeric keypad, handles click
//     (appends digit to DAT_07ea9814 buffer), Enter → sends packet, ESC → cancel.
//   - SEH. Implemented in SecondPassword_UI.cpp.
void __cdecl FUN_004e6c40(void) {
    // SecondPassword_Screen5 — Main numeric PIN keypad
    // Guard: DAT_07eaa11c must be non-zero
    if (DAT_07eaa11c == '\0') return;

    if (DAT_07eaa120 != 0) {
        if (DAT_07eaa120 != 1) return;
        // mode==1: resetea el origen a (0x104, 0) y vuelve a chequear
        DAT_07eaa0c8 = 0x104;
        DAT_07eaa0cc = 0;
        if (!IsClickPushed()) { DAT_07eaa0c8 = 0x104; DAT_07eaa0cc = 0; return; }
        // Itera hasta 6 botones de dígito (el stride varía según move_type), busca el clickeado
        float fBase = 170.0f;
        for (int iStep = 0; iStep < 6; iStep++) {
            int moveType = (int)(*(BYTE*)(DAT_07abf5d8 + 0x1bc) & 7);
            int local_418 = (moveType != 3 ? 6 : 6) + iStep; // simplified
            // Check button position (approximation from decompile)
            float fY = fBase + (float)iStep * _DAT_005528e4;
            bool inX = (_DAT_00552c24 <= (float)DAT_083a427c && (float)DAT_083a427c < _DAT_00552c20);
            bool inY = (fY <= (float)DAT_083a4278 && (float)DAT_083a4278 < fY + _DAT_00552c1c);
            if (inX && inY) {
                DAT_083a4124 = 0;
                if (DAT_07e91388 != 0) { DAT_07eaa0c8 = 0x104; DAT_07eaa0cc = 0; DAT_083a4124 = 0; return; }
                int iSlot = FUN_00482d70(0x1b2, iStep + 1);
                if (iSlot == -1) { FUN_0051d6f0((char*)&DAT_07d685ec); return; }
                // Build and send digit packet (opcode 0x9A, 5 bytes)
                {
                    static const BYTE key[32] = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                                                  0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                                                  0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                                                  0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56};
                    BYTE pkt[5];
                    pkt[0] = 0xC1; pkt[1] = 1; pkt[2] = 0x9A; pkt[3] = 1;
                    // XOR encode
                    for (uint ui = 3; ui < 4; ui++) {
                        uint uk = ui & 0x1f;
                        pkt[ui] ^= key[uk] ^ pkt[ui+1];
                    }
                    pkt[4] = (char)iSlot + 0x0c;
                    // XOR encode byte 4
                    { uint uk = 4 & 0x1f; pkt[4] ^= key[uk] ^ pkt[5 > 4 ? 4 : 4]; }
                    // Send 5 bytes
                    int off = 0; unsigned int rem = 5;
                    if (DAT_055ca168 != 0xffffffff) {
                        do {
                            int r = send((SOCKET)DAT_055ca168, (char*)pkt+off, (int)(rem-off), 0);
                            if (r == -1) {
                                int e = WSAGetLastError();
                                if (e == WSAEWOULDBLOCK && (int)(DAT_055cc16c + rem) < 0x2001) {
                                    memcpy(DAT_055ca16c + DAT_055cc16c, pkt, rem);
                                    DAT_055cc16c += rem;
                                } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                                break;
                            }
                            if (r == 0) break;
                            if (DAT_055ce174) FUN_0043de60();
                            rem -= r; off += r;
                        } while ((int)rem > 0);
                    }
                }
                return;
            }
            fBase += _DAT_00552844;
            if (iStep > 4) return;
        }
        DAT_07eaa0c8 = 0x104; DAT_07eaa0cc = 0; return;
    }

    // mode==0: setea el origen y procesa los clicks en los botones de dígito del PIN
    DAT_07eaa0c8 = 0x104;
    DAT_07eaa0cc = 0;
    if (!IsClickPushed()) { DAT_07eaa0c8 = 0x104; DAT_07eaa0cc = 0; return; }

    int iStep2 = 0;
    float fY2 = _DAT_00552c28;
    while (!(_DAT_00552c24 <= (float)DAT_083a427c && (float)DAT_083a427c < _DAT_00552c20 &&
             fY2 <= (float)DAT_083a4278 && (float)DAT_083a4278 < fY2 + _DAT_00552a2c)) {
        fY2 += _DAT_00552844;
        iStep2++;
        if (iStep2 > 3) { DAT_07eaa0c8 = 0x104; DAT_07eaa0cc = 0; return; }
    }
    DAT_083a4124 = 0;
    if (DAT_07e91388 != 0) { DAT_07eaa0c8 = 0x104; DAT_07eaa0cc = 0; DAT_083a4124 = 0; return; }

    // Determina el nivel de equipo y lo compara contra el umbral
    uint uVar5 = (uint)*(ushort*)((int)DAT_07cf1ff4 + 0xe);
    if ((*(BYTE*)((int)DAT_07cf1ff4 + 0xb) & 7) == 3) uVar5 = ((uVar5 + 1) / 2) * 3;

    if ((int)(&DAT_00559f64)[iStep2 * 2] < (int)uVar5) {
        // Level too low — send cancel (C1/03/31) and show "level too low" message
        DAT_07eaa117 = 0;
        FUN_004cba60();
        BYTE pkt[3] = {0xC1, 3, 0x31};
        int off = 0; unsigned int rem = 3;
        if (DAT_055ca168 != 0xffffffff) {
            do {
                int r = send((SOCKET)DAT_055ca168, (char*)pkt+off, (int)(rem-off), 0);
                if (r == -1) {
                    int e = WSAGetLastError();
                    if (e == WSAEWOULDBLOCK && (int)(DAT_055cc16c + rem) < 0x2001) {
                        memcpy(DAT_055ca16c + DAT_055cc16c, pkt, rem);
                        DAT_055cc16c += rem;
                    } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                    break;
                }
                if (r == 0) break;
                if (DAT_055ce174) FUN_0043de60();
                rem -= r; off += r;
            } while ((int)rem > 0);
        }
        FUN_004cd3b0();
        FUN_0051d6f0((char*)&DAT_07d5c10c);
    } else if ((int)uVar5 < (int)(&DAT_00559f60)[iStep2 * 2]) {
        // Level too high — send cancel and show "level too high" message
        DAT_07eaa117 = 0;
        FUN_004cba60();
        BYTE pkt[3] = {0xC1, 3, 0x31};
        int off = 0; unsigned int rem = 3;
        if (DAT_055ca168 != 0xffffffff) {
            do {
                int r = send((SOCKET)DAT_055ca168, (char*)pkt+off, (int)(rem-off), 0);
                if (r == -1) {
                    int e = WSAGetLastError();
                    if (e == WSAEWOULDBLOCK && (int)(DAT_055cc16c + rem) < 0x2001) {
                        memcpy(DAT_055ca16c + DAT_055cc16c, pkt, rem);
                        DAT_055cc16c += rem;
                    } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                    break;
                }
                if (r == 0) break;
                if (DAT_055ce174) FUN_0043de60();
                rem -= r; off += r;
            } while ((int)rem > 0);
        }
        FUN_004cd3b0();
        FUN_0051d6f0((char*)&DAT_07d5c238);
    } else {
        // Nivel dentro del rango — intenta encontrar el slot de dígito y manda el paquete opcode 0x90
        int iSlot = FUN_00482d70(0x1d3, 0);
        if (iSlot == -1) iSlot = FUN_00482d70(0x1d3, iStep2 + 1);
        if (iSlot == -1) {
            FUN_0051d6f0((char*)&DAT_07d5b680);
        } else {
            static const BYTE key[32] = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                                          0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                                          0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                                          0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56};
            BYTE pkt[5];
            pkt[0] = 0xC1; pkt[1] = 1; pkt[2] = 0x90; pkt[3] = 1;
            for (uint ui = 3; ui < 4; ui++) { uint uk = ui & 0x1f; pkt[ui] ^= key[uk] ^ pkt[ui+1]; }
            pkt[4] = (char)iSlot + 0x18;
            { uint uk = 4 & 0x1f; pkt[4] ^= key[uk] ^ pkt[4]; }
            int off = 0; unsigned int rem = 5;
            if (DAT_055ca168 != 0xffffffff) {
                do {
                    int r = send((SOCKET)DAT_055ca168, (char*)pkt+off, (int)(rem-off), 0);
                    if (r == -1) {
                        int e = WSAGetLastError();
                        if (e == WSAEWOULDBLOCK && (int)(DAT_055cc16c + rem) < 0x2001) {
                            memcpy(DAT_055ca16c + DAT_055cc16c, pkt, rem);
                            DAT_055cc16c += rem;
                        } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                        break;
                    }
                    if (r == 0) break;
                    if (DAT_055ce174) FUN_0043de60();
                    rem -= r; off += r;
                } while ((int)rem > 0);
            }
        }
    }
}
// FUN_004e7ac0 @ 0x004E7AC0 — SecondPassword_Screen6 (854 lines)
//   - Segunda contraseña del char-select: parecido a Screen5 pero para el flujo de selección de personaje.
//     Checks DAT_07eaa14c mode (2=new PIN, 3=confirm PIN, 6=set-mode).
//   - SEH. Implemented in SecondPassword_UI.cpp.
void __cdecl FUN_004e7ac0(void) {
    // SecondPassword_Screen6 — CharSelect second-password digit input
    // Guard: DAT_07eaa128 must be non-zero
    if (DAT_07eaa128 == 0) return;

    // Mouse hover zone 0x1c2-0x280 x 0-0x1b1
    if ((0x1c1 < (int)DAT_083a427c) && ((int)DAT_083a427c < 0x280) &&
        (-1 < (int)DAT_083a4278) && ((int)DAT_083a4278 < 0x1b1)) {
        DAT_07d78094 = 1;
    }

    // mode==4: final submit button (0x1e4-0x25d, 0xdb-0xf4)
    if (DAT_07eaa128 == 4) {
        if ((0x1e4 < (int)DAT_083a427c) && ((int)DAT_083a427c < 0x25d) &&
            (0xdb < (int)DAT_083a4278) && ((int)DAT_083a4278 < 0xf4) &&
            DAT_083a413c != '\0') {
            // Build and send 4-byte packet C1/01/98
            static const BYTE key[32] = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                                          0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                                          0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                                          0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56};
            BYTE pkt[4];
            pkt[0] = 0xC1; pkt[1] = 1; pkt[2] = 0x98; pkt[3] = 1;
            for (uint ui = 3; ui < 4; ui++) { uint uk = ui & 0x1f; pkt[ui] ^= key[uk] ^ pkt[ui+1]; }
            int off = 0; unsigned int rem = 4;
            if (DAT_055ca168 != 0xffffffff) {
                do {
                    int r = send((SOCKET)DAT_055ca168, (char*)pkt+off, (int)(rem-off), 0);
                    if (r == -1) {
                        int e = WSAGetLastError();
                        if (e == WSAEWOULDBLOCK && (int)(DAT_055cc16c + rem) < 0x2001) {
                            memcpy(DAT_055ca16c + DAT_055cc16c, pkt, rem);
                            DAT_055cc16c += rem;
                        } else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                        break;
                    }
                    if (r == 0) break;
                    if (DAT_055ce174) FUN_0043de60();
                    rem -= r; off += r;
                } while ((int)rem > 0);
            }
            DAT_083a413c = '\0';
        }
        goto LAB_004e8950_impl;
    }

    {
        int iX9 = (DAT_07eaa128 != 3) ? 0x1e5 : 0x1c2;
        int iW9 = 0x78, iBY9 = 0x16;

        if (DAT_07eaa128 != 3) {
            iX9 = 0x1e5; iW9 = 0x78; iBY9 = 0x16;
            // Digit button (0x1e4-0x25d, 0xe6-0xfd)
            if ((0x1e4 < (int)DAT_083a427c) && ((int)DAT_083a427c < 0x25d) &&
                (0xe6 < (int)DAT_083a4278) && ((int)DAT_083a4278 < 0xfd) &&
                DAT_083a413c != '\0') {
                DAT_083a413c = '\0';
                int iSlot = FUN_00482d70(0x1d5, (int)DAT_07eaa128 - 1);
                if (iSlot != -1) {
                    // Send 5-byte packet C1/01/95/iSlot
                    static const BYTE key[32] = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                                                  0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                                                  0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                                                  0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56};
                    BYTE pkt[5];
                    pkt[0] = 0xC1; pkt[1] = 1; pkt[2] = 0x95; pkt[3] = 1;
                    for (uint ui=3;ui<4;ui++){uint uk=ui&0x1f;pkt[ui]^=key[uk]^pkt[ui+1];}
                    pkt[4] = (char)iSlot;
                    {uint uk=4&0x1f;pkt[4]^=key[uk]^pkt[4];}
                    int off=0; unsigned int rem=5;
                    if (DAT_055ca168 != 0xffffffff) {
                        do {
                            int r=send((SOCKET)DAT_055ca168,(char*)pkt+off,(int)(rem-off),0);
                            if (r==-1){int e=WSAGetLastError();if(e==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pkt,rem);DAT_055cc16c+=rem;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                            if(r==0)break;if(DAT_055ce174)FUN_0043de60();rem-=r;off+=r;
                        } while ((int)rem > 0);
                    }
                }
            }
        } else {
            // mode==3: confirm-PIN digit button (uses __ftol Y boundaries → approx 0xdb-0xf4, 0x13f-0x158)
            // Las dos llamadas a __ftol() se aproximan como (int)DAT_07ea5288 para X y constantes fijas para Y
            int lX3 = (int)DAT_07ea5288;
            if ((lX3 <= (int)DAT_083a427c) && ((int)DAT_083a427c < lX3 + 0x78) &&
                (0x13f < (int)DAT_083a4278) && ((int)DAT_083a4278 < 0x158) &&
                DAT_083a413c != '\0') {
                DAT_083a413c = '\0';
                DAT_083a42c4 = 0;
                // CharSelect second-password lookup via GetItemCount(-1, -1)
                int cnt = FUN_00482ff0(0x1d5, 0xffffffff);
                if (cnt == 0) {
                    FUN_0051d6f0((char*)&DAT_07d6b724);
                } else {
                    // Build 0x9D packet with char data (13 bytes total)
                    DWORD uVar1 = *(DWORD*)(&DAT_07db8714 + DAT_07e11d78 * 0x100);
                    int local_428 = *(int*)(&DAT_07db8718 + DAT_07e11d78 * 0x80);
                    static const BYTE key[32] = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                                                  0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                                                  0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                                                  0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56};
                    BYTE pkt[13];
                    pkt[0] = 0xC1; pkt[1] = 1; pkt[2] = 0x9D; pkt[3] = 1;
                    pkt[4] = 0; pkt[5] = 0; pkt[6] = 0; pkt[7] = 0; pkt[8] = 0;
                    // XOR encode bytes 3..7
                    for (uint ui=3;ui<8;ui++){uint uk=ui&0x1f;pkt[ui]^=key[uk]^pkt[ui+1];}
                    memcpy(pkt+8, &uVar1, 4);
                    pkt[12] = 0;
                    // XOR encode 8..12
                    for (uint ui=8;ui<13;ui++){uint uk=ui&0x1f;pkt[ui]^=key[uk]^(ui+1<13?pkt[ui+1]:0);}
                    memcpy(pkt+8, &local_428, 4);
                    pkt[12] = 0;
                    for (uint ui=8;ui<13;ui++){uint uk=ui&0x1f;pkt[ui]^=key[uk]^(ui+1<13?pkt[ui+1]:0);}
                    int off=0; unsigned int rem=13;
                    if (DAT_055ca168 != 0xffffffff) {
                        do {
                            int r=send((SOCKET)DAT_055ca168,(char*)pkt+off,(int)(rem-off),0);
                            if(r==-1){int e=WSAGetLastError();if(e==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pkt,rem);DAT_055cc16c+=rem;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                            if(r==0)break;if(DAT_055ce174)FUN_0043de60();rem-=r;off+=r;
                        } while((int)rem>0);
                    }
                }
            }
        }

        // mode==1: OK button at fixed position
        if (DAT_07eaa128 == 1) {
            if ((iX9 <= (int)DAT_083a427c) && ((int)DAT_083a427c < iW9 + iX9) &&
                (0xfe < (int)DAT_083a4278) && ((int)DAT_083a4278 < iBY9 + 0xff) &&
                DAT_083a413c != '\0') {
                DAT_083a413c = '\0';
                int cnt2 = FUN_00482ff0(0x1d5, 0xffffffff);
                if (cnt2 < 10) {
                    // Short send 3-byte C1/01/96
                    if (_DAT_00559f58 != -1 && DAT_00559f5c != -1) {
                        // Proceed
                        goto LAB_Screen6_ok_send;
                    }
                    goto LAB_004e83a5_impl;
                }
            LAB_Screen6_ok_send:
                {
                    BYTE pkt2[3] = {0xC1, 3, 0x96};
                    int off=0; unsigned int rem=3;
                    if (DAT_055ca168 != 0xffffffff) {
                        do {
                            int r=send((SOCKET)DAT_055ca168,(char*)pkt2+off,(int)(rem-off),0);
                            if(r==-1){int e=WSAGetLastError();if(e==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pkt2,rem);DAT_055cc16c+=rem;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                            if(r==0)break;if(DAT_055ce174)FUN_0043de60();rem-=r;off+=r;
                        } while((int)rem>0);
                    }
                }
            }
        }
    LAB_004e83a5_impl:
        if (_DAT_00559f58 != -1 &&
            (int)(WORD)_DAT_00559f58 != -1 && DAT_00559f5c != -1 &&
            (iX9 <= (int)DAT_083a427c) && ((int)DAT_083a427c < 0x78 + iX9) &&
            (0xc9 < (int)DAT_083a4278) && ((int)DAT_083a4278 < 0xca + iBY9) &&
            DAT_083a413c != '\0') {
            DAT_083a413c = '\0';
            FUN_0051d780(0x2c9, 5);
        }
    }

LAB_004e8950_impl:
    // Botón atrás: DAT_07ea5288+0x19 a +0x31 x DAT_07ea5284+0x18b a +0x1a3
    if ((int)(DAT_07ea5288 + 0x19) <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5288 + 0x31) &&
        (int)(DAT_07ea5284 + 0x18b) <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea5284 + 0x1a3) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        // Send C1/03/97 cancel packet
        BYTE pkt3[3] = {0xC1, 3, 0x97};
        int off=0; unsigned int rem=3;
        if (DAT_055ca168 != 0xffffffff) {
            do {
                int r=send((SOCKET)DAT_055ca168,(char*)pkt3+off,(int)(rem-off),0);
                if(r==-1){int e=WSAGetLastError();if(e==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pkt3,rem);DAT_055cc16c+=rem;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                if(r==0)break;if(DAT_055ce174)FUN_0043de60();rem-=r;off+=r;
            } while((int)rem>0);
        }
        DAT_07eaa128 = 0;
        DAT_07eaa117 = 0;
        FUN_004cba60();
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        Input_ClearState(0);
        DAT_00559c84 = 0;
        DAT_07e11d72 = 0;
        DAT_07e11d74 = 0;
        DAT_07eaa108 = 0;
        DAT_07e11d73 = 0;
    }

    if (DAT_083a413c != '\0') DAT_083a413c = '\0';
}
// FUN_004e8b70 @ 0x004E8B70 — SecondPassword_Screen7 (183 lines)
//   - Contiene dos llamadas a __ftol() (FPU ST0 → long) para calcular el límite X del
//     checkbox y del botón OK. El float en ST0 lo setea un callee anterior y Ghidra no lo trackea.
//     La lógica del botón "atrás" y las llamadas a send() están limpias, pero los dos hit-tests
//     que dependen de __ftol() no se pueden expresar sin el valor de la FPU.
//   STUB: two __ftol() button boundaries unresolvable — keep as empty body.
void __cdecl FUN_004e8b70(void) {
    // SecondPassword_Screen7 — timeout / retry handler
    // Guard: DAT_07eaa130 must be non-zero
    // Llamadas a __ftol() → aproximadas como (int)DAT_07ea5288 (origen X de pantalla)
    int iVar5 = DAT_083a427c;
    int iVar9 = DAT_083a4278;
    if (DAT_07eaa130 == '\0') return;

    // Mouse hover check 0x1c2-0x280 x 0-0x1b1
    if ((0x1c1 < iVar5) && (iVar5 < 0x280) && (-1 < iVar9) && (iVar9 < 0x1b1))
        DAT_07d78094 = 1;

    // Checkbox click: DAT_07ea5288+0x19 to +0x29, 0xef-0x100
    if ((int)(DAT_07ea5288 + 0x19) <= iVar5 && iVar5 < (int)(DAT_07ea5288 + 0x29) &&
        (0xef < iVar9) && (iVar9 < 0x100) && IsClickPushed()) {
        DAT_083a4124 = '\0';
        DAT_07eaa131 ^= 1;
        DAT_083a42c4 = 0;
    }

    if (DAT_07eaa131 != 0) {
        // OK button: __ftol() X boundary → (int)DAT_07ea5288, Y 0x13f-0x158
        int lX = (int)DAT_07ea5288;
        if ((lX <= iVar5) && (iVar5 < lX + 0x78) &&
            (0x13f < iVar9) && (iVar9 < 0x158) && IsClickPushed()) {
            DAT_083a4124 = '\0';
            DAT_083a42c4 = 0;
            DAT_07eaa13c = 4;
            DAT_00559f5e = (char)0xff;
            FUN_0051e240(1, 0x1c0, 0x97);
            iVar5 = DAT_083a427c;
            iVar9 = DAT_083a4278;
        }
    }

    // Cancel button: __ftol() X → (int)DAT_07ea5288, Y 0x15d-0x176
    {
        int lX2 = (int)DAT_07ea5288;
        if ((lX2 <= iVar5) && (iVar5 < lX2 + 0x78) &&
            (0x15d < iVar9) && (iVar9 < 0x176) && IsClickPushed()) {
            DAT_083a4124 = '\0';
            DAT_083a42c4 = 0;
            DAT_07eaa117 = 0;
            FUN_004cba60();
            // Send C1/03/31 cancel packet
            BYTE pkt[3] = {0xC1, 3, 0x31};
            int off=0; unsigned int rem=3;
            if (DAT_055ca168 != 0xffffffff) {
                do {
                    int r=send((SOCKET)DAT_055ca168,(char*)pkt+off,(int)(rem-off),0);
                    if(r==-1){int e=WSAGetLastError();if(e==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pkt,rem);DAT_055cc16c+=rem;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                    if(r==0)break;if(DAT_055ce174)FUN_0043de60();rem-=r;off+=r;
                } while((int)rem>0);
            }
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
            iVar5 = DAT_083a427c;
            iVar9 = DAT_083a4278;
        }
    }

    // Botón atrás (el principal): DAT_07ea5288+0x19 a +0x31 x DAT_07ea5284+0x18b a +0x1a3
    if ((int)(DAT_07ea5288 + 0x19) <= iVar5 &&
        iVar5 < (int)(DAT_07ea5288 + 0x31) &&
        (int)(DAT_07ea5284 + 0x18b) <= iVar9 &&
        iVar9 < (int)(DAT_07ea5284 + 0x1a3) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        // Send C1/01/31 keepalive-style packet
        BYTE pkt2[3] = {0xC1, 3, 0x31};
        int off=0; unsigned int rem=3;
        if (DAT_055ca168 != 0xffffffff) {
            do {
                int r=send((SOCKET)DAT_055ca168,(char*)pkt2+off,(int)(rem-off),0);
                if(r==-1){int e=WSAGetLastError();if(e==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pkt2,rem);DAT_055cc16c+=rem;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                if(r==0)break;if(DAT_055ce174)FUN_0043de60();rem-=r;off+=r;
            } while((int)rem>0);
        }
        DAT_07eaa128 = 0;
        DAT_07eaa117 = 0;
        FUN_004cba60();
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    }
}
// FUN_004e9050 @ 0x004E9050 — SecondPassword_Screen8 (81 lines)
// Handler del click en el botón OK del diálogo de segunda contraseña.
// Guard: DAT_07eaa11a must be non-zero (button active flag).
// Hit-test: mouse within [DAT_07eaa0c8+0x4b, DAT_07eaa0c8+0x77) x [DAT_07eaa0cc+300, DAT_07eaa0cc+0x14c).
// DAT_07eaa140 must be 0 (no timeout in progress), DAT_083a4124 must be non-zero (click pending).
// Switch on DAT_07eaa16c:
//   0         → FUN_00480620(&DAT_07eaa1a0, &DAT_07d544d4, 2) — show wrong-PIN message
//   1,2,3,4,5,6,7,8,0xb → FUN_004e3db0(0x7ea8410, 8, 8, iVar1, iVar3) — send auth
//     sub-switch: cases 1,7,0xb → iVar1=5 iVar3=4; case 8 → iVar1=2 iVar3=2; else → iVar1=DAT_0055a3f8 iVar3=DAT_0055a3fc
//     si tiene éxito (retorno distinto de cero) y DAT_07e91388 < 1: setea DAT_07eaa13c=2, DAT_00559f5e=0xff,
//     call FUN_0051e240(1, 0x21b, 0x97)
//   0xfffffff8, 0xfffffffe → FUN_00480620(&DAT_07eaa19c, &DAT_07d55c44, 2) — show error message
// After switch: FUN_00404bc0(0x19,0,0).
// "Back" button: [DAT_07ea5288+0x19,DAT_07ea5288+0x31) x [DAT_07ea5284+0x18b,DAT_07ea5284+0x1a3)
//   → FUN_004f6850(); DAT_07e11d28=0; DAT_00559bec=6; FUN_00404bc0(0x19,0,0).
void __cdecl FUN_004e9050(void) {
    if (DAT_07eaa11a == '\0') return;

    // OK button hit-test
    if (DAT_07eaa140 == 0 &&
        (int)DAT_083a427c >= (int)(DAT_07eaa0c8 + 0x4b) &&
        (int)DAT_083a427c <  (int)(DAT_07eaa0c8 + 0x77) &&
        (int)DAT_083a4278 >= (int)(DAT_07eaa0cc + 300) &&
        (int)DAT_083a4278 <  (int)(DAT_07eaa0cc + 0x14c) &&
        IsClickPushed()) {

        DAT_083a4124 = '\0';
        switch ((int)DAT_07eaa16c) {
        case 0:
            FUN_00480620(&DAT_07eaa1a0, &DAT_07d544d4, 2);
            break;
        case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 0xb: {
            int iVar1 = 5, iVar3 = 4;
            switch ((int)DAT_07eaa16c) {
            case 1: case 7: case 0xb:
                break; // iVar1=5, iVar3=4 (defaults)
            case 8:
                iVar1 = 2; iVar3 = 2;
                break;
            default:
                iVar1 = DAT_0055a3f8;
                iVar3 = DAT_0055a3fc;
                break;
            }
            // 004E3DB0 comprueba que exista espacio en el inventario normal
            // para el resultado; en el binario el primer argumento era la
            // dirección absoluta de ese pool, no un literal portable.
            uint uVar2 = FUN_004e3db0((int)(uintptr_t)OffsetInventoryItems, 8, 8, iVar1, iVar3);
            if ((char)uVar2 == '\0') {
                FUN_00480620(&DAT_07eaa198, &DAT_07d54600, 2);
            } else {
                if ((int)DAT_07e91388 < 1) {
                    DAT_07eaa13c = 2;
                    DAT_00559f5e = 0xff;
                    FUN_0051e240(1, 0x21b, 0x97);
                }
            }
            break;
        }
        case (int)0xfffffff8:
        case (int)0xfffffffe:
            FUN_00480620(&DAT_07eaa19c, &DAT_07d55c44, 2);
            break;
        }
        FUN_00404bc0(0x19, 0, 0);
    }

    // Back button hit-test
    if ((int)DAT_083a427c >= (int)(DAT_07ea5288 + 0x19) &&
        (int)DAT_083a427c <  (int)(DAT_07ea5288 + 0x31) &&
        (int)DAT_083a4278 >= (int)(DAT_07ea5284 + 0x18b) &&
        (int)DAT_083a4278 <  (int)(DAT_07ea5284 + 0x1a3) &&
        IsClickPushed()) {

        DAT_083a4124 = '\0';
        FUN_004f6850();
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        FUN_00404bc0(0x19, 0, 0);
    }
}
// FUN_004eb5d0 @ 0x004EB5D0 — SecondPassword_Screen9 (102 lines)
// Detección de fila del teclado numérico del diálogo de segunda contraseña.
// Chequea 4 filas de botones en los offsets x {0x1a, 0x4c, 0x7e, 0xd7} (0x18 de ancho cada uno)
// y los offsets y {0x186, 0x186, 0x186, 0x18b} relativos a DAT_07eaa0c8/0cc.
// Al hacer click (DAT_083a4124 != '\0'), despacha por índice de fila:
//   0 → new char: SetErrorMessage(0x74), Input_ClearState(0), set substate flags
//   1 → borrar carácter: igual que 0 pero DAT_07eaa108=1
//   2 → shuffle/new-PIN: shuffle DAT_07e91394 short[10] via Fisher-Yates (20 passes),
//        reset _DAT_07ea9814=0, DAT_07ea9818=0, DAT_07eaa14c = 4 - DAT_00559f5f,
//        DAT_07ea981c=0, DAT_07ea981e=0
//   3 → exit: FUN_004f6a70(); DAT_07e11d28=0; DAT_00559bec=6
void __cdecl FUN_004eb5d0(void) {
    if (DAT_07eaa119 == '\0') return;

    static const int xOff[4] = { 0x1a, 0x4c, 0x7e, 0xd7 };
    static const int yOff[4] = { 0x186, 0x186, 0x186, 0x18b };

    int iVar6 = 0;
    while (true) {
        int xMin = xOff[iVar6] + (int)DAT_07eaa0c8;
        int xMax = xMin + 0x18;
        int yMin = yOff[iVar6] + (int)DAT_07eaa0cc;
        int yMax = yMin + 0x18;
        if ((int)DAT_083a427c >= xMin && (int)DAT_083a427c < xMax &&
            (int)DAT_083a4278 >= yMin && (int)DAT_083a4278 < yMax)
            break;
        iVar6++;
        if (iVar6 > 3) return;
    }

    if (!IsClickPushed()) return;
    DAT_083a4124 = '\0';
    FUN_00404bc0(0x19, 0, 0);

    switch (iVar6) {
    case 0:
        SetErrorMessage(0x74);
        Input_ClearState(0);
        DAT_07e11d74 = 0;
        DAT_07eaa108 = 0;
        _DAT_00559c94 = 8;
        DAT_00559c88 = 1;
        DAT_00559c84 = 0;
        DAT_07e11d72 = 1;
        break;
    case 1:
        SetErrorMessage(0x74);
        Input_ClearState(0);
        DAT_07e11d74 = 0;
        _DAT_00559c94 = 8;
        DAT_00559c88 = 1;
        DAT_00559c84 = 0;
        DAT_07e11d72 = 1;
        DAT_07eaa108 = 1;
        break;
    case 2: {
        // Inicializa el array short[10] en DAT_07e91394 con 0..9 y después lo mezcla (Fisher-Yates, 20 pasadas)
        // Confirmado en Ghidra: el stride es psVar2+1 por iteración, el array termina antes de 0x7e913a8
        short *pArr = (short *)&DAT_07e91394;
        for (short s = 0; s < 10; s++) pArr[s] = s;
        for (int pass = 0x14; pass != 0; pass--) {
            int i = rand() % 10;
            int j = rand() % 10;
            if (i != j) {
                pArr[i] = (short)(pArr[i] ^ pArr[j]);
                pArr[j] = (short)(pArr[j] ^ pArr[i]);
                pArr[i] = (short)(pArr[i] ^ pArr[j]);
            }
        }
        _DAT_07ea9814 = 0.0f;
        *(DWORD*)&DAT_07ea9818 = 0;  // clear 4 bytes from DAT_07ea9818
        DAT_07eaa14c = 4u - (DAT_00559f5f != '\0' ? 1u : 0u);
        DAT_07ea981c = 0;
        DAT_07ea981e = 0;
        break;
    }
    case 3:
        FUN_004f6a70();
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        break;
    }
}
// IDA: FUN_004EB7F0 — entrada de Trade: Zen, confirmación y cancelación.
//   - Usa TradeInventoryStartX/Y y TradeOpened como guard de la interfaz.
//   - Los botones envían C3 C1:04:3C:01 y C3 C1:03:3D según corresponda.
void __cdecl FUN_004eb7f0(void) {
    // IDA: el guard lee TradeOpened; se omite únicamente el ruido de HashTable.

    FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b);
    char cGuard = DAT_07eaa11b;
    {
        uint uVar3 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_07eaa11b);
        if (uVar3 != 0xffffffff) {
            BYTE* pb = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_07eaa11b);
            BYTE b = pb[1]; pb[1] = b - 1;
            if ((BYTE)(b-1) == 0) FUN_00423710(pb, &DAT_07eaa11b);
        }
    }
    if (cGuard == '\0') return;

    // Botón Zen: [x+26,x+50) × [y+390,y+414).
    if ((int)(DAT_07ea5290 + 0x1a) <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5290 + 0x32) &&
        (int)(DAT_07ea528c + 0x186) <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea528c + 0x19e) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        FUN_005142d0(0x74);
        Input_ClearState(0);
        _DAT_00559c94 = 8;
        DAT_00559c88 = 1;
        DAT_00559c84 = 0;
        DAT_07e11d72 = 1;
        DAT_07e11d74 = 0;
        DAT_07eaa108 = 2;
        FUN_00404bc0(0x19, 0, 0);
    }

    // IDA: FUN_004EB7F0 — m_nMyTradeWait se descuenta una vez por tick
    // mientras Trade está abierto. No es DAT_07EAA104, que corresponde a otro
    // estado de selección de interfaz.
    if (TradeMyWait > 0) --TradeMyWait;

    // Botón de confirmación: [x+97,x+121) × [y+390,y+414).
    if (TradeMyWait == 0 && DAT_07e91388 == 0 &&
        (int)(DAT_07ea5290 + 0x61) <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5290 + 0x79) &&
        (int)(DAT_07ea528c + 0x186) <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea528c + 0x19e) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        FUN_00404bc0(0x19, 0, 0);

        if (DAT_07eaa0e8 == '\0') {
            if (DAT_07eaa0fd == '\0') {
                DAT_07eaa0fd = '\x01';
            } else {
                DAT_07eaa0fd = '\0';
            }
        } else {
            if (DAT_07eaa0fd == '\0') {
                DAT_07eaa13c = 3;
                DAT_00559f5e = (char)0xff;
                FUN_0051e240(4, 0x173, 0x97);
                return;
            }
            DAT_07eaa0fd = '\0';
        }
        DAT_07eaa0e8 = '\x01';

        // PMSG_TRADE_OK_BUTTON_RECV: C1:3C, flag=1. El emisor compartido
        // aporta correctamente el envoltorio C3, chain-XOR y serial rotativo.
        BYTE pkt[4] = { 0xC1, 0x04, 0x3C, 0x01 };
        Net_SendSmallPacket(pkt, sizeof(pkt));
    }

    // Botón de cancelación: [x+137,x+161) × [y+390,y+414).
    if ((int)(DAT_07ea5290 + 0x89) <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5290 + 0xa1) &&
        (int)(DAT_07ea528c + 0x186) <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea528c + 0x19e) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        FUN_00404bc0(0x19, 0, 0);
        if (DAT_07e91388 == 0) {
            DAT_07eaa0e8 = '\0';
            // CGTradeCancelButtonRecv: C1:3D, no payload.
            BYTE pkt3[3] = { 0xC1, 0x03, 0x3D };
            Net_SendSmallPacket(pkt3, sizeof(pkt3));
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }
}
// FUN_004ec330 @ 0x004EC330 — SecondPassword_Screen11 (389 lines)
//   - Cleanup / resource release: resets all DAT_07eaa1xx buffers, clears PIN state,
//     calls FUN_004cba60, resets DAT_07eaa14c=0, DAT_07eaa108=0.
//   - SEH. Implemented in SecondPassword_UI.cpp.
void __cdecl FUN_004ec330(void) {
    // SecondPassword_Screen11 — main checkbox/toggle panel + auth packet builder.
    // Handles: B-key area toggle (DAT_07eaa150), optional second checkbox (DAT_07eaa134),
    // Click del botón "OK" que arma y manda el paquete XOR completo del PIN (opcode 0x34/F1 de auth),
    // and "Cancel" / "Back" button. SEH frame stripped; HashTable noise stripped.

    uint uVar3 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_07eaa118);
    if (uVar3 == 0xffffffff) {
        void* pv = operator_new(2);
        *(unsigned char*)((int)pv + 1) = 1;
        FUN_00403f80(&DAT_055c9bc8, pv, &DAT_07eaa118);
    } else {
        BYTE* pb = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_07eaa118);
        BYTE b = pb[1]; pb[1] = b + 1;
        if ((BYTE)(b+1) < 2) FUN_00404330((BYTE*)&DAT_07eaa118, pb);
    }
    char cGuard = DAT_07eaa118;
    FUN_00404040(&DAT_055c9bc8, &DAT_07eaa118);

    if (cGuard != '\0') {
        if (DAT_07eaa132 != '\0') {
            // Checkbox 1 area: [DAT_07eaa0c8+0x19, +0x31) x [DAT_07eaa0cc+0x16d, +0x185)
            int iX1 = (int)DAT_07eaa0c8 + 0x19;
            int iY1 = (int)DAT_07eaa0cc + 0x16d;
            if (iX1 <= (int)DAT_083a427c && (int)DAT_083a427c < iX1 + 0x18 &&
                iY1 <= (int)DAT_083a4278 && (int)DAT_083a4278 < iY1 + 0x18 &&
                IsClickPushed()) {
                DAT_083a4124 = '\0';
                if (((BYTE*)&DAT_07eaa150)[2] == 0) {
                    ((BYTE*)&DAT_07eaa150)[2] = 2;
                    DAT_07eaa134 = 1;
                } else {
                    ((BYTE*)&DAT_07eaa150)[2] = 0;
                    DAT_07eaa134 = 0;
                }
            }
            // Checkbox 2 area: [+0x55, +0x6d) x same Y
            int iX2 = (int)DAT_07eaa0c8 + 0x55;
            if (iX2 <= (int)DAT_083a427c && (int)DAT_083a427c < iX2 + 0x18 &&
                iY1 <= (int)DAT_083a4278 && (int)DAT_083a4278 < iY1 + 0x18 &&
                IsClickPushed()) {
                DAT_07eaa134 ^= 1;
                DAT_083a4124 = '\0';
                ((BYTE*)&DAT_07eaa150)[2] = -(DAT_07eaa134 != 0) & 2;
            }

            // "OK" button: [+0x73, +0x8b) x [+0x16d, +0x185)
            int iX3 = (int)DAT_07eaa0c8 + 0x73;
            if (iX3 <= (int)DAT_083a427c && (int)DAT_083a427c < iX3 + 0x18 &&
                iY1 <= (int)DAT_083a4278 && (int)DAT_083a4278 < iY1 + 0x18 &&
                IsClickPushed()) {
                // Build full XOR-encoded PIN packet and send
                // Packet header: C1 / len / 34 (opcode) / payload ...
                static const BYTE key[32] = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
                                              0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
                                              0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
                                              0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56};
                BYTE hdr[3]; hdr[0]=0xC1; hdr[1]=1; hdr[2]=0x34;
                for (uint ui=3;ui!=4;ui++){uint uk=ui&0x1f;hdr[ui-3]^=key[uk]^hdr[ui-2];}

                BYTE rawbuf[1024];
                uint rawLen = (uint)(hdr[1] & 0xffff);
                if (rawLen + 1 < 0x401) {
                    BYTE pktBuf[1028];
                    // random tail byte
                    rawbuf[rawLen] = (BYTE)rand();
                    // counter byte
                    uint uCtr = (uint)(rawbuf[0] != 0xC1);
                    {
                        uint uVar7 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_05826ceb);
                        if (uVar7 == 0xffffffff) {
                            void* pv2 = operator_new(2);
                            *(unsigned char*)((int)pv2 + 1) = 1;
                            FUN_00403f80(&DAT_055c9bc8, pv2, &DAT_05826ceb);
                        } else {
                            BYTE* pb2 = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_05826ceb);
                            BYTE b2 = pb2[1]; pb2[1] = b2 + 1;
                            if ((BYTE)(b2+1) < 2) FUN_00404330(&DAT_05826ceb, pb2);
                        }
                    }
                    rawbuf[uCtr + 1] = DAT_05826ceb;
                    DAT_05826ceb = DAT_05826ceb + 1;
                    {
                        uint uVar7 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_05826ceb);
                        if (uVar7 != 0xffffffff) {
                            BYTE* pb2 = (BYTE*)FUN_00404280(&DAT_055c9bc8, &DAT_05826ceb);
                            BYTE b2 = pb2[1]; pb2[1] = b2 - 1;
                            if ((BYTE)(b2-1) == 0) FUN_00423710(pb2, (char*)&DAT_05826ceb);
                        }
                    }
                    int iPayLen = (int)rawLen - (int)(uCtr + 1);
                    BYTE* pbPay = rawbuf + uCtr + 1;
                    int encLen = FUN_0053cc30(0, pbPay, iPayLen);
                    if (encLen < 0x100) {
                        uint uSz = (uint)(encLen + 2);
                        pktBuf[0] = (BYTE)0xC3; pktBuf[1] = (BYTE)uSz;
                        FUN_0053cc30((int)(pktBuf+2), pbPay, iPayLen);
                        int off2=0; unsigned int rem2=uSz;
                        if (DAT_055ca168 != 0xffffffff) {
                            do {
                                int r2=send((SOCKET)DAT_055ca168,(char*)pktBuf+off2,(int)(rem2-off2),0);
                                if(r2==-1){int e2=WSAGetLastError();if(e2==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem2)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pktBuf,rem2);DAT_055cc16c+=rem2;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                                if(r2==0)break;if(DAT_055ce174)FUN_0043de60();rem2-=r2;off2+=r2;
                            } while((int)rem2>0);
                        }
                    } else {
                        uint uSz2 = (uint)(encLen + 3);
                        pktBuf[0] = (BYTE)0xC4; pktBuf[2] = (BYTE)uSz2;
                        pktBuf[1] = (BYTE)((uSz2 + ((int)uSz2 >> 0x1f & 0xff)) >> 8);
                        FUN_0053cc30((int)(pktBuf+3), pbPay, iPayLen);
                        int off2=0; unsigned int rem2=uSz2;
                        if (DAT_055ca168 != 0xffffffff) {
                            do {
                                int r2=send((SOCKET)DAT_055ca168,(char*)pktBuf+off2,(int)(rem2-off2),0);
                                if(r2==-1){int e2=WSAGetLastError();if(e2==WSAEWOULDBLOCK&&(int)(DAT_055cc16c+rem2)<0x2001){memcpy(DAT_055ca16c+DAT_055cc16c,pktBuf,rem2);DAT_055cc16c+=rem2;}else Net_Disconnect(((int)(uintptr_t)DAT_055ca160));break;}
                                if(r2==0)break;if(DAT_055ce174)FUN_0043de60();rem2-=r2;off2+=r2;
                            } while((int)rem2>0);
                        }
                    }
                }

                Item_RecalculateRepairCost();
            }
        }

        // Back/cancel button at [DAT_07ea5288+0x19, +0x31) x [DAT_07ea5284+0x18b, +0x1a3)
        if ((int)(DAT_07ea5288 + 0x19) <= (int)DAT_083a427c &&
            (int)DAT_083a427c < (int)(DAT_07ea5288 + 0x31) &&
            (int)(DAT_07ea5284 + 0x18b) <= (int)DAT_083a4278 &&
            (int)DAT_083a4278 < (int)(DAT_07ea5284 + 0x1a3) &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            DAT_07eaa117 = 0;
            FUN_004cba60();
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }
}

// =============================================================================
// 2026-05-07 B3 refactor — SecondPassword UI helper stubs
// moved from stubs.cpp lines 5297-7762 (2466 lines).
// =============================================================================
// ── SecondPassword UI helper stubs (bodies in original binary) ────────────────
// FUN_0051e240 @ 0x0051E240 — ShowCheckBox
// param_1=count, param_2=índice base de GlobalText (stride 300),
// param_3=mensaje de destino (0x99 activa el rótulo especial de ítem).
// Copia las líneas al buffer del panel DAT_083a44c4 (stride 0x26) y configura el descriptor.
undefined4 __cdecl FUN_0051e240(int param_1, int param_2, int param_3)
{
    int iVar3 = param_1;
    if (param_3 != 0x99) {
        if (0 < param_1) {
            char *dst = (char *)&DAT_083a44c4;
            for (int i = 0; i < param_1; ++i) {
                // IDA: ShowCheckBox 0x51E240 copia desde
                // GlobalText[index + i] (slots de 300 bytes). En Trade el
                // caller FUN_004EB7F0 usa (4, 371, 151): GlobalText[371..374].
                strncpy_s(dst, 0x26, GlobalText[param_2 + i], _TRUNCATE);
                dst += 0x26;
            }
        }
        goto LAB_0051e377;
    }
    // mode 0x99: build item class label for current item (DAT_07ea5240)
    {
        char local_34[0x34] = {};
        if ((short)DAT_07ea5240 == 0x1af) {
            byte *pbVar13 = nullptr;
            switch (DAT_07ea5244 >> 3 & 0xf) {
            case 0: pbVar13 = &DAT_005618b8; break;
            case 1: pbVar13 = &DAT_005618bc; break;
            case 2: pbVar13 = &DAT_005618c0; break;
            case 3: pbVar13 = &DAT_005618c4; break;
            default: goto switchD_default;
            }
            crt_sprintf(local_34, (const char*)pbVar13);
        }
switchD_default:
        crt_sprintf((char *)&DAT_083a44c4, s____s___005618c8);
        iVar3 = param_1 + 1;
        if (1 < iVar3) {
            char *puVar9 = &DAT_083a44ea;
            do {
                crt_sprintf(puVar9, &DAT_07d29d24 + param_2 * 300);
                puVar9 += 0x26;
                param_1--;
            } while (param_1 != 0);
        }
    }
LAB_0051e377:
    {
        static const unsigned int local_5c[10] = {1,0x15,0x5a,0x46,0x15, 3,0x78,0x5a,0x46,0x15};
        DAT_083a4324 = (DWORD)iVar3;
        memset(&DAT_083a42f8[0], 0, 10*4);
        for (int i = 0; i < 5; i++) DAT_083a42f8[i] = local_5c[i];
        unsigned int uVar2 = DAT_083a7c28;
        for (int i = 0; i < 5; i++) DAT_083a430c[i] = local_5c[i+5];
        if (param_3 != 0) {
            if (DAT_083a7c24 == 0) { DAT_083a7c24 = (undefined4)param_3; return 1; }
            DAT_083a7c28 = (undefined4)param_3;
            return 1;
        }
        DAT_083a7c28 = (undefined4)param_3;
        DAT_083a7c24 = (undefined4)uVar2;
        return 1;
    }
}

// FUN_004e3db0 @ 0x004E3DB0 — SecondPassword_GridSlotAvail
// Escanea una grilla 2D (param_3×param_2 filas/columnas) en el array de inventario en param_1,
// con stride 0x44 por celda; devuelve 1 si alguna celda de la ventana está libre (slot==-1), si no 0.
// FUN_004e3db0 (IDA-activated, was Ghidra stub)
uint __cdecl FUN_004e3db0(int a1, int a2, int a3, int a4, int a5)
{
  int v5; // ebp
  int v6; // eax
  int v8; // esi
  char v10; // bl
  int v11; // edi
  WORD *v12; // ecx
  bool v13; // cc
  int v15; // [esp+10h] [ebp-Ch]
  int v16; // [esp+14h] [ebp-8h]
  int v17; // [esp+18h] [ebp-4h]
  int v18; // [esp+24h] [ebp+8h]
  int v19; // [esp+28h] [ebp+Ch]

  v5 = a5;
  v6 = a3 - a5 + 1;
  v16 = 0;
  v17 = v6;
  if ( v6 <= 0 )
  {
    return 0;
  }
  v8 = a4;
  v18 = 0;
  v15 = a2 - a4 + 1;
  while ( 1 )
  {
    v19 = 0;
    if ( v15 > 0 )
    {
      break;
    }
LABEL_16:
    v13 = ++v16 < v6;
    v18 += a2;
    if ( !v13 )
    {
      return 0;
    }
  }
  while ( 1 )
  {
    v10 = 1;
    if ( v5 <= 0 )
    {
      return 1;
    }
    v11 = a1 + 68 * (v19 + v18);
    do
    {
      if ( v8 > 0 )
      {
        v12 = (WORD *)v11;
        do
        {
          if ( *v12 != 0xFFFF )
          {
            v10 = 0;
          }
          v12 += 34;
          --v8;
        }
        while ( v8 );
        v8 = a4;
      }
      v11 += 68 * a2;
      --v5;
    }
    while ( v5 );
    if ( v10 )
    {
      return 1;
    }
    v5 = a5;
    if ( ++v19 >= v15 )
    {
      v6 = v17;
      goto LABEL_16;
    }
  }
}


// FUN_004f6850 @ 0x004F6850 — SecondPassword_CancelReturn
// Chequea si hay slots de entidad de segunda contraseña ocupados; si los hay, muestra el diálogo.
// Otherwise sends a cancel packet (C1 03 87) over the socket.
undefined4 __cdecl FUN_004f6850(void)
{
    if (!ChaosMixOpened) return 0;
    return ChaosBoxRequestClose() ? 1 : 0;
#if 0
    bool bVar1 = true;
    short *psVar2 = (short *)&DAT_07ea9848;
    do {
        int iVar6 = 8;
        do {
            if ((*psVar2 != -1) && (0 < *(int *)((char*)psVar2 + 0x1c * 2))) bVar1 = false;
            psVar2 += 0x22;
            iVar6--;
        } while (iVar6 != 0);
    } while ((int)psVar2 < 0x7eaa0c8);
    if ((!bVar1) || (0 < (int)DAT_07e91388)) {
        FUN_00480620((const char*)&lpDefault_00583d88, (const char*)&DAT_07d55410, 2);
        return 0;
    }
    // Send C1 03 87 logout/cancel packet
    char pkt[3]; pkt[0] = (char)0xC1; pkt[1] = 3; pkt[2] = (char)0x87;
    unsigned int uVar8 = 3;
    int iVar6 = 0;
    if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
        do {
            int iVar3 = send(DAT_055ca168, pkt + iVar6, 3 - iVar6, 0);
            if (iVar3 == -1) {
                iVar6 = WSAGetLastError();
                if (iVar6 != 0x2733) { Net_Disconnect(((int)(uintptr_t)DAT_055ca160)); break; }
                if (0x2000 < (int)(DAT_055cc16c + 3)) { Net_Disconnect(((int)(uintptr_t)DAT_055ca160)); break; }
                memcpy(DAT_055ca16c + DAT_055cc16c, pkt, uVar8);
                DAT_055cc16c += uVar8;
                break;
            }
            if (iVar3 == 0) break;
            if (DAT_055ce174 != 0) FUN_0043de60();
            uVar8 -= iVar3; iVar6 += iVar3;
        } while (0 < (int)uVar8);
    }
    return CONCAT31((int3)(DAT_055ca168 >> 8), 1);
#endif
}

// FUN_004f6a70 @ 0x004F6A70 — Net_Disconnect_Clean
// Limpia el estado de la UI y después manda un paquete de desconexión (C1 03 82) por el socket.
uint __cdecl FUN_004f6a70(void)
{
    if (DAT_07eaa165 != '\0') return 0;
    DAT_07eaa117 = 0;
    FUN_004cba60();
    if (0 < (int)DAT_07e91388) FUN_004cd3b0();
    char pkt[3]; pkt[0] = (char)0xC1; pkt[1] = 3; pkt[2] = (char)0x82;
    unsigned int uVar4 = 3;
    int iVar6 = 0;
    SOCKET SVar2 = DAT_055ca168;
    if (DAT_055ca168 != (SOCKET)INVALID_SOCKET) {
        do {
            int iVar1 = send(DAT_055ca168, pkt + iVar6, 3 - iVar6, 0);
            if (iVar1 == -1) {
                iVar6 = WSAGetLastError();
                SVar2 = (SOCKET)DAT_055cc16c;
                if (iVar6 == 0x2733) {
                    if ((int)(DAT_055cc16c + 3) < 0x2001) {
                        memcpy(DAT_055ca16c + DAT_055cc16c, pkt, uVar4);
                        DAT_055cc16c += uVar4;
                    } else { Net_Disconnect(((int)(uintptr_t)DAT_055ca160)); SVar2 = 0; }
                } else { Net_Disconnect(((int)(uintptr_t)DAT_055ca160)); SVar2 = 0; }
                break;
            }
            SVar2 = 0;
            if (iVar1 == 0) break;
            SVar2 = 0;
            if (DAT_055ce174 != 0) { FUN_0043de60(); SVar2 = 0; }
            uVar4 -= iVar1; iVar6 += iVar1;
        } while (0 < (int)uVar4);
    }
    return CONCAT31((int3)(SVar2 >> 8), 1);
}

// FUN_004d1fc0 @ 0x004D1FC0 — SecondPassword_WidgetGrid_Init
// Inicializa la grilla del widget de segunda contraseña insertando / actualizando entradas en la
// hash-table global (DAT_055c9bc8) con clave DAT_07cf1ffc, y después llama a FUN_004cdc70 para
// place 12 grid-slot widgets at fixed screen positions:
//   slot 0-1 : (700,184)  (380,184) size 40×40 / 60×40
//   slot 2-4 : (300,360)  (300,400) (180,360)
//   slot 5-6 : (390,400)  (198,400) size 40×40
//   slot 7   : checkbox (190,360)   wait (190,400)
//   slot 8-10: (350,360)  (350,400) (380,400) size 20×20
//   slot 11  : (180,400)
// Usa HashTable_GetIndex / FUN_00403f80 / FUN_00404370 con el ref-count ofuscado por XOR
// sobre el blob de widget de 0x584 bytes. La clave XOR sale de DAT_00559050 (tabla de 16 bytes).
// STUB: la lógica real son 12 llamadas a FUN_004cdc70 (función de render de inventario de 3554 líneas, sin declarar)
// setting up second-password widget grid at fixed screen coordinates.
// No se puede implementar hasta que FUN_004cdc70 esté declarada en functions.h.
// Widget positions (hex float → decimal):
//   slot 8: (15.0, 46.0) 40×40    slot 7: (115.0, 46.0) 60×40
//   slot 2: (75.0, 46.0) 40×40    slot 3: (75.0, 89.0) 40×60
//   slot 4: (75.0, 152.0) 40×40   slot 0: (15.0, 89.0) 40×60
//   slot 1: (134.0, 89.0) 40×60   slot 5: (15.0, 152.0) 40×40
//   slot 6: (134.0, 152.0) 40×40  slot 9: (55.0, 89.0) 20×20
//   slot10: (55.0, 152.0) 20×20   slot11: (115.0, 152.0) 20×20
// Anti-tamper hash table blocks (DAT_055c9bc8) interspersed — skipped.
// FUN_004d1fc0 @ 0x004D1FC0 — Render Character Equipment Slots (12 slots).
// Port FIEL del IDA: 12 llamadas a sub_4CDC70(x, y, w, h, slotIdx) renderizando
// los slots del Character panel. STRUCT_DECRYPT/ENCRYPT (HashTable obfuscation)
// skipped per project policy.
//
// Slot layout (per IDA decompile):
//   slot 8  @ (15, 46, 40x40)   Helmet
//   slot 7  @ (115, 46, 60x40)  Wings/Cape
//   slot 2  @ (75, 46, 40x40)   Pendant (skip if class & 7 == 3 = SM)
//   slot 3  @ (75, 89, 40x60)   Body Armor
//   slot 4  @ (75, 152, 40x40)  Boots
//   slot 0  @ (15, 89, 40x60)   Weapon Left
//   slot 1  @ (134, 89, 40x60)  Weapon Right / Shield
//   slot 5  @ (15, 152, 40x40)  Pants
//   slot 6  @ (134, 152, 40x40) Gloves
//   slot 9  @ (55, 89, 20x20)   Ring
//   slot 10 @ (55, 152, 20x20)  Ring 2
//   slot 11 @ (115, 152, 20x20) Necklace
// FUN_004cdc70 @ 0x004CDC70 — RenderEquipmentSlot(sx, sy, w, h, slotIdx)
// Port simplificado: el IDA decompile son 3396 líneas, ~65% es HashTable
// obfuscation (anti-tamper). El render real:
//   1. Leer item desde CharacterMachine + slotOffset (stride 68B = sizeof(ITEM)).
//   2. Si Type != -1, llamar RenderItem3D para dibujar el modelo.
//   3. Anti-tamper STRUCT_DECRYPT/ENCRYPT — skipped per project policy.
//
// Slot → offset mapping (from RenderEquipment3D en IDA):
//   slot 0  = WeaponL (536),  slot 1 = WeaponR (604),  slot 2  = Pendant (672)
//   slot 3  = Armor   (740),  slot 4 = Boots   (808),  slot 5  = Pants   (876)
//   slot 6  = Gloves  (944),  slot 7 = Wings  (1012),  slot 8  = Helmet (1080)
//   slot 9  = Ring1  (1148), slot 10 = Ring2  (1216),  slot 11 = Necklace(1284)
extern "C" void __cdecl FUN_004cdc70(float sx, float sy, float w, float h, int slotIdx)
{
    if (!CharacterMachine || slotIdx < 0 || slotIdx >= 12) return;

    BYTE* CM = (BYTE*)CharacterMachine;
    int slotOffset = 536 + slotIdx * 68;
    short itemType = *(short*)(CM + slotOffset);
    if (itemType == -1) return;   // empty slot

    int   itemLevel  = *(int*)(CM + slotOffset + 4);
    BYTE  itemOption = *(BYTE*)(CM + slotOffset + 27);

    // Read InventoryStartX/Y para el offset absoluto del panel.
    int startX = (int)DAT_07ea5288;   // InventoryStartX
    int startY = (int)DAT_07ea5284;   // InventoryStartY

    // RenderItem3D quiere coords screen-space; sx/sy son relativas al panel.
    int itemExt = *(BYTE*)(CM + slotOffset + 61);
    RenderItem3D((float)startX + sx, (float)startY + sy, w, h,
                 itemType, itemLevel, (int)itemOption, itemExt, false);
}

void __cdecl FUN_004d1fc0(void) {
    if (!CharacterAttribute) return;
    FUN_004cdc70(15.0f,  46.0f,  40.0f, 40.0f, 8);
    FUN_004cdc70(115.0f, 46.0f,  60.0f, 40.0f, 7);

    BYTE classByte = *(BYTE*)((BYTE*)CharacterAttribute + 11);
    if ((classByte & 7) != 3) {  // not SM (Soul Master)
        FUN_004cdc70(75.0f,  46.0f,  40.0f, 40.0f, 2);
    }
    FUN_004cdc70(75.0f,  89.0f,  40.0f, 60.0f, 3);
    FUN_004cdc70(75.0f,  152.0f, 40.0f, 40.0f, 4);
    FUN_004cdc70(15.0f,  89.0f,  40.0f, 60.0f, 0);
    FUN_004cdc70(134.0f, 89.0f,  40.0f, 60.0f, 1);
    FUN_004cdc70(15.0f,  152.0f, 40.0f, 40.0f, 5);
    FUN_004cdc70(134.0f, 152.0f, 40.0f, 40.0f, 6);
    FUN_004cdc70(55.0f,  89.0f,  20.0f, 20.0f, 9);
    FUN_004cdc70(55.0f,  152.0f, 20.0f, 20.0f, 10);
    FUN_004cdc70(115.0f, 152.0f, 20.0f, 20.0f, 11);
}

// FUN_004d23b0 @ 0x004D23B0 — Inventory grid render + click dispatcher.
// 2026-05-08: port FIEL completo movido a `Item/Item_ClickHandler.cpp`
// (~600 líneas). Antes era stub vacío bloqueando toda la cadena de
// interacción con items (pickup, drop, sell, use, hotkey).
//
// La signatura real (per IDA `004D23B0_sub_4D23B0.c`) es:
//   void __cdecl FUN_004d23b0(char* origin_x, int origin_y, short* inv_base,
//                             int grid_w, int grid_h, char mode_flag);
//
// Caller sites en este archivo (SecondPassword.cpp:712-782) ya pasan los
// argumentos correctos como esa signatura — el stub anterior tenía una
// signatura errónea pero la convención de llamada __cdecl + tamaños
// compatibles hicieron que linkeara sin warning.

// Player input helpers
// FUN_004430c0 @ 0x004430C0 — SetPlayerStop(entity_ptr)
// Port directo del IDA sub_4430C0 (2155 bytes). Selecciona la animación
// idle/stop del personaje según el equipamiento (alas, armas, helm, armadura)
// y el estado del mundo (g_GameState, World, terrain wall flag).
//
// Mapping de offsets:
//   c+0x002 short  EntityType (0x186 = Player)
//   c+0x010 float  WorldX
//   c+0x014 float  WorldY
//   c+0x105 byte   CurrentAction
//   c+0x106 byte   PriorAction
//   c+0x108 float  AnimationFrame
//   c+0x10C float  PriorAnimationFrame
//   c+0x1BC byte   Class      (low 3 bits = base class, &7==2 = Elf)
//   c+0x270 short  Helm.Type  (= c+624)
//   c+0x288 short  Armor.Type (= c+648)
//   c+0x2A0 short  Wing.Type  (= c+672)  ← controla Fly
//   c+0x2B8 short  Helper.Type(= c+696)  ← 818/819 trigger pet-fly
//   c+0x300 byte   stamina/state flag (cleared on entry)
//   c+0x34e byte   SafeZone (no dead — el dead real es +0x2FD)
//   c+0x2BE word   (= c+846) class-change indicator
//
// Hash-table refcounting (líneas 181-433 IDA) es anti-tamper, omitido.
extern int DAT_07d78068;   // ItemAttribute base (declared in globals.h)

void __cdecl FUN_004430c0(int c) {
    *(unsigned char*)(c + 0x300) = 0;   // c+768 = stamina counter

    auto SetAction_local = [c](unsigned char act) {
        if (*(unsigned char*)(c + 0x105) != act) {
            *(unsigned char*)(c + 0x106) = *(unsigned char*)(c + 0x105);
            *(float*)(c + 0x10c) = *(float*)(c + 0x108);
            *(unsigned char*)(c + 0x105) = act;
            *(float*)(c + 0x108) = 0.0f;
        }
    };

    // Helper para leer el byte TwoHand de ItemAttribute[type-399].
    // IDA: `*((_BYTE *)&ItemAttribute[v5 - 399] - 34)` → struct stride 0x40 con
    // un offset peculiar (-34 desde el inicio del elemento). Mu structures.h
    // muestra TwoHand en offset +30 dentro de ITEM_ATTRIBUTE. La expresión
    // `&arr[i] - 34` con sizeof=64 == (i-1)*64 + 30, o sea TwoHand del item
    // anterior. Mantenemos el cálculo idéntico al IDA para preservar semántica.
    // 2026-08-08 CRASH-FIX (0xC0000005 al cerrar el inventario con V):
    // stack = Game_CharSelectTick → FUN_00454cd0 → FUN_004430c0 → este lambda,
    // leyendo `[base + 0x1A00 - 0x21]` con base ≈ 0 (log: addr=0x005D4E95,
    // param1=0x000019DF, eax=0x1A00, edi=0).
    // Dos agujeros: (a) el guard sólo miraba `== 0`, pero DAT_07d78068 se
    // corrompe a valores CHICOS (ver el watchdog de ItemAttribute en
    // Render_Frame.cpp / Item_GetAttribute), y (b) `type` llega como 0xFFFF
    // cuando la mano está vacía — justo lo que pasa al desequiparse todo — y
    // `(0xFFFF-399)*64` indexa ~4 MB después de la tabla.
    // Validamos base y rango igual que el resto de los accesos a ItemAttribute.
    auto ItemTwoHand = [](int type) -> unsigned char {
        unsigned int base = (unsigned int)(uintptr_t)DAT_07d78068;
        if (base < 0x100000u || base >= 0x80000000u) return 0;
        int idx = type - 399;
        if (idx < 1 || idx >= 1024) return 0;   // idx>=1: la expresión resta 34
        return *(unsigned char*)((char*)(uintptr_t)base + idx * 64 - 34);
    };

    if (*(short*)(c + 2) == 390) {
        // ── Player path ─────────────────────────────────────────────────────
        short v1 = *(short*)(c + 696);  // Helper.Type
        if ((v1 == 818 || v1 == 819) && !*(unsigned char*)(c + 846)) {
            unsigned char act = (*(unsigned short*)(c + 624) == 0xFFFF
                              && *(unsigned short*)(c + 648) == 0xFFFF) ? 11 : 12;
            SetAction_local(act);
            goto LABEL_129;
        }

        // Fly = 1 si NO está cambiando clase y Wing.Type != 0xFFFF
        char Fly = 0;
        if (!*(unsigned char*)(c + 846) && *(unsigned short*)(c + 672) != 0xFFFF) {
            Fly = 1;
        }

        // Terrain wall check: en agua/Atlans (World==7) sin pared y g_GameState==5
        // también dispara la animación de flotar. En char-select (g_GameState==4)
        // queda solo Fly.
        bool gateA = false;
        if (DAT_005615c0 == 5 && DAT_0055a7ac == 7) {
            int gx = (int)(*(float*)(c + 16) * 0.0099999998f);
            int gy = (int)(*(float*)(c + 20) * 0.0099999998f);
            int v3 = FUN_004f6c40((unsigned int)gx, (unsigned int)gy);
            if ((DAT_0838bc70[v3] & 1) != 1) gateA = true;
        }
        if (gateA || Fly) {
            short v4 = *(short*)(c + 624);  // Helm
            // bow/crossbow helms: 536-542, 544, 546 → action 10 (fly w/ bow)
            bool bowHelm = (v4 >= 536 && v4 < 543) || (v4 >= 544 && v4 < 545) || (v4 == 546);
            SetAction_local(bowHelm ? 10 : 9);
            goto LABEL_129;
        }

        short v5 = *(short*)(c + 624);  // Helm
        // Sin helm/armor o cambiando clase fuera de mapas 11-16 → idle estándar
        if ((v5 == -1 && *(unsigned short*)(c + 648) == 0xFFFF)
            || (*(unsigned char*)(c + 846)
                && (DAT_0055a7ac < 11 || DAT_0055a7ac > 16))) {
            bool isElf = ((*(unsigned char*)(c + 444) & 7) == 2);
            SetAction_local(isElf ? 2 : 1);
            goto LABEL_129;
        }

        // Helm 400-495: arma de melee → action 3 (one-hand) / 4 (two-hand) / 78 (special 431)
        if (v5 >= 400 && v5 < 496) {
            if (!ItemTwoHand(v5)) {
                SetAction_local(3);
                goto LABEL_129;
            }
            unsigned char act = (v5 == 431) ? 78 : 4;
            SetAction_local(act);
            goto LABEL_129;
        }

        // Helm 497-498: special weapons → action 5
        if (v5 == 497 || v5 == 498) {
            SetAction_local(5);
            goto LABEL_129;
        }

        // Helm 496-527 ó 560-591: depende de TwoHand → action 3 / 6
        if ((v5 >= 496 && v5 < 528) || (v5 >= 560 && v5 < 592)) {
            unsigned char act = ItemTwoHand(v5) ? 6 : 3;
            SetAction_local(act);
            goto LABEL_129;
        }

        // Resto: chequear armor (bow/crossbow staff/etc.) y clase Elf
        short v8 = *(short*)(c + 648);
        if ((v8 >= 528 && v8 < 535) || v8 == 545) {  // Bows
            SetAction_local(7);
            goto LABEL_129;
        }
        if ((v5 >= 536 && v5 < 543) || (v5 >= 544 && v5 < 545) || v5 == 546) {  // Crossbows
            SetAction_local(8);
            goto LABEL_129;
        }
        {
            bool isElf = ((*(unsigned char*)(c + 444) & 7) == 2);
            SetAction_local(isElf ? 2 : 1);
        }
    } else {
        // ── Non-player path (NPCs, monsters) ───────────────────────────────
        // IDA líneas 181-433: 4 hash-table operations sobre c+908 y c+904
        // (refcount + obfuscación XOR). Anti-tamper, no afecta el render.
        // Sólo el resultado final importa: terrain check + SetAction.
        int gx = (int)(*(float*)(c + 16) * 0.0099999998f);
        int gy = (int)(*(float*)(c + 20) * 0.0099999998f);
        int v50 = FUN_004f6c40((unsigned int)gx, (unsigned int)gy);
        if (*(short*)(c + 2) == 302 && (DAT_0838bc70[v50] & 1) == 1) {
            SetAction_local(7);
        } else {
            SetAction_local(0);
        }
    }

LABEL_129:
    // 1/16 chance de reproducir un sonido random (idle voice)
    int rnd = rand() & 0x8000000F;
    bool zero = (rnd == 0);
    if (rnd < 0) {
        zero = (((unsigned char)rnd - 1) | 0xFFFFFFF0) == 0xFFFFFFFFu;
    }
    if (zero) {
        short v53 = *(short*)(c + 2);
        if (v53 != 390 || (*(int*)(c + 4) >= 206 && *(int*)(c + 4) <= 208)) {
            // Models[v53].SoundIdle (offset 170)
            short modSnd = *(short*)((char*)(uintptr_t)DAT_05828d58 + 188 * v53 + 170);
            if (modSnd != (short)0xFFFF) {
                int v55 = rand() % 2;
                short s = *(short*)((char*)(uintptr_t)DAT_05828d58
                                    + 2 * (v55 + 94 * v53) + 170);
                // PlayBuffer(s + 170, c, 0) — sound playback (FUN_00404bc0 en nuestra port)
                FUN_00404bc0(s + 170, (DWORD)c, 0);
            }
        }
    }
}

// FUN_00443930 @ 0x00443930 — SetPlayerWalk (1337 bytes IDA)
// Full port: selects walk/run animation based on character class, equipped
// weapon, wings, stamina, and world.  IDA action IDs:
//   13 (0x0d) = walk no-weapon
//   14 (0x0e) = run no-weapon
//   15-28     = walk/run with various weapon combos (sword/bow/staff/spear/wand)
//   30-33     = wings active
//   76-77     = wings + Atlans world (8) or Aida (10) (swim)
//   79-80     = walk/run with item 431 (special weapon)
//   29        = exhausted walk in Atlans (world 7)
//   21        = walk in Atlans (world 7)
//   22        = run no-weapon (exhausted)
//   32-33     = wings stand/walk
//   2         = non-player default (NPC/monster)
//   0x20/0x21 = DarkLord stand/walk (legacy)
//
// 2026-05-04: REEMPLAZA stub que devolvía hardcoded `uVar8 = 2` para todo
// non-DarkLord. Por eso el player caminaba sin animación (action=2 es la
// pose idle del modelo). Port faithful from IDA L62-227.
void __cdecl FUN_00443930(int param_1) {
    // +0x34E (=846) es **SafeZone**, NO dead (el dead real es +0x2FD).
    // Ver CLAUDE.md 2026-08-10; el nombre viejo `dead` mentía.
    char bSafeZone0 = *(char *)(param_1 + 0x34e);

    // Tick de stamina (IDA L27-56). El campo en +768 (= +0x300) es el contador de stamina.
    if (bSafeZone0) {
        *(unsigned char *)(param_1 + 0x300) = 0;
    } else {
        unsigned char v2 = *(unsigned char *)(param_1 + 0x300);
        if (v2 < 0x28) {
            // 1) move_type bit = 3 (running): always increment.
            // 2) World 7 (Atlans): si c+0x240 (Wings1) != -1 Y c+0x242 (Wings1.lvl) > 4, incrementa.
            // 3) Si no, si c+0x258 (Helper) != -1 Y c+0x25a > 4, incrementa.
            if ((*(unsigned char *)(param_1 + 0x1bc) & 7) == 3) {
                *(unsigned char *)(param_1 + 0x300) = v2 + 1;
            } else if (DAT_0055a7ac == 7) {
                if (*(short *)(param_1 + 0x240) != -1 &&
                    *(unsigned char *)(param_1 + 0x242) > 4)
                    *(unsigned char *)(param_1 + 0x300) = v2 + 1;
            } else if (*(short *)(param_1 + 600) != -1 &&
                       *(unsigned char *)(param_1 + 0x25a) > 4) {
                *(unsigned char *)(param_1 + 0x300) = v2 + 1;
            }
        }
    }

    // Action selection (IDA L57-227)
    short etype = *(short *)(param_1 + 2);

    // Non-player (NPC/monster): always action 2.
    if (etype != 390) {
        FUN_0043e820(param_1, 2);
        goto label_119;
    }

    // Player path. Read pendant (c+696), L-hand (c+624), R-hand (c+648),
    // wings (c+672), stamina (c+768), is-dead (c+846).
    short pendant = *(short *)(param_1 + 696);
    short LH      = *(short *)(param_1 + 624);
    short RH      = *(short *)(param_1 + 648);
    short wings   = *(short *)(param_1 + 672);
    unsigned char stamina = *(unsigned char *)(param_1 + 0x300);
    char bSafeZone = *(char *)(param_1 + 0x34e);

    // Pendant 818 + alive: stand-with-fairy (32) or holding-something (33).
    if (pendant == 818 && !bSafeZone) {
        if ((unsigned short)LH == 0xFFFF && (unsigned short)RH == 0xFFFF) {
            FUN_0043e820(param_1, 32);
            goto label_119;
        }
        FUN_0043e820(param_1, 33);
        goto label_119;
    }

    // Pendant 819 alive (master/wing pendant): special anim by world.
    if (pendant != 819 || bSafeZone) {
        // Las alas están activas cuando c+672 != -1 (sólo si está vivo).
        if (!bSafeZone && wings != -1) {
            // Wings flying: action 30 (no spear) or 31 (spear 536-543, 544, 546).
            if (LH >= 536 && LH < 543) { FUN_0043e820(param_1, 31); goto label_119; }
            if (LH == 545 || LH == 546)    { FUN_0043e820(param_1, 31); goto label_119; }
            FUN_0043e820(param_1, 30);
            goto label_119;
        }
        // World 7 (Atlans): swim animations. Stamina < 0x28 → 21, else 29.
        // IDA L98: `if ( v7 && World == 7 )` — el gate `v7` faltaba. Analizando
        // los caminos que llegan acá, v7 == !SafeZone siempre:
        //   v6 = c+846 (SafeZone); v7 = (v6 == 0);
        //   if (!v6) { si hay alas → SetAction 30/31 y sale; si no, v7 = 1; }
        // O sea en zona segura NO se nada (ni se vuela): se camina.
        if (!bSafeZone && DAT_0055a7ac == 7) {
            FUN_0043e820(param_1, (stamina < 0x28) ? 21 : 29);
            goto label_119;
        }
        // No weapons equipped (or dead in non-event-map world):
        bool noWeapons = ((unsigned short)LH == 0xFFFF) && ((unsigned short)RH == 0xFFFF);
        bool bSafeZoneNonEvent = bSafeZone && (DAT_0055a7ac < 11 || DAT_0055a7ac > 16);
        if (noWeapons || bSafeZoneNonEvent) {
            if (stamina >= 0x28) {
                // Exhausted: action 22.
                FUN_0043e820(param_1, 22);
                goto label_119;
            }
            // Non-exhausted no-weapon walk/run: 13 (walk) or 14 (run).
            unsigned int act = ((*(unsigned char *)(param_1 + 0x1bc) & 7) == 2) ? 14u : 13u;
            FUN_0043e820(param_1, act);
            goto label_119;
        }
        // Tiene armas, está vivo, no es Atlans, no está exhausto:
        // 2026-05-07: TwoHand approximation reemplazado por lookup REAL en
        // ItemAttribute table (DAT_07d78068, stride 64). IDA hace
        // *((BYTE*)&ItemAttribute[N-399] - 34) — stride 64, byte-offset 30
        // dentro del item anterior. Mantener idéntico para preservar semántica.
        auto ItemTwoHand = [](short type) -> unsigned char {
            if (DAT_07d78068 == 0) return 0;
            return *(unsigned char*)((char*)(uintptr_t)DAT_07d78068 + (type - 399) * 64 - 34);
        };
        if (stamina < 0x28) {
            // Sword class (LH 400..495):
            if (LH >= 400 && LH < 496) {
                if (ItemTwoHand(LH)) {
                    FUN_0043e820(param_1, (LH == 431) ? 79 : 16);
                    goto label_119;
                }
                FUN_0043e820(param_1, 15);   // 1H sword walk
                goto label_119;
            }
            // Lanza (560..591): IDA chequea TwoHand → 18, si no cae a LABEL_64 (15)
            if (LH >= 560 && LH < 592) {
                if (!ItemTwoHand(LH)) {
                    FUN_0043e820(param_1, 15);  // LABEL_64
                    goto label_119;
                }
                FUN_0043e820(param_1, 18);
                goto label_119;
            }
            // Mace (497) / War-axe (530):
            if (LH == 497 || LH == 530) { FUN_0043e820(param_1, 17); goto label_119; }
            // IDA L155-176: NOT magic-book range (LH < 496 || LH >= 528):
            //   Bow RH → 19, Staff LH → 20, fallthrough → LABEL_87 (13/14).
            // SI NO (mano izquierda en [496, 528)): SetAction 18 (terminal).
            if (LH < 496 || LH >= 528) {
                if ((RH >= 528 && RH < 535) || RH == 545) {
                    FUN_0043e820(param_1, 19);
                    goto label_119;
                }
                if ((LH >= 536 && LH < 543) || LH == 545 || LH == 546) {
                    FUN_0043e820(param_1, 20);
                    goto label_119;
                }
                // LABEL_87: no-weapon walk class-conditional
                unsigned int act = ((*(unsigned char *)(param_1 + 0x1bc) & 7) == 2) ? 14u : 13u;
                FUN_0043e820(param_1, act);
                goto label_119;
            }
            // LH in [496, 528): magic books — terminal action 18
            FUN_0043e820(param_1, 18);
            goto label_119;
        }
        // Stamina >= 0x28 (exhausted) with weapons:
        if (LH >= 400 && LH < 496) {
            if (RH >= 400 && RH < 496) {
                FUN_0043e820(param_1, 24);     // dual-sword exhausted
                goto label_119;
            }
            if (ItemTwoHand(LH)) {
                FUN_0043e820(param_1, (LH == 431) ? 80 : 25);
                goto label_119;
            }
            FUN_0043e820(param_1, 23);
            goto label_119;
        }
        // Spear exhausted: TwoHand → 26, else 23 (LABEL_97)
        if (LH >= 560 && LH < 592) {
            if (!ItemTwoHand(LH)) {
                FUN_0043e820(param_1, 23);  // LABEL_97
                goto label_119;
            }
            FUN_0043e820(param_1, 26);
            goto label_119;
        }
        if (LH >= 496 && LH < 528) {
            FUN_0043e820(param_1, 26);
            goto label_119;
        }
        if ((RH >= 528 && RH < 535) || RH == 545) {
            FUN_0043e820(param_1, 27);
            goto label_119;
        }
        if ((LH >= 536 && LH < 543) || LH == 545 || LH == 546) {
            FUN_0043e820(param_1, 28);
            goto label_119;
        }
        FUN_0043e820(param_1, 22);  // exhausted no-weapon walk (LABEL_59)
        goto label_119;
    }

    // Pendant 819 alive: world 8 (Tarkan) / world 10 (Aida) wings:
    if (DAT_0055a7ac != 8 && DAT_0055a7ac != 10) {
        if ((unsigned short)LH == 0xFFFF && (unsigned short)RH == 0xFFFF) {
            FUN_0043e820(param_1, 32);
            goto label_119;
        }
        FUN_0043e820(param_1, 33);
        goto label_119;
    }
    // Wings + Atlans/Aida: swim wings.
    FUN_0043e820(param_1,
                 ((unsigned short)LH == 0xFFFF && (unsigned short)RH == 0xFFFF) ? 76 : 77);

label_119:

    // Entity type 0x129 — special case with a fixed sound
    if (etype == 0x129) {
        FUN_00404bc0(0x5e, param_1, 0);
        return;
    }
    // Sonido de movimiento aleatorizado (1/64 para el jugador, 1/16 para el resto)
    if (param_1 == (int)DAT_07abf5d8) {
        if ((rand() & 0x3f) != 0) return;
    } else {
        if ((rand() & 0xf) != 0) return;
    }
    // Reproduce el sonido de paso de la tabla de sonidos del modelo si no es DarkLord (o si está en el rango de altura de paso)
    if (etype != 0x186 || (*(int *)(param_1 + 4) > 0xcd && *(int *)(param_1 + 4) < 0xd1)) {
        if (*(int *)(param_1 + 4) > 0xcd && *(int *)(param_1 + 4) < 0xd1)
            FUN_00404bc0(0x5d, param_1, 0);
        if (*(short *)(DAT_05828d58 + 0xaa + etype * 0xbc) != -1) {
            // BUG-FIX 2026-08-18: el indice de la tabla de sonidos era el action ID
            // actual del entity (+0x105 anim_state). IDA 0x443930 L288-289 es:
            //     v19 = rand() % 2;
            //     PlayBuffer(*(short*)(Models + 2*(v19 + 94*type) + 170) + 170, c, 0);
            // o sea la tabla tiene SOLO 2 entradas (los dos sonidos de paso del
            // modelo) y se elige una al azar. Con el action ID (13..33 al caminar)
            // el indice se iba muy lejos de esas 2 entradas y leia campos ajenos
            // del struct de modelo -> ids basura: 136 -> mBaliAttack2 (sonido de
            // ATAQUE sonando al caminar) y 104 -> slot 274, que ni existe.
            int v19 = rand() % 2;
            int iVar9 = 0;
            FUN_00404bc0(*(short *)(DAT_05828d58 + 0xaa + (v19 + etype * 0x5e) * 2) + 0xaa,
                         param_1, iVar9);
        }
    }
}

// FUN_00454ba0 @ 0x00454BA0 — Entity_StopMove(entity_ptr)
// Applies backward velocity step (FUN_00454b00) rotated by entity facing matrix,
// y después fija la altura de la entidad a la del terreno (FUN_004f7500).
void __cdecl FUN_00454ba0(int param_1) {
    float local_30[12];
    float vel[3] = { 0.0f, -(float)FUN_00454b00(param_1), 0.0f };
    // PORT FIX: el mismo artefacto de float[3] partido por Ghidra que en Terrain_Light FUN_004fa930.
    // local_3c/local_38/local_34 eran el buffer de salida contiguo de 3 floats que
    // esperaba Vector_Rotate, pero MSVC no garantiza el layout de los locales.
    float out[3] = {0.0f, 0.0f, 0.0f};
    Matrix_BuildFromEuler((float*)(param_1 + 0x1c), local_30);
    Vector_Rotate(vel, local_30, out);
    *(float*)(param_1 + 0x10) = out[0] + *(float*)(param_1 + 0x10);
    *(float*)(param_1 + 0x14) = out[1] + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x18) = out[2] + *(float*)(param_1 + 0x18);
    float terrainH = FUN_004f7500(*(float*)(param_1 + 0x10), *(float*)(param_1 + 0x14));
    if (*(short*)(param_1 + 0x2b8) == 0x333) {
        if (DAT_0055a7ac == 8 || DAT_0055a7ac == 10)
            terrainH += _DAT_00552848;
        else
            terrainH += _DAT_0055284c;
    }
    *(float*)(param_1 + 0x18) = terrainH;
    // Wyvern swim bob (+0x80 sine offset) omitted (type 0x110 case)
    *(float*)(param_1 + 0x80) = *(float*)(param_1 + 0x80) + _DAT_00552934;
}

// FUN_0045c130 @ 0x0045C130 — SetCharacterClass(entity)
//
// IDA-ported 2026-04-26 (audit #5). Antes era un stub parcial mal-llamado
// "Entity_CancelTarget" que sólo copiaba el cluster primario con offsets
// incorrectos. Reescrito completo per `0045C130_SetCharacterClass.c`:
//
//   1. Sólo opera sobre entity_type == 390 (= 0x186, local player).
//   2. Copia cluster primario (4 slots: 624/648/672/696) con +400 offset.
//   3. Si entity[+847] != 0 (caso "ya seteado"): early return.
//   4. Si entity[+847] == 0: copia cluster secundario (504/528/552/576/600)
//      con fallback formula `(skin&7) + 4*(skin>>3) + base` cuando el slot
//      del CharData es -1.
//   5. SetCharacterScale + sub_47E3C0 al final del else branch.
//
// Hashtable obfuscation y comparación con `Hero` global: omitidos (anti-tamper
// noise / no requeridos para gameplay observable).
void __cdecl FUN_0045c130(int c) {
    if (*(short*)(c + 2) != 390) return;   // 390 = 0x186 = local player

    char* cd = (char*)DAT_07cf1ffc;
    int v7 = (int)cd + 536;   // matches IDA: CharacterMachine + 536

    // ── Primary cluster (4 equipment slots → entity offsets 624/648/672/696) ──
    auto writeSlot = [&](int srcOff, int eOff) {
        short v = *(short*)(v7 + srcOff);
        *(short*)(c + eOff) = (v == -1) ? (short)-1 : (short)(v + 400);
    };
    // 2026-08-24: valor previo del helper, para detectar el CAMBIO abajo.
    const short prevHelper = *(short*)(c + 696);
    writeSlot(0,   624);
    writeSlot(68,  648);
    writeSlot(476, 672);
    writeSlot(544, 696);

    // ── Spawn/borrado del pet del HEROE (Guardian Angel y monturas) ─────────
    // El pet NO se dibuja desde RenderCharacter: `ChangeCharacterExt`
    // (0x45C8C0 L73-88) lo crea como entidad del pool de bugs —
    //     *(WORD*)(c + 696) = Type + 816;
    //     CreateBug(816 | 195 | 267, c + 16, c, 0, 0);
    // — y de ahi lo tickea MoveBugs y lo dibuja RenderBugs. Por eso
    // RenderCharacter solo tiene rama para el Imp (817) y ninguna para el 816.
    //
    // Pero `ChangeCharacterExt` se llama SOLO con el CharSet que llega por red:
    // char-select (ReceiveCharacterList L68), otros jugadores del viewport
    // (Combat_PacketDispatch L317) y el F3/13 (ProtocolCore L1629). El equipo
    // del HEROE in-world no viene por ahi sino de CharacterMachine, o sea pasa
    // por esta funcion — que setea el tipo pero nunca creaba el bug. Medido con
    // sonda: todos los `CreateBug type=816` salian con `isHero=0`, y el pet
    // propio no se dibujaba nunca (el de otros jugadores si).
    //
    // DESVIACION documentada: IDA no crea el bug en SetCharacterClass. No
    // encontre el camino por el que el original se lo da al heroe; puede estar
    // en el arrastre del pool desde char-select. Replicamos aca el par
    // DeleteBug+CreateBug de ChangeCharacterExt, gateado al CAMBIO de tipo para
    // no re-spawnear en cada llamada (esta funcion corre al cambiar el equipo,
    // no por frame).
    {
        const short newHelper = *(short*)(c + 696);
        if (newHelper != prevHelper) {
            FUN_004fffa0((DWORD)(uintptr_t)c);          // DeleteBug
            int bugType = 0;
            if      (newHelper == 816) bugType = 816;   // Guardian Angel
            else if (newHelper == 818) bugType = 195;   // Uniria
            else if (newHelper == 819) bugType = 267;   // Dinorant
            // 817 (Imp) NO lleva bug: lo dibuja RenderLinkObject desde
            // Render_PlayerHelper, fiel a RenderCharacter L1267-1287.
            if (bugType)
                FUN_004fffd0(bugType, (void*)(c + 16), (void*)(uintptr_t)c, 0);
        }
    }

    *(unsigned char*)(c + 626) = (unsigned char)((*(int*)(v7 + 4)   >> 3) & 0xF);
    *(unsigned char*)(c + 650) = (unsigned char)((*(int*)(v7 + 72)  >> 3) & 0xF);
    *(unsigned char*)(c + 627) = *(unsigned char*)(v7 + 27);
    *(unsigned char*)(c + 651) = *(unsigned char*)(v7 + 95);
    unsigned char v11 = *(unsigned char*)(c + 261);   // anim_state @ 0x105
    *(unsigned char*)(c + 674) = (unsigned char)((*(int*)(v7 + 480) >> 3) & 0xF);
    *(unsigned char*)(c + 698) = (unsigned char)((*(int*)(v7 + 548) >> 3) & 0xF);

    bool skipCancel = (v11 >= 0x85u && v11 <= 0x8Cu);
    if (!skipCancel && (v11 < 0x22u || v11 > 0x5Bu)) {
        FUN_004430c0(c);   // SetPlayerStop
    }

    // entity[+847] == 0 → secondary cluster needs filling
    if (*(unsigned char*)(c + 847)) return;

    // ── Secondary cluster (5 slots → entity offsets 504/528/552/576/600) ──
    // Cuando el CharData slot vale -1, fallback a `(skin&7)+4*(skin>>3)+base`.
    unsigned char skin = *(unsigned char*)(c + 444);
    unsigned char skinLo = skin & 7;
    unsigned char skinHi = skin >> 3;

    short v17 = *(short*)(v7 + 136);
    *(short*)(c + 504) = (v17 == -1)
        ? (short)(skinLo + 4 * skinHi + 912)
        : (short)(v17 + 400);

    short v21 = *(short*)(v7 + 204);
    *(short*)(c + 528) = (v21 == -1)
        ? (short)(skinLo + 4 * skinHi + 919)
        : (short)(v21 + 400);

    short v22 = *(short*)(v7 + 272);
    *(short*)(c + 552) = (v22 == -1)
        ? (short)(skinLo + 4 * skinHi + 926)
        : (short)(v22 + 400);

    short v26 = *(short*)(v7 + 340);
    *(short*)(c + 576) = (v26 == -1)
        ? (short)(skinLo + 4 * skinHi + 933)
        : (short)(v26 + 400);

    short v27 = *(short*)(v7 + 408);
    *(short*)(c + 600) = (v27 == -1)
        ? (short)(skinLo + 4 * skinHi + 940)
        : (short)(v27 + 400);

    *(unsigned char*)(c + 506) = (unsigned char)((*(int*)(v7 + 140) >> 3) & 0xF);
    *(unsigned char*)(c + 530) = (unsigned char)((*(int*)(v7 + 208) >> 3) & 0xF);
    *(unsigned char*)(c + 554) = (unsigned char)((*(int*)(v7 + 276) >> 3) & 0xF);
    *(unsigned char*)(c + 578) = (unsigned char)((*(int*)(v7 + 344) >> 3) & 0xF);
    *(unsigned char*)(c + 602) = (unsigned char)((*(int*)(v7 + 412) >> 3) & 0xF);
    *(unsigned char*)(c + 507) = *(unsigned char*)(v7 + 163);
    *(unsigned char*)(c + 531) = *(unsigned char*)(v7 + 231);
    *(unsigned char*)(c + 555) = *(unsigned char*)(v7 + 299);
    *(unsigned char*)(c + 579) = *(unsigned char*)(v7 + 367);
    *(unsigned char*)(c + 603) = *(unsigned char*)(v7 + 435);

    SetCharacterScale((int)c);
    // sub_47E3C0(CharacterMachine) — full stat recalculation (CharData_RecalcStats).
    // La implementación real son ~293 bytes de tiradas de base/máximo/random sobre los slots
    // 1374..1407 de CharacterMachine (HP/MP/ATK/DEF/AS/crítico/acierto). No hace falta para el char-select; el gate
    // que la rodea (entity[+847] != 0) sólo dispara después de entrar al mundo. El stub de abajo mantiene
    // el link verde; portar la implementación completa cuando se retome el trabajo in-game.
    FUN_0047e3c0((int)cd, 0, 0);
}

// sub_47E3C0 @ 0x0047E3C0 (293 bytes) — CharData_RecalcStats wrapper.
// Port FIEL desde IDA decompile (2026-05-02). Calls 8 stat helpers in order
// then computes derived stats:
//   - this[+1396] = this[+1388] - this[+1378]   (rango de stat máx - mín)
//   - this[+1398] = this[+1390] - this[+1378]
//   - this[+1400] = this[+1374] + rand%(1376-1374+1) - this[+78]  (tirada de daño aleatorio)
//   - this[+1402] = this[+58]  - this[+1384] (clampeado a 100)  (% de crítico)
//   - this[+1404] = this[+76]  - this[+1382] (clampeado a 100)  (% de esquive)
//   - this[+1406] = (rand()%100 < this[+1402]) ? 1 : 0  (tirada de crítico)
//   - this[+1407] = (rand()%100 < this[+1404]) ? 1 : 0  (tirada de esquive)
int __cdecl FUN_0047e3c0(int characterMachine, int /*p2*/, int /*p3*/) {
    int this_ = characterMachine;
    if (!this_) return 0;

    // Anti-tamper hash table — skipped per project policy

    FUN_0047d410(this_);            // Stats_CalcBase (attack damage)
    FUN_0047dae0(this_);            // Stats_CalcMagicDmgRange
    FUN_0047dd50((short*)this_);    // Stats_CalcAddStrength
    FUN_0047dd80(this_);            // CalculateAttackSpeed
    FUN_0047dfe0(this_);            // Stats_CalcDefense
    FUN_0047e160(this_);            // Stats_CalcCritBase / DefRate
    FUN_0047e2e0((short*)this_);    // Stats_CalcExtraOption1
    FUN_0047e310(this_);            // Stats_CalcExtraOption2

    // Derived stats: max-min ranges
    unsigned short v2 = *(unsigned short*)(this_ + 1390);
    *(unsigned short*)(this_ + 1396) = *(unsigned short*)(this_ + 1388) - *(unsigned short*)(this_ + 1378);
    *(unsigned short*)(this_ + 1398) = v2 - *(unsigned short*)(this_ + 1378);

    // Random damage roll
    unsigned short v3 = *(unsigned short*)(this_ + 1374);
    int range = *(unsigned short*)(this_ + 1376) - v3 + 1;
    if (range < 1) range = 1;
    int v4 = rand() % range;
    *(unsigned short*)(this_ + 1400) = (unsigned short)(v3 + v4 - *(unsigned short*)(this_ + 78));

    // Crit chance % (clamped to 100)
    unsigned short v5 = *(unsigned short*)(this_ + 76);
    unsigned short v6 = (unsigned short)(*(unsigned short*)(this_ + 58) - *(unsigned short*)(this_ + 1384));
    *(unsigned short*)(this_ + 1402) = v6;
    *(unsigned short*)(this_ + 1404) = (unsigned short)(v5 - *(unsigned short*)(this_ + 1382));
    if (*(unsigned short*)(this_ + 1402) > 100) *(unsigned short*)(this_ + 1402) = 100;
    if (*(unsigned short*)(this_ + 1404) > 100) *(unsigned short*)(this_ + 1404) = 100;

    // Did-crit / did-dodge rolls
    *(unsigned char*)(this_ + 1406) = (rand() % 100 < *(unsigned short*)(this_ + 1402)) ? 1 : 0;
    int v7 = rand();
    *(unsigned char*)(this_ + 1407) = ((v7 % 100) < *(unsigned short*)(this_ + 1404)) ? 1 : 0;
    return v7 / 100;
}
// FUN_004ac140 @ 0x004AC140 — NPC_Script_Tick(void)
// Scans NPC script table (DAT_07cf5600, stride 8, 100 entries).
// Entry layout: [0]=active(1), [1]=required_substate, [2]=min_x, [3]=min_y, [4]=max_x, [5]=max_y, [8]=speed
// Si se cumplen las condiciones y pasaron 3000ms: manda el keepalive C1/01/1C y actualiza el estado.
// Además: si DAT_07e11d1c no es nulo o entity+0x305 está seteado → muestra texto de UI vía FUN_00480620.
// Las llamadas a HashTable (FUN_0043d3e0 / FUN_004233e0) sobre las lecturas de cached_wp son ruido anti-tamper.
void __cdecl FUN_004ac140(void)
{
    static const unsigned char xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    for (int entry = 0; entry < 100; entry++) {
        // Entry address in script table (stride 8)
        // DAT_07cf5600 es un puntero DWORD a un buffer alocado con malloc
        char* scriptBase = (char*)DAT_07cf5600;
        char* eptr  = scriptBase + entry * 8;

        // Tiene que estar activo y coincidir con el sub-estado actual
        if (eptr[0] != '\x01') goto next_entry;
        if ((unsigned char)eptr[1] != (unsigned char)DAT_0055a7ac) goto next_entry;

        {
            char* playerEntity = DAT_07abf5d8;
            int cachedX = *(int*)(playerEntity + 0x388);
            int cachedY = *(int*)(playerEntity + 0x38c);

            // Bounding box check
            if ((int)(unsigned char)eptr[2] > cachedX) goto next_entry;
            if ((int)(unsigned char)eptr[3] > cachedY) goto next_entry;
            if (cachedX > (int)(unsigned char)eptr[4]) goto next_entry;
            if (cachedY > (int)(unsigned char)eptr[5]) goto next_entry;

            // Check dialog state and player busy flag
            if (DAT_07e11d1c != 0 || *(char*)(playerEntity + 0x305) != '\0') {
                // Dialog active or busy — show UI messages but don't send
                // (las llamadas a FUN_00480620 se omiten — son sólo UI)
                goto next_entry;
            }

            // Speed threshold for running entities
            unsigned int speedReq = (unsigned int)(unsigned char)eptr[8];
            if ((*(unsigned char*)(playerEntity + 0x1bc) & 7) == 3)
                speedReq = (speedReq << 1) / 3;

            // Chequeo especial para la entrada 0x1c: compara el nivel con el de CharData
            if (entry == 0x1c) {
                unsigned short charLevel = *(unsigned short*)((char*)DAT_07cf1ff4 + 0xe);
                if (charLevel < speedReq) goto acec0;
            }

            // Timer check: 3000ms keepalive
            DWORD now = GetTickCount();
            if (now - DAT_07e11dc8 <= 2999) {
                DAT_07e11dc4 = 0;
                DAT_07e11d1c = 0;
                goto next_entry;
            }

            // Hora de enviar: marca la primera entrada
            if (entry == 0) {
                DAT_05826d14 = '\x01';
            }

            // Build C1/01/1C keepalive packet
            // Payload: C1 04 00 1C (4 bytes before XOR)
            unsigned char pktBuf[8];
            pktBuf[0] = 0xC1;
            pktBuf[1] = 4;
            pktBuf[2] = 0x00;
            pktBuf[3] = 0x1C;

            // Codifica con XOR el byte 3 (opcode) — sólo hay 1 byte de datos después del header
            pktBuf[3] ^= xorKey[3 & 0x1f] ^ pktBuf[2];

            // Send
            if (DAT_055ca168 != 0xFFFFFFFF) {
                int offset = 0, remaining = 4;
                unsigned int pktLen = 4;
                while (remaining > 0) {
                    int sent = send((SOCKET)DAT_055ca168, (const char*)pktBuf + offset, remaining, 0);
                    if (sent == -1) {
                        int err = WSAGetLastError();
                        if (err == 0x2733) {
                            if ((int)(DAT_055cc16c + pktLen) < 0x2001) {
                                memcpy((char*)DAT_055ca16c + DAT_055cc16c, pktBuf, pktLen);
                                DAT_055cc16c += pktLen;
                            } else {
                                Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                            }
                        } else {
                            Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                        }
                        break;
                    }
                    if (sent == 0) break;
                    remaining -= sent;
                    offset += sent;
                    if (DAT_055ce174 != 0) FUN_0043de60();
                }
            }

            // Post-send: clear hover targets, set dialog-active, reset timer state
            SelectedItem = 0xffffffff;
            SelectedNpc = 0xffffffff;
            SelectedCharacter = 0xffffffff;
            SelectedOperate = 0xffffffff;
            DAT_00559c58 = 0xffffffff;
            DAT_07e11dc4 = 1;
            DAT_07e11db8 = 0;
            goto next_entry;
        }

acec0:;
next_entry:;
    }
}

// Combat_SendMovePathPacket (Send_MovePacket), Combat_DispatchHeroSkillAttack (Attack), Combat_CheckArrowRequirement (CheckArrow),
// Combat_UseElfSkill (UseSkillElf), Combat_ProcessQueuedAction (Action big switch),
// + Send_MovePacket_Player_legacy_stub moved to src/Combat/Combat.cpp
// (B3 refactor 2026-05-07, 1216 lines).

// FUN_004f6c30 @ 0x004F6C30 — Terrain_GetAttrDirect(grid_x, grid_y) → grid_y * 0x100 + grid_x
int  __cdecl FUN_004f6c30(int param_1, int param_2) { return param_2 * 0x100 + param_1; }
// FUN_004f9ac0 @ 0x004F9AC0 — RenderTerrain(EditFlag)  ── PORT 1:1 (2026-06-27)
// Reemplaza la fallback flat-shaded previa por el decompile fiel de IDA (352 b).
// Mantiene flujo y orden de Render States del binario:
//   sub_4F98C0 (terrain light setup) → WaterMove update → [Edit: SelectFlag=0 +
//   Map_InitRayCast | Run: DisableAlphaBlend] → TerrainFlag=0 →
//   RenderTerrainFrustrum(EditFlag) → [Edit: si SelectFlag, RenderTerrainTile del
//   tile pickeado | Run: EnableAlphaTest(1) → overlay pass TerrainFlag=2 →
//   ambient objects → DisableDepthTest/EnableCullFace/AlphaBitmaps/EnableDepthTest]
//   → toggle ^=1 → sub_4F9A30.
//
// Dependencias verificadas en IDA (bytes de operando):
//   Hero=0x07abf5d8, World=g_GameSubState(0x0055a7ac), WorldTime=0x05826e08,
//   WaterMove=0x07eeb214, SelectFlag=0x07eab1fc, SelectXF/YF=0x080ab288/28c,
//   TerrainFlag=0x0838bc44, toggle=0x0839bc88, unk_55A76C=0x0055a76c.
//   - WorldTime: en el binario es float ((float)timeGetTime() en CalcFPS 0x43FD70);
//     en nuestro codebase es int g_AnimTick y todos sus readers lo usan como int.
//     Se lee (int)WorldTime % N (equivalente; cambiar el tipo rippléaría a decenas
//     de funciones fuera de esta cadena). DEPENDENCIA reportada, no modificada.
//   - unk_55A76C: único xref es el read de abajo (sin writer en el binario) → el
//     2º pass overlay (TerrainFlag=2) es inerte también en el original.
//   - Callees aún fallback (a portar en esta cadena): RenderTerrainFrustrum_stub
//     (#2, 0x004F97E0), FUN_004f8480 RenderTerrainTile (#3, 0x004F8480).
void __cdecl FUN_004f9ac0(char EditFlag) {
    FUN_004f98c0(
        (int)(*(float*)(Hero + 16) * 0.039999999f),   // hero X * 0.04 → tile coord
        (int)(*(float*)(Hero + 20) * 0.039999999f),   // hero Y * 0.04
        3,
        -70,
        (int)DAT_0839bc88);

    if (World == 8)
        DAT_07eeb214 = (float)((int)WorldTime % 40000) * 0.000024999999f;  // WaterMove (Tarkan)
    else
        DAT_07eeb214 = (float)((int)WorldTime % 20000) * 0.000049999999f;  // WaterMove

    if (EditFlag) {
        DAT_07eab1fc = 0;                 // SelectFlag = 0
        FUN_00512d30();                   // Map_InitRayCast (sub_512D30)
    } else {
        GL_ResetState();                   // DisableAlphaBlend
    }

    DAT_0838bc44 = 0;                     // TerrainFlag = 0
    RenderTerrainFrustrum_stub(EditFlag != 0);

    if (EditFlag) {
        if (DAT_07eab1fc) {               // SelectFlag → render del tile pickeado
            float sxf = *(float*)&DAT_080ab288;   // SelectXF
            float syf = *(float*)&DAT_080ab28c;   // SelectYF
            FUN_004f8480(*(int*)&sxf, *(int*)&syf, (int)sxf, (int)syf,
                         1.0f, 1, (int)(unsigned char)EditFlag);
        }
    } else {
        GL_SetBlendSrcOver('\x01');             // EnableAlphaTest(1)
        if (DAT_0055a76c && World != 7) { // overlay (inerte: unk_55A76C nunca seteado)
            DAT_0838bc44 = 2;             // TerrainFlag = 2
            RenderTerrainFrustrum_stub(false);
        }
        FUN_004f7060();                   // Terrain_SpawnAmbientObjects (sub_4F7060)
        GL_DisableDepthTest();                   // DisableDepthTest
        GL_EnableCullFace();                   // EnableCullFace
        FUN_00479540();                   // RenderTerrainAlphaBitmaps (sub_479540)
        GL_EnableDepthTest();                   // EnableDepthTest
    }

    DAT_0839bc88 ^= 1u;                   // terrain-light double-buffer toggle
    FUN_004f9a30((int)DAT_0839bc88);
}

#if 0
// ── Fallback flat-shaded previa (REEMPLAZADA por el port 1:1 de RenderTerrain) ──
// Bloque inactivo, conservado solo como referencia del path cámara/proyección.
static void RenderTerrain_FallbackUnused(char EditFlag) {
    if (!DAT_07abf5d8) return;
    // BUG-FIX 2026-04-28: Edit mode (EditFlag=1) lo llama Player_InputTick para
    // mouse picking — necesita iterar tiles y llamar a FUN_004f8480 con flag de
    // picking, que calcula la intersección rayo-tile y guarda el resultado en
    // DAT_080ab288 / DAT_080ab28c. Sin esto el click al suelo no genera path,
    // entonces el hero nunca se mueve.

    // Simplified iteration: 64x64 tile area centered on hero.
    if (!DAT_07abf5d8) return;
    BYTE* hero = (BYTE*)DAT_07abf5d8;
    float hx = *(float*)(hero + 0x10);
    float hy = *(float*)(hero + 0x14);
    int gridCenterX = (int)(hx / _DAT_005524f0);
    int gridCenterY = (int)(hy / _DAT_005524f0);

    // BUG-FIX 2026-04-29: 64x64 era muy chico. Cámara con pitch -55° y zoom
    // estándar muestra ~80-100 tiles. Cursor podía caer fuera del scan.
    int xStart = gridCenterX - 64, xEnd = gridCenterX + 64;
    int yStart = gridCenterY - 64, yEnd = gridCenterY + 64;
    if (xStart < 0) xStart = 0;
    if (yStart < 0) yStart = 0;
    if (xEnd > 254) xEnd = 254;
    if (yEnd > 254) yEnd = 254;

    if (EditFlag) {
        // Mouse-pick mode: don't draw, just iterate tiles + call FUN_004f8480
        // with picking flag.  FUN_004f8480 internally tests mouse ray against
        // el quad del tile y guarda las coordenadas de grilla del impacto en DAT_080ab288/28c.
        for (int yi = yStart; yi < yEnd; ++yi) {
            for (int xi = xStart; xi < xEnd; ++xi) {
                float xf = (float)xi;
                float yf = (float)yi;
                FUN_004f8480(*(int*)&xf, *(int*)&yf, xi, yi, 1.0f, 1, (int)'\x01');
            }
        }
        return;
    }

    // IDA RenderTerrain (0x004F9AC0) abre el path EditFlag=0 con DisableAlphaBlend().
    // Usamos el wrapper 1:1 (GL_ResetState) en vez de glEnable/glDisable crudo para
    // mantener sincronizado el caché de estado GL (AlphaBlendType/AlphaTestEnable/
    // TextureEnable) que comparten todos los passes. Estado resultante: blend OFF,
    // cull ON, escritura de profundidad ON, alpha-test OFF, textura ON.
    GL_EnableDepthTest();                  // EnableDepthTest (la fallback dibuja con depth test)
    GL_ResetState();                  // DisableAlphaBlend — estado opaco base

    // Trackea la última textura bindeada para minimizar los binds.
    int lastTex = -1;

    for (int yi = yStart; yi < yEnd; ++yi) {
        for (int xi = xStart; xi < xEnd; ++xi) {
            float xf = (float)xi;
            float yf = (float)yi;
            int idx0 = yi * 256 + xi;
            int idx1 = yi * 256 + (xi + 1);
            int idx2 = (yi + 1) * 256 + (xi + 1);
            int idx3 = (yi + 1) * 256 + xi;
            float wx0 = xf * 100.0f, wy0 = yf * 100.0f;
            float wx1 = wx0 + 100.0f, wy1 = wy0 + 100.0f;
            float h0 = DAT_080cb2cc[idx0];
            float h1 = DAT_080cb2cc[idx1];
            float h2 = DAT_080cb2cc[idx2];
            float h3 = DAT_080cb2cc[idx3];

            // BUG-FIX 2026-05-01: el .map almacena tile-type INDICES (0-13)
            // por celda (TileGrass01=0, TileGrass02=1, TileGround01=2, ...,
            // TileRock07=13). El GL slot real es 0x23 + index. Antes el código
            // usaba el index directo como slot → leía texturas de UI/items
            // (slots 0..0xC son cursor sprites) → "suelo mosaico" UI.
            // Tiles cargados en OpenWorld: 0x23 grass1, 0x24 grass2, 0x25 ground1,
            // 0x26 ground2, 0x27 ground3, 0x28 water, 0x29 wood, 0x2a-0x30 rock1-7.
            int tileIdx = (int)(unsigned char)DAT_080bb2b4[idx0];
            int tileTex = 0x23 + tileIdx;
            if (tileTex < 0x23 || tileTex > 0x30) tileTex = 0x23;   // fallback grass01
            if (tileTex != lastTex) {
                GL_BindTextureSlot(tileTex);          // glBindTexture (texture system)
                lastTex = tileTex;
            }

            // Per-vertex lighting (DAT_0828b608 ya confirmado en valores 0.84-0.94 OK).
            // BUG-FIX 2026-05-03: algunos tiles se renderizan violeta/magenta porque
            // DAT_0828b608 tiene valores por canal raros (p.ej. R=alto, G=0, B=alto)
            // en tiles donde TerrainLight.OZJ tiene un color inusual o
            // FUN_004f71c0 wasn't fully populated. Helper sanitizes per-vertex
            // luz: si algún canal está en 0, lo reemplazamos por el promedio de los otros o 0.85.
            auto sanitize = [](float r, float g, float b, float* out) {
                int zeros = (r == 0.0f) + (g == 0.0f) + (b == 0.0f);
                if (zeros >= 2) {
                    out[0] = out[1] = out[2] = 0.85f;
                    return;
                }
                if (zeros == 1) {
                    float avg = (r + g + b) * 0.5f;
                    r = (r == 0.0f) ? avg : r;
                    g = (g == 0.0f) ? avg : g;
                    b = (b == 0.0f) ? avg : b;
                }
                // Anti-violet: si la diferencia entre el canal max y min es muy
                // grande (>0.4), el tile tiene una dominancia chromática
                // probablemente del lightmap mal-interpretado. Promediamos para
                // eliminar el tinte saturado pero conservar brillo.
                float mx = r > g ? (r > b ? r : b) : (g > b ? g : b);
                float mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
                if (mx - mn > 0.4f) {
                    float avg = (r + g + b) * (1.0f / 3.0f);
                    out[0] = out[1] = out[2] = avg;
                    return;
                }
                out[0] = r; out[1] = g; out[2] = b;
            };
            float c0[3], c1[3], c2[3], c3[3];
            sanitize(DAT_0828b608[idx0*3+0], DAT_0828b608[idx0*3+1], DAT_0828b608[idx0*3+2], c0);
            sanitize(DAT_0828b608[idx1*3+0], DAT_0828b608[idx1*3+1], DAT_0828b608[idx1*3+2], c1);
            sanitize(DAT_0828b608[idx2*3+0], DAT_0828b608[idx2*3+1], DAT_0828b608[idx2*3+2], c2);
            sanitize(DAT_0828b608[idx3*3+0], DAT_0828b608[idx3*3+1], DAT_0828b608[idx3*3+2], c3);

            glBegin(GL_QUADS);
            glColor3fv(c0);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(wx0, wy0, h0);
            glColor3fv(c1);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(wx1, wy0, h1);
            glColor3fv(c2);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(wx1, wy1, h2);
            glColor3fv(c3);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(wx0, wy1, h3);
            glEnd();
        }
    }

    // IDA RenderTerrain: tras el pass de terreno llama EnableAlphaTest(1). Deja
    // GL_ALPHA_TEST ON (+ blend SRC_ALPHA/ONE_MINUS_SRC_ALPHA, cull OFF) y ese
    // estado PERSISTE hacia los passes de objetos del mundo (Terrain_Render →
    // Entity_PrepareRender), que renderean vegetación/decals con alpha. Sin esto
    // GL_ALPHA_TEST quedaba OFF → las esquinas de los quads con alpha salían
    // opacas = los triángulos negros alrededor de vegetación/objetos.
    GL_SetBlendSrcOver('\x01');            // EnableAlphaTest(1)
}
#endif  // fallback flat-shaded obsoleta

// ─────────────────────────────────────────────────────────────────────────────
// FUN_00449900 @ 0x00449900 — MoveCharacter(c, o)  (full port from IDA, 2026-05-04)
// ─────────────────────────────────────────────────────────────────────────────
// Tick de entidad por frame. Decompile de IDA: 2773 líneas / 32793 bytes; ~70% es
// anti-tamper hash-table noise (dword_55C9BC8/BCC/BD0/BD4 + sub_403F80/04280/
// 04330/0423710/0404400 ref-count + XOR encryption around CharacterMachine
// lecturas). Per la política del proyecto (CLAUDE.md) todas las operaciones de hash-table se saltean
// — son ofuscación, no lógica de juego.
//
// El llamador pasa la misma entidad como `c` y como `o` (FUN_00454fc0 → cc, cc).
// Mapped: `c == o == ent`. Behavior:
//   1. Sync entity world pos+rot → Models[entType] render slot
//   2. Hero-only: decrement attack/magic speed buff timers (CA+42/44),
//      recalc stats when timer expires
//   3. Stationary NPC tagging (event NPCs 206..208 in cities)
//   4. Knockback/tambaleo (c+772) y deslizamiento al objetivo (c+773)
//   5. Pickup-bag bouncing physics (entType=236)
//   6. Death dissolve (c+765 ragdoll)
//   7. CharacterAnimation() drives idle action selection
//   8. Attack swing (c+757) → AttackStage / AttackEffect
//   9. Bone particle effect spawn (o+404)
//  10. Skill effect dispatch from c+770 queue (cases 5/8/9/10/12/13/14/30..36/
//      41/42/48/49/55/56) → CreateEffect/CreateJoint/PlayBuffer
//  11. Targeted-skill effects when c+784 != -1 (cases 1/2/4/7/11/16/17/24/26/27/
//      28/51/52)
//  12. Joint chain rendering (c+816 ∈ 1253/1254/1256/1260) — sword/spear trail
//  13. Weapon blur trail (CreateBlur) when AnimFrame >= 3.0
//  14. Pickup-bag splash (entType=236)
//
// Original signature: `void __cdecl MoveCharacter(DWORD c, DWORD o)`. Our ABI
// es `void __cdecl FUN_00449900(int p1)`; los dos argumentos son la misma entidad.
//
// Dependencias (todas ya en el árbol como FUN_xxxxxxxx):
//   FUN_0047d410   Stats_CalcBase (sub_47D410)
//   FUN_0047dae0   Stats_CalcMagicDmgRange (sub_47DAE0)
//   FUN_0047dd80   CHARACTER_MACHINE::CalculateAttackSpeed
//   FUN_004f7500   RequestTerrainHeight
//   Particle_Spawn   Particle_Spawn
//   CharacterAnimation (alias of FUN_00448600)
//   FUN_004430c0   SetPlayerStop
//   FUN_0043e820   SetAction
//   FUN_00481ba0   CreateChat
//   FUN_0046c680   CreateBlood (CreateBlood_stub)
//   FUN_00449840   DeleteCloth
//   FUN_00448930   AttackStage (AttackStage_stub)
//   FUN_00445230   AttackEffect (existing port at line 3083)
//   Effect_Create   CreateEffect
//   Joint_Create   CreateJoint
//   Effect_SpawnSmokeBurst   CreateBomb
//   BMD_TransformPosition   BMD::TransformPosition
//   FUN_00440060   BMD::Animation
//   AngleMatrix    (no FUN_)
//   VectorRotate   = Vector_InverseRotate
//   FUN_004b1170   FindHotKey (FindHotKey_stub)
//   FUN_00474bd0   CreateArrows (CreateArrows_stub)
//   FUN_005129f0   fabs
//   FUN_0046fe40   Joint_Find
//   FUN_004451c0   AngleVectorOffset
//   Effect_SpawnBombRing   bomb-ring effect
//   FUN_0046c5a0   skill impact particles
//   FUN_0046c7f0   directional blood
//   FUN_0045fae0   lectura hash de 1 byte (lectura de la cola de skills)
//   FUN_0043e5c0   Alpha
//   FUN_00404bc0   PlayBuffer
//   SetPlayerDie / DeleteJoint / CreateBlur — local helpers below
//
// Anti-tamper SKIPPED everywhere — all `if (c == Hero) { hash-decrypt; ...
// hash-encrypt; }` colapsan a una lectura directa de los campos de CharacterMachine.
//
// Declaraciones externas locales a esta unidad de traducción:
extern "C" bool __cdecl CharacterAnimation(int c, int o);
extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_00449840(int c, int o, int flag);   // DeleteCloth
extern bool __cdecl AttackStage_stub(DWORD c, DWORD o);
extern void __cdecl CreateBlood_stub(DWORD o);
extern int  __stdcall FindHotKey_stub(int Skill);
extern void __cdecl CreateArrows_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey);
extern unsigned char __cdecl FUN_0045fae0(DWORD ecx, unsigned char* p);

// Helpers definidos localmente, usados sólo por MoveCharacter:
static inline void mc_AngleVectorOffset(float *origin7, float ax, float ay, float az, float out[3])
{
    // sub_4451C0 — offset = origin[4..6] + Rotate([ax,ay,az], AngleMatrix(origin[7..9]))
    float in1[3]  = { ax, ay, az };
    float in2[3][4];
    AngleMatrix(origin7 + 7, in2);
    VectorRotate(in1, (float*)in2, out);
    out[0] += origin7[4];
    out[1] += origin7[5];
    out[2] += origin7[6];
}

// 2026-08-24 FIX (Soul Barrier: arcos gruesos y "dobles"): esto era un NO-OP,
// con el comentario "no-op until joint pool wired" — pero el pool esta cableado
// desde 2026-05-08, cuando se porto `DeleteJoint` (0x0046FE00). Quedo el stub
// local y `MoveCharacter` siguio llamandolo, o sea el borrado nunca ocurria.
//
// Efecto medido: el skill 16 spawnea 5 joints 266 con Scale 20 y ANTES hace
// `DeleteJoint(266, Owner, 0)` para sacar los que ya hubiera. Los de Scale 50
// que deja `InsertBuffPhysicalEffect` (0x43BDE0, el camino de viewport cuando la
// entidad aparece con el buff ya activo) nunca se borraban, asi que convivian
// los dos grupos: censo del pool `quads20=3190 quads50=3190`, exactamente 50/50,
// ~10 joints donde el original tiene 5. De ahi que los arcos se vieran mas
// gruesos y cargados que en el cliente original.
extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType);
static inline void mc_DeleteJoint(int Type, DWORD Owner, int SubType)
{
    DeleteJoint(Type, Owner, SubType);
}

static inline void mc_CreateBlur(DWORD c, float* p1, float* p2,
                                 float* color, int type, int flag)
{
    // 0x0046C300 — CreateBlur (estela del arma). El pool no está alocado; se saltea per
    // Portado abajo desde IDA 0046C320; ahora respalda el pool de Trail_RenderAll.
    BYTE* const base = (BYTE*)g_RenderPool_07c608a8;
    constexpr int slotSize = 0x2f0;
    constexpr int slotCount = 100;

    for (int index = 0; index < slotCount; ++index) {
        BYTE* slot = base + index * slotSize;
        if (slot[0] && *(DWORD*)(slot + 12) == c) {
            BYTE* b = slot;
            *(int*)(b + 4) = type;
            *(float*)(b + 20) = color[0]; *(float*)(b + 24) = color[1]; *(float*)(b + 28) = color[2];
            const int previousCount = *(int*)(b + 16) - 1;
            for (int item = previousCount; item >= 0; --item) {
                float* dstA = (float*)(b + 44 + item * 12); float* srcA = dstA - 3;
                dstA[0] = srcA[0]; dstA[1] = srcA[1]; dstA[2] = srcA[2];
                float* dstB = (float*)(b + 404 + item * 12); float* srcB = dstB - 3;
                dstB[0] = srcB[0]; dstB[1] = srcB[1]; dstB[2] = srcB[2];
            }
            int count = *(int*)(b + 16) + 1;
            *(float*)(b + 32) = p1[0]; *(float*)(b + 36) = p1[1]; *(float*)(b + 40) = p1[2];
            *(float*)(b + 392) = p2[0]; *(float*)(b + 396) = p2[1]; *(float*)(b + 400) = p2[2];
            *(int*)(b + 16) = count >= 29 ? 29 : count;
            return;
        }
    }
    for (int index = 0; index < slotCount; ++index) {
        BYTE* slot = base + index * slotSize;
        if (!slot[0]) {
            *(DWORD*)(slot + 12) = c;
            slot[0] = 1;
            *(int*)(slot + 16) = 0;
            *(int*)(slot + 8) = flag ? 15 : 30;
            *(int*)(slot + 4) = type;
            *(float*)(slot + 20) = color[0]; *(float*)(slot + 24) = color[1]; *(float*)(slot + 28) = color[2];
            *(float*)(slot + 32) = p1[0]; *(float*)(slot + 36) = p1[1]; *(float*)(slot + 40) = p1[2];
            *(float*)(slot + 392) = p2[0]; *(float*)(slot + 396) = p2[1]; *(float*)(slot + 400) = p2[2];
            *(int*)(slot + 16) = 1;
            return;
        }
    }
}

static inline char mc_JointFind(int Type, DWORD Owner, int flag)
{
    // sub_46FE40 (FUN_0046fe40) — search joint pool for matching slot.
    // El pool no está alocado; siempre devuelve 0 (no encontrado).
    (void)Type; (void)Owner; (void)flag;
    return 0;
}

// FUN_00444d90 @ 0x00444D90 — SetPlayerDie (1057 bytes IDA, port FIEL 2026-05-07).
// Real signature: void __cdecl SetPlayerDie(DWORD c).
// (functions.h declaró FUN_00444d90 como "Entity_TeleportEnd" — eso es un
// mismap del port-time. La función AT 0x00444D90 ES SetPlayerDie per IDA.)
//
// Differentiation:
//   - Player (type 390), normal class (NOT 206-208): SetAction(c, 131)  ← death anim
//   - Player class 206-208 (special): explosion FX (210 + 211×10), c[0]=0
//   - NPC type 295 (=v13==0): explosion FX (226+227)×8, c[0]=0
//   - NPC type 300 (=v13==5): explosion FX (210 + 211×10), c[0]=0
//   - Other NPCs: SetAction(c, 6)  ← death anim
// El ruido de hash table (camino sólo-Hero, L32-119) se saltea per la política del proyecto.
void __cdecl FUN_00444d90(int c_in)
{
    DWORD c = (DWORD)c_in;
    if (!c) return;

    short v11 = *(short*)(c + 2);
    bool playFxBuf = false;

    if (v11 == 390) {
        int v12 = *(int*)(c + 4);
        if (v12 < 206 || v12 > 208) {
            FUN_0043e820((int)c, 131);
            goto LABEL_41;
        }
        // Player special class 206-208: explosion
        *(BYTE*)c = 0;
        Effect_Create(210, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                     nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
        for (int i = 0; i < 10; ++i) {
            Effect_Create(211, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                         nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
        }
        playFxBuf = true;
    } else {
        int v13 = v11 - 295;
        if (v13 == 0) {
            // Type 295: 8x (226+227)
            *(BYTE*)c = 0;
            for (int i = 0; i < 8; ++i) {
                Effect_Create(226, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                             nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
                Effect_Create(227, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                             nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            }
            playFxBuf = true;
        } else if (v13 == 5) {
            // Type 300: 1x 210 + 10x 211
            *(BYTE*)c = 0;
            Effect_Create(210, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                         nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            for (int i = 0; i < 10; ++i) {
                Effect_Create(211, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                             nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            }
            playFxBuf = true;
        } else {
            FUN_0043e820((int)c, 6);
            goto LABEL_41;
        }
    }
    if (playFxBuf) PlayBuffer(94, c, 0);

LABEL_41:
    // Death sound
    if (*(float*)(c + 264) == 0.0f) {
        short v14 = *(short*)(c + 2);
        if (v14 == 390) {
            int v15 = *(int*)(c + 4);
            if (v15 < 206 || v15 > 208) {
                if ((*(BYTE*)(c + 444) & 7) == 2) {
                    PlayBuffer(80, c, 0);
                } else {
                    PlayBuffer(78, c, 0);
                }
                return;
            }
        }
        // Non-player or player[206-208]: model action sound at +178
        short modSnd = *(short*)((char*)(uintptr_t)DAT_05828d58 + 188 * v14 + 178);
        if (modSnd != -1) {
            PlayBuffer(modSnd + 170, c, 0);
        }
    }
}

// mc_SetPlayerDie — wrapper que usa la lógica de envejecimiento del ragdoll de MoveCharacter.
// El SetPlayerDie de IDA NO toca dead_flag (lo hace el llamador, ReceiveDie).
// Acá agregamos los efectos secundarios del flag porque el camino del ragdoll de MoveCharacter
// espera que la entidad quede marcada como muerta después de esta llamada.
static inline void mc_SetPlayerDie(DWORD c)
{
    if (!c) return;
    FUN_00444d90((int)c);
    *(char*)(c + 0x2FD) = 1;          // ragdoll counter
    // 2026-07-27 FIX (alas rojas "PK"): NO setear dead_flag (0x34e) aquí. El
    // IDA SetPlayerDie NO lo toca — sólo ReceiveDie (el packet de muerte real)
    // lo setea. Este mc_SetPlayerDie lo llama el ragdoll-aging (c+765 counter);
    // si ese counter se activa espuriamente sobre el héroe VIVO, el dead_flag
    // quedaba stuck en 1 → el render (bDead = c+0x34e) lo trataba como muerto →
    // body light rojo (1.0,0.1,0.1) = el bug de "alas rojas PK" intermitente.
    // Confirmado por el diag HEROLIGHT (34e=1 con el pj vivo caminando).
    // Removidos también el clear de alive_flag (0x2EC) — sólo ReceiveDie maneja
    // la transición de estado de vida.
}

void __cdecl FUN_00449900(int p1)
{
    if (!p1) return;
    DWORD c = (DWORD)p1;
    DWORD o = (DWORD)p1;

    // Common locals
    float WorldPosition[3];
    float Position[3];
    float TargetPosition[3];
    float Light[3];
    float p1f[3];
    float p2f[3];
    float v389[3];     // out coord buffer for skill targets
    float v399[3];     // case 9 anchor pos
    float v394[3];     // case 9 angle
    float v396[3];     // case 48 base pos
    float Angle[3];    // case 48
    float v407[3];     // bone joint pos
    float v417[3];     // c+816 chain joint angle
    float v371[3], v372[3], v373[3], v379[3];  // CreateBlur params
    float Out[3];
    float in1[3], in2[3][4];
    float BoneMatrix[200][3][4];   // for case CreateBlur loop with BMD_Animation
    int   v422 = 188 * (*(short*)(o + 2)) + (int)DAT_05828d58;  // model slot
    bool  bEventNpc = false;
    DWORD Owner = 0;
    DWORD v390  = 0;
    int   v392 = 0;     // ground-item flag
    int   v393 = 0;     // skill-id captured (24)
    int   v375 = 0, Type = 0, v376 = 0, v378 = 0, v377 = 0;
    float v412, v413, v414;
    int   v48;          // entType16

    // ─── IDA L431-436: sync entity world pos → model render slot
    *(DWORD*)(v422 + 108) = *(DWORD*)(o + 16);   // x
    *(DWORD*)(v422 + 112) = *(DWORD*)(o + 20);   // y
    *(DWORD*)(v422 + 116) = *(DWORD*)(o + 24);   // z
    *(DWORD*)(v422 + 104) = *(DWORD*)(o + 12);   // scale
    *(BYTE*) (v422 + 160) = *(BYTE*) (o + 261);  // anim_state

    // ─── IDA L437-477: Hero-only buff timers (anti-tamper stripped)
    if (c == (DWORD)Hero)
    {
        if (CharacterAttribute)
        {
            // L449-457: attack-speed buff (CA+42 timer, CA+40 bit 0)
            unsigned short* t1 = (unsigned short*)((char*)CharacterAttribute + 42);
            if (*t1) (*t1)--;
            if (!*t1) {
                *((BYTE*)CharacterAttribute + 40) &= ~1u;
                FUN_0047dd80((int)(uintptr_t)CharacterMachine);  // CalculateAttackSpeed
            }
            // L458-467: magic-speed buff (CA+44 timer, CA+40 bit 1)
            unsigned short* t2 = (unsigned short*)((char*)CharacterAttribute + 44);
            if (*t2) (*t2)--;
            if (!*t2) {
                *((BYTE*)CharacterAttribute + 40) &= ~2u;
                FUN_0047d410((int)(uintptr_t)CharacterMachine);  // Stats_CalcBase
                FUN_0047dae0((int)(uintptr_t)CharacterMachine);  // Stats_CalcMagicDmgRange
            }
        }
    }

    // ─── IDA L478-498: tag stationary event NPCs in cities (Lorencia,Dungeon)
    if (*(BYTE*)(o + 132) == 4
        && (!World || World == 2)
        && *(short*)(o + 2) == 390
        && *(int*)(o + 4) >= 206 && *(int*)(o + 4) <= 208)
    {
        if (World) {
            *(DWORD*)(o + 28) = 0;
            *(DWORD*)(o + 32) = 0;
            *(DWORD*)(o + 36) = 0;
        } else {
            *(DWORD*)(o + 28) = 0;
            *(DWORD*)(o + 32) = 0;
            *(DWORD*)(o + 36) = 1119092736;  // 90.0f
        }
        bEventNpc = true;
    }

    // ─── IDA L499-502: decaimiento del knockback (mece el modelo hacia arriba mientras c+772 > 0)
    if (*(BYTE*)(c + 772)) {
        unsigned char k = (unsigned char)(*(BYTE*)(c + 772));
        (*(BYTE*)(c + 772))--;
        *(float*)(o + 36) += (float)(10 * k);
    }

    // ─── IDA L503-527: deslizamiento al objetivo (cuando c+773 está activo, interpola x/y a la grilla 774/775)
    if (*(BYTE*)(c + 773)) {
        v414 = 0.2f;
        if (*(short*)(o + 2) == 322) v414 = 0.07f;
        *(float*)(o + 16) += (((float)*(BYTE*)(c + 774) + 0.5f) * 100.0f - *(float*)(o + 16)) * v414;
        *(float*)(o + 20) += (((float)*(BYTE*)(c + 775) + 0.5f) * 100.0f - *(float*)(o + 20)) * v414;
        if (*(short*)(o + 2) != 236)
            *(float*)(o + 24) = FUN_004f7500(*(float*)(o + 16), *(float*)(o + 20));
        unsigned char step = (unsigned char)(*(BYTE*)(c + 773));
        (*(BYTE*)(c + 773))++;
        if (step > 15) {
            if (*(short*)(o + 2) == 322) FUN_004430c0((int)c);
            *(BYTE*)(c + 773) = 0;
        }
    }

    // ─── IDA L528-544: pickup-bag bouncing (entType=236)
    if (*(short*)(o + 2) == 236) {
        *(float*)(o + 24)  += *(float*)(o + 216);
        *(float*)(o + 216) -= 6.0f;
        v413 = FUN_004f7500(*(float*)(o + 16), *(float*)(o + 20)) + 30.0f;
        if (*(float*)(o + 24) < v413) {
            *(float*)(o + 24)  = v413;
            *(float*)(o + 216) = -*(float*)(o + 216) * 0.40000001f;
        }
        *(float*)(o + 28) += *(float*)(o + 192);
        *(float*)(o + 32) += *(float*)(o + 196);
        *(float*)(o + 36) += *(float*)(o + 200);
        *(float*)(o + 192) *= 0.80000001f;
        *(float*)(o + 196) *= 0.80000001f;
        *(float*)(o + 200) *= 0.80000001f;
    }

    // ─── IDA L545-606: death-dissolve (c+765=ragdoll, c+820=alpha-decay timer)
    if (*(BYTE*)(c + 765)) {
        if (*(short*)(o + 2) == 302 || World == 7)
            *(float*)(c + 820) += 0.050000001f;
        else
            *(float*)(c + 820) += 0.02f;

        v412 = 1.0f;
        if (*(float*)(c + 820) >= 1.0f) {
            *(float*)(o + 360) = 1.0f - (*(float*)(c + 820) - v412);
            if (*(float*)(o + 360) < 0.0099999998f) {
                if (c != (DWORD)Hero) *(BYTE*)o = 0;
            } else {
                *(float*)(o + 24) -= 0.40000001f;
            }
            FUN_00449840((int)c, (int)o, 0);  // DeleteCloth
        }
        // L572-592: Crywolf falling-debris physics (worlds 11..16)
        if (World >= 11 && World <= 16 && *(BYTE*)(o + 405)) {
            float out_[3] = {0,0,0};
            in1[0] = 0.0f; in1[1] = *(float*)(o + 196); in1[2] = 0.0f;
            AngleMatrix((float*)(o + 408), in2);
            VectorRotate(in1, (float*)in2, out_);
            *(float*)(o + 196) += *(float*)(o + 192);
            *(float*)(o + 216) += *(float*)(o + 204);
            *(float*)(o + 204) -= 5.0f;
            *(float*)(o + 200) -= 2.0f;
            *(float*)(o + 28)  -= 5.0f;
            if (*(float*)(o + 200) < 1.0f) *(DWORD*)(o + 200) = 1065353216;  // 1.0f
            *(float*)(o + 16) = out_[0] + *(float*)(o + 420);
            *(float*)(o + 20) = out_[1] + *(float*)(o + 424);
            *(float*)(o + 24) = *(float*)(o + 428) + *(float*)(o + 216);
        }
        // L593-605: Atlans bubble particles (world 7 in-game)
        if (g_GameState == 5 && World == 7) {
            for (int jj = 0; jj < 4; ++jj) {
                v407[0] = (float)(rand() % 128 - 64);
                v407[1] = (float)(rand() % 128 - 64);
                v407[2] = (float)(rand() % 256);
                v407[0] += *(float*)(o + 16);
                v407[1] += *(float*)(o + 20);
                v407[2] += *(float*)(o + 24);
                Particle_Spawn(1241, v407, (float*)(o + 28), (float*)(o + 232), 0, 1.0f, 0);
            }
        }
    }

    // ─── IDA L607: Alpha(o)
    FUN_0043e5c0((int)o);

    // ─── IDA L608-611: shaky timer decay
    if (*(float*)(c + 836) > 0.0f) *(float*)(c + 836) -= 0.029999999f;

    bool skipIdleSelect = false;

    // ─── IDA L612-614: tick de animación — si la acción actual continúa, salta a L195
    if (CharacterAnimation((int)c, (int)o)) {
        skipIdleSelect = true;
    }

    if (!skipIdleSelect)
    {
        // L616-618: clear chain-skill state
        *(int*)(c + 816)  = -1;
        *(BYTE*)(c + 844) = 0;
        *(BYTE*)(c + 845) = 0;

        // L619-739: player (entType=390) idle animation selection
        if (*(short*)(o + 2) == 390)
        {
            if (!bEventNpc)
            {
                // L623-631: hit/stun anims 131/132 → Blood + return
                if (*(BYTE*)(o + 261) == 131 || *(BYTE*)(o + 261) == 132) {
                    if (!*(BYTE*)(c + 766)) { *(BYTE*)(c + 766) = 1; CreateBlood_stub(o); }
                    return;
                }
                // L632-639: walking/running/dying actions stay; otherwise stop
                BYTE act = *(BYTE*)(o + 261);
                if (act < 0x0Du
                    || (act >= 0x22u && act <= 0x82u && act != 79 && act != 80)) {
                    FUN_004430c0((int)c);  // SetPlayerStop
                }
                skipIdleSelect = true;  // jump to L195
            }
            else
            {
                // L642-738: event NPC idle chatter / random pose dispatch
                int action = rand() % 100;
                if (*(BYTE*)(o + 261) == 1)  // PLAYER_STOP_MALE
                {
                    int TextIndex = 0;
                    if (action >= 80) {
                        if (action >= 85) {
                            if (action >= 90) {
                                if (action >= 95) {
                                    if (action < 100) {
                                        TextIndex = (World == 2) ? 905 : 0;
                                        if (!(rand() % 3) && TextIndex)
                                            FUN_00481ba0((char*)(c + 449), GlobalText[TextIndex], c, 0, -1);
                                        FUN_0043e820((int)c, 105);
                                    }
                                } else {
                                    TextIndex = (World == 2) ? 905 : 0;
                                    if (!(rand() % 3) && TextIndex)
                                        FUN_00481ba0((char*)(c + 449), GlobalText[TextIndex], c, 0, -1);
                                    FUN_0043e820((int)c, 111);
                                }
                            } else {
                                TextIndex = (World == 2) ? 904 : 823;
                                if (!(rand() % 2) && TextIndex)
                                    FUN_00481ba0((char*)(c + 449), GlobalText[TextIndex], c, 0, -1);
                                FUN_0043e820((int)c, 99);
                            }
                        } else {
                            TextIndex = (World == 2) ? 904 : 0;
                            if (!(rand() % 2) && TextIndex)
                                FUN_00481ba0((char*)(c + 449), GlobalText[TextIndex], c, 0, -1);
                            FUN_0043e820((int)c, 97);
                        }
                        skipIdleSelect = true;
                    }
                    if (!skipIdleSelect) {
                        TextIndex = (World == 2) ? 904 : 0;
                        if (!(rand() % 2) && TextIndex)
                            FUN_00481ba0((char*)(c + 0x1C1), GlobalText[TextIndex], c, 0, -1);
                    }
                }
                if (!skipIdleSelect) FUN_0043e820((int)c, 1);
                skipIdleSelect = true;  // fall to L195 either way (event NPCs done)
            }
        }
        else if (World == 1 && *(short*)(o + 2) == 40) {
            FUN_0043e820((int)o, 0);
        }
        else if (*(short*)(o + 2) < 270 || *(short*)(o + 2) >= 335) {
            // Non-player non-monster (NPCs, props)
            v48 = *(short*)(o + 2);
            if (v48 == 170 || v48 <= 337 || v48 > 339) {
                FUN_0043e820((int)o, rand() % 2);
            } else if (rand() % 16 >= 12) {
                FUN_0043e820((int)o, rand() % 2 + 1);
            } else {
                FUN_0043e820((int)o, 0);
            }
        }
        else {
            // Monster (270..334)
            if (*(BYTE*)(o + 261) == 6) {  // MONSTER01_DIE
                if (!*(BYTE*)(c + 766)) { *(BYTE*)(c + 766) = 1; CreateBlood_stub(o); }
                return;
            }
            BYTE act = *(BYTE*)(o + 261);
            if (act == 1 || act == 5 || act == 3 || act == 4 || act == 8 || act == 9) {
                FUN_0043e820((int)o, 0);
            }
        }
    }

    // ─── LABEL_195 (IDA L783): mid-way joint logic ──────────────────────────
    // L784-795: planta enredadera (tipo 377) — cambio de idle aleatorio al terminar la acción
    if (*(short*)(o + 2) == 377
        && *(short*)(v422 + 168)
            == *(short*)(*(int*)(v422 + 48) + 16 * (*(BYTE*)(o + 261)) + 8) - 1)
    {
        FUN_0043e820((int)o, (rand() % 32) ? 0 : 1);
    }

    // L796-806: ragdoll-counter aging (force die after 0xF frames)
    if (*(BYTE*)(c + 765)) {
        unsigned char inc = (unsigned char)(++*(BYTE*)(c + 765));
        if (inc >= 0xFu) mc_SetPlayerDie(c);
        if (World >= 11 && World <= 16 && *(BYTE*)(o + 405)) mc_SetPlayerDie(c);
    }

    // L807-811: setup defaults
    /*v424 unused*/ (void)((rand() % 6 + 2) * 0.1f);
    memset(p1f, 0, sizeof(p1f));
    Light[0] = 1.0f; Light[1] = 1.0f; Light[2] = 1.0f;

    // L812-817: attack swing tick
    if (*(BYTE*)(c + 757)) {
        AttackStage_stub(c, o);
        FUN_00445230((int)c);         // AttackEffect @ 00445230
        ++*(BYTE*)(c + 757);
    }

    // L818-839: bone-particle effect (o+404 = effect counter)
    if (*(BYTE*)(o + 404)) {
        DWORD This = 188 * (*(short*)(o + 2)) + (int)DAT_05828d58;
        memset(p1f, 0, sizeof(p1f));
        for (int kk = 0; kk < *(short*)(This + 34); ++kk) {
            int v401 = *(short*)(*(int*)(This + 44) + 140 * kk + 32);
            if (v401 > -1 && v401 < 200) {
                BMD_TransformPosition((void*)This, (float*)(48 * kk + *(int*)(o + 276)), p1f, Position, 1);
                BMD_TransformPosition((void*)This, (float*)(48 * v401 + *(int*)(o + 276)), p1f, TargetPosition, 1);
                Position[0] += (float)(rand() % 41 - 20);
                Position[1] += (float)(rand() % 41 - 20);
                Position[2] += (float)(rand() % 41 - 20);
                TargetPosition[0] += (float)(rand() % 41 - 20);
                TargetPosition[1] += (float)(rand() % 41 - 20);
                TargetPosition[2] += (float)(rand() % 41 - 20);
                Joint_Create(1254, Position, TargetPosition, (float*)(o + 28), 7, 0, 20.0f, (short)-1, 0);
            }
        }
        --*(BYTE*)(o + 404);
    }

    // ─── L840-2486: SKILL DISPATCH (when c+757 exhaust limit, drain c+770 queue)
    if (*(unsigned char*)(c + 757) >= (unsigned char)DAT_00559858)
    {
        *(BYTE*)(c + 757) = 0;
        *(short*)(o + 134) = -1;
        if (c == (DWORD)Hero && SelectedCharacter != -1) {
            DWORD selSlot = (DWORD)(916 * SelectedCharacter + (int)CharactersClient);
            // Guild-war target lock (skipped guild-mark strcmp — same guild = no target)
            if (EnableGuildWar
                && *(unsigned char*)(selSlot + 746) >= 6u
                && *(short*)(selSlot + 474) != -1
                && Hero
                && !strcmp((char*)((char*)&DAT_07e919bc + 80 * (*(short*)((int)Hero + 474))),
                           (char*)((char*)&DAT_07e919bc + 80 * (*(short*)(selSlot + 474)))))
            {
                *(short*)(o + 134) = -1;
            }
            else if ((EnableGuildWar && *(BYTE*)(selSlot + 745) == 2)
                  || (!EnableGuildWar && *(unsigned char*)(selSlot + 746) >= 6u)
                  || ((unsigned short)GetAsyncKeyState(17) >> 8))
            {
                *(short*)(o + 134) = *(short*)(selSlot + 476);
            }
        }

        // Lee el ID de skill encolado en c+770 (anti-tamper: venía envuelto en
        // ref-count + XOR; we use direct byte read since no encryption in our build).
        unsigned char v328 = *(BYTE*)(c + 770);
        if (v328 == 3 || v328 == 7) {
            char trace[128];
            wsprintfA(trace, "SKILL19 DISPATCH skill=%u hero=%d targetSlot=%d stage=%u",
                      (unsigned)v328, c == (DWORD)Hero,
                      (int)*(short*)(c + 784), (unsigned)*(BYTE*)(c + 757));
            DbgLogPublic(trace);
        }

        switch ((char)v328)
        {
        case 5: {  // FireBall
            WorldPosition[0] = ((float)*(BYTE*)(c + 776) + 0.5f) * 100.0f;
            WorldPosition[1] = ((float)*(BYTE*)(c + 777) + 0.5f) * 100.0f;
            WorldPosition[2] = FUN_004f7500(WorldPosition[0], WorldPosition[1]);
            int hk = FindHotKey_stub(5);
            Effect_Create(1200, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(91, 0, 0);
            break;
        }
        case 8: {  // Heal
            int hk = FindHotKey_stub(8);
            Effect_Create(204, (float*)(o + 16), (float*)(o + 28), Light, (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(86, 0, 0);
            break;
        }
        case 9: {  // Twister
            v399[0] = *(float*)(o + 16);
            v399[1] = *(float*)(o + 20);
            v399[2] = *(float*)(o + 24) + 100.0f;
            for (int kk = 0; kk < 4; ++kk) {
                v394[0] = 0.0f; v394[1] = 0.0f; v394[2] = (float)kk * 90.0f;
                int hk = FindHotKey_stub(9);
                Joint_Create(1253, v399, (float*)(o + 16), v394, 0, (int)o, 80.0f,
                             *(short*)(o + 134), (unsigned char)hk);
                Joint_Create(1253, v399, (float*)(o + 16), v394, 0, (int)o, 20.0f, (short)-1, 0);
            }
            PlayBuffer(87, 0, 0);
            break;
        }
        case 10: {  // Defense
            int hk = FindHotKey_stub(10);
            Effect_Create(200, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            Effect_Create(201, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                         (float*)(uintptr_t)-1, nullptr, 0);
            PlayBuffer(89, 0, 0);
            break;
        }
        case 12: {  // FallingSlash
            mc_AngleVectorOffset((float*)o, -20.0f, -90.0f, 100.0f, WorldPosition);
            int hk = FindHotKey_stub(12);
            Effect_Create(1210, WorldPosition, (float*)(o + 28), Light, (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(92, 0, 0);
            break;
        }
        case 13: {  // Lunge / Crescent
            WorldPosition[0] = ((float)*(BYTE*)(c + 776) + 0.5f) * 100.0f;
            WorldPosition[1] = ((float)*(BYTE*)(c + 777) + 0.5f) * 100.0f;
            WorldPosition[2] = FUN_004f7500(WorldPosition[0], WorldPosition[1]);
            int hk = FindHotKey_stub(13);
            Effect_Create(240, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            Effect_Create(240, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)(hk & 0xFF), 0);
            break;
        }
        case 14: {  // Decay (bomb-ring)
            Effect_SpawnBombRing((float*)(o + 16));
            int hk = FindHotKey_stub(14);
            Effect_Create(241, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            break;
        }
        case 30: case 31: case 32: case 33: case 34: case 35: case 36:  // Buff / Debuff
            Effect_Create(1264, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)3, (float*)o,
                         (float*)(uintptr_t)-1, nullptr, 0);
            break;
        case 41: {  // Bow special 1
            *(BYTE*)(o + 136) = (BYTE)(*(short*)(c + 624) + 112);
            *(BYTE*)(o + 137) = *(BYTE*)(c + 626);
            int hk = FindHotKey_stub(41);
            Effect_Create(238, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(85, 0, 0);
            break;
        }
        case 42: {  // Bow special 2
            *(BYTE*)(o + 136) = (BYTE)(*(short*)(c + 624) + 112);
            *(BYTE*)(o + 137) = *(BYTE*)(c + 626);
            int hk = FindHotKey_stub(42);
            Effect_Create(244, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(85, 0, 0);
            break;
        }
        case 48: {  // Mass-stun ring (36 vertical bolts)
            memset(Angle, 0, sizeof(Angle));
            int v398 = 36;
            for (int kk = 0; kk < v398; ++kk) {
                Angle[0] = -10.0f; Angle[1] = 0.0f; Angle[2] = (float)kk * 10.0f;
                v396[0] = *(float*)(o + 16);
                v396[1] = *(float*)(o + 20);
                v396[2] = *(float*)(o + 24) + 100.0f;
                Joint_Create(1253, v396, v396, Angle, 2, (int)o, 60.0f, 0, 0);
                if (!(kk % 20)) {
                    Effect_Create(1264, (float*)(o + 16), Angle, (float*)(o + 232), (float*)(uintptr_t)4, (float*)o,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                }
            }
            // L1664: seteo de flag sólo para el Hero (anti-tamper removido)
            // salteado — era una ronda de hash-encrypt sobre CharacterMachine
            // 2026-08-23: faltaba el sonido del skill (IDA L1817, justo despues
            // del bloque de hash-encrypt que se omite por policy).
            PlayBuffer(97, 0, 0);
            break;
        }
        case 49: {  // Magic — uses dword_5826D10 hotkey state
            int hk = (int)DAT_05826d10;
            Effect_Create(1382, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short*)(o + 134), (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(84, 0, 0);
            break;
        }
        case 55:
            *(BYTE*)(o + 136) = (BYTE)(*(short*)(c + 624) + 112);
            *(BYTE*)(o + 137) = *(BYTE*)(c + 626);
            break;
        case 56:
            *(BYTE*)(o + 136) = (BYTE)(*(short*)(c + 624) + 112);
            *(BYTE*)(o + 137) = *(BYTE*)(c + 626);
            break;
        default: break;
        }

        // ─── L1836-2477: targeted vs. self-cast skill effects
        if (*(short*)(c + 784) == -1)
        {
            // Self-cast (no target). Read action via FUN_0045fae0 (skipped XOR).
            v393 = (v328 == 24) ? 1 : 0;

            // L1917-2097: Bow/Crossbow arrow shoot (player anims 46..49,54,55)
            BYTE act = *(BYTE*)(o + 261);
            if (*(short*)(o + 2) == 390
                && (act == 46 || act == 47 || act == 48 || act == 49 || act == 54 || act == 55))
            {
                unsigned char arrowSkill = *(BYTE*)(c + 770);  // direct read (anti-tamper stripped)
                int hk = FindHotKey_stub(arrowSkill);
                CreateArrows_stub(c, o, 0, (WORD)hk, (WORD)v393, (WORD)*(BYTE*)(c + 770));
            }
            // L2098-2101: ranged-monster auto-arrows
            if (*(short*)(o + 2) == 292 || *(short*)(o + 2) == 305 || *(short*)(o + 2) == 310 || *(short*)(o + 2) == 316)
            {
                CreateArrows_stub(c, o, 0, 0, 1, 0);
            }
        }
        else
        {
            // Targeted skill — Owner = CharactersClient[c+784]
            v390 = (DWORD)(916 * (*(short*)(c + 784)) + (int)CharactersClient);
            Owner = v390;

            if (v328 == 3 || v328 == 7) {
                char trace[128];
                wsprintfA(trace, "SKILL19 TARGET skill=%u slot=%d key=%u ownerType=%d",
                          (unsigned)v328, (int)*(short*)(c + 784),
                          (unsigned)*(unsigned short*)(Owner + 476),
                          (int)*(short*)(Owner + 2));
                DbgLogPublic(trace);
            }

            // L2107-2197: bow/crossbow arrow path (player only)
            BYTE act = *(BYTE*)(o + 261);
            if (*(short*)(o + 2) == 390
                && (act == 46 || act == 47 || act == 48 || act == 49 || act == 54 || act == 55))
            {
                unsigned char arrowSkill = *(BYTE*)(c + 770);
                int hk = FindHotKey_stub(arrowSkill);
                CreateArrows_stub(c, o, Owner, (WORD)hk, 0, (WORD)*(BYTE*)(c + 770));
            }
            // L2198-2201: ranged-monster
            if (*(short*)(o + 2) == 292 || *(short*)(o + 2) == 305 || *(short*)(o + 2) == 310)
                CreateArrows_stub(c, o, Owner, 0, 1, 0);

            // L2202-2229: target halo + Atlans bubble shower
            if (*(short*)(v390 + 760)) {
                if (*(short*)(Owner + 2) != 277) {
                    for (int kk = 0; kk < 10; ++kk) {
                        WorldPosition[0] = (float)(rand() % 64 - 32) + *(float*)(Owner + 16);
                        WorldPosition[1] = (float)(rand() % 64 - 32) + *(float*)(Owner + 20);
                        WorldPosition[2] = (float)(rand() % 64 + 90) + *(float*)(Owner + 24);
                        Particle_Spawn(1206, WorldPosition, (float*)(o + 28), Light, 0, 1.0f, 0);
                    }
                }
                if (*(short*)(Owner + 2) == 330) {
                    for (int kk = 0; kk < 5; ++kk) {
                        if (!(rand() % 2)) {
                            WorldPosition[0] = *(float*)(Owner + 16);
                            WorldPosition[1] = *(float*)(Owner + 20);
                            float v45 = *(float*)(Owner + 24) + 50.0f;
                            WorldPosition[2] = (float)(rand() % 30) + v45;
                            Effect_Create(261, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                         (float*)(uintptr_t)-1, nullptr, 0);
                        }
                    }
                    PlayBuffer(107, 0, 0);
                }
            }

            // L2230-2233: Magic anim 0x38..0x3C bonus particles
            if (*(unsigned char*)(o + 261) >= 0x38u && *(unsigned char*)(o + 261) <= 0x3Cu) {
                FUN_0046c5a0(0, (int)v390, (float*)(Owner + 16), (float*)(o + 28));
            }

            // L2234-2237: la tercera componente del ángulo se recalcula a partir de las
            // posiciones del caster y del objetivo. A pesar de la etiqueta vieja del decompilador,
            // esto es CreateAngle/FUN_0043e050 (00449900_MoveCharacter.c), no
            // a movement tick.
            v389[0] = *(float*)(o + 28);
            v389[1] = *(float*)(o + 32);
            v389[2] = *(float*)(o + 36);
            v389[2] = FUN_0043e050(*(float*)(o + 16), *(float*)(o + 20),
                                    *(float*)(Owner + 16), *(float*)(Owner + 20));

            // L2238-2479: targeted-skill effect dispatch
            unsigned char skillId = *(BYTE*)(c + 770);
            switch (skillId)
            {
            case 1:
                if (*(short*)(o + 2) == 390)
                    Effect_Create(192, (float*)(Owner + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                Light[0] = 0.40000001f; Light[1] = 0.60000002f; Light[2] = 1.0f;
                for (int kk = 0; kk < 10; ++kk)
                    Particle_Spawn(1220, (float*)(Owner + 16), (float*)(o + 28), Light, 1, 1.0f, 0);
                if (*(BYTE*)(c + 769)) *(DWORD*)(Owner + 120) |= 1u;
                PlayBuffer(34, 0, 0);
                break;
            case 2:
                Effect_Create(191, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                             (float*)(uintptr_t)-1, nullptr, 0);
                PlayBuffer(46, 0, 0);
                break;
            case 4:
                Effect_Create(191, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)1, (float*)Owner,
                             (float*)(uintptr_t)-1, nullptr, 0);
                PlayBuffer(46, 0, 0);
                break;
            case 7:
                Effect_Create(190, (float*)(Owner + 16), (float*)(o + 28), Light, (float*)(uintptr_t)0, nullptr,
                             (float*)(uintptr_t)-1, nullptr, 0);
                for (int kk = 0; kk < 5; ++kk)
                    Effect_Create(199, (float*)(Owner + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                if (*(BYTE*)(c + 769) && (*(DWORD*)(Owner + 120) & 2) != 2)
                    *(DWORD*)(Owner + 120) |= 2u;
                PlayBuffer(90, 0, 0);
                break;
            case 11:
                if (*(short*)(o + 2) == 288) {
                    v389[2] += 10.0f;
                    Effect_Create(203, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                    v389[2] -= 20.0f;
                    Effect_Create(203, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                    v389[2] += 10.0f;
                }
                Effect_Create(203, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                             (float*)(uintptr_t)-1, nullptr, 0);
                PlayBuffer(88, 0, 0);
                break;
            case 16:
                if (*(short*)(o + 2) == 325) {
                    DWORD v30 = *(DWORD*)(o + 120);
                    *((BYTE*)&v30 + 1) |= 1u;
                    *(DWORD*)(o + 120) = v30;
                } else {
                    DWORD v31 = *(DWORD*)(Owner + 120);
                    *((BYTE*)&v31 + 1) |= 1u;
                    *(DWORD*)(Owner + 120) = v31;
                    PlayBuffer(103, 0, 0);
                    mc_DeleteJoint(266, Owner, 0);
                    for (int i = 0; i < 5; ++i) {
                        Joint_Create(266, (float*)(Owner + 16), (float*)(Owner + 16),
                                     (float*)(Owner + 28), 0, (int)Owner, 20.0f, (short)-1, 0);
                    }
                }
                break;
            case 17: {
                bool skip766 = false;
                switch (*(BYTE*)(c + 747)) {
                case 0x25: case 0x2E: case 0x3D: case 0x42: case 0x45: case 0x46:
                case 0x49: case 0x4B: case 0x4D: case 0x57: case 0x59: case 0x5D:
                case 0x5F: case 0x63: case 0x70: case 0x74: case 0x76: case 0x7A:
                case 0x7C: case 0x80: case 0x82: case 0x86: case 0x88:
                    skip766 = true; break;
                default:
                    if (*(short*)(o + 2) == 282) {
                        Effect_Create(212, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, (float*)Owner,
                                     (float*)(uintptr_t)-1, nullptr, 0);
                    } else {
                        Effect_Create(1180, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, (float*)Owner,
                                     (float*)(uintptr_t)-1, nullptr, 0);
                        PlayBuffer(88, 0, 0);
                    }
                }
                if (skip766) goto LABEL_766;
                break;
            }
            case 24: {
                int hk = FindHotKey_stub(skillId);
                CreateArrows_stub(c, o, 0, (WORD)hk, 1, 0);
                goto LABEL_720;
            }
            case 26:
                Effect_Create(1264, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(Owner + 232), (float*)(uintptr_t)1, (float*)Owner,
                             (float*)(uintptr_t)-1, nullptr, 0);
                break;
            case 27:
                if (*(BYTE*)(c + 769)) {
                    Effect_Create(1264, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(Owner + 232), (float*)(uintptr_t)2, (float*)Owner,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                    if ((*(DWORD*)(Owner + 120) & 8) == 8) {
                        if (!mc_JointFind(266, Owner, 4)) {
                            for (int i = 0; i < 5; ++i) {
                                Joint_Create(266, (float*)(Owner + 16), (float*)(Owner + 16),
                                             (float*)(Owner + 28), 4, (int)Owner, 20.0f, (short)-1, 0);
                            }
                        }
                    } else {
                        DWORD v29 = *(DWORD*)(Owner + 120);
                        *((BYTE*)&v29 + 0) = (BYTE)(v29 | 8);
                        *(DWORD*)(Owner + 120) = v29;
                        for (int i = 0; i < 5; ++i) {
                            Joint_Create(266, (float*)(Owner + 16), (float*)(Owner + 16),
                                         (float*)(Owner + 28), 4, (int)Owner, 20.0f, (short)-1, 0);
                        }
                    }
                }
                break;
            case 28:
                Effect_Create(1264, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(Owner + 232), (float*)(uintptr_t)3, (float*)Owner,
                             (float*)(uintptr_t)-1, nullptr, 0);
                if (*(BYTE*)(c + 769)) *(DWORD*)(Owner + 120) |= 4u;
                break;
            case 51:
            case 52:
            LABEL_720:
                {
                    unsigned char skill2 = *(BYTE*)(c + 770);
                    int hk = FindHotKey_stub(skill2);
                    CreateArrows_stub(c, o, 0, (WORD)hk, 0, (WORD)skill2);
                }
                break;
            default: break;
            }

        LABEL_766:
            // L2430-2479: ambient particle + shield-up sound
            WorldPosition[0] = *(float*)(Owner + 16);
            WorldPosition[1] = *(float*)(Owner + 20);
            WorldPosition[2] = *(float*)(Owner + 24) + 120.0f;
            v392 = 0;
            if (*(BYTE*)(o + 261) == 37 || *(BYTE*)(o + 261) == 38) v392 = 1;
            Light[0] = 1.0f;
            Light[1] = (v390 == (DWORD)Hero) ? 0.0f : 0.60000002f;
            Light[2] = 0.0f;

            if (*(BYTE*)(c + 756) == 2) {
                Joint_Create(1258, (float*)(Owner + 16), (float*)(Owner + 16), (float*)(o + 28),
                             0, (int)o, 20.0f, (short)-1, 0);
                Joint_Create(1258, (float*)(Owner + 16), (float*)(Owner + 16), (float*)(o + 28),
                             1, (int)o, 20.0f, (short)-1, 0);
            }

            // L2455-2478: hit-impact "thud" sound for non-buff skills
            unsigned char queuedSkill = *(BYTE*)(c + 770);
            switch (queuedSkill) {
            case 16: case 26: case 27: case 28:
            case 30: case 31: case 32: case 33: case 34: case 35: case 36:
            case 43: case 47: case 48:
                break;
            default:
                if (*(unsigned char*)(c + 747) < 0x44u || *(unsigned char*)(c + 747) > 0x4Bu) {
                    PlayBuffer(rand() % 7 + 50, (DWORD)o, 0);
                }
                break;
            }
        }

        // L2481-2485: clear queue + state — hash decrypt/encrypt skipped
        *(BYTE*)(c + 770) = 0;
        *(short*)(c + 758) = 0;
        *(BYTE*)(c + 756) = 0;
    }

    // ─── L2487-2507: Skill_Throw / projectile (c+912 = throwing flag)
    if (*(BYTE*)(c + 912)) {
        *(int*)(o + 100) = -2;
        *(float*)(o + 104) += 0.1f;
        float v384f = (*(float*)(c + 788) - *(float*)(o + 16)) * 0.30000001f;
        float v385f = (*(float*)(c + 792) - *(float*)(o + 20)) * 0.30000001f;
        *(float*)(o + 16) += v384f;
        *(float*)(o + 20) += v385f;
        float v33f = FUN_004f7500(*(float*)(o + 16), *(float*)(o + 20));
        *(float*)(o + 24) = v33f;
        if ((float)FUN_005129f0(v384f) < 1.0f) {
            if ((float)FUN_005129f0(v385f) < 1.0f) {
                *(BYTE*)(c + 912) = 0;
                *(int*)(o + 100) = -1;
            }
        }
    }

    // ─── L2508-2540: c+816 chained joint (Sword/spear/wand trail types 1253/1254/1260)
    memset(v417, 0, sizeof(v417));
    if (*(int*)(c + 816) == 1254 || *(int*)(c + 816) == 1260 || *(int*)(c + 816) == 1253)
    {
        int v383i = -1036779520;  // -10.0f
        if (*(int*)(c + 816) == 1253) v383i = -1028390912;  // -8.0f
        // (v380/v381/v382 unused outside debug)
        memset(p1f, 0, sizeof(p1f));
        BMD_TransformPosition((void*)v422, (float*)(*(int*)(o + 276) + 1584), p1f, WorldPosition, 1);
        *(int*)&v417[0] = v383i;
        v417[1] = 0.0f;
        v417[2] = *(float*)(o + 36) - 60.0f;
        Joint_Create(*(int*)(c + 816), WorldPosition, (float*)(c + 788), v417, 0, (int)o, 50.0f, (short)-1, 0);
        if (*(int*)(c + 816) == 1254) {
            Joint_Create(*(int*)(c + 816), WorldPosition, (float*)(c + 788), v417, 0, (int)o, 10.0f, (short)-1, 0);
            Joint_Create(*(int*)(c + 816), WorldPosition, (float*)(c + 788), v417, 0, (int)o, 10.0f, (short)-1, 0);
        }
        BMD_TransformPosition((void*)v422, (float*)(*(int*)(o + 276) + 2016), p1f, WorldPosition, 1);
        *(int*)&v417[0] = v383i;
        v417[1] = 0.0f;
        v417[2] = *(float*)(o + 36) + 60.0f;
        Joint_Create(*(int*)(c + 816), WorldPosition, (float*)(c + 788), v417, 0, (int)o, 50.0f, (short)-1, 0);
        if (*(int*)(c + 816) == 1254) {
            Joint_Create(*(int*)(c + 816), WorldPosition, (float*)(c + 788), v417, 0, (int)o, 10.0f, (short)-1, 0);
            Joint_Create(*(int*)(c + 816), WorldPosition, (float*)(c + 788), v417, 0, (int)o, 10.0f, (short)-1, 0);
        }
    }

    // L2541-2572: Spear stab (1256) — trail at f >= 9.0
    if (*(int*)(c + 816) == 1256) {
        memset(p1f, 0, sizeof(p1f));
        BMD_TransformPosition((void*)v422, (float*)(*(int*)(o + 276) + 1968), p1f, WorldPosition, 0);
        memset(p1f, 0, sizeof(p1f));
        BMD_TransformPosition((void*)v422, (float*)(*(int*)(o + 276) + 1536), p1f, WorldPosition, 0);
        if (*(float*)(o + 264) >= 9.0f) {
            if (*(short*)(c + 624) == 560) {
                WorldPosition[0] = 0.0f; WorldPosition[1] = -130.0f; WorldPosition[2] = 0.0f;
                BMD_TransformPosition((void*)v422,
                             (float*)(48 * (*(BYTE*)(c + 628)) + *(int*)(o + 276)),
                             WorldPosition, p1f, 0);
            } else {
                memset(WorldPosition, 0, sizeof(WorldPosition));
                BMD_TransformPosition((void*)v422, (float*)(*(int*)(o + 276) + 1968), WorldPosition, p1f, 0);
            }
            v379[0] = *(float*)(o + 28);
            v379[1] = *(float*)(o + 32);
            v379[2] = *(float*)(o + 36);
            Joint_Create(*(int*)(c + 816), p1f, (float*)(c + 788), v379, 0, (int)o, 50.0f, (short)-1, 0);
            *(int*)(c + 816) = -1;
        }
    }

    // ─── L2573-2768: Weapon-blur trail (when AnimFrame >= 3.0 or anim=81)
    if (*(float*)(o + 264) >= 3.0f || *(BYTE*)(o + 261) == 81) {
        v377 = 0;
        if (*(short*)(o + 2) == 390 && (*(BYTE*)(o + 261) == 37 || *(BYTE*)(o + 261) == 38)) v377 = 1;
        v375 = 0; Type = 0;
        v378 = *(short*)(c + 24 * v377 + 624);
        v376 = *(unsigned char*)(c + 24 * v377 + 626);

        BYTE actLocal = *(BYTE*)(o + 261);
        if (*(short*)(o + 2) == 390) {
            // Player blur dispatch by weapon ItemType + action
            if (actLocal == 57 || actLocal == 58 || actLocal == 59) {
                v375 = 1;
                Type = (v378 == 414 || v378 == 431) ? 1 : 2;
            } else if (actLocal == 60) {
                v375 = 1;
                Type = (v378 == 469) ? 1 : 2;
            } else if (v378 < 400 || v378 >= 432) {
                if (v378 == 435 || (v378 >= 437 && v378 < 496)) {
                    if (actLocal >= 0x38u && actLocal <= 0x3Cu) {
                        v375 = 1; Type = 2;
                    }
                } else if (v378 >= 496 && v378 < 528 && actLocal >= 0x2Au && actLocal <= 0x2Du) {
                    v375 = 3;
                    if (v378 == 506) { v375 = 1; Type = 0; }
                    else if (actLocal == 45) { Type = 1; }
                }
            } else if ((actLocal >= 0x23u && actLocal <= 0x29u) || actLocal == 81) {
                v375 = 1;
                if (v378 == 417) Type = 6;
                else if (actLocal == 41 || actLocal == 81) Type = 1;
            }
        }
        else if (*(BYTE*)(c + 747) == 71 || *(BYTE*)(c + 747) == 74) {
            if (actLocal >= 3u && actLocal <= 4u) { v375 = 1; Type = 6; }
        }
        else if (v378 >= 400 && v378 < 432 && actLocal >= 3u && actLocal <= 4u) {
            v375 = 1;
        }

        if (v375 > 0) {
            v372[0] = 0.0f;
            v372[1] = (v375 == 1) ? -20.0f : -100.0f;
            v372[2] = 0.0f;
            v371[0] = 0.0f; v371[1] = -120.0f; v371[2] = 0.0f;

            if (v378 == 413 || v378 == 470 || v378 == 505) {
                v373[0] = 1.0f; v373[1] = 0.2f; v373[2] = 0.2f;
            } else if (Type) {
                v373[0] = 1.0f; v373[1] = 1.0f; v373[2] = 1.0f;
            } else if (v376 < 7) {
                if (v376 < 5) {
                    if (v376 < 3) {
                        v373[0] = 0.80000001f; v373[1] = 0.80000001f; v373[2] = 0.80000001f;
                    } else {
                        v373[0] = 1.0f; v373[1] = 0.2f; v373[2] = 0.2f;
                    }
                } else {
                    v373[0] = 0.2f; v373[1] = 0.40000001f; v373[2] = 1.0f;
                }
            } else {
                v373[0] = 1.0f; v373[1] = 0.60000002f; v373[2] = 0.2f;
            }

            if (*(short*)(o + 2) != 390 || v378 == 403 || v378 == 406 || v378 == 409 || v378 == 411 || v378 == 500) {
                BMD_TransformPosition((void*)v422,
                             (float*)(48 * (*(BYTE*)(c + 24 * v377 + 628)) + *(int*)(o + 276)),
                             v372, p1f, 1);
                BMD_TransformPosition((void*)v422,
                             (float*)(48 * (*(BYTE*)(c + 24 * v377 + 628)) + *(int*)(o + 276)),
                             v371, p2f, 1);
                mc_CreateBlur(c, p1f, p2f, v373, Type, 0);
            } else {
                // Weapon-trail interpolation across 10 sub-frames using BMD_Animation
                float v368f = 10.0f;
                float AnimationFrame = *(float*)(o + 264)
                                     - *(float*)(*(int*)(v422 + 48) + 16 * (*(BYTE*)(v422 + 160)) + 4);
                float PriorFrame = *(float*)(o + 268);
                float v370f = *(float*)(*(int*)(v422 + 48) + 16 * (*(BYTE*)(v422 + 160)) + 4) / 10.0f;
                for (int kk = 0; kk < (int)v368f; ++kk) {
                    unsigned int colorPack[3] = { 0, 0, 0 };
                    FUN_00440060((void*)v422, (int)BoneMatrix, AnimationFrame,
                                 *(unsigned int*)&PriorFrame,
                                 *(BYTE*)(o + 262), colorPack,
                                 (float*)(o + 40), 0, 1);
                    BMD_TransformPosition((void*)v422,
                                 (float*)BoneMatrix[3 * (*(BYTE*)(c + 24 * v377 + 628))],
                                 v372, p1f, 0);
                    BMD_TransformPosition((void*)v422,
                                 (float*)BoneMatrix[3 * (*(BYTE*)(c + 24 * v377 + 628))],
                                 v371, p2f, 0);
                    mc_CreateBlur(c, p1f, p2f, v373, Type, 1);
                    AnimationFrame += v370f;
                }
            }
        }
    } else {
        // L2763-2768: cache pos for blur start
        *(DWORD*)(o + 368) = *(DWORD*)(o + 16);
        *(DWORD*)(o + 372) = *(DWORD*)(o + 20);
        *(DWORD*)(o + 376) = *(DWORD*)(o + 24);
    }

    // L2769-2772: pickup-bag splash
    if (*(short*)(o + 2) == 236) {
        FUN_0046c7f0(0, (int)o, 0.0f, 0.0f, 0.0f);
    }
}

// FUN_004520c0 @ 0x004520C0 — Entity_UpdateState(entity_ptr)
// Updates model position/animation from entity. When model has no actions: copies cached
// los parámetros de anim de entity+0x118..+0x12c a +0x130..+0x15c, sumando la posición de mundo de la entidad.
void __cdecl FUN_004520c0(int entity_ptr)
{
    if (!entity_ptr) return;

    // Model slot = DAT_05828d58 + entity_type * 0xbc
    void *model = (void *)(DAT_05828d58 + (int)*(short *)(entity_ptr + 2) * 0xbc);

    // If model has no action data: copy cached transform params to render slots
    if (*(short *)((int)model + 0x26) == 0) {
        *(unsigned int *)(entity_ptr + 0x140) = 0;
        *(unsigned int *)(entity_ptr + 0x130) = *(unsigned int *)(entity_ptr + 0x118);
        *(unsigned int *)(entity_ptr + 0x134) = *(unsigned int *)(entity_ptr + 0x11c);
        *(unsigned int *)(entity_ptr + 0x138) = *(unsigned int *)(entity_ptr + 0x120);
        *(unsigned int *)(entity_ptr + 0x144) = 0;
        *(unsigned int *)(entity_ptr + 0x148) = 0;
        *(unsigned int *)(entity_ptr + 0x150) = 0;
        *(unsigned int *)(entity_ptr + 0x154) = 0;
        *(unsigned int *)(entity_ptr + 0x158) = 0;
        *(float *)(entity_ptr + 0x13c) = *(float *)(entity_ptr + 0x124) - *(float *)(entity_ptr + 0x118);
        *(float *)(entity_ptr + 0x14c) = *(float *)(entity_ptr + 0x128) - *(float *)(entity_ptr + 0x11c);
        *(float *)(entity_ptr + 0x15c) = *(float *)(entity_ptr + 300) - *(float *)(entity_ptr + 0x120);
        *(float *)(entity_ptr + 0x130) = *(float *)(entity_ptr + 0x10) + *(float *)(entity_ptr + 0x130);
        *(float *)(entity_ptr + 0x134) = *(float *)(entity_ptr + 0x14) + *(float *)(entity_ptr + 0x134);
        *(float *)(entity_ptr + 0x138) = *(float *)(entity_ptr + 0x18) + *(float *)(entity_ptr + 0x138);
        return;
    }

    // Con acciones: copia la posición de mundo de la entidad a los slots de transform del modelo
    *(unsigned int *)((int)model + 0x6c) = *(unsigned int *)(entity_ptr + 0x10);
    *(unsigned int *)((int)model + 0x70) = *(unsigned int *)(entity_ptr + 0x14);
    *(unsigned int *)((int)model + 0x74) = *(unsigned int *)(entity_ptr + 0x18);
    *(unsigned int *)((int)model + 0x68) = *(unsigned int *)(entity_ptr + 0x0c);
    *(unsigned char *)((int)model + 0xa0) = *(unsigned char *)(entity_ptr + 0x105);

    // 2026-08-10 — SafeZone update por frame (IDA MoveCharacterVisual L614):
    //     *(BYTE*)(c + 846) = (TerrainWall[Terrain_Load(x, y)] & 1) == 1;
    // +0x34E (846) es **SafeZone**, NO dead_flag (el dead real es +0x2FD, ver
    // IDA ReceiveDie L18). Sin este write el flag quedaba pegado en su valor de
    // spawn: nunca se activaba la música de pueblo, ni el bind del arma a la
    // espalda, ni el gate de "no atacar en zona segura".
    //
    // Hex-Rays perdió los args de Terrain_Load acá (los slots de stack `x`/`yg`
    // los reusó el ruido de hash-table), así que usamos la posición de mundo
    // actual de la entidad → celda, que es lo que hace CreateCharacterPointer
    // en el spawn y la única lectura posible (la celda del personaje).
    {
        int gx = (int)(*(float *)(entity_ptr + 0x10) * 0.01f);
        int gy = (int)(*(float *)(entity_ptr + 0x14) * 0.01f);
        if (gx >= 0 && gx < 256 && gy >= 0 && gy < 256) {
            int attrIdx = FUN_004f6c40(gx, gy);
            *(unsigned char *)(entity_ptr + 0x34e) =
                (unsigned char)((((unsigned char *)DAT_0838bc70)[attrIdx] & 1) == 1);
        }
    }

    // ── switch por tipo de entidad (IDA MoveCharacterVisual L764) ──────────
    // El binario tiene acá un switch enorme con los efectos ambientales de cada
    // NPC/monstruo. Sólo está portado el case del HERRERO; el resto sigue
    // pendiente (cada uno necesita su propia verificación contra IDA).
    //
    // Luminosidad con flicker, común a todo el switch (IDA L761-763):
    //     v63 = (rand() % 8 + 2) * 0.1     → 0.2 .. 0.9
    {
        // IDA L756-763 — estado COMPARTIDO por todos los cases del switch, no
        // local a cada uno. Varios cases usan `Light` y `WorldPosition` sin
        // reinicializarlos, dando por sentados estos valores:
        //     Light[0..2] = 1.0
        //     memset(WorldPosition, 0, sizeof(WorldPosition))
        //     v63 = Scalec = (rand() % 8 + 2) * 0.1
        // (Sin esto no se pueden portar 0x119, 0x12B y varios más: los llaman
        //  a `Particle_Spawn(..., Light, ...)` con el (1,1,1) de acá.)
        // 004520C0 L617-679: estado visual común, previo al switch por tipo.
        // Maneja el seguimiento con la cabeza, su giro suave y el pico temporal en
        // c+848; nada de esto es lógica de combate.
        if (*(unsigned short *)(entity_ptr + 2) != 390) {
            MoveHead_stub(entity_ptr);
        }
        if (entity_ptr != (int)(uintptr_t)DAT_07abf5d8 &&
            !*(unsigned char *)(entity_ptr + 765) && !(rand() % 32)) {
            float *headTarget = (float *)(entity_ptr + 52);
            headTarget[0] = (float)(rand() % 128 - 64);
            headTarget[1] = (float)(rand() % 32 - 16);
            for (int i = 0; i < 2; ++i) {
                if (headTarget[i] < 0.0f) headTarget[i] += 360.0f;
            }
        }
        for (int i = 0; i < 2; ++i) {
            float *headCurrent = (float *)(entity_ptr + 40 + i * sizeof(float));
            const float delta = Angle_GetDifference(*headCurrent, headCurrent[3], 1) * 0.2f;
            *headCurrent = FUN_0043e1b0(*headCurrent, headCurrent[3], delta);
        }
        if (*(unsigned char *)(entity_ptr + 848)) {
            --*(unsigned char *)(entity_ptr + 848);
            float *light = (float *)(entity_ptr + 232);
            for (int i = 0; i < 20; ++i) {
                light[0] = light[1] = light[2] = 1.0f;
                float position[3] = {
                    (float)(rand() % 64 - 32) + *(float *)(entity_ptr + 16),
                    (float)(rand() % 64 - 32) + *(float *)(entity_ptr + 20),
                    (float)(rand() % 32 - 16) + *(float *)(entity_ptr + 24)
                };
                if (!(rand() % 10))
                    Particle_Spawn(1221, position, (float *)(entity_ptr + 28), light, 1, 1.0f, 0);
                if (!(rand() % 10))
                    Effect_Create(rand() % 2 + 197, (float *)(entity_ptr + 16),
                                 (float *)(entity_ptr + 28), light, nullptr, nullptr,
                                 (float *)(uintptr_t)0xffffffff, nullptr, 0);
            }
        }

        extern void __cdecl AddTerrainLight(float x, float y, float* light, int range, float* buffer);
        extern int __cdecl FUN_004793f0(int a1, DWORD *a2, int a3, DWORD *a4, int a5);
        // 004520C0 L697-725: equipped-item terrain light.  The two item
        // los registros arrancan en c+624 y c+648 (stride de 24 bytes).
        if (*(unsigned char *)(entity_ptr + 746) < 6) {
            for (int itemOffset = 624; itemOffset <= 648; itemOffset += 24) {
                const short itemType = *(short *)(entity_ptr + itemOffset);
                if (itemType == 412) {
                    float itemLight[3] = {0.80000001f, 0.64000005f, 0.40000001f};
                    AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20),
                                    itemLight, 3, PrimaryTerrainLight[0]);
                } else if (itemType == 419 || itemType == 546 || itemType == 570) {
                    float itemLight[3] = {0.64000005f, 0.40000001f, 0.24000001f};
                    AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20),
                                    itemLight, 2, PrimaryTerrainLight[0]);
                }
            }
        }
        float Light[3] = { 1.0f, 1.0f, 1.0f };
        float WorldPosition[3] = { 0.0f, 0.0f, 0.0f };
        float Luminosity = (float)(rand() % 8 + 2) * 0.1f;   // v63 / Scalec
        short entType = *(short *)(entity_ptr + 2);
        (void)Light; (void)WorldPosition;

        switch (entType)
        {
        // 0x10E — IDA raw L766-796.
        case 0x10E:
        {
            const unsigned char curAction = *(unsigned char *)(entity_ptr + 261);
            const float animFrame = *(float *)(entity_ptr + 264);
            const bool sparkWindow =
                (!curAction && animFrame >= 15.0f && animFrame <= 20.0f) ||
                (curAction == 1 && animFrame >= 20.0f && animFrame <= 25.0f) ||
                (curAction == 2 && ((animFrame >= 2.0f && animFrame <= 3.0f) ||
                                    (animFrame >= 5.0f && animFrame <= 6.0f)));
            if (sparkWindow && !(rand() & 1)) {
                float origin[3], local[3] = { 0.0f, -4.0f, 0.0f };
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 1152), local, origin, '\x01');
                Particle_Spawn(1220, origin, (float *)(entity_ptr + 28), (float *)(entity_ptr + 232), 0, 1.0f, 0);
            }
            break;
        }

        // 0x110 / 0x111 / 0x114 / 0x122 — IDA raw L797-847.
        case 0x110:
            if (*(unsigned char *)(entity_ptr + 261) == 3 && *(float *)(entity_ptr + 264) <= 4.0f) {
                float p[3], local[3] = {0.0f, (float)(rand() % 32 + 32), 0.0f};
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 336), local, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 1, 1.0f, 0);
            }
            // IDA fall-through to LABEL_253.
        case 0x111:
        case 0x114:
        case 0x122:
            if (entType == 0x122) {
                float p[3];
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 336), WorldPosition, p, '\x01');
                float spriteLight[3] = {Luminosity, Luminosity * 0.40000001f, Luminosity * 0.2f};
                FUN_004795c0(1150, p, 1.0f, spriteLight, entity_ptr, 0.0f, 0);
            }
            if (!*(unsigned char *)(entity_ptr + 765) && !(rand() & 3)) {
                float p[3] = {(float)(rand() % 64 - 32) + *(float *)(entity_ptr + 16),
                              (float)(rand() % 64 - 32) + *(float *)(entity_ptr + 20),
                              (float)(rand() % 32 - 16) + *(float *)(entity_ptr + 24)};
                Particle_Spawn(World == 2 ? 1220 : 1221, p, (float *)(entity_ptr + 28),
                              World == 2 ? (float *)(entity_ptr + 232) : Light, 0, 1.0f, 0);
            }
            break;

        // 0x113 — IDA raw L848-850.
        case 0x113:
            Combat_SpawnDeathDustParticles(entity_ptr);
            break;

        // 0x119 — IDA raw L851-873. Usa el `Light` (1,1,1) del frame compartido.
        // Gate `*(WORD*)(c + 446) == 2`: sólo cuando la entidad está en ese
        // sub-estado escupe las 10 partículas de fuego. El hueso es ALEATORIO
        // (`48 * (rand() % NumBones)`), o sea las partículas salen de todo el
        // cuerpo, no de un punto fijo. `v2 + 34` es NumBones del modelo (+0x22).
        case 0x119:
        {
            *(float *)(entity_ptr + 104) = (float)(rand() % 10) * 0.1f;
            if (*(unsigned short *)(entity_ptr + 446) == 2)
            {
                short numBones = *(short *)((char *)model + 0x22);
                if (numBones > 0)
                {
                    float origin[3];
                    for (int i = 0; i < 10; ++i)
                    {
                        BMD_TransformPosition(model,
                                     (float *)(*(int *)(entity_ptr + 276) + 48 * (rand() % numBones)),
                                     WorldPosition, origin, '\x01');
                        Particle_Spawn(1195, origin, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
                    }
                }
                float L2[3] = { Luminosity, Luminosity * 0.2f, 0.0f };
                AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20),
                                L2, 2, PrimaryTerrainLight[0]);
            }
            break;
        }

        // 0x11A / 0x11B — IDA raw L874-891.
        case 0x11A:
        case 0x11B:
            if (!(rand() & 3)) {
                float local[3] = { 0.0f, 0.0f, 0.0f }, origin[3];
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 1056), local, origin, '\x01');
                Particle_Spawn(1220, origin, (float *)(entity_ptr + 28), (float *)(entity_ptr + 232), 0, 1.0f, 0);
            }
            break;

        // 0x11D / 0x128 / 0x129 — IDA raw L892-900.
        case 0x11D: *(float *)(entity_ptr + 112) = (float)((long long)WorldTime % 2000) * -0.00050000002f; break;
        case 0x128: *(float *)(entity_ptr + 108) = (float)((long long)WorldTime % 10000) * -0.000099999997f; break;
        case 0x129: *(float *)(entity_ptr + 112) = (float)((long long)WorldTime % 1000) * -0.001f; break;

        // 0x12B — IDA raw L901-915.
        case 0x12B:
            *(int *)(entity_ptr + 100) = 3;
            *(float *)(entity_ptr + 112) = (float)((long long)WorldTime % 1000) * -0.001f;
            if (!(rand() & 1)) {
                float p[3];
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 96), WorldPosition, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
            }
            break;

        // 0x12E — IDA raw L916-971.
        case 0x12E:
        {
            float local[3] = { 0.0f, 0.0f, 0.0f }, origin[3];
            float light[3] = { 0.60000002f, 1.0f, 0.80000001f };
            const unsigned char action = *(unsigned char *)(entity_ptr + 261);
            int bone = action == 3 ? 33 : action == 4 ? 20 : action == 8 ? 41 : action == 9 ? 49 : -1;
            if (bone >= 0) {
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * bone), local, origin, '\x01');
                Particle_Spawn(1180, origin, (float *)(entity_ptr + 28), light, 0, 1.0f, 0);
                float whitePink[3] = { 1.0f, 0.60000002f, 1.0f };
                Particle_Spawn(1195, origin, (float *)(entity_ptr + 28), whitePink, 0, 1.0f, 0);
            }
            if (action == 6 && *(float *)(entity_ptr + 264) < 12.0f) {
                float green[3] = { 0.1f, 0.80000001f, 0.60000002f };
                for (int i = 0; i < 20; ++i) {
                    const int randomBone = rand() % *(short *)((int)model + 34);
                    BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * randomBone), local, origin, '\x01');
                    Particle_Spawn(1195, origin, (float *)(entity_ptr + 28), green, 0, 1.0f, 0);
                }
            }
            break;
        }

        // 0x12F — IDA raw L972-990.
        case 0x12F:
            if (!*(unsigned char *)(entity_ptr + 765) && *(unsigned short *)(entity_ptr + 446) == 1 && !(rand() & 3)) {
                float p[3] = {(float)(rand() % 64 - 32) + *(float *)(entity_ptr + 16),
                              (float)(rand() % 64 - 32) + *(float *)(entity_ptr + 20),
                              (float)(rand() % 32 - 16) + *(float *)(entity_ptr + 24)};
                Particle_Spawn(1221, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
            }
            break;

        // 0x133 — IDA raw L991-1007.
        case 0x133:
            if (*(unsigned char *)(entity_ptr + 261) >= 3 && *(unsigned char *)(entity_ptr + 261) <= 4)
                *(float *)(entity_ptr + 104) = (*(float *)(entity_ptr + 104) + 0.1f > 1.0f) ? 1.0f : *(float *)(entity_ptr + 104) + 0.1f;
            else
                *(float *)(entity_ptr + 104) = (*(float *)(entity_ptr + 104) - 0.1f < 0.0f) ? 0.0f : *(float *)(entity_ptr + 104) - 0.1f;
            break;

        // 0x135 — IDA raw L1008-1010: sub_451EA0(o,v2,28,27).
        case 0x135:
        {
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 28), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 27), zero, (float *)(entity_ptr + 64), '\x01');
            break;
        }
        // 0x137 — IDA raw L1011-1015.
        case 0x137:
        {
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 8), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 9), zero, (float *)(entity_ptr + 64), '\x01');
            Combat_SpawnIdleAmbientParticle(entity_ptr); Combat_SpawnDeathDustParticles(entity_ptr);
            break;
        }
        // 0x138 — IDA raw L1016-1034.
        case 0x138:
        {
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 24), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 25), zero, (float *)(entity_ptr + 64), '\x01');
            if (*(int *)(entity_ptr + 4) == 1) {
                float p[3];
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 288), WorldPosition, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 624), WorldPosition, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
                float darkness[3] = {-1.3f,-1.3f,-1.3f};
                AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20), darkness, 3, PrimaryTerrainLight[0]);
            } else { Combat_SpawnIdleAmbientParticle(entity_ptr); Combat_SpawnDeathDustParticles(entity_ptr); }
            break;
        }
        // 0x139 / 0x13B — IDA raw L1035-1038 / L1114-1117.
        case 0x139:
        case 0x13B:
        {
            const int a = entType == 0x139 ? 11 : 8, b = entType == 0x139 ? 12 : 9;
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * a), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * b), zero, (float *)(entity_ptr + 64), '\x01');
            Combat_SpawnIdleAmbientParticle(entity_ptr);
            break;
        }
        // 0x13A — IDA raw L1039-1113.  Las tablas se leyeron de IDA en
        // 0x5597D8 (118 bytes: las últimas lecturas alcanzan 0x55984B).
        // Hex-Rays separa los aliases v169/v170/v171,
        // pero están en el mismo bloque contiguo: bones[0], bones[1], bones[30].
        case 0x13A:
        {
            static const unsigned char boneOrder[35] = {
                5, 6, 33, 53, 35, 49, 50, 45, 46, 41, 42, 37, 38, 11, 31, 13,
                27, 28, 23, 24, 19, 20, 15, 16, 54, 55, 62, 69, 70, 77, 2, 79,
                81, 84, 86
            };
            static const unsigned char linksA[32] = {
                0, 2, 2, 3, 2, 4, 4, 5, 5, 6, 4, 7, 7, 8, 4, 9,
                9, 10, 4, 11, 11, 12, 6, 5, 8, 7, 10, 9, 12, 11, 0, 0
            };
            static const unsigned char linksB[32] = {
                0, 13, 13, 14, 13, 15, 15, 16, 16, 17, 17, 16, 15, 18, 18, 19,
                15, 20, 20, 21, 15, 22, 22, 23, 15, 20, 20, 21, 18, 19, 21, 22
            };
            static const unsigned char linksC[8] = {29, 28, 28, 27, 34, 33, 33, 30};
            static const unsigned char linksD[8] = {26, 25, 25, 24, 32, 31, 31, 30};

            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 8), zero,
                          (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * 9), zero,
                          (float *)(entity_ptr + 64), '\x01');

            if (*(unsigned char *)(entity_ptr + 747) == 63) {
                float targetPosition[3] = {0.0f, 0.0f, 0.0f};
                float bones[35][3] = {};

                for (int bone = 0; bone < 35; ++bone) {
                    BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * boneOrder[bone]),
                                  zero, bones[bone], '\x01');
                }

                for (int link = 0; link < 30; link += 2) {
                    float particleLight[3];
                    FUN_0043e4a0(bones[linksA[link + 1]], targetPosition, bones[linksA[link]], 360.0f);
                    particleLight[0] = Luminosity; // IDA sólo escribe este componente (Position[0] = v63).
                    Particle_Spawn(1200, bones[linksA[link]], targetPosition, particleLight, 2,
                                  link < 22 ? 1.0f : 0.5f, 0);
                    FUN_0043e4a0(bones[linksB[link + 1]], targetPosition, bones[linksB[link]], 360.0f);
                    particleLight[0] = Luminosity;
                    Particle_Spawn(1200, bones[linksB[link]], targetPosition, particleLight, 2,
                                  link < 22 ? 1.0f : 0.5f, 0);
                }

                for (int link = 0; link < 8; link += 2) {
                    float particleLight[3];
                    FUN_0043e4a0(bones[linksC[link + 1]], targetPosition, bones[linksC[link]], 360.0f);
                    particleLight[0] = Luminosity;
                    Particle_Spawn(1200, bones[linksC[link]], targetPosition, particleLight, 2, 0.60000002f, 0);
                    FUN_0043e4a0(bones[linksD[link + 1]], targetPosition, bones[linksD[link]], 360.0f);
                    particleLight[0] = Luminosity;
                    Particle_Spawn(1200, bones[linksD[link]], targetPosition, particleLight, 2, 0.60000002f, 0);
                }

                if (!((long long)WorldTime % 2)) {
                    float particleLight[3];
                    FUN_0043e4a0(bones[0], targetPosition, bones[30], 360.0f);
                    particleLight[0] = (float)WorldTime;
                    Particle_Spawn(1200, bones[30], targetPosition, particleLight, 2, 1.3f, 0);
                    Particle_Spawn(1200, bones[1], targetPosition, particleLight, 3, 0.5f, 0);
                }

                float darkness[3] = {-1.3f, -1.3f, -1.3f};
                AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20), darkness, 3,
                                PrimaryTerrainLight[0]);
            } else {
                float targetPosition[3] = {0.0f, 0.0f, 0.0f};
                float effectLight = sinf((float)WorldTime * 0.0020000001f) * 0.30000001f + 0.69999999f;
                float light[3] = {effectLight, effectLight * 0.5f, effectLight * 0.5f};
                float source[3], destination[3];

                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 2640), WorldPosition, source, '\x01');
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 2976), WorldPosition, destination, '\x01');
                FUN_0043e4a0(source, targetPosition, destination, 360.0f);
                Particle_Spawn(1200, destination, targetPosition, light, 1, 0.2f, 0);

                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 3360), WorldPosition, source, '\x01');
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 3696), WorldPosition, destination, '\x01');
                FUN_0043e4a0(source, targetPosition, destination, 360.0f);
                Particle_Spawn(1200, destination, targetPosition, light, 1, 0.2f, 0);
                Combat_SpawnIdleAmbientParticle(entity_ptr);
                Combat_SpawnDeathDustParticles(entity_ptr);
            }
            break;
        }
        // 0x13E — IDA raw L1118-1128; sub_4793F0 == FUN_004793f0 (verificado en IDA).
        case 0x13E:
            if (!(rand() % 5)) {
                float p[3] = {(float)(rand() % 21 - 10) * 3.6571429f + *(float *)(entity_ptr + 16),
                              (float)(rand() % 21 - 10) * 3.6571429f + *(float *)(entity_ptr + 20), 0.0f};
                FUN_004793f0(1205, (DWORD *)p, *(int *)(entity_ptr + 28), (DWORD *)(entity_ptr + 232), 0x3F266666);
            }
            break;

        // 0x141 — IDA raw L1129-1178.
        case 0x141:
        {
            float local[3] = { 0.0f, 0.0f, 0.0f }, a[3], b[3];
            const int boneBase = *(int *)(entity_ptr + 276);
            for (int offset = 96; offset < 240; offset += 48) {
                BMD_TransformPosition(model, (float *)(boneBase + offset), local, a, '\x01');
                BMD_TransformPosition(model, (float *)(boneBase + offset + 48), local, b, '\x01');
                Joint_Create(1254, a, b, (float *)(entity_ptr + 28), 7, 0, 14.0f, -1, 0);
            }
            const int pairs[][2] = {{2,9},{10,11},{9,18},{18,22},{22,23},{23,24},{24,25},{18,31},{31,32},{32,33},{33,34}};
            for (int i = 0; i < 11; ++i) {
                BMD_TransformPosition(model, (float *)(boneBase + 48 * pairs[i][0]), local, a, '\x01');
                BMD_TransformPosition(model, (float *)(boneBase + 48 * pairs[i][1]), local, b, '\x01');
                Joint_Create(1254, a, b, (float *)(entity_ptr + 28), 7, 0, 14.0f, -1, 0);
            }
            break;
        }

        // 0x142 / 0x145 — IDA raw L1179-1184.
        case 0x142: *(float *)(entity_ptr + 108) = (float)((long long)WorldTime % 10000) * -0.00039999999f; break;
        case 0x145: *(float *)(entity_ptr + 112) = (float)((long long)WorldTime % 10000) * 0.000099999997f; break;

        // 0x152 — MODEL_SMITH / herrero (OpenNpc case 338), IDA raw L1185-1221.
        case 0x152:
        {

            unsigned char curAction = *(unsigned char *)(entity_ptr + 261);  // +0x105
            float         animFrame = *(float *)(entity_ptr + 264);          // +0x108

            // Golpe de martillo: suena entre los frames 5 y 10 de la anim 0.
            // El buffer 120 (0x78) lo carga OpenNpc justo para este NPC.
            if (curAction == 0 && animFrame >= 5.0f && animFrame <= 10.0f)
                PlayBuffer(120, 0, 0);

            // BlendMesh = 4 → la mesh 4 de Smith01.bmd es `fire_02.jpg`, el
            // fuego de la fragua: se dibuja en modo blend en vez de opaca.
            // BlendMeshLight le da el parpadeo.
            *(int   *)(entity_ptr + 100) = 4;            // +0x64 BlendMesh
            *(float *)(entity_ptr + 104) = Luminosity;   // +0x68 BlendMeshLight

            // Luz naranja proyectada sobre el terreno alrededor de la fragua.
            float Light[3] = { Luminosity, Luminosity * 0.40000001f, 0.0f };
            AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20),
                            Light, 3, PrimaryTerrainLight[0]);

            // Chispas: 4 por golpe, en la ventana de frames 5..6 (el impacto).
            // El origen es el hueso 17 (BoneTransform + 48*17 = +816) = el yunque.
            Light[0] = 1.0f; Light[1] = 1.0f; Light[2] = 1.0f;
            float WorldPosition[3] = { 0.0f, 0.0f, 0.0f };
            if (curAction == 0 && animFrame >= 5.0f && animFrame <= 6.0f)
            {
                float sparkOrigin[3];
                BMD_TransformPosition(model,
                             (float *)(*(int *)(entity_ptr + 276) + 816),
                             WorldPosition, sparkOrigin, '\x01');
                for (int i = 0; i < 4; ++i)
                {
                    float Angle[3];
                    Angle[0] = (float)(rand() % 60 + 150);
                    Angle[1] = 0.0f;
                    Angle[2] = (float)(rand() % 30);
                    Joint_Create(1257, sparkOrigin, sparkOrigin, Angle, 0, 0, 10.0f, -1, 0);
                    Particle_Spawn(1175, sparkOrigin, Angle, Light, 0, 1.0f, 0);
                }
            }
            break;
        }

        // 0x157 / 0x15C — IDA raw L1222-1245.
        case 0x157:
            if (!(rand() & 0xFF)) PlayBuffer(120, 0, 0);
            break;
        case 0x15C:
            if (!(rand() & 0x3F)) PlayBuffer(121, 0, 0);
            break;

        // 0x179 — IDA raw L1246-1281.
        case 0x179:
            if (!*(unsigned char *)(entity_ptr + 261)) {
                float white[3] = { 1.0f, 1.0f, 1.0f };
                float local[3] = { 0.0f, 5.0f, 10.0f }, origin[3];
                float terrainLight[3] = { Luminosity * 0.5f, Luminosity * 0.30000001f, 0.0f };
                AddTerrainLight(*(float *)(entity_ptr + 16), *(float *)(entity_ptr + 20), terrainLight, 3, PrimaryTerrainLight[0]);
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 1776), local, origin, '\x01');
                for (int i = 0; i < 4; ++i) {
                    float angle[3] = { (float)(rand() % 60 + 90), 0.0f, (float)(rand() % 30) };
                    Joint_Create(1257, origin, origin, angle, 0, 0, 10.0f, -1, 0);
                    if (rand() & 1)
                        Particle_Spawn(1175, origin, angle, white, 0, 1.0f, 0);
                }
            }
            break;

        // 0x186 — IDA raw L1282-1342.
        case 0x186:
        {
            float p[3];
            if (g_GameState == 5 && World == 7 && (long long)WorldTime % 10000 < 1000) {
                float local[3] = {0.0f,20.0f,-10.0f};
                BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * *(int *)((int)model + 84)), local, p, '\x01');
                Particle_Spawn(1241, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
            }
            float local[3] = {-15.0f,0.0f,0.0f};
            if (World == 9) {
                if (!(rand() & 3)) { BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 1248), local, p, '\x01'); Particle_Spawn(104,p,(float *)(entity_ptr+28),Light,0,1.0f,0); }
                if (!(rand() & 3)) { BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 1680), local, p, '\x01'); Particle_Spawn(104,p,(float *)(entity_ptr+28),Light,0,1.0f,0); }
            }
            if (*(unsigned char *)(entity_ptr + 261) == 90) {
                const short n = *(short *)((int)model + 34);
                if (n > 0) for (int i = 0; i < 10; ++i) {
                    BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * (rand() % n)), local, p, '\x01');
                    Particle_Spawn(1195,p,(float *)(entity_ptr+28),Light,0,1.0f,0);
                }
            }
            float tail[3] = {0.0f,-30.0f,0.0f};
            BMD_TransformPosition(model, (float *)(*(int *)(entity_ptr + 276) + 48 * *(unsigned char *)(entity_ptr + 628)), tail, p, '\x01');
            break;
        }

        default:
            break;
        }
    }
    // 004520C0 L1383-1396: los pasos del jugador local se eligen según las dos
    // animation-frame thresholds, once per walk/run cycle.
    if (*(unsigned short *)(entity_ptr + 2) == 390 &&
        entity_ptr == (int)(uintptr_t)DAT_07abf5d8) {
        const unsigned char action = *(unsigned char *)(entity_ptr + 261);
        if ((action >= 13 && action <= 33) || action == 79 || action == 80) {
            const float frame = *(float *)(entity_ptr + 264);
            extern char FUN_00451a90();
            if (frame >= 1.5f && !*(unsigned char *)(entity_ptr + 844)) {
                *(unsigned char *)(entity_ptr + 844) = 1;
                FUN_00451a90();
            }
            if (frame >= 4.5f && !*(unsigned char *)(entity_ptr + 845)) {
                *(unsigned char *)(entity_ptr + 845) = 1;
                FUN_00451a90();
            }
        }
    }
    // (Full animation bone update: HashTable obfuscation blocks — skipped)
}

// FUN_00454cd0 @ 0x00454CD0 — Entity_PathTick(entity, player_entity)
// Tick de camino por frame: si +0x2fd (teleport) está seteado → saltea. Si +0x2ec (flag de movimiento) está seteado →
//   avanza el waypoint (Entity_AdvancePath), y al llegar limpia +0x2ec y cancela la acción.
// Si no se está moviendo y la grilla objetivo difiere del cached_wp → llama a PathFinding2.
// param_1 es el puntero a la entidad (Ghidra lo tipó float — castear a int).
void __cdecl FUN_00454cd0(int param_1_i, int param_2)
{
    // Ghidra tipó param_1 como float pero es un puntero a entidad (dirección int)
    int entity = param_1_i;

    // Sale si esto es el centinela de la entidad del jugador
    if (entity == (int)(void*)DAT_07abf5d8) return;

    // 00454CD0: el flag de teleport/altura tiene una única rama de actualización visual
    // antes de suprimir el walker normal de camino. El tipo 272 se mantiene sobre
    // la superficie del terreno mientras ese flag está activo.
    if (*(char*)(entity + 0x2fd) != '\0') {
        if (*(short*)(param_2 + 2) == 272) {
            *(float*)(param_2 + 24) = FUN_004f7500(
                *(float*)(param_2 + 16), *(float*)(param_2 + 20));
        }
        return;
    }

    if (*(char*)(entity + 0x2ec) != '\0') {
        // Entity is currently moving — advance one step
        FUN_00443930(entity);
        unsigned int arrived = Entity_AdvancePath((void*)entity, '\x01');
        if ((char)arrived != '\0') {
            // Arrived at waypoint — stop
            *(unsigned char*)(entity + 0x2ec) = 0;
            FUN_004430c0(entity);
            // Actualiza el facing según el byte de dirección del camino en +0x2fc
            *(float*)(entity + 0x24) =
                ((float)*(unsigned char*)(entity + 0x2fc) - _DAT_0055256c) * _DAT_00552844;
        }
        FUN_00454ba0(entity);
        return;
    }

    // No se está moviendo: chequea si hay que pathfindear para llegar a target_grid
    if (*(char*)(entity + 0x350) != '\0') return;
    if (*(short*)(param_2 + 2) == 0x115) return;

    // Lee cached_wp_x y cached_wp_y (las decoraciones de HashTable son ruido anti-tamper)
    unsigned int cachedX = *(unsigned int*)(entity + 0x388);
    unsigned int cachedY = *(unsigned int*)(entity + 0x38c);

    // Si ya está en el objetivo, no hay nada que hacer
    if (cachedX == *(unsigned char*)(entity + 0x306) &&
        cachedY == *(unsigned char*)(entity + 0x307))
        return;

    // Pathfind desde cached_wp hasta target_grid
    unsigned int found = Path_FindRoute(cachedX, (float)cachedY,
                                      (unsigned int)*(unsigned char*)(entity + 0x306),
                                      (unsigned int)*(unsigned char*)(entity + 0x307),
                                      (unsigned char*)(entity + 0x354), 0.0f);
    if ((char)found != '\0') {
        *(unsigned char*)(entity + 0x2ec) = 1;
    }
}

// UI_OpenWindow alias (UI_AddNotice)
void __cdecl UI_OpenWindow(char* title, int mode) {
    UI_AddNotice(title, (unsigned char)mode);
}

// FUN_004f8eb0 @ 0x004f8eb0 — CreateFrustrum2D
// CORRECCIÓN 2026-05-04: la decompilación previa de Ghidra confundió los
// nombres. FUN_004cb520 NO es un frame counter — es GetScreenWidth(). Los
// constants `_DAT_00552cbc=1190.0` y `_DAT_00552cb8=540.0` son las half-widths
// de la frustum quad (en view-space units), NO velocidades de rotación.
// _DAT_0055283c = 1/640 (= screen pixel→aspect ratio).
//
// Construye un quad 2D top-view del frustum proyectado: rota por Z=45° las 4
// corners hardcoded, las traslada por param_1 (cam pos), y las escribe a
// DAT_07eeb228/218 (X/Y respectivamente) en tile coords (×0.01 = ÷100).
//
// Esta versión SÍ funciona para in-game. La duplicada Camera_SetMatrix en
// src/Render/Camera.cpp es código muerto que opera sobre DAT_07eab1bc..1e8
// (corners ya transformadas por Camera_SetupFrustum a world coords), lo cual
// duplicaría la transformación si se invocara — no se llama desde ningún
// lado y no debe wirearse.
void __cdecl FUN_004f8eb0(float *param_1)
{
    float pts[15];   // euler[0..2], then 4×vec3 input offsets [3..14]
    float rot[12];   // 3×4 rotation matrix
    float out[12];   // 4 transformed output positions

    int iVar2 = FUN_004cb520();   // GetScreenWidth (= 640 normalmente)
    float angle_cbc = (float)iVar2 * _DAT_0055283c * _DAT_00552cbc;  // sw/640 * 1190 = far half-width
    float angle_cb8 = (float)iVar2 * _DAT_0055283c * _DAT_00552cb8;  // sw/640 * 540  = near half-width

    // Euler angles for AngleMatrix (X=0, Y=0, Z=45°) — top-down orientation
    pts[0] = 0.0f; pts[1] = 0.0f; pts[2] = 45.0f;

    // 4 view-space corner offsets (X stride = far/near width, Y stride = depth)
    pts[3]  = -angle_cbc; pts[4]  =  1272.0f; pts[5]  = 0.0f;  // top-left
    pts[6]  =  angle_cbc; pts[7]  =  1272.0f; pts[8]  = 0.0f;  // top-right
    pts[9]  =  angle_cb8; pts[10] =  -672.0f; pts[11] = 0.0f;  // bot-right
    pts[12] = -angle_cb8; pts[13] =  -672.0f; pts[14] = 0.0f;  // bot-left

    Matrix_BuildFromEuler(pts, rot);   // AngleMatrix (Euler→3x4)

    for (int i = 0, j = 0; i < 0x30; j += 4, i += 0xc) {
        float *pfVar1 = (float*)((char*)out + i);
        Vector_Rotate((float*)((char*)pts + i + 0xc), rot, pfVar1);  // VectorRotate
        pfVar1[0] += param_1[0];
        pfVar1[1] += param_1[1];
        pfVar1[2] += param_1[2];
        *(float*)((char*)&DAT_07eeb228 + j) = pfVar1[0] * _DAT_005524f8;  // X (tile coords)
        *(float*)((char*)&DAT_07eeb218 + j) = pfVar1[1] * _DAT_005524f8;  // Y
    }
}
