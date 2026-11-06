// UI_PanelInput.cpp
// Hit-tests y entradas de los paneles por mouse: creación de guild, click en
// miembros de party, PIN del baúl, trade, cambio de server y grillas.
#include <cstdint>
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

// Inventory_DropDispatch @ 0x004DF410 — dispatcher de drop sobre las 4
// inventories abiertas + sell-to-shop + drop-on-ground. Vive en
// `Item/Item_ClickHandler.cpp`.
// Sub-handlers de SecondPassword: funciones de render/tick por frame del
// subsistema del diálogo de 2da contraseña, implementadas a continuación.
static bool GetGuildCreatorOrigin(int& originX, int& originY)
{
    // El scratch del GuildCreator tiene globals propios (no vive en
    // Inventory[32], que es el slot 0 del pool de la tienda del original).
    originX = g_GuildCreatorScratchX;
    originY = g_GuildCreatorScratchY;
    return (originX != 0 || originY != 0);
}

// SecondPassword_Screen1 @ 0x004E4760 — SecondPassword_Screen1 (727 lines)
//   - Chequea DAT_07eaa124 (hover habilitado). Si DAT_07eaa144==1: grilla de click para 8 botones
//     (8×0x0F tiles con origen en DAT_07ea5b1c/20+100), escribe el array DAT_07ea51f5 con DAT_07eaa0dc.
//   - Segunda pasada: itera los pasos de DAT_07ea5b18 (+0x32 cada uno), los compara con la posición del mouse para encontrar
//     hovered button index, sets DAT_07ea51ed.
//   - SEH + llamadas a UI_SetScene(0x19/0x1c).
void __cdecl SecondPassword_Screen1(void) {
    // El sub_4E4760 de IDA es el hit-test del diálogo GuildCreator (gatea con
    // GuildCreatorOpened, usa Inventory[32].Level/Part como origen del panel).
    // Acá el gate es DAT_07eaa124 (= GuildCreatorOpened) y el origen sale de
    // GetGuildCreatorOrigin (globals propios); sin origen no se hace el
    // hit-test, para que no disparen clicks fantasma del lado izquierdo
    // (x=0..190) con su sonido.
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
        // IDA: SecondPassword_Screen1. La grilla de edición tiene 8 filas de 8 celdas;
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

        // IDA: SecondPassword_Screen1 — paleta 2×8, índice lineal fila*8+col.
        for (int row = 0; row < 2; ++row) {
            const int colorY = originY + 260 + row * 20;
            for (int col = 0; col < 8; ++col) {
                const int colorX = originX + 15 + col * 20;
                if ((int)DAT_083a427c >= colorX && (int)DAT_083a427c < colorX + 20 &&
                    (int)DAT_083a4278 >= colorY && (int)DAT_083a4278 < colorY + 20 &&
                    IsClickPushed()) {
                    DAT_083a4124 = '\0';
                    PlayBuffer(0x19, 0, 0);
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
            // Este boton — el izquierdo, rect [+20,+90) x [+350,+371) — es el de CREAR:
            // IDA `sub_4E4760` L210 manda aca el **0x55** (PMSG_GUILD_CREATE_RECV); el
            // 0x54 es el del boton derecho.
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
            // La rama `== 0` de este gate es la del dialogo previo (manda su 0x54).
            if (DAT_07eaa144) {
                if (Chat_ValidateInputCommand()) {
                    // nombre invalido / vacio
                    SetErrorMessage(115);
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
                        SetErrorMessage(127);
                        PlayBuffer(25, 0, 0);
                    } else {
                        // IDA: strcpy(&dword_7EA51EC, InputText[0])
                        memset(DAT_07ea51ec, 0, sizeof(DAT_07ea51ec));
                        TextEngine::CopyTruncated(DAT_07ea51ec, sizeof(DAT_07ea51ec), (const char*)DAT_07db8710[0]);
            
                        // El rango de guild entero es Encrypt=0 -> C1 plano, y
                        // `gNetwork.SendC1` aplica el mismo chain-XOR sobre 3..len-1
                        // que hace el original.
                        // PMSG_GUILD_CREATE_RECV (0.97.40): GuildName[32], Mark[32].
                        const int NAME = (int)NetText::GuildNameSize;
                        BYTE pkt[3 + NetText::GuildNameSize + 32];
                        memset(pkt, 0, sizeof(pkt));
                        pkt[0] = 0xC1;
                        pkt[1] = (BYTE)sizeof(pkt);
                        pkt[2] = 0x55;
                        NetText::ToServer(pkt + 3, NAME, DAT_07ea51ec);
                        memcpy(pkt + 3 + NAME, mark, 32);
                        gNetwork.SendC1(pkt, (int)sizeof(pkt));

                        // IDA: SecondPassword_Screen1 cae al cierre común inmediatamente
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
                    gNetwork.SendC1(pkt, sizeof(pkt));
                }
            }
            PlayBuffer(0x19, 0, 0);
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
                gNetwork.SendC1(pkt2, sizeof(pkt2));
            } else {
                // IDA: C1:03:57 cancela el editor y libera INTERFACE_GUILD_CREATE.
                const BYTE pkt3[3] = { 0xC1, 0x03, 0x57 };
                gNetwork.SendC1(pkt3, sizeof(pkt3));
                *(short*)&((CHARACTER*)(uintptr_t)(DAT_07abf5d8))->GuildMarkIndex = (short)0xffff;
            }
            PlayBuffer(0x19, 0, 0);
            // DAT_07db8710 es char[10][256] — escribimos el primer byte del slot 0
            // explícitamente. (Los globals 8714/8718 son símbolos separados: también se
            // limpian, para terminar en NUL lo que haya quedado del username.)
            DAT_07db8710[0][0] = 0;
            DAT_07db8714 = 0;
            DAT_07db8718 = 0;
            *(DWORD*)DAT_07d780a8 = 0;
            InputTextMax = 10;
            GuildInputEnable = 0;
            DAT_07eaa124 = '\0';
            DAT_07eaa144 = 0;
            PlayBuffer(0x1c, 0, 0);
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }

LAB_FUN_004e4760_end:
    // DAT_07eaa114 NPC-window close button
    if (DAT_07eaa114 != '\0') {
        // vtable[+0x14] = slot 5 = ChatLB_tick, __fastcall (this en ECX +
        // argumento). Despacharlo como cdecl dejaría ECX con basura y ChatLB_tick
        // leería memoria random como `self[3]` (contador de list1).
        if (DAT_055c9ff4 && ChatWidget_GetVTable(DAT_055c9ff4)) {
            DWORD* obj = (DWORD*)DAT_055c9ff4;
            void** vt  = ChatWidget_GetVTable(obj);
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
            PlayBuffer(0x19, 0, 0);
            PlayBuffer(0x1c, 0, 0);
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }
}
// Party_MemberClickHandler @ 0x004E5500 — Party panel input.
// The Party row action is gated by leader/self name matching and emits the
// server-owned delete request.  The close button only closes the panel.
void __cdecl Party_MemberClickHandler(void) {
    if (DAT_07eaa115 == '\0' || DAT_07ea5b24 == 0) return;

    if ((int)DAT_083a427c >= (int)DAT_07ea5b24 &&
        (int)DAT_083a427c <  (int)(DAT_07ea5b24 + 0xbe) &&
        (int)DAT_083a4278 >= (int)DAT_07ea5b28 &&
        (int)DAT_083a4278 <  (int)(DAT_07ea5b28 + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    const char* const heroName = DAT_07abf5d8 ? EntityName::Get(DAT_07abf5d8) : "";
    const bool heroLeads = PartyNumber > 0 && strcmp(Party_GetName(0), heroName) == 0;
    for (int row = 0; row < PartyNumber; ++row) {
        BYTE* const member = Party + row * 36;
        const bool mayRemove = heroLeads || strcmp(Party_GetName(row), heroName) == 0;
        const int rowY = (int)DAT_07ea5b28 + 55 + row * 35;
        if (mayRemove && (int)DAT_083a427c >= (int)DAT_07ea5b24 + 140 &&
            (int)DAT_083a427c < (int)DAT_07ea5b24 + 164 &&
            (int)DAT_083a4278 >= rowY && (int)DAT_083a4278 < rowY + 24 &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            PlayBuffer(0x19, 0, 0);
            // The original click gate uses the visual row.  MuEmu validates
            // the wire party number, which is explicitly supplied by 0x42.
            const BYTE pkt[4] = { 0xC1, 0x04, 0x43, member[11] };
            gNetwork.SendC1(pkt, sizeof(pkt));
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
        PlayBuffer(0x19, 0, 0);
        PlayBuffer(0x1c, 0, 0);
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    }
}
// SecondPassword_Screen3 @ 0x004E5DE0 — SecondPassword_Screen3 (443 lines)
// Envío del paquete de selección de personaje: por cada uno de los hasta 0x10 botones de personaje de la columna,
// chequea si el mouse clickeó en el área de la fila [DAT_07ea982c+0x7d,+0x95) x [DAT_07ea9830+0x73+row*15, +0x18).
// Guard de entrada: DAT_07eaa116 tiene que ser distinto de cero. También chequea *(short*)(DAT_07cf1ff4+0x54) != 0.
// On click: sends 4-byte packet {0xC1,0x01,0x00,opcode_xored}.
//   opcode plain byte = (byte)(row_index), XOR CNetwork::XorKey[3]=0x89, prev_plain_at[4]=4 → cipher = row^0x89^4
//   Después appendea el byte del índice de fila como byte 4 (si total_len+1 < 0x401).
//   El largo del paquete depende de los datos de cada fila. Clave: la misma de 32 bytes.
// After click: PlayBuffer(0x19,0,0).
// HashTable ref-count noise around DAT_07cf1ffc is anti-tamper, skipped.
// Back button [DAT_07ea982c+0x19,+0x31) x [DAT_07ea9830+0x18b,+0x1a3):
//   → clear DAT_07eaa116, PlayBuffer(0x19/0x1c), DAT_07e11d28=0, DAT_00559bec=6.
void __cdecl SecondPassword_Screen3(void) {
    // IDA sub_4E5DE0 = hit-test del panel Character, con CharacterInfoStartX/Y
    // (DAT_07ea982c/30) como base. Mientras el origen no esté escrito (== 0) se
    // sale, para no disparar hit-tests fantasma del lado izquierdo.
    if (DAT_07eaa116 == '\0' || DAT_07ea982c == 0) return;

    // Mouse hover zone
    if ((int)DAT_083a427c >= (int)DAT_07ea982c &&
        (int)DAT_083a427c <  (int)(DAT_07ea982c + 0xbe) &&
        (int)DAT_083a4278 >= (int)DAT_07ea9830 &&
        (int)DAT_083a4278 <  (int)(DAT_07ea9830 + 0x1b1)) {
        DAT_07d78094 = 1;
    }

    // Clave XOR (32 bytes, la misma que se usa en todo el archivo)

    int iRow     = 0;
    int iRowAccum = 0;
    int iX = DAT_083a427c;
    int iY = DAT_083a4278;

    // ── Botones [+] de stats ─────────────────────────────────────────────────
    // Fiel a IDA sub_4E5DE0 L85-244:
    //   1. Paso de fila **60 px y 4 filas** (`v8 += 60; while (v8 < 240)`) =
    //      Fuerza / Agilidad / Vitalidad / Energía.
    //   2. El paquete es `[C1][len][F3][06][statIdx]` (buf[2]=0xC1, buf[4]=0xF3,
    //      el subcode 6 y el índice al final) = PMSG_LEVEL_UP_POINT_RECV
    //      (Protocol.h:128) — y sale por el camino C3 con serial.
    //   3. No usar `send()` crudo: sin serial ni CSM el server cierra el socket
    //      ("Protocol size error").
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
                gNetwork.Send(pkt, 5);

                PlayBuffer(0x19, 0, 0);
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
        PlayBuffer(0x19, 0, 0);
        PlayBuffer(0x1c, 0, 0);
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    }
}
// SecondPassword_Screen4 @ 0x004E6550 — SecondPassword_Screen4 (257 lines)
//   - Wrong-password error handler: displays SetErrorMessage error dialog for various
//     server error codes. Resets PIN buffer (DAT_07ea9814/18/1c = 0).
//   - Contiene una llamada a __ftol() (FPU ST0 → long) para el límite Y del botón, con origen float desconocido.
//     Casi toda la lógica (ruido de ref-count de HashTable + zonas de hover) está limpia, pero el origen del FPU
//     unresolvable without call-site context. STUB: __ftol() button region.
void __cdecl SecondPassword_Screen4(void) {
    // DESVIACION: publicar el origen antes del hit-test, sin esperar al draw.
    InventoryStartX = gWindow.PanelOrigin((CharacterOpened || PartyOpened) ? Ui::InventoryPair : Ui::RightPanel);
    InventoryStartY = 0;

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

    char sv1 = DAT_07eaa118;
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

    char sv2 = DAT_07eaa11b;
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
        uint uVar4 = HashTable_GetIndex(&MAIN_HASH_CLASS, (void*)DAT_07cf1ffc);
        if (uVar4 == 0xffffffff) {
            void* pvVar10 = AntiTamper_HashNode();
            *(static_cast<BYTE*>(pvVar10) + 0x584) = 1;
            HashTable_Insert(&MAIN_HASH_CLASS, pvVar10, puVar8);
        }
        // Update slot visibility based on item type == -1
        if (*(short*)(static_cast<BYTE*>(DAT_07cf1ffc) + 0x218 + i) == -1)
            *(static_cast<BYTE*>(DAT_07cf1ffc) + 0x258 + i) = 0;
        else
            *(static_cast<BYTE*>(DAT_07cf1ffc) + 0x258 + i) = 1;
    }

    // IDA sub_4E6550 L160-231: reset por frame del byte ITEM.Color (+0x40) de
    // todas las grillas, ANTES del hover (sub_4D23B0) y del marcado del drop
    // (sub_4DF410), que corren despues en UpdateWindowsMouse.  Celda ocupada
    // = 1, vacia = 0; en el pool de la tienda la marca 99 se conserva.
    {
        struct { BYTE* pool; int cells; } grids[] = {
            { OffsetInventoryItems, 64 }, { OffsetTradeItems, 32 },
            { OffsetWarehouseItems, 120 }, { OffsetMixItems, 32 },
            // IDA recorre tambien las 120 celdas de la tienda (L200-208);
            // sin esto las marcas de hover (Color 2, azul) nunca se borraban.
            { ShopItems, 120 },
        };
        for (auto& g : grids)
            for (int i = 0; i < g.cells; ++i) {
                BYTE* cell = g.pool + i * 0x44;
                cell[0x40] = (*(short*)cell != -1) ? 1 : 0;
            }
        for (int i = 0; i < 32; ++i) {
            BYTE* cell = Inventory + i * 0x44;
            if (*(short*)cell == -1)       cell[0x40] = 0;
            else if (cell[0x40] != 99)     cell[0x40] = 1;
        }
    }

    // IDA sub_4E6550 L232-240: por debajo de InventoryStartY + 200 esta la
    // grilla (sub_4D23B0); por arriba, los casilleros de equipo (sub_4D1FC0).
    DAT_07eaa164 = 0;
    int lVal = (int)((double)(int)DAT_07ea5284 + 200.0);
    if ((int)DAT_083a4278 < lVal) {
        FUN_004d1fc0();
    } else {
        // inv_base = OffsetInventoryItems: en el binario original DAT_07ea8410 ES la
        // base del pool de inventario; en nuestro build es un DWORD de 4 bytes en
        // globals.cpp y el pool es OffsetInventoryItems.
        FUN_004d23b0((char*)(uintptr_t)(DAT_07ea5288 + 0xf),
                     (int)(DAT_07ea5284 + 200),
                     (short*)OffsetInventoryItems, 8, 8, '\0');
    }

    DAT_07eaa138 = 0;
    // Char-count check and back-button
    char sv3 = DAT_07eaa11b;
    if (sv3 == '\0') {
        char sv4 = DAT_07eaa118;
        if (sv4 == '\0' && DAT_07eaa119 == '\0' && DAT_07eaa11a == '\0' && GoldenArcherOpenType == 0) {
            ushort uVar3 = *(ushort*)(static_cast<BYTE*>(DAT_07cf1ff4) + 0xe);
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
                CloseInventoryRelatedWindows();
                DAT_07e11d28 = 0;
                DAT_00559bec = 6;
            }
        }
    }

    char sv5 = DAT_07eaa11b;
    if (sv5 != '\0') {
        // Trade — DAT_07ea5298 / DAT_07ea7b88 son DWORDs (4 bytes)
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
    // El render de shop/warehouse/chaos (RenderShopInterface, HUD_Pass6.cpp) usa
    // 260 / 0 como origen (DAT_07eaa0c8 / DAT_07eaa0cc).  El hover de abajo lee
    // DAT_07eaa0c8, así que se sincroniza el global con los valores del render
    // para que coincidan.
    DAT_07eaa0c8 = gWindow.PanelOrigin(Ui::InventoryPair);
    DAT_07eaa0cc = 0;
    char sv6 = DAT_07eaa118;
    if (sv6 != '\0') {
        // Pool de TIENDA: ShopItems (en el original, el overlay
        // &Inventory[32].WalkSpeed, offset +24), el mismo que usa el render. Grid 8×15.
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
// SecondPassword_Screen5 @ 0x004E6C40 — SecondPassword_Screen5 (783 lines)
//   - Click del selector de nivel de la ventana de evento (Devil Square /
//     Blood Castle); ver el detalle abajo.
void __cdecl SecondPassword_Screen5(void) {
    // Click del selector de nivel del evento.  El binario NO tiene aca ningun
    // teclado de PIN: son 4 filas (Devil Square) o 6 (Blood Castle) alineadas
    // con las que dibuja `RenderEventWindow` (0x4F3C50).
    //
    // Ojo con la CLAVE XOR anti-tamper que Hex-Rays emite inline
    // (`v59 = -25; v60 = 109; ...` = E7 6D 3A 89 ...): no son bytes del
    // paquete.  El chequeo de nivel usa `m_iDevilSquareLimitLevel` (ver
    // globals.h), no `(&DAT_00559f60)[i*2]`.
    if (!EventWindowOpened) return;

    // ── Blood Castle ────────────────────────────────────────────────────────
    if (EventType == 1) {
        DAT_07eaa0c8 = gWindow.PanelOrigin(Ui::Event);
        DAT_07eaa0cc = 0;
        if (!MouseLButtonPush) return;

        const WORD charLevel = *(WORD*)((BYTE*)CharacterAttribute + 14);
        const int mgOffset = ((((const BYTE*)Hero)[EntityOffset(444)] & 7) == 3) ? 6 : 0;

        int row = 0;
        for (; row < 6; ++row) {
            const int limitRow = row + mgOffset;
            // Fila fuera del rango de nivel: no es clickeable (y RenderEventWindow
            // la dibuja grisada).  Esto lo tiene Blood Castle y no Devil Square.
            if (charLevel > (unsigned)m_iBloodCastleLimitLevel[limitRow][1] ||
                charLevel < (unsigned)m_iBloodCastleLimitLevel[limitRow][0])
                continue;
            if (Ui::EventButton(true, row, gWindow.GetRightShift()).Contains((int)MouseX, (int)MouseY))
                break;
        }
        if (row >= 6) return;

        MouseLButtonPush = 0;
        if (DAT_07e91388) return;

        const int itemSlot = GetItemSlot(434, row + 1);
        if (itemSlot == -1) { CreateOkMessageBox(GlobalText[854]); return; }

        // MuEmu PMSG_BLOOD_CASTLE_ENTER_RECV: el server hace `level -= 1` y
        // `slot -= INVENTORY_WEAR_SIZE`.
        const BYTE pkt[5] = { 0xC1, 0x05, 0x9A, (BYTE)(row + 1), (BYTE)(itemSlot + 12) };
        gNetwork.SendC1(pkt, 5);
        return;
    }

    if (EventType != 0) return;

    // ── Devil Square ────────────────────────────────────────────────────────
    DAT_07eaa0c8 = gWindow.PanelOrigin(Ui::Event);
    DAT_07eaa0cc = 0;
    if (!MouseLButtonPush) return;

    int row = 0;
    for (; row < 4; ++row) {
        if (Ui::EventButton(false, row, gWindow.GetRightShift()).Contains((int)MouseX, (int)MouseY))
            break;
    }
    if (row >= 4) return;

    MouseLButtonPush = 0;
    if (DAT_07e91388) return;

    int playerLevel = *(WORD*)((BYTE*)CharacterAttribute + 14);
    if ((*((BYTE*)CharacterAttribute + 11) & 7) == 3)   // Magic Gladiator
        playerLevel = 3 * ((playerLevel + 1) / 2);

    if (playerLevel > m_iDevilSquareLimitLevel[row][1]) {
        // Nivel demasiado alto: cierra el inventario, manda el cancel y avisa.
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
        const BYTE cancel[3] = { 0xC1, 0x03, 0x31 };
        gNetwork.SendC1(cancel, 3);
        Item_ReturnPickedItem();
        CreateOkMessageBox(GlobalText[686]);
        return;
    }
    if (playerLevel < m_iDevilSquareLimitLevel[row][0]) {
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
        const BYTE cancel[3] = { 0xC1, 0x03, 0x31 };
        gNetwork.SendC1(cancel, 3);
        Item_ReturnPickedItem();
        CreateOkMessageBox(GlobalText[687]);
        return;
    }

    int itemSlot = GetItemSlot(467, 0);
    if (itemSlot == -1) itemSlot = GetItemSlot(467, row + 1);
    if (itemSlot == -1) { CreateOkMessageBox(GlobalText[677]); return; }

    // MuEmu PMSG_DEVIL_SQUARE_ENTER_RECV.  Ojo: Devil Square suma **24** al
    // slot, no 12 como Blood Castle (IDA: `buf[size+2] = v20 + 24`).
    const BYTE pkt[5] = { 0xC1, 0x05, 0x90, (BYTE)row, (BYTE)(itemSlot + 24) };
    gNetwork.SendC1(pkt, 5);
}

// CheckGoldenArcherWindow vive en UI/GoldenArcher.cpp.
extern "C" int g_GoldenArcherCustom;
extern "C" bool __cdecl GoldenArcher_CustomNpcIdle(intptr_t c, int action);
// ServerTransfer_HitTest @ 0x004E8B70 -- IDA: sub_4E8B70, clicks de la ventana de
// transferencia de server (la etiqueta vieja "SecondPassword_Screen7" era falsa).
// Los dos `__ftol()` que el port no habia resuelto son `InventoryStartX + 35.0`:
// el hit-test de Aceptar y Cancelar estaba 35 px a la izquierda del boton.
void __cdecl ServerTransfer_HitTest(void) {
    if (!g_bServerDivisionEnable) return;

    int mx = (int)DAT_083a427c;   // MouseX
    int my = (int)DAT_083a4278;   // MouseY
    if (mx >= gWindow.PanelOrigin(Ui::RightPanel) && mx < gWindow.PanelOrigin(Ui::WindowRight) && my >= 0 && my < 433)
        DAT_07d78094 = 1;         // MouseOnWindow

    const int startX = (int)DAT_07ea5288;   // InventoryStartX
    const int startY = (int)DAT_07ea5284;   // InventoryStartY

    // Casilla "acepto".
    if (mx >= startX + 25 && mx < startX + 41 && my >= 240 && my < 256 && DAT_083a4124) {
        DAT_083a4124 = 0;
        DAT_083a42c4 = 0;
        g_bServerDivisionAccept ^= 1;
    }

    const int buttonX = (int)((double)startX + 35.0);
    if (g_bServerDivisionAccept) {
        // Aceptar: confirmacion con ShowCheckBox.
        if (mx >= buttonX && mx < buttonX + 120 && my >= 320 && my < 344 && DAT_083a4124) {
            DAT_083a4124 = 0;
            DAT_083a42c4 = 0;
            DAT_07eaa13c = 4;
            DAT_00559f5e = (char)0xff;
            ShowCheckBox(1, 448, 151);
            mx = (int)DAT_083a427c;
            my = (int)DAT_083a4278;
        }
    }

    // Cancelar.
    if (mx >= buttonX && mx < buttonX + 120 && my >= 350 && my < 374 && DAT_083a4124) {
        DAT_083a4124 = 0;
        DAT_083a42c4 = 0;
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
        const BYTE pkt[3] = { 0xC1, 0x03, 0x31 };
        gNetwork.SendC1(pkt, sizeof(pkt));
        DAT_07e11d28 = 0;         // MouseUpdateTime
        DAT_00559bec = 6;         // MouseUpdateTimeMax
        mx = (int)DAT_083a427c;
        my = (int)DAT_083a4278;
    }

    // X de cerrar.
    if (mx >= startX + 25 && mx < startX + 49 && my >= startY + 395 && my < startY + 419 && DAT_083a4124) {
        DAT_083a4124 = 0;
        const BYTE pkt[3] = { 0xC1, 0x03, 0x31 };
        gNetwork.SendC1(pkt, sizeof(pkt));
        GoldenArcherOpenType = 0;         // g_bEventChipDialogEnable
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
    }
}
// SecondPassword_Screen8 @ 0x004E9050 — SecondPassword_Screen8 (81 lines)
// Handler del click en el botón OK del diálogo de segunda contraseña.
// Guard: DAT_07eaa11a must be non-zero (button active flag).
// Hit-test: mouse within [DAT_07eaa0c8+0x4b, DAT_07eaa0c8+0x77) x [DAT_07eaa0cc+300, DAT_07eaa0cc+0x14c).
// DAT_07eaa140 must be 0 (no timeout in progress), DAT_083a4124 must be non-zero (click pending).
// Switch on MixType:
//   0         → UIChatLogWindow_AddText(&DAT_07eaa1a0, &DAT_07d544d4, 2) — show wrong-PIN message
//   1,2,3,4,5,6,7,8,0xb → SecondPassword_GridSlotAvail(0x7ea8410, 8, 8, iVar1, iVar3) — send auth
//     sub-switch: cases 1,7,0xb → iVar1=5 iVar3=4; case 8 → iVar1=2 iVar3=2; else → iVar1=DAT_0055a3f8 iVar3=DAT_0055a3fc
//     si tiene éxito (retorno distinto de cero) y DAT_07e91388 < 1: setea DAT_07eaa13c=2, DAT_00559f5e=0xff,
//     call ShowCheckBox(1, 0x21b, 0x97)
//   0xfffffff8, 0xfffffffe → UIChatLogWindow_AddText(&DAT_07eaa19c, &DAT_07d55c44, 2) — show error message
// After switch: PlayBuffer(0x19,0,0).
// "Back" button: [DAT_07ea5288+0x19,DAT_07ea5288+0x31) x [DAT_07ea5284+0x18b,DAT_07ea5284+0x1a3)
//   → SecondPassword_CancelReturn(); DAT_07e11d28=0; DAT_00559bec=6; PlayBuffer(0x19,0,0).
void __cdecl SecondPassword_Screen8(void) {
    if (DAT_07eaa11a == '\0') return;

    // OK button hit-test
    if (DAT_07eaa140 == 0 &&
        (int)DAT_083a427c >= (int)(DAT_07eaa0c8 + 0x4b) &&
        (int)DAT_083a427c <  (int)(DAT_07eaa0c8 + 0x77) &&
        (int)DAT_083a4278 >= (int)(DAT_07eaa0cc + 300) &&
        (int)DAT_083a4278 <  (int)(DAT_07eaa0cc + 0x14c) &&
        IsClickPushed()) {

        DAT_083a4124 = '\0';
        switch ((int)MixType) {
        case 0:
            UIChatLogWindow_AddText(&DAT_07eaa1a0, &DAT_07d544d4, 2);
            break;
        case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 0xb: {
            int iVar1 = 5, iVar3 = 4;
            switch ((int)MixType) {
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
            uint uVar2 = SecondPassword_GridSlotAvail(reinterpret_cast<uintptr_t>(OffsetInventoryItems), 8, 8, iVar1, iVar3);
            if ((char)uVar2 == '\0') {
                UIChatLogWindow_AddText(&DAT_07eaa198, &DAT_07d54600, 2);
            } else {
                if ((int)DAT_07e91388 < 1) {
                    DAT_07eaa13c = 2;
                    DAT_00559f5e = 0xff;
                    ShowCheckBox(1, 0x21b, 0x97);
                }
            }
            break;
        }
        case (int)0xfffffff8:
        case (int)0xfffffffe:
            UIChatLogWindow_AddText(&DAT_07eaa19c, &DAT_07d55c44, 2);
            break;
        }
        PlayBuffer(0x19, 0, 0);
    }

    // Back button hit-test
    if ((int)DAT_083a427c >= (int)(DAT_07ea5288 + 0x19) &&
        (int)DAT_083a427c <  (int)(DAT_07ea5288 + 0x31) &&
        (int)DAT_083a4278 >= (int)(DAT_07ea5284 + 0x18b) &&
        (int)DAT_083a4278 <  (int)(DAT_07ea5284 + 0x1a3) &&
        IsClickPushed()) {

        DAT_083a4124 = '\0';
        SecondPassword_CancelReturn();
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;
        PlayBuffer(0x19, 0, 0);
    }
}
// SecondPassword_Screen9 @ 0x004EB5D0 — SecondPassword_Screen9 (102 lines)
// Detección de fila del teclado numérico del diálogo de segunda contraseña.
// Chequea 4 filas de botones en los offsets x {0x1a, 0x4c, 0x7e, 0xd7} (0x18 de ancho cada uno)
// y los offsets y {0x186, 0x186, 0x186, 0x18b} relativos a DAT_07eaa0c8/0cc.
// Al hacer click (DAT_083a4124 != '\0'), despacha por índice de fila:
//   0 → new char: SetErrorMessage(0x74), ClearInput(0), set substate flags
//   1 → borrar carácter: igual que 0 pero StorageGoldFlag=1
//   2 → shuffle/new-PIN: shuffle DAT_07e91394 short[10] via Fisher-Yates (20 passes),
//        reset _DAT_07ea9814=0, DAT_07ea9818=0, DAT_07eaa14c = 4 - DAT_00559f5f,
//        DAT_07ea981c=0, DAT_07ea981e=0
//   3 → exit: Net_Disconnect_Clean(); DAT_07e11d28=0; DAT_00559bec=6
void __cdecl SecondPassword_Screen9(void) {
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
    PlayBuffer(0x19, 0, 0);

    switch (iVar6) {
    case 0:
        SetErrorMessage(0x74);
        ClearInput(0);
        InputGold = 0;
        StorageGoldFlag = 0;
        // DESVIACION (fix del DLL, Patchs.cpp 0x004EB6C3): depósito de Zen con diez dígitos.
        InputTextMax = 10;
        InputNumber = 1;
        DAT_00559c84 = 0;
        GoldInputEnable = 1;
        break;
    case 1:
        SetErrorMessage(0x74);
        ClearInput(0);
        InputGold = 0;
        // DESVIACION (fix del DLL, Patchs.cpp 0x004EB6FE): extracción de Zen con diez dígitos.
        InputTextMax = 10;
        InputNumber = 1;
        DAT_00559c84 = 0;
        GoldInputEnable = 1;
        StorageGoldFlag = 1;
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
        memset(DAT_07ea9814, 0, 11);   // el texto tipeado (10 + NUL)
        DAT_07eaa14c = 4u - (DAT_00559f5f != 0 ? 1u : 0u);
        break;
    }
    case 3:
        Net_Disconnect_Clean();
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

    char cGuard = DAT_07eaa11b;
    if (cGuard == '\0') return;

    // Botón Zen: [x+26,x+50) × [y+390,y+414).
    if ((int)(DAT_07ea5290 + 0x1a) <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5290 + 0x32) &&
        (int)(DAT_07ea528c + 0x186) <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea528c + 0x19e) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        SetErrorMessage(0x74);
        ClearInput(0);
        // DESVIACION (fix del DLL, Patchs.cpp 0x004EB8C7): oferta de Zen con diez dígitos.
        InputTextMax = 10;
        InputNumber = 1;
        DAT_00559c84 = 0;
        GoldInputEnable = 1;
        InputGold = 0;
        StorageGoldFlag = 2;
        PlayBuffer(0x19, 0, 0);
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
        PlayBuffer(0x19, 0, 0);

        if (DAT_07eaa0e8 == '\0') {
            if (m_bMyConfirm == '\0') {
                m_bMyConfirm = '\x01';
            } else {
                m_bMyConfirm = '\0';
            }
        } else {
            if (m_bMyConfirm == '\0') {
                DAT_07eaa13c = 3;
                DAT_00559f5e = (char)0xff;
                ShowCheckBox(4, 0x173, 0x97);
                return;
            }
            m_bMyConfirm = '\0';
        }
        DAT_07eaa0e8 = '\x01';

        // PMSG_TRADE_OK_BUTTON_RECV: C1:3C, flag=1. El emisor compartido
        // aporta correctamente el envoltorio C3, chain-XOR y serial rotativo.
        BYTE pkt[4] = { 0xC1, 0x04, 0x3C, 0x01 };
        gNetwork.Send(pkt, sizeof(pkt));
    }

    // Botón de cancelación: [x+137,x+161) × [y+390,y+414).
    if ((int)(DAT_07ea5290 + 0x89) <= (int)DAT_083a427c &&
        (int)DAT_083a427c < (int)(DAT_07ea5290 + 0xa1) &&
        (int)(DAT_07ea528c + 0x186) <= (int)DAT_083a4278 &&
        (int)DAT_083a4278 < (int)(DAT_07ea528c + 0x19e) &&
        IsClickPushed()) {
        DAT_083a4124 = '\0';
        PlayBuffer(0x19, 0, 0);
        if (DAT_07e91388 == 0) {
            DAT_07eaa0e8 = '\0';
            // CGTradeCancelButtonRecv: C1:3D, no payload.
            BYTE pkt3[3] = { 0xC1, 0x03, 0x3D };
            gNetwork.Send(pkt3, sizeof(pkt3));
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }
}

// FUN_004ec330 @ 0x004EC330 — Shop controls and inventory close hit-test (389 lines)
//   - Handles shop bottom buttons (Buy, Repair, Repair All) when DAT_07eaa132 != 0
//   - Updates repair cost per-frame via Item_RecalculateRepairCost()
//   - Handles inventory close button click
void __cdecl FUN_004ec330(void) {
    uint uVar3 = HashTable_GetIndex(&MAIN_HASH_CLASS, &DAT_07eaa118);
    if (uVar3 == 0xffffffff) {
        void* pv = AntiTamper_HashNode();
        static_cast<unsigned char*>(pv)[1] = 1;
        HashTable_Insert(&MAIN_HASH_CLASS, pv, &DAT_07eaa118);
    }
    char cGuard = DAT_07eaa118;

    if (cGuard != '\0') {
        if (DAT_07eaa132 != '\0') {
            // Button 1 (Buy): [panelX + 25, panelX + 49) x [panelY + 365, panelY + 389)
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

            // Button 3 (Repair): [panelX + 85, panelX + 109) x [panelY + 365, panelY + 389)
            int iX2 = (int)DAT_07eaa0c8 + 0x55;
            if (iX2 <= (int)DAT_083a427c && (int)DAT_083a427c < iX2 + 0x18 &&
                iY1 <= (int)DAT_083a4278 && (int)DAT_083a4278 < iY1 + 0x18 &&
                IsClickPushed()) {
                DAT_07eaa134 ^= 1;
                DAT_083a4124 = '\0';
                ((BYTE*)&DAT_07eaa150)[2] = DAT_07eaa134 ? 2 : 0;
            }

            // IDA sub_4EC330: boton "reparar todo" (x+115, y+365).  Manda
            // C1:05:34:FF:00 (slot 0xFF = todo, 0 = reparacion en NPC).  Si no
            // se clickea, LABEL_71 recalcula el costo (sub_4C4080) cada frame.
            int iX3 = (int)DAT_07eaa0c8 + 0x73;
            if (iX3 <= (int)DAT_083a427c && (int)DAT_083a427c < iX3 + 0x18 &&
                iY1 <= (int)DAT_083a4278 && (int)DAT_083a4278 < iY1 + 0x18 &&
                IsClickPushed()) {
                DAT_083a4124 = '\0';
                BYTE pkt[5] = { 0xC1, 0x05, 0x34, 0xFF, 0x00 };
                gNetwork.Send(pkt, sizeof(pkt));
            } else {
                Item_RecalculateRepairCost();   // sub_4C4080
            }

            // main.exe FUN_004ec330:004eca7c calls FUN_004c4080 per-frame when DAT_07eaa132 != 0
            Item_RecalculateRepairCost();
        }

        // Back/cancel button at [DAT_07ea5288+0x19, +0x31) x [DAT_07ea5284+0x18b, +0x1a3)
        if ((int)(DAT_07ea5288 + 0x19) <= (int)DAT_083a427c &&
            (int)DAT_083a427c < (int)(DAT_07ea5288 + 0x31) &&
            (int)(DAT_07ea5284 + 0x18b) <= (int)DAT_083a4278 &&
            (int)DAT_083a4278 < (int)(DAT_07ea5284 + 0x1a3) &&
            IsClickPushed()) {
            DAT_083a4124 = '\0';
            DAT_07eaa117 = 0;
            CloseInventoryRelatedWindows();
            DAT_07e11d28 = 0;
            DAT_00559bec = 6;
        }
    }
}

// ShowCheckBox (0x0051E240) vive en src/Item/Item_ClickHandler.cpp (port
// fiel, con GlobalText[166..169] en la rama del mensaje 153); no duplicarla acá.

// SecondPassword_GridSlotAvail @ 0x004E3DB0 — SecondPassword_GridSlotAvail
// Escanea una grilla 2D (param_3×param_2 filas/columnas) en el array de inventario en param_1,
// con stride 0x44 por celda; devuelve 1 si alguna celda de la ventana está libre (slot==-1), si no 0.
// SecondPassword_GridSlotAvail (IDA-activated)
uint __cdecl SecondPassword_GridSlotAvail(uintptr_t a1, int a2, int a3, int a4, int a5)
{
  int v5; // ebp
  int v6; // eax
  int v8; // esi
  char v10; // bl
  uintptr_t v11;
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


// SecondPassword_CancelReturn @ 0x004F6850 — SecondPassword_CancelReturn
// Chequea si hay slots de entidad de segunda contraseña ocupados; si los hay, muestra el diálogo.
// Otherwise sends a cancel packet (C1 03 87) over the socket.
undefined4 __cdecl SecondPassword_CancelReturn(void)
{
    if (!ChaosMixOpened) return 0;
    return ChaosBoxRequestClose() ? 1 : 0;
#if 0
    bool bVar1 = true;
    // El bound `< 0x7eaa0c8` es una direccion absoluta del binario
    // fuente.  Es el mismo pool de 32 slots de 0x44 que resetea Net_PacketSession
    // (DAT_07ea9880), abordado 0x38 antes: (0x7EAA0C8 - 0x7EA9848) / 0x44 = 32,
    // o sea 4 vueltas del bucle externo por 8 del interno.
    short *psVar2 = (short *)&DAT_07ea9848;
    for (int nRow = 0; nRow < 4; ++nRow) {
        int iVar6 = 8;
        do {
            if ((*psVar2 != -1) && (0 < *(int *)((char*)psVar2 + 0x1c * 2))) bVar1 = false;
            psVar2 += 0x22;
            iVar6--;
        } while (iVar6 != 0);
    }
    if ((!bVar1) || (0 < (int)DAT_07e91388)) {
        UIChatLogWindow_AddText((const char*)&lpDefault_00583d88, (const char*)&DAT_07d55410, 2);
        return 0;
    }
    // Send C1 03 87 logout/cancel packet
    char pkt[3]; pkt[0] = (char)0xC1; pkt[1] = 3; pkt[2] = (char)0x87;
    unsigned int uVar8 = 3;
    int iVar6 = 0;
    if (Platform::GetSocket().IsOpen(SocketClientSocket)) {
        do {
            int iVar3 = send(SocketClientSocket, pkt + iVar6, 3 - iVar6, 0);
            if (iVar3 == -1) {
                iVar6 = Platform::GetSocket().LastError();
                if (iVar6 != 0x2733) { Net_Disconnect(SocketClient); break; }
                if (0x2000 < (int)(SocketClientSendBufferLength + 3)) { Net_Disconnect(SocketClient); break; }
                memcpy(SocketClientSendBuffer + SocketClientSendBufferLength, pkt, uVar8);
                SocketClientSendBufferLength += uVar8;
                break;
            }
            if (iVar3 == 0) break;
            if (SocketClientLogPrint != 0) FUN_0043de60();
            uVar8 -= iVar3; iVar6 += iVar3;
        } while (0 < (int)uVar8);
    }
    return CONCAT31((int3)(SocketClientSocket >> 8), 1);
#endif
}

// Net_Disconnect_Clean @ 0x004F6A70 — Net_Disconnect_Clean
// Limpia el estado de la UI y después manda un paquete de desconexión (C1 03 82) por el socket.
uint __cdecl Net_Disconnect_Clean(void)
{
    if (DAT_07eaa165 != '\0') return 0;
    DAT_07eaa117 = 0;
    CloseInventoryRelatedWindows();
    if (0 < (int)DAT_07e91388) Item_ReturnPickedItem();
    const char pkt[3] = { (char)0xC1, 3, (char)0x82 };
    gNetwork.SendRaw(pkt, 3);
    return 1;
}


// UI_OpenWindow alias (UI_AddNotice)
void __cdecl UI_OpenWindow(char* title, int mode) {
    UI_AddNotice(title, (unsigned char)mode);
}
