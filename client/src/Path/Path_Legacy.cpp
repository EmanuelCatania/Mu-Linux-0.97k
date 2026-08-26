// Path_Legacy.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// ── PF_AStar — sustituto del PATH::FindPath original (0x0043F500) ────────────
//
// 2026-08-17 — CRITERIO DE BLOQUEO CORREGIDO CONTRA EL BINARIO.
//
// El A* de este port decidía el bloqueo con una MÁSCARA DE BITS inventada:
//     return (a & 0x0e) != 0;      // "mask correcto verificado contra Terrain1.att"
// El original NO usa máscara. `PATH::FindPath` @ 0x0043F500 hace una COMPARACIÓN
// NUMÉRICA contra el parámetro `iWall`, restando antes el bit 0x20 si está puesto:
//
//     uVar5 = TerrainWall[idx];
//     if ((TerrainWall[idx] & 0x20) == 0x20) uVar5 = uVar5 - 0x20;
//     if (((visited[idx] & 1) == 0) && ((int)uVar5 < iWall)) { ...expandir vecino... }
//
// Con iWall=2 ambos criterios coinciden para los attrs comunes (0,1 pasan; 2,3,4,5
// bloquean), y por eso el bug pasó desapercibido — pero DIVERGEN en los bits altos:
// 0x10, 0x40 y 0x80 pasan la máscara 0x0e (`a & 0x0e == 0` → "libre") y en cambio
// el original los bloquea (0x10 < 2 es falso). De ahí que el héroe caminara por
// encima de terreno prohibido.
//
// El chequeo del destino cuando bErrorCheck=true también es del binario, y ahí el
// attr va CRUDO (sin restar 0x20), exigiendo que el bit 0x20 no esté puesto:
//     if ((bErrorCheck) && (iWall <= TerrainWall[dst]) &&
//         ((TerrainWall[dst] & 0x20) != 0x20)) return false;
//
// Con bErrorCheck=false el original no falla si no alcanza el destino: se queda con
// el mejor nodo alcanzado (`local_18`/`local_14`) y devuelve el camino hasta ahí.
// La métrica de "mejor" es la del propio binario:
//     dx=|x-xEnd|, dy=|y-yEnd|; m = (dx==1 && dy==1) ? 0 : min(dx,dy);
//     coste = (|dx-dy| * 0xf + 3 + m * 0x15) >> 2;
//
// Se elimina de paso la búsqueda de "walkable más cercano en radio 3" que había
// aquí: era invención del port. El original falla y deja que PathFinding2 reintente
// con otro iWall, que es lo que se replica en FUN_0043f3e0.
//
// 2026-08-17 (b) — `fDistance` (el `radius` de PathFinding2) TAMPOCO se usaba.
// En el binario ese parámetro llega a FindPath como `Value` y parte la función en
// dos ramas bien distintas:
//     if (_Value == 0.0) { ...un único destino: marca[dst] = 4... }
//     else               { ...recorre el DISCO de radio Value alrededor del destino
//                            y marca = 4 toda celda con SQRT(dx*dx+dy*dy) < Value... }
// y la búsqueda termina al alcanzar CUALQUIER celda marcada con 4. O sea: con
// fDistance > 0 basta con acercarse al destino, no hace falta pisarlo.
// El combate (Combat.cpp:1053 y siguientes) llama siempre con `skillRange`, así que
// ignorar el parámetro hacía que el héroe intentara pisar la casilla exacta del
// objetivo — la que suele estar ocupada por el propio mob. Nótese además que el
// chequeo estricto del destino sólo existe en la rama `_Value == 0.0`.
static int PF_AStar(int sx, int sy, int tx, int ty, int iWall, bool bErrorCheck,
                    unsigned char* path, float fDistance)
{
    const unsigned char* attr = (const unsigned char*)&DAT_0838bc70;

    if (sx < 0 || sx >= 256 || sy < 0 || sy >= 256 ||
        tx < 0 || tx >= 256 || ty < 0 || ty >= 256) return 0;
    if (sx == tx && sy == ty) return 0;

    // Bloqueo por comparación numérica, igual que el binario.
    auto walkable = [&](int x, int y) -> bool {
        if (x < 0 || x >= 256 || y < 0 || y >= 256) return false;
        unsigned int a = attr[y * 256 + x];
        if ((a & 0x20) == 0x20) a -= 0x20;
        return (int)a < iWall;
    };
    // Métrica de cercanía al destino del original (para el mejor esfuerzo).
    auto origCost = [&](int x, int y) -> int {
        int dx = x > tx ? x - tx : tx - x;
        int dy = y > ty ? y - ty : ty - y;
        int m  = (dx == 1 && dy == 1) ? 0 : (dx < dy ? dx : dy);
        int d  = dx > dy ? dx - dy : dy - dx;
        return (d * 0xf + 3 + m * 0x15) >> 2;
    };

    // Meta: con fDistance == 0 es la casilla exacta; con fDistance > 0, cualquier
    // casilla dentro del disco de ese radio (rama `else` de FindPath).
    const bool bRadius = (fDistance > 0.0f);
    auto isGoal = [&](int x, int y) -> bool {
        if (!bRadius) return (x == tx && y == ty);
        int dx = x - tx, dy = y - ty;
        return ((float)(dx * dx + dy * dy)) < (fDistance * fDistance);
    };

    // Chequeo estricto del destino: sólo existe en la rama `_Value == 0.0` del
    // binario, y ahí el attr va CRUDO (sin restarle 0x20).
    if (bErrorCheck && !bRadius) {
        unsigned int aDst = attr[ty * 256 + tx];
        if ((int)aDst >= iWall && (aDst & 0x20) != 0x20) return 0;
    }

    static unsigned char  s_cameDir[65536];
    static unsigned short s_gScore[65536];
    struct AstarNode { unsigned short f; unsigned short idx; };
    const int HEAP_CAP = 8192;
    static AstarNode s_heap[HEAP_CAP];
    // 2026-08-17 — tope de iteraciones tomado del binario. FindPath hace:
    //     iVar10 = (-(uint)(bErrorCheck != false) & 0x1c2) + 0x32;
    // o sea 0x1c2+0x32 = 500 con bErrorCheck, y 0x32 = 50 sin él. Acá había 4096
    // fijo para ambos: contra una pared el A* barría 4096 nodos POR TICK
    // (visible en debug.log como "explored=4096" repetido).
    const int MAX_NODES = bErrorCheck ? 500 : 50;

    memset(s_cameDir, 0xFF, sizeof(s_cameDir));
    for (int i = 0; i < 65536; ++i) s_gScore[i] = 0xFFFF;
    int heapSize = 0;

    auto heapPush = [&](AstarNode n) {
        if (heapSize >= HEAP_CAP) return;
        int i = heapSize++;
        s_heap[i] = n;
        while (i > 0) {
            int p = (i - 1) / 2;
            if (s_heap[p].f <= s_heap[i].f) break;
            AstarNode t = s_heap[p]; s_heap[p] = s_heap[i]; s_heap[i] = t;
            i = p;
        }
    };
    auto heapPop = [&]() -> AstarNode {
        AstarNode top = s_heap[0];
        s_heap[0] = s_heap[--heapSize];
        int i = 0;
        while (true) {
            int l = i*2 + 1, r = i*2 + 2, sm = i;
            if (l < heapSize && s_heap[l].f < s_heap[sm].f) sm = l;
            if (r < heapSize && s_heap[r].f < s_heap[sm].f) sm = r;
            if (sm == i) break;
            AstarNode t = s_heap[sm]; s_heap[sm] = s_heap[i]; s_heap[i] = t;
            i = sm;
        }
        return top;
    };
    auto heuristic = [&](int x, int y) -> int {
        int adx = x > tx ? x - tx : tx - x;
        int ady = y > ty ? y - ty : ty - y;
        int dmin = adx < ady ? adx : ady;
        int dmax = adx < ady ? ady : adx;
        return 14 * dmin + 10 * (dmax - dmin);
    };

    int sIdx = sy * 256 + sx;
    int tIdx = ty * 256 + tx;
    s_gScore[sIdx] = 0;
    s_cameDir[sIdx] = 0xFE;               // marca visitado (ni 0..7 ni 0xFF)
    heapPush(AstarNode{ (unsigned short)heuristic(sx, sy), (unsigned short)sIdx });

    const int DX[8] = { 1, 1, 1, 0, -1, -1, -1,  0 };
    const int DY[8] = {-1, 0, 1, 1,  1,  0, -1, -1 };
    const int COST[8] = { 14, 10, 14, 10, 14, 10, 14, 10 };

    int explored = 0;
    bool found = false;
    int goalIdx = tIdx;
    int bestIdx = sIdx, bestCost = origCost(sx, sy);
    if (isGoal(sx, sy)) return 0;          // ya estamos dentro del radio
    while (heapSize > 0 && explored < MAX_NODES) {
        AstarNode cur = heapPop();
        int cx0 = cur.idx & 0xFF, cy0 = cur.idx >> 8;
        if (isGoal(cx0, cy0)) { found = true; goalIdx = cur.idx; break; }
        int cx = cur.idx & 0xFF;
        int cy = cur.idx >> 8;
        int curG = s_gScore[cur.idx];
        ++explored;
        if (!bErrorCheck) {                // mejor esfuerzo, como el original
            int c = origCost(cx, cy);
            if (c < bestCost) { bestCost = c; bestIdx = cur.idx; }
        }
        for (int d = 0; d < 8; ++d) {
            int nx = cx + DX[d];
            int ny = cy + DY[d];
            if (!walkable(nx, ny)) continue;
            if ((d & 1) == 0) {            // no cortar esquinas en diagonal
                if (!walkable(cx + DX[d], cy) && !walkable(cx, cy + DY[d])) continue;
            }
            int newG = curG + COST[d];
            int nIdx = ny * 256 + nx;
            if (newG < s_gScore[nIdx]) {
                s_gScore[nIdx] = (unsigned short)(newG > 0xFFFF ? 0xFFFF : newG);
                s_cameDir[nIdx] = (unsigned char)d;
                int f = newG + heuristic(nx, ny);
                if (f > 0xFFFF) f = 0xFFFF;
                heapPush(AstarNode{ (unsigned short)f, (unsigned short)nIdx });
            }
        }
    }

    int endIdx;
    if (found) {
        endIdx = goalIdx;
    } else if (!bErrorCheck && bestIdx != sIdx) {
        endIdx = bestIdx;                  // camino parcial hacia el destino
    } else {
        static int s_pflogF = 0;
        if (s_pflogF++ < 30) {
            char b[200];
            wsprintfA(b, "A* FAIL #%d: src=(%d,%d) dst=(%d,%d) iWall=%d rad=%d errChk=%d explored=%d",
                      s_pflogF, sx, sy, tx, ty, iWall, (int)fDistance, (int)bErrorCheck, explored);
            DbgLogPublic(b);
        }
        // 2026-08-17 (c) — NO tocar `path` al fallar. Ver nota de abajo.
        return 0;
    }

    // Backtrack COMPLETO destino → origen (ver BUG-FIX del truncado, abajo el
    // bucle de emisión se queda con los 15 primeros contados desde el origen).
    static unsigned char wpX[4096], wpY[4096];
    int wpCount = 0;
    int curIdx = endIdx;
    while (curIdx != sIdx && wpCount < 4096) {
        wpX[wpCount] = (unsigned char)(curIdx & 0xFF);
        wpY[wpCount] = (unsigned char)(curIdx >> 8);
        ++wpCount;
        unsigned char dir = s_cameDir[curIdx];
        if (dir > 7) break;                // 0xFE (origen) o 0xFF (no visitado)
        int prevX = (curIdx & 0xFF) - DX[dir];
        int prevY = (curIdx >> 8)   - DY[dir];
        if (prevX < 0 || prevX >= 256 || prevY < 0 || prevY >= 256) break;
        curIdx = prevY * 256 + prevX;
    }

    // 2026-08-17 (c) — CAUSA RAÍZ de "atraviesa la pared y no para".
    //
    // Este bloque escribía directo sobre `path` (= entidad+0x354) y, en los
    // caminos de fallo, lo dejaba en cero: `path[0]=0; path[1]=0; path[2]=0;`.
    // path[2] es PATH_t.PathNum — o sea que un pathfind fallido BORRABA el
    // camino que la entidad ya venía siguiendo, sin tocar Movement (+0x2EC).
    //
    // Y ese estado (Movement=1, PathNum=0) es una deriva infinita, también en el
    // original: MovePath @ 0x0043EA20 abre con
    //     if ((c->Path).PathNum <= (c->Path).CurrentPath) return false;
    // y MoveHero, ante ese false, llama MoveCharacterPosition, que avanza la
    // posición en línea recta según el Angle actual — sin mirar terreno ni path.
    // Nunca se alcanza el `if (MovePath(...))` que hace Movement=0 + SetPlayerStop.
    //
    // El original NUNCA cae ahí porque PathFinding2 @ 0x0043F3E0 no toca el buffer
    // cuando falla: `a` sólo se escribe pasado LAB_0043f483, en la rama de éxito;
    // los dos `return false` salen con el camino anterior intacto.
    //
    // Se hizo visible al arreglar el hold: el recálculo por tick contra una pared
    // falla una y otra vez, y cada fallo borraba el PathNum del camino en curso.
    // Evidencia en debug.log: `wp=0/2 2ec=1` y al tick siguiente `wp=0/0 2ec=1`,
    // con la posición avanzando en línea recta a paso constante.
    //
    // Ahora se arma todo en un temporal y `path` sólo se escribe si hay éxito.
    unsigned char tmpX[16], tmpY[16];
    tmpX[0] = (unsigned char)sx;
    tmpY[0] = (unsigned char)sy;
    int validSteps = 1;
    for (int i = wpCount - 1; i >= 0 && validSteps < 15; --i) {
        tmpX[validSteps] = wpX[i];
        tmpY[validSteps] = wpY[i];
        ++validSteps;
    }
    if (validSteps <= 1) return 0;      // sin tocar `path`
    for (int i = 0; i < validSteps; ++i) {
        path[3 + i]    = tmpX[i];
        path[0x12 + i] = tmpY[i];
    }
    path[0] = 0;
    path[1] = 0;
    path[2] = (unsigned char)validSteps;
    {
        static int s_pflog = 0;
        if (s_pflog++ < 30) {
            char b[176];
            wsprintfA(b, "A* OK #%d: src=(%d,%d) dst=(%d,%d) iWall=%d rad=%d wp=%d explored=%d%s",
                      s_pflog, sx, sy, tx, ty, iWall, (int)fDistance, validSteps, explored,
                      found ? "" : " [parcial]");
            DbgLogPublic(b);
        }
    }
    return 1;
}

// FUN_0043f3e0 @ 0x0043F3E0 — PathFinding2(sx, sy, tx, ty, path_buf, radius)
// Calls A* solver (FUN_0043f500). On fail, checks terrain walk flags at src/dst
// to decide filter mode (2 or 4) and retries. On success (path_len >= 2),
// copies waypoints from DAT_05826df4 result buffer into path_buf.
// path_buf layout: [0]=0, [1]=0, [2]=wp_count, [3..17]=wp_x, [0x12..0x20]=wp_y.
unsigned int __cdecl FUN_0043f3e0(int sx, int sy, int tx, int ty,
                                   unsigned char* path, float radius)
{
    int filterMode = 2;
    // BUG-FIX 2026-04-26 (audit #4): _this debe ser el contexto del pathfinder
    // (DAT_05826df4), no `sx`. Net_Process.cpp documenta el wrapper:
    //   FUN_0043f500(DAT_05826df4, id, t, x, y, 1, 2, t)
    // Antes pasábamos `(void*)sx` → la función deref-eaba un coord como ptr.
    void* pfCtx = (void*)(intptr_t)DAT_05826df4;

    // 2026-08-17: el contexto YA se construye completo. Antes se reservaba en
    // WinMain con `malloc(0x420)` + memset y el vtable de la cola de prioridad
    // (+0x414) quedaba NULL, así que FUN_0043f500 (PATH::FindPath) crasheaba al
    // dereferenciarlo — de ahí el `pfReady = false` forzado desde 2026-05-03.
    // Ahora PathContext_Create() (src/Game/PathFinder.cpp, llamada desde WinMain)
    // replica el ctor del binario: 0x0043F280..0x0043F2C7, reserva de 0x424 bytes
    // -no 0x420- y vtable en +0x414. InitPath (FUN_0043f2d0, mas abajo en este
    // mismo archivo) ya estaba portada y la llama FUN_0050f690 (World_Init), igual
    // que en el binario; corre despues del ctor, que es el orden correcto.
    //
    // El camino original queda detrás de un switch porque FUN_0043f500 todavía
    // no se ejercitó en runtime: nuestro A* sustituto sigue siendo el default.
    // Poner PF_USE_ORIGINAL en 1 para usar el algoritmo del binario.
    // 2026-08-17: probado en runtime con 1 → CRASH inmediato en la primera llamada
    // (0xC0000005 leyendo 0x63082BFC). El contexto se construye bien -el log
    // muestra `pfCtx check #1: vtbl@0x414=0x6960b4 pfReady=1`-, asi que el ctor
    // esta ok y el problema esta dentro de la propia portacion de FUN_0043f500.
    // Queda en 0 hasta auditar esa funcion contra el decompile. Ver DESCOBERTAS.md.
    #define PF_USE_ORIGINAL 0
    bool pfReady = false;
#if PF_USE_ORIGINAL
    pfReady = (pfCtx != nullptr) && (((DWORD*)pfCtx)[0x105] != 0);
#endif
    {
        static int s_dbg_pf = 0;
        if (s_dbg_pf++ < 5) {
            char b[200];
            DWORD vtbl = 0;
            if (pfCtx && (uintptr_t)pfCtx >= 0x100000) {
                vtbl = ((DWORD*)pfCtx)[0x105];
            }
            wsprintfA(b, "pfCtx check #%d: pfCtx=%p vtbl@0x414=0x%x pfReady=%d",
                      s_dbg_pf, pfCtx, (unsigned)vtbl, (int)pfReady);
            DbgLogPublic(b);
        }
    }
    if (!pfReady) {
        // Replica de ZzzAI::PathFinding2 @ 0x0043F3E0, con nuestro A* (PF_AStar)
        // en lugar de PATH::FindPath (0x0043F500), cuyo ctor de PriorityQueue no
        // esta portado.  El original hace:
        //
        //   local_4 = 2;
        //   if (PATH__FindPath(sx,sy,tx,ty, true, 2, ...)) goto ok;
        //   if ((TerrainWall[src] & 1) == 1 || (TerrainWall[dst] & 1) == 1) {
        //       iVar3 = 4;
        //       if ((TerrainWall[dst] & 2) == 2) iVar3 = local_4;   // vuelve a 2
        //   } else  iVar3 = local_4;                                 // = 2
        //   if (!PATH__FindPath(sx,sy,tx,ty, false, iVar3, ...)) return false;
        //
        // O sea: primer intento estricto (bErrorCheck=true, iWall=2) y, si falla,
        // segundo intento a mejor esfuerzo (bErrorCheck=false) con iWall=4 SOLO
        // cuando origen o destino tienen el bit 0 y el destino NO tiene el bit 1.
        if (!path) return 0;

        if (PF_AStar(sx, sy, tx, ty, 2, true, path, radius)) return 1;

        const unsigned char* tw = (const unsigned char*)&DAT_0838bc70;
        int iWall2 = 2;
        if (sx >= 0 && sx < 256 && sy >= 0 && sy < 256 &&
            tx >= 0 && tx < 256 && ty >= 0 && ty < 256) {
            unsigned char aSrc = tw[sy * 256 + sx];
            unsigned char aDst = tw[ty * 256 + tx];
            if ((aSrc & 1) == 1 || (aDst & 1) == 1) {
                iWall2 = 4;
                if ((aDst & 2) == 2) iWall2 = 2;
            }
        }
        return PF_AStar(sx, sy, tx, ty, iWall2, false, path, radius) ? 1 : 0;
    }

    // First attempt: strict walkable filter
    unsigned int found = FUN_0043f500(pfCtx, sx, (float)sy, tx, ty, 1, 2, radius);
    if (found) goto success;

    // Check terrain flags at src and dst
    {
        int srcAttr = FUN_004f6c40((unsigned int)sx, (unsigned int)sy);
        int dstAttr = FUN_004f6c40((unsigned int)tx, (unsigned int)ty);
        // 2026-08-17: estaba INVERTIDO respecto del binario. En PathFinding2 el
        // filtro sube a 4 cuando origen/destino tienen el bit 0, y vuelve a 2 sólo
        // si el destino tiene además el bit 1:
        //     iVar3 = 4;  if ((TerrainWall[dst] & 2) == 2) iVar3 = local_4 /*2*/;
        // Acá se hacía al revés (subía a 4 justo cuando el bit 1 estaba puesto).
        // Camino muerto hoy (pfReady siempre false), pero queda alineado.
        if ((DAT_0838bc70[srcAttr] & 1) == 1 || (DAT_0838bc70[dstAttr] & 1) == 1) {
            int dstAttr2 = FUN_004f6c40((unsigned int)tx, (unsigned int)ty);
            filterMode = 4;
            if ((DAT_0838bc70[dstAttr2] & 2) == 2) {
                filterMode = 2;
            }
        }
    }

    // Second attempt: relaxed filter
    found = FUN_0043f500(pfCtx, sx, (float)sy, tx, ty, 0, filterMode, radius);
    if (!found) return 0;

success:
    {
        int pathLen = *(int*)(DAT_05826df4 + 0x10);
        if (pathLen < 2) return 0;
        if (pathLen > 0xF) pathLen = 0xF;

        path[2] = (unsigned char)pathLen;

        // Copy waypoints from result buffer into path arrays
        int base = DAT_05826df4 - *(int*)(DAT_05826df4 + 0x10);
        for (int i = 0; i < (int)(unsigned char)path[2]; i++) {
            path[3 + i]    = *(unsigned char*)(base + 0x208 + i); // wp_x
            path[0x12 + i] = *(unsigned char*)(base + 0x3FC + i); // wp_y
        }
        path[0] = 0;
        path[1] = 0;
        return 1;
    }
}
// FUN_0043e370 @ 0x0043E370 — FarAngle(curAngle, tgtAngle, mode)
// Returns signed angular difference between two angles, handling wrap-around at 360.
// If mode==1, returns absolute value (unsigned distance).
//
// BUG-FIX 2026-04-26 (audit #1): la decompilación IDA emitió `if (v6)` con `v6`
// como flag FPU x87 (`c0`) sin reconstruir → undefined branch. El asm hace
// `fcom a2, a1` (comparando a2 con a1) ANTES del `fsub` → v6 representa
// `a2 > a1`, no el signo del result. Reescrito preservando exactamente las
// asignaciones del decomp (`360 - a2 + a1` y `360 - a1 + a2`) en cada rama.
float __cdecl FUN_0043e370(float a1, float a2, char a3)
{
  if ( a1 < 0.0f ) a1 += 360.0f;
  if ( a2 < 0.0f ) a2 += 360.0f;

  float result = a2 - a1;
  if ( a2 > a1 )
  {
    if ( a1 + 180.0f <= a2 ) result = 360.0f - a2 + a1;
  }
  else if ( a1 - 180.0f > a2 )
  {
    result = 360.0f - a1 + a2;
  }

  if ( a3 == 1 && result < 0.0f ) return -result;
  return result;
}



// FUN_0043ea20 @ 0x0043EA20 — Entity_MovePath(entity, flag)
// Advances entity along its Catmull-Rom waypoint path.
// path_wp_x/y arrays at entity+0x357/0x366 (grid coords); path_substep 0-3 per segment.
// Returns 1 when entity arrives at final waypoint; 0 otherwise.
// Anti-tamper HashTable blocks (DAT_055c9bc8/bd0/bd4) skipped per project policy.
static unsigned int MovePath_IDA_0043EA20(char *ent, char turn)
{
    // 0043EA20 without its hash-table obfuscation blocks.  Offsets are decimal
    // in the original: current=852, subframe=853, count=854, X=855, Y=870.
    byte current = *(byte *)(ent + 852);
    const byte count = *(byte *)(ent + 854);
    if (current >= count) return 0;

    if (!*(byte *)(ent + 853)) {
        int next = current + 1;
        if (next > count - 1) next = count - 1;
        *(int *)(ent + 904) = *(byte *)(ent + 855 + next);
        *(int *)(ent + 908) = *(byte *)(ent + 870 + next);
    }

    float x[4], y[4];
    for (int i = 0; i < 4; ++i) {
        int point = (int)current - 1 + i;
        if (point < 0) point = 0;
        if (point > count - 1) point = count - 1;
        x[i] = ((float)*(byte *)(ent + 855 + point) + 0.5f) * 100.0f;
        y[i] = ((float)*(byte *)(ent + 870 + point) + 0.5f) * 100.0f;
    }

    float targetX = x[1], targetY = y[1];
    switch (*(byte *)(ent + 853)) {
    case 0: targetX=x[2]*.2265625f-x[3]*.0234375f+x[1]*.8671875f-x[0]*.0703125f;
            targetY=y[2]*.2265625f-y[3]*.0234375f+y[1]*.8671875f-y[0]*.0703125f; break;
    case 1: targetX=x[2]*.5625f-x[3]*.0625f+x[1]*.5625f-x[0]*.0625f;
            targetY=y[2]*.5625f-y[3]*.0625f+y[1]*.5625f-y[0]*.0625f; break;
    case 2: targetX=x[2]*.8671875f-x[3]*.0703125f+x[1]*.2265625f-x[0]*.0234375f;
            targetY=y[2]*.8671875f-y[3]*.0703125f+y[1]*.2265625f-y[0]*.0234375f; break;
    case 3: targetX=x[2]; targetY=y[2]; break;
    }

    const float dx = *(float *)(ent + 16) - targetX;
    const float dy = *(float *)(ent + 20) - targetY;
    if (sqrtf(dx*dx + dy*dy) > 20.0f || ++*(byte *)(ent + 853) <= 3) {
        if (turn) {
            const float angle = FUN_0043e050(*(float *)(ent + 16), *(float *)(ent + 20), targetX, targetY);
            const float delta = FUN_0043e370(*(float *)(ent + 36), angle, 1);
            *(float *)(ent + 36) = (delta >= 45.0f) ? angle : FUN_0043e1b0(*(float *)(ent + 36), angle, delta * 0.5f);
        }
        return 0;
    }

    ++*(byte *)(ent + 852);
    *(byte *)(ent + 853) = 0;
    current = *(byte *)(ent + 852);
    if (current < count - 1) return 0;

    *(byte *)(ent + 852) = count - 1;
    *(int *)(ent + 904) = *(byte *)(ent + 855 + count - 1);
    *(int *)(ent + 908) = *(byte *)(ent + 870 + count - 1);
    *(float *)(ent + 16) = targetX;
    *(float *)(ent + 20) = targetY;
    return 1;
}

unsigned int __cdecl FUN_0043ea20(void *entity, char flag)
{
    char *ent = (char *)entity;
    return MovePath_IDA_0043EA20(ent, flag);

#if 0

    // 2026-04-30 BUG-FIX: cuando no hay path (wp_count <= cur_wp), DEBE
    // retornar 1 ("arrived/idle"), NO 0.  El caller hace:
    //     if (moveOk == 0) FUN_00454ba0(ent);   // mueve al hero
    //     else             FUN_004430c0(ent);   // detiene
    // Si retornábamos 0 cuando no hay path → caller llamaba al mover →
    // hero caminaba en facing direction sin importar si había path.
    // Por eso "se movía solo" sin click.
    if (*(byte *)(ent + 0x356) <= *(byte *)(ent + 0x354))
        return 1;

    byte cur_wp   = *(byte *)(ent + 0x354);
    byte wp_count = *(byte *)(ent + 0x356);
    byte substep  = *(byte *)(ent + 0x355);

    // path_substep == 0: initialize cached waypoint grid coords
    if (substep == 0) {
        int next_wp = (int)cur_wp + 1;
        int max_wp  = (int)wp_count - 1;
        if (max_wp < next_wp) next_wp = max_wp;
        *(int *)(ent + 0x388) = (int)*(byte *)(ent + 0x357 + next_wp); // cached_wp_x
        *(int *)(ent + 0x38c) = (int)*(byte *)(ent + 0x366 + next_wp); // cached_wp_y
    }

    // 2026-04-30 SIMPLIFIED: en lugar de Catmull-Rom (que con coeficientes
    // _DAT_00552880..88c daba targets atrás del hero / dirección opuesta y
    // hacía giros en círculo), ir directamente al siguiente waypoint con
    // lerp lineal.  Pierde "smoothness" del spline pero hace que el walker
    // efectivamente avance.
    int next_wp = (int)cur_wp + 1;
    if (next_wp >= (int)wp_count) {
        return 1;   // arrived
    }
    float target_x = ((float)*(byte *)(ent + 0x357 + next_wp) + _DAT_00552504) * _DAT_005524f0;
    float target_y = ((float)*(byte *)(ent + 0x366 + next_wp) + _DAT_00552504) * _DAT_005524f0;

    // Distance check
    float dx   = *(float *)(ent + 0x10) - target_x;
    float dy   = *(float *)(ent + 0x14) - target_y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Threshold: ~25 units tight enough to prevent overshoot, generous
    // enough to consume waypoints quickly. Original used _DAT_005524fc.
    float threshold = _DAT_005524fc;
    if (threshold < 25.0f) threshold = 25.0f;

    if (dist <= threshold) {
        // Arrived at this waypoint — snap to it and advance
        *(float *)(ent + 0x10) = target_x;
        *(float *)(ent + 0x14) = target_y;
        cur_wp++;
        *(byte *)(ent + 0x354) = cur_wp;
        *(byte *)(ent + 0x355) = 0;
        if ((int)(wp_count - 1) <= (int)cur_wp) {
            return 1;   // arrived at final
        }
        // Update target for facing calc this frame (so hero turns immediately)
        next_wp = (int)cur_wp + 1;
        if (next_wp < (int)wp_count) {
            target_x = ((float)*(byte *)(ent + 0x357 + next_wp) + _DAT_00552504) * _DAT_005524f0;
            target_y = ((float)*(byte *)(ent + 0x366 + next_wp) + _DAT_00552504) * _DAT_005524f0;
        }
    }

    // 2026-04-30 BUG-FIX (v3): actualizar facing.  Convención REAL de MU
    // (verificada con la rotación de matriz en FUN_00454ba0):
    //   0°   = NORTH (-Y)
    //   90°  = EAST  (+X)
    //   180° = SOUTH (+Y)
    //   270° = WEST  (-X)
    //
    // FUN_00454ba0 hace: vel_local=(0,-speed,0), rotated by facing (Z-axis).
    // Result: out_x = +speed*sin(θ), out_y = -speed*cos(θ).
    // → θ=0 ⇒ out=(0,-speed) = north ✓
    // → θ=90 ⇒ out=(+speed,0)  = east  ✓
    //
    // Para que `atan2(...)` devuelva 0° cuando dy<0 (target al norte),
    // necesitamos `atan2(dx, -dy)`.
    //
    // (Versión v2 usaba atan2(dx, dy) que daba ángulo OPUESTO — click
    //  arriba mandaba al hero hacia abajo y caminaba infinito al sur.)
    if (flag != '\0') {
        float ent_x = *(float*)(ent + 0x10);
        float ent_y = *(float*)(ent + 0x14);
        float ddx = target_x - ent_x;
        float ddy = target_y - ent_y;
        if (ddx * ddx + ddy * ddy > 0.01f) {
            float facingDeg = (float)(atan2((double)ddx, -(double)ddy) * 57.29577951);
            // Normalize to [0..360)
            if (facingDeg < 0.0f) facingDeg += 360.0f;
            *(float*)(ent + 0x24) = facingDeg;
        }
    }
    return 0;
}

#endif // obsolete non-IDA linear MovePath reconstruction
}

// FUN_004830b0 @ 0x004830B0 — PathRange_Check(sx,sy,tx,ty)
// Bresenham line from (sx,sy) to (tx,ty) testing terrain attr DAT_0838bc70.
// Returns 1 if path is clear, 0 if blocked (attr>3 and not walkable).
char __cdecl FUN_004830b0(int sx, int sy, int tx, int ty) {
    int tile = FUN_004f6c40((unsigned int)sx, (unsigned int)sy);
    int err  = 0;
    unsigned int dx = (unsigned int)(tx - sx);
    unsigned int dy = (unsigned int)(ty - sy);
    int step_major, step_minor;
    if ((int)dx < 0) { dx = (unsigned int)(-(int)dx); step_major = -1; }
    else { step_major = 1; }
    if ((int)dy < 0) { dy = (unsigned int)(-(int)dy); step_minor = -0x100; }
    else { step_minor = 0x100; }
    int inc_a = step_major, inc_b = step_minor;
    unsigned int major = dx, minor_steps = dy;
    if ((int)dy < (int)dx) {
        // swap so major >= minor
        inc_a    = step_minor;
        major    = dy;
        inc_b    = step_major;
        minor_steps = dx;
    }
    unsigned int steps = 0;
    do {
        byte attr = ((byte*)&DAT_0838bc70)[tile];
        if (attr > 3 && (attr & 0x20) != 0x20) return 0; // blocked
        err += minor_steps;
        if ((int)major / 2 < err) { tile += inc_a; err -= major; }
        tile += inc_b;
        steps++;
    } while ((int)steps <= (int)major);
    return 1;
}

// Net / packet — all use HashTable obfuscation; stubs preserve observable side effects.
