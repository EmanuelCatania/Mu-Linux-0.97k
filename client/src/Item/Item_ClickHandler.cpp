// Item_ClickHandler.cpp
//
// Port FIEL del IDA decompile `sub_4D23B0` (`004D23B0_sub_4D23B0.c`, 14783
// bytes / ~1521 líneas). Esta es la función central de **interacción con
// items** del cliente: render del grid + hit-test + click handlers para
// pickup / drop / sell / right-click-use / hotkey-assign / pet-renaming.
//
// Llamada por SecondPassword.cpp:712-782 (4-7 sitios) per scene tick:
//   - Main inventory: `FUN_004d23b0(InventoryStartX+15, InventoryStartY+200,
//     OffsetInventoryItems, 8, 8, 0)`
//   - Trade slots (own/peer): grids 8x4
//   - Warehouse: 8x15 con flag=1 (sell mode) o flag=0 (open mode)
//   - Mix: 8x4
//
// La firma original en IDA es:
//   void __cdecl sub_4D23B0(char *a1, int a2, __int16 *a3, int a4, int a5,
//                            char a6)
// Donde:
//   a1 = origin_x  (top-left grid X en pantalla)
//   a2 = origin_y  (top-left grid Y)
//   a3 = pointer al ITEM array (Inventory / Trade / Warehouse / Mix)
//   a4 = grid width (typically 8)
//   a5 = grid height (8 / 4 / 15)
//   a6 = mode flag:
//        0 = inventory mode (left-click pickup, right-click use, hotkey)
//        1 = modo venta (el click izquierdo vende el item al NPC vía el paquete 0x32)
//
// **Anti-tamper noise omitido** per project policy:
//   - Bloques XOR con keys v263..v294 alrededor de cada packet send
//   - HashTable encrypt/decrypt sobre CharacterMachine (sub_4041E0,
//     sub_403F80, sub_404280, sub_404330, sub_404370, sub_404400, sub_423710)
//   - Repeated key init + reverse loops (compiler artifact / anti-tamper)
//
// Lo que SÍ está aquí (game logic):
//   - Hit-test mouse over cell + multi-cell footprint highlight
//   - Pickup: qmemcpy slot bytes → pPickedItem buffer + set source slot
//   - Venta a NPC: arma el paquete 0x32 y lo manda por el socket
//   - Repair: build packet 0x35 (sub-mode 1 / 2 / 3)
//   - Use potion/scroll: build packet 0x29 + EnableUse=10 cooldown
//   - Right-click "open scroll" specials: 0x49 sub 0x91 type 1/2 (item 467/434)
//   - Right-click pet egg (item 431): qmemcpy slot a word_7EA5240 + ShowCheckBox
//   - Right-click default: pickup with WarehouseOpened auto-drop logic
//   - Ctrl+Q/W/E hotkey assignment: dword_559C60[slot] = item type
//
// 2026-05-08: port completo desde IDA (sustituye al stub no-op anterior en
// SecondPassword.cpp:2027). Habilita la cadena entera:
//   FUN_004d23b0 → pPickedItem set → FUN_004df410 (drop dispatcher,
//   también stub — port pendiente) → SendRequestEquipmentItem.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"
#include "Net/Net.h"
#include <winsock2.h>

extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;   // defined in Render_Frame.cpp

// ── Globals referenced (declared elsewhere) ──────────────────────────────────
extern "C" {
    extern BYTE  OffsetInventoryItems[];   // HUD_Pass3.cpp — main inv (8x8 + 12 wear)
    extern BYTE  OffsetTradeItems[];       // HUD_Pass3.cpp — trade grid
    extern BYTE  OffsetMixItems[];
    extern BYTE  OffsetWarehouseItems[];
    extern BYTE  Inventory[];              // HUD_Pass3.cpp — shop/trade-in pool
}

// Alias que usa el guard de la RAMA A del dispatch de clicks. Per IDA L396-399, el
// shared `Inventory` pool (shop/trade-in) skips click handling.
static BYTE* const g_InventoryPoolForClickGuard = Inventory;

// IDA aliases for our DAT_ globals
#define pPickedItem        (*(BYTE(*)[0x44])DAT_07e91350)
#define dword_7E91388      DAT_07e91388        // pickup-active flag
#define dword_7EA9800      DAT_07ea9800        // current inv base ptr
// dword_7EA9844 NO existe: en el binario 0x7EA9844 es un BYTE flag (bSell).
// La "posicion del item agarrado" vive en Inventory[32].Type → macro ItemPickedPos.
#define byte_7EA9844       (*((BYTE*)&DAT_07ea9844))   // first byte of dword_7EA9844 = mode flag
#define byte_83A42EB       DAT_083a42eb        // auto-drop trigger flag
#define dword_55CC16C      DAT_055cc16c        // queued send buffer cursor
// 2026-08-22 FIX: este alias apuntaba a DAT_05826d1c, que es OTRO global.
// `ida_xrefs_to` los separa: 0x05826D18 lo escribe ProtocolCore y lo lee
// sub_4D23B0 (cooldown de COMPRA), mientras 0x05826D1C lo escriben InitGame,
// ReceiveLife y ReceiveDurability (cooldown de equipar/usar, `EnableUse`).
// Compartiendo el mismo byte, comprar bloqueaba el equipar y viceversa.
// Las dos direcciones están a ~4 bytes; acá las tratamos como el mismo concepto.
#define dword_5826D18      DAT_05826d18

// CheckInventory: aliasa el puntero al ITEM del slot bajo el mouse que usa Scene_MapTick
// to dispatch RenderItemInfo (tooltip).
//
// 2026-05-08: BUG-FIX MAYÚSCULO. Antes apuntaba a `DAT_07e11d24` que IDA
// llama `dword_7E11D24` (= un global completamente distinto, usado por
// sub_494520 IME/text input). El símbolo correcto es `DAT_07eaa160` —
// confirmado por Ghidra-decompiled Scene_MapTick línea 35/89 que lee
// `DAT_07eaa160` como el item pointer y línea 89 lo pasa como 3er arg
// a `FUN_004c4650` (RenderItemInfo).
//
// Sin este fix:
//   * Hover loop seteaba DAT_07e11d24 (wrong global) → Scene_MapTick leía
//     DAT_07eaa160 (= 0) → no entraba a la rama de tooltip → nunca se
//     renderizaba RenderItemInfo.
//   * Y peor: estábamos contaminando dword_7E11D24 que es input-buffer-state
//     → bugs latentes en chat input.
extern DWORD DAT_07eaa160;                     // IDA `CheckInventory`
#define CheckInventory     DAT_07eaa160

// Tabla de hotkeys en dword_559C60[3] (Q/W/E) — los globals ya están declarados como
// `int` in globals.h.
// (Indexamos DAT_00559c60 como un array de 3 int para guardar los hotkeys.)

// DAT_07ea5240 (buffer del slot del diálogo del huevo de mascota, 0x44 bytes) está declarado como DWORD
// en globals.h — acá lo tratamos como un buffer de bytes contiguo para el memcpy.
// DAT_07ea5249 es el primer byte del campo de índice de slot, que en el layout
// de IDA viene inmediatamente después de DAT_07ea5240. Como no lo tenemos como
// símbolo separado en nuestro build, lo proyectamos con aritmética de offsets.
#define DAT_07ea5249_byte  (*((BYTE*)&DAT_07ea5240 + 9))

// Posiciones del overlay del baúl (se usan en la rama del click derecho por defecto).
// DAT_07eaa0c8/cc están declarados en globals.h.

// dword_5826D18 = cooldown de RepairEnable (en nuestro build es `int DAT_05826d1c`
// según globals.h). Usamos la coincidencia más cercana — el valor numérico sigue el
// repair-cooldown frames.

// Índices de los IDs de mensaje "trade ocupado" / "no podés usar esto" — declarados como DWORD o
// char en globals.h, y se acceden con aritmética de offsets sobre UIChatLogWindow_AddText.
// En el original de IDA, byte_7EAA184/188/18C son strings de id de slot. Nuestro build
// los referencia vía la familia DAT_07eaa11b, que tiene otra semántica —
// los mensajes se mandan con ID vacío porque la UI del chat lo acepta así.

// ── Declaraciones adelantadas de los helpers que usamos ──────────────────────
void __cdecl ShowCheckBox(int mode, int x, int y);   // declared in stubs
void __cdecl CreateOkMessageBox(char* strMsg);
void __cdecl DeleteBug(int Owner);

// PressKey/sub_4D6020: los provee Input.cpp/stubs — no siempre están expuestos
// por un header. Los declaramos acá inline. Los dos tienen que ser `__cdecl` sin
// mangling, igual que las implementaciones que ya existen.
extern int  __cdecl PressKey(int vk);
extern unsigned int __cdecl FUN_004d6020(int origin_x, int origin_y,
                                          int inv_base, int grid_w, int grid_h);
#define sub_4D6020 FUN_004d6020

// Latches de MousePosition para el pickup con click derecho. En IDA los originales están en
// 0x083a42e0 / 0x083a42e4, pero en nuestro build esas direcciones se superponen con CameraAngle
// — usamos statics locales al archivo para no pisar el estado de la cámara.
static DWORD g_PickupLatchX = 0;
static DWORD g_PickupLatchY = 0;

// Trade item array (declared in HUD_Pass3.cpp). Some translation units
// ya lo exponen con linkage C; si nuestra referencia no enlaza, la
// capa de compatibilidad de abajo usa un offset dentro del pool OffsetInventoryItems.
extern "C" BYTE OffsetWarehouseItems[];   // declared in HUD_Pass3.cpp
extern "C" BYTE OffsetTradeItems[];

// ── PressKey (FUN_0047ec20, 61 bytes) — port FIEL desde IDA ──────────────────
// Chequeo de "tecla recién apretada" por flanco, usando DAT_07e118ec como tabla
// de estado anterior. Devuelve true en el primer frame que la tecla está apretada,
// y después false hasta que se suelte y se vuelva a apretar.
int __cdecl PressKey(int vk)
{
    if (vk < 0 || vk >= 256) return 0;
    if (((unsigned short)GetAsyncKeyState(vk) >> 8) == 0x80) {
        if (DAT_07e118ec[vk] == 0) {
            DAT_07e118ec[vk] = 1;
            return 1;
        }
    } else {
        DAT_07e118ec[vk] = 0;
    }
    return 0;
}

// ── sub_4D6020 (367 bytes) — port FIEL desde IDA ────────────────────────────
// Busca un slot vacío en la grilla destino donde entre pPickedItem, y después
// engancha el cursor del mouse (DAT_083a427c/DAT_083a4278) al centro de
// ese slot. Devuelve 1 si lo encontró, 0 si no.
//
// Args: (origin_x, origin_y, grid_base, grid_w, grid_h)
unsigned int __cdecl FUN_004d6020(int origin_x, int origin_y,
                                   int grid_base, int grid_w, int grid_h)
{
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

    short pickedType = *(short*)pPickedItem;
    if (pickedType < 0) return 0;
    ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    if (!attr) return 0;
    int itemW = attr[pickedType].Width;
    int itemH = attr[pickedType].Height;
    if (itemW <= 0 || itemW > 8 || itemH <= 0 || itemH > 8) return 0;
    int maxX  = grid_w - itemW;
    int maxY  = grid_h - itemH;
    if (maxX < 0 || maxY < 0) return 0;

    // Busca columna por columna de derecha→izquierda, y fila por fila de abajo→arriba
    // (matches IDA's reverse iteration). Look for any rectangle where all
    // (itemW*itemH) cells have Type == 0xFFFF (empty).
    for (int x = maxX; x >= 0; --x) {
        for (int y = maxY; y >= 0; --y) {
            int empty = 0;
            for (int dx = 0; dx < itemW; ++dx) {
                for (int dy = 0; dy < itemH; ++dy) {
                    BYTE* cell = (BYTE*)(uintptr_t)(grid_base + 68 *
                                 ((y + dy) * grid_w + (x + dx)));
                    if (*(short*)cell == (short)0xFFFF || *(int*)(cell + 0x38) <= 0) {
                        ++empty;
                    }
                }
            }
            if (empty == itemW * itemH) {
                // Snap mouse to slot center
                DAT_083a427c = (DWORD)(int)(
                    ((double)itemW * 0.5 + (double)x - 0.5) * 20.0
                    + (double)origin_x);
                DAT_083a4278 = (DWORD)(int)(
                    ((double)itemH * 0.5 + (double)y - 0.5) * 20.0
                    + (double)origin_y);
                return 1;
            }
        }
    }
    return 0;
}

// ── ShowCheckBox (FUN_0051E240, 506 bytes) — port FIEL desde IDA ────────────
// Opens a 2-button (Yes/No) confirmation dialog. The dialog uses the same
// `g_lpszMessageBoxCustom` 7×38 buffer as `CreateOkMessageBox`, plus 2
// button-rect descriptors for the Yes/No clickable areas.
//
// Args:
//   num     = cantidad de líneas de mensaje adicionales (después de la línea de título)
//   index   = GlobalText[index..index+num-1] = body text rows
//   message = which dialog template:
//               153 = "Really use pet egg?" (item 431 right-click)
//                 1 = "Sell this item?" (shop sell-confirm)
//                 2 = "¿Tirar este item al piso?" (confirmación de drop)
//                 3 = generic NPC sell confirm
//
// Returns: TRUE always (matches IDA — never errors out).
//
// 2026-05-08: port completo, reemplaza al placeholder que llamaba a
// CreateOkMessageBox. Maneja la máquina de estados del diálogo en la que
// FUN_004df410 se apoya para los flujos de confirmación de venta/drop/renombrar mascota.
extern char DAT_083a44c4[7 * 0x26];      // g_lpszMessageBoxCustom (266 bytes)
extern DWORD DAT_07ea5244;               // pet egg item Level field
// DAT_083a42f8 (2 entradas × 5 ints) y su alias DAT_083a430c (= entrada 1)
// vienen de globals.h — NO redeclarar aca: DAT_083a430c es un macro que
// proyecta dentro de DAT_083a42f8, y declararlo como array independiente
// dejaba los botones Yes/No escritos en memoria que el render no lee.

// NextErrorMessage vive en DAT_083a7c28, según las notas de la máquina de estados de CLAUDE.md.
extern DWORD DAT_083a7c24;               // ErrorMessage (currently-shown dialog)
extern DWORD DAT_083a7c28;               // NextErrorMessage (queued)
#define ErrorMessage      DAT_083a7c24
#define NextErrorMessage  DAT_083a7c28

void __cdecl ShowCheckBox(int num, int index, int message)
{
    char* lpMessageLines = DAT_083a44c4;        // 7 × 38-byte line buffer
    int   numLines = num;

    if (message == 153) {
        // Special case: pet egg rename dialog. Build dynamic title with
        // the egg's "level" → pet kind (Demon/Spirit/Drake/etc.).
        char Buffer[52];
        memset(Buffer, 0, 50);
        if (*(short*)&DAT_07ea5240 == 431) {
            int eggKind = (((int)DAT_07ea5244) >> 3) & 0xF;
            const char* kindName = nullptr;
            switch (eggKind) {
            case 0: kindName = GlobalText[168]; break;  // Demon
            case 1: kindName = GlobalText[169]; break;  // Spirit
            case 2: kindName = GlobalText[167]; break;  // ?
            case 3: kindName = GlobalText[166]; break;  // ?
            default:
                kindName = "";
                break;
            }
            if (kindName) snprintf(Buffer, sizeof(Buffer), "%s", kindName);
        }
        snprintf(lpMessageLines, 38, "( %s )", Buffer);
        numLines = num + 1;

        // Las filas del cuerpo (una por cada argumento `num`) salen de GlobalText[index..]
        if (numLines > 1) {
            char* dst = lpMessageLines + 38;
            for (int i = 0; i < num; ++i) {
                snprintf(dst, 38, "%s", GlobalText[index + i]);
                dst += 38;
            }
        }
    } else {
        // Generic case: copy `num` lines starting from GlobalText[index].
        if (num > 0) {
            char* dst = lpMessageLines;
            for (int i = 0; i < num; ++i) {
                snprintf(dst, 38, "%s", GlobalText[index + i]);
                dst += 38;
            }
        }
        numLines = num;
    }
    g_iNumLineMessageBoxCustom = numLines;
    // 2026-07-27 FIX (cartel de confirmación vacío): el render de las líneas del
    // message box (UI_StatsPanel case 0x97/0x99) usa DAT_083a4324 como count del
    // loop, pero ShowCheckBox sólo seteaba g_iNumLineMessageBoxCustom. En el
    // binario original son la MISMA dirección; en nuestro build están separados
    // → el render iteraba 0 líneas → cartel en blanco. Seteamos ambos.
    extern DWORD DAT_083a4324;
    DAT_083a4324 = (DWORD)numLines;

    // Yes button rectangle: id=1 at (21, 90) size 70x21 → DAT_083a42f8[0..4]
    DAT_083a42f8[0] = 1;
    DAT_083a42f8[1] = 21;
    DAT_083a42f8[2] = 90;
    DAT_083a42f8[3] = 70;
    DAT_083a42f8[4] = 21;

    // No button rectangle: id=3 at (120, 90) size 70x21 → DAT_083a430c[0..4]
    DAT_083a430c[0] = 3;
    DAT_083a430c[1] = 120;
    DAT_083a430c[2] = 90;
    DAT_083a430c[3] = 70;
    DAT_083a430c[4] = 21;

    if (message != 0) {
        // Mostrar este mensaje — encolarlo si ya hay otro diálogo arriba.
        if (ErrorMessage != 0) {
            NextErrorMessage = (DWORD)message;
        } else {
            ErrorMessage = (DWORD)message;
        }
    } else {
        // message==0: desencola (descarta el actual y muestra el siguiente)
        ErrorMessage     = NextErrorMessage;
        NextErrorMessage = 0;
    }
}

// PressKey helper (Input.cpp)
extern int __cdecl PressKey(int vk);

// Envío de red: send de bajo nivel por socket, con desborde al buffer encolado.
// Imita el patrón `send / cola de WSAEWOULDBLOCK` de la función de IDA.
static void SendPacketBytes(const void* data, int size)
{
    if (!data || size <= 0) return;
    if (DAT_055ca168 == (DWORD)INVALID_SOCKET) return;

    const char* p = (const char*)data;
    int sent = 0, remaining = size;
    while (remaining > 0) {
        int r = ::send((SOCKET)DAT_055ca168, p + sent, remaining, 0);
        if (r == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                // Encola la cola no enviada en DAT_055ca16c+DAT_055cc16c
                int qlen = (int)DAT_055cc16c;
                if (qlen + remaining <= 0x2000) {
                    memcpy((char*)DAT_055ca16c + qlen, p + sent, remaining);
                    DAT_055cc16c = qlen + remaining;
                }
            } else {
                // Other error: close socket
                if (DAT_055ca168 != (DWORD)INVALID_SOCKET) {
                    closesocket((SOCKET)DAT_055ca168);
                    DAT_055ca168 = (DWORD)INVALID_SOCKET;
                }
            }
            return;
        }
        if (r == 0) return;
        sent      += r;
        remaining -= r;
    }
}

// Build & send a C1-header packet [C1][size][header...payload].
//
// 2026-08-08 FIX MAYÚSCULO (desconexiones "de la nada" + al subir stats):
// esto incrementaba `DAT_05826ceb` (g_byPacketSerialSend) en CADA envío C1.
// Pero el serial SÓLO viaja en los frames C3/C4: el server lee
// `QueueInfo.serial = DecSerial` para C3/C4 y `serial = -1` para C1/C2
// (SocketManagerLinux.cpp:248-316), y sólo entonces avanza su `m_RecvSerial`
// (CSerialCheck::CheckSerial exige `m_RecvSerial + 1 == serial`).
// O sea: cada packet C1 que salía por acá corría NUESTRO contador sin que el
// server corriera el suyo → el siguiente packet C3 llegaba con el serial
// adelantado → `CheckPacketHack` loguea "Packet serial error" y hace
// CloseClient. Como el keep-alive 0x0E es C3 y sale cada segundo, la
// desconexión llegaba ~1 s después de cualquier click que mandara un C1
// (de ahí el "me desconectó estando quieto" tras usar el diálogo de venta).
// El bump lo hace Net_SendSmallPacket, que es quien realmente escribe el
// serial en el frame.
static void SendC1Packet(BYTE* payload, int payloadSize)
{
    if (payloadSize <= 0 || payloadSize > 250) return;

    BYTE pkt[256];
    pkt[0] = 0xC1;
    pkt[1] = (BYTE)(payloadSize + 2);   // total size = header(2) + payload
    memcpy(pkt + 2, payload, payloadSize);
    SendPacketBytes(pkt, payloadSize + 2);
}

// 2026-07-27: envío C3 (CSimpleModulus + serial) para los opcodes de tienda
// que el server exige con Encrypt=1 (HackPacketCheck.txt): 0x32 buy, 0x33 sell,
// 0x23 drop. Enviarlos como C1 (SendC1Packet) → el server los rechaza en
// HackPacketCheck ("Packet encryption error") → CloseClient (desconexión al
// clickear un item de la tienda). Net_SendSmallPacket arma [C1][len][head]...,
// pisa len con el serial, aplica chain-XOR + CSM y emite el frame C3 final.
void Net_SendSmallPacket(const BYTE* pkt, int totalLen);
static void SendC3Packet(BYTE* payload, int payloadSize)
{
    if (payloadSize <= 0 || payloadSize > 250) return;
    BYTE pkt[256];
    pkt[0] = 0xC1;
    pkt[1] = (BYTE)(payloadSize + 2);   // total size (Net_SendSmallPacket lo pisa con serial)
    memcpy(pkt + 2, payload, payloadSize);
    Net_SendSmallPacket(pkt, payloadSize + 2);
}

// 2026-07-27: tile del terreno donde dropear un item al suelo. El server
// (CGItemDropRecv) usa x/y como TILE del mapa y valida cercanía al player —
// mandar pixels de mouse (lo que hacía el port) daba un tile off-map →
// gMap.ItemDrop rechazaba → result=0 → el item nunca se dropeaba. Usamos el
// tile actual del héroe (ent+0x306/0x307 = target_grid_x/y), siempre válido y
// pegado al player.
static void GetHeroDropTile(BYTE* outX, BYTE* outY)
{
    *outX = 0; *outY = 0;
    BYTE* hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
    if (!hero) return;
    *outX = hero[0x306];
    *outY = hero[0x307];
}

extern "C" void __cdecl SyncPickedItemVisualState(void);

// 2026-07-27: devuelve el item agarrado a su slot de origen y suelta el cursor.
// Se usa al cancelar los diálogos de confirmación (venta / drop al suelo); sin
// esto el item quedaba pegado al mouse y no había forma de soltarlo.
// No es `static`: la usa tambien el handler del 0x33 (venta rechazada por el
// server) en Net/Net_Process.cpp.
void RestorePickedItemToSource(void)
{
    if (dword_7E91388 == 0) return;
    BYTE* srcPool = (BYTE*)(uintptr_t)dword_7EA9800;
    int   srcSlot = (int)DAT_07ea5b18;
    if (srcPool && srcSlot >= 0) {
        // Reconstruir los bytes "wire" desde el ITEM struct guardado y
        // re-insertarlo en su celda original.
        BYTE* it = (BYTE*)pPickedItem;
        BYTE wire[6] = { 0, 0, 0, 0, 0, 0 };
        wire[0] = it[0];     // Type low
        wire[1] = it[4];     // optByte (Level int, low byte)
        wire[2] = it[26];    // Durability
        wire[3] = it[60];    // Unkown (bit8 de type + exc)
        wire[4] = it[61];    // byColorState
        int gridH = (srcPool == &OffsetWarehouseItems[0]) ? 15
                  : ((srcPool == &OffsetInventoryItems[0]) ? 8 : 4);
        FUN_004cc660(srcPool, 8, gridH, srcSlot, wire, 1);
    }
    dword_7E91388 = 0;
    memset(pPickedItem, 0, 0x44);
    dword_7EA9800 = 0;
    DAT_07ea5b18  = 0xFFFFFFFFu;
    DAT_07eaa165  = 0;
    DbgLogPublic("DROP-CANCEL: item devuelto a su slot de origen");
}

// ─────────────────────────────────────────────────────────────────────────────
// UI_Main — port FIEL desde IDA decompile `004CCE00_UI_Main.c` (1455 bytes).
//
// Saca del grid de inventario el item recién levantado, limpiando su
// huella multi-celda (Type=-1, Level=0). Para los slots de equipo del inventario
// principal, también limpia el slot de equipo correspondiente de CharacterMachine y
// updates Hero's equip pointers (Lefthand=624, Righthand=648, Pet=696).
//
// Anti-tamper hash-table noise around CharacterMachine ref-count omitted
// per project policy.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" void __cdecl UI_Main(int slot_idx, short* inv_base, unsigned int gridW)
{
    if (slot_idx == 255) return;   // sentinel: no-op

    auto clear_footprint = [&](ITEM* slot) {
        if (!slot) return;
        short type = slot->Type;
        if (type == -1) return;

        ITEM_ATTRIBUTE* attr = &((ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068)[type];
        int width = attr->Width;
        int height = attr->Height;
        int y = slot->y;
        while (y < slot->y + height) {
            unsigned int rowBase = gridW * y;
            int x = slot->x;
            while (x < slot->x + width) {
                short* cell = &inv_base[34 * rowBase + 34 * x];
                *cell = -1;
                *((DWORD*)cell + 14) = 0;
                ++x;
            }
            ++y;
        }
    };

    // Branch 1: a2 != OffsetInventoryItems → trade/warehouse/mix grids.
    // Just clear the footprint at slot[a1] — no equip-slot bookkeeping.
    if ((BYTE*)inv_base != (BYTE*)&OffsetInventoryItems[0]) {
        ITEM* slot = &((ITEM*)inv_base)[slot_idx];
        if (slot->Type == -1) return;
        clear_footprint(slot);
        return;
    }

    // Branch 2: main inventory (a3 == OffsetInventoryItems).
    if (slot_idx >= 12) {
        ITEM* slot = &((ITEM*)inv_base)[slot_idx - 12];
        if (slot->Type == -1) return;
        clear_footprint(slot);
        return;
    }

    // Branch 3: equipment slot 0..11. Clear the corresponding wear slot
    // en CharacterMachine (+536 para la fila de la mano izquierda).
    static const int kEquipSlotToByteOffset[12] = {
        536, 604, 672, 740, 808, 876, 944, 1012, 1080, 1148, 1216, 1284
    };

    BYTE* CM = (BYTE*)DAT_07cf1ffc;
    if (CM == nullptr) return;
    BYTE* wearSlot = CM + 68 * slot_idx + 536;
    if (*(WORD*)wearSlot != 0xFFFF) {
        // 2026-08-08 FIX (glow dorado pegado al desequiparse):
        // este clear era PARCIAL — ponía Type=-1, Key=-1 y el byte +27, pero
        // dejaba el campo **Level** (+4) con el valor del item que se acababa de
        // sacar. El diagnóstico SETGLOW lo mostró:
        //   cmT=242/-1/-1/338/370   cmL=11/11/11/11/11
        // o sea los slots vaciados seguían con nivel 11. Después
        // `SetCharacterClass` copia ese nivel al body-part por defecto
        //   *(BYTE*)(c+530) = (*(int*)(CM_armor + 4) >> 3) & 0xF
        // → lvlE=11 sobre el cuerpo desnudo (partsE=923/930 = modelos default)
        // → `Entity_DrawSetup` le aplica el glow de +11 al cuerpo desnudo.
        // Por eso el pj quedaba dorado aunque `CheckFullSet` ya devolvía 0.
        // Dejamos el slot en el MISMO estado que produce
        // `WriteEquipmentSlot(slot, -1, …)` (memset + Type=-1), que es también
        // el que manda el server en el snapshot F3/10.
        memset(wearSlot, 0, sizeof(ITEM));
        *(WORD*)(wearSlot + 0) = 0xFFFF;
        *(DWORD*)(wearSlot + 56) = 0xFFFFFFFFu;

        // Ajustes del puntero al héroe para los slots de equipo conocidos.
        BYTE* heroPtr = (BYTE*)DAT_07abf5d8;
        if (heroPtr) {
            switch (slot_idx) {
            case 0:  // Lefthand weapon
                *(WORD*)(heroPtr + 624) = 0xFFFF;
                *(BYTE*)(heroPtr + 627) = 0;
                break;
            case 1:  // Righthand weapon
                *(WORD*)(heroPtr + 648) = 0xFFFF;
                *(BYTE*)(heroPtr + 651) = 0;
                break;
            case 8:  // Pet (Helper)
                DeleteBug((int)(uintptr_t)heroPtr);
                *(WORD*)(heroPtr + 696) = 0xFFFF;
                *(BYTE*)(heroPtr + 699) = 0;
                break;
            default:
                break;
            }
        }
    }

    if (inv_base == (__int16*)Inventory) {
        SortInventory_stub(inv_base);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_004d23b0 — port FIEL desde IDA `004D23B0_sub_4D23B0.c` (14783 bytes).
// Inventory grid render + click dispatcher.  See file header for signature.
// ─────────────────────────────────────────────────────────────────────────────
void __cdecl FUN_004d23b0(char* origin_x, int origin_y, short* inv_base,
                          int grid_w, int grid_h, char mode_flag)
{
    // ── DIAG (2026-05-08): loguea la entrada cada vez que hay un pulso de click, para
    // poder ver si la función se alcanza y qué estado ve.
    if (DAT_083a4124 != 0 || DAT_083a42d0 != 0) {
        char b[400];
        int p = wsprintfA(b,
            "FUN_004d23b0 CLICK ENTRY: pool=%08X origX=%d origY=%d gw=%d gh=%d mf=%d "
            "EnableUse=%d Eq=%d picked=%d  Lpush=%d Rpush=%d  mouse=(%d,%d)\n",
            (unsigned)(uintptr_t)inv_base, (int)(uintptr_t)origin_x, origin_y,
            grid_w, grid_h, (int)mode_flag,
            (int)EnableUse, (int)DAT_07eaa165, (int)dword_7E91388,
            (int)DAT_083a4124, (int)DAT_083a42d0,
            (int)DAT_083a427c, (int)DAT_083a4278);
        // Calcula la celda bajo el mouse y vuelca los primeros 4 shorts de ese slot.
        int mx = (int)DAT_083a427c;
        int my = (int)DAT_083a4278;
        int gx = (mx - (int)(uintptr_t)origin_x) / 20;
        int gy = (my - origin_y) / 20;
        if (gx >= 0 && gx < grid_w && gy >= 0 && gy < grid_h) {
            int slotIdx = gy * grid_w + gx;
            short* slot = inv_base + 34 * slotIdx;
            p += wsprintfA(b + p,
                "  hover slot=%d (gx=%d gy=%d) types[0..3]=%04hX %04hX %04hX %04hX  xy=(%d,%d)",
                slotIdx, gx, gy,
                slot[0], slot[1], slot[2], slot[3],
                ((BYTE*)slot)[62], ((BYTE*)slot)[63]);
        }
        DbgLogPublic(b);
    }

    // ── Guards (IDA lines 307-318) ──────────────────────────────────────────
    bool wasClick = (DAT_083a4124 != 0);

    // 2026-05-09: ItemAttribute base watchdog. Algún writer desconocido
    // sets DAT_07d78068 a valores bogus (e.g., 0x1) ocasionalmente. Hay un
    // watchdog en Render_GameFrame, pero entre éste y FUN_004d23b0 puede
    // re-corromperse — bloqueando el pickup en attr-bogus check más abajo.
    // Restaurar acá del backup si está corrupt.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            if (wasClick) {
                char db[160];
                wsprintfA(db, "FUN_004d23b0 RESTORE attr: was=%08X → backup=%08X",
                          (unsigned)p, (unsigned)g_ItemAttribute_Backup);
                DbgLogPublic(db);
            }
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }

    if (grid_h <= 0)                { if (wasClick) DbgLogPublic("FUN_004d23b0 EXIT: gh<=0"); return; }

    // ── Pre-pasada: limpia todos los resaltados de hover de este pool (slot[64] = 0) ──
    // 2026-05-08: el IDA original NO resetea entre frames y depende de que el
    // caller (FUN_004ecb00 chain en state=4) lo limpie en otro path. En state=5
    // nuestro hook desde RenderInventoryWindow llama esta función directamente,
    // y sin reset todos los items quedan permanentemente azulados (slot[64]=2)
    // tras pasar el mouse. Recreamos aquí el "limpiar antes de re-marcar":
    // dejamos en 0 todas las celdas con `Color != 99` (=99 es la marca dorada
    // de currency/zen blink y debe persistir).
    for (int r = 0; r < grid_h; ++r) {
        for (int c = 0; c < grid_w; ++c) {
            int idx = r * grid_w + c;
            BYTE* p = (BYTE*)(inv_base + 34 * idx) + 64;
            if (*p != 99) *p = 0;
        }
    }

    if ((int)EnableUse > 0)         { if (wasClick) DbgLogPublic("FUN_004d23b0 EXIT: EnableUse>0"); return; }
    // 2026-07-27 FIX (baúl: no se puede meter ni sacar nada): DAT_07eaa165 es el
    // guard "item-move en vuelo" — se setea al mandar el 0x24 y sólo lo limpia
    // la RESPUESTA del server (ItemMove_ClearPickedState). Si un move se pierde
    // o el server no responde, el guard queda pegado en 1 y ESTE early-return
    // bloquea TODO el manejo de inventario/baúl para siempre. Timeout de
    // seguridad: si lleva >2 s seteado, lo liberamos.
    if (DAT_07eaa165 != 0) {
        static DWORD s_guardSince = 0;
        DWORD now = GetTickCount();
        if (s_guardSince == 0) s_guardSince = now;
        else if (now - s_guardSince > 2000) {
            s_guardSince = 0;
            DAT_07eaa165 = 0;
        }
        if (DAT_07eaa165 != 0) {
            if (wasClick) DbgLogPublic("FUN_004d23b0 EXIT: EquipmentItem!=0");
            return;
        }
    } else {
        static DWORD s_guardReset = 0; (void)s_guardReset;
    }
    if (dword_7E91388 > 0)          { if (wasClick) DbgLogPublic("FUN_004d23b0 EXIT: already-picked"); return; }

    int rowStride = 68 * grid_w;              // bytes per row in the inv array

    // ── Outer loop: rows ────────────────────────────────────────────────────
    for (int row = 0; row < grid_h; ++row) {
        int      rowY    = origin_y + 20 * row;
        char*    rowX    = origin_x;
        short*   rowSlot = inv_base + (34 * grid_w) * row;

        // ── Inner loop: columns ─────────────────────────────────────────────
        for (int col = 0; col < grid_w; ++col, rowX += 20, rowSlot += 34) {
            int cellX = (int)(uintptr_t)rowX;
            int cellY = rowY;

            // Hit-test: el mouse está adentro de esta celda Y el slot tiene un item
            if ((int)DAT_083a427c < cellX || (int)DAT_083a427c >= cellX + 20 ||
                (int)DAT_083a4278 < cellY || (int)DAT_083a4278 >= cellY + 20 ||
                rowSlot[0] == -1)
            {
                // Asignación de hotkey (Ctrl+Q/W/E) cuando no se está sobre un item
                // activo — líneas 1010-1023 de IDA. Se queda con el
                // hovered slot via CheckInventory.
                continue;
            }
            // DIAG: cell passed hit-test
            if (DAT_083a4124 != 0) {
                char db2[160];
                wsprintfA(db2,
                    "FUN_004d23b0 HIT row=%d col=%d type=%04X Lpush=%d",
                    row, col, (unsigned short)rowSlot[0],
                    (int)DAT_083a4124);
                DbgLogPublic(db2);
            }

            // ── Hovered cell with item: highlight footprint (color=2) ──────
            // 2026-05-08: defensive guards. Crash reported with addr=0x21
            // (= offset de ITEM_ATTRIBUTE.Height) al pasar sobre un item, que
            // means `attr + type*64` reduced to NULL. Possible causes:
            //   * DAT_07d78068 not yet initialized (loader race)
            //   * `type` fuera de rango (rowSlot apuntando a basura)
            // Los dos están acotados ahora.
            ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
            if (!attr) { if (DAT_083a4124) DbgLogPublic("FUN_004d23b0 SKIP: attr=NULL"); continue; }
            // Reject implausible attr base.
            if ((uintptr_t)attr < 0x100000 || (uintptr_t)attr >= 0x80000000) {
                if (DAT_083a4124) {
                    char dbsk[160];
                    wsprintfA(dbsk, "FUN_004d23b0 SKIP: attr-bogus DAT_07d78068=%08X", (unsigned)DAT_07d78068);
                    DbgLogPublic(dbsk);
                }
                continue;
            }
            short typeRaw   = rowSlot[0];
            if (typeRaw < 0 || typeRaw >= 1024) { if (DAT_083a4124) DbgLogPublic("FUN_004d23b0 SKIP: typeRaw OOB"); continue; }
            int   type      = (int)(unsigned short)typeRaw;
            BYTE  slotX     = ((BYTE*)rowSlot)[62];
            BYTE  slotY     = ((BYTE*)rowSlot)[63];

            // Normaliza las sub-celdas bajo el mouse al slot de origen real del item.
            // El inventario guarda el origen (x,y) en cada celda de la huella; la lógica
            // de IDA siempre razona desde ese origen, no desde la celda arbitraria
            // donde cayó el mouse.
            if ((int)((ITEM*)rowSlot)->Key <= 0) {
                if ((int)slotX < grid_w && (int)slotY < grid_h) {
                    short* originSlot = (short*)(inv_base + 34 * (grid_w * (int)slotY + (int)slotX));
                    short originType = originSlot[0];
                    if (originType >= 0 && originType < 1024) {
                        rowSlot = originSlot;
                        typeRaw = originType;
                        type = (int)(unsigned short)originType;
                        slotX = ((BYTE*)rowSlot)[62];
                        slotY = ((BYTE*)rowSlot)[63];
                    }
                }
            }

            int   itemW     = attr[type].Width;
            int   itemH     = attr[type].Height;
            if (itemW <= 0 || itemW > 8 || itemH <= 0 || itemH > 8) {
                if (DAT_083a4124) {
                    char dbsk[120];
                    wsprintfA(dbsk, "FUN_004d23b0 SKIP: itemWH bad type=%d W=%d H=%d", type, itemW, itemH);
                    DbgLogPublic(dbsk);
                }
                continue;
            }
            if ((int)slotX + itemW > grid_w) {
                if (DAT_083a4124) {
                    char dbsk[120];
                    wsprintfA(dbsk, "FUN_004d23b0 SKIP: slotX+W>gw slotX=%d W=%d gw=%d", (int)slotX, itemW, grid_w);
                    DbgLogPublic(dbsk);
                }
                continue;
            }
            if ((int)slotY + itemH > grid_h) {
                if (DAT_083a4124) {
                    char dbsk[120];
                    wsprintfA(dbsk, "FUN_004d23b0 SKIP: slotY+H>gh slotY=%d H=%d gh=%d", (int)slotY, itemH, grid_h);
                    DbgLogPublic(dbsk);
                }
                continue;
            }

            for (int yy = 0; yy < itemH; ++yy) {
                int cellRow = (slotY + yy) * grid_w;
                for (int xx = 0; xx < itemW; ++xx) {
                    int idx = cellRow + slotX + xx;
                    BYTE* p = (BYTE*)(inv_base + 34 * idx) + 64;
                    // Don't overwrite the special "currency" highlight (=99).
                    if ((BYTE*)inv_base != (BYTE*)&OffsetInventoryItems[0] || *p != 99) {
                        *p = 2;
                    }
                }
            }

            CheckInventory     = (int)(uintptr_t)rowSlot;
            byte_7EA9844       = (BYTE)mode_flag;

            // ── 2026-05-08: escribe los globals de posición del tooltip (sx/sy en IDA) ──
            // Scene_MapTick los lee en cada frame y se los pasa a
            // RenderItemInfo. Sin estas escrituras el tooltip no aparece nunca
            // (o aparece en 0,0). Per IDA L388-395:
            //   sx = origin_x + 20*slotX + 20*ItemAttribute[type].Width / 2
            //   sy = origin_y + 20*slotY
            extern DWORD DAT_07ea840c;   // tooltip X (= IDA `sx`)
            extern DWORD DAT_07ea8408;   // tooltip Y (= IDA `sy`)
            DAT_07ea840c = (DWORD)((int)(uintptr_t)origin_x +
                                   20 * (int)slotX + 20 * itemW / 2);
            DAT_07ea8408 = (DWORD)(origin_y + 20 * (int)slotY);

            int gridSlotIdx    = grid_w * slotY + slotX;

            // ── RAMA A: per IDA L396-399, cuando la grilla bajo el mouse es el
            // pool compartido `Inventory` (= el pool de entrada de tienda/trade), se saltea
            // el dispatch de clicks — ese pool es de sólo lectura del lado del usuario.
            // Antes este chequeo estaba invertido (matcheaba OffsetInventoryItems),
            // lo que silenciaba todos los handlers de click del inventario PRINCIPAL.
            if ((BYTE*)inv_base == g_InventoryPoolForClickGuard)
            {
                // Read-only shop/trade inventory pool — no pickup/sell here.
                continue;
            }

            // ── BRANCH B: mode_flag != 0 → SHOP BUY (click en item de tienda) ──
            // 2026-07-27 FIX: esta rama sólo la alcanza el shop (único caller con
            // mode_flag=1, SecondPassword.cpp:818). NO es sell — es BUY: clickeás
            // un item del grid de la tienda para comprarlo. El port anterior:
            //   (1) lo etiquetó "sell" y mandó `0x32 0x01 slot 0` (sub-byte que el
            //       server 0.97k/MuEmu NO usa → leía slot=0x01), y
            //   (2) lo envió como C1, pero PMSG_ITEM_BUY_RECV (0x32) exige C3
            //       (HackPacketCheck Encrypt=1) → CloseClient = la desconexión que
            //       el usuario veía al tocar un item de la tienda.
            // Server espera: [C1][04][32][slot], C3-encrypted (ItemManager.h:63).
            if (mode_flag != 0) {
                if (!DAT_083a4124) return;   // no LMB push — wait
                if ((int)dword_5826D18 != 0) return;  // buy cooldown activo
                DAT_083a4124 = 0;            // consume click

                int slotIdx = ((BYTE*)rowSlot)[62] + grid_w * ((BYTE*)rowSlot)[63];
                // guarda el precio para que el server pueda confirmar (anti-dupe)
                dword_5826D18 = (DWORD)ItemValue((ITEM*)(inv_base + 34 * slotIdx), 0);

                BYTE pkt[4];
                pkt[0] = 0x32;              // CGItemBuyRecv
                pkt[1] = (BYTE)slotIdx;     // shop slot 0..119
                {
                    char db[96];
                    wsprintfA(db, "SHOP BUY (C3): slot=%d price=%d", slotIdx, (int)dword_5826D18);
                    DbgLogPublic(db);
                }
                SendC3Packet(pkt, 2);
                return;
            }

            // ── DIAG: loguea el estado del dispatch justo antes de las ramas ──
            {
                char db[200];
                wsprintfA(db,
                    "FUN_004d23b0 DISPATCH type=%d slotXY=(%d,%d) Lpush=%d Rpush=%d "
                    "DAT_07e11d18=%d DAT_07eaa134=%d mode=%d",
                    (int)typeRaw, (int)slotX, (int)slotY,
                    (int)DAT_083a4124, (int)DAT_083a42d0,
                    (int)DAT_07e11d18, (int)DAT_07eaa134, (int)mode_flag);
                DbgLogPublic(db);
            }

            // ── BRANCH C: RepairEnable mode (NPC repair UI active) ─────────
            // 2026-05-09 FIX: per IDA xrefs `RepairEnable_0` = address
            // 0x07EAA134 (= our DAT_07eaa134, the B-key/repair-mode flag).
            // Antes usábamos `DAT_07e11d18` que es OTRO global no relacionado;
            // pero como siempre vale 0, no era el bug. Lo dejamos al lado por
            // si un day el reading DWORD del IDA picks up DAT_07e11d18+...
            // bytes adyacentes. Lo importante: *ambos* deben ser 0 para que
            // pickup dispare. Si DAT_07eaa134 está pegado en 1 (porque
            // Scene_MapTick lo mantiene en 1 cuando DAT_07eaa138 != 0), nunca
            // hay pickup. Usar el OR para detectar el bug.
            if (DAT_07e11d18 != 0 || DAT_07eaa134 != 0) {
                // Tipos de item que SE PUEDEN reparar (= armas/armaduras con
                // durability), excluding stackables like potions/jewels.
                // Per IDA lines 579-586.
                if (!((type >= 416 && type <= 419) || type == 426 || type == 135 ||
                      type == 143 || type >= 448 ||
                      (type >= 391 && type <= 403) ||
                      (type >= 430 && type <= 435))
                    && DAT_083a4124)
                {
                    DAT_083a4124 = 0;

                    // 2026-08-08 FIX: el port mandaba `[0x35][0x01][slot][flag]`
                    // — opcode inventado. El server MuEmu no tiene case 0x35
                    // (Protocol.cpp) así que el paquete se descartaba y reparar
                    // nunca hacía nada. Per IDA sub_4D23B0 L588-640 el paquete es
                    //   [C1][05][34][slot][RepairEnable]
                    // = PMSG_ITEM_REPAIR_RECV (ItemManager.h:75), con
                    // RepairEnable = 0 (reparar en NPC) / 1 (auto-reparar).
                    int slotIdx = ((BYTE*)rowSlot)[62] + grid_w * ((BYTE*)rowSlot)[63] + 12;
                    BYTE pkt[4];
                    pkt[0] = 0x34;
                    pkt[1] = (BYTE)slotIdx;
                    pkt[2] = (BYTE)DAT_07eaa138;    // RepairEnable (0=NPC, 1=self)
                    // HackPacketCheck.txt índice 52 (=0x34) → Encrypt=1: el
                    // server exige frame C3. Mandarlo C1 daba "Packet
                    // encryption error" → CloseClient (la desconexión al
                    // reparar).
                    SendC3Packet(pkt, 3);
                }
                continue;
            }

            // ── BRANCH D: Left-click → PICKUP ──────────────────────────────
            if (DAT_083a4124 != 0) {     // MouseLButtonPush
                {
                    char b[160];
                    wsprintfA(b,
                        "FUN_004d23b0 PICKUP type=%d slotX=%d slotY=%d gw=%d pool=%08X",
                        (int)typeRaw, (int)slotX, (int)slotY, grid_w,
                        (unsigned)(uintptr_t)inv_base);
                    DbgLogPublic(b);
                }
                int sx = ((BYTE*)rowSlot)[62];
                int sy = ((BYTE*)rowSlot)[63];
                int abs = grid_w * sy + sx;
                DAT_083a4124 = 0;

                dword_7EA9800 = (DWORD)(uintptr_t)inv_base;
                memcpy(pPickedItem, (BYTE*)(inv_base + 34 * abs), 0x44);
                SyncPickedItemVisualState();

                // Para el inventario principal, el índice del slot de origen incluye el
                // 12 wear slots offset.
                int sourceSlot = abs;
                if ((BYTE*)inv_base == (BYTE*)&OffsetInventoryItems[0]) {
                    sourceSlot += 12;
                }
                DAT_07ea5b18 = (DWORD)sourceSlot;
                ItemPickedPos = sourceSlot;   // era DAT_07ea9844 (= bSell) — ver globals.h
                DAT_07e9138e = (byte)slotX;
                DAT_07e9138f = (byte)slotY;
                UI_Main(sourceSlot, inv_base, grid_w);
                dword_7E91388 = 1;          // mark "carrying an item"
                CheckInventory = 0;

                // Trade: cuando el item está en la grilla de trade Y ya
                // confirmed our side (m_bMyConfirm), uncomfirm + tell server.
                if (DAT_07eaa11b != 0 &&
                    DAT_07eaa0fd != 0 &&     // m_bMyConfirm
                    (BYTE*)inv_base == (BYTE*)&OffsetTradeItems[0])
                {
                    DAT_07eaa0fd = 0;
                    BYTE pkt[3];
                    pkt[0] = 0x3C;            // opcode (= '<' — trade-uncomfirm)
                    pkt[1] = 0x01;
                    pkt[2] = 0;
                    // HackPacketCheck.txt índice 60 (=0x3C) → Encrypt=1.
                    SendC3Packet(pkt, 3);
                }
                FUN_00404bc0(29, 0, 0);       // pickup sound
                return;
            }

            // ── BRANCH E: Right-click → use item / open dialog ─────────────
            if (DAT_083a42d0 != 0) {       // MouseRButtonPush
                DAT_083a42d0 = 0;

                int slotIdx = grid_w * ((BYTE*)rowSlot)[63] + ((BYTE*)rowSlot)[62];

                // ── Item 458 (Teleport scroll) — handled by Teleport check ─
                if (type == 458) {
                    extern int DAT_05826d04;   // Teleport global flag
                    (void)DAT_05826d04;
                    if ((int)DAT_07e11d18 != 0) return;   // mid-action
                    if (DAT_07eaa119 != 0 || DAT_07eaa11b != 0) {
                        UIChatLogWindow_AddText("", GlobalText[474], 2);
                        continue;
                    }
                    if ((int)EnableUse > 0) continue;
                    EnableUse = 10;

                    // IDA 0x004D23B0 builds this as 0x26 and sends it through
                    // the C3/serial path (not as a direct C1 frame).
                    BYTE pkt[5];
                    pkt[0] = 0x26;
                    pkt[1] = (BYTE)(slotIdx + 12);
                    pkt[2] = 0;
                    SendC3Packet(pkt, 3);

                    // Sonido: 33 para el tipo 448, 32 para 449..457.
                    short t = ((short*)(uintptr_t)&OffsetInventoryItems[0])[34 * slotIdx];
                    if (t == 448) FUN_00404bc0(33, 0, 0);
                    else if (t >= 449 && t <= 457) FUN_00404bc0(32, 0, 0);
                    continue;
                }

                // ── Item 467 (special open scroll, type 1) ────────────────
                if (type == 467) {
                    int level = (((int*)rowSlot)[1] >> 3) & 0xF;
                    BYTE pkt[6];
                    pkt[0] = 0x49;
                    pkt[1] = 0x91;
                    pkt[2] = 0x01;        // sub-type 1
                    pkt[3] = (BYTE)slotIdx;
                    pkt[4] = (BYTE)level;
                    SendC1Packet(pkt, 5);
                    continue;
                }

                // ── Item 434 (special open scroll, type 2) ────────────────
                if (type == 434) {
                    int level = (((int*)rowSlot)[1] >> 3) & 0xF;
                    BYTE pkt[6];
                    pkt[0] = 0x49;
                    pkt[1] = 0x91;
                    pkt[2] = 0x02;        // sub-type 2
                    pkt[3] = (BYTE)slotIdx;
                    pkt[4] = (BYTE)level;
                    SendC1Packet(pkt, 5);
                    continue;
                }

                // ── Items 448-454 / 456-457 / 468 — potions ───────────────
                if ((type >= 448 && type <= 454) ||
                    (type >= 456 && type <= 457) ||
                     type == 468)
                {
                    if (DAT_07eaa119 != 0) {
                        // Baúl abierto → no se puede usar
                        UIChatLogWindow_AddText("", GlobalText[474], 2);
                        continue;
                    }
                    if (DAT_07eaa11b != 0) {
                        UIChatLogWindow_AddText("", GlobalText[474], 2);
                        continue;
                    }
                    if ((int)EnableUse > 0) continue;
                    EnableUse = 10;

                    BYTE pkt[3];
                    // Use the reference client's encrypted C3 envelope.
                    pkt[0] = 0x26;
                    pkt[1] = (BYTE)(slotIdx + 12);
                    pkt[2] = 0;
                    SendC3Packet(pkt, 3);

                    short t = ((short*)(uintptr_t)&OffsetInventoryItems[0])[34 * slotIdx];
                    if (t == 448)               FUN_00404bc0(33, 0, 0);
                    else if (t >= 449 && t <= 457) FUN_00404bc0(32, 0, 0);
                    continue;
                }

                // ── Items 480-511 / 391-398 / 400-403 — scrolls/spells ───
                if ((type >= 480 && type < 512) ||
                    (type >= 391 && type <= 398) ||
                    (type >= 400 && type <= 403))
                {
                    // Class/level/stat requirement gate (per IDA L1278-1283).
                    BYTE* CA = (BYTE*)DAT_07cf1ff4;
                    if (CA &&
                        (*(WORD*)(CA + 14) < attr[type].RequireLevel ||
                         *(WORD*)(CA + 26) < ((WORD*)rowSlot)[16]   ||
                         *(WORD*)(CA + 20) < ((WORD*)rowSlot)[14]))
                    {
                        continue;
                    }
                    if (DAT_07eaa119 != 0 || DAT_07eaa11b != 0) {
                        UIChatLogWindow_AddText("", GlobalText[474], 2);
                        continue;
                    }
                    if ((int)EnableUse > 0) continue;
                    EnableUse = 10;

                    BYTE pkt[3];
                    // IDA 0x004D23B0 proves the reference client sends 0x26
                    // through the C3/serial path. A raw C1 0x26 is rejected
                    // by the deployed server and causes an immediate FD_CLOSE.
                    pkt[0] = 0x26;
                    pkt[1] = (BYTE)(slotIdx + 12);
                    pkt[2] = 0;
                    SendC3Packet(pkt, 3);

                    short t = ((short*)(uintptr_t)&OffsetInventoryItems[0])[34 * slotIdx];
                    if (t == 448)              FUN_00404bc0(33, 0, 0);
                    else if (t >= 449 && t <= 457) FUN_00404bc0(32, 0, 0);
                    continue;
                }

                // ── Item 431 (Pet egg) — open ShowCheckBox dialog ─────────
                if (type == 431) {
                    BYTE* CA = (BYTE*)DAT_07cf1ff4;
                    if (CA && *(WORD*)(CA + 14) < 10) {
                        // Por debajo de nivel 10 — muestra el mensaje "todavía no podés usar esto"
                        CreateOkMessageBox(GlobalText[749]);
                    } else {
                        // Copy item data + open the "really name your pet?" box
                        memcpy(&DAT_07ea5240,
                               (BYTE*)(uintptr_t)&OffsetInventoryItems[0] + 68 * slotIdx,
                               0x44);
                        DAT_07ea5249_byte = (BYTE)slotIdx;
                        ShowCheckBox(1, 376, 153);
                    }
                    return;
                }

                // ── Default right-click: pickup-with-warehouse-auto-drop ──
                // (per líneas 1401-1438 de IDA). Igual que el pickup con click izquierdo pero
                // con lógica extra de auto-drop cuando el baúl está abierto.
                dword_7EA9800 = (DWORD)(uintptr_t)inv_base;
                memcpy(pPickedItem, (BYTE*)(inv_base + 34 * gridSlotIdx), 68);
                g_PickupLatchX = (DWORD)DAT_083a427c;
                g_PickupLatchY = (DWORD)DAT_083a4278;

                if ((BYTE*)inv_base == (BYTE*)&OffsetInventoryItems[0]) {
                    // Pickup from main inventory
                    if (DAT_07eaa119 != 0) {
                        // Warehouse open → try auto-drop into warehouse
                        byte_83A42EB = (char)sub_4D6020(
                            (int)(DAT_07eaa0c8 + 15),
                            (int)(DAT_07eaa0cc + 50),
                            (int)(uintptr_t)&OffsetWarehouseItems[0],
                            8, 15);
                    }
                    if (byte_83A42EB) {
                        int abs = grid_w * ((BYTE*)rowSlot)[63] +
                                  ((BYTE*)rowSlot)[62] + 12;
                        DAT_07ea5b18 = (DWORD)abs;
                        ItemPickedPos = abs;   // era DAT_07ea9844 (= bSell) — ver globals.h
                        DAT_07e9138e = (byte)slotX;
                        DAT_07e9138f = (byte)slotY;
                        UI_Main(abs, inv_base, grid_w);
                        dword_7E91388 = 1;
                        CheckInventory = 0;
                        FUN_00404bc0(29, 0, 0);
                        return;
                    }
                } else {
                    // Pickup from warehouse / mix / trade
                    // 2026-07-27 FIX (no se podían sacar items del baúl): se
                    // pasaba `&InventoryStartX` (la DIRECCIÓN del global) como
                    // origen X del grid destino en vez de su VALOR → el scan de
                    // slot libre en el inventario devolvía 0 → byte_83A42EB=0 →
                    // el pickup nunca se completaba. El origen del grid del
                    // inventario es (InventoryStartX+15, InventoryStartY+200),
                    // igual que en el resto de los call sites.
                    if (DAT_07eaa119 != 0) {
                        byte_83A42EB = (char)sub_4D6020(
                            (int)(InventoryStartX + 15),
                            (int)(InventoryStartY + 200),
                            (int)(uintptr_t)&OffsetInventoryItems[0],
                            8, 8);
                    }
                    if (byte_83A42EB) {
                        int abs = grid_w * ((BYTE*)rowSlot)[63] +
                                  ((BYTE*)rowSlot)[62];
                        DAT_07ea5b18 = (DWORD)abs;
                        ItemPickedPos = abs;   // era DAT_07ea9844 (= bSell) — ver globals.h
                        DAT_07e9138e = (byte)slotX;
                        DAT_07e9138f = (byte)slotY;
                        UI_Main(abs, inv_base, grid_w);
                        dword_7E91388 = 1;
                        CheckInventory = 0;
                        FUN_00404bc0(29, 0, 0);
                        return;
                    }
                }
                memset(pPickedItem, 0, 0x44);
                return;
            } // end right-click

            // ── BRANCH F: Ctrl + Q/W/E → hotkey assignment ─────────────────
            // Per líneas 989-1009 de IDA. Guarda el tipo de item en la tabla de 3 slots
            // table at dword_559C60[0..2].
            SHORT ctrl = GetAsyncKeyState(0x11);   // VK_CONTROL
            if ((char)((unsigned short)ctrl >> 8) == (char)0x80) {
                int hk = -1;
                if (PressKey(0x51))      hk = 0;   // Q
                else if (PressKey(0x57)) hk = 1;   // W
                else if (PressKey(0x45)) hk = 2;   // E
                if (hk >= 0) {
                    short tt = *(short*)(uintptr_t)CheckInventory;
                    // dword_559C60[3] is contiguous (Q/W/E hotkey ring).
                    int* hkArr = &DAT_00559c60;
                    hkArr[hk] = (int)tt;
                }
            }
        } // end column loop
    } // end row loop
}

// Alias del nombre de IDA (algunos headers pueden referirse a ella como RenderItemsBoxes — el
// nombre canónico de IDA `RenderItemsBoxes` en realidad vive en 0x004E37B0, pero el
// Companion-DLL Offsets.h documentation marks 0x004D23B0 as a related
// "RenderItems" entry — provide a friendlier alias for clarity).
extern "C" void __cdecl Inventory_RenderAndClick(char* origin_x, int origin_y,
                                                  short* inv_base, int grid_w,
                                                  int grid_h, char mode_flag)
{
    FUN_004d23b0(origin_x, origin_y, inv_base, grid_w, grid_h, mode_flag);
}

// ─────────────────────────────────────────────────────────────────────────────
// FUN_004df410 — port FIEL desde IDA `004DF410_sub_4DF410.c` (8067 bytes).
//
// Dispatcher de drop del inventario: punto de entrada por frame que llama el tick
// PacketUpdate de la escena (Net_PacketSession.cpp:284). Cuando el jugador tiene
// un item levantado (dword_7E91388 > 0), esta función llama a
// `Inventory_DropItem` (FUN_004D6470 = sub_4D6470) up to four times — once
// por cada contexto de inventario visible: principal, trade, baúl y mix. Cada
// llamada intenta soltar el item si el mouse-up cae sobre esa grilla.
//
// If no drop succeeded AND ShopOpened is set: special bless/luck/level
// guards antes de mandar el paquete de venta al NPC. Si no: soltar al piso
// (item-drop packet 0x23).
//
// State machine: dword_7EAA13C tracks "sell-confirm" / "drop-confirm"
// dialogs spawned via ShowCheckBox(3, ...) — case 1 = sell ok, case 2 =
// drop ok. Both are completed via byte_559F5E response (1=yes, 2=no).
//
// Anti-tamper: cada envío de paquete pasa por la misma encriptación XOR /
// hash-table noise pattern documented in Item_ClickHandler.cpp's main
// dispatcher. Skipped per project policy.
//
// 2026-05-08: port completo. Reemplaza al stub no-op anterior en
// Net/SecondPassword.cpp:153 que decía "STUB: SEH + HashTable".
// ─────────────────────────────────────────────────────────────────────────────

// Inventory_DropItemEx: real entry point with explicit screen origin/grid
// dimensiones (según la firma de sub_4D6470 en IDA). El camino de drop correcto necesita origin_x
// y origin_y para convertir los píxeles del mouse en celdas de la grilla.
extern unsigned int __stdcall Inventory_DropItemEx(int origin_x, int origin_y,
                                                    BYTE* invBase, int gridW,
                                                    int gridH, int slotType);

static unsigned int CallDropItem(int /*a1*/, int origin_x, int origin_y,
                                  void* invBase, int gridW, int gridH,
                                  int slotType)
{
    return Inventory_DropItemEx(origin_x, origin_y, (BYTE*)invBase,
                                gridW, gridH, slotType);
}

void __cdecl FUN_004df410(unsigned int a1, unsigned int /*a2*/)
{
    // ── Header: ChaosMix state machine ───────────────────────────────────────
    // (IDA L200-217). Con ChaosMixOpened, refresca MixType según el contenido de la grilla.
    // Este bloque no depende del item levantado — corre en cada tick.
    extern char DAT_07eaa11a;        // ChaosMixOpened
    extern DWORD DAT_07eaa140;       // MixState
    if (DAT_07eaa11a != 0) {
        // sub_4DF330(invBase, 8, 4) + sub_4E40F0(invBase, 8, 4):
        // chaos-mix-validity helpers. Stubbed elsewhere; safe to skip.
        // La actualización de MixType (DAT_07eaa16c) le corresponde a UI_InGameMenu.
    }

    if (DAT_07eaa165 != 0) return;   // EquipmentItem in-flight

    // ── Drop-confirm / sell-confirm dialog responses ─────────────────────────
    extern DWORD DAT_07eaa13c;       // dialog-active flag (1=sell, 2=drop)
    extern char  DAT_00559f5e;       // dialog response (1=yes, 2=no)
    if (DAT_07eaa13c == 1) {
        if (DAT_00559f5e == 1) {
            // User confirmed sell-to-NPC. Server: [C1][04][33][slot], C3
            // (PMSG_ITEM_SELL_RECV, Encrypt=1). Antes: sub-byte 0x01 + C1 →
            // desconexión.
            DAT_07eaa13c = 0; DAT_00559f5e = 0;
            BYTE pkt[4];
            pkt[0] = 0x33;
            pkt[1] = (BYTE)DAT_07ea5b18;
            SendC3Packet(pkt, 2);
        } else if (DAT_00559f5e == 2) {
            // 2026-07-27 FIX (item pegado al mouse): al CANCELAR el confirm se
            // limpiaban los flags del diálogo pero el item quedaba "en la mano"
            // (dword_7E91388=1) y sin volver a su slot → arrastrado por el cursor
            // para siempre, y cualquier movimiento re-disparaba el cartel.
            // Ahora lo devolvemos al slot de origen y soltamos el cursor.
            DAT_07eaa13c = 0; DAT_00559f5e = 0;
            RestorePickedItemToSource();
        }
        return;
    }
    if (DAT_07eaa13c == 2) {
        if (DAT_00559f5e == 1) {
            // User confirmed drop-on-ground. Server: [C1][05][23][x][y][slot],
            // C3 (PMSG_ITEM_DROP_RECV, Encrypt=1). Formato ya era correcto; sólo
            // faltaba enviarlo C3.
            DAT_07eaa13c = 0; DAT_00559f5e = 0;
            BYTE dx, dy; GetHeroDropTile(&dx, &dy);
            BYTE pkt[4];
            pkt[0] = 0x23;
            pkt[1] = dx;   // tile X del héroe (no pixels de mouse)
            pkt[2] = dy;   // tile Y del héroe
            pkt[3] = (BYTE)DAT_07ea5b18;
            SendC3Packet(pkt, 4);
        } else if (DAT_00559f5e == 2) {
            DAT_07eaa13c = 0; DAT_00559f5e = 0;
            RestorePickedItemToSource();   // ver nota arriba
        }
        return;
    }

    // ── Dispatcher principal de drop: sólo cuando se está cargando un item ──────
    if (dword_7E91388 == 0) return;

    // DIAG: loguea la entrada al dispatcher de drop con el estado del click.
    if (DAT_083a4124 != 0 || DAT_083a42d0 != 0) {
        char db[160];
        wsprintfA(db,
            "FUN_004df410 ENTRY: picked=%d Lpush=%d Rpush=%d invStart=(%d,%d) mouse=(%d,%d) WH=%d Trade=%d Mix=%d",
            (int)dword_7E91388, (int)DAT_083a4124, (int)DAT_083a42d0,
            (int)InventoryStartX, (int)InventoryStartY,
            (int)DAT_083a427c, (int)DAT_083a4278,
            (int)DAT_07eaa119, (int)DAT_07eaa11b, (int)DAT_07eaa11a);
        DbgLogPublic(db);
    }

    // Try drop on main inventory grid (8x8, origin = InventoryStart + 15,200)
    unsigned int dropMain = CallDropItem(
        (int)a1,
        (int)(InventoryStartX + 15),
        (int)(InventoryStartY + 200),
        &OffsetInventoryItems[0], 8, 8, 0);
    if (DAT_083a4124 != 0 || DAT_083a42d0 != 0 || dropMain != 0) {
        char db[120];
        wsprintfA(db, "FUN_004df410 dropMain=%u Lpush_after=%d", dropMain, (int)DAT_083a4124);
        DbgLogPublic(db);
    }

    // Trade grid (8x4, origin = TradeStart + 15,270) if trade open
    unsigned int dropTrade = 0;
    if (DAT_07eaa11b != 0) {
        extern DWORD DAT_07ea5290, DAT_07ea528c;   // TradeInventoryStartX/Y
        dropTrade = CallDropItem((int)a1,
            (int)(DAT_07ea5290 + 15),
            (int)(DAT_07ea528c + 270),
            &OffsetTradeItems[0], 8, 4, 1);
    }

    // Warehouse grid (8x15) if warehouse open
    unsigned int dropWH = 0;
    if (DAT_07eaa119 != 0) {
        dropWH = CallDropItem((int)a1,
            (int)(DAT_07eaa0c8 + 15),
            (int)(DAT_07eaa0cc + 50),
            &OffsetWarehouseItems[0], 8, 15, 2);
    }
    // (diag WHDROP removido 2026-08-08 — ya cumplió: el drop sobre el baúl
    //  funciona y el bug de mover DENTRO del baúl era el clobber de
    //  dword_7EA9800 en Inventory_DropItemEx, no la conversión mouse→celda.)

    // Grilla de mezcla del caos (8x4) si está abierta y la mezcla no se está procesando
    unsigned int dropMix = 0;
    if (DAT_07eaa11a != 0 && DAT_07eaa140 != 1) {
        dropMix = CallDropItem((int)a1,
            (int)(DAT_07eaa0c8 + 15),
            (int)(DAT_07eaa0cc + 110),
            &OffsetMixItems[0], 8, 4, 3);
    }

    // ── Mouse-up → drop didn't land in any inventory: shop sell or ground ──
    if (DAT_083a4124) {
        // 2026-08-08 FIX ("No tienes permitido tirar este item costoso" al
        // mover un item EQUIPADO): soltar sobre una casilla de equipo caía en
        // la rama de tirar-al-suelo.
        //
        // En el binario el drop sobre las casillas de equipo lo consume
        // `sub_4D6470` (36 KB; maneja el grid 8×8 **y** los 12 recuadros de
        // equipo), así que `v177 != 0` y nunca se llega acá. Nuestro
        // `Inventory_DropItemEx` sólo cubre el grid 8×8 y devuelve 0 apenas la
        // celda calculada da negativa — que es justo lo que pasa arriba del
        // grid (mouseY < InventoryStartY+200), o sea toda la zona de equipo.
        // Esa región la maneja `InventoryEquipmentHitTest` (HUD_Pass6) durante
        // el render, así que acá salimos SIN consumir el click para que le
        // llegue. Sin esto: mensaje rojo + `RestorePickedItemToSource`, y el
        // item nunca se equipaba/desequipaba.
        if (!dropMain && !dropTrade && !dropWH && !dropMix &&
            InventoryOpened != 0 &&
            (int)DAT_083a427c >= (int)InventoryStartX &&
            (int)DAT_083a4278 <  (int)(InventoryStartY + 200))
        {
            return;
        }

        DAT_083a4124 = 0;
        // NB: MouseUpdateTime (DAT_07e11d28) and MouseUpdateTimeMax
        // (DAT_00559bec) se resetea para hacer debounce de la UI en el frame siguiente.
        DAT_07e11d28 = 0;
        DAT_00559bec = 6;

        if (!dropMain && !dropTrade && !dropWH && !dropMix) {
            short pickType = *(short*)pPickedItem;
            int   pickLevel = (((int*)pPickedItem)[1] >> 3) & 0xF;

            // Shop NPC open + mouse over shop area → sell-with-confirm
            if (DAT_07eaa118 != 0   // ShopOpened
                && (int)DAT_083a427c >= (int)(DAT_07eaa0c8 + 15)
                && (int)DAT_083a427c <  (int)(DAT_07eaa0c8 + 195)
                && (int)DAT_083a4278 >= (int)(DAT_07eaa0cc + 50)
                && (int)DAT_083a4278 <  (int)(DAT_07eaa0cc + 370))
            {
                // Items that need confirmation (jewels, special weapons, level>4):
                bool needConfirm =
                    (pickType >= 416 && pickType <= 419) ||
                    pickType == 461 || pickType == 462 || pickType == 464 ||
                    pickType == 470 ||
                    (pickType >= 384 && pickType <= 390) ||
                    pickType == 19  || pickType == 170 || pickType == 146 ||
                    pickType == 399 || pickType == 430 || pickType == 431 ||
                    (pickLevel > 4 && pickType < 384);

                if (needConfirm) {
                    DAT_07eaa13c = 1;
                    DAT_00559f5e = (char)0xFF;   // = -1, "waiting for response"
                    ShowCheckBox(3, 536, 151);
                    return;
                }

                // Cannot sell: pet egg variants 471-474, stat-reset book 435
                if ((pickType >= 471 && pickType <= 474) || pickType == 435) {
                    return;
                }

                // Direct sell — [C1][04][33][slot], C3 (Encrypt=1).
                DAT_07eaa13c = 0;
                DAT_00559f5e = (char)0xFF;
                BYTE pkt[4];
                pkt[0] = 0x33;
                pkt[1] = (BYTE)DAT_07ea5b18;
                SendC3Packet(pkt, 2);
                return;
            }

            // 2026-07-27 FIX: tirar al PISO un item valioso NO abre un Yes/No
            // (ese cartel es de la TIENDA, para confirmar una venta). Per IDA
            // sub_4DF410 L950-964 el original sólo muestra un mensaje rojo que
            // lo prohíbe y NO suelta el item:
            //     UIChatLogWindow_AddText(byte_7EAA194, GlobalText[269], 2)
            // Nuestro port abría ShowCheckBox(2,...) → cartel de venta al soltar
            // en el suelo + item pegado al cursor esperando una respuesta.
            // Lista de tipos 1:1 con IDA (incluye 435 y el gate de excellent).
            {
                BYTE excByte = *((BYTE*)pPickedItem + 0x6b - 0x44);  // byte_7E9136B
                bool bForbidden =
                    (pickType >= 416 && pickType <= 419) ||
                    pickType == 461 || pickType == 462 || pickType == 464 ||
                    pickType == 470 ||
                    (pickType >= 384 && pickType <= 390) ||
                    pickType == 19  || pickType == 170 || pickType == 146 ||
                    pickType == 399 || pickType == 430 || pickType == 431 ||
                    pickType == 435 ||
                    (pickLevel > 4 && pickType < 384) ||
                    ((excByte & 0x3F) != 0);
                if (bForbidden) {
                    // (byte_7EAA194 en IDA es el buffer de sender; acá va vacío
                    //  como en el resto de los mensajes de sistema del archivo.)
                    UIChatLogWindow_AddText("", GlobalText[269], 2);
                    RestorePickedItemToSource();   // el item vuelve a su slot
                    return;
                }
            }

            // 2026-08-08: guard `v144` de IDA (sub_4DF410 L966-969) que faltaba
            //   v144 = 1; if (InventoryOpened && MouseX >= InventoryStartX) v144 = 0;
            // Con el inventario abierto, un click sobre su panel NUNCA tira el
            // item al suelo (la zona de abajo del grid son la barra de zen y los
            // botones). El item queda agarrado, como en el original.
            if (InventoryOpened != 0 && (int)DAT_083a427c >= (int)InventoryStartX) {
                return;
            }

            // Plain ground drop — [C1][05][23][tileX][tileY][slot], C3.
            BYTE dx, dy; GetHeroDropTile(&dx, &dy);
            BYTE pkt[4];
            pkt[0] = 0x23;
            pkt[1] = dx;   // tile X del héroe (no pixels de mouse)
            pkt[2] = dy;   // tile Y del héroe
            pkt[3] = (BYTE)DAT_07ea5b18;
            SendC3Packet(pkt, 4);
        }
    }
}
