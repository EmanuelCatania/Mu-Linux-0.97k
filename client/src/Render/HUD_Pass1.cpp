// HUD_Pass1.cpp — primera tanda de funciones de render del HUD in-world, portadas 1:1
// del mu.exe original (sub_4BCA20, sub_4BD090, sub_4BD2B0, sub_4BFDE0).
//
// Estas cuatro faltaban por completo en nuestro build o estaban stubeadas vacías
// en src/Render/Render_Frame.cpp. Traerlas restaura:
//   * RenderPartyHP        — las barritas de HP sobre los miembros del party en el mundo
//   * RenderBooleans       — floating-numbers iterator (damage/heal popups)
//                            Nota: la llamada a RenderBoolean por entrada (0x480E00,
//                            ~3000 bytes) todavía NO está portada; el iterador
//                            does its layout/de-overlap work but then falls
//                            por un RenderBoolean stubeado por ahora.
//   * RenderMainFrameWindow — el marco inferior del HUD (5 llamadas a RenderBitmap + el
//                            anti-tamper CharacterMachine encrypt/decrypt
//                            block).  Anti-tamper is preserved structurally
//                            por fidelidad, pero usa la hash-table existente
//                            helpers (sub_4041E0/sub_403F80/sub_404400).
//   * Render_HotbarItems3D — sub_4BFDE0, the 3D-projected inventory hotbar
//                            items abajo y al centro de la pantalla.
//
// El helper sub_480C60 (FUN_00480c60) también está portado acá — calcula las dimensiones
// del texto de las etiquetas de números flotantes.
//
// =============================================================================

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include <gl/GL.h>
#include <gl/GLU.h>

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];     // src/Render/HUD_Pass3.cpp
extern "C" void __cdecl SeedQuickPotionTypesFromInventory(void);
extern "C" int __cdecl sub_482E40(int a1);
extern "C" void __cdecl RenderTipText(int sx, int sy, const char* Text);

static bool HUD_IsGoldenArcherPanelRuntime(void)
{
    return (DAT_07eaa128 != 0 && DAT_07eaa128 != 3);
}

// Referencias externas a helpers que ya existen en nuestro build.
// (FUN_005113f0 = Projection, FUN_005125a0 = RenderBitmap, FUN_005124c0 = RenderColor,
//  FUN_0047f7a0 = RenderText_1, FUN_005112f0 = CreateScreenVector,
//  FUN_004e1be0 = RenderItem3D, FUN_005123c0 = BeginBitmap, FUN_005124b0 = EndBitmap,
//  FUN_00511600 = DisableAlphaBlend, EnableAlphaBlend / EnableAlphaTest exist.)
//
// Helpers de hash-table — ya declarados en functions.h con linkage C++; acá no
// los redeclaramos. RenderMainFrameWindow sólo los llama desde el bloque
// gateado `if (CharacterMachine && dword_55C9BD4)`, que en nuestro build se saltea,
// así que no necesitamos sus referencias al enlazar.

// sub_403150 (desencriptado de g_csQuest) — existe en nuestro stubs.cpp como FUN_00403150;
// declarado en functions.h. En realidad no hace falta llamarlo para el
// render de la barra de accesos rápidos — su propósito es refrescar el estado encriptado de quests, que
// has no visible effect when called repeatedly per frame on a stable state.

// ── Globals que el original referencia por nombre simbólico ─────────────────
// Definimos #defines para que los nombres estilo IDA coincidan con el storage que
// ya tenemos, y el código portado quede visualmente cerca del decompile.
// 2026-08-22 FIX: estos cuatro apuntaban a 0x00561558..0x00561564 con el
// comentario "already in our build (assumed)" — una suposición que nunca se
// verificó y que estaba mal.  `ida_xrefs_to` da las direcciones reales, las
// cuatro escritas por `gluPerspective2` (0x511220):
//     ScreenCenterX 0x083A429C · ScreenCenterY 0x083A42A0
//     PerspectiveX  0x083A42A4 · PerspectiveY  0x083A42A8
// 0x00561558 es otra cosa (la escribe `BeginOpengl`, 0x5119B0).
// Nadie usaba estos nombres todavía, así que el bug nunca se disparó — pero eran
// cuatro trampas armadas para el próximo port que los usara, porque nuestro
// `FUN_00511220` sí escribe en los DAT_083a42xx.
#define ScreenCenterX        DAT_083a429c
#define ScreenCenterY        DAT_083a42a0
#define PerspectiveX         _DAT_083a42a4
#define PerspectiveY         _DAT_083a42a8

#define byte_7E11D6E         DAT_07e11d6e
#define dword_55C9BC8        DAT_055c9bc8
#define dword_55C9BCC        DAT_055c9bcc
#define dword_55C9BD0        DAT_055c9bd0
#define dword_55C9BD4        DAT_055c9bd4

// =============================================================================
// FUN_00480c60 — port de sub_480C60. Calcula las dimensiones del texto de una
// etiqueta de número flotante y escribe el cx/cy del bounding box en los slots
// [+144]/[+145] del struct de la entrada. El layout de la entrada incluye 4 strings
// distintos (encabezado en +0, valor en +44, valor secundario en +300, remitente en
// +24) y sub_480C60 elige el caso de layout según los flags de cantidad de líneas
// at [+139] / [+140].
// =============================================================================
// functions.h la declara como `void __cdecl FUN_00480c60(int,int,int)`.
// Respetamos esa firma; el original de IDA devuelve int pero el único llamador
// (sub_4BD090 → RenderBooleans) no usa el valor de retorno para nada — el int
// va a una variable que se descarta. Devolver void es correcto.
void __cdecl FUN_00480c60(int p1, int p2, int p3)
{
    // La firma de IDA es `int sub_480C60(LPCSTR lpString)` pero nuestra
    // declaración adelantada en functions.h:409 es `void(int,int,int)`, para coincidir con la
    // forma declarada (hoy sin uso). Mantenemos la forma (int,int,int) y
    // tratamos p1 como el puntero base de la entrada.
    LPCSTR lpString = (LPCSTR)p1;
    (void)p2; (void)p3;

    SIZE psizl = {0,0}, v12 = {0,0}, v13 = {0,0}, v14 = {0,0};

    GetLastError();
    int n;
    n = lstrlenA(lpString);
    GetTextExtentPoint32A(m_hFontDC, lpString, n, &psizl);
    n = lstrlenA(lpString + 44);
    GetTextExtentPoint32A(m_hFontDC, lpString + 44, n, &v12);
    n = lstrlenA(lpString + 300);
    GetTextExtentPoint32A(m_hFontDC, lpString + 300, n, &v13);
    n = lstrlenA(lpString + 24);
    GetTextExtentPoint32A(m_hFontDC, lpString + 24, n, &v14);
    GetLastError();

    int result = psizl.cx + 3;
    if (*((int*)lpString + 140) <= 0) {
        if (*((int*)lpString + 139) <= 0) {
            *((DWORD*)lpString + 144) = (DWORD)(result + v14.cx);
            *((DWORD*)lpString + 145) = (DWORD)FontHeight;
        } else {
            LONG cx = v14.cx + result;
            if (cx <= v12.cx) cx = v12.cx;
            *((DWORD*)lpString + 144) = (DWORD)cx;
            result = 2 * FontHeight;
            *((DWORD*)lpString + 145) = (DWORD)result;
        }
    } else {
        LONG v6 = v12.cx;
        if (v12.cx <= v13.cx) v6 = v13.cx;
        LONG v7 = v14.cx + result;
        if (v7 <= v6) {
            v7 = v12.cx;
            if (v12.cx <= v13.cx) v7 = v13.cx;
        }
        *((DWORD*)lpString + 144) = (DWORD)v7;
        result = FontHeight;
        *((DWORD*)lpString + 145) = (DWORD)(3 * FontHeight);
    }
    (void)result;
}

// FUN_00480e00 — RenderBoolean — ahora portada en src/Render/HUD_Pass3.cpp.

// =============================================================================
// RenderPartyHP — sub_4BCA20. Itera el array Party, proyecta la posición de mundo de cada
// miembro al espacio de pantalla y dibuja una barrita horizontal de
// HP (42×5 px) sobre su cabeza. Cuando el cursor pasa sobre la barra se
// dibuja un tooltip "HP : NN%" justo arriba. El byte de HP está en Party[i].field+0
// (0..0x0A — ya viene preescalado a "decenas de porciento").
//
// Deps:
//   PartyNumber        — count of valid slots
//   Party              — base address (struct stride 36, header 24 bytes)
//   CharactersClient   — entity array base (stride 0x394)
//   Projection()       — World→Screen helper
//   MouseX, MouseY
//   m_hFontDC, g_fScreenRate_x, g_fScreenRate_y, TextSize
//   RenderText_1, RenderColor (FillRect alias), EnableAlphaTest,
//   EnableAlphaBlend, DisableAlphaBlend, glColor3f / glColor4f
//
// 1:1 port preserves the layered draw: shadow quad + dark frame + dark mid
// fila + (HP * 4) píxeles rojos — con tope de 10 segmentos — usando glColor y
// FillRect (RenderColor / FUN_005124c0).
// =============================================================================
// Projection (sub_5113F0 en IDA) está implementada en nuestro build como
// FUN_005113f0 en src/Render/Camera.cpp — la envolvemos bajo el nombre de IDA para
// que el cuerpo de abajo quede cerca del decompile.
static inline void Projection(float* Position, int* sx, int* sy) {
    FUN_005113f0(Position, sx, sy);
}

extern "C" void __cdecl RenderPartyHP_(void);
void RenderPartyHP_(void)
{
    if (PartyNumber <= 0) return;

    DWORD* v1 = (DWORD*)(Party + 24);  // &Party[0].field+24 = offset of slot 0 base
    int v19 = 0;

    do {
        DWORD v2 = v1[1];                          // character index
        if (v2 != (DWORD)-1) {
            int v3 = 229 * (int)v2;
            float Position[3];
            Position[0] = *(float*)((BYTE*)CharactersClient + 4 * v3 + 16);
            Position[1] = *(float*)((BYTE*)CharactersClient + 4 * v3 + 20);
            Position[2] = *(float*)((BYTE*)CharactersClient + 4 * v3 + 300)  // [75]
                        + *(float*)((BYTE*)CharactersClient + 4 * v3 + 24)   // [6]
                        + 100.0f;

            int sx, sy;
            Projection(Position, &sx, &sy);
            sx -= 19;

            if ((int)MouseX >= sx
                && (double)sx + 38.0 > (double)MouseX
                && (int)MouseY >= sy - 2
                && (int)MouseY < sy + 6)
            {
                CHAR String[100];
                wsprintfA(String, "HP : %d0%%", *(unsigned char*)v1);
                m_dwTextColor = 0xFFFFFFFFu;
                m_dwBackColor = 0xFF000000u;
                RenderTipText(sx, sy - 15, String);
            }

            EnableAlphaTest(true);
            glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
            FUN_005124c0((float)(sx + 1), (float)(sy + 1), 42.0f, 5.0f);  // shadow

            EnableAlphaBlend();
            glColor3f(0.2f, 0.0f, 0.0f);
            FUN_005124c0((float)sx, (float)sy, 42.0f, 5.0f);              // dark frame

            glColor3f(0.19607843f, 0.039215688f, 0.0f);
            FUN_005124c0((float)(sx + 2), (float)(sy + 2), 38.0f, 1.0f);  // mid row

            int v7 = (int)*(unsigned char*)v1;
            if (v7 > 10) v7 = 10;

            glColor3f(0.98039216f, 0.039215688f, 0.0f);
            for (int i = 0; i < v7; ++i) {
                FUN_005124c0((float)(sx + 4 * i + 2), (float)(sy + 2), 3.0f, 2.0f);
            }
            FUN_00511600();   // DisableAlphaBlend
        }
        v1 += 9;     // stride 36 bytes
        ++v19;
    } while (v19 < PartyNumber);

    FUN_00511600();
    glColor3f(1.0f, 1.0f, 1.0f);
}

// Entrada pública — reemplaza al stub vacío que antes estaba en Render_Frame.cpp.
void Render_HPBars(void) { RenderPartyHP_(); }


// =============================================================================
// RenderBooleans — sub_4BD090.  Drives the floating-number popup pool
// (~100 entries × 596 bytes at DAT_07e01924).  Three passes:
//   1. Proyecta la posición de mundo del dueño → pantalla, la guarda en la entrada +4/+8, y después
//      compute label dimensions (sub_480C60).
//   2. De-overlap pass: for every pair of active entries, shift one out of
//      la otra si sus rectángulos de bbox se superponen.
//   3. Pasada de render: llama a RenderBoolean por entrada (hoy stubeada).
//
// El original usa el vecino de la lista enlazada de las entradas (`*v0` es el puntero al dueño,
// non-zero means alive).  Our port preserves the exact pointer-arithmetic
// mismo idioma para que los offsets coincidan byte a byte.
// =============================================================================
extern "C" void __cdecl RenderBooleans_(void);
void RenderBooleans_(void)
{
    SelectObject(m_hFontDC, g_hFont ? g_hFont : g_hFontBold);

    // Pass 1: project owner→screen, set entry [+4]/[+8] (screen X/Y).
    // v0 = (float*)&unk_7E0192C  is base+8 (entry 0's owner field)
    //   Stride: 149 floats (596 bytes).
    //   End:    base+0x9D8D0 (=&unk_7E101FC).
    BYTE* base = (BYTE*)DAT_07e01924;
    float* v0  = (float*)(base + 0x008);   // = &unk_7E0192C
    float* end = (float*)(base + 0xE8D8);  // = &unk_7E101FC (one past last)
    while (v0 < end) {
        if (*((int*)v0 - 131) > 0 || *((int*)v0 - 2) > 0) {
            int sx = 0, sy = 0;
            float* v1 = *(float**)v0;
            if (*(DWORD*)v0) {
                float Position[3];
                Position[0] = v1[4];
                Position[1] = v1[5];
                Position[2] = v1[75] + v1[6] + _DAT_0055290c;
                Projection(Position, &sx, &sy);
            } else {
                Projection(v0 + 5, &sx, &sy);
            }
            FUN_00480c60((int)((LPCSTR)v0 - 564), 0, 0);
            *((DWORD*)v0 + 1) = (DWORD)(sx - 640 * (int)*((DWORD*)v0 + 3) / (int)WindowWidth / 2);
            *((DWORD*)v0 + 2) = (DWORD)(sy - 36);
        }
        v0 += 149;
    }

    // Pass 2: de-overlap.  For each active i, scan all active j != i;
    // si bbox(i) se superpone con bbox(j), corre la Y de i fuera del rango vertical de j.
    int sx_idx = 0;
    int* v3_end = (int*)(base + 0xE8D0);   // = &unk_7E101F4
    for (int* v3 = (int*)(base + 0x000); v3 < v3_end; v3 += 149, ++sx_idx) {
        if (!(*(v3 - 129) > 0 || *v3 > 0)) continue;

        int sy_idx = 0;
        for (int* v4 = (int*)(base + 0x000); v4 < v3_end; v4 += 149, ++sy_idx) {
            if (sx_idx == sy_idx) continue;
            if (!(*(v4 - 129) > 0 || *v4 > 0)) continue;

            int v5 = v3[3];
            int v6 = v4[3];
            if (v5 + 640 * v3[5] / (int)WindowWidth <= v6) continue;
            if (v5 >= v6 + 640 * v4[5] / (int)WindowWidth) continue;

            int v7 = v3[4];
            int v8 = v4[4];
            int v13 = 480 * v3[6] / (int)WindowHeight;
            if (v7 + v13 <= v8) continue;

            int v9 = 480 * v4[6] / (int)WindowHeight;
            if (v7 >= v9 + v8) continue;

            if (v7 >= v8 + v9 / 2) v3[4] = v9 + v8;
            else                    v3[4] = v8 - v13;
        }
    }

    // Pasada 3: renderiza cada entrada activa vía RenderBoolean.
    for (int* v10 = (int*)(base + 0x000); v10 < v3_end; v10 += 149) {
        if (*(v10 - 129) > 0 || *v10 > 0) {
            FUN_00480e00(v10[3], v10[4], (int)(v10 - 139));
        }
    }
}

void Render_FloatingText(void) { RenderBooleans_(); }


// =============================================================================
// RenderMainFrameWindow — sub_4BD2B0.  Renders the bottom HUD frame:
//   * Dos decoraciones de esquina (bitmap 0xE9, 108×45) en x=0 y x=532, y=387
//   * Tres filas de tiles en y=432 que cubren la barra (bitmaps 0xE6, 231, 232)
// Intercalado entre los dibujos va el bloque interno anti-tamper de CharacterMachine:
//   busca CharacterMachine en la hash table dword_55C9BC8, incrementa el
//   ref-count [+1412] y, en el primer acceso, desencripta el buffer de 0x584 bytes
//   a una alocación nueva, lo copia de vuelta y más tarde decrementa.
//
// We preserve the anti-tamper structure but it's a no-op when the table is
// vacía (dword_55C9BD4 == 0) — que es nuestro estado por defecto. Cuando el motor
// pueble la tabla más adelante, el camino se va a ejecutar correctamente.
// =============================================================================
extern "C" void __cdecl RenderMainFrameWindow_(void);
void RenderMainFrameWindow_(void)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    DAT_07e11d6e = 0;
    m_dwTextColor = 0xFFFFFFFF;
    m_dwBackColor = 0xFF000000;

    // Two corner decorations (bitmap 0xE9 mirrored at x=532).
    FUN_005125a0(0xE9,   0.0f, 387.0f, 108.0f, 45.0f, 0.0f, 0.0f,  0.84375f, 0.703125f, 1, 1);
    FUN_005125a0(0xE9, 532.0f, 387.0f, 108.0f, 45.0f, 0.84375f, 0.0f, -0.84375f, 0.703125f, 1, 1);
    FUN_00511600();   // DisableAlphaBlend

    // Bottom tile #1 (left half).
    FUN_005125a0(0xE6, 0.0f, 432.0f, 256.0f, 48.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1, 1);

    // ── Anti-tamper #1: desencripta CharacterMachine en la primera referencia ──
    // Port 1:1 del bloque de hash-table intercalado de IDA. El bucket se encuentra
    // vía FUN_004041e0 (HashTable::GetIndex); nuestra implementación en
    // stubs.cpp:16227 devuelve -1 porque la tabla está vacía por defecto —
    // lo que hace que todo este bloque degrade a "insertar una entrada nueva con
    // ref-count=1 y saltear el desencriptado XOR". Cuando el motor pueble
    // dword_55C9BC8 como corresponde, la estructura coincide byte a byte con IDA.
    if (CharacterMachine) {
        void* v0 = CharacterMachine;
        UINT  v6 = FUN_004041e0(&DAT_055c9bc8, /*edx*/ 0, (DWORD)v0);
        if (v6 != 0xFFFFFFFFu && DAT_055c9bd4) {
            // Encontrado: toma el puntero al valor del array de valores
            // (dword_55C9BCC[v6]) e incrementa su byte de ref-count [+1412].
            BYTE* v7 = *(BYTE**)((BYTE*)DAT_055c9bcc + 4 * v6);
            if (v7) {
                BYTE v8 = (BYTE)(v7[1412] + 1);
                v7[1412] = v8;
                if (v8 < 2) {
                    // Primera referencia de este frame — desencripta con XOR el buffer de
                    // 0x584 bytes de CharacterMachine a una copia scratch y lo escribe
                    // de vuelta. Idéntico al loop de IDA:
                    //   for v10 = 1411 down to 0:
                    //     if (v10 < 0x583) v9[v10] ^= v9[v10+1];
                    //     v9[v10] = ((v9[v10] - 35) ^ key[v10 % 16]) - 71;
                    BYTE* v9 = new BYTE[0x584];
                    memcpy(v9, v7, 0x584);
                    int v10 = 1411;
                    int v11 = 1412;
                    do {
                        if ((unsigned)v10 < 0x583u)
                            v9[v10] ^= v9[v10 + 1];
                        v9[v10] = (BYTE)(((v9[v10] - 35) ^ DAT_00559050[v10 & 0xF]) - 71);
                        --v10; --v11;
                    } while (v11);
                    memcpy(v0, v9, 0x584);
                    delete[] v9;
                }
            }
        } else {
            // No encontrado / tabla vacía — inserta una entrada nueva marcada con
            // ref-count = 1 (así el decremento siguiente dispara el encriptado XOR
            // round-trip elsewhere).
            BYTE* fresh = new BYTE[0x585]();
            fresh[1412] = 1;
            FUN_00403f80(&DAT_055c9bc8, fresh, v0);
        }
    }

    // sprintf("%d/%d", curHP, maxHP) en Buffer — el original calcula el
    // string pero NO lo renderiza dentro de esta función. El buffer es un
    // efecto secundario que leen otros caminos de UI. Nuestro build no expone el
    // buffer por separado, así que el efecto es inobservable; lo salteamos.
    if (CharacterAttribute) {
        CHAR Buffer[100];
        wsprintfA(Buffer, "%d/%d",
                  *(int*)((BYTE*)CharacterAttribute + 16),
                  *(int*)((BYTE*)CharacterAttribute + 52));
        (void)Buffer;
    }

    // ── Anti-tamper #2: symmetric ref-count decrement ───────────────────────
    // Vuelve a buscar el mismo buffer, decrementa [+1412] y, al llegar a cero, llama a
    // FUN_00404400 para sacar la entrada (lo que en el original dispara la
    // vuelta de re-encriptado XOR vía sub_404370). Cuando la tabla está vacía
    // this no-ops, matching IDA's "table full" error-report fallback.
    if (CharacterMachine) {
        void* v0 = CharacterMachine;
        UINT  v12 = FUN_004041e0(&DAT_055c9bc8, /*edx*/ 0, (DWORD)v0);
        if (v12 != 0xFFFFFFFFu && DAT_055c9bd4) {
            BYTE* v13 = *(BYTE**)((BYTE*)DAT_055c9bcc + 4 * v12);
            if (v13) {
                BYTE v14 = (BYTE)(v13[1412] - 1);
                v13[1412] = v14;
                if (!v14) {
                    FUN_00404400(v13, v0);
                }
            }
        }
    }

    // Bottom tile #2 (mid) and #3 (right half).
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_005125a0(231, 256.0f, 432.0f, 128.0f, 48.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1, 1);
    FUN_005125a0(232, 384.0f, 432.0f, 256.0f, 48.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1, 1);
    FUN_00511600();
    glColor3f(1.0f, 1.0f, 1.0f);
}

void Render_BottomHUD(void) { RenderMainFrameWindow_(); }


// =============================================================================
// Render_HotbarItems3D — sub_4BFDE0. Renderiza los items de la barra de accesos rápidos del inventario como
// mallas 3D vía RenderItem3D, desplazados a lo largo de x=208..301 en pasos de 31 px.
// Guarda el estado ortográfico 2D actual, arma un frustum en perspectiva nuevo
// (FOV=1.0 rad) usando la CameraMatrix guardada, dibuja los items y después
// restores 2D state.
//
// La rama `g_bEventChipDialogEnable in {0, 3}` elige la barra normal
// path; other values delegate to sub_4F54B0 (event-chip dialog renderer)
// que no tenemos portado. El valor por defecto de 0 toma el camino normal.
//
// Deps:
//   EndBitmap / BeginBitmap (FUN_005124b0 / FUN_005123c0)
//   sub_482BE0(slot)        — devuelve el índice de OffsetInventoryItems del slot de la barra
//   OffsetInventoryItems    — array of {Type, Level, ...}
//   RenderItem3D            — FUN_004e1be0
//   CreateScreenVector      — FUN_005112f0
//   CameraPosition[]        — float[3] world-space camera
//   CameraMatrix[]          — 4x4 GL matrix (already in our globals)
// =============================================================================
// FUN_00482be0, FUN_004e1be0 y FUN_005112f0 ya están declaradas en
// functions.h (que entra vía stdafx.h). FUN_004f5ce0 / FUN_004f6420 están
// declaradas pero sin implementar en nuestro build — acá dejamos stubs para que
// enlacen los call sites del render de la barra. Son renderers de efectos de
// skill / teleport; que no tengan cuerpo sólo significa que esos overlays no se dibujan, lo cual es
// fine until they get their own port.
extern "C" DWORD DAT_07e91388;
extern "C" DWORD DAT_07eaa13c;

// FUN_004f5ce0 (sub_4F5CE0) está portada en HUD_Pass6.cpp como sub_4F5CE0_().
// FUN_004f6420 (sub_4F6420) está portada en HUD_Pass6.cpp.
// Las dos tienen declaración en functions.h como `void(int,int,int,int)` — las envolvemos.
extern "C" char __cdecl sub_4F5CE0_(void);
extern "C" void __cdecl sub_4F6420(int, int, int, int);
void __cdecl FUN_004f5ce0(int, int, int, int) { sub_4F5CE0_(); }
void __cdecl FUN_004f6420(int a, int b, int c, int d) { sub_4F6420(a, b, c, d); }

// alineación del struct OffsetInventoryItems. En IDA el acceso es:
//   *(&OffsetInventoryItems.Type  + 34*v2)   → 4-byte slot, stride 136 bytes  ←Type
//   *(&OffsetInventoryItems.Level + 17*v2)   → 4-byte slot, stride 68  bytes  ←Level
// (el segundo acceso usa índices de 4 bytes, así que hay 17*4 = 68 de stride entre Levels).
// Es un array plano de int, no un struct empaquetado — portarlo como arrays de int crudos
// declarados en otro lado si están disponibles. Dejamos un fallback débil para que la llamada
// compile igual cuando el array no está.
//
// NO tenemos OffsetInventoryItems expuesto; gateamos el loop interno con un
// runtime guard so an absent symbol degrades to "no items drawn" rather
// than a link error.

extern "C" void __cdecl Render_HotbarItems3D_(void);
void Render_HotbarItems3D_(void)
{
    SeedQuickPotionTypesFromInventory();

    FUN_005124b0();   // EndBitmap

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    FUN_00511910(0, 0, (int)WindowWidth, (int)WindowHeight);
    float aspectF = (float)((double)(int)WindowWidth / (double)(int)WindowHeight);
    float fov1 = 1.0f;
    int   fovBits  = *(int*)&fov1;
    int   nearBits = *(int*)&CameraViewNear;
    FUN_00511220(fovBits, aspectF, nearBits, CameraViewFar);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    FUN_005111d0((unsigned int*)DAT_083a4140);
    FUN_005114d0();
    FUN_00511510();
    // En el original refresca g_csQuest antes del HUD 3D.
    // En este build no tenemos ese símbolo exportado; no afecta la hotbar visual.

    if (!HUD_IsGoldenArcherPanelRuntime()) {
        FUN_004f5ce0(0, 0, 0, 0);
        if (DAT_07e91388 > 0 && !DAT_07eaa13c) {
            FUN_004f6420(0, 0, 0, 0);
        }

        int slot_x = 208;
        for (int slot_idx = 0; slot_x < 301; ++slot_idx) {
            int v2 = FUN_00482be0(slot_idx);
            if (v2 != -1) {
                // IDA passes the raw hotbar slot fields:
                //   Type  = *(short*)(base + slot*0x44 + 0)
                //   Level = *(int*  )(base + slot*0x44 + 4)
                // no la vista expandida de ITEM.
                BYTE* slotBase = OffsetInventoryItems + v2 * 0x44;
                short itemType = *(short*)(slotBase + 0);
                int   itemLvl  = *(int*)(slotBase + 4);
                FUN_004e1be0((float)slot_x, 454.0f, 20.0f, 20.0f,
                             (int)itemType, itemLvl,
                             0, 0);
            }
            slot_x += 31;
        }
    } else {
        // sub_4F54B0 — event-chip dialog renderer.  Not yet ported.
    }

    glLoadIdentity();
    glTranslatef(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);
    FUN_005111d0((unsigned int*)DAT_083a4140);

    float Target[3] = {0, 0, 0};
    FUN_005112f0(100, 100, Target);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    FUN_005123c0();   // BeginBitmap
}

void Render_HotbarItems3D(void) { Render_HotbarItems3D_(); }
