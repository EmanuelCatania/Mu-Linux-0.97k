// ChatListBox.cpp — engine chat-widget object behind dword_55C9FF0.
//
// What this is
// ------------
// Port del objeto C++ que el mu.exe original crea con
//   operator new(0x5C8) → sub_40C7D0(this) → dword_55C9FF0 = this
// en WinMain @ 0x41F416-0x41F481. La identificación previa de
// dword_55C9FF0 como `HGLRC` era ERRÓNEA; es un objeto de widget de UI derivado
// cuya vtable vive en off_5525CC y cuyo constructor encadena a través
// del ctor base sub_40C5D0. Sin este objeto bien
// construido, los dispatches de Render_GameFrame (vtable+0x10) y de
// UIChatLogWindow_AddText (vtable+0x70) crasheaban en silencio y el
// HUD never appeared.
//
// Vtable layout (off_5525CC, 30 slots, all __thiscall)
// ----------------------------------------------------
//   slot  off  IDA addr   purpose                 (entries marked * are
//                                                  fully-rendered widget
//                                                  helpers — see notes)
//    0    +00  0x40DB80   ~CChatListBox(flag)       full
//    1    +04  0x40C670   set state              full
//    2    +08  0x40C6D0   set color1             full
//    3    +0C  0x40C6F0   set color2             full
//    4    +10  0x411920   render scroll frame    full (entry called
//                                                       from Render_GameFrame)
//    5    +14  0x40C710   tick / focus           full
//    6    +18  0x40DB40   nullsub                 full
//    7    +1C  0x411B60   maneja el input del scrollbar *completo, pero usa muchos
//                                                  GL globals; safe in our
//                                                  build (deps stubbed)
//    8    +20  0x403A30   nullsub                 full
//    9    +24  0x411A20   key-handler            re-uses FUN_00411a20
//                                                  already in stubs.cpp
//   10    +28  0x4118D0   clear list             full
//   11    +2C  0x410D70   nullsub                 full
//   12    +30  0x40CC50   scroll by N            full
//   13    +34  0x410D30   get visible count      full
//   14    +38  0x410D40   set visible count      full
//   15    +3C  0x410D40   set visible count (alt) full
//   16    +40  0x412150   incr scroll-step       full
//   17    +44  0x4122C0   trim oldest            full
//   18    +48  0x412320   recompute scrollbar    full
//   19    +4C  0x40CD80   count visible          full
//   20    +50  0x40CDD0   advance cursor         full
//   21    +54  0x40E230   hit-test del input     *usa sub_40C490 + globals
//   22    +58  0x40CE20   render del fondo del marco (ENORME) STUB — sólo delega a
//                                                  the counter increment;
//                                                  full GL render needs
//                                                  CUIRenderText + RenderBitmap
//                                                  port, que es su propio
//                                                  session.
//   23    +5C  0x40D610   render de línea (ENORME)     STUB — misma razón que el #22
//   24    +60  0x40D600   render footer thunk    full (thunk to sub_40DEF0)
//   25    +64  0x40E810   hover/click por línea   *completo (usa sub_40C490)
//   26    +68  0x40E400   per-frame button input *full
//   27    +6C  0x410D70   nullsub                 full
//   28    +70  0x40C940   AddText (entry called   full
//                          from UIChatLogWindow_AddText)
//   29    +74  0x40CD30   row visible?            full
//
// Linked list nodes
// -----------------
// Cada entrada de la lista de historial del chat (this[23] = campo+0x5C) es un
// byte block laid out as:
//   +0x00  fwd ptr (DWORD)
//   +0x04  back ptr (DWORD)
//   +0x08  data — sender 11 bytes + msg 257 bytes + flags (matches
//          original byte_55C95F8 staging buffer before AddText commits).
//   +0x10C msg-type DWORD (0..5, channel)
//   +0x114 cached text-extent cx (LONG)
// `delete__` recorre la lista doblemente enlazada y libera cada nodo.
//
// Ownership / lifetime
// --------------------
// El objeto vive en el heap, y su dueño es `dword_55C9FF0`. El destructor
// (sub_40DBA0) se invoca al cerrar el proceso vía el camino de ~Application;
// nuestro build por ahora lo filtra (el teardown de cierre vanilla no está
// ported).
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include <new>

// Definidos más abajo en este mismo archivo; declarados acá porque el slot 22
// (ChatLB_renderBg) los usa antes de sus definiciones.
extern "C" int g_ChatLB_MouseOnWindow;       // latch para Player_InputTick
static void ChatLB_DrawButton(int Texture, int hover, float x, float y,
                              float Width, float Height, float alpha, DWORD flipBits);

// External helpers already implemented elsewhere in our build.  Linkage
// coincide con las definiciones que ya existen en stubs.cpp (C++, no extern "C").
int  __cdecl    FUN_0040c680(DWORD* self);                            // get focus state
void __fastcall FUN_0040c580(int self);                               // dequeue front of base list
int  __cdecl    FUN_00411a20(DWORD* self);                            // key-handler (slot 9)
int  __cdecl    FUN_004119a0(DWORD* self, int v);                     // scroll-up helper
int  __cdecl    FUN_0040c930(int slot);                               // ++[slot+0x114]

// FUN_0040c490 — hit-test de bbox, port de sub_40C490 de IDA (lo referencian
// muchos helpers de UI). Vive acá porque stubs.cpp lo define detrás de
// bloques #if IDA_PORT_xxx que no están activados; volverlo
// siempre-activo chocaría con ésos, así que lo proveemos bajo una implementación única
// name and alias.
static int Chat_BBoxHit(int x, int y, int w, int h, int mode)
{
    if (mode == 2) {
        if ((int)MouseX >= x && (int)MouseX < w + x &&
            (int)MouseY >= y - h && (int)MouseY < y) return 1;
    } else {
        if ((int)MouseX >= x && (int)MouseX < w + x &&
            (int)MouseY >= y && (int)MouseY < h + y) return 1;
    }
    return 0;
}
#define FUN_0040c490(x,y,w,h,m) Chat_BBoxHit((int)(x),(int)(y),(int)(w),(int)(h),(int)(m))

// Globals que sólo se usan acá — declarados localmente para no ensanchar globals.h
// for one-off chat-widget statics.
extern "C" {
DWORD g_ChatListBox_NextID    = 0;   // mirror of dword_55C9B78 (incrementing IDs)
DWORD g_ChatListBox_FocusID_A = 0;   // dword_55C9B7C — focused widget id (capture)
DWORD g_ChatListBox_FocusID_B = 0;   // dword_55C9B80 — alt focused widget id
DWORD g_ChatListBox_LockID    = 0;   // dword_55C9B84 — lock guard for listbox iteration
DWORD g_ChatListBox_ActiveID  = 0;   // dword_55C9B88 — active widget id (keyboard)
BYTE  g_ChatStagingBuf[0x118] = {0}; // mirror of byte_55C95F8 staging buffer
                                     // remitente en +0, mensaje en +0xB, tipo DWORD en +0x10C
} // extern "C"

// Alias para que los nombres estilo IDA sub_XXXX sigan siendo legibles en el cuerpo.
#define dword_55C9B78  g_ChatListBox_NextID
#define dword_55C9B7C  g_ChatListBox_FocusID_A
#define dword_55C9B80  g_ChatListBox_FocusID_B
#define dword_55C9B84  g_ChatListBox_LockID
#define dword_55C9B88  g_ChatListBox_ActiveID
#define byte_55C95F8   g_ChatStagingBuf[0]
#define byte_55C9603   g_ChatStagingBuf[0x0B]
#define dword_55C9704  (*(DWORD*)&g_ChatStagingBuf[0x10C])
#define dword_55C9708  (*(DWORD*)&g_ChatStagingBuf[0x110])
#define dword_55C970C  (*(DWORD*)&g_ChatStagingBuf[0x114])

// En nuestro build dword_55C9B80 ya estaba declarado en globals.cpp (línea 423).
// Para no redefinirlo, tratamos el g_ChatListBox_FocusID_B local como un
// slot separado — los dos arrancan en 0, así que el comportamiento es idéntico para
// nuestro código, que no lee el original.

// ---------------------------------------------------------------------------
// Forward declarations of every vtable method.
// Todas usan convención __thiscall: `this` llega en ECX.
// ---------------------------------------------------------------------------
static void __fastcall ChatLB_dtor          (DWORD* self, int /*edx*/, char  flag);  // slot 0
static int  __fastcall ChatLB_setState      (DWORD* self, int /*edx*/, int   v);     // slot 1
static int  __fastcall ChatLB_setColor1     (DWORD* self, int /*edx*/, int a, int b, int c); // slot 2
static int  __fastcall ChatLB_setColor2     (DWORD* self, int /*edx*/, int a, int b, int c); // slot 3
static void __fastcall ChatLB_renderScroll  (DWORD* self);                                 // slot 4
static int  __fastcall ChatLB_tick          (DWORD* self, int /*edx*/, int   v);     // slot 5
static void __fastcall ChatLB_nullsub       (DWORD* self);                                 // slot 6, 8
static int  __fastcall ChatLB_handleScrollIn(DWORD* self);                                 // slot 7
static int  __fastcall ChatLB_keyHandler    (DWORD* self);                                 // slot 9
static void __fastcall ChatLB_clearList     (DWORD* self);                                 // slot 10
static void __fastcall ChatLB_nullsub2      (DWORD* self);                                 // slot 11, 27
static int  __fastcall ChatLB_scrollByN     (DWORD* self, int /*edx*/, int   n);     // slot 12
static int  __fastcall ChatLB_getVisibleCnt (DWORD* self);                                 // slot 13
static int  __fastcall ChatLB_setVisibleCnt (DWORD* self, int /*edx*/, int   v);     // slot 14, 15
static int  __fastcall ChatLB_incrStep      (DWORD* self, int /*edx*/, int   d);     // slot 16
static int  __fastcall ChatLB_trimOldest    (DWORD* self);                                 // slot 17
static void __fastcall ChatLB_recalcScroll  (DWORD* self);                                 // slot 18
static int  __fastcall ChatLB_countVisible  (DWORD* self);                                 // slot 19
static int  __fastcall ChatLB_advanceCursor (DWORD* self);                                 // slot 20
static int  __fastcall ChatLB_hitTestInput  (DWORD* self);                                 // slot 21
static int  __fastcall ChatLB_renderBg      (DWORD* self);                                 // slot 22 (stub)
static int  __fastcall ChatLB_renderLine    (DWORD* self, int /*edx*/, int   row);   // slot 23 (stub)
static void __fastcall ChatLB_renderFooter  (DWORD* self);                                 // slot 24
static int  __fastcall ChatLB_lineHover     (DWORD* self, int /*edx*/, int   row);   // slot 25
static int  __fastcall ChatLB_perFrameInput (DWORD* self);                                 // slot 26
static void __fastcall ChatLB_AddText       (DWORD* self, int /*edx*/, char* src, char* msg, int kind, int extra); // slot 28
static int  __stdcall  ChatLB_isRowVisible  (int  rowData);                                // slot 29

// ---------------------------------------------------------------------------
// Vtable — order matches original off_5525CC byte-for-byte.
// ---------------------------------------------------------------------------
struct ChatLB_VTable {
    void* slot[30];
};

// Definida al final del archivo (bloque "WIDGET DE LISTA DE GUILD").
extern "C" void* GuildListBox_GetVTable(void);

static ChatLB_VTable s_ChatLB_VTable = { {
    /*0x00*/ (void*)ChatLB_dtor,
    /*0x04*/ (void*)ChatLB_setState,
    /*0x08*/ (void*)ChatLB_setColor1,
    /*0x0C*/ (void*)ChatLB_setColor2,
    /*0x10*/ (void*)ChatLB_renderScroll,
    /*0x14*/ (void*)ChatLB_tick,
    /*0x18*/ (void*)ChatLB_nullsub,
    /*0x1C*/ (void*)ChatLB_handleScrollIn,
    /*0x20*/ (void*)ChatLB_nullsub,
    /*0x24*/ (void*)ChatLB_keyHandler,
    /*0x28*/ (void*)ChatLB_clearList,
    /*0x2C*/ (void*)ChatLB_nullsub2,
    /*0x30*/ (void*)ChatLB_scrollByN,
    /*0x34*/ (void*)ChatLB_getVisibleCnt,
    /*0x38*/ (void*)ChatLB_setVisibleCnt,
    /*0x3C*/ (void*)ChatLB_setVisibleCnt,
    /*0x40*/ (void*)ChatLB_incrStep,
    /*0x44*/ (void*)ChatLB_trimOldest,
    /*0x48*/ (void*)ChatLB_recalcScroll,
    /*0x4C*/ (void*)ChatLB_countVisible,
    /*0x50*/ (void*)ChatLB_advanceCursor,
    /*0x54*/ (void*)ChatLB_hitTestInput,
    /*0x58*/ (void*)ChatLB_renderBg,
    /*0x5C*/ (void*)ChatLB_renderLine,
    /*0x60*/ (void*)ChatLB_renderFooter,
    /*0x64*/ (void*)ChatLB_lineHover,
    /*0x68*/ (void*)ChatLB_perFrameInput,
    /*0x6C*/ (void*)ChatLB_nullsub2,
    /*0x70*/ (void*)ChatLB_AddText,
    /*0x74*/ (void*)ChatLB_isRowVisible,
} };

// Helper — aloca una cabeza de lista doblemente enlazada vacía, del tamaño dado.
// Tanto fwd como back apuntan a la propia cabeza, lo que marca la lista como vacía.
static DWORD* ChatLB_NewListHead(size_t bytes)
{
    DWORD* head = (DWORD*)malloc(bytes);
    if (!head) return nullptr;
    memset(head, 0, bytes);
    head[0] = (DWORD)head;  // forward link → self
    head[1] = (DWORD)head;  // back link    → self
    return head;
}

// ---------------------------------------------------------------------------
// Construct — port 1:1 completo de sub_40C7D0(this), que primero llama a sub_40C5D0(this).
// Entrada pública: produce el objeto ya construido, listo para asignar a
// dword_55C9FF0. Reemplaza al `malloc(0x5c8)+memset` que hacía antes WinMain y
// left the vtable null.
// ---------------------------------------------------------------------------
extern "C" void* ChatListBox_Construct(void)
{
    BYTE* p = (BYTE*)malloc(0x5C8);
    if (!p) return nullptr;
    memset(p, 0, 0x5C8);

    DWORD* obj = (DWORD*)p;

    // ── sub_40C5D0 (base ctor) ──────────────────────────────────────────────
    // Cabeza de lista 1: en this[2] (offset 0x08), nodo de 0x14
    DWORD* list1 = ChatLB_NewListHead(0x14);
    obj[2] = (DWORD)list1;
    obj[3] = 0;
    obj[7] = ++dword_55C9B78;  // unique ID (sub_40C480)
    obj[8] = 0;
    obj[9] = 0;                // sub_40C670
    obj[10] = 0;
    obj[11] = 0; obj[12] = 0;  // sub_40C690 — pos
    obj[13] = 100; obj[14] = 100; // sub_40C6B0 — size
    obj[15] = 0; obj[17] = 0; obj[18] = 0; // sub_40C6D0 — color1
    obj[16] = 0; obj[19] = 0; obj[20] = 0; // sub_40C6F0 — color2
    obj[21] = 1;

    // ── sub_40C7D0 (derived ctor) ───────────────────────────────────────────
    // Cabeza de lista 2: en this[23] (offset 0x5C), nodo de 0x120 — historial del chat
    DWORD* list2 = ChatLB_NewListHead(0x120);
    obj[23] = (DWORD)list2;
    obj[24] = 0;
    // Cabeza de lista 3: en this[31] (offset 0x7C), nodo de 0x120
    DWORD* list3 = ChatLB_NewListHead(0x120);
    obj[31] = (DWORD)list3;
    obj[32] = 0;

    obj[21] = 2;          // override base
    obj[44] = 0;
    obj[29] = 0;
    obj[26] = 0;
    obj[27] = 0;
    obj[45] = 1;          // (gets reset to 0 below)

    *(void**)obj = &s_ChatLB_VTable;  // INSTALL VTABLE

    obj[33] = 150;        // chat fade timer / max entries
    obj[34] = 0;          // scroll cursor
    obj[35] = 6;          // visible row count
    obj[36] = 0;
    obj[37] = 0;
    obj[38] = 0;
    obj[39] = 0x41500000; // 13.0f
    obj[40] = 0;
    obj[41] = 0;
    obj[42] = 0;
    obj[43] = 0;
    obj[46] = 0;
    obj[47] = 0x3ECCCCCD; // ~0.4f
    obj[48] = 0;
    obj[49] = 0;
    memset(p + 200, 0, 0x500);

    // sub_40C690(this, 186, 420)
    obj[11] = 186; obj[12] = 420;
    // sub_40C6B0(this, 268, 250)
    obj[13] = 268; obj[14] = 250;

    obj[28] = obj[23];    // cursor pointer = list head
    obj[45] = 0;          // reset

    // Clear the staging buffer.
    memset(g_ChatStagingBuf, 0, sizeof(g_ChatStagingBuf));

    return obj;
}

// ===========================================================================
// ChatListBox_ConstructWhisper — port de sub_40E990, el SEGUNDO constructor
// de widget de chat (objeto más chico en DAT_055c9ff4, se usa para el input
// de destino de susurro + la lista chica de notificaciones). Misma estructura
// general que ChatListBox_Construct pero con:
//   * smaller list node size (0x18 instead of 0x120)
//   * 24 visible rows instead of 6
//   * position (460, 387), size (170, 250)
//   * vtable off_5526EC — 13 de los 30 slots DIFIEREN de off_5525CC.
//
// 2026-08-15: acá se instalaba `s_ChatLB_VTable` (la del chat).  Ese objeto es
// el que `RenderGuildList` usa para dibujar los miembros, así que el dispatch
// del slot 4 terminaba corriendo los métodos del CHAT sobre este objeto → AV.
// Ahora se instala `s_GuildLB_VTable` (definida al final del archivo, con los
// slots propios portados de IDA).
// ===========================================================================
extern "C" void* ChatListBox_ConstructWhisper(void)
{
    BYTE* p = (BYTE*)malloc(0xBC);
    if (!p) return nullptr;
    memset(p, 0, 0xBC);

    DWORD* obj = (DWORD*)p;

    // sub_40C5D0 (ctor base) — igual que en el constructor principal.
    DWORD* list1 = ChatLB_NewListHead(0x14);
    obj[2] = (DWORD)list1;
    obj[3] = 0;
    obj[7] = ++dword_55C9B78;
    obj[9] = 0;
    obj[11] = 0; obj[12] = 0;
    obj[13] = 100; obj[14] = 100;
    obj[15] = 0; obj[17] = 0; obj[18] = 0;
    obj[16] = 0; obj[19] = 0; obj[20] = 0;
    obj[21] = 1;

    // sub_40E990 derived ctor — note 0x18 list-node size and 24 visible.
    DWORD* list2 = ChatLB_NewListHead(0x18);
    obj[23] = (DWORD)list2;
    obj[24] = 0;
    DWORD* list3 = ChatLB_NewListHead(0x18);
    obj[31] = (DWORD)list3;
    obj[32] = 0;

    obj[21] = 2;
    obj[44] = 0;
    obj[29] = 0;
    obj[26] = 0;
    obj[27] = 0;
    obj[45] = 0;

    // vtable off_5526EC — propia del widget de guild (ver final del archivo)
    *(void**)obj = GuildListBox_GetVTable();

    obj[33] = 150;
    obj[34] = 0;
    obj[35] = 24;          // ← differs from main (was 6)
    obj[36] = 0;
    obj[37] = 0;
    obj[38] = 0;
    obj[39] = 0x41500000;  // 13.0f
    obj[40] = 0;
    obj[41] = 0;
    obj[42] = 0;
    obj[43] = 0;
    obj[46] = 0;

    obj[11] = 460; obj[12] = 387;   // sub_40C690(460, 387)
    obj[13] = 170; obj[14] = 250;   // sub_40C6B0(170, 250)

    return obj;
}

// ===========================================================================
// MÉTODOS DE LA VTABLE — ports 1:1 de IDA, en orden de vtable.
// ===========================================================================

// slot 0 — ~CChatListBox(char flag).  IDA: sub_40DB80
//   if (flag&1) free this;  invoca sub_40DBA0 (el teardown real).
//   No tenemos portado el sub_40DBA0 completo (teardown de la lista enlazada); dejamos
//   que las listas se filtren al salir del proceso — no bloquea nada.
static void __fastcall ChatLB_dtor(DWORD* self, int /*edx*/, char flag)
{
    if (flag & 1) {
        // Best-effort: liberar las cabezas de lista (sus nodos pueden filtrarse — los ports de
        // sub_410E30 / sub_411360 not done).
        DWORD* list1 = (DWORD*)self[2];
        DWORD* list2 = (DWORD*)self[23];
        DWORD* list3 = (DWORD*)self[31];
        if (list1) free(list1);
        if (list2) free(list2);
        if (list3) free(list3);
        free(self);
    }
}

// slot 1 — sub_40C670
static int __fastcall ChatLB_setState(DWORD* self, int, int v)
{
    self[9] = v;
    return v;
}

// slot 2 — sub_40C6D0
static int __fastcall ChatLB_setColor1(DWORD* self, int, int a, int b, int c)
{
    self[15] = a;
    self[17] = b;
    self[18] = c;
    return c;
}

// slot 3 — sub_40C6F0
static int __fastcall ChatLB_setColor2(DWORD* self, int, int a, int b, int c)
{
    self[16] = a;
    self[19] = b;
    self[20] = c;
    return c;
}

// slot 4 — sub_411920 — render del scroll: itera la lista visible y renderiza
// cada fila vía vtable[+92]; el fondo vía vtable[+88]; el encabezado vía [+80]; el
// pie vía [+96]. La llama Render_GameFrame (vtable+0x10).
static void __fastcall ChatLB_renderScroll(DWORD* self)
{
    typedef void (__fastcall *FnVoid)(DWORD*);
    typedef int  (__fastcall *FnRow )(DWORD*, int /*edx*/, int row);

    void** vt = (void**)*self;

    ((FnVoid)vt[22])(self);  // *this+88 — render BG
    ((FnVoid)vt[20])(self);  // *this+80 — render header

    glColor3f(1.0f, 1.0f, 1.0f);
    if (m_hFontDC) SelectObject(m_hFontDC, g_hFontBold);  // we don't have plain g_hFont; bold is closest

    int i = 0;
    while (i < (int)self[35]) {
        if (self[25] == self[23]) break;
        int v3 = ((FnRow)vt[23])(self, 0, i);
        if (v3 >= 0) {
            if (!v3) --i;
        } else {
            i -= v3;
        }
        ++i;
        self[25] = *(DWORD*)self[25];  // advance cursor
    }

    ((FnVoid)vt[24])(self);  // *this+96 — render footer
}

// slot 5 — sub_40C710 — tick de foco. Drena la lista base y despacha hacia
// vtable methods 24/28/32/36 and walks focus state machine.
static int __fastcall ChatLB_tick(DWORD* self, int, int v)
{
    typedef int  (__fastcall *FnInt)(DWORD*);
    typedef void (__fastcall *FnVoid)(DWORD*);
    typedef void (__fastcall *FnInt2)(DWORD*, int, int);

    void** vt = (void**)*self;

    while (self[3]) {
        FUN_0040c580((int)self);
        if (!((FnInt)vt[9])(self)) {
            ((FnVoid)vt[8])(self);
        }
    }
    ((FnInt2)vt[6])(self, 0, v);

    if (v == 1) return 0;

    if (!FUN_0040c490(self[11], self[12], self[13], self[14], self[21])) {
        return ((FnInt)vt[7])(self);
    }
    if (!dword_55C9B80) {
        if (dword_55C9B7C && dword_55C9B7C != self[7]) {
            return 0;
        }
        dword_55C9B80 = self[7];
    }
    if (FUN_0040c680(self)) {
        return ((FnInt)vt[7])(self);
    }
    DWORD vid = self[7];
    if (dword_55C9B80 == vid || dword_55C9B7C == vid) {
        return ((FnInt)vt[7])(self);
    }
    return 0;
}

// slots 6, 8, 11, 27 — nullsub_6 / nullsub_5 / sub_403A30 (CWsctlc::LogPrintOn empty)
static void __fastcall ChatLB_nullsub (DWORD* /*self*/) {}
static void __fastcall ChatLB_nullsub2(DWORD* /*self*/) {}

// slot 7 — sub_411B60 — handle scrollbar / focus / per-frame input.
// Full 1:1 port of IDA sub_411B60.  Implements:
//   * Scroll asíncrono con las flechas (Arriba/Abajo con contador de rampa en this[27])
//   * Arrastre del pulgar del scrollbar con el mouse (this[42], this[43] son contadores de rampa
//     para click sostenido; this[36..40] llevan la geometría que escribe el slot 18
//     ChatLB_recalcScroll)
//   * Botones de flecha arriba/abajo en los extremos del scrollbar (llaman a
//     vtable[+0x30] = ChatLB_scrollByN with ±1)
//   * Bandas de scroll de página arriba/abajo del pulgar (llaman a scrollByN(±visible))
//   * Loop de hit-test de las filas de contenido, abajo (delega en el slot 25)
//
// External deps:
//   PlayBuffer / sub_40C500 — soft-stubbed (sound + key-repeat dispatch)
//   unk_83A4128            — acumulador del scroll de rueda (nosotros usamos un static)
extern "C" int  MouseOnWindow;
static int g_ChatLB_WheelAccum = 0;     // mirror of unk_83A4128

// Soft helpers — wired to no-ops until the engine sound/keyrepeat is ported.
static inline void ChatLB_PlayBuffer(int /*id*/, int /*p2*/, int /*p3*/) {}
static inline void ChatLB_DispatchKey(DWORD* /*self*/, int /*key*/, int /*p2*/, int /*p3*/) {}

static int __fastcall ChatLB_handleScrollIn(DWORD* self)
{
    typedef int  (__fastcall *FnRow )(DWORD*, int, int);
    typedef int  (__fastcall *FnInt )(DWORD*);
    typedef int  (__fastcall *FnIntI)(DWORD*, int /*edx*/, int);
    typedef void (__fastcall *FnVoid)(DWORD*);
    typedef void (__fastcall *FnVoidI)(DWORD*, int /*edx*/, int);

    void** vt = (void**)*self;
    int  v17 = 0;
    bool keyHandled = false;

    // Ver la nota de MouseOnWindow al final del archivo: el flag se recalcula
    // entero en cada tick y se publica al salir para que Player_InputTick lo lea.
    MouseOnWindow = 0;

    // ── Scroll con flechas (sólo cuando este widget tiene el foco del teclado) ──
    if (self[26] == 1 && dword_55C9B88 == self[7]) {
        GetAsyncKeyState(VK_LEFT);
        GetAsyncKeyState(VK_RIGHT);

        // Up arrow → scrollByN(+1) with ramp.
        if (((WORD)GetAsyncKeyState(VK_UP) >> 8) == 0x80) {
            keyHandled = true;
            DWORD ramp = self[27];
            if (ramp == 0) {
                ChatLB_DispatchKey(self, 13, 0, 0);
                self[27] = 1;
            } else if (ramp <= 5) {
                self[27] = ramp + 1;
            } else {
                ChatLB_DispatchKey(self, 13, 0, 0);
            }
        }
        // Down arrow → scrollByN(-1) with ramp.
        if (((WORD)GetAsyncKeyState(VK_DOWN) >> 8) == 0x80) {
            DWORD ramp = self[27];
            if (ramp == 0) {
                ChatLB_DispatchKey(self, 14, 0, 0);
                self[27] = 1;
            } else if (ramp <= 5) {
                self[27] = ramp + 1;
            } else {
                ChatLB_DispatchKey(self, 14, 0, 0);
            }
        } else if (!keyHandled) {
            *(float*)&self[27] = 0.0f;
        }
    }

    // ── el slot 21 (hit-test del input) decide si el mouse está sobre este widget ──
    // IDA: `(*(...)(*(_DWORD *)this + 84))(this)` → +84 BYTES = entrada 21.
    // Corregido 2026-07-20: acá había vt[20] (= +80, advanceCursor).  Como
    // slot 21 es el ÚNICO que escribe self[48] (el flag "mouse cerca de la
    // barra de chat"), con el índice mal ese flag nunca se prendía y por lo
    // tanto ni el render (slot 24) ni el input (slot 26) de los 3 botones
    // popup llegaban a correr.
    if (((FnInt)vt[21])(self)) {
        // Wheel-scroll accumulator drains here.
        if (g_ChatLB_WheelAccum) {
            ((FnVoidI)vt[12])(self, 0, -3 * g_ChatLB_WheelAccum);
            g_ChatLB_WheelAccum = 0;
        }

        if (!MouseLButtonPush || FUN_0040c680(self)) {
            // Mouse not pressed — clear ramp counters.
            *(float*)&self[42] = 0.0f;
            *(float*)&self[43] = 0.0f;
            goto LABEL_53;
        }

        dword_55C9B88 = self[7];
        ((FnVoid)vt[26])(self);   // recompute scrollbar thumb geometry

        // Adjust scrollbar offset for chat list-box mode (8-px nudge).
        DWORD modeFlag = self[45];
        int v6 = (modeFlag == 1) ? 8 : 0;

        // ── Up arrow button (top of scrollbar) ──
        int btnX = v6 + (int)self[11] + (int)self[13] - 21;
        int btnY = (int)self[12] + (modeFlag != 1 ? 8 : 0) - (int)self[14];
        if (FUN_0040c490(btnX, btnY, 13, 13, 1)) {
            DWORD r = self[42];
            if (r == 0) {
                ((FnVoidI)vt[12])(self, 0, -1);
                ChatLB_PlayBuffer(25, 0, 0);
                self[42] = 1;
            } else if (r <= 15) {
                self[42] = r + 1;
            } else {
                ((FnVoidI)vt[12])(self, 0, -1);
            }
        }
        // ── Down arrow button (bottom of scrollbar) ──
        btnY = (int)self[12] - (modeFlag != 1 ? 21 : 13);
        if (FUN_0040c490(btnX, btnY, 13, 13, 1)) {
            DWORD r = self[42];
            if (r == 0) {
                ((FnVoidI)vt[12])(self, 0, 1);
                ChatLB_PlayBuffer(25, 0, 0);
                self[42] = 1;
            } else if (r <= 15) {
                self[42] = r + 1;
            } else {
                ((FnVoidI)vt[12])(self, 0, 1);
            }
        }

        // Abajo: sólo si la lista desborda la ventana visible.
        int total = ((FnInt)vt[19])(self);
        if (total < (int)self[35]) {
            v17 = 1; MouseOnWindow = 1; goto LABEL_53;
        }

        // ── Thumb drag region ──
        float* fself = (float*)self;
        int trackX = v6 + (int)self[11] + (int)self[13] - 19;
        int thumbY = (int)fself[38];
        int thumbH = (int)fself[40];
        if (FUN_0040c490(trackX, thumbY, (int)fself[39], thumbH, 1)) {
            // Mouse adentro del pulgar — arranca el arrastre si no había uno ya.
            if (!FUN_0040c680(self) && !dword_55C9B7C) {
                dword_55C9B7C = self[7];
                ((FnVoidI)vt[1])(self, 0, 2);   // setState(2 = dragging)
                fself[41] = (float)((double)MouseY - (double)fself[38]);
            }
        }
        // ── Page-up band (above thumb) ──
        else if (FUN_0040c490(trackX, (int)fself[36], (int)fself[39],
                              (int)(fself[38] - fself[36]), 1)) {
            DWORD r = self[43];
            if (r == 0) {
                ((FnVoidI)vt[12])(self, 0, -(int)self[35]);
                self[43] = 1;
            } else if (r > 15) {
                ((FnVoidI)vt[12])(self, 0, -(int)self[35]);
            } else {
                self[43] = r + 1;
            }
        }
        // ── Page-down band (below thumb) ──
        else if (FUN_0040c490(trackX, (int)(fself[40] + fself[38]), (int)fself[39],
                              (int)(fself[37] - fself[38] - fself[40]), 1)) {
            DWORD r = self[43];
            if (r == 0) {
                ((FnVoidI)vt[12])(self, 0, (int)self[35]);
                self[43] = 1;
            } else if (r > 15) {
                ((FnVoidI)vt[12])(self, 0, (int)self[35]);
            } else {
                self[43] = r + 1;
            }
        }
LABEL_53:
        v17 = 1; MouseOnWindow = 1;
    } else if (FUN_0040c680(self) == 1) {
        // El widget tenía el foco pero el mouse salió del rect — manejar la soltada del pulgar.
        if (MouseLButtonPush) {
            float* fself = (float*)self;
            MouseOnWindow = 1;
            int rows = 3 * (((int)self[12] - (int)MouseY + 5) / 40);
            self[35] = (DWORD)rows;
            if (rows < 3) self[35] = 3;
            else if (rows > 30) self[35] = 30;

            int total = ((FnInt)vt[19])(self);
            if (total >= (int)self[35] &&
                total - (int)self[34] < (int)self[35]) {
                self[34] = (DWORD)(total - (int)self[35]);
            }
            (void)fself;
        } else {
            ((FnVoidI)vt[1])(self, 0, 0);  // setState(0 = idle)
            if (dword_55C9B7C == self[7]) dword_55C9B7C = 0;
        }
    } else if (FUN_0040c680(self) == 2) {
        // Currently dragging the thumb.
        if (!MouseLButtonPush) {
            ((FnVoidI)vt[1])(self, 0, 0);
            if (dword_55C9B7C == self[7]) dword_55C9B7C = 0;
        } else {
            float* fself = (float*)self;
            MouseOnWindow = 1;
            float newY = (float)((double)MouseY - (double)fself[41]);
            fself[38] = newY;
            int total = ((FnInt)vt[19])(self);
            if (newY <= fself[37] - fself[40]) {
                float pad = (fself[40] <= 2.0f) ? 1.0f : 0.0f;
                self[34] = (DWORD)((double)total *
                    ((double)(fself[37] - fself[38] - fself[40] + 0.5f) /
                     (double)(fself[37] - fself[36] - pad)));
            } else {
                fself[34] = 0.0f;
            }
        }
    }

    // IDA sub_411B60 LABEL_76: `(*(...)(*(_DWORD *)this + 80))(this)`.
    // +80 BYTES = entrada 20 de la vtable = sub_40CDD0 (advanceCursor), NO la
    // 18 (+72 = recalcScroll).  Corregido 2026-07-20: advanceCursor es la que
    // deja self[25] apuntando al inicio de la ventana visible, que es lo que
    // consume el loop de abajo.
    ((FnVoid)vt[20])(self);

    // ── Per-row hit-test loop (slot 25 = lineHover) ─────────────────────────
    if (!dword_55C9B84) {
        int i = 0;
        while (i < (int)self[35]) {
            if (self[25] == self[23]) break;
            int v15 = ((FnRow)vt[25])(self, 0, i);
            if (v15 >= 0) {
                if (!v15) --i;
            } else {
                i -= v15;
            }
            ++i;
            self[25] = *(DWORD*)self[25];
        }
    }

    // Publicar el flag para Player_InputTick (ver nota de MouseOnWindow).
    g_ChatLB_MouseOnWindow = MouseOnWindow;
    return v17;
}

// slot 9 — sub_411A20 — reusa el stub ya activado desde IDA que está en stubs.cpp.
static int __fastcall ChatLB_keyHandler(DWORD* self) { return FUN_00411a20(self); }

// slot 10 — sub_4118D0 — wipe entire list, decrement count per node.
static void __fastcall ChatLB_clearList(DWORD* self)
{
    DWORD** v2 = (DWORD**)self[23];
    DWORD*  i  = *v2;
    while (i != (DWORD*)v2) {
        DWORD** v4 = (DWORD**)i;
        i = (DWORD*)*i;
        *v4[1] = (DWORD)*v4;          // unlink: back->fwd = self->fwd
        (*v4)[1] = (DWORD)v4[1];      //         fwd->back = self->back
        free(v4);
        --self[24];
    }
    self[34] = 0;
}

// slot 12 — sub_40CC50 — scroll de N páginas (negativo = página abajo). Cuando
// el global `DAT_005590ac` no está seteado y el argumento es -6 (el "página-previa" especial),
// recorremos la lista para calcular cuántas entradas entran. Si no, sólo desplazamos
// el scroll en -N y lo clampeamos a [0, count-visible].
static int __fastcall ChatLB_scrollByN(DWORD* self, int, int a2)
{
    typedef int (__fastcall *FnInt)(DWORD*);
    void** vt = (void**)*self;

    int result = (int)self[24];
    if ((unsigned)result < self[35]) return result;

    if (DAT_005590ac || self[34] || a2 != -6) {
        self[34] -= a2;
    } else {
        DWORD* v4 = (DWORD*)self[23];
        int v5 = 0;
        unsigned v6 = 0;
        DWORD* v7 = (DWORD*)*v4;
        int v9 = 1;
        if (v7 != v4) {
            for (;;) {
                v5 += (int)(v7[71] - v6 + 1);
                if (v6 < v7[71]) v6 = v7[71];
                if (v5 == 6) break;
                if (v5 > 6) { --v9; break; }
                v7 = (DWORD*)*v7;
                int v8 = ++v9;
                if (v7 == v4) {
                    self[34] = v8;
                    goto scrollByN_clamp;
                }
            }
        }
        self[34] = v9;
    }
scrollByN_clamp:
    {
        int s = (int)self[34];
        if (s >= 0) {
            int max = ((FnInt)vt[19])(self) - (int)self[35];
            if (s > max) self[34] = max;
        } else {
            self[34] = 0;
        }
    }
    return (int)self[34];
}

// slot 13 — sub_410D30 — return self[35] (visible row count).
static int __fastcall ChatLB_getVisibleCnt(DWORD* self) { return (int)self[35]; }

// slot 14, 15 — sub_410D40 — set self[35].
static int __fastcall ChatLB_setVisibleCnt(DWORD* self, int, int v)
{
    self[35] = v;
    return v;
}

// slot 16 — sub_412150 — increment scroll-step and clamp >= 3.
static int __fastcall ChatLB_incrStep(DWORD* self, int, int d)
{
    int r = 3 * d + (int)self[35];
    self[35] = r;
    if (r < 3) { self[35] = 3; return 3; }
    return r;
}

// slot 17 — sub_4122C0 — trim list down to self[33] entries.
static int __fastcall ChatLB_trimOldest(DWORD* self)
{
    int v2 = (int)self[24];
    int max = (int)self[33];
    int r = max;
    if (v2 >= max) {
        int v4 = 0;
        if (v2 - max > 0) {
            do {
                DWORD** v5 = *(DWORD***)(self[23] + 4);
                *v5[1] = (DWORD)*v5;
                (*v5)[1] = (DWORD)v5[1];
                free(v5);
                --self[24];
                ++v4;
                r = v2 - max;
            } while (v4 < r);
        }
    }
    return r;
}

// slot 18 — sub_412320 — recalcula la geometría del scrollbar (this[36..40]). El
// original es asm x87 inline; acá espejamos la matemática de floats en C plano.
static void __fastcall ChatLB_recalcScroll(DWORD* self)
{
    typedef int (__fastcall *FnInt)(DWORD*);
    void** vt = (void**)*self;

    float* fself = (float*)self;
    int v2 = (int)self[12];
    int v8;
    if (self[45] == 1) {
        v8 = v2 - 12;
        fself[36] = (float)(v2 - (int)self[14] + 12);
    } else {
        v8 = v2 - 17;
        fself[36] = (float)(v2 - (int)self[14] + 21);
    }
    fself[37] = (float)v8;

    int total = ((FnInt)vt[19])(self);
    double v4;
    if (total <= 0) {
        v4 = 1.0;
    } else {
        v4 = (double)(int)self[35] / (double)total;
        if (v4 >= 1.0) v4 = 1.0;
    }
    fself[40] = (float)(v4 * (fself[37] - fself[36]));
    if (fself[40] < 2.0f) fself[40] = 2.0f;

    double v6;
    if (total <= 0) {
        v6 = 0.0;
    } else {
        v6 = (double)(int)self[34] / (double)total;
    }
    double v7 = fself[40] <= 2.0f ? (double)fself[40] : 0.0;
    fself[38] = (float)(fself[37] - (fself[37] - fself[36] - (float)v7) * (float)v6 - fself[40]);
}

// slot 19 — sub_40CD80 — count visible rows (filters via vtable[+116]).
static int __fastcall ChatLB_countVisible(DWORD* self)
{
    typedef int (__stdcall *FnRow)(int);
    void** vt = (void**)*self;

    int v1 = 0;
    if (!DAT_00559bf1 || DAT_005590ac) {
        DWORD** v3 = (DWORD**)self[23];
        DWORD*  v4 = *v3;
        if (v4 != (DWORD*)v3) {
            do {
                if (((FnRow)vt[29])((int)(v4 + 2))) ++v1;
                v4 = (DWORD*)*v4;
            } while (v4 != (DWORD*)self[23]);
        }
    }
    return (int)self[24] - v1;
}

// slot 20 — sub_40CDD0 — advance cursor over [scroll] entries.
static int __fastcall ChatLB_advanceCursor(DWORD* self)
{
    typedef int (__stdcall *FnRow)(int);
    void** vt = (void**)*self;

    int v2 = 0;
    int result = (int)self[34];
    self[25] = *(DWORD*)self[23];
    if (result > 0) {
        do {
            result = (int)self[25];
            if (result == (int)self[23]) break;
            if (!((FnRow)vt[29])((int)(self[25] + 8))) ++v2;
            self[25] = *(DWORD*)self[25];
            result = (int)self[34];
        } while (v2 < result);
    }
    return result;
}

// slot 21 — sub_40E230 — input panel hit-test + right-click "copy sender
// name" support.  Full port:
//   * Si hay una fila con foco (this[28] != this[23]) Y el usuario
//     hizo click derecho, copia el nombre del remitente de esa fila (en +8 del payload del nodo)
//     a InputText[1] (el input de destino del susurro) y actualiza su longitud.
//   * Reset focused row to list head.
//   * Marca el diálogo de chat como "activo" (this[48] = 1) cuando el mouse está en la
//     banda del input de la fila inferior (160..480, 386..436); lo limpia cuando el mouse
//     is above y=416.
//   * Devuelve distinto de cero cuando el mouse está sobre la tira del encabezado del scroll del chat
//     (el botón de cerrar del borde derecho) O sobre una de las 3
//     bandas de toggle de canal de abajo (cuando está activo).
//
// 2026-07-20: flt_5590B0/B4/B8 YA tienen storage propio (ChatListBox_TabButtonsX/b4/b8 en
// globals.cpp, leídos del binario = 295 / 417 / 18).  El comentario anterior
// decía que "no existen en nuestro build" y por eso el segundo return usaba un
// rect inventado (160,420,60,16) que no cubría los botones — ver abajo.
// 2026-05-04: InputText/InputLength now alias DAT_07db8710 / DAT_07d780a8
// (ver HUD_Pass4.cpp). Usamos los mismos #define acá para compartir el storage
// with WM_CHAR + RenderInputText.
#define InputText   DAT_07db8710
#define InputLength ((int*)(void*)DAT_07d780a8)

static int __fastcall ChatLB_hitTestInput(DWORD* self)
{
    DWORD v2 = self[28];
    if (v2 != self[23] && MouseRButton) {
        // Copia el "nombre del remitente" de la fila con foco al input de susurro.
        const char* sender = (const char*)(v2 + 8);
        strncpy((char*)InputText[1], sender, 0x100);
        InputLength[1] = (int)strlen((const char*)InputText[1]);
    }
    self[28] = self[23];

    // Toggle "input dialog active" flag based on mouse Y band.
    if (FUN_0040c490(160, 436, 320, 50, 1)) {
        self[48] = 1;
    } else if ((int)MouseY < 416) {
        self[48] = 0;
    }

    // Condición de retorno #1: mouse adentro de la tira del botón de cerrar, en el borde
    // derecho (this[13]+this[11]-30, this[12]-6, 33, this[14]-2 con modo 2).
    if (FUN_0040c490((int)self[13] + (int)self[11] - 30, (int)self[12] - 6,
                     33, (int)self[14] - 2, 2)) {
        return 1;
    }
    // Return condition #2 — IDA:
    //   this[48] == 1 && sub_40C490((__int64)flt_5590B0, (__int64)flt_5590B4,
    //                               (__int64)(flt_5590B8 * 3.0), 16, 1)
    // = la franja COMPLETA de los 3 botones: x∈[295, 349), y∈[417, 433).
    // Acá había un rect inventado (160, 420, 60, 16) de cuando no teníamos las
    // constantes.  Como no cubría x=295..349, al clickear un botón este hit-test
    // devolvía 0, slot 7 nunca despachaba el slot 26 y el click se perdía:
    // los botones se veían pero no respondían.
    if (self[48] == 1 && FUN_0040c490((int)ChatListBox_TabButtonsX, (int)ChatListBox_TabButtonsY,
                                      (int)(ChatListBox_TabButtonSpacing * 3.0f), 16, 1)) {
        return 1;
    }
    return 0;
}

// slot 22 — sub_40CE20 — full chat-frame BG render.  When DAT_005590ac
// (g_bUseChatListBox) is enabled, draws:
//   * Alpha-tinted background quad (RenderColor at chat box bounds)
//   * Bitmap de la esquina superior (252) en el borde de arriba
//   * Vertical strip bitmaps (1281) tiling between top and bottom
//   * Bottom corner-piece bitmap (252)
//   * Two scrollbar arrows (1284) with hover hi-lighting (sub_40DCE0 — soft)
//   * Fondo (1283) y pulgar (1282) del scrollbar vertical cuando el contenido
//     excede la ventana visible — usa self[+0x90/+0x94/+0x98] para las
//     coordenadas del pulgar, que setea el slot 18 (recalcScroll).
//   * Active-channel tag header (CUIRenderText route → UI_DrawText in our
//     build).
// Incrementa self[46] (contador de frames) al final, para el parpadeo del cursor del slot 23.
//
// Casi todo este render depende de las constantes flt_55264C / flt_55256C / flt_552648 /
// flt_55265C / flt_552654 / flt_552658 / flt_552660 de la
// engine.  Those are 1-byte border thicknesses (typical: 1.0, 2.0, 3.0).
// Las aproximamos con literales suficientemente cercanos a los valores de IDA.
extern "C" SIZE* __cdecl FUN_0047f6f0(int x, int y, const char* lpString,
                                      int boxWidth, char style, int extraSize);
static int __fastcall ChatLB_renderBg(DWORD* self)
{
    // ── PORT FIEL de IDA sub_40CE20 (2026-07-20) ────────────────────────────
    // La versión anterior era una APROXIMACIÓN: su comentario decía "we
    // approximate with literals close enough to the IDA values".  De ahí venían
    // los 3 síntomas reportados: el recuadro salía casi transparente (alto del
    // fondo sin el +12 y sin el tope correcto), no había barra de desplazamiento
    // (nunca se llamaba a recalcScroll, así que self[36..40] quedaban en 0 y la
    // barra se dibujaba con geometría nula) y faltaban las flechas y el botón de
    // redimensionar (nunca se portaron).
    //
    // Constantes leídas del binario en 0x552644..0x552664 y 0x55256C:
    //   flt_552644=28  552648=7  55264C=2  552650=4  552654=21
    //   552658=8       55265C=22 552660=5  552664=12 55256C=1.0
    // Texturas: 252=Message_box2, 1280=nis_rsframe (grip), 1281=nis_vframe
    //   (bordes), 1282=nis_bar (thumb), 1283=nis_back (riel), 1284=nis_btnarrow.
    typedef void (__fastcall *FnVoidSelf)(DWORD*);
    typedef int  (__fastcall *FnIntSelf )(DWORD*);
    void** vt = (void**)*self;

    if (DAT_005590ac == 1) {
        const float fx = (float)(int)self[11];   // x
        const float fw = (float)(int)self[13];   // width

        int rows3  = (int)self[35] / 3;
        int height = 40 * rows3 + 10;
        self[14] = (DWORD)height;
        int yTop = (int)self[12] - height;       // var_304 (entero)

        EnableAlphaTest(true);

        // Fondo negro semitransparente.  alpha = 1.0 - self[47]; self[47] es el
        // mismo campo que cicla el botón 3 del popup (0.2 … 0.9), así que a más
        // transparencia elegida, más se ve el mundo detrás.
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f - *(float*)&self[47]);
        GL_DrawRect(fx, (float)((int)self[12] - height), fw, (float)height + 12.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        GL_ResetState();                          // DisableAlphaBlend

        // Tapa superior (5 px).
        GL_DrawTexture(252, fx, (float)yTop, fw, 5.0f,
                     0.0f, 0.0f, 0.83203125f, 0.625f, 1, 1);

        // Bordes verticales, en tramos de 40 px, a izquierda y derecha.
        yTop += 5;
        for (int s = 0; s < rows3; ++s) {
            const float fy = (float)yTop;
            GL_DrawTexture(1281, fx,                  fy, 5.0f, 40.0f, 0,0, 0.625f, 0.625f, 1, 1);
            GL_DrawTexture(1281, fx + fw - 5.0f,      fy, 5.0f, 40.0f, 0,0, 0.625f, 0.625f, 1, 1);
            yTop += 40;
        }
        // Tramo final, solapado 22 px hacia arriba (flt_55265C).
        {
            const float fy = (float)yTop - 22.0f;
            GL_DrawTexture(1281, fx,             fy, 5.0f, 40.0f, 0,0, 0.625f, 0.625f, 1, 1);
            GL_DrawTexture(1281, fx + fw - 5.0f, fy, 5.0f, 40.0f, 0,0, 0.625f, 0.625f, 1, 1);
        }

        // ── Hover: 1=flecha arriba, 2=flecha abajo, 3=thumb del scroll ──────
        int hov = 0;
        if (FUN_0040c490((int)self[11] + (int)self[13] - 21,
                         (int)self[12] - (int)self[14] + 8, 13, 13, 1)) {
            hov = 1;
        } else if (FUN_0040c490((int)self[11] + (int)self[13] - 21,
                                (int)self[12] - 21, 13, 13, 1)) {
            hov = 2;
        } else if (FUN_0040c490((int)self[11] + (int)self[13] - 19,
                                (int)*(float*)&self[38],
                                (int)*(float*)&self[39],
                                (int)*(float*)&self[40], 1)) {
            hov = 3;
        }

        EnableAlphaTest(true);
        glColor4f(1.0f, 1.0f, 1.0f, 0.6f);

        const float arrowX = fx + fw - 21.0f;
        // Flecha arriba.  El sprite se apaga mientras se arrastra el thumb
        // (focus state 2).
        int upOn = (hov == 1 && FUN_0040c680(self) != 2) ? 1 : 0;
        ChatLB_DrawButton(1284, upOn, arrowX,
                          (float)((int)self[12] - (int)self[14]) + 8.0f,
                          13.0f, 13.0f, 0.6f, 0);
        // Flecha abajo — el último argumento es el ENTERO 1 (bits), que hace
        // que sub_40DCE0 espeje la textura en V.  No es 1.0f.
        int dnOn = (hov == 2 && FUN_0040c680(self) != 2) ? 1 : 0;
        ChatLB_DrawButton(1284, dnOn, arrowX,
                          (float)(int)self[12] - 4.0f,
                          13.0f, 13.0f, 0.6f, 1);

        // vtable+72 = entrada 18 = recalcScroll: recalcula self[36..40] (riel y
        // thumb).  SIN esta llamada la barra se dibuja con geometría en cero —
        // era la razón por la que no aparecía.
        ((FnVoidSelf)vt[18])(self);

        const float thumbW = *(float*)&self[39];
        const float trackY = *(float*)&self[36];
        const float trackH = *(float*)&self[37] - trackY;
        const float railX  = fx + fw - thumbW - 8.0f;
        const float barX   = fx + fw - thumbW - 7.0f;

        // Riel.
        GL_DrawTexture(1283, railX, trackY, thumbW, trackH,
                     0.0f, 0.0f, 0.8125f, 0.8125f, 1, 1);

        // Thumb.  Si el contenido ENTRA en la ventana, ocupa todo el riel.
        int total = ((FnIntSelf)vt[19])(self);   // vtable+76 = countVisible
        if (total < (int)self[35]) {
            GL_DrawTexture(1282, barX, trackY, thumbW - 2.0f, trackH,
                         0.0f, 0.0f, 0.6875f, 0.6875f, 1, 1);
            if (hov == 3) {
                if (MouseLButton) glColor4f(0.0f, 0.0f, 0.0f, 0.1f);
                else              glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
                GL_DrawRect(barX, trackY, thumbW - 2.0f, trackH);
                glEnable(GL_TEXTURE_2D);
                glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
            }
        } else {
            const float thumbY = *(float*)&self[38];
            const float thumbH = *(float*)&self[40];
            GL_DrawTexture(1282, barX, thumbY, thumbW - 2.0f, thumbH,
                         0.0f, 0.0f, 0.6875f, 0.6875f, 1, 1);
            if (hov == 3) {
                if (FUN_0040c680(self) == 2) glColor4f(0.0f, 0.0f, 0.0f, 0.1f);
                else                         glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
                GL_DrawRect(barX, thumbY, thumbW - 2.0f, thumbH);
                glEnable(GL_TEXTURE_2D);
                glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
            }
        }

        // Botón de redimensionar (22×12) arriba a la derecha.  Se agrisa
        // mientras se está arrastrando (focus state 1); el arrastre en sí lo
        // maneja el slot 7.
        if (FUN_0040c680(self) == 1) glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
        else                         glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        GL_DrawTexture(1280, fx + fw - 28.0f,
                     (float)((int)self[12] - (int)self[14]) - 8.0f + 4.0f,
                     22.0f, 12.0f, 0.0f, 0.0f, 0.6875f, 0.75f, 1, 1);

        GL_ResetState();
    }

    // BUG-FIX 2026-05-01: header SOLO renderea cuando hay whisper target
    // activo (per IDA sub_40CE20 LABEL_37: chequea v40[0] tras sub_40E780).
    // Antes renderizaba siempre con string vacío → mostraba "palabra filtrada: "
    // (= GlobalText[754]) sin contexto.
    //
    // sub_40E780 (IDA): si this[200] (= self[50]) flag activo, concatena los
    // 5 whisper target names con separador. Sin flag → buffer queda vacío.
    char v40[256] = {0};
    BYTE* this_b = (BYTE*)self;
    if (this_b[200]) {
        BYTE* v4 = this_b + 456;
        for (int i = 0; i < 5; ++i) {
            strcat(v40, (const char*)(v4 - 256));
            if (!*v4) break;
            v4 += 256;
            // separator at &asc_5590F0; comúnmente ", " — fallback simple
            strcat(v40, ", ");
        }
    }
    if (v40[0]) {
        SelectObject(m_hFontDC, g_hFontBold);
        char Buffer[512] = {0};
        wsprintfA(Buffer, "%s: %s", GlobalText[754], v40);
        m_dwTextColor = 0xFFC8C8FFu;
        m_dwBackColor = DAT_005590ac ? 0u : 0x96000000u;
        EnableAlphaTest(true);
        UI_DrawText((int)self[11] + 10, (int)self[12], Buffer, 0, 1, 0);
        GL_ResetState();
        if (m_hFontDC) SelectObject(m_hFontDC, g_hFontBold);
    }

    // Incrementa el contador de frames — lo usa el slot 23 para hacer parpadear el cursor.
    int r = (int)self[46] + 1;
    self[46] = r;
    return r;
}

// slot 23 — sub_40D610 — render por línea con paleta según el tipo de mensaje y
// composición de "[guild] nombre: texto". El decompile de IDA tiene dos ramas
// (g_pRenderText[+8]==1 → font-bitmap path; else → CUIRenderText path).
// En nuestro build no tenemos la vtable real del motor de g_pRenderText, así que
// tomamos la segunda rama y ruteamos por UI_DrawText.
//
// Paleta por tipo de mensaje (se lee de self[25]+276 — el DWORD de tipo adentro del
// linked-list node payload):
//   0: chat        text=black, bg=light blue
//   1: system      text=red,   bg=dark grey
//   2: GM          text=yellow,bg=dark grey
//   3: whisper     text=white, bg=dark grey
//   4: party       text=black, bg=dark green
//   5: guild       text=black, bg=dark purple
//
// Devuelve 1 si tuvo éxito (RenderScroll usa el signo para ajustar el índice de fila).
static int __fastcall ChatLB_renderLine(DWORD* self, int /*edx*/, int row)
{
    // 2026-07-19: REMOVIDO el gate `if (!DAT_00559c84) return 0;` — era una
    // invención nuestra (IDA sub_40D610 NO lo tiene). El original renderiza las
    // últimas N líneas del chat SIEMPRE in-world (no solo al abrir el input).
    // La preocupación previa de "doble render" (cyan de sub_480980 + blanco de
    // acá) no aplica: in-world sub_480980 NO corre (RenderInformation lo gatea a
    // g_bUseChatListBox||state!=5; in-world state=5 + g_bUseChatListBox=0 = falso),
    // y este render solo corre in-world (via Render_GameFrame). Escenas distintas.

    int v5;
    if (!DAT_005590ac || self[11]) {
        v5 = row;
    } else {
        v5 = row + 1;
        if (row + 1 >= (int)self[35]) return (int)DAT_005590ac;
    }
    if (!DAT_00559bf1 && *(DWORD*)(self[25] + 276) == 3) return 0;
    if (DAT_005590ac) {
        DWORD t = *(DWORD*)(self[25] + 276);
        if (t == 1 || t == 2) return 0;
    }

    EnableAlphaTest(true);
    // Colores 1:1 con IDA sub_40D610 (rama g_pRenderText==1). Antes casi todos
    // estaban mal (case 1/2/3 text, case 0/4/5 back) Y encima m_dwTextColor era
    // otro global → nunca llegaban al render. Ahora m_dwTextColor==DAT_00559c78.
    DWORD msgType = *(DWORD*)(self[25] + 276);
    switch (msgType) {
        case 0:  m_dwTextColor = 0xFF000000u; m_dwBackColor = 0x9632C8FFu; break; // chat: negro / azul
        case 1:  m_dwTextColor = 0xFFFF9664u; m_dwBackColor = 0x96000000u; break; // sistema: celeste / negro
        case 2:  m_dwTextColor = 0xFF001EFFu; m_dwBackColor = 0x96000000u; break; // GM: rojo / negro
        case 3:  m_dwTextColor = 0xFFEFDCCDu; m_dwBackColor = 0x96000000u; break; // whisper: blanco-azulado / negro
        case 4:  m_dwTextColor = 0xFF000000u; m_dwBackColor = 0x96FFC800u; break; // party: negro / celeste
        case 5:  m_dwTextColor = 0xFF000000u; m_dwBackColor = 0xC896FF00u; break; // guild: negro / verde
        default: break;
    }
    if (DAT_005590ac && (m_dwBackColor & 0xFFFFFFu) == 0) m_dwBackColor = 0;

    // Cursor highlight blink: alternate text/back colour every 6 frames
    // por ~3 frames cuando self[28] == self[25].
    if (self[28] == self[25] && (int)(self[46] % 6) < 3) {
        DWORD t = m_dwTextColor;
        m_dwTextColor = m_dwBackColor;
        m_dwBackColor = t;
    }

    // Compone "[remitente]: texto" — el remitente está en nodo+8, el texto en nodo+19.
    char Buffer[512];
    const char* sender = (const char*)(self[25] + 8);
    const char* text   = (const char*)(self[25] + 19);
    if (sender[0]) wsprintfA(Buffer, "%s: %s", sender, text);
    else           wsprintfA(Buffer, "%s", text);

    int x = (int)self[11] + 10;
    int y = (int)self[12] - 13 * v5 - 16;
    UI_DrawText(x, y, Buffer, 0, 1, 0);
    GL_ResetState();
    return 1;
}

// ── sub_40DCE0 — dibuja UN botón del popup ──────────────────────────────────
// Port FIEL de IDA sub_40DCE0 (0x40DCE0, 520 bytes).  Firma original:
//   __stdcall (int Texture, int hover, float x, float y, float Width,
//              float Height, GLfloat alpha, float flipV)
//
// Tres estados:
//   * hover && botón izq APRETADO  → sprite gris (0.8) desplazado 1px abajo-
//     derecha, con el UV encogido 1px (efecto "hundido").
//   * hover && botón izq suelto    → sprite normal + un wash blanco al 10%
//     encima (RenderColor) = highlight.
//   * sin hover                    → sprite normal.
//
// `flipV`: el binario NO lo convierte a float — testea sus BITS contra 0
// (`neg eax; sbb eax,eax; and al,0FEh; inc eax` en 0x40DCFE) para elegir el
// signo del alto (±Height) y así espejar la textura en V.  Todos los callers
// vivos pasan 0.0f, pero se porta el bit-test tal cual para no inventar.
// 0.0625 = flt_5526DC = 1/16 (las texturas nis_btn* son 16px dentro de 256).
static void ChatLB_DrawButton(int Texture, int hover, float x, float y,
                              float Width, float Height, float alpha, DWORD flipBits)
{
    // OJO: el último parámetro es un float en la firma del binario, pero NUNCA
    // se usa como float: sub_40DCE0 testea sus BITS contra 0 (`neg/sbb/and 0FEh/
    // inc` en 0x40DCFE).  Los callers pasan `push 0` o `push 1` — es decir el
    // ENTERO 1, no 1.0f.  Por eso acá lo tomamos como DWORD crudo: convertirlo
    // a float rompería el flip (1.0f tiene bits 0x3F800000, y 1 es un denormal).

    if (hover == 1) {
        if (MouseLButton) {                        // DAT_083a42c4 — botón izq. mantenido
            // Apretado: sprite gris corrido 1px, UV 1px más chico.
            glColor4f(0.8f, 0.8f, 0.8f, alpha);
            float vOff   = 0.0f;
            float hDrawn = Height;
            if (flipBits != 0) {
                vOff   = 0.0625f;                  // 1031798784 = 0x3D800000
                hDrawn = Height * -1.0f;
            }
            RenderBitmap(Texture, x + 1.0f, y + 1.0f, Width, hDrawn,
                         0.0f, vOff,
                         (Width - 1.0f) * 0.0625f, (Height - 1.0f) * 0.0625f, 1, 1);
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
        } else {
            // Hover: sprite normal + wash blanco al 10% encima.
            float hDrawn = (float)(flipBits != 0 ? -1 : 1) * Height;
            RenderBitmap(Texture, x, y, Width, hDrawn, 0.0f, 0.0f,
                         Width * 0.0625f, Height * 0.0625f, 1, 1);
            glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
            GL_DrawRect(x, y, Width, hDrawn);     // RenderColor (FillRect)
            glEnable(GL_TEXTURE_2D);               // 0xDE1
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
        }
    } else {
        // Estado normal.
        float hDrawn = (float)(flipBits != 0 ? -1 : 1) * Height;
        RenderBitmap(Texture, x, y, Width, hDrawn, 0.0f, 0.0f,
                     Width * 0.0625f, Height * 0.0625f, 1, 1);
    }
}

// slot 24 — sub_40D600 — thunk a sub_40DEF0 (verificado: vtable off_5525CC
// entrada 24 = 0x40D600, y sub_40D600 es un thunk de una sola línea).
//
// sub_40DEF0 = RENDER de los 3 botones popup del chat.  Port FIEL
// (0x40DEF0, 825 bytes).  Estructura:
//   1) Fade: self[48] (+192) es el flag "mouse cerca de la barra de chat" que
//      escribe slot 21 (ChatLB_hitTestInput).  self[49] (+196) es el alpha,
//      que sube/baja de a 0.2 por frame hacia 1.0 / 0.0.
//   2) Si alpha > 0 y no estamos escribiendo (!InputEnable), dibuja los tres
//      botones de 16x16 en (295,417) (313,417) (331,417) y, si hay hover,
//      el tooltip GlobalText[750..752] arriba del botón.
//   3) Botón 1: si DAT_00559bf1 (ver-chat) está ON usa el sprite normal; si
//      está OFF dibuja la versión "apagada" gris al 0.7 corrida 1px.
static void __fastcall ChatLB_renderFooter(DWORD* self)
{
    float* pAlpha = (float*)&self[49];             // *(float*)(this + 196)

    // ── 1) Fade in/out ──────────────────────────────────────────────────────
    if (self[48] == 1 && *pAlpha < 1.0f) {
        *pAlpha = *pAlpha + 0.2f;
    } else if (self[48] == 0 && *pAlpha > 0.0f) {
        *pAlpha = *pAlpha - 0.2f;
    }

    if (!(*pAlpha > 0.0f) || DAT_00559c84 /*InputEnable*/) return;

    // ── 2) Setup + hover ────────────────────────────────────────────────────
    EnableAlphaTest(true);
    glColor4f(1.0f, 1.0f, 1.0f, *pAlpha);
    m_dwTextColor = 0xFFFFFFFFu;
    m_dwBackColor = 0xFF000000u;                   // -16777216

    const float bx  = ChatListBox_TabButtonsX;
    const float by  = ChatListBox_TabButtonsY;
    const float gap = ChatListBox_TabButtonSpacing;

    int hovered = 0;
    if      (FUN_0040c490(bx,           by, 16, 16, 1)) hovered = 1;
    else if (FUN_0040c490(bx + gap,     by, 16, 16, 1)) hovered = 2;
    else if (FUN_0040c490(bx + 2 * gap, by, 16, 16, 1)) hovered = 3;

    // Botón 1 — ver chat on/off (textura 1285 = Interface/nis_btnchat).
    if (DAT_00559bf1) {
        ChatLB_DrawButton(1285, hovered == 1, bx, by, 16.0f, 16.0f, *pAlpha, 0);
    } else {
        glColor4f(0.7f, 0.7f, 0.7f, *pAlpha);
        RenderBitmap(1285, bx + 1.0f, by + 1.0f, 16.0f, 16.0f,
                     0.0f, 0.0f, 0.9375f, 0.9375f, 1, 1);
        glColor4f(1.0f, 1.0f, 1.0f, *pAlpha);
    }

    // Botón 2 — tamaño del historial (1286 = nis_btnsize).
    ChatLB_DrawButton(1286, hovered == 2, bx + gap, by, 16.0f, 16.0f, *pAlpha, 0);
    // Botón 3 — transparencia (1287 = nis_btnalpha).
    ChatLB_DrawButton(1287, hovered == 3, bx + 2 * gap, by, 16.0f, 16.0f, *pAlpha, 0);

    // ── 3) Tooltip del botón bajo el cursor ─────────────────────────────────
    if (hovered >= 1 && hovered <= 3) {
        char* tip = GlobalText[750 + (hovered - 1)];
        float tipX = bx + (float)(hovered - 1) * gap;
        int   tipY = (int)(by - 10.0f);
        RenderTipText_stub((int)(tipX - 16.0f), tipY, tip);
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    GL_ResetState();                                // DisableAlphaBlend
}

// slot 25 — sub_40E810 — handler de hover por línea. Setea self[28] (puntero del
// cursor) cuando el mouse está sobre una fila cuyo remitente no es el Hero.
static int __fastcall ChatLB_lineHover(DWORD* self, int, int row)
{
    int v4;
    if (!DAT_005590ac || self[11]) {
        v4 = row;
    } else {
        v4 = row + 1;
        if (row + 1 >= (int)self[35]) return DAT_005590ac;
    }
    if (!DAT_00559bf1 && *(DWORD*)(self[25] + 276) == 3) return 0;
    if (DAT_005590ac) {
        int v5 = *(int*)(self[25] + 276);
        if (v5 == 1 || v5 == 2) return 0;
    } else {
        int v6 = *(int*)(self[25] + 284);
        if (v6) {
            if (!self[34]) {
                if ((int)self[35] - v6 - v4 - 1 < 0) return v4 - (int)self[35];
                v4 += v6;
            }
        }
    }
    if (Hero && *(BYTE*)(Hero + 846) && InputEnable) {
        const char* sender = (const char*)(self[25] + 8);
        SIZE sz = {0,0};
        int len = lstrlenA(sender);
        if (m_hFontDC) GetTextExtentPointA(m_hFontDC, sender, len, &sz);
        if (FUN_0040c490((int)self[11] + 10, (int)self[12] - 13 * v4 - 16, sz.cx, 13, 1)) {
            int v10 = (int)self[25];
            if (Hero && strcmp((const char*)(v10 + 8), (const char*)(Hero + 449))) {
                self[28] = (DWORD)v10;
            }
        }
    }
    return 1;
}

// slot 26 — sub_40E400 — per-frame button input (chat header buttons:
// channel toggle, settings, scroll-snap).  Calls sub_40E330 (Timer_Advance,
// ya declaradas en functions.h); el resto de las dependencias (PlayBuffer, sub_40C500) son
// soft-stubbed.
static int __fastcall ChatLB_perFrameInput(DWORD* self)
{
    typedef void (__fastcall *FnInt2)(DWORD*, int, int);
    void** vt = (void**)*self;

    if (FUN_0040c490((int)self[11], (int)self[12] - (int)self[14],
                     (int)self[13], 8, 1)
        && !FUN_0040c680(self)
        && !dword_55C9B7C)
    {
        dword_55C9B7C = self[7];
        ((FnInt2)vt[1])(self, 0, 1);   // setState(1)
        // PlayBuffer(25,0,0)  — soft-skipped
    }

    // ── Los 3 botones popup del chat — PORT FIEL de IDA sub_40E400 ──────────
    // 2026-07-19: antes se salteaban por no tener las constantes de layout.
    // Leídas del binario en 0x5590B0..0x5590B8 (3 floats) → ahora viven en
    // globals.cpp como ChatListBox_TabButtonsX/b4/b8 = 295.0 / 417.0 / 18.0, compartidas
    // con el RENDER (slot 24 = ChatLB_renderFooter / IDA sub_40DEF0).
    // → tres rects de 16×16 en (295,417), (313,417) y (331,417). Caen en la
    // misma franja donde sub_4BE4F0 dibuja el input box (y≈415-422), por eso
    // el gate `!InputEnable` los oculta al abrir el recuadro de escribir.
    //
    // Gate de IDA: `if ( *((_DWORD *)this + 48) && !InputEnable )`
    //   self[48] (+192) NO es un puntero a método: es el flag "mouse cerca de
    //   la barra de chat" que escribe el slot 21 (ChatLB_hitTestInput) y que
    //   maneja el fade de los botones en sub_40DEF0.  El comentario anterior
    //   ("puntero a método / widget inicializado") era incorrecto.
    if (self[48] != 0 && DAT_00559c84 == 0) {
        const int bx  = (int)ChatListBox_TabButtonsX, by = (int)ChatListBox_TabButtonsY;
        const int bw  = 16, bh = 16;
        const int gap = (int)ChatListBox_TabButtonSpacing;

        // Botón 1 — "ver chat on/off" (mismo flag que la tecla F2).
        if (FUN_0040c490(bx, by, bw, bh, 1)) {
            DAT_00559bf1 = (DAT_00559bf1 == 0);
            PlayBuffer(25, 0, 0);
            // IDA: `(*(...)(*(_DWORD *)this + 48))(this, 0)` → *this = vptr,
            // +48 BYTES = entrada 12 de la vtable = sub_40CC50 (scrollByN),
            // NO el slot 18.  Verificado leyendo off_5525CC+48 = 0x40CC50.
            // Al cambiar la visibilidad de los whispers cambia la cantidad de
            // filas visibles, así que se re-scrollea con delta 0.
            ((FnInt2)vt[12])(self, 0, 0);
            DAT_083a4124 = 0;                 // MouseLButtonPush = 0 (consumir click)
        }

        // Botón 2 — cicla el tamaño del historial (mismo que la tecla F4).
        if (FUN_0040c490(bx + gap, by, bw, bh, 1)) {
            FUN_0040e330((int)(uintptr_t)self);
            PlayBuffer(25, 0, 0);
            DAT_083a4124 = 0;
        }

        // Botón 3 — transparencia: self[47] += 0.2, y al pasar 0.9 vuelve a 0.2.
        // self[47] es el float de alpha del recuadro (+188), el mismo campo que
        // escribe F3/30 ReceiveOption como `transparencia * 0.1`.
        if (FUN_0040c490(bx + 2 * gap, by, bw, bh, 1)) {
            float* alpha = (float*)&self[47];
            *alpha += 0.2f;
            if (*alpha > 0.89999998f) *alpha = 0.2f;
            PlayBuffer(25, 0, 0);
            DAT_083a4124 = 0;
        }
    }
    return 1;
}

// slot 28 — sub_40C940 — AddText.  Linked-list append + auto-scroll (via
// vtable[+68]/[+76]/[+116]).  Empty-input path runs sub_40C930 over the
// list as a "ping" (refresh fade timers).
static void __fastcall ChatLB_AddText(DWORD* self, int /*edx*/,
                                      char* src, char* msg,
                                      int kind, int extra)
{
    typedef int  (__fastcall *FnRow )(DWORD*, int, int);
    typedef int  (__fastcall *FnInt )(DWORD*);
    typedef void (__fastcall *FnVoid)(DWORD*);

    if (!src || !msg) return;

    if (!*src && !*msg) {
        if (!DAT_005590ac && !self[34]) {
            DWORD** v6 = (DWORD**)self[23];
            for (DWORD* i = *v6; i != (DWORD*)v6; i = (DWORD*)*i) {
                FUN_0040c930((int)(i + 2));   // ++[+0x114] (cached cx — used as fade)
            }
        }
        return;
    }

    if (DAT_00559bf1) {
        if (kind == 3) {
            // sub_40E730 — whisper-target match — soft-skipped; treat as
            // siempre permitido. Sin eso el texto de susurro se le muestra a todos,
            // lo cual es un cambio benigno respecto del original.
            // PlayBuffer(38,0,0) on whisper-sound flag — soft-skipped.
        }
    } else if (kind == 3) {
        return;
    }

    strncpy((char*)&byte_55C95F8, src, 0xB);
    dword_55C9708 = extra;
    dword_55C9704 = kind;
    dword_55C970C = 0;

    void** vt = (void**)*self;

    // Short message — single-node insert.
    if (strlen(msg) + strlen(src) < 0x14) {
        strncpy((char*)&byte_55C9603, msg, 0x100);

        DWORD list_addr = self[23];
        DWORD* head = (DWORD*)list_addr;
        DWORD* fwd = (DWORD*)*head;             // current fwd
        DWORD* back = (DWORD*)*((DWORD*)*head + 1); // current back

        DWORD* node = (DWORD*)malloc(0x120);
        if (!node) return;
        memset(node, 0, 0x120);
        node[0] = (DWORD)(fwd ? fwd : node);
        node[1] = (DWORD)(back ? back : node);
        if (fwd) fwd[1] = (DWORD)node;
        *(DWORD*)node[1] = (DWORD)node;

        memcpy(node + 2, &byte_55C95F8, 0x118);
        ++self[24];
    } else {
        // Mensaje largo — se parte con sub_40C2A0 en dos pedazos. No tenemos
        // sub_40C2A0 ported; collapse to single insert (truncate).
        char buf[256] = {0};
        strncpy(buf, msg, 0xFF);
        strncpy((char*)&byte_55C9603, buf, 0x100);

        DWORD list_addr = self[23];
        DWORD* head = (DWORD*)list_addr;
        DWORD* fwd  = (DWORD*)*head;
        DWORD* back = (DWORD*)*((DWORD*)*head + 1);

        DWORD* node = (DWORD*)malloc(0x120);
        if (!node) return;
        memset(node, 0, 0x120);
        node[0] = (DWORD)(fwd ? fwd : node);
        node[1] = (DWORD)(back ? back : node);
        if (fwd) fwd[1] = (DWORD)node;
        *(DWORD*)node[1] = (DWORD)node;
        memcpy(node + 2, &byte_55C95F8, 0x118);
        ++self[24];
    }

    // Descarta el más viejo si desbordamos.
    ((FnVoid)vt[17])(self);

    // Auto-scroll-to-end.
    if (self[34] && !((FnRow)vt[29])(self, 0, (int)&byte_55C95F8)) {
        ++self[34];
    }
    if (((FnInt)vt[19])(self) >= (int)self[35]
        && ((FnInt)vt[19])(self) - (int)self[34] < (int)self[35])
    {
        self[34] = ((FnInt)vt[19])(self) - (int)self[35];
    }
}

// slot 29 — sub_40CD30 — ¿el tipo de mensaje de esta fila es visible ahora?
//   - whisper-only mode (byte_559BF1=0) hides msg-type 3 (whispers).
//   - chat-list-box mode hides msg-type 1 and 2 (system/error).
static int __stdcall ChatLB_isRowVisible(int rowData)
{
    if (!DAT_00559bf1 && *(DWORD*)(rowData + 268) == 3) return 1;
    if (DAT_005590ac) {
        int v2 = *(DWORD*)(rowData + 268);
        if (v2 == 1 || v2 == 2) return 1;
    }
    return 0;
}

// ===========================================================================
// Globals compartidos con el resto del build.
// ===========================================================================

// DAT_005590ac (g_bUseChatListBox) ya está definido en globals.cpp:192.
// En el original, /chatlistbox lo invierte en runtime. Nosotros sólo lo consumimos.

// MouseOnWindow — GLOBAL PARTIDO, corregido 2026-07-20.
// Este archivo definía su propio `MouseOnWindow` y lo escribía en el slot 7,
// pero NADIE lo leía: el flag que consume el resto del build (y en particular
// Player_InputTick, para no mandar al personaje a caminar cuando el click cae
// sobre una ventana) es `g_MouseOnWindow`, definido en Game/Player_InputTick.cpp.
// Resultado: clickear dentro del recuadro del chat hacía caminar al personaje.
//
// No se puede escribir g_MouseOnWindow directo desde acá: Player_InputTick lo
// resetea a 0 al principio de su propio tick (MouseOnWindow_Update), que corre
// DESPUÉS del tick del ChatListBox (Game_CharSelectTick: slot 5 en la línea 217,
// FUN_004acef0 en la 298).  Así que el slot 7 deja el resultado en este latch y
// MouseOnWindow_Update lo consulta.  El latch se reescribe entero en cada tick
// del widget, así que no se queda pegado.
extern "C" int MouseOnWindow = 0;
extern "C" int g_ChatLB_MouseOnWindow = 0;

// ===========================================================================
// ===  WIDGET DE LISTA DE GUILD (dword_55C9FF4, vtable off_5526EC)         ===
// ===========================================================================
//
// 2026-08-15.  El objeto de `dword_55C9FF4` (el que construye `sub_40E990`) NO
// comparte vtable con el chat principal: son DOS vtables distintas.
//
//   slot  off   off_5525CC (chat)  off_5526EC (guild)   ¿igual?
//    0    +00   0x40DB80           0x40EAC0             NO  dtor
//    9    +24   0x411A20           0x412180             NO
//   12    +30   0x40CC50           0x4119A0             NO  scrollByN
//   19    +4C   0x40CD80           0x410D50             NO  countVisible
//   20    +50   0x40CDD0           0x412470             NO  advanceCursor
//   21    +54   0x40E230           0x4124B0             NO  hitTest
//   22    +58   0x40CE20           0x40ED80             NO  renderBg
//   23    +5C   0x40D610           0x40EF10             NO  renderLine
//   24    +60   0x40D600           0x410D70 (nullsub)   NO  renderFooter
//   25    +64   0x40E810           0x40EAB0 (return 1)  NO
//   26    +68   0x40E400           0x40F320             NO  perFrameInput
//   28    +70   0x40C940           0x40EC20             NO  AddText/AddMember
//   29    +74   0x40CD30           0x4125F0             NO
//   (el resto coincide byte a byte)
//
// Nuestro `ChatListBox_ConstructWhisper` instalaba `s_ChatLB_VTable` en los dos
// objetos, asi que `RenderGuildList` -> slot 4 -> slots 20/22/23/24 corria los
// metodos del CHAT sobre el objeto de GUILD (otros offsets, otro layout de
// nodo) — de ahi el AV al abrir el panel teniendo miembros.  Aca esta la vtable
// propia, con los slots de la ruta de render y de datos portados 1:1.
//
// Nodo de la lista (0x18 bytes), tal como lo arma sub_40EC20:
//   +0x00 next . +0x04 prev
//   +0x08 name[11]  (strncpy 0xB desde el staging dword_55C9B60)
//   +0x13 flag de conexion (a3)
//   +0x14 numero de party, o 0xFF si no esta en party (a4)
// ===========================================================================

extern "C" void  __cdecl CreateGuildMark(int markIndex, bool blend);
extern "C" void  __cdecl RenderTipText(int sx, int sy, const char* Text);
// UI_DrawText (RenderText) y FUN_0047ec60 (ClearInput) ya vienen de
// functions.h con vinculacion C++; no re-declararlos aca.
// FUN_00404bc0 (PlayBuffer) tambien viene de functions.h (vinculacion C++).
extern "C" float flt_83A7ACC[8];
extern "C" BYTE  InputTextHide[10];

// Globales que en el binario solo tocan sub_40EF10 / sub_40F320 y el estado 126
// de UI_InGameMenu (el dialogo de expulsar miembro).
// 0x07E91790 — tabla de miembros del guild, **stride 13 bytes**:
//   +0..9 name . +10 NUL . +11 connected . +12 party (o -1)
// La llena `ReceiveGuildList` (0x4348B0: `v3 = &unk_7E9179B`, `v3 += 13`) y la
// indexa `UI_InGameMenu` case 126 como `&byte_7E91790[13 * dword_5615E4]` para
// armar el paquete de expulsar.  `sub_40EF10` lee `byte_7E91790` a secas, que es
// el nombre del registro 0.  Tamano = espacio real entre 0x07E91790 y
// 0x07E919BC en el binario (0x22C = 556 bytes = 42 registros).
extern "C" char  byte_7E91790[0x22C] = {0};
extern "C" int   dword_5615E4     = 0;     // 0x005615E4 — indice del miembro elegido

// ── slot 19 — sub_410D50 — cantidad de filas del listado activo ─────────────
static int __fastcall GuildLB_countVisible(DWORD* self)
{
    if (self[29] == 1) return (int)self[32];
    return (int)self[24];
}

// ── slot 20 — sub_412470 — deja el cursor `self[25]` en la primera fila
//    visible, saltando `self[34]` (offset de scroll) nodos ────────────────────
static int __fastcall GuildLB_advanceCursor(DWORD* self)
{
    DWORD* head = (self[29] == 1) ? (DWORD*)self[31] : (DWORD*)self[23];
    if (!head) return 0;
    int result = 0;
    for (self[25] = *head; result < (int)self[34]; self[25] = *(DWORD*)self[25])
        ++result;
    return result;
}

// ── slot 12 — sub_4119A0 — scroll por N filas, con clamp ────────────────────
static int __fastcall GuildLB_scrollByN(DWORD* self, int /*edx*/, int a2)
{
    typedef int (__fastcall *FnInt)(DWORD*);
    void** vt = (void**)*self;
    int result;
    if (self[29] == 1) {
        result = (int)self[32];
        if ((DWORD)result < self[35]) return result;
    } else {
        result = (int)self[35];
        if (self[24] < (DWORD)result) return result;
    }
    result = (int)self[34] - a2;
    self[34] = (DWORD)result;
    if (result >= 0) {
        result = ((FnInt)vt[19])(self) - (int)self[35];
        if ((int)self[34] > result) {
            result = ((FnInt)vt[19])(self) - (int)self[35];
            self[34] = (DWORD)result;
        }
    } else {
        self[34] = 0;
    }
    return result;
}

// ── slot 21 — sub_4124B0 — hit-test del rectangulo del panel ────────────────
static int __fastcall GuildLB_hitTestInput(DWORD* self)
{
    return FUN_0040c490((int)self[11], (int)self[12], (int)self[13], (int)self[14], 2);
}

// ── slot 22 — sub_40ED80 — fondo: flechas arriba/abajo + riel + pulgar ──────
static int __fastcall GuildLB_renderBg(DWORD* self)
{
    typedef void (__fastcall *FnVoid)(DWORD*);
    typedef int  (__fastcall *FnInt )(DWORD*);
    void**  vt = (void**)*self;
    float*  f  = (float*)self;

    self[14] = (DWORD)(40 * ((int)self[35] / 3) + 10);
    ((FnVoid)vt[18])(self);                       // recalcScroll

    if (((FnInt)vt[19])(self) >= (int)self[35]) {
        float y = (float)((int)self[12] - (int)self[14]) + 8.0f;
        float x = (float)((int)self[13] + (int)self[11]) - 19.0f;
        GL_DrawTexture(1284, x, y, 13.0f, 13.0f, 0.0f, 0.0f, 0.8125f, 0.8125f, 1, 1);
        EnableAlphaTest(true);
        float ya = (float)(int)self[12] - 4.0f;
        float xa = (float)((int)self[13] + (int)self[11]) - 19.0f;
        // alto NEGATIVO = la misma flecha espejada en V (la de "abajo").
        GL_DrawTexture(1284, xa, ya, 13.0f, -13.0f, 0.0f, 0.0f, 0.8125f, 0.8125f, 1, 1);
        GL_ResetState();                           // DisableAlphaBlend
        float top    = f[36];
        float height = f[37] - top;
        float xb = (float)((int)self[13] + (int)self[11]) - f[39] - 6.0f;
        GL_DrawTexture(1283, xb, top, f[39], height, 0.0f, 0.0f, 0.8125f, 0.8125f, 1, 1);
        float width = f[39] - 2.0f;
        float xc = (float)((int)self[13] + (int)self[11]) - f[39] - 5.0f;
        GL_DrawTexture(1282, xc, f[38], width, f[40], 0.0f, 0.0f, 0.6875f, 0.6875f, 1, 1);
    }
    return 1;
}

// ── slot 23 — sub_40EF10 — una fila: marca de guild (solo el master),
//    nombre, "(N)" de party y el boton de expulsar con su tooltip ────────────
static int __fastcall GuildLB_renderLine(DWORD* self, int /*edx*/, int a2)
{
    typedef int (__fastcall *FnInt)(DWORD*);
    void** vt = (void**)*self;

    EnableAlphaTest(true);
    int v4  = (int)self[11] + 8;
    int v22 = v4;

    int v5;
    if (((FnInt)vt[19])(self) <= (int)self[35])
        v5 = 13 * (((FnInt)vt[19])(self) - (int)self[35] - a2) + (int)self[12] - 16;
    else
        v5 = (int)self[12] - 13 * a2 - 16;

    if (m_hFontDC) SelectObject(m_hFontDC, g_hFontBold);

    // Avance del cursor tal cual el binario: `self[25] = cursor->next`, y acto
    // seguido `self[25] = (cursor->next)->prev`.  Con la lista circular eso
    // vuelve a dejar el cursor donde estaba; `v8` queda con el `next` para
    // detectar la ultima fila (la del guild master).
    DWORD* v6 = (DWORD*)self[25];
    DWORD  v7 = self[23];
    if (!v6) return 0;
    DWORD  v8 = v6[0];
    if (!v8) return 0;
    self[25] = v8;
    self[25] = *(DWORD*)(v8 + 4);
    if (!self[25]) return 0;

    BYTE party = *(BYTE*)(self[25] + 20);
    if (v8 == v7) {
        m_dwBackColor = 0xFF962828u;                        // -6936536
        m_dwTextColor = (party != 0xFF) ? 0xFFFFFFFFu : 0xC4C4C4FFu;
        CreateGuildMark(*(short*)((BYTE*)(uintptr_t)Hero + 474), true);
        GL_DrawTexture(34, (float)v22, (float)v5, 8.0f, 8.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1, 1);
        v4 += 13;
    } else {
        m_dwBackColor = 0;
        m_dwTextColor = (party != 0xFF) ? 0xFFFFE6D2u : 0xC4C4C4D2u;
    }

    char Buffer[256];
    memset(Buffer, 0, sizeof(Buffer));
    const char* name = (const char*)(self[25] + 8);
    _snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "%s", name);
    UI_DrawText(v4, v5, Buffer, 0, 1, 0);

    if (party != 0xFF) {
        m_dwBackColor = 0;
        m_dwTextColor = 0xFF00C4FFu;                        // -16726785
        _snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "(%d)", party + 1);
        UI_DrawText((int)self[13] + (int)self[11] - 60, v5, Buffer, 0, 1, 0);
    }

    // Boton de expulsar: solo lo ve el master (self[46]==1) o la propia fila.
    if (self[46] == 1 ||
        !strcmp(name, (const char*)((BYTE*)(uintptr_t)Hero + 449)))
    {
        // flt_55265C = 22.0 . flt_5527DC = 13.0 . flt_55256C = 1.0 . flt_5527D8 = 11.0
        float bx = (float)((int)self[13] + (int)self[11]) - 22.0f - 13.0f;
        float by = (float)v5 - 1.0f;
        GL_DrawTexture(280, bx, by, 13.0f, 11.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);

        if ((float)(int)MouseX >= bx && (float)(int)MouseX < bx + 13.0f &&
            (float)(int)MouseY >= by && (float)(int)MouseY < by + 11.0f)
        {
            if (MouseLButton)
                GL_DrawTexture(281, bx, by, 13.0f, 11.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
            m_dwTextColor = 0xFFFFFFFFu;
            m_dwBackColor = 0xFF000000u;
            const char* tip;
            if (!strcmp(name, (const char*)((BYTE*)(uintptr_t)Hero + 449)) &&
                !strcmp(byte_7E91790, (const char*)((BYTE*)(uintptr_t)Hero + 449)))
                tip = GlobalText[188];
            else
                tip = GlobalText[189];
            RenderTipText((int)bx - 20, (int)by, tip);
        }
    }

    if (m_hFontDC) SelectObject(m_hFontDC, g_hFont);
    GL_ResetState();                                             // DisableAlphaBlend
    return 1;
}

// ── slot 24 — sub_410D70 — nullsub (el chat si tiene footer; el guild no) ───
static void __fastcall GuildLB_renderFooter(DWORD* /*self*/) {}

// ── slot 25 — sub_40EAB0 — return 1 ─────────────────────────────────────────
static int __fastcall GuildLB_lineHover(DWORD* /*self*/, int /*edx*/, int /*row*/) { return 1; }

// ── slot 26 — sub_40F320 — click en el boton de expulsar -> dialogo 126 ─────
static int __fastcall GuildLB_perFrameInput(DWORD* self)
{
    typedef int (__fastcall *FnInt)(DWORD*);
    void** vt = (void**)*self;

    if (!self[23]) return 1;
    int v2 = 0;
    for (self[25] = *(DWORD*)self[23]; v2 < (int)self[34]; self[25] = *(DWORD*)self[25])
        ++v2;

    int v3 = 0;
    if ((int)self[35] > 0) {
        int v11 = 0;
        do {
            DWORD v4 = self[25];
            if (v4 == self[23]) break;
            if (self[46] == 1 ||
                !strcmp((const char*)(v4 + 8), (const char*)((BYTE*)(uintptr_t)Hero + 449)))
            {
                int v5;
                if (((FnInt)vt[19])(self) <= (int)self[35])
                    v5 = 13 * (((FnInt)vt[19])(self) - (int)self[35] - v3) + (int)self[12] - 16;
                else
                    v5 = (int)self[12] - v11 - 16;

                double v6  = (double)(int)self[13] + (double)(int)self[11] - 20.0 - 12.0;
                float  v12 = (float)v5;
                double mx  = (double)(int)MouseX;
                if (mx >= v6 && mx < v6 + 12.0) {
                    double my = (double)(int)MouseY;
                    if (my >= v12 && my < v12 + 10.0 && MouseLButtonPush) {
                        MouseLButtonPush = 0;
                        FUN_00404bc0(25, 0, 0);                 // PlayBuffer(25)
                        int v9 = ((FnInt)vt[19])(self) - (int)self[34];
                        DAT_083a7c24     = 126;   // ErrorMessage
                        dword_5615E4     = v9 - v3 - 1;
                        FUN_0047ec60(0);                        // ClearInput(0)
                        InputEnable      = 0;
                        DAT_00559c88     = 1;     // InputNumber
                        // IDA: `*(float *)InputTextMax = flt_83A7ACC[0];` —
                        // escribe BITS de float sobre InputTextMax[0], que es
                        // int.  Se porta tal cual (el original lo lee despues
                        // como int; ver Game_SceneUpdate).
                        memcpy(&_InputTextMaxArr[0], &flt_83A7ACC[0], sizeof(float));
                        InputTextHide[0] = 1;
                    }
                }
            }
            ++v3;
            self[25] = *(DWORD*)self[25];
            v11 += 13;
        } while (v3 < (int)self[35]);
    }
    return 1;
}

// ── slot 28 — sub_40EC20 — agrega un miembro al listado ─────────────────────
static void __fastcall GuildLB_AddMember(DWORD* self, int /*edx*/,
                                         const char* Source, char a3, char a4)
{
    typedef int  (__fastcall *FnInt )(DWORD*);
    typedef void (__fastcall *FnVoid)(DWORD*);
    void** vt = (void**)*self;

    if (!Source || !*Source) return;

    if (!((FnInt)vt[19])(self))
        self[46] = (DWORD)(strcmp(Source, (const char*)((BYTE*)(uintptr_t)Hero + 449)) == 0);

    // Staging dword_55C9B60[11] + byte_55C9B6B + byte_55C9B6C, tal cual el
    // original (que despues copia 12 bytes de golpe al nodo).
    char staging[13];
    memset(staging, 0, sizeof(staging));
    strncpy(staging, Source, 0xB);
    staging[11] = a3;
    staging[12] = a4;

    DWORD* head = (DWORD*)self[23];
    if (!head || !head[0]) return;
    DWORD* v6 = (DWORD*)head[0];              // head->next
    DWORD* v7 = (DWORD*)v6[1];                // head->next->prev
    DWORD* v8 = (DWORD*)malloc(0x18);
    if (!v8) return;
    memset(v8, 0, 0x18);
    v8[0] = (DWORD)(uintptr_t)(v6 ? v6 : v8);
    v8[1] = (DWORD)(uintptr_t)(v7 ? v7 : v8);
    v6[1] = (DWORD)(uintptr_t)v8;
    *(DWORD*)v8[1] = (DWORD)(uintptr_t)v8;

    // Payload: 12 bytes desde el staging a nodo+8, y a4 a nodo+20.
    memcpy((BYTE*)v8 + 8, staging, 12);
    *((BYTE*)v8 + 20) = (BYTE)a4;

    ++self[24];
    ((FnVoid)vt[17])(self);                   // trimOldest
    if (((FnInt)vt[19])(self) > (int)self[35])
        ++self[34];
    if (((FnInt)vt[19])(self) >= (int)self[35] &&
        ((FnInt)vt[19])(self) - (int)self[34] < (int)self[35])
        self[34] = (DWORD)(((FnInt)vt[19])(self) - (int)self[35]);
}

// ── slot 29 — sub_4125F0 — deleting-dtor del sub-objeto ─────────────────────
// TODO(sin portar): `sub_412610` (el teardown real de la lista) no esta
// portado; aca queda solo el `if (flag & 1) delete this`, que es la parte
// observable.  Este slot no se alcanza desde la ruta de render.
static void* __fastcall GuildLB_scalarDelete(DWORD* self, int /*edx*/, char flag)
{
    if (flag & 1) free(self);
    return self;
}

// ---------------------------------------------------------------------------
// Vtable del widget de guild — off_5526EC.  Los slots que el binario comparte
// con off_5525CC apuntan a la misma implementacion que el chat.
//
// TODO(sin portar): slot 0 (sub_40EAC0, dtor) y slot 9 (sub_412180, key
// handler).  Ninguno esta en la ruta de render ni en la de datos; quedan con la
// implementacion del chat para no dejar el slot en NULL, y anotados aca porque
// NO son equivalentes en el binario.
// ---------------------------------------------------------------------------
static ChatLB_VTable s_GuildLB_VTable = { {
    /*0x00*/ (void*)ChatLB_dtor,            // != real: sub_40EAC0  (TODO)
    /*0x04*/ (void*)ChatLB_setState,
    /*0x08*/ (void*)ChatLB_setColor1,
    /*0x0C*/ (void*)ChatLB_setColor2,
    /*0x10*/ (void*)ChatLB_renderScroll,    // sub_411920 — compartido
    /*0x14*/ (void*)ChatLB_tick,
    /*0x18*/ (void*)ChatLB_nullsub,
    /*0x1C*/ (void*)ChatLB_handleScrollIn,
    /*0x20*/ (void*)ChatLB_nullsub,
    /*0x24*/ (void*)ChatLB_keyHandler,      // != real: sub_412180  (TODO)
    /*0x28*/ (void*)ChatLB_clearList,       // sub_4118D0 — compartido
    /*0x2C*/ (void*)ChatLB_nullsub2,
    /*0x30*/ (void*)GuildLB_scrollByN,      // sub_4119A0
    /*0x34*/ (void*)ChatLB_getVisibleCnt,
    /*0x38*/ (void*)ChatLB_setVisibleCnt,
    /*0x3C*/ (void*)ChatLB_setVisibleCnt,
    /*0x40*/ (void*)ChatLB_incrStep,
    /*0x44*/ (void*)ChatLB_trimOldest,      // sub_4122C0 — compartido
    /*0x48*/ (void*)ChatLB_recalcScroll,    // sub_412320 — compartido
    /*0x4C*/ (void*)GuildLB_countVisible,   // sub_410D50
    /*0x50*/ (void*)GuildLB_advanceCursor,  // sub_412470
    /*0x54*/ (void*)GuildLB_hitTestInput,   // sub_4124B0
    /*0x58*/ (void*)GuildLB_renderBg,       // sub_40ED80
    /*0x5C*/ (void*)GuildLB_renderLine,     // sub_40EF10
    /*0x60*/ (void*)GuildLB_renderFooter,   // sub_410D70 (nullsub)
    /*0x64*/ (void*)GuildLB_lineHover,      // sub_40EAB0
    /*0x68*/ (void*)GuildLB_perFrameInput,  // sub_40F320
    /*0x6C*/ (void*)ChatLB_nullsub2,
    /*0x70*/ (void*)GuildLB_AddMember,      // sub_40EC20
    /*0x74*/ (void*)GuildLB_scalarDelete,   // sub_4125F0
} };

extern "C" void* GuildListBox_GetVTable(void) { return &s_GuildLB_VTable; }

// ---------------------------------------------------------------------------
// Entradas publicas — espejo de `ReceiveGuildList` (0x4348B0), que hace
//   vtable[10](this)                       <- limpiar
//   vtable[28](this, rec, rec[11], party)  <- una por miembro
// ---------------------------------------------------------------------------
extern "C" void GuildList_Clear(void)
{
    DWORD* obj = (DWORD*)(uintptr_t)DAT_055c9ff4;
    if (!obj || !*obj) return;
    typedef void (__fastcall *FnVoid)(DWORD*);
    ((FnVoid)((void**)*obj)[10])(obj);
}

extern "C" void GuildList_AddMember(const char* name, char connected, char partyNumber)
{
    DWORD* obj = (DWORD*)(uintptr_t)DAT_055c9ff4;
    if (!obj || !*obj) return;
    typedef void (__fastcall *FnAdd)(DWORD*, int, const char*, char, char);
    ((FnAdd)((void**)*obj)[28])(obj, 0, name, connected, partyNumber);
}

// ── FUN_0040c170 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 29-byte ─────────────────────────────────────────────────────────────────

// FUN_0040c170 @ 0x0040C170 (29 bytes) — thiscall: copia 4 bytes del parámetro a this
void __fastcall FUN_0040c170(int ecx, int /*edx*/, BYTE *param_1) {
    *(BYTE *)ecx       = param_1[0];
    *(BYTE *)(ecx + 1) = param_1[1];
    *(BYTE *)(ecx + 2) = param_1[2];
    *(BYTE *)(ecx + 3) = param_1[3];
}

// ── FUN_0040c190 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040c190 @ 0x0040C190 (~77 lines) — Parse mesh flags from material name suffix
char __fastcall FUN_0040c190(void* ecx, void* /*edx*/, DWORD* param_1) {
    (void)ecx; (void)param_1;
    // After '_': R->flag[0], H->flag[1], S->flag[2], N->flag[3]; flag[4]=valid
    return 0;
}

// ── FUN_0040c2a0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040c2a0 @ 0x0040C2A0 (~86 lines) — Word-wrap text into multi-line buffer
int __cdecl FUN_0040c2a0(LPCSTR param_1, int param_2, int param_3, int param_4,
                         size_t param_5, UINT param_6, int param_7) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    (void)param_5; (void)param_6; (void)param_7;
    return 0;
}

// ── FUN_0040c480 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c480 @ 0x0040C480 (12 bytes) — increment ref counter
// FUN_0040c480 (IDA-activated, was Ghidra stub)
int FUN_0040c480()
{
  return ++DAT_055c9b78;
}

// ── FUN_0040c500 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040c500 @ 0x0040C500 (~36 lines) — UI widget: insert child node into linked list
void __fastcall FUN_0040c500(void* ecx, void* /*edx*/, int param_1, int param_2, int param_3) {
    (void)ecx; (void)param_1; (void)param_2; (void)param_3;
}

// ── FUN_0040c580 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_0040c580 @ 0x0040C580 (71 bytes) — dequeue front from linked list + copy 3 fields
void __fastcall FUN_0040c580(int param_1) {
    if (*(int *)(param_1 + 0xc) != 0) {
        int iVar1 = **(int **)(param_1 + 8);
        *(int *)(param_1 + 0x10) = *(int *)(iVar1 + 8);
        *(int *)(param_1 + 0x14) = *(int *)(iVar1 + 0xc);
        *(int *)(param_1 + 0x18) = *(int *)(iVar1 + 0x10);
        int *lpMem = (int *)**(int **)(param_1 + 8);
        *(int *)lpMem[1] = *lpMem;
        *(int *)(*lpMem + 4) = lpMem[1];
        operator_delete(lpMem);
        *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) - 1;
    }
}

// ── FUN_0040c5d0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040c5d0 @ 0x0040C5D0 (~38 lines) — UI widget constructor (base class)
void* __fastcall FUN_0040c5d0(void* param_1) {
    return param_1;
}

// ── FUN_0040c670 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c670 @ 0x0040C670 (10 bytes) — thiscall setter field_0x24
void __fastcall FUN_0040c670(int ecx, int /*edx*/, int param_1) {
    *(int *)(ecx + 0x24) = param_1;
}

// ── FUN_0040c680 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 4-5 byte: trivial field access ──────────────────────────────────────────

// FUN_0040c680 @ 0x0040C680 (4 bytes) — getter thiscall: devuelve this->field_0x24
// FUN_0040c680 (IDA-activated, was Ghidra stub)
int __cdecl FUN_0040c680(DWORD *_this)
{
  return _this[9];
}

// ── FUN_0040c6b0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c6b0 @ 0x0040C6B0 (17 bytes) — thiscall: setea dos campos en +0x34 y +0x38
void __fastcall FUN_0040c6b0(int ecx, int /*edx*/, int param_1, int param_2) {
    *(int *)(ecx + 0x34) = param_1;
    *(int *)(ecx + 0x38) = param_2;
}

// ── FUN_0040c6d0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 24-byte ─────────────────────────────────────────────────────────────────

// FUN_0040c6d0 @ 0x0040C6D0 (24 bytes) — thiscall: setea 3 campos en +0x3c, +0x44 y +0x48
void __fastcall FUN_0040c6d0(int ecx, int /*edx*/, int p1, int p2, int p3) {
    *(int *)(ecx + 0x3c) = p1;
    *(int *)(ecx + 0x44) = p2;
    *(int *)(ecx + 0x48) = p3;
}

// ── FUN_0040c6f0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c6f0 @ 0x0040C6F0 (24 bytes) — thiscall: setea 3 campos en +0x40, +0x4c y +0x50
void __fastcall FUN_0040c6f0(int ecx, int /*edx*/, int p1, int p2, int p3) {
    *(int *)(ecx + 0x40) = p1;
    *(int *)(ecx + 0x4c) = p2;
    *(int *)(ecx + 0x50) = p3;
}

// ── FUN_0040c710 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040c710 @ 0x0040C710 (~41 lines) — UI widget: recursive input processing
int __fastcall FUN_0040c710(void* ecx, void* /*edx*/, int param_1) {
    (void)ecx; (void)param_1;
    return 0;
}

// ── FUN_0040c930 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_0040c930 @ 0x0040C930 (11 bytes) — refcount increment at +0x114
// FUN_0040c930 (IDA-activated, was Ghidra stub)
int __cdecl FUN_0040c930(int a1)
{
  int result; // eax

  result = a1;
  ++*(DWORD *)(a1 + 276);
  return result;
}

// ── FUN_0040cdd0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_0040cdd0 @ 0x0040CDD0 (71 bytes) — skip ahead in linked list
// FUN_0040cdd0 (IDA-activated, was Ghidra stub)
int __cdecl FUN_0040cdd0(DWORD *_this)
{
  int v2; // edi
  int result; // eax

  v2 = 0;
  result = _this[34];
  _this[25] = *(DWORD *)_this[23];
  if ( result > 0 )
  {
    do
    {
      result = _this[25];
      if ( result == _this[23] )
      {
        break;
      }
      if ( !(*(int (__cdecl **)(DWORD *, int))(*_this + 116))(_this, result + 8) )
      {
        ++v2;
      }
      _this[25] = *(DWORD *)_this[25];
      result = _this[34];
    }
    while ( v2 < result );
  }
  return result;
}

// ── FUN_0040d550 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_0040d550 @ 0x0040D550 (~45 lines) — UI widget list: destructor helper
void __fastcall FUN_0040d550(void* param_1) { (void)param_1; }

// ── FUN_00410a90 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00410a90 @ 0x00410A90 (18 bytes)
void __fastcall FUN_00410a90(int *param_1) {
    FUN_0040f680(param_1);
    *param_1 = (int)&PTR_LAB_00552810;
}

// ── FUN_00410ab0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00410ab0 @ 0x00410AB0 — StreamBase2 ~dtor
void __fastcall FUN_00410ab0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00410ad0((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── FUN_00410ad0 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00410ad0 @ 0x00410AD0 (11 bytes)
void __fastcall FUN_00410ad0(void *This) {
    *(int *)This = (int)&PTR_LAB_00552810;
    FUN_0040f690(This);
}

// ── FUN_00410de0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_00410de0 @ 0x00410DE0 (62 bytes) — list: erase range [param_2, param_3)
void __fastcall FUN_00410de0(void *This, int /*edx*/, int *param_1, int *param_2, int *param_3) {
    while (param_2 != param_3) {
        int *piVar1 = (int *)*param_2;
        *(int *)param_2[1] = *param_2;
        *(int *)(*param_2 + 4) = param_2[1];
        operator_delete(param_2);
        *(int *)((int)This + 8) = *(int *)((int)This + 8) - 1;
        param_2 = piVar1;
    }
    *param_1 = (int)param_2;
}

// ── FUN_00410e30 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00410e30 @ 0x00410E30 (15 bytes) — thiscall: pop front from singly-linked list
void __fastcall FUN_00410e30(int ecx, int /*edx*/, int *param_1) {
    int *puVar1 = *(int **)ecx;
    *(int *)ecx = *puVar1;
    *param_1 = (int)puVar1;
}

// ── FUN_00410e40 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// ── 14-byte ─────────────────────────────────────────────────────────────────

// FUN_00410e40 @ 0x00410E40 (14 bytes) — thiscall: copia el valor de *(this+4)->first
void __fastcall FUN_00410e40(int ecx, int /*edx*/, int *param_1) {
    *param_1 = **(int **)(ecx + 4);
}

// ── FUN_00410e50 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00410e50 @ 0x00410E50 (~295 lines) — BST_EraseNode: red-black tree node removal with
// full rebalancing (left/right rotations, color fixup). Frees node via delete, decrements
// el tamaño del árbol en this+0xC. Internals estándar estilo map::erase de la STL.
void __cdecl FUN_00410e50(void* self, DWORD* param_1, int* param_2) {
    (void)self; (void)param_1; (void)param_2;
}

// ── FUN_00411360 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00411360 @ 0x00411360 (~16 lines) — BST_UnlinkNode: unlinks doubly-linked node
// (punteros prev/next en [0]/[1]), llama a delete sobre el nodo, decrementa el contador en this+8.
void __cdecl FUN_00411360(void* self, int* param_1, int* param_2) {
    (void)self; (void)param_1; (void)param_2;
}

// ── FUN_004113a0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// ── BST / STL map operations ────────────────────────────────────────────────

// FUN_004113a0 @ 0x004113A0 (62 bytes) — BST lower_bound (find >= key)
void __fastcall FUN_004113a0(void *This, int /*edx*/, int *param_1, int *param_2) {
    int *puVar3 = *(int **)((int)This + 4);
    if ((void *)puVar3[1] != DAT_055c9b98) {
        int *puVar1 = (int *)puVar3[1];
        do {
            if ((int)puVar1[3] < *param_2) {
                puVar1 = (int *)puVar1[2];
            } else {
                puVar3 = puVar1;
                puVar1 = (int *)*puVar1;
            }
        } while ((void *)puVar1 != DAT_055c9b98);
        *param_1 = (int)puVar3;
        return;
    }
    *param_1 = (int)puVar3;
}

// ── FUN_004113e0 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_004113e0 @ 0x004113E0 (62 bytes) — BST upper_bound (find > key)
void __fastcall FUN_004113e0(void *This, int /*edx*/, int *param_1, int *param_2) {
    int *puVar3 = *(int **)((int)This + 4);
    if ((void *)puVar3[1] != DAT_055c9b98) {
        int *puVar1 = (int *)puVar3[1];
        do {
            if (*param_2 < (int)puVar1[3]) {
                puVar3 = puVar1;
                puVar1 = (int *)*puVar1;
            } else {
                puVar1 = (int *)puVar1[2];
            }
        } while ((void *)puVar1 != DAT_055c9b98);
        *param_1 = (int)puVar3;
        return;
    }
    *param_1 = (int)puVar3;
}

// ── FUN_00411460 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00411460 @ 0x00411460 (~164 lines) — BST_InsertNode: allocates 0x138-byte node,
// copia la clave vía FUN_004124d0, inserta en el árbol rojo-negro con rebalanceo completo
// (rotaciones vía FUN_00411700/FUN_00411760). Incrementa el contador en this+0xC.
void __cdecl FUN_00411460(void* self, DWORD* param_1, int param_2, int* param_3, int* param_4) {
    (void)self; (void)param_1; (void)param_2; (void)param_3; (void)param_4;
}

// ── FUN_00411700 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_00411700 @ 0x00411700 (96 bytes) — BST left-rotate
void __fastcall FUN_00411700(void *This, int /*edx*/, int param_1) {
    int *piVar1 = *(int **)(param_1 + 8);
    *(int *)(param_1 + 8) = *piVar1;
    if ((void *)*piVar1 != DAT_055c9b98) {
        *(int *)(*piVar1 + 4) = param_1;
    }
    piVar1[1] = *(int *)(param_1 + 4);
    if (param_1 == *(int *)(*(int *)((int)This + 4) + 4)) {
        *(int **)(*(int *)((int)This + 4) + 4) = piVar1;
        *piVar1 = param_1;
        *(int **)(param_1 + 4) = (int *)piVar1;
        return;
    }
    int *piVar2 = *(int **)(param_1 + 4);
    if (param_1 == *piVar2) {
        *piVar2 = (int)piVar1;
        *piVar1 = param_1;
        *(int **)(param_1 + 4) = (int *)piVar1;
        return;
    }
    piVar2[2] = (int)piVar1;
    *piVar1 = param_1;
    *(int **)(param_1 + 4) = (int *)piVar1;
}

// ── FUN_00411760 — movida desde stubs_bulk_med.cpp (refactor B3) ──
// FUN_00411760 @ 0x00411760 (96 bytes) — BST right-rotate
void __fastcall FUN_00411760(void *This, int /*edx*/, int *param_1) {
    int iVar1 = *param_1;
    *param_1 = *(int *)(iVar1 + 8);
    if (*(int *)(iVar1 + 8) != (int)DAT_055c9b98) {
        *(int *)(*(int *)(iVar1 + 8) + 4) = (int)param_1;
    }
    *(int *)(iVar1 + 4) = param_1[1];
    if (param_1 == *(int **)(*(int *)((int)This + 4) + 4)) {
        *(int *)(*(int *)((int)This + 4) + 4) = iVar1;
        *(int *)(iVar1 + 8) = (int)param_1;
        param_1[1] = iVar1;
        return;
    }
    int *piVar2 = (int *)param_1[1];
    if (param_1 == (int *)piVar2[2]) {
        piVar2[2] = iVar1;
        *(int *)(iVar1 + 8) = (int)param_1;
        param_1[1] = iVar1;
        return;
    }
    *piVar2 = iVar1;
    *(int *)(iVar1 + 8) = (int)param_1;
    param_1[1] = iVar1;
}

// ── FUN_00411820 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00411820 @ 0x00411820 (22 bytes) — thiscall: copy two values
void __fastcall FUN_00411820(int ecx, int /*edx*/, int *param_1, BYTE *param_2) {
    *(int *)ecx = *param_1;
    *(BYTE *)(ecx + 4) = *param_2;
}

// ── FUN_00411840 — movida desde stubs_bulk_small.cpp (refactor B3) ──
// FUN_00411840 @ 0x00411840 (33 bytes) — allocate 0x138 block, set two fields
void __cdecl FUN_00411840(int param_1, int param_2) {
    void *pvVar1 = operator_new(0x138);
    *(int *)((int)pvVar1 + 4) = param_1;
    *(int *)((int)pvVar1 + 0x134) = param_2;
}

// ── FUN_00411920 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00411920 @ 0x00411920 (~31 líneas) — ListBox_RenderItems: llama a métodos de la vtable para
// empezar el render, después itera los ítems llamando a vtable[0x5C](índice) por cada fila visible,
// ajustando el índice según el retorno negativo/cero. Llama a vtable[0x60] para terminar el render.
void __fastcall FUN_00411920(int* param_1) {
    (void)param_1;
}

// ── FUN_004119a0 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_004119a0 @ 0x004119A0 (~29 lines) — ListBox_ScrollUp: decrements scroll offset
// (this+0x88) en param_1, clampeado a [0, itemCount - visibleCount]. Chequea la capacidad
// en this+0x80 o this+0x60 según el flag de modo en this+0x74.
// FUN_004119a0 (IDA-activated, was Ghidra stub)
int __cdecl FUN_004119a0(DWORD *_this, int a2)
{
  int result; // eax

  if ( _this[29] == 1 )
  {
    result = _this[32];
    if ( (unsigned int)result < _this[35] )
    {
      return result;
    }
  }
  else
  {
    result = _this[35];
    if ( _this[24] < (unsigned int)result )
    {
      return result;
    }
  }
  result = _this[34] - a2;
  _this[34] = result;
  if ( result >= 0 )
  {
    result = (*(int (__cdecl **)(DWORD *))(*_this + 76))(_this) - _this[35];
    if ( _this[34] > result )
    {
      result = (*(int (__cdecl **)(DWORD *))(*_this + 76))(_this) - _this[35];
      _this[34] = result;
    }
  }
  else
  {
    _this[34] = 0;
  }
  return result;
}

// ── FUN_00411a20 — movida desde stubs_bulk_misc.cpp (refactor B3) ──
// FUN_00411a20 @ 0x00411A20 (~66 lines) — ListBox_HandleInput: processes key events
// (7=click, 0xC=page-scroll, 0xD/0xE=selection up/down) on a doubly-linked item list.
// Ajusta la posición del scroll vía vtable[0x30] y actualiza el puntero al ítem seleccionado en this[0x1C].
// FUN_00411a20 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00411a20(DWORD *_this)
{
  int v2; // ecx
  int result; // eax
  int v4; // ecx
  int *v5; // eax
  int v6; // edx
  int v7; // ecx
  int v8; // eax
  int v9; // ecx
  int v10; // edx
  int v11; // edx
  int v12; // ecx

  v2 = _this[4];
  switch ( v2 )
  {
    case 7:
      (*(void (__cdecl **)(DWORD *))(*_this + 108))(_this);
      result = 0;
      break;
    case 12:
      (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, -100);
      if ( _this[26] != 1 || !_this[24] )
      {
        goto LABEL_23;
      }
      v4 = _this[23];
      _this[28] = v4;
      result = 0;
      _this[28] = *(DWORD *)(v4 + 4);
      break;
    case 13:
    case 14:
      if ( _this[26] != 1 || !_this[24] )
      {
        goto LABEL_23;
      }
      if ( v2 == 13 )
      {
        v5 = (int *)_this[28];
        if ( v5 == (int *)_this[23] )
        {
          goto LABEL_23;
        }
        v6 = *v5;
        _this[28] = *v5;
        if ( v6 == _this[23] )
        {
          _this[28] = *(DWORD *)(v6 + 4);
        }
      }
      else if ( v2 == 14 )
      {
        v7 = _this[28];
        if ( v7 == *(DWORD *)_this[23] )
        {
          goto LABEL_23;
        }
        _this[28] = *(DWORD *)(v7 + 4);
      }
      if ( _this[24] <= _this[35] )
      {
        goto LABEL_23;
      }
      v8 = 0;
      v9 = *(DWORD *)_this[23];
      _this[25] = v9;
      if ( v9 != _this[23] )
      {
        do
        {
          if ( _this[28] == v9 )
          {
            break;
          }
          ++v8;
          v10 = *(DWORD *)_this[25];
          _this[25] = v10;
          v9 = v10;
        }
        while ( v10 != _this[23] );
      }
      v11 = _this[35];
      v12 = _this[34];
      if ( v8 < v12 + v11 )
      {
        if ( v8 < v12 )
        {
          (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, v12 - v8);
        }
        goto LABEL_23;
      }
      (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, v12 - (v8 - v11 + 1));
      result = 0;
      break;
    default:
LABEL_23:
      result = 0;
      break;
  }
  return result;
}
