// stubs_externs.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 5298-8814 (3517 lines).
//
// Contiene 3 secciones unidas por razones de cohesión:
//   "Missing function stubs (all LNK2019 unresolved externals)" — sección
//     histórica que agrupa funciones que LNK2019 reportaba como unresolved
//     en pases iniciales: Movement/pathfinding (FUN_0043e050 CreateAngle
//     etc), entity helpers, etc.
//   "Particle / animation / bone math" — bone-matrix transforms, SetAttackSpeed,
//     particle helpers.
//   "Weapon/Entity color helpers" — FUN_00503cf0 (Weapon_SetColor) and
//     related hue helpers.
//
// Conservadas en un archivo único hasta que cada función se mueva a su
// módulo definitivo (Movement.cpp, Render/, Item/, etc.).

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
// FUN_00423040 @ 0x00423040 — HashTable_Insert_Obfuscated (__thiscall this, param_1)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl FUN_00423040(void *ctx, void *chardata) {
    // STUB: HashTable insert with obfuscation — cannot implement safely (unaff_retaddr)
    (void)ctx; (void)chardata;
}
// FUN_00422df0 @ 0x00422DF0 — HashTable_Insert_Ptr (__thiscall this, param_1)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl FUN_00422df0(void *ctx, void *counter) {
    // STUB: HashTable insert (ptr) with obfuscation — cannot implement safely
    (void)ctx; (void)counter;
}
// FUN_0040e330 @ 0x0040E330 — NO es "Timer_Advance": es el ciclador del TAMAÑO
// del historial del ChatListBox (tecla F4 y botón 2 del popup del chat).
// Cicla this[35] (visible row count, +0x8C): 3 → 6 → 30 → 6 …, alternando
// g_bUseChatListBox (DAT_005590ac), y después re-scrollea.
//
// FIX 2026-07-20 — CRASH 0xC0000005 con param0=8 (violación de EJECUCIÓN):
// las 4 ramas hacían `(**(void(__cdecl**)(int))(*(int*)param_1 + 0x30))(0)`.
// `*param_1 + 0x30` es vtable+48 = entrada 12 (sub_40CC50 / scrollByN), que es
// __thiscall.  Al invocarla como __cdecl con un solo argumento, el `this` no
// viajaba en ECX: la callee tomaba como `this` la basura que hubiera quedado en
// ECX, deferenciaba su "vtable" y saltaba a una dirección arbitraria.
// El disasm (0x40E35D, 0x40E375, 0x40E39C, 0x40E3BE) muestra las 4 ramas como
// `mov eax,[ecx] / push 0 / call [eax+30h]` con ECX intacto = __thiscall(this, 0).
// Hex-Rays tipó UNA de las ramas como __stdcall sin this (perdió el tracking de
// ECX al hoistear `v2 = *this`); las otras tres sí salen como __thiscall.
static void ChatLB_ScrollBy0(int* self)
{
    // vtable+48 = entrada 12 = scrollByN(this, n).  __fastcall en nuestro build.
    typedef int (__fastcall *FnScrollByN)(int* /*ecx=this*/, int /*edx*/, int /*n*/);
    void** vt = *(void***)self;
    ((FnScrollByN)vt[12])(self, 0, 0);
}

void __cdecl FUN_0040e330(unsigned long val) {
    int *param_1 = (int*)(uintptr_t)val;
    if (!param_1 || !*(int*)param_1) return;   // objeto sin construir / vtable nula
    switch (param_1[0x23]) {
    case 3:
        param_1[0x23] = 6;
        ChatLB_ScrollBy0(param_1);
        return;
    default:
        if (param_1[0x23] >= 0x1f) {
            DAT_005590ac = 1;
            param_1[0x23] = 6;
        }
        ChatLB_ScrollBy0(param_1);
        return;
    case 6: case 9: case 0xc: case 0xf: case 0x12: case 0x15: case 0x18: case 0x1b:
        if (DAT_005590ac == 1) {
            param_1[0x23] = 0x1e;
        } else {
            DAT_005590ac = 1;
            param_1[0x23] = 3;
        }
        ChatLB_ScrollBy0(param_1);
        return;
    case 0x1e:
        param_1[0x23] = 6;
        DAT_005590ac = 0;
        ChatLB_ScrollBy0(param_1);
        return;
    }
}

// Scene / map helpers
// FUN_004c4650 and FUN_004c8d70 — implemented in src/UI/RenderItemInfo.cpp

// FUN_004c9730 @ 0x004C9730 — UI_CommandPanel_BuildEntry(chardata, slot)
// Real logic: calls FUN_0047e4f0 (GetMagicSkillDamage) and GetSkillInformation for the
// skill in CharacterAttribute->Skill[param_2+4], then sprintf's skill name, damage, mana cost,
// distance, and class-specific descriptions into the Items[999] text buffer (stride 100 bytes).
// Class 0 (Dark Wizard): skill 0x10 gets 3 extra stat lines (MaxMana, Energy, Dexterity).
// Class 2 (Fairy Elf): skills 0x1A/0x1B/0x1C get special description lines.
// Class 1 (Dark Knight): skill 0x2F gets extra combo line.
// Finally calls FUN_004c2420 (CharMenu_RenderTextList) with unaff_retaddr as Y position.
// STUB: unaff_retaddr carries screen Y position from caller — cannot resolve without
// call-site disassembly. Also uses CharacterAttribute (undeclared typed struct).
// FUN_004c9730 @ 0x004C9730 — Skill_RenderTooltip(float a1, int a2, int hoveredSkillIdx)
// Ported from IDA `sub_4C9730` decompile (1844 bytes).
//
// Builds a tooltip text-list for the hovered skill (TextList[0..n]) describing
// name / damage / distance / mana / skillMana, then calls FUN_004c2420
// (CharMenu_RenderTextList) to draw it at (a1, a2).
//
// Skipped (per anti-tamper policy):
//   - Hash-table ref-count dance on CharacterMachine (start + end)
//   - XOR encryption pass over the CharacterMachine buffer (end of function)
//
// Ported game logic:
//   1. Read SkillTable[a3].Skill = *(BYTE*)(CharacterAttribute + a3 + 87)
//   2. CHARACTER_MACHINE::GetMagicSkillDamage → piMinDamage / piMaxDamage
//   3. GetSkillInformation → szName, piMana, piDistance, piSkillMana
//   4. Class-specific damage formulas (DW skill 16 / Elf 0x1A/0x1B/0x1C / DK 47)
//   5. Distance / Mana / SkillMana lines
//   6. FUN_004c2420 with computed Y / count
extern "C++" {
extern char    GlobalText[GLOBALTEXT_ROWS][300];   // ver globals.h
extern char    lpString_07e90798[];
extern int     DAT_07e91708[30];
extern int     DAT_07ea7b10[30];
extern HDC     m_hFontDC;
extern float   _DAT_055c9b74;
}

static void SkillTooltip_RenderLines(int sx, int sy, char lines[][100], int count)
{
    if (count <= 0 || !m_hFontDC) return;

    TEXTMETRICA tm = {};
    GetTextMetricsA(m_hFontDC, &tm);
    const int lineH = tm.tmHeight + tm.tmExternalLeading;
    const int padX = 4;
    const int padY = 3;

    int maxWidth = 0;
    for (int i = 0; i < count; ++i) {
        SIZE sz = {};
        GetTextExtentPointA(m_hFontDC, lines[i], (int)strlen(lines[i]), &sz);
        if (sz.cx > maxWidth) maxWidth = sz.cx;
    }

    int boxW = maxWidth + padX * 2;
    int boxH = count * lineH + padY * 2;
    int drawX = sx - boxW / 2;
    int drawY = sy - boxH - 8;
    if (drawY < 0) drawY = sy + 8;
    if (drawX < 0) drawX = 0;
    if (drawX + boxW > (int)WindowWidth) drawX = (int)WindowWidth - boxW;
    if (drawX < 0) drawX = 0;

    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    FUN_005124c0((float)(drawX - 1), (float)(drawY - 1), (float)(boxW + 2), 1.0f);
    FUN_005124c0((float)(drawX - 1), (float)(drawY + boxH), (float)(boxW + 2), 1.0f);
    FUN_005124c0((float)(drawX - 1), (float)(drawY - 1), 1.0f, (float)(boxH + 2));
    FUN_005124c0((float)(drawX + boxW), (float)(drawY - 1), 1.0f, (float)(boxH + 2));
    glColor4f(0.0f, 0.0f, 0.0f, 0.82f);
    FUN_005124c0((float)drawX, (float)drawY, (float)boxW, (float)boxH);

    int lineY = drawY + padY;
    for (int i = 0; i < count; ++i) {
        DAT_00559c78 = (i == 0) ? 0xFFFFFF00 : 0xFFFFFFFF;
        m_dwBackColor = 0;
        FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, drawX + padX, lineY, lines[i], 0);
        lineY += lineH;
    }
}

static void FUN_004c9730_old(float a1, int a2, int a3)
{
    // 2026-05-05: SIMPLIFIED safe version. La versión completa hacía 10+
    // sprintf_s con GlobalText[N] format strings. Si cualquier slot de
    // GlobalText estaba sin cargar/corrupted (e.g. tenía "%s" donde el código
    // pasa un int), sprintf interpretaba el int como char* → AV crash on
    // hover. Esta versión solo muestra el nombre del skill (sin damage,
    // mana, distance lines) hasta que GlobalText loader esté validado.
    //
    // Bounds check: a3 (slot index hovered) debe ser 0..63 para evitar OOB
    // read en CharacterAttribute[87 + a3].
    if (a3 < 0 || a3 >= 60 || !CharacterAttribute) return;

    unsigned char skillType = *(unsigned char*)((char*)CharacterAttribute + a3 + 87);
    if (skillType == 0 || skillType >= 64) return;  // empty slot or OOB type

    char szName[256];
    szName[0] = 0;
    int piMana = 0, piDistance = 0, piSkillMana = 0;
    GetSkillInformation((int)skillType, 1, szName, &piMana, &piDistance, &piSkillMana);
    if (szName[0] == 0) return;  // no name → don't render

    char lines[5][100] = {};
    int count = 0;
    sprintf_s(lines[count++], 100, "%s", szName);
    if (piDistance > 0) sprintf_s(lines[count++], 100, "Distance: %d", piDistance);
    sprintf_s(lines[count++], 100, "Mana: %d", piMana);
    if (piSkillMana > 0) sprintf_s(lines[count++], 100, "Skill Mana: %d", piSkillMana);
    SkillTooltip_RenderLines((int)a1, a2, lines, count);
    return;

    // Original full impl preserved below for reference but disabled.
#if 0
    int  piMinDamage = 0, piMaxDamage = 0;
    int  piMana = 0, piDistance = 0, piSkillMana = 0;
    (void)piMinDamage; (void)piMaxDamage; (void)piMana; (void)piDistance; (void)piSkillMana;

    // Get min/max damage range — FUN_0047e4f0 (GetMagicSkillDamage, ~700 bytes, IDA-only
    // and gated behind IDA_PORT_0047E4F0). Without it we leave piMin/piMaxDamage at 0;
    // the damage line will show "0~0" until the helper is unconditionally ported.

    // Skill name + mana / distance / mana costs
    GetSkillInformation(skillType, 1, szName, &piMana, &piDistance, &piSkillMana);

    // ── Build TextList lines ────────────────────────────────────────────────
    char* TextList0 = lpString_07e90798;
    auto TextListN = [&](int i) -> char* { return lpString_07e90798 + i * 100; };
    int* TextListColor = DAT_07e91708;
    int* TextBold = DAT_07ea7b10;

    sprintf_s(TextList0, 100, "\n");
    sprintf_s(TextListN(1), 100, "%s", szName);
    TextListColor[1] = 1;
    TextBold[1] = 1;
    sprintf_s(TextListN(2), 100, "\n");

    int v10 = 3;
    int heroClassFlag = *(unsigned char*)(Hero + 444) & 7;

    // Class 0 (Dark Wizard / magic class): damage line
    if (heroClassFlag == 0) {
        if (skillType == 16) {
            // Twister-style: derived from Strength (CA+34), Energy (CA+26), Agility (CA+22)
            int agi = *(unsigned short*)((char*)CharacterAttribute + 22);
            int ene = *(unsigned short*)((char*)CharacterAttribute + 26);
            int str = *(unsigned short*)((char*)CharacterAttribute + 34);
            int dmg = (int)((double)agi * 0.02 + (double)ene * 0.0049999999 + 10.0);
            int strBoost = (int)((double)str * 0.02);
            int dur = (int)((double)ene * 0.025 + 60.0);
            sprintf_s(TextListN(3), 100, GlobalText[578], (unsigned int)dmg);
            TextListColor[3] = 0; TextBold[3] = 0;
            sprintf_s(TextListN(4), 100, GlobalText[880], strBoost);
            TextListColor[4] = 0; TextBold[4] = 0;
            sprintf_s(TextListN(5), 100, GlobalText[881], (unsigned int)dur);
            TextListColor[5] = 0; TextBold[5] = 0;
            v10 = 6;
        } else {
            sprintf_s(TextListN(3), 100, GlobalText[170], piMinDamage, piMaxDamage);
            TextListColor[3] = 0; TextBold[3] = 0;
            v10 = 4;
        }
    }

    // Class 2 (Elf): buff strength preview
    if (heroClassFlag == 2) {
        int ene = *(unsigned short*)((char*)CharacterAttribute + 26);
        bool emitted = true;
        switch (skillType) {
        case 0x1A:
            sprintf_s(TextListN(v10), 100, GlobalText[171], ene / 5 + 5);
            break;
        case 0x1B:
            sprintf_s(TextListN(v10), 100, GlobalText[172], (ene >> 3) + 2);
            break;
        case 0x1C:
            sprintf_s(TextListN(v10), 100, GlobalText[173], ene / 7 + 3);
            break;
        default:
            emitted = false;
            break;
        }
        if (emitted) {
            TextListColor[v10] = 0;
            TextBold[v10] = 0;
            v10++;
        }
    }

    // Distance line
    if (piDistance) {
        sprintf_s(TextListN(v10), 100, GlobalText[174], piDistance);
        TextListColor[v10] = 0;
        TextBold[v10] = 0;
        v10++;
    }

    // Mana line
    sprintf_s(TextListN(v10), 100, GlobalText[175], piMana);
    TextListColor[v10] = 0;
    TextBold[v10] = 0;
    int v15 = v10 + 1;

    // SkillMana (if > 0)
    if (piSkillMana > 0) {
        sprintf_s(TextListN(v15), 100, GlobalText[360], piSkillMana);
        TextListColor[v15] = 0;
        TextBold[v15] = 0;
        v15++;
    }

    // Class 1 (DK) + skill 47 → "cannot use" message
    if (heroClassFlag == 1 && skillType == 47) {
        sprintf_s(TextListN(v15), 100, "%s", GlobalText[96]);
        TextListColor[v15] = 5;
        TextBold[v15] = 0;
        v15++;
    }

    sprintf_s(TextListN(v15), 100, "\n");
    int v16 = v15 + 1;

    // Anti-tamper XOR encryption pass on CharacterMachine — skipped (anti-tamper).

    // Compute Y position from text height
    SIZE sz; sz.cx = 0; sz.cy = 0;
    GetTextExtentPointA(m_hFontDC, TextList0, 1, &sz);
    int v31 = 3 * sz.cy / 2 + sz.cy * (v16 - 3);
    float yPos = (float)v31 / _DAT_055c9b74;

    // Render. Our FUN_004c2420 has 6-int signature (mode, startIdx, count, x, layout, border).
    // Best-effort mapping of IDA's 7-arg float-mixed call:
    //   mode=2 (boxed), startIdx=0, count=v16, x=a2-yPos, layout=0, border=1
    FUN_004c2420(2, 0, v16, a2 - (int)yPos, 0, 1);
    (void)a1;  // a1 (float Y) not used by our simplified render path
#endif
}

void __cdecl FUN_004c9730(float a1, int a2, int a3)
{
    int skillTipX = *(int*)&a1;
    if (a3 < 0 || a3 >= 60 || !CharacterAttribute || !Hero) return;

    unsigned char skillType = *(unsigned char*)((char*)CharacterAttribute + a3 + 87);
    if (skillType == 0 || skillType >= 64) return;

    unsigned char* skillBaseTbl = DAT_07d29d20 ? (unsigned char*)(uintptr_t)DAT_07d29d20 : nullptr;
    unsigned char* skillStatTbl = DAT_07cf1ff8 ? (unsigned char*)(uintptr_t)DAT_07cf1ff8 : skillBaseTbl;
    if (!skillBaseTbl && !skillStatTbl) return;

    char szName[256] = {};
    int piMinDamage = 0, piMaxDamage = 0;
    int piMana = 0, piDistance = 0, piSkillMana = 0;

    unsigned char* skillBaseRec = skillBaseTbl ? skillBaseTbl + (int)skillType * 0x28 : nullptr;
    unsigned char* skillStatRec = skillStatTbl ? skillStatTbl + (int)skillType * 0x28 : skillBaseRec;
    const char* skillName = "";
    if (skillBaseRec && ((const char*)skillBaseRec)[0] != '\0')
        skillName = (const char*)skillBaseRec;
    else if (skillStatRec && ((const char*)skillStatRec)[0] != '\0')
        skillName = (const char*)skillStatRec;
    if (skillName && skillName[0]) {
        strncpy_s(szName, sizeof(szName), skillName, 31);
        szName[31] = '\0';
    }
    GetSkillInformation((int)skillType, 1, szName[0] ? NULL : szName, &piMana, &piDistance, &piSkillMana);

    if (skillStatRec) {
        int rangeVal = (int)skillStatRec[0x27];
        if (rangeVal > 0 && rangeVal < 50) {
            piDistance = rangeVal;
        } else if (piDistance < 0 || piDistance > 50) {
            piDistance = 0;
        }
    }

    if (piMana < 0 || piMana > 5000) piMana = 0;
    if (piSkillMana < 0 || piSkillMana > 5000) piSkillMana = 0;

    if (CharacterMachine && skillStatRec) {
        __try {
            int baseDamage = (int)*(WORD*)(skillStatRec + 0x24);
            if (baseDamage > 0 && baseDamage < 5000) {
                piMinDamage = baseDamage + *(unsigned short*)((char*)CharacterMachine + 70);
                piMaxDamage = baseDamage + (baseDamage >> 1) + *(unsigned short*)((char*)CharacterMachine + 72);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            piMinDamage = 0;
            piMaxDamage = 0;
        }
    }

    // ── Build TextList lines ────────────────────────────────────────────────
    // Estructura del binario (0x004C97xx, y el volcado de IDA que quedo en
    // FUN_004c9730_old bajo `#if 0`):
    //   slot 0 = "\n"                       (separador de media altura)
    //   slot 1 = nombre de la skill         color 1 (azul claro), NEGRITA
    //   slot 2 = "\n"
    //   slot 3+ = dano / rango / mana / AG  color 0 (blanco)
    //   opcional "no puede usarla"          color 5 (blanco con franja)
    //   ultimo  = "\n"
    //
    // 2026-08-18: antes esto pintaba su PROPIA caja (cuarta reimplementacion
    // inventada del tooltip, con colores ARGB y textos en ingles hardcodeados).
    // Ahora usa lpString_07e90798 + FUN_004c2420, que es lo que hace el binario
    // — misma rutina que el tooltip de item y el menu de personaje.
    auto  TextListN     = [](int i) -> char* { return lpString_07e90798 + i * 100; };
    auto  GlobalTextOr  = [](int idx, const char* fallback) -> const char* {
        if (idx >= 0 && idx < GLOBALTEXT_ROWS && GlobalText[idx][0])
            return GlobalText[idx];
        return fallback;
    };

    int idx = 0;
    for (int i = 0; i < 30; ++i) {
        lpString_07e90798[i * 100] = 0;
    }

    crt_sprintf(TextListN(idx), "\n");
    DAT_07e91708[idx] = 0;
    DAT_07ea7b10[idx] = 0;
    idx++;

    _snprintf_s(TextListN(idx), 100, _TRUNCATE, "%s", szName);
    DAT_07e91708[idx] = 1;
    DAT_07ea7b10[idx] = 1;
    idx++;

    crt_sprintf(TextListN(idx), "\n");
    DAT_07e91708[idx] = 0;
    DAT_07ea7b10[idx] = 0;
    idx++;

    if (piMinDamage > 0 || piMaxDamage > 0) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(170, "Wizardry Dmg:%d~%d"), piMinDamage, piMaxDamage);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }
    if (piDistance > 0) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(174, "Range: %d"), piDistance);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }
    {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(175, "Mana: %d"), piMana);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }
    if (piSkillMana > 0) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(360, "AG: %d"), piSkillMana);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }

    // DK (clase 1) con la skill 47: linea de "no puede usarla", color 5 — el
    // unico color con franja de fondo (m_dwBackColor = 0xff0000a0).
    if (Hero && ((*(BYTE*)((char*)Hero + 0x1BC) & 7) == 1) && skillType == 47) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE, "%s", GlobalTextOr(96, ""));
        DAT_07e91708[idx] = 5;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }

    crt_sprintf(TextListN(idx), "\n");
    DAT_07e91708[idx] = 0;
    DAT_07ea7b10[idx] = 0;
    idx++;

    // Posicion Y — port literal de 0x004C9DF7..0x004C9E36:
    //   v31 = (count - 3) * cy + (3 * cy) / 2
    //   y   = a2 - (int)(v31 / g_fScreenRate_y)
    //   DrawItemInfoBox(x, y, count, 0, 2, 1)
    {
        SIZE sz;
        sz.cx = 0;
        sz.cy = 0;
        GetTextExtentPointA(m_hFontDC, lpString_07e90798, 1, &sz);
        int v31 = (idx - 3) * sz.cy + (3 * sz.cy) / 2;
        int yBox = a2 - (int)((float)v31 / _DAT_055c9b74);
        FUN_004c2420(skillTipX, yBox, idx, 0, 2, 1);
    }
}

extern "C" int Text_MeasureOrthoWidth(const char* text);   // definido más abajo

// FUN_0047f7a0 @ 0x0047F7A0 — Text_Draw(x, y, text, maxw, iSort, extra)
// Draws text via Font vtable dispatch (FUN_0040f610) if non-empty or maxw!=0.
// Return type is void per functions.h declaration.
//
// 2026-05-04: BUG-FIX — el flag `param_5` (iSort) determinaba alineación:
//   1 = left-align (default)
//   2 = center within [x, x+maxw]
// Antes era `(void)param_5` → todo render quedaba left-aligned. El menú C
// (RenderText con centered=1 sobre maxw=70-150) renderizaba "mago", "[Soul
// Master]", "Fuerza:1000" etc. pegados a la izquierda en lugar de centrados.
// IDA original delega al `CUIRenderText::RenderText` que maneja iSort
// internamente; lo replicamos inline acá.
void __cdecl FUN_0047f7a0(int param_1, int param_2, char *param_3, int param_4, int param_5, int param_6) {
    (void)param_6;
    if (param_3 == nullptr) return;
    // 2026-05-08: bug-fix — múltiples crashes en ucrtbase.dll's strlen/lstrlenA
    // venían de pasar punteros pequeños (< 0x100000) tipo 0x2A00 (= type*64
    // con DAT_07d78068=0). Validar el puntero antes de cualquier strlen.
    // Range: heap user-space [0x100000..0x80000000). rdata strings in our
    // exe live in [0x004XXXXX..0x00500000) — also valid.
    {
        uintptr_t p = (uintptr_t)param_3;
        if (p < 0x100000 || p >= 0x80000000) return;
    }
    __try {
        if (strlen(param_3) == 0 && param_4 == 0) return;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return;   // strlen crashed — bogus param_3
    }

    int x = param_1;

    // Aplicar centrado dentro del box [param_1, param_1+param_4].
    // param_1 y param_4 están en unidades del ortho; el extent de GDI viene en
    // píxeles de framebuffer.  Text_MeasureOrthoWidth hace la conversión (es el
    // equivalente correcto del `sz.cx / g_fScreenRate_x` de IDA para nuestro
    // pipeline).  Sin ella el texto quedaba descentrado hacia la izquierda.
    if (param_5 >= 2 && param_4 > 0 && DAT_055c9fec) {
        int textW = Text_MeasureOrthoWidth(param_3);
        if (textW > 0 && textW < param_4) {
            x = param_1 + (param_4 - textW) / 2;
        }
    }

    FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, x, param_2,
                 (const char*)param_3, (DWORD)param_4);
}

// FUN_004977f0 @ 0x004977F0 — String_FindSubstr(str, pattern, from_start)
// DBCS-aware strstr. param_3!=0 forces search from position 0 only.
// Returns 1 if found, 0 otherwise.
unsigned int __cdecl FUN_004977f0(char *param_1, void *param_2, char param_3) {
    char *pat = (char*)param_2;
    int iVar5 = (int)strlen(pat);
    // BUG-FIX 2026-07-17: patrón vacío = no-match. El IDA devuelve 1 para patrón
    // vacío, pero eso solo es "correcto" porque en el original los strings de filtro
    // están cargados (no vacíos). Varios de esos globals llegan vacíos en runtime en
    // nuestro build (ej. GlobalText[457/458] si el Text.bmd no tiene esas filas) →
    // FindText(nombre,"")=1 rechazaba TODO nombre en el create-char. Un patrón vacío
    // no debe matchear nada.
    if (iVar5 < 1) return 0;
    int iVar6 = (int)strlen(param_1) - iVar5;
    if (param_3 != '\0') iVar6 = 0;
    if (iVar6 < 0) return 0;
    for (int iVar8 = 0; iVar8 <= iVar6; ) {
        int iVar2 = 0;
        if (iVar5 < 1) return 1;
        while ((param_1 + iVar8)[iVar2] == pat[iVar2]) {
            if (++iVar2 >= iVar5) return 1;
        }
        char c = FUN_00541eab((byte*)(param_1 + iVar8));
        iVar8 += (unsigned int)(unsigned char)c;
    }
    return 0;
}

// FUN_0047fe30 @ 0x0047FE30 — Text_SplitAtMiddle(src, dstB_ptr, dstA, len)
// Splits src at middle: first half → dstA (param_3), second half → *(char*)param_2.
// Split point: first space near len/2, or forced at len/2+2.
void __cdecl FUN_0047fe30(void *param_1_v, int param_2, void *param_3_v, int param_4) {
    char *param_1 = (char*)param_1_v;
    char *param_3 = (char*)param_3_v;
    unsigned int uVar4 = 0, uVar3 = 0;
    if (param_4 > 0) {
        do {
            uVar3 = uVar4;
            if ((param_4/2 - 2 <= (int)uVar4 && param_1[uVar4] == ' ') ||
                (param_4/2 + 2 <= (int)uVar4)) break;
            char c = FUN_00541eab((byte*)(param_1 + uVar4));
            uVar4 += (unsigned int)(unsigned char)c;
            uVar3 = 0;
        } while ((int)uVar4 < param_4);
    }
    if ((int)uVar3 > 0) memcpy(param_3, param_1, uVar3);
    param_3[uVar3] = '\0';
    for (unsigned int i = uVar3; (int)i < param_4; i++)
        ((char*)param_2)[i - uVar3] = param_1[i];
    ((char*)param_2)[param_4 - uVar3] = '\0';
}

// FUN_0040f610 @ 0x0040F610 — CUIRenderText::RenderText (vtable dispatcher)
// Original IDA: `(*(vtable[0]+4))(this, x, y, text, ...)` — thiscall through
// CUIRenderText->pSubclass->vtable[1]. El subclass se instancia en OpenFont
// (0x0050f690) vía sub_40F570 y puede ser tipo-0 (simple, TGA font) o tipo-1
// (compleja, bitmap font procesada). El subclass tipo-0 usa `Bitmaps[0/1]`
// (Interface/FontInput.tga) para renderizar cada glyph como un quad.
//
// Port: implementación autónoma usando wglUseFontBitmapsA sobre la HFONT
// GDI ya seleccionada en m_hFontDC. Genera 256 display lists a partir de la
// fuente GDI y emite el texto con glCallLists. Suficiente para UI (login,
// char-select, chat) — el look no matchea la fuente TGA original píxel a
// píxel pero los textos aparecen en su posición con el color y layout
// correctos, que era lo que faltaba.
//
// Coordenadas: callers pasan Y con origen TOP-LEFT (convención game). La GL
// ortho es `gluOrtho2D(0, vw, 0, vh)` Y-bottom (ver FUN_005123c0 en
// Render/GL_2D.cpp). FUN_005125a0 Y-flipa via `vh - param_3`. Acá hacemos
// lo mismo para raster pos.
//
// Color: DAT_00559c78 es el COLORREF GDI (0x00BBGGRR). Se convierte a glColor.
// El Alpha del byte alto (cuando está seteado, ej 0xffd2e6ff) se respeta.
// ── Escala "píxeles de framebuffer" → "unidades del ortho 2D" ───────────────
// 2026-07-20.  El punto que hacía fallar todos los recuadros de texto:
//
//   · La GEOMETRÍA 2D (RenderColor / RenderBitmap) se emite en unidades del
//     ortho, que es `gluOrtho2D(0, DAT_0056156c, 0, DAT_00561570)`, y la GPU la
//     estira hasta el viewport.  Un quad de ancho W se ve W * (viewport/ortho).
//   · Los GLIFOS, en cambio, los pinta wglUseFontBitmaps + glBitmap, que hace
//     un blit 1:1 EN PÍXELES DE FRAMEBUFFER desde el raster position.  NO se
//     estiran.  Y `GetTextExtentPointA` mide en esos mismos píxeles.
//
// O sea que el ancho del texto y el ancho de su fondo viven en unidades
// distintas, y hay que dividir el extent por la relación viewport/ortho para
// que el recuadro cubra exactamente las letras.
//
// Deliberadamente NO usamos `g_fScreenRate_x` (_DAT_055c9b70) ni
// `FUN_00511950` para esto: son dos fuentes de escala que en nuestro build
// están DESINCRONIZADAS.  `g_fScreenRate_x` sale de `g_ScreenW` (Config_Load),
// mientras que el ortho y el viewport salen de `DAT_0056156c` — y son dos
// variables separadas (globals.cpp:303 vs Config_Load.cpp:49), donde nada
// copia una a la otra.  Preguntarle a OpenGL por su viewport es la única
// fuente que no puede desincronizarse, y sigue siendo correcta si algún día
// se unifican esos globals.
static void Text_PixelToOrthoScale(float* outX, float* outY)
{
    *outX = 1.0f;
    *outY = 1.0f;
    GLint vp[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, vp);
    DWORD ow = DAT_0056156c ? DAT_0056156c : 640;
    DWORD oh = DAT_00561570 ? DAT_00561570 : 480;
    if (vp[2] > 0 && ow > 0) *outX = (float)vp[2] / (float)ow;
    if (vp[3] > 0 && oh > 0) *outY = (float)vp[3] / (float)oh;
    if (*outX <= 0.0f) *outX = 1.0f;
    if (*outY <= 0.0f) *outY = 1.0f;
}

// Ancho del texto EN UNIDADES DEL ORTHO (que es donde vive todo el layout).
// Es el equivalente correcto, para nuestro pipeline, del `sz.cx /
// g_fScreenRate_x` que hace IDA en RenderText (0x47F650) y RenderTipText
// (0x47F7F0).
// Escala pixeles-de-framebuffer -> unidades del ortho en las que dibuja el
// stack de texto (la misma que aplica FUN_0040f610 a los glifos).
//
// OJO, NO es lo mismo que g_fScreenRate_x: ese es el factor del BINARIO, que
// pasa a espacio-640 porque su CUIRenderText reescala internamente. En nuestro
// pipeline el reescalado lo hace el ortho, asi que un ancho medido con
// GetTextExtentPointA hay que dividirlo por ESTE factor para mezclarlo con el
// layout. Usar los dos mezclados deja la caja y el texto a escalas distintas
// (ver el fix del ancho del tooltip en CharMenu_Build.cpp).
extern "C" float Text_GetOrthoScaleX(void)
{
    float sx, sy;
    Text_PixelToOrthoScale(&sx, &sy);
    (void)sy;
    return sx;
}

extern "C" int Text_MeasureOrthoWidth(const char* text)
{
    if (!text || !*text) return 0;
    HDC hFontDC = DAT_055c9fec;
    if (!hFontDC) return 0;
    SIZE sz = {0, 0};
    if (!GetTextExtentPointA(hFontDC, text, (int)strlen(text), &sz)) return 0;
    float sx, sy;
    Text_PixelToOrthoScale(&sx, &sy);
    return (int)((float)sz.cx / sx);
}

// ═══════════════════════════════════════════════════════════════════════════
// Marcadores de estilo embebidos:  "\x02" <byte-de-estilo>
// ---------------------------------------------------------------------------
// Port de sub_4104B0 (parser) + sub_4102E0 (tabla estilo→colores) + la regla de
// selección de color por columna de sub_4105F0.
//
// Dónde viven en el binario:  CUIRenderText (0x40F610) es SOLO un dispatcher
// (`(*pSubclass->vtable[1])(...)`).  El subclass lo instancia `sub_40F570` desde
// OpenFont según `g_iRenderTextType` (registro SOFTWARE\Webzen\Mu\Config
// "TextOut"):
//   · type != 1 → sub_410A90, objeto de 4 bytes (solo vtable).  Su RenderText
//                 (0x410AF0) NO conoce marcadores: rasteriza el string entero
//                 con TextOutA y lo sube como una textura de un solo color.
//   · type == 1 → sub_40F730, objeto de 0x2C4 bytes.  Su RenderText (0x40FB70)
//                 va a sub_40FCD0 → sub_4104B0 (parsea y ELIMINA los
//                 marcadores) → sub_4105F0 (pinta cada columna con el color del
//                 tramo activo), con caché de texturas por string.
//
// DESVIACIÓN CONSCIENTE: nuestro FUN_0040f610 es una reimplementación propia
// basada en glifos (wglUseFontBitmapsA), sin la partición type-0/type-1 del
// original ni su caché de texturas.  Aplicamos siempre el comportamiento del
// type-1, porque el alternativo es dibujar los bytes de control como glifos
// basura.  Consecuencia: un cliente vanilla con "TextOut" != 1 muestra esa
// basura y el nuestro no.
//
// Layout del objeto original (para poder volver a cruzarlo con IDA):
//   this+36 + 16*i → charIndex   (índice en el texto YA limpio)
//   this+40 + 16*i → pixelX      (extent del prefijo − 1; umbral de columna)
//   this+44 + 16*i → fg
//   this+48 + 16*i → bg
//   this+196       → cantidad de marcadores
//   memset(this+36, 0, 0xA0)  ⇒ 10 entradas de 16 bytes
// ═══════════════════════════════════════════════════════════════════════════

#define TEXT_STYLE_MAX_MARKERS 10   // 0xA0 / 16

struct TextStyleMarker {
    int   charIndex;    // this+36+16*i
    int   pixelX;       // this+40+16*i  (tal cual lo guarda el binario)
    int   pixelStart;   // NO está en el binario: el extent crudo (pixelX+1),
                        // que es donde arranca el tramo.  El original solo
                        // necesita el umbral, nosotros necesitamos posicionar.
    DWORD fg;           // this+44+16*i
    DWORD bg;           // this+48+16*i
};

// sub_4102E0 — tabla estilo→colores.  Port literal, incluidas las constantes
// tal como las muestra el decompile (decimales con signo).  Los DWORD están en
// orden de memoria RGBA (0xAABBGGRR), igual que m_dwTextColor/m_dwBackColor.
static void Text_StyleColors(char style, DWORD *fg, DWORD *bg)
{
    DWORD result = 0;
    DWORD v4;
    switch (style) {
        case 1:
        case -12: v4 = (DWORD)-16776961; break;                 // 0xFF0000FF
        case -16: v4 = m_dwTextColor; result = m_dwBackColor; break;  // reset
        case -15: v4 = m_dwBackColor; result = m_dwTextColor; break;  // invertido
        case -14: v4 = (DWORD)-14116;    break;                 // 0xFFFFC8DC
        case -13: v4 = (DWORD)-16711736; break;                 // 0xFF00FFC8
        case -11: v4 = (DWORD)-11521516; break;                 // 0xFF503214
        case -10: v4 = (DWORD)-16751556; break;                 // 0xFF00643C
        case -9:  v4 = (DWORD)-16777116; break;                 // 0xFF000064
        case -8:
            v4 = (DWORD)-3613466;                               // 0xFFC8DCE6
            if (!DAT_005590ac /* g_bUseChatListBox */)
                result = (DWORD)-1778384896;                    // 0x96000000
            break;
        default:  v4 = 0xFFFFFFFFu; break;
    }
    *fg = v4;
    *bg = result;
}

// sub_4104B0 — extrae los marcadores y devuelve el texto sin ellos.
// El original limpia el string IN PLACE sobre el buffer del caller; acá el
// caller nos pasa `const char*`, así que escribimos en `dst` (que debe tener
// al menos 0x100 bytes, igual que el `Destination[292]` de sub_40FCD0).
static int Text_ParseStyleMarkers(const char *src, char *dst, size_t dstCap,
                                  TextStyleMarker *out)
{
    memset(out, 0, TEXT_STYLE_MAX_MARKERS * sizeof(TextStyleMarker));

    int len = (int)strlen(src);
    int rd = 0;          // v5 — índice de lectura
    int consumed = 0;    // v6 — bytes de marcador ya consumidos
    int n = 0;           // v3 — cantidad de marcadores

    while (rd < len) {
        if (consumed >= 18) break;          // tope del original: 9 marcadores
        if (src[rd] == 2) {
            out[n].charIndex = rd - consumed;
            ++rd;
            if (rd >= len) break;           // marcador truncado al final
            Text_StyleColors(src[rd], &out[n].fg, &out[n].bg);
            ++n;
            consumed += 2;
        }
        ++rd;
    }

    if (n == 0) return 0;

    // Strip: copia todo menos los pares "\x02 <estilo>".
    size_t w = 0;
    for (int r = 0; r < len && w + 1 < dstCap; ) {
        char ch = src[r];
        if (ch == 2) { r += 2; continue; }
        dst[w++] = ch;
        ++r;
    }
    dst[w] = '\0';

    // pixelX = GetTextExtentPointA(texto limpio, charIndex).cx − 1  (o 0).
    HDC hFontDC = DAT_055c9fec;
    for (int i = 0; i < n; ++i) {
        SIZE sz = {0, 0};
        int  nch = out[i].charIndex;
        if (nch > (int)w) nch = (int)w;
        if (hFontDC && nch > 0) GetTextExtentPointA(hFontDC, dst, nch, &sz);
        out[i].pixelStart = sz.cx;
        out[i].pixelX     = sz.cx ? sz.cx - 1 : 0;
    }
    return n;
}

void __cdecl FUN_0040f610(HDC /*hdc_unused*/, int x, int y, const char *text, DWORD /*color_unused*/)
{
    if (text == NULL || *text == '\0') return;

    // ── DIAG: log first call per second to see what text is being requested
    {
        static DWORD s_lastTxt = 0;
        DWORD now = GetTickCount();
        if (now - s_lastTxt > 1000) {
            s_lastTxt = now;
            char b[200];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "TEXT call x=%d y=%d str='%s' color=0x%08X",
                x, y, text ? text : "(null)", (unsigned)DAT_00559c78);
            DbgLogPublic(b);
        }
    }

    // ── Marcadores de estilo "\x02<estilo>" (ver bloque de arriba) ───────────
    // El guard `strlen <= 0xFF` es el de sub_40FCD0 (el original solo parsea
    // strings que entran en su Destination[292]).
    char             strippedBuf[0x100];
    TextStyleMarker  markers[TEXT_STYLE_MAX_MARKERS];
    int              nMarkers = 0;
    const char      *drawText = text;
    if (strchr(text, 2) != NULL && strlen(text) <= 0xFF) {
        nMarkers = Text_ParseStyleMarkers(text, strippedBuf, sizeof(strippedBuf), markers);
        if (nMarkers > 0) drawText = strippedBuf;
        if (*drawText == '\0') return;
    }

    HGLRC hRC = wglGetCurrentContext();
    if (!hRC) return;

    HDC hFontDC = DAT_055c9fec;
    if (hFontDC == NULL) return;

    // ── FUENTE ACTIVA (fix 2026-07-20) ──────────────────────────────────────
    // Acá había `hFont = DAT_055ca00c` HARDCODEADO (la fuente regular), pero
    // los callers seleccionan fuentes DISTINTAS en este mismo DC antes de
    // llamarnos: g_hFontBold (chat, HUD_Pass2/3/4, sub_40CE20 vía
    // CUIRenderText::SetFont) y g_hFontBig (HUD_Pass3:486, doble tamaño).
    //
    // Consecuencia doble:
    //   1. Los glifos salían SIEMPRE en regular — bold y big nunca se veían.
    //   2. El recuadro de fondo se medía con GetTextExtentPointA usando la
    //      fuente que el caller seleccionó (bold/big = más ancha) mientras las
    //      letras se dibujaban en regular (más angosta) → el fondo excedía al
    //      texto.  Y cuando el caller sí había dejado la regular, calzaba.
    //      De ahí el "a veces sobra, a veces no" que quedaba después de
    //      arreglar la escala viewport/ortho.
    //
    // Ahora la fuente sale del DC (que es la que efectivamente usó el caller
    // para medir), y cacheamos las display lists POR fuente.
    HFONT hFont = (HFONT)GetCurrentObject(hFontDC, OBJ_FONT);
    if (hFont == NULL) hFont = (HFONT)(uintptr_t)DAT_055ca00c;
    if (hFont == NULL) return;

    // Cache de hasta 4 fuentes (regular / bold / big / repuesto).  Antes era
    // una sola entrada, así que alternar fuentes entre llamadas habría
    // reconstruido 256 display lists en CADA llamada.
    struct FontLists { HGLRC rc; HFONT font; GLuint base; int ascent; };
    static FontLists s_cache[4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };
    static int       s_next = 0;

    int slot = -1;
    for (int i = 0; i < 4; ++i) {
        if (s_cache[i].base != 0 && s_cache[i].rc == hRC && s_cache[i].font == hFont) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        slot = s_next;
        s_next = (s_next + 1) & 3;
        if (s_cache[slot].base != 0) {
            glDeleteLists(s_cache[slot].base, 256);
            s_cache[slot].base = 0;
        }
        GLuint base = glGenLists(256);
        if (base == 0) return;
        // hFont ya está seleccionada en el DC (de ahí la sacamos), así que no
        // hace falta SelectObject — el que había acá encima pisaba la elección
        // del caller y contaminaba las mediciones posteriores de otro código.
        if (!wglUseFontBitmapsA(hFontDC, 0, 256, base)) {
            glDeleteLists(base, 256);
            return;
        }
        TEXTMETRICA tm;
        int ascent = 11;
        if (GetTextMetricsA(hFontDC, &tm)) ascent = tm.tmAscent;
        s_cache[slot].rc     = hRC;
        s_cache[slot].font   = hFont;
        s_cache[slot].base   = base;
        s_cache[slot].ascent = ascent;
    }
    const GLuint s_fontListBase = s_cache[slot].base;
    const int    s_ascent       = s_cache[slot].ascent;

    // Y-flip: game pasa y con top=0, ortho GL usa bottom=0.
    extern DWORD DAT_00561570;  // alto del ortho 2D
    DWORD vh = DAT_00561570 ? DAT_00561570 : 480;
    int   rasterY = (int)vh - y - s_ascent;

    // Color base desde DAT_00559c78 (COLORREF 0x00BBGGRR + opcional alpha en
    // byte 3).  Con marcadores presentes esto es solo el color del PRIMER tramo;
    // el resto sale de markers[].fg (ver el loop de abajo).
    //
    // 2026-05-04: respetar el alpha que el CALLER setea via glColor4f(...,α)
    // antes de RenderText. Antes pisábamos con `glColor4ub(R,G,B,A)` y se
    // perdía el cross-fade del banner clase/zona (los dos textos siempre a
    // α=1 → solapaban). Multiplicamos los alphas para que tanto el del
    // texto-color (DAT_00559c78 byte 3) como el del caller respeten su
    // contribución.
    GLfloat curColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glGetFloatv(GL_CURRENT_COLOR, curColor);
    GLubyte callerA = (GLubyte)(curColor[3] * 255.0f + 0.5f);

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIST_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ── FONDO + GLIFOS, POR TRAMOS DE COLOR ──────────────────────────────────
    // Sin marcadores hay un único tramo con (m_dwTextColor, m_dwBackColor), o
    // sea exactamente el comportamiento previo.
    //
    // El original (sub_4105F0) resuelve el color por COLUMNA de píxel: busca el
    // último marcador cuyo pixelX sea menor que la columna.  Como pixelX es el
    // extent del prefijo, ese corte cae justo en el borde del carácter, así que
    // partir por charIndex — que es lo natural para un renderer de glifos — da
    // el mismo resultado.
    //
    // FONDO: 2026-07-19 — faltaba por completo.  En el original el subclass
    // pinta el fondo (m_dwBackColor / bg del marcador) en los píxeles NO-texto
    // del tramo; por eso los mensajes del chat salían sin su recuadro.
    // Formato 0xAABBGGRR igual que el color de texto.  Alpha 0 = sin fondo.
    {
        float sx, sy;
        Text_PixelToOrthoScale(&sx, &sy);

        const int drawLen = (int)strlen(drawText);
        const int nRuns   = nMarkers + 1;

        glListBase(s_fontListBase);

        for (int run = 0; run < nRuns; ++run) {
            int startChar = (run == 0)        ? 0        : markers[run - 1].charIndex;
            int endChar   = (run == nMarkers) ? drawLen  : markers[run].charIndex;
            if (startChar < 0)        startChar = 0;
            if (startChar > drawLen)  startChar = drawLen;
            if (endChar   > drawLen)  endChar   = drawLen;
            if (endChar <= startChar) continue;

            DWORD fg    = (run == 0) ? DAT_00559c78 : markers[run - 1].fg;
            DWORD bc    = (run == 0) ? DAT_00559c80 : markers[run - 1].bg;
            int   runPx = (run == 0) ? 0            : markers[run - 1].pixelStart;
            float runX  = (float)x + (float)runPx / sx;

            SIZE bsz = {0, 0};
            BOOL haveExtent = GetTextExtentPointA(hFontDC, drawText + startChar,
                                                  endChar - startChar, &bsz);

            GLubyte bA = (GLubyte)((bc >> 24) & 0xFF);
            if (bA != 0 && haveExtent && bsz.cx > 0) {
                GLubyte bR = (GLubyte)( bc        & 0xFF);
                GLubyte bG = (GLubyte)((bc >>  8) & 0xFF);
                GLubyte bB = (GLubyte)((bc >> 16) & 0xFF);
                GLubyte bFinal = (GLubyte)((unsigned)bA * (unsigned)callerA / 255u);
                // glRasterPos deja el ORIGEN DEL GLIFO en la baseline, así que el
                // texto ocupa [rasterY - descent, rasterY + ascent]. Antes usaba
                // `rasterY-2 .. rasterY+cy-2`, que corría la caja hacia arriba y
                // dejaba aire abajo. descent = cy - ascent.
                int descent = (int)bsz.cy - s_ascent;
                if (descent < 0) descent = 0;
                // bsz está en píxeles de framebuffer (así mide GDI y así
                // blitea glBitmap); el quad se emite en unidades del ortho y la
                // GPU lo estira.  Sin esta división el fondo salía más ancho
                // que las letras exactamente por la relación viewport/ortho.
                float wOrtho    = (float)bsz.cx  / sx;
                float ascOrtho  = (float)s_ascent / sy;
                float descOrtho = (float)descent  / sy;
                float x0 = runX - 1.0f;
                float x1 = runX + wOrtho + 1.0f;
                float y0 = (float)rasterY - descOrtho;
                float y1 = (float)rasterY + ascOrtho;
                glColor4ub(bR, bG, bB, bFinal);
                glBegin(GL_QUADS);
                    glVertex2f(x0, y0);
                    glVertex2f(x1, y0);
                    glVertex2f(x1, y1);
                    glVertex2f(x0, y1);
                glEnd();
            }

            GLubyte R = (GLubyte)( fg        & 0xFF);
            GLubyte G = (GLubyte)((fg >>  8) & 0xFF);
            GLubyte B = (GLubyte)((fg >> 16) & 0xFF);
            GLubyte A = (GLubyte)((fg >> 24) & 0xFF);
            if (A == 0) A = 0xFF;
            GLubyte finalA = (GLubyte)((unsigned)A * (unsigned)callerA / 255u);

            glColor4ub(R, G, B, finalA);
            glRasterPos2f(runX, (float)rasterY);
            glCallLists((GLsizei)(endChar - startChar), GL_UNSIGNED_BYTE,
                        drawText + startChar);
        }
    }
    glPopAttrib();
}

// FUN_0043e820 @ 0x0043E820 — SetAction(object_ptr, action)
// Sets animation action on an OBJECT. Checks action < Models[type].numActions,
// with exceptions for actions 77 and 76. If action changes, saves prior state.
//
// Models base = DAT_05828d58 (already a pointer, no extra indirection).
// Per disasm @ 0x0043e830-0x0043e836:
//   ECX = [0x05828d58]                       ; table base
//   slot offset = type * 47 * 4 = type*0xBC
//   numLimit = *(short*)(ECX + slot_off + 0x26)
// El offset 0x26 en el slot BMD es nBones (no nActions); el original valida
// el action_id contra ese campo igualmente — replicamos sin reinterpretar.
//
// BUG previo: agregábamos un deref *(DWORD*)slot que leía los primeros 4 bytes
// del modelName ("Play"=0x79616C50) como puntero y crasheaba en *(short*)(0x79616C50+38).
void* __cdecl FUN_0043e820(int entity_ptr, int anim_id)
{
    if (entity_ptr == 0) return NULL;
    if (DAT_05828d58 == 0) return NULL;           // tabla aún no inicializada
    short objType = *(short*)(entity_ptr + 0x02); // OBJECT.Type
    short numActions = *(short*)(DAT_05828d58 + (int)objType * 0xBC + 0x26);

    // Allow action if < numActions, or if it's 77 (0x4D) or 76 (0x4C)
    if (anim_id >= numActions && anim_id != 77 && anim_id != 76)
        return NULL;

    BYTE curAction = *(BYTE*)(entity_ptr + 0x105); // OBJECT.CurrentAction
    if ((int)(unsigned int)curAction != anim_id) {
        *(BYTE*)(entity_ptr + 0x106) = curAction;             // PriorAction
        *(float*)(entity_ptr + 0x10C) = *(float*)(entity_ptr + 0x108); // PriorAnimationFrame = AnimationFrame
        *(BYTE*)(entity_ptr + 0x105) = (BYTE)anim_id;         // CurrentAction
        *(float*)(entity_ptr + 0x108) = 0.0f;                 // AnimationFrame = 0
    }
    return (void*)entity_ptr;
}

// GL / 2D render
// FUN_005126e0 @ 0x005126E0 — GL_DrawRotatedRect
// Draws a textured, rotated 2D quad using GL_TRIANGLE_FAN.
// x,y = center angles (converted via sin/cos), w,h = half-extents,
// color = z-depth as float bits.
void __cdecl FUN_005126e0(int id, float x, float y, float w, float h, unsigned int color)
{
    float fSinX = (float)FUN_00511950(x);
    float fCosX = (float)FUN_00511980(y);
    float fSinW = (float)FUN_00511950(w);
    float fCosW = (float)FUN_00511980(h);
    FUN_00511480(id);
    float sz = (float)DAT_00561570;
    float depth = *(float*)&color;

    // Build rotation matrix from direction vector pointing at (depth)
    float bvec[3] = { 0.0f, 0.0f, depth };
    float mat[12];
    FUN_004f9db0(bvec, mat);

    // 4 corner UV + positions
    static float uvs[8] = { 0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f };
    float corners[4][3] = {
        { fSinW * _DAT_00552a14,  fCosW * _DAT_00552504, 0.0f },
        { fSinW * _DAT_00552a14, -fCosW * _DAT_00552504, 0.0f },
        {-fSinW * _DAT_00552a14,  fCosW * _DAT_00552504, 0.0f },
        {-fSinW * _DAT_00552a14, -fCosW * _DAT_00552504, 0.0f }
    };

    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 4; i++) {
        float out[3];
        FUN_004fa0b0(corners[i], mat, out);
        glTexCoord2f(uvs[i*2], uvs[i*2+1]);
        glVertex2f(fSinX + out[0], (sz - fCosX) + out[1]);
    }
    glEnd();
}

// Guild / party UI

// FUN_0051ddf0 @ 0x0051DDF0 — GuildMemberList_Render()
// Renders guild member list (name, level, class, kills, deaths) using
// FUN_0051ddf0 @ 0x0051DDF0 — GuildLeaderboard_Render
// Draws guild war score leaderboard: title, current player name, column headers,
// then one row per member (rank, name, kills, deaths, score).
// Member array base: DAT_083a7af8, stride 0x18 per entry.
// DAT_083a7c30 = member count; DAT_083a7c34 = own rank.
int __cdecl FUN_0051ddf0(void)
{
    return 0;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
    int xCols[5];
    xCols[0] = 0xdb; // col 0 X (rank)
    int x_110 = 0xeb; // col 1 X (name)
    int x_10c = 0x11f; // col 2 X (kills)
    int x_108 = 0x159; // col 3 X (deaths)
    int x_104 = 0x17f; // col 4 X (score)
    xCols[1] = x_110; xCols[2] = x_10c; xCols[3] = x_108; xCols[4] = x_104;

    glColor3f(0.5f, 1.0f, 0.5f);
    FUN_0047f650(0xe5, 0x46, &DAT_07d59358, (LPSIZE)0, '\0', 0);

    char buf[256];
    wsprintfA(buf, &param_2_07d59484, (char *)((int)DAT_07abf5d8 + 0x1c1));
    FUN_0047f650(0xe5, 0x56, buf, (LPSIZE)0, '\0', 0);

    glColor3f(0.5f, 0.5f, 1.0f);
    // Column headers: array at DAT_07d5ba04, stride 300 bytes, 5 entries to 0x7d5bfe0
    int *pXCol = xCols;
    int iColIdx = 0;
    const char *pHdr = &DAT_07d5ba04;
    while ((int)pHdr < 0x7d5bfe0) {
        if (pHdr != &DAT_07d5bfe0) {
            int xOff = (iColIdx != 2) ? 0 : 10;
            FUN_0047f650(*pXCol + xOff, 0x6e, pHdr, (LPSIZE)0, '\0', 0);
        }
        pHdr += 300;
        iColIdx++;
        pXCol++;
    }

    int iY = 0x7e;
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < DAT_083a7c30; i++) {
        int iNext = i + 1;
        int iMod  = iNext % DAT_083a7c30;
        const char *fmtRank;
        int iRank;
        if (i == DAT_083a7c30 - 1) {
            // last entry: own rank highlighted
            glColor3f(0.4f, 0.4f, 0.0f);
            FUN_0047f650(0xdb, 0x11e, &DAT_07d5bfe0, (LPSIZE)0, '\0', 0);
            iY = 0x12e;
            fmtRank = PTR_DAT_005618a0;
            iRank   = DAT_083a7c34;
        } else {
            fmtRank = &param_2_005618a4;
            iRank   = iNext;
        }
        wsprintfA(buf, fmtRank, iRank);
        DAT_00559c78 = 0xffffffff;
        FUN_0047f650(0xdb, iY, buf, (LPSIZE)0, '\0', 0);

        // member name (char[12] starting at DAT_083a7af8 + iMod*0x18)
        char namebuf[14] = {};
        *(DWORD *)namebuf       = *(DWORD *)((BYTE *)&DAT_083a7af8 + iMod * 0x18);
        *(DWORD *)(namebuf + 4) = *(DWORD *)((BYTE *)&DAT_083a7afc + iMod * 0x18);
        *(WORD  *)(namebuf + 8) = *(WORD  *)((BYTE *)&DAT_083a7b00 + iMod * 0x18);
        namebuf[13] = '\0';
        FUN_0047f650(x_110, iY, namebuf, (LPSIZE)0, '\0', 0);

        DAT_00559c78 = 0xffffd2d2;
        wsprintfA(buf, &param_2_005618a8, *(DWORD *)((BYTE *)&DAT_083a7b04 + iMod * 0x18));
        FUN_0047f650(x_10c, iY, buf, (LPSIZE)0, '\0', 0);

        DAT_00559c78 = 0xffd2ffd2;
        wsprintfA(buf, &param_2_005618b0, *(DWORD *)((BYTE *)&DAT_083a7b08 + iMod * 0x18));
        FUN_0047f650(x_108, iY, buf, (LPSIZE)0, '\0', 0);

        DAT_00559c78 = 0xffd2d2ff;
        wsprintfA(buf, &param_2_005618b4, *(DWORD *)((BYTE *)&DAT_083a7b0c + iMod * 0x18));
        FUN_0047f650(x_104, iY, buf, (LPSIZE)0, '\0', 0);

        iY += 0x10;
    }
    return 0;
}

// FUN_0051db00 @ 0x0051DB00 — GuildOverview_Render
// Renders guild war overview panel: win/draw title, total score, kills, deaths.
// Uses GetTextExtentPointA for centering. unaff_EDI is the text width from
// the last GetTextExtentPointA call — approximated via lstrlenA * pixel_per_char.
// DAT_083a7c30=0 → "draw" strings; !=0 → "win/loss" strings.
int __cdecl FUN_0051db00(void)
{
    glColor3f(0.5f, 0.5f, 0.5f);
    const char *title, *subtitle;
    tagSIZE sz = {};
    HDC hdc = (HDC)(uintptr_t)DAT_055c9fec;
    if (DAT_083a7c30 == 0) {
        title    = lpString_07d68bc8;
        subtitle = lpString_07d68cf4;
    } else {
        title    = lpString_07d68970;
        subtitle = lpString_07d68a9c;
    }
    int len = lstrlenA(title);
    GetTextExtentPointA(hdc, title, len, &sz);
    int cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    FUN_0047f650(0x140 - cx, 0x46, title, (LPSIZE)0, '\0', 0);

    len = lstrlenA(subtitle);
    GetTextExtentPointA(hdc, subtitle, len, &sz);
    cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    FUN_0047f650(0x140 - cx, 0x56, subtitle, (LPSIZE)0, '\0', 0);

    glColor3f(1.0f, 1.0f, 1.0f);
    DAT_00559c78 = 0xffffffff;
    SelectObject(hdc, (HGDIOBJ)(uintptr_t)DAT_055ca010);

    // Score total
    char buf[256];
    DAT_00559c78 = 0xffd2ffd2;
    wsprintfA(buf, &param_2_07d68e20);
    len = lstrlenA(buf);
    GetTextExtentPointA(hdc, buf, len, &sz);
    cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    FUN_0047f650(0x140 - cx, 0x6a, buf, (LPSIZE)0, '\0', 0);

    int iY = 0x82;
    if (DAT_083a7c30 != 0) {
        DAT_00559c78 = 0xffd2d2ff;
        wsprintfA(buf, &param_2_07d68f4c, DAT_083a7b0c);
        len = lstrlenA(buf);
        GetTextExtentPointA(hdc, buf, len, &sz);
        cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
        FUN_0047f650(0x140 - cx, 0x82, buf, (LPSIZE)0, '\0', 0);
        iY = 0x9a;
    }
    DAT_00559c78 = 0xffffd2d2;
    wsprintfA(buf, &param_2_07d69078, DAT_083a7b04);
    len = lstrlenA(buf);
    GetTextExtentPointA(hdc, buf, len, &sz);
    cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    return (int)FUN_0047f650(0x140 - cx, iY, buf, (LPSIZE)0, '\0', 0);
}

// CreateOkMessageBox @ 0x0051D6F0 — show an OK dialog by setting the UI state.
// Wraps text at 7 chars / 0x26 lines into DAT_083a44c4, sets a fixed panel descriptor,
// then transitions DAT_083a7c24 or DAT_083a7c28 to state 0x8b.
void __cdecl CreateOkMessageBox(char *msg)
{
    DAT_083a4324 = SeparateTextIntoLines(msg, (char *)&DAT_083a44c4, 7, 0x26);
    // clear panel descriptor (10 dwords)
    int *p = (int*)&DAT_083a42f8[0];
    for (int i = 0; i < 10; i++) p[i] = 0;
    // set fixed descriptor: {1, 0x47, 0x8c, 0x46, 0x15}
    DAT_083a42f8[0] = 1;
    DAT_083a42f8[1] = 0x47;
    DAT_083a42f8[2] = 0x8c;
    DAT_083a42f8[3] = 0x46;
    DAT_083a42f8[4] = 0x15;
    if (DAT_083a7c24 != 0)
        DAT_083a7c28 = 0x8b;
    else
        DAT_083a7c24 = 0x8b;
}
// FUN_0051d9e0 @ 0x0051D9E0 — GuildMemberList_Update
// Sets UI state 0x8c, stores count/param2, copies menu descriptor and member list data.
void __cdecl FUN_0051d9e0(int count, int p2, void *data)
{
    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x8c;
    else                   DAT_083a7c28 = 0x8c;
    DAT_083a7c34 = (DWORD)p2;
    DAT_083a7c30 = count;
    // Copy menu descriptor: 5 DWORDs
    DWORD desc[5] = { 1, 0x47, 0x104, 0x46, 0x15 };
    for (int i = 0; i < 5; i++) DAT_083a42f8[i] = desc[i];
    // Copy member list data: count * 0x18 bytes into DAT_083a7af8
    unsigned int dwords = (unsigned int)(count * 0x18) >> 2;
    unsigned int *src = (unsigned int*)data;
    unsigned int *dst = (unsigned int*)&DAT_083a7af8;
    for (unsigned int i = 0; i < dwords; i++) *dst++ = *src++;
}

// FUN_0051da80 @ 0x0051DA80 — GuildMemberList_Add (single member record)
// Sets UI state 0x9a, stores param_1 as member count, copies 5-entry menu desc + 6 DWORDs data.
void __cdecl FUN_0051da80(int p1, void *data)
{
    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x9a;
    else                   DAT_083a7c28 = 0x9a;
    DAT_083a7c30 = (int)(DWORD)p1;
    DWORD desc[5] = { 1, 0x47, 0x82, 0x46, 0x15 };
    for (int i = 0; i < 5; i++) DAT_083a42f8[i] = desc[i];
    DWORD *src = (DWORD*)data;
    DWORD *dst = (DWORD*)&DAT_083a7af8;
    for (int i = 0; i < 6; i++) *dst++ = *src++;
}

// FUN_0051d840 @ 0x0051D840 — ItemList_Select(slot)
// Selects character slot `slot` for the in-game item/skill list display.
// Sets DAT_005615dc, populates DAT_083a4324 and skill/item display arrays,
// then transitions UI state to 0x8e.
int __cdecl FUN_0051d840(int param_1) {
    DAT_005615dc = (DWORD)param_1;
    // SeparateTextIntoLines and skill data population skipped
    // (requires DAT_07cf5608 char data arrays not yet mapped)
    DAT_083a7c0c = 1;
    if (DAT_083a7c24 == 0) {
        DAT_083a7c24 = 0x8e;
    } else {
        DAT_083a7c28 = 0x8e;
    }
    return 1;
}
// FUN_0051e7e0 — implemented in src/Scene/Scene_ServerSelect_Input.cpp

// CRT / string helpers

// FUN_00541eab @ 0x00541EAB — IsLeadByte(str)
// DBCS lead-byte check using CRT _pctype table at DAT_083bc1a0.
// Bit 2 of table[*str+1] set → double-byte (return 2), else single-byte (return 1).
// Implemented via Win32 IsDBCSLeadByteEx(949) to avoid needing the 256-byte table.
int __cdecl FUN_00541eab(byte *param_1) {
    return IsDBCSLeadByteEx(949, *param_1) ? 2 : 1;
}
// FUN_0053d5a0 @ 0x0053D5A0 — Resource_Load(filename)
// Calls FUN_0053ed30(DAT_083bbb14, filename) if resource manager is initialized.
// Returns non-zero on success. DAT_083bbb14 is the resource manager context pointer.
// FUN_0053ed30 not implemented — returning 0 (no-op stub).
unsigned int  __cdecl FUN_0053d5a0(char *path)
{
    if (DAT_083bbb14 == 0) return 0;
    FUN_0053ed30((void *)(ULONG_PTR)DAT_083bbb14, path);
    return 1;
}
// FUN_0053d5c0 @ 0x0053D5C0 — Pipe_QueryResource
unsigned int  __cdecl FUN_0053d5c0(char *path)
{
    if (DAT_083bbb14 == 0) return 0;
    return FUN_0053ed00((void *)(ULONG_PTR)DAT_083bbb14, path);
}

// FUN_005030c0 @ 0x005030C0 — Entity_GravityInit(entity_ptr)
// Sets initial gravity velocity components at +0x1c/+0x20/+0x24 and scale +0x0c
// based on entity type (short at +2). Each entity type has hardcoded float offsets.
void __cdecl FUN_005030c0(int param_1) {
    short sVar1 = *(short*)(param_1 + 2);
    *(unsigned int*)(param_1 + 0x1c) = 0;
    *(unsigned int*)(param_1 + 0x20) = 0;
    *(unsigned int*)(param_1 + 0x24) = 0xc2340000; // -45.0f
    if ((399 < sVar1) && (sVar1 < 0x1d0)) {
        *(unsigned int*)(param_1 + 0x1c) = 0x42700000; // 60.0f
        if (sVar1 == 0x1a3)
            *(unsigned int*)(param_1 + 0xc) = 0x3f333333; // 0.7f
        return;
    }
    if (((0x217 < sVar1) && (sVar1 < 0x221)) || sVar1 == 0x222) {
        *(unsigned int*)(param_1 + 0x1c) = 0x42b40000; // 90.0f
        *(unsigned int*)(param_1 + 0x20) = 0;
        return;
    }
    if (sVar1 < 0x1d0) {
        if (!(0x24f < sVar1)) { *(unsigned int*)(param_1 + 0x1c) = 0; return; }
    } else {
        if (sVar1 < 0x250) {
            *(unsigned int*)(param_1 + 0x1c) = 0;
            *(unsigned int*)(param_1 + 0x20) = 0x43870000; // 270.0f
            return;
        }
        if (sVar1 < 0x270) {
            *(unsigned int*)(param_1 + 0x20) = 0x43870000;
            *(unsigned int*)(param_1 + 0x24) = 0x43610000; // 225.0f
            return;
        }
    }
    if ((0x28f < sVar1) && (sVar1 < 0x2f0)) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; return; }
    // [0x310, 0x32f]: x=270 + z=45
    if (!((sVar1 < 0x310) || (0x32f < sVar1))) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; goto lbl_z45; }
    if (sVar1 == 0x3b7) { *(unsigned int*)(param_1 + 0x1c) = 0x42b40000; return; }
    if ((sVar1 == 0x3bb) || (sVar1 == 0x3bc)) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; goto lbl_z45; }
    if (sVar1 == 0x3bd) { *(unsigned int*)(param_1 + 0xc) = 0x3e4ccccd; return; }
    if (sVar1 == 0x3b8) {
        *(unsigned int*)(param_1 + 0x1c) = 0;
        *(unsigned int*)(param_1 + 0x24) = 0x42340000; // 45.0f
        *(unsigned int*)(param_1 + 0xc)  = 0x3f8ccccd;
        return;
    }
    if (sVar1 == 0x367) {
        *(unsigned int*)(param_1 + 0x20) = 0x42340000;
        *(unsigned int*)(param_1 + 0x24) = 0x42340000;
        return;
    }
    if ((sVar1 == 0x368)||(sVar1 == 0x369)||(sVar1 == 0x36a)||(sVar1 == 0x33e)) goto lbl_z45;
    if (sVar1 == 0x361) { *(unsigned int*)(param_1 + 0x1c) = 0x42b40000; return; }
    if (sVar1 == 0x362) { *(unsigned int*)(param_1 + 0x24) = 0x43870000; *(unsigned int*)(param_1 + 0x1c) = 0x43870000; return; }
    if (sVar1 == 0x363) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; *(unsigned int*)(param_1 + 0x24) = 0x42b40000; return; }
    if (sVar1 == 0x3be) {
        *(unsigned int*)(param_1 + 0x1c) = 0x42e60000; *(unsigned int*)(param_1 + 0x20) = 0x42960000;
        *(unsigned int*)(param_1 + 0x24) = 0x41000000; *(unsigned int*)(param_1 + 0xc)  = 0x3ecccccd;
        return;
    }
    if (sVar1 == 0x365) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; *(unsigned int*)(param_1 + 0x24) = 0x42b40000; return; }
    if ((sVar1 == 0x3ba) || (sVar1 == 0x364)) goto lbl_z45;
    *(unsigned int*)(param_1 + 0x1c) = 0;
    return;
lbl_z45:
    *(unsigned int*)(param_1 + 0x24) = 0x42340000; // 45.0f
}

// FUN_00503650 @ 0x00503650 — Entity_SparkleUpdate(entity_ptr)
// Every 0x30 ticks spawns two Shiny01 particles (type 0x4ce) at random direction offset
// from entity's facing matrix. Stack layout: {local_48[3], 0, local_34} = random XZ offsets.
void __cdecl FUN_00503650(int param_1)
{
    int iVar2 = *(int *)(param_1 + 4);
    *(int *)(param_1 + 4) = iVar2 + 1;
    if (iVar2 % 0x30 == 0) {
        float *pfVar1 = (float *)(param_1 + 0x1c);
        float local_30[12];
        FUN_004f9db0(pfVar1, local_30);

        unsigned int uVar4 = (unsigned int)_rand() & 0x8000001f;
        if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
        float angX = (float)(int)(uVar4 + 0x10);

        uVar4 = (unsigned int)_rand() & 0x8000001f;
        if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
        float angZ = (float)(int)(uVar4 + 0x10);

        // Original stack layout: {angX, 0.0f, angZ} passed as float[3] to Matrix_TransformPoint
        float inVec[3] = { angX, 0.0f, angZ };
        float outPos[3];
        FUN_004fa0b0(inVec, local_30, outPos);
        outPos[0] += *(float *)(param_1 + 0x10);
        outPos[1] += *(float *)(param_1 + 0x14);
        outPos[2] += *(float *)(param_1 + 0x18);

        float size[4] = { 1.0f, 1.0f, 1.0f, angX };
        FUN_00475220(0x4ce, outPos, pfVar1, size, 0, 1.0f, 0);
        FUN_00475220(0x4ce, outPos, pfVar1, size, 1, 1.0f, 0);
    }
}


// FUN_00408940 @ 0x00408940 — Sound_UpdateChannel3D_Tick(channel)
// Updates 3D sound position sin/cos from entity facing angle at param_1[1]+0x24,
// scaled by random key _DAT_00590af0. Calls constraint update and validity check.
// IDA `sub_408940` devuelve `sub_408E30(this) != 0` — el "¿convergió?" que usa
// el bucle de `sub_408900`. El port lo descartaba (void).
int __cdecl FUN_00408940(int *param_1, float dt) {
    double angle = (*(float*)(param_1[1] + 0x24) + *(float*)&_DAT_005524ec) * *(float*)&_DAT_0055253c;
    DAT_00590af4 = (float)(sin(angle)  * *(float*)&_DAT_00590af0);
    DAT_00590af8 = (float)(-cos(angle) * *(float*)&_DAT_00590af0);
    FUN_00408cb0(param_1, dt);
    return FUN_00408e30((DWORD *)param_1) != 0;
}

// ── Particle / animation / bone math ─────────────────────────────────────────

// FUN_0043e820 — replaced by full implementation above (line ~6057)

// FUN_00443e70 @ 0x00443E70 — SetAttackSpeed
// Real logic (after anti-tamper hash table blocks):
//   1. Reads CharacterAttribute->MagicDamageMax and AttackDamageMinRight
//   2. Computes animation speed: fVar2 = AttackDamageMinRight * _DAT_005524bc
//      fStack_8 = MagicDamageMax * _DAT_005524bc, local_18 = MagicDamageMax * _DAT_005528e0
// SetAttackSpeed @ 0x00443E70 — set player animation speeds based on
// CharacterAttribute stats. Port FIEL desde IDA decompile (2026-05-02).
//
// Reads CharacterAttribute[+0x38] (AttackSpeed) and [+0x44] (MagicSpeed),
// computes scale factors, then writes per-animation speed floats into the
// player model's animation table (Models[Player=390].Data[+0x30]).
//
// Anti-tamper hash table operations (sub_403F80/sub_4041E0/sub_404370/etc
// wrapping CharacterMachine encrypt/decrypt) are skipped per project policy.
//
// Note: IDA decompile shows v33 and v38 as uninitialized stack locals used
// for FIST/SWORD/RIDE-attack speeds. The disasm doesn't show explicit
// assignments visible in hex-rays output — treating as 0 (stack default).
// For attack-speed stat scaling on melee, this means baseline speeds are
// used. Magic skills (which use v34 = AttackSpeed*0.004 and v35/v39 from
// MagicSpeed) DO scale per-stat correctly.
//
// Animation table layout (from Models[390]+0x30 base, P):
//   P+548   FIST          v33 + 0.6
//   P+560..880 (stride 16, at i-12)  SWORD/RIDE attacks v33 + 0.25
//   P+900,916 SWORD1/2 magic         v33 + 0.3
//   P+932   SWORD3                   v33 + 0.27
//   P+948   SWORD4                   v33 + 0.3
//   P+964,980 SWORD5/WHEEL           v33 + 0.24
//   P+996   FURY_STRIKE              0.38 (constant 0x3EC28F5C)
//   P+1012  VITALITY                 0.34 (constant 0x3EAE147B)
//   P+1028  RIDER                    v33 + 0.3
//   P+1044  RIDER_FLY                v33 + 0.3
//   P+1060  SPEAR                    v33 + 0.3
//   P+1076  ONE_TO_ONE               v33 + 0.3
//   P+736..784 (stride 16, at i-12)  BOW attacks  v35 = MagicSpeed*0.002
//   P+864..880 (stride 16)           RIDE BOW     v35
//   P+1300  TWO_HAND_SWORD_TWO       v33 + 0.25
//   P+1312..1360 (stride 16)         HAND/WEAPON  v34 + 0.29
//   P+1380  ELF1                     v38 + 0.25
//   P+1396  TELEPORT                 v34 + 0.3
//   P+1412  FLASH                    v34 + 0.4
//   P+1428  INFERNO                  v34 + 0.6
//   P+1444  HELL                     v34 + 0.5
//   P+1460  RIDE_SKILL               v34 + 0.3
extern DWORD CharacterAttribute_var;     // alias - already in our globals
void __cdecl FUN_00443e70(void) {
    DWORD ca = (DWORD)DAT_07cf1ff4;       // CharacterAttribute base
    if (ca == 0) return;
    DWORD models = DAT_05828d58;          // Models base
    if (models == 0) return;

    // Read stats (16-bit unsigned)
    int  attackSpeed = (int)(*(unsigned short*)(ca + 0x38));
    int  magicSpeed  = (int)(*(unsigned short*)(ca + 0x44));

    float v34 = (float)attackSpeed * _DAT_005524bc;  // = AttackSpeed * 0.004
    float v39 = (float)magicSpeed  * _DAT_005524bc;  // = MagicSpeed  * 0.004 (unused below)
    (void)v39;
    float v35 = (float)magicSpeed  * _DAT_005528e0;  // = MagicSpeed  * 0.002

    // v33 / v38 — uninitialized in IDA decompile. Default to 0 so base
    // animation speeds (0.6, 0.25, etc.) are applied without stat scaling.
    float v33 = 0.0f;
    float v38 = 0.0f;

    // Models[Player(390)] base animation table pointer at offset 0x11E18
    // (= 188*390 + 48 = entry +0x30 = animation array ptr).
    int* tablePtrPtr = (int*)((char*)(uintptr_t)models + 73368);  // 0x11E18
    if (*tablePtrPtr == 0) return;
    char* P = (char*)(uintptr_t)*tablePtrPtr;

    // FIST
    *(float*)(P + 548) = v33 + 0.6f;

    // SWORD/RIDE attacks: P+i-12 for i ∈ {560,576,592,...,880}
    float swordSpeed = v33 + 0.25f;
    for (int i = 560; i <= 880; i += 16) {
        *(float*)(P + i - 12) = swordSpeed;
    }

    // Magic sword skills
    float skillSpeed3 = v33 + 0.3f;
    *(float*)(P + 900)  = skillSpeed3;   // SKILL_SWORD1
    *(float*)(P + 916)  = skillSpeed3;   // SKILL_SWORD2
    *(float*)(P + 932)  = v33 + 0.27f;   // SKILL_SWORD3
    *(float*)(P + 948)  = skillSpeed3;   // SKILL_SWORD4
    float skillSpeed5 = v33 + 0.24f;
    *(float*)(P + 964)  = skillSpeed5;   // SKILL_SWORD5
    *(float*)(P + 980)  = skillSpeed5;   // SKILL_WHEEL
    *(float*)(P + 1076) = skillSpeed3;   // ATTACK_ONETOONE
    *(DWORD*)(P + 996)  = 0x3EC28F5C;    // FURY_STRIKE = 0.38
    *(DWORD*)(P + 1012) = 0x3EAE147B;    // VITALITY    = 0.34
    *(float*)(P + 1060) = skillSpeed3;   // SKILL_SPEAR
    *(float*)(P + 1028) = skillSpeed3;   // SKILL_RIDER
    *(float*)(P + 1044) = skillSpeed3;   // SKILL_RIDER_FLY

    // Two-hand sword2
    *(float*)(P + 1300) = swordSpeed;

    // BOW attacks: P+i-12 for i ∈ {752,768,784} (loop starts at 736+16)
    int v28 = 736;
    do {
        v28 += 16;
        *(float*)(P + v28 - 12) = v35;
    } while (v28 <= 784);

    // RIDE BOW: P+j-12 for j ∈ {864, 880}
    for (int j = 864; j <= 880; j += 16) {
        *(float*)(P + j - 12) = v35;
    }

    // Skills
    *(float*)(P + 1380) = v38 + 0.25f;        // ELF1
    float magicSkill = v34 + 0.29f;
    for (int k = 1312; k <= 1360; k += 16) {
        *(float*)(P + k - 12) = magicSkill;   // HAND1..WEAPON2
    }

    float teleportSpeed = v34 + 0.3f;
    *(float*)(P + 1396) = teleportSpeed;       // TELEPORT
    *(float*)(P + 1412) = v34 + 0.4f;          // FLASH
    *(float*)(P + 1428) = v34 + 0.6f;          // INFERNO
    *(float*)(P + 1444) = v34 + 0.5f;          // HELL
    *(float*)(P + 1460) = teleportSpeed;       // RIDE_SKILL
}

// FUN_004f9e90 @ 0x004F9E90 — EulerToMatrix(angles[3], out_mat[12])
// Converts Euler angles (in game units × π/180) to 3×4 rotation matrix.
// Row-major: out[0..2]=X-row, out[4..6]=Y-row, out[8..10]=Z-row; out[3,7,11]=0.
void __cdecl FUN_004f9e90(float *param_1, float *param_2) {
    float sz = sinf(param_1[2] * _DAT_00552ce8);
    float cz = cosf(param_1[2] * _DAT_00552ce8);
    float sy = sinf(param_1[1] * _DAT_00552ce8);
    float cy = cosf(param_1[1] * _DAT_00552ce8);
    float sx = sinf(param_1[0] * _DAT_00552ce8);
    float cx = cosf(param_1[0] * _DAT_00552ce8);
    param_2[3]  = 0.0f;
    param_2[7]  = 0.0f;
    param_2[11] = 0.0f;
    param_2[0]  = cy * cz;
    param_2[1]  = cy * sz;
    param_2[2]  = -sy;
    param_2[4]  = sx * sy * cz - cx * sz;
    param_2[5]  = cx * cz    + sx * sy * sz;
    param_2[6]  = sx * cy;
    param_2[8]  = sx * sz    + cx * sy * cz;
    param_2[9]  = cx * sy * sz - sx * cz;
    param_2[10] = cx * cy;
}

// FUN_004f9f70 @ 0x004F9F70 — Bone_CombineMatrices(parent[12], rot[12], out[12])
// 3×4 matrix multiply: out = parent × rot.
void __cdecl FUN_004f9f70(float *p, float *r, float *o) {
    o[0]  = p[2]*r[8]  + p[1]*r[4]  + p[0]*r[0];
    o[1]  = p[2]*r[9]  + p[0]*r[1]  + p[1]*r[5];
    o[2]  = p[2]*r[10] + p[0]*r[2]  + p[1]*r[6];
    o[3]  = p[2]*r[11] + p[0]*r[3]  + p[1]*r[7]  + p[3];
    o[4]  = p[4]*r[0]  + p[5]*r[4]  + p[6]*r[8];
    o[5]  = p[4]*r[1]  + p[5]*r[5]  + p[6]*r[9];
    o[6]  = p[4]*r[2]  + p[5]*r[6]  + p[6]*r[10];
    o[7]  = p[4]*r[3]  + p[5]*r[7]  + p[6]*r[11] + p[7];
    o[8]  = r[0]*p[8]  + p[9]*r[4]  + p[10]*r[8];
    o[9]  = r[1]*p[8]  + p[9]*r[5]  + p[10]*r[9];
    o[10] = r[2]*p[8]  + p[9]*r[6]  + p[10]*r[10];
    o[11] = r[3]*p[8]  + p[9]*r[7]  + p[10]*r[11] + p[11];
}

// FUN_004fa1d0 @ 0x004FA1D0 — EulerToQuat(angles[3], out_quat[4])
// Converts Euler XYZ (game angle units) to quaternion (x,y,z,w).
// Declared as (int,int,int,int) in functions.h; callers pass float* cast to int.
void __cdecl FUN_004fa1d0(int ia, int ib, int ic, int id) {
    float *param_1 = (float*)ia;
    float *param_2 = (float*)ib;
    (void)ic; (void)id;
    float k   = _DAT_00552ce0;
    float sz  = sinf(param_1[2] * k), cz = cosf(param_1[2] * k);
    float sy  = sinf(param_1[1] * k), cy = cosf(param_1[1] * k);
    float sx  = sinf(param_1[0] * k), cx = cosf(param_1[0] * k);
    float sxcy = sx * cy, cxsy = cx * sy;
    param_2[0] = (float)((double)sxcy * cz - (double)cxsy * sz);
    param_2[1] = (float)((double)sxcy * sz + (double)cxsy * cz);
    float sxsy = sx * sy;
    param_2[2] = (float)(cx * cy * sz - sxsy * cz);
    param_2[3] = (float)(cx * cy * cz + sxsy * sz);
}

// FUN_004fa270 @ 0x004FA270 — QuatToMatrix(quat[4], out_mat[12])
// Converts unit quaternion to 3×3 rotation matrix (stored in [0,1,2,4,5,6,8,9,10]).
void __cdecl FUN_004fa270(int ia, int ib, int ic, int id) {
    float *q = (float*)ia;
    float *m = (float*)ib;
    (void)ic; (void)id;
    float x2 = q[1]*q[1], z2 = q[2]*q[2];
    m[0]  = _DAT_00552cf0 - (x2+x2) - (z2+z2);
    float t = q[0]*q[1] + q[2]*q[3]; m[4]  = t+t;
    m[8]  = (q[2]*q[0]+q[2]*q[0]) - (q[3]*q[1]+q[3]*q[1]);
    m[1]  = (q[0]*q[1]+q[0]*q[1]) - (q[2]*q[3]+q[2]*q[3]);
    float y2 = q[0]*q[0];           z2 = q[2]*q[2];
    m[5]  = _DAT_00552cf0 - (y2+y2) - (z2+z2);
    t = q[0]*q[3] + q[2]*q[1];      m[9]  = t+t;
    t = q[3]*q[1] + q[2]*q[0];      m[2]  = t+t;
    m[6]  = (q[2]*q[1]+q[2]*q[1]) - (q[0]*q[3]+q[0]*q[3]);
    y2 = q[0]*q[0]; x2 = q[1]*q[1];
    m[10] = _DAT_00552cf0 - (y2+y2) - (x2+x2);
}

// FUN_004fa350 @ 0x004FA350 — QuatSlerp(q1[4], q2[4], t, out[4])
// Spherical linear interpolation between two quaternions.
void __cdecl FUN_004fa350(int ia, int ib, int ic, int id) {
    float *q1  = (float*)ia;
    float *q2  = (float*)ib;
    float  t   = *(float*)&ic;
    float *out = (float*)id;
    // ensure shortest path
    float dot = q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
    if (dot < 0.0f) { for(int i=0;i<4;i++) q2[i]=-q2[i]; dot=-dot; }
    float scale0, scale1;
    if (dot < 1.0f - _DAT_00552d00) {
        float angle = acosf(dot);
        float sinA  = sinf(angle);
        scale0 = sinf((1.0f - t) * angle) / sinA;
        scale1 = sinf(t * angle) / sinA;
    } else {
        scale0 = 1.0f - t;
        scale1 = t;
    }
    for (int i=0;i<4;i++) out[i] = scale0*q1[i] + scale1*q2[i];
}


// FUN_004ff580 @ 0x004FF580 — Entity_InitRenderState(entity)
// Scans render-state pool at DAT_083a2370 (stride 0xc, 128 slots).
// Finds first free slot (byte[0]==0), marks it active and stores entity ptr.
// FUN_004ff580 (IDA-activated, was Ghidra stub)
void *__cdecl FUN_004ff580(void *a1)
{
  DWORD *result; // eax

  result = (DWORD *)&DAT_083a2370;
  while ( *(BYTE *)result )
  {
    result += 3;
    if ( (int)result >= (int)&DAT_083a2cd0 )
    {
      return result;
    }
  }
  *(BYTE *)result = 1;
  result[2] = (DWORD)a1;
  return result;
}

// FUN_005129f0 @ 0x005129F0 — fabs(float) → double; was lying stub returning v unchanged.
long double   __cdecl FUN_005129f0(float v)                                  { return (long double)(v >= 0.0f ? v : -v); }
// FUN_0043e570 @ 0x0043E570 — Vector_AddRotated(pos, angle_ptr, offset_ptr)
// Builds rotation matrix from angle_ptr, rotates offset_ptr through it,
// then adds the result to pos[0..2].
void __cdecl FUN_0043e570(float *param_1, float *param_2, float *param_3) {
    float out[3], mat[12];
    FUN_004f9db0(param_2, mat);
    FUN_004fa0b0(param_3, mat, out);
    param_1[0] += out[0];
    param_1[1] += out[1];
    param_1[2] += out[2];
}

// FUN_00475170 @ 0x00475170 — ItemDrop_SetupRenderRef(slot_ptr)
// Resolves the entity reference at slot+0x3c, copies its world position to the
// model render slot, selects the target bone via equip-flags, then calls
// FUN_004409a0 to animate/position it.
void __cdecl FUN_00475170(int param_1) {
    int iVar1 = *(int*)(param_1 + 0x3c);
    float local_c[3] = {0.0f, 0.0f, 0.0f};
    void *this_ = (void*)(DAT_05828d58 + *(short*)(iVar1 + 2) * 0xbc);
    if (*(int*)(param_1 + 4) == 0x4d0)
        local_c[1] = -120.0f;
    *(unsigned int*)((int)this_ + 0x6c) = *(unsigned int*)(iVar1 + 0x10);
    *(unsigned int*)((int)this_ + 0x70) = *(unsigned int*)(iVar1 + 0x14);
    *(unsigned int*)((int)this_ + 0x74) = *(unsigned int*)(iVar1 + 0x18);
    uint uVar2 = *(uint*)(param_1 + 8) & 0x80000001;
    if ((int)uVar2 < 0) uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
    FUN_004409a0(this_,
        (float*)((unsigned int)(*(byte*)(DAT_07abf5d8 + 0x274 + uVar2 * 0x18)) * 0x30
                 + *(int*)(iVar1 + 0x114)),
        local_c,
        (float*)(param_1 + 0x10),
        '\x01');
}
// FUN_00474f90 @ 0x00474F90 — Player_DrawInstance
// Renders a billboard quad at pos[], scaled by sc, rotated around Z by rot[0] angle,
// using texture slot param_1.
void __cdecl FUN_00474f90(int cls, float *pos, float *rot, float sc) {
    FUN_00511480(cls);
    FUN_00511710();
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);
    float angle = rot ? rot[0] : 0.0f;
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    // Quad: half-size sc, CCW, full UV
    float q[4][3] = {
        {-sc, -sc, 0.0f},
        {-sc,  sc, 0.0f},
        { sc,  sc, 0.0f},
        { sc, -sc, 0.0f}
    };
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3fv(q[0]);
    glTexCoord2f(0.0f, 0.0f); glVertex3fv(q[1]);
    glTexCoord2f(1.0f, 0.0f); glVertex3fv(q[2]);
    glTexCoord2f(1.0f, 1.0f); glVertex3fv(q[3]);
    glEnd();
    glPopMatrix();
    FUN_00511600();
}
// FUN_00440aa0 (BMD::PlayAnimation / BMD_AnimTick) — moved to src/Render/BMD_Anim.cpp
// CharacterAnimation @ 0x00448600       — moved to src/Render/BMD_Anim.cpp
// (B3 refactor 2026-05-07)

// ── Weapon/Entity color helpers ───────────────────────────────────────────────

// FUN_00503cf0 @ 0x00503CF0 — Weapon_SetColor: maps item type to RGB color into color[3].
// color[3] *= scale * half_scale (scaled product), direction varies by item type index.
// flag=1 or flag=8 overrides selection for type 0x129/0x1f9.
void __cdecl FUN_00503cf0(int param_1, float param_2, float param_3, float *param_4, char param_5)
{
    unsigned int uVar4 = 0;
    // flag overrides
    if (param_5 != '\0' && (param_1 == 0x129 || param_1 == 0x1f9)) { uVar4 = 8; goto apply; }
    if (param_1 == 0x129 || param_1 == 0x1f9) { uVar4 = 1; goto apply; }
    if (param_1 == 0x131 || param_1 == 0x215 || param_1 == 0x21d) { uVar4 = 5; goto apply; }
    if (param_1 == 0x19e || param_1 == 0x235 || (param_1 >= 0x369 && param_1 <= 0x36a)) goto fallthrough;
    if (param_1 == 0x221) { uVar4 = 9; goto apply; }
    if (param_1 == 0x222) { uVar4 = 10; goto apply; }
    if (param_1 == 0x239) { uVar4 = 5; goto apply; }
    if (param_1 == 0x1af) { uVar4 = 10; goto apply; }
    if (param_1 == 0x260) { uVar4 = 6; goto apply; }
    if (param_1 == 0x1fa) { uVar4 = 9; goto apply; }
    if (param_1 == 0x31e) { uVar4 = 2; goto apply; }
    // 5.2 / original monk inventory armors:
    //   MODEL_ARMORINVEN_60 -> color 16
    //   MODEL_ARMORINVEN_61 -> color 42
    //   MODEL_ARMORINVEN_62 -> color 18
    {   // range dispatch: (param_1 - 400) / 32 → slot
        unsigned int uVar5 = (unsigned int)(param_1 - 400);
        int iVar3 = (int)(uVar5 + ((int)uVar5 >> 31 & 0x1fu)) >> 5;
        if (iVar3 >= 7 && iVar3 <= 0xb) {
            unsigned int u = uVar5 & 0x8000001fu;
            if ((int)u < 0) u = (u - 1 | 0xffffffe0u) + 1;
            switch (u) {
            case 1:  uVar4 = 1; break;
            case 3:  uVar4 = 3; break;
            case 4: case 0xe: case 0x12: uVar4 = 5; break;
            case 6:  uVar4 = 6; break;
            case 9: case 0xc: uVar4 = 2; break;
            case 0xd: uVar4 = 4; break;
            case 0xf: uVar4 = 7; break;
            case 0x10: uVar4 = 10; break;
            case 0x11: case 0x13: case 0x14: uVar4 = 9; break;
            default: break;
            }
        }
    }
fallthrough:;
apply:;
    float fVar1 = param_2 * param_3;
    switch (uVar4) {
    case 0:  param_4[2]=0.0f; param_4[0]=fVar1; param_4[1]=fVar1*_DAT_00552504; return;
    case 1:  param_4[2]=0.0f; param_4[0]=fVar1; param_4[1]=fVar1*_DAT_005526e4; return;
    case 2: case 3: { float h=fVar1*_DAT_00552504; param_4[0]=0.0f; param_4[1]=h; param_4[2]=fVar1; return; }
    case 4:  { float h=fVar1*_DAT_00552530; param_4[0]=0.0f; param_4[1]=h; param_4[2]=fVar1*_DAT_005528b4; return; }
    case 5:  param_4[0]=fVar1; param_4[1]=fVar1; param_4[2]=fVar1; return;
    case 6:  param_4[0]=fVar1*_DAT_00552534; param_4[1]=fVar1*_DAT_00552530; param_4[2]=fVar1*_DAT_005528b4; return;
    case 7:  param_4[0]=fVar1*_DAT_005526e8; param_4[1]=fVar1*_DAT_00552530; param_4[2]=fVar1; return;
    case 8:  { float h=fVar1*_DAT_00552530; param_4[0]=h; param_4[1]=h; param_4[2]=fVar1; return; }
    case 9:  { float h=fVar1*_DAT_00552504; param_4[0]=h; param_4[1]=h; param_4[2]=fVar1*_DAT_00552530; return; }
    case 10: param_4[0]=fVar1*_DAT_00552adc; param_4[1]=fVar1*_DAT_00552948; param_4[2]=fVar1*_DAT_00552504; return;
    case 0xb: { float h=fVar1*_DAT_005526e4; param_4[0]=fVar1; param_4[1]=h; param_4[2]=h; return; }
    default: return;
    }
}

// FUN_00503fe0 @ 0x00503FE0 — Weapon_SetColorAlt: simpler color selector, no flag param.
// Scales existing color[3] in place by (scale * half_scale).
void __cdecl FUN_00503fe0(int param_1, float param_2, float param_3, float *param_4)
{
    int iVar4 = 0;
    if ((param_1==0x215)||(param_1==0x21d)||(param_1==0x19e)||(param_1==0x235)) { iVar4=2; }
    else if (param_1==0x1a2) { iVar4=0; }
    else if (param_1==0x221||param_1==0x239) { iVar4=0; }
    else if (param_1==0x1a4) { iVar4=1; }
    else {
        unsigned int uVar3 = (unsigned int)(param_1 - 400);
        int iVar2 = (int)(uVar3 + ((int)uVar3 >> 31 & 0x1fu)) >> 5;
        if (iVar2 >= 7 && iVar2 <= 0xb) {
            unsigned int u = uVar3 & 0x8000001fu;
            if ((int)u < 0) u = (u - 1 | 0xffffffe0u) + 1;
            switch (u) {
            case 0x12: iVar4=2; break;
            case 4: case 0xe: case 0xf: case 0x11: iVar4=0; break; // (break → iVar4=1 after)
            default: iVar4=0; break;
            }
        }
    }
    float fVar1 = param_2 * param_3;
    if (iVar4 == 0) {
        param_4[0] = fVar1 * param_4[0];
        param_4[1] = fVar1 * param_4[1];
        param_4[2] = fVar1 * param_4[2];
    } else if (iVar4 == 1) {
        param_4[2] = 0.0f;
        param_4[0] = fVar1 * param_4[0];
        param_4[1] = fVar1 * param_4[1] * _DAT_00552504;
    } else if (iVar4 == 2) {
        param_4[0] = 0.0f;
        param_4[1] = fVar1 * param_4[1] * _DAT_00552504;
        param_4[2] = fVar1 * param_4[2];
    }
}

// FUN_00504960 @ 0x00504960 — Entity_SetModelColor: weapon type → color/alpha → render.
// Sets model color at +0x48..+0x50. Special cases for type 0x144 (two-tone), 0x1d7
// (sets entity +0x58=2 then resets to -1 afterward), 0x235 (FUN_00441e00 with extra arg).
// Falls through to FUN_00441e00 for bone rendering.
void* __cdecl FUN_00504960(void *model, int entity, int etype, float scale,
                            int flags, float alpha, int rgba)
{
    static const int kHiddenMeshAllBits = -1;
    static const float kHiddenMeshAll = *(const float*)&kHiddenMeshAllBits;
    float *color = (float *)((char*)model + 0x48);
    if ((flags & 0x10) == 0x10) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    } else if (etype == 0x144) {
        if ((flags & 0x100) == 0) {
            color[0] = 0.2f; color[1] = 0.2f; color[2] = 0.8f;
        } else {
            flags -= 0x100;
            color[0] = 1.0f; color[1] = 0.1f; color[2] = 0.1f;
        }
        FUN_00441e00(model, (uint)flags, scale,
                     *(float*)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), kHiddenMeshAll, (uint)rgba);
        return nullptr;
    } else {
        FUN_00503cf0(etype, scale, alpha, color, (char)((flags >> 8) & 1));
    }
    if (etype == 0x1d7) {
        *(int *)(entity + 0x58) = 2;
    } else if (etype == 0x235) {
        FUN_00441e00(model, (uint)flags, scale,
                     *(float*)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), 1.4013e-45f, (uint)rgba);
        return nullptr;
    } else if (etype != 0x1af && etype != 0x1fa && etype != 0x260) {
        FUN_00441e00(model, (uint)flags, scale,
                     *(float*)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), kHiddenMeshAll, (uint)rgba);
        return nullptr;
    }
    float fVar1 = *(float*)(entity + 0x58);
    FUN_00441e00(model, (uint)flags, scale,
                 *(float*)(entity+100), *(float*)(entity+0x68),
                 *(float*)(entity+0x6c), *(float*)(entity+0x70), fVar1, (uint)rgba);
    if (etype == 0x1d7) {
        *(int *)(entity + 0x58) = -1;
    }
    return nullptr;
}

// FUN_00504ac0 @ 0x00504AC0 — Entity_SetModelColorAlt: simpler version.
// No special type 0x144 path; uses FUN_00503fe0 instead of FUN_00503cf0.
void* __cdecl FUN_00504ac0(void *model, int entity, int etype, float scale,
                             int flags, float alpha, int rgba)
{
    static const int kHiddenMeshAllBits = -1;
    static const float kHiddenMeshAll = *(const float*)&kHiddenMeshAllBits;
    float *color = (float *)((char*)model + 0x48);
    if ((flags & 0x10) == 0x10) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    } else {
        FUN_00503fe0(etype, scale, alpha, color);
    }
    float fVar1 = (etype == 0x235) ? 1.4013e-45f : kHiddenMeshAll;
    FUN_00441e00(model, (uint)flags, scale,
                 *(float*)(entity+100), *(float*)(entity+0x68),
                 *(float*)(entity+0x6c), *(float*)(entity+0x70), fVar1, (uint)rgba);
    return nullptr;
}
// FUN_00455430 @ 0x00455430 — RenderLinkObject (COMPLETO)
// Implemented in src/Render/RenderLinkObject.cpp
// FUN_00449840 @ 0x00449840 — Entity_ClearBoneLinks(param1, param2, param3)
// Clears bone/widget link arrays on entity objects.
// For param2: iterates (+0x184, count at +0x180), calls FUN_004086e0 + vtable[0](3) per entry.
// For param1: iterates 6 weapon/equip slots (stride 0x18 at +0x1f4), calls FUN_004086e0 + vtable[0](1).
// For param3: clears one link at +0x14 via FUN_004086e0 + vtable[0](1).
// FUN_004086e0 signature: (int, int, int) — called here as (ptr, 0, 0) (3-arg form, per functions.h).
void __cdecl FUN_00449840(int param_1, int param_2, int param_3)
{
    if ((param_2 != 0) && (*(int*)(param_2 + 0x184) != 0)) {
        int count = (int)(unsigned char)*(char*)(param_2 + 0x180);
        int *puVar1 = (int*)*(int*)(param_2 + 0x184);
        for (int i = 0; i < count; i++) {
            FUN_004086e0((int)puVar1, 0, 0);
            puVar1 += 0x15;
        }
        int *base = (int*)*(int*)(param_2 + 0x184);
        if (base != nullptr) {
            FUN_0045aaa0_impl(base, 3);   // vtable[0] = sub_45AAA0
        }
        *(int*)(param_2 + 0x184) = 0;
        *(char*)(param_2 + 0x180) = '\0';
    }
    if (param_1 != 0) {
        int *piVar4 = (int*)(param_1 + 0x1f4);
        for (int i = 0; i < 6; i++) {
            int *puVar1 = (int*)*piVar4;
            if (puVar1 != nullptr) {
                FUN_004086e0((int)puVar1, 0, 0);
                (**(void (__cdecl**)(int))*puVar1)(1);
                *piVar4 = 0;
            }
            piVar4 += 6;
        }
    }
    if ((param_3 != 0) && (*(int*)(param_3 + 0x14) != 0)) {
        int *puVar1 = (int*)*(int*)(param_3 + 0x14);
        FUN_004086e0((int)puVar1, 0, 0);
        (**(void (__cdecl**)(int))*puVar1)(1);
        *(int*)(param_3 + 0x14) = 0;
    }
}
// FUN_004f8bb0 @ 0x004F8BB0 — Particle_DrawBillboard: draws a tiled billboard quad in world space.
// Loads texture (param_1), sets GL color, computes tile grid from scale/position,
// transforms each tile corner via FUN_004fa0b0 (bone matrix), calls FUN_004f8740 per tile.
// Uses __ftol for int grid coords from float positions.
void* __cdecl FUN_004f8bb0(int type, float x, float y, float sx, float sy, float *col, float angle, float alpha)
{
    if (alpha == _DAT_0055256c)
        glColor3fv(col);
    else
        glColor4f(col[0], col[1], col[2], alpha);

    float rot_data[3] = { 0.0f, 0.0f, angle };
    float rot_mat[12];
    FUN_004f9db0(rot_data, rot_mat);
    FUN_00511480(type);

    float cx = x * _DAT_00552594;
    float cy = y * _DAT_00552594;
    int icx = (int)cx;
    int icy = (int)cy;
    float size = (sx > sy) ? sx : sy;
    if (size <= 0.0f)
        return nullptr;
    float half = size * _DAT_00552504;
    float inv  = _DAT_0055256c / size;
    float xScale = (sy != 0.0f) ? (sx / sy) : 1.0f;
    int   itiles = (int)half + 1;
    float loopRadius = (float)itiles;
    float oneTile = _DAT_0055256c;

    for (float tj = -loopRadius; tj <= loopRadius; tj += oneTile) {
        float v0 = ((tj + (float)icy - cy) + half) * inv;
        float v1 = ((tj + oneTile + (float)icy - cy) + half) * inv;
        for (float ti = -loopRadius; ti <= loopRadius; ti += oneTile) {
            float corners[4][3];
            float u0 = ((ti + (float)icx - cx) + half) * inv;
            float u1 = ((ti + oneTile + (float)icx - cx) + half) * inv;
            float inp[4][3] = {
                {u0 - _DAT_00552504, v0 - _DAT_00552504, 0.0f},
                {u1 - _DAT_00552504, v0 - _DAT_00552504, 0.0f},
                {u1 - _DAT_00552504, v1 - _DAT_00552504, 0.0f},
                {u0 - _DAT_00552504, v1 - _DAT_00552504, 0.0f},
            };
            for (int k = 0; k < 4; k++) {
                float out[3];
                FUN_004fa0b0(inp[k], rot_mat, out);
                corners[k][0] = xScale * out[0] + _DAT_00552504;
                corners[k][1] = out[1] + _DAT_00552504;
                corners[k][2] = out[2];
            }
            FUN_004f8740((float)icx + ti, (float)icy + tj, 1.0f, 1,
                         (int)corners, '\0', alpha);
        }
    }
    return nullptr;
}

// Widget / skill-beam system
// FUN_004086e0 @ 0x004086E0 — Widget_Release(widget_ptr) [__fastcall]
// Releases a widget/beam linked-list container.  Clean decompile, 35 lines, __fastcall(int).
// Logic:
//   1. Walk forward-linked list from *(int**)(param1+0x4c)+8 to sentinel *(param1+0x50):
//      for each node: if node->vtable != 0, call vtable[0](1)  (decref child)
//   2. Clear back-pointer: *(*(param1+0x50)+4)+8 = 0
//   3. Free all list nodes (operator_delete each)
//   4. Relink sentinel: head->prev = param1+0x4c, tail->next = param1+0x50
//   5. *(param1+0x48) = 0  (count = 0)
//   6. operator_delete(*(param1+0x3c))  (free internal buffer)
//   7. if *(param1+0x34) != 0: call vtable[0](3) on it  (parent release)
//   8. FUN_004080f0(param1)  (base widget destructor — not declared)
// FUN_004080f0 @ 0x004080F0 — Widget_NodeInit
// Zeros all node fields and sets the four scale floats to 1.0f.
void __cdecl FUN_004080f0(int param_1)
{
    *(int *)(param_1 + 0x04) = 0; *(int *)(param_1 + 0x08) = 0;
    *(int *)(param_1 + 0x0c) = 0; *(int *)(param_1 + 0x10) = 0;
    *(int *)(param_1 + 0x14) = 0; *(int *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0x1c) = 0;
    *(unsigned int *)(param_1 + 0x20) = 0x3f800000u; // 1.0f
    *(unsigned int *)(param_1 + 0x24) = 0x3f800000u; // 1.0f
    *(int *)(param_1 + 0x28) = 0; *(int *)(param_1 + 0x2c) = 0;
    *(int *)(param_1 + 0x30) = 0; *(int *)(param_1 + 0x34) = 0;
    *(int *)(param_1 + 0x38) = 0; *(int *)(param_1 + 0x3c) = 0;
    *(unsigned int *)(param_1 + 0x40) = 0x3f800000u; // 1.0f
    *(unsigned int *)(param_1 + 0x44) = 0x3f800000u; // 1.0f
}

// FUN_004086e0 @ 0x004086E0 — Widget_Release(widget, 0, 0)
// Iterates linked list at (*(widget+0x4c)+8), calls vtable[0](1) on each node,
// frees list nodes, relinks head/tail sentinels, zeroes count, frees buffer at
// widget+0x3c, calls vtable[0](3) on widget+0x34, then calls Widget_BaseRelease.
void __cdecl FUN_004086e0(int param_1, int, int) {
    int *piVar1 = *(int **)(*(int *)(param_1 + 0x4c) + 8);
    if (piVar1 != *(int **)(param_1 + 0x50)) {
        do {
            if (piVar1 == nullptr) break;
            if (*piVar1 != 0) {
                // IDA `sub_4086E0` L11: `(**vtable)(obj, 1)`. Los objetos de
                // esta lista son ANCLAS (0x24 B, vtable off_552514), no nodos
                // de la grilla (0x3C B, vtable 0x5524E8): llamaba al dtor
                // equivocado. Con la vtable del ancla ya cableada
                // (g_ClothAnchorVTable), se despacha por ella como el original.
                void **vt = *(void ***)(*piVar1);
                if (vt) ((void (__cdecl *)(void *, char))vt[0])((void *)*piVar1, 1);
            }
            piVar1 = (int *)piVar1[2];
        } while (*(int **)(param_1 + 0x50) != piVar1);
    }
    *(DWORD *)(*(int *)(*(int *)(param_1 + 0x50) + 4) + 8) = 0;
    BYTE *puVar2 = *(BYTE **)(*(int *)(param_1 + 0x4c) + 8);
    while (BYTE *puVar3 = puVar2) {
        puVar2 = *(BYTE **)(puVar3 + 8);
        operator_delete(puVar3);
    }
    *(DWORD *)(*(int *)(param_1 + 0x4c) + 8) = *(DWORD *)(param_1 + 0x50);
    *(DWORD *)(*(int *)(param_1 + 0x50) + 4) = *(DWORD *)(param_1 + 0x4c);
    *(DWORD *)(param_1 + 0x48) = 0;
    operator_delete(*(BYTE **)(param_1 + 0x3c));
    if (*(DWORD *)(param_1 + 0x34) != 0) {
        // array de nodos: dtor de array (flags 3 = vector + free)
        FUN_00408680((void *)*(int *)(param_1 + 0x34), 3);
    }
    FUN_004080f0(param_1);
}

// FUN_00407fe0 @ 0x00407FE0 — Widget_CtorBase: allocate and cross-link two doubly-linked-list
// sentinel nodes, zero the count field, set vtable, then zero all widget node fields.
// List layout at param_1: [+0x4c]=head_sentinel*, [+0x50]=tail_sentinel*, [+0x48]=count.
// Sentinel node (0xc bytes): [+0]=unused, [+4]=prev, [+8]=next.
// Initially: head->+8 = tail; tail->+4 = head (empty list).
// Vtable de la tela (off_552520, leída del binario original):
//     { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
// El slot 0 (dtor) se llama directo en este build. Los slots 1 y 2 los invoca
// `sub_408CB0` en cada tick de simulación: 1 = re-fijar la fila superior al
// hueso (Cloth_PinTopRow), 2 = repartir viento/gravedad (Cloth_Wind).
void __cdecl FUN_00408780(int _this, float (*Matrix)[4]);
void __cdecl FUN_004089b0(DWORD *_this);
static void __cdecl Cloth_VSlot0(int a)  { (void)a; }  // dtor: se llama directo
static void *g_ClothVTable[4] = {
    (void *)Cloth_VSlot0,  (void *)FUN_00408780,
    (void *)FUN_004089b0,  (void *)FUN_00408ff0
};

void* __fastcall FUN_00407fe0(void *param_1)
{
    // head sentinel
    int *head = (int *)operator_new(0xc);
    if (head) { head[1] = 0; head[2] = 0; }
    ((int **)param_1)[0x13] = head;   // *(param_1 + 0x4c) = head

    // tail sentinel
    int *tail = (int *)operator_new(0xc);
    if (tail) { tail[1] = 0; tail[2] = 0; }
    ((int **)param_1)[0x14] = tail;   // *(param_1 + 0x50) = tail

    // cross-link: head->+8 = tail; tail->+4 = head
    if (head) head[2] = (int)tail;
    if (tail) tail[1] = (int)head;

    ((int *)param_1)[0x12] = 0;       // count at +0x48 = 0

    // 2026-08-11 — vtable. IDA hace `*(_DWORD *)this = &off_552520;` y varias
    // rutinas la usan por indirección; con el campo sin inicializar se ejecuta
    // basura (crash 0xC0000005 param0=8 al entrar al mundo, desde
    // `FUN_00449840`/DeleteCloth que llama vtable[0](3)).
    //
    // Leída del binario original (`Cliente armado/main.exe`, MD5 eb95ac…):
    //     off_552520 = { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
    // De esas cuatro sólo `sub_408FF0` está portada (y es un stub vacío); las
    // otras tres no existen en este build. Se instala una vtable bien formada
    // con no-ops para que el objeto sea válido y las indirecciones no salten a
    // basura. TODO: portar 0x0045AAA0 (dtor), 0x00408780, 0x004089B0 y el
    // cuerpo real de 0x00408FF0 (render de la tela).
    *(const void **)param_1 = (const void *)g_ClothVTable;

    FUN_004080f0((int)param_1);
    return param_1;
}

// FUN_00541ec1 @ 0x00541EC1 — Array_Construct(arr, elem_size, count, ctor): calls __fastcall ctor(elem) for each element.
void __cdecl FUN_00541ec1(void *arr, int elem_size, int count, void *ctor) {
    typedef void* (__fastcall *CtorFn)(void*);
    CtorFn fn = (CtorFn)ctor;
    char *p = (char*)arr;
    for (int i = 0; i < count; i++, p += elem_size)
        fn(p);
}
// Forward declarations — physics helpers defined in VerletNode section below
void  __fastcall FUN_00407950(void *node);   // ctor (thiscall en el original: arg en ECX)  // IDA-port: void return
void  __fastcall FUN_004079b0(void *node, float x, float y, float z, int pinned);
void  __fastcall FUN_00407b30(void *node, float *out);
float __fastcall FUN_00407b50(void *nodeA, int nodeB_ptr, float *out_delta);
void  __fastcall FUN_004088b0(void *sys, int idx, short na, short nb, float rest_scaled, float dist, BYTE flags);
DWORD* __cdecl   FUN_00407e50(DWORD *node); // IDA-port: returns this
void  __fastcall FUN_00407ef0(void *node, float p1, float p2, float p3, float radius, int boneIdx);


// FUN_00408900 @ 0x00408900 — Widget_CheckState(widget, hash, flags)
// __thiscall in original (this=widget via ECX). Calls FUN_00408940 `flags` times,
// returns 0 if any fails, 1 if all pass. FUN_00408940 is a void stub → always return 1.
// Port FIEL de IDA `sub_408900` (Hex-Rays perdió el `this`, que viaja en ECX):
//     v2 = 0;
//     if (a2 <= 0) return 1;
//     while (sub_408940(a1)) { if (++v2 >= a2) return 1; }
//     return 0;
// `hash` son los BITS del dt (0x3ba3d70a = 0.005f) y `flags` el nº de
// iteraciones. 2026-08-11: era un stub que devolvía 1 SIN ejecutar la
// simulación, así que los nodos de la tela nunca se movían.
int __cdecl FUN_00408940(int *param_1, float dt);
int __cdecl FUN_00408900(int *widget, unsigned int hash, int flags) {
    if (!widget || flags <= 0) return 1;
    float dt; memcpy(&dt, &hash, 4);
    int it = 0;
    while (FUN_00408940(widget, dt)) {
        if (++it >= flags) return 1;
    }
    return 0;
}
// FUN_00408130 @ 0x00408130 — GridSpring_Create: builds cols×rows cloth spring system.
// Params: widget=system, entity=scale_factor, p3=cols(W), p4=uRange, p5=vRange,
//         p6=rows(H), p7=entity_ptr, p8=uOrigin, p9=vOrigin, ta=bone_idx, tb=bone_data_idx, flags=flag_mask.
// Layout mirrors FUN_004093e0: +4..+44 store params, +30=node_count, +34=node_array*, +38=spring_count, +3c=spring_array*.
// Nodes initialized from bone transform if entity+0x114 != 0, else from grid coords.
// Springs: vertical edges (col-to-col+W), horizontal (col-to-col+1), and diagonal shear springs.
void __cdecl FUN_00408130(void *widget, float entity, int p3, float p4, float p5,
                          int p6, int p7, float p8, float p9, int ta, int tb, int flags)
{
    char *thiz = (char*)widget;
    *(float *)(thiz + 0x04) = entity;
    *(int   *)(thiz + 0x10) = tb;
    // 2026-08-11 FIX: los tres campos de abajo estaban CORRIDOS un parámetro.
    // IDA `sub_408130` L78-89:
    //     this[6] (+0x18) = a4        ← nuestro port ponía p3
    //     this[7] (+0x1C) = a5        ← ponía p4
    //     this[8] (+0x20) = a8        ← ponía p5
    //     this[9] (+0x24) = a9        ← ok
    // El de +0x20 es el ANCHO de la grilla: con p5 (0.0 en la llamada de la
    // capa) todos los nodos quedaban en la misma columna → los 100 vértices
    // salían en (0,0,0) (medido con el probe CLOTHDBG).
    *(float *)(thiz + 0x18) = *(float*)&p4;   // a4
    *(float *)(thiz + 0x1c) = *(float*)&p5;   // a5
    // 2026-08-11 FIX: IDA `sub_408130` L80-90 guarda
    //     this[10] (+0x28) = a6   ← 6º param
    //     this[11] (+0x2c) = a7   ← 7º param  (nuestro port ponía p3, el 3º)
    //     this[12] (+0x30) = a7 * a6
    // Con p3 en +0x2c la grilla quedaba de 19 filas sobre 10x10 → el render
    // (sub_408FF0 / sub_4091D0, que leen +0x28 y +0x2c) se iba de rango.
    *(int   *)(thiz + 0x28) = p6;             // cols
    int node_count = p6 * p7;
    *(int   *)(thiz + 0x08) = p3;
    *(int   *)(thiz + 0x2c) = p7;             // rows
    *(int   *)(thiz + 0x0c) = ta;
    *(float *)(thiz + 0x20) = p8;             // a8 — ancho de la grilla
    *(unsigned int *)(thiz + 0x14) |= (unsigned int)flags;
    *(float *)(thiz + 0x24) = *(float*)&p9;   // vOrigin

    *(int *)(thiz + 0x30) = node_count;

    // allocate node array (same pattern as FUN_004093e0)
    int *raw = (int*)operator_new(node_count * 0x3c + 4);
    int *nodes;
    if (raw == NULL) {
        nodes = NULL;
    } else {
        nodes = raw + 1;
        *raw  = node_count;
        FUN_00541ec1(nodes, 0x3c, node_count, (void*)FUN_00407950);
    }
    *(int **)(thiz + 0x34) = nodes;

    // spring count: (rows-1)*cols + (cols-1)*rows horizontal+vertical connections (×2)
    int sc = (p3 - 1) * p6 + (p3 - 1) * p3;  // per Ghidra: (cols-1)*rows + (cols-1)*cols — approximate
    {
        int W = *(int *)(thiz + 0x28);   // rows stored at +28
        int H = *(int *)(thiz + 0x2c);   // cols stored at +2c
        sc = (H - 1) * W + (W - 1) * H;
    }
    *(int *)(thiz + 0x38) = sc * 2;

    void *springs = operator_new(sc * 0x20);
    *(void **)(thiz + 0x3c) = springs;

    // cell spacing
    int W = *(int *)(thiz + 0x28);   // rows
    int H = *(int *)(thiz + 0x2c);   // cols
    float cellU = *(float *)(thiz + 0x20) / (float)(W - 1);
    float cellV = *(float *)(thiz + 0x24) / (float)(H - 1);
    *(float *)(thiz + 0x40) = cellU;
    *(float *)(thiz + 0x44) = cellV;

    // compute bone rotation matrix from entity+0x1c
    // 2026-08-11 FIX (la capa salía con todos los vértices en (0,0,0)):
    // `FUN_004f9db0` es EulerToMatrix3x4 — escribe una matriz 3x4 = **12
    // floats**. Estaba declarado `float local_3c[3]` → 36 bytes de desborde de
    // stack justo encima de los locales siguientes. El probe INITDBG lo mostró
    // sin lugar a dudas: después del loop de init, `nodes` había pasado a NULL,
    // `W` a 0 y `H` a 1063105495 (= bits de 0.85f, o sea un elemento de la
    // matriz). Con `W`/`H` pisados el loop no escribía ningún nodo.
    // Mismo patrón que el `local_3c[14]` de Effect_Create (ver la nota de
    // "Locales que Ghidra separó" en CLAUDE.md): el callee escribe más de lo
    // que declara el local.
    float local_3c[12];
    FUN_004f9db0((float *)(*(int *)(thiz + 4) + 0x1c), local_3c);

    // init node positions
    int entity_ptr = *(int *)(thiz + 4);
    int has_bone = *(int *)(entity_ptr + 0x114);

    for (int row = 0; row < H; row++) {
        float scaleU = cellU;
        float uOrig  = *(float *)(thiz + 0x20);

        // flag-based UV scale (from Ghidra conditional at +0x14 bits 2-3)
        if ((*(unsigned int *)(thiz + 0x14) & 0xc) == 4) {
            float t = ((float)row * _DAT_00552538) / (float)(H - 1) + _DAT_00552534;
            uOrig  = t * *(float *)(thiz + 0x20);
            scaleU = uOrig / (float)(W - 1);
        }

        float vOffset = -((float)row * cellV);
        float halfU   = uOrig * _DAT_00552504;

        for (int col = 0; col < W; col++) {
            float lx = (float)col * scaleU - halfU;
            float ly = 20.0f;
            if ((*(unsigned int *)(thiz + 0x14) & 3) == 1) {
                float frac = fabsf((float)col / (float)(W - 1) - _DAT_00552504);
                frac += frac;
                ly = _DAT_005524fc - frac * frac * _DAT_00552488;
            }
            float lz = vOffset;

            float nx, ny, nz;
            if (has_bone == 0) {
                nx = lx; ny = ly; nz = lz;
            } else {
                float out_pos[3] = {lx, ly, lz};
                float out_col[4] = {0};
                FUN_004409a0((void *)(DAT_05828d58 + *(short *)(entity_ptr + 2) * 0xbc),
                             local_3c, out_pos, out_col, '\x01');
                // 2026-08-11 FIX: BMD__TransformPosition LEE del 3er arg (Pos)
                // y ESCRIBE en el 4º (WorldPos). El port leía de vuelta
                // `out_pos` — la ENTRADA sin transformar — y descartaba el
                // resultado, así que la malla quedaba en espacio LOCAL
                // (bbox -37.5..37.5 x 0..-120, medido con CLOTHDBG) en lugar de
                // en la posición del personaje.
                nx = out_col[0]; ny = out_col[1]; nz = out_col[2];
            }

            // IDA `v31 = v30 + i * v29` con v29 = this[10] = W (columnas).
            // 2026-08-11: era `H * row + col`. Coincide sólo cuando W == H
            // (la capa del MG es 10x10); los demás cloths quedaban barajados.
            int ni = W * row + col;
            FUN_004079b0((char*)nodes + ni * 0x3c, nx, ny, nz, 0);
        }
    }

    // ── INITDBG (temporal): ¿el loop de init escribió las posiciones?
    // Lee de vuelta el nodo 0 y el 50 en +0x1c, que es donde FUN_004079b0
    // escribe y FUN_00407b30 lee. Si acá salen no-cero pero CLOTHDBG ve (0,0,0),
    // el problema está en la lectura/puntero; si acá ya salen cero, en la
    // escritura.
    {
        char ib[240];
        if (nodes && node_count > 0) {
            const float *n0 = (const float *)((char *)nodes + 0x1c);
            int last = (node_count > 1) ? (node_count - 1) : 0;
            const float *nL = (const float *)((char *)nodes + (size_t)last * 0x3c + 0x1c);
            _snprintf_s(ib, sizeof(ib), _TRUNCATE,
                "INITDBG nodes=%p cnt=%d W=%d H=%d uRange=%.1f vRange=%.1f hasBone=%d "
                "n0=(%.1f,%.1f,%.1f) nLast=(%.1f,%.1f,%.1f)",
                (void *)nodes, node_count, W, H,
                *(float *)(thiz + 0x20), *(float *)(thiz + 0x24), has_bone,
                n0[0], n0[1], n0[2], nL[0], nL[1], nL[2]);
        } else {
            _snprintf_s(ib, sizeof(ib), _TRUNCATE,
                "INITDBG nodes=%p cnt=%d W=%d H=%d  (SIN ARRAY)",
                (void *)nodes, node_count, W, H);
        }
        DbgLogPublic(ib);
    }

    // build spring edges
    // IDA: `if ((this[5] & 0x300) != 256) { v58 = 4; v63 = 1; }`
    //   v58 -> springs VERTICALES (bit 4 = solver de rango, sub_407B90)
    //   v63 -> springs DIAGONALES (bit 1 = solver de igualdad, sub_407C60)
    // 2026-08-11: las diagonales recibían `vert_flag` (4) en vez de v63 (1),
    // o sea entraban al solver de rango y nunca al de igualdad.
    BYTE vert_flag = (((*(unsigned int *)(thiz + 0x14) & 0x300) != 0x100)) ? 4 : 0;
    BYTE diag_flag = (((*(unsigned int *)(thiz + 0x14) & 0x300) != 0x100)) ? 1 : 0;
    int sp = 0;

    for (int row = 0; row < H; row++) {
        for (int col = 0; col < W; col++) {
            int ni = W * row + col;
            short sni = (short)ni;
            float delta[3];
            float dist;

            // vertical spring (row → row+1, same col)
            if (row < H - 1) {
                dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                   (int)nodes + (W + ni) * 0x3c, delta);
                FUN_004088b0(widget, sp, sni, sni + (short)W,
                             dist * _DAT_00552530, dist, (BYTE)(vert_flag | 2));
                sp++;
            }

            // horizontal spring (col → col+1, same row)
            if (col < W - 1) {
                dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                   (int)nodes + (ni + 1) * 0x3c, delta);
                FUN_004088b0(widget, sp, sni, sni + 1,
                             dist * _DAT_00552530, dist, 3);
                sp++;

                // diagonal shear spring down-right (to row+1, col+1)
                if (row < H - 1) {
                    dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                       (int)nodes + (ni + 1 + W) * 0x3c, delta);
                    FUN_004088b0(widget, sp, sni, sni + 1 + (short)W,
                                 dist * _DAT_00552530, dist, diag_flag);
                    sp++;
                }

                // diagonal shear spring up-right (to row-1, col+1)
                if (row > 1) {
                    dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                       (int)nodes + (ni - W + 1) * 0x3c, delta);
                    FUN_004088b0(widget, sp, sni, (short)(sni - (short)W + 1),
                                 dist * _DAT_00552530, dist, diag_flag);
                    sp++;
                }
            }
        }
    }
    *(int *)(thiz + 0x38) = sp;
    // vtable call: (*vtable[1])(local_3c) — skipped in re-impl
}

// Skills / teleport
// Entity_TeleportAnim @ 0x004792C0 — allocate a teleport-anim slot in DAT_07c80110 pool.
// Pool stride 0x70; slot layout: [0]=active, [4]=id, [0xc]=param4, [0x10..0x18]=src_pos+height,
// [0x1c..0x24]=dst_pos, [0x38..0x3b]=zeros, [0x48..0x4b]="\0\0 A".
// This overload takes dst as individual floats (a,b,c); FUN_0046c3e0 renders the trail.
void __cdecl Entity_TeleportAnim(float *pos, float a, float b, float c)
{
    return;  // AUTO-SKIP: dead duplicate (kept for reference). Active impl is the
             // 4-arg overload in stubs.cpp:3324 (Entity_TeleportAnim with float* dst_pos).
    char *slot = &DAT_07c80110[0];
    while ((int)slot < 0x7c82cd0) {
        if (*slot == '\0') {
            *slot = '\x01';
            *(float *)(slot + 0x10) = pos[0];
            *(float *)(slot + 0x14) = pos[1];
            *(float *)(slot + 0x18) = pos[2] + _DAT_00552958;
            *(float *)(slot + 0x1c) = a;
            *(float *)(slot + 0x20) = b;
            *(float *)(slot + 0x24) = c;
            slot[0x38] = '\0'; slot[0x39] = '\0'; slot[0x3a] = '\0'; slot[0x3b] = '\0';
            slot[0x48] = '\0'; slot[0x49] = '\0'; slot[0x4a] = ' ';  slot[0x4b] = 'A';
            return;
        }
        slot += 0x70;
    }
}

// Particle_Update @ 0x0046C3E0 — Trail_RenderAll: render weapon/beam trails in pool.
// Pool: g_RenderPool_07c608a8 (= shared joint/trail pool, 100 slots × 0x2f0).
// 2026-05-03: AUTO-SKIP removed. Pool now properly sized; iteration count
// is 100 (matching IDA bound `< 0x7c72e74` = base + 100*0x2f0).
void __cdecl Particle_Update(void)
{
    int *slot = (int *)&DAT_07c608b4;
    for (int slotIdx = 0; slotIdx < 100; ++slotIdx, slot = (int*)((int)slot + 0x2f0)) {
        if ((char)slot[-3] != '\0') {
            int mode = slot[-2];
            if (*(short *)(*slot + 0x1be) == 0 && mode < 3)
                FUN_00511710();
            else
                FUN_00511790();
            if (mode > 2) mode -= 3;
            int seg = slot[1];
            if (seg > 1) {
                FUN_00511480(mode + 0x48d);
                int *vp = slot + 0x5f;
                for (int i = 0; i < seg - 1; i++, vp += 3) {
                    glBegin(6);
                    float alpha = _DAT_0055256c;
                    if (*(short *)(*slot + 0x1be) == 0)
                        alpha = (float)(seg - i) / (float)seg;
                    glColor3f(alpha*(float)slot[2], alpha*(float)slot[3], alpha*(float)slot[4]);
                    glTexCoord2f((float)i / (float)seg, 1.0f);
                    glVertex3fv((float *)(vp - 0x5a));
                    glTexCoord2f((float)i / (float)seg, 0.0f);
                    glVertex3fv((float *)vp);
                    float alpha2 = _DAT_0055256c;
                    if (*(short *)(*slot + 0x1be) == 0)
                        alpha2 = (float)(seg - i - 1) / (float)seg;
                    glColor3f(alpha2*(float)slot[2], alpha2*(float)slot[3], alpha2*(float)slot[4]);
                    glTexCoord2f((float)(i+1) / (float)seg, 0.0f);
                    glVertex3fv((float *)(vp + 3));
                    glTexCoord2f((float)(i+1) / (float)seg, 1.0f);
                    glVertex3fv((float *)(vp - 0x57));
                    glEnd();
                }
            }
        }
    }  // end of explicit count loop (advance happens in for-statement)
}

// UI/game helpers
// FUN_004cba60 @ 0x004CBA60 — CharPreview_Reset
// Resets the char-select / second-password UI state:
//   - Clears DAT_07eaa118 (2nd-pass hover flag), DAT_07eaa132, DAT_07eaa134
//   - Clears DAT_07eaa119, DAT_00559f5f, DAT_07eaa14c, DAT_07eaa11a
//   - Resets two entity-slot arrays (stride 0x44, reset id/select fields to 0xffff/0)
//   - Clears DAT_07e11d28, calls FUN_00404bc0(0x19,0,0) and FUN_00404bc0(0x1c,0,0)
void __cdecl FUN_004cba60(void) {
    // Reset second-password hover/state flags
    DAT_07eaa118 = 0;
    DAT_07eaa132 = 0;
    DAT_07eaa134 = 0;
    DAT_07eaa119 = 0;
    DAT_00559f5f  = 0;
    DAT_07eaa14c  = 0;
    DAT_07eaa11a  = 0;
    DAT_07eaa11b  = 0;
    DAT_07eaa11c  = 0;
    DAT_07eaa128  = 0;
    DAT_07eaa12c  = 0;
    DAT_07eaa130  = 0;
    *((char*)&DAT_07eaa130 + 1) = 0; // DAT_07eaa131 (adjacent byte, not separately declared)

    // Reset char-slot entry arrays.
    // 2026-04-30 BUG-FIX: bound by array size not hardcoded original-binary
    // address.  Previously loops compared `(int)p < 0x7EA7B48` against our
    // build's globals which sit at OS-allocated addresses (~0x01F90000).
    // The condition was always true → walk overran into random memory →
    // write-AV at arbitrary address (we observed p=0x1F9E040 crashing).
    //
    // IDA original walks pointer +0x44 each step.  IDA decomp `p + 0x11`
    // is `p + 17 dwords = +68 bytes = +0x44`, but it accesses `p - 0x38`
    // (= -56 bytes from p).  In IDA the array layout is interpreted as
    // (record-base + 0x38) = `p`, so `p - 0x38` is the record base id field.
    {
        BYTE* base = DAT_07ea5b68;
        BYTE* end  = base + sizeof(DAT_07ea5b68);   // 0x1FE0 = 8160 bytes
        UINT* p    = (UINT*)(base + 0x38);          // start at offset 0x38 within first record
        while ((BYTE*)p < end) {
            *(unsigned short*)((char*)p - 0x38) = 0xffff;
            *p = 0;
            p = (UINT*)((BYTE*)p + 0x44);
        }
    }
    {
        BYTE* base = DAT_07ea9880;
        BYTE* end  = base + sizeof(DAT_07ea9880);   // 0x880 = 2176 bytes
        UINT* p    = (UINT*)(base + 0x38);
        while ((BYTE*)p < end) {
            *(unsigned short*)((char*)p - 0x38) = 0xffff;
            *p = 0;
            p = (UINT*)((BYTE*)p + 0x44);
        }
    }
    // Reset char-name index arrays
    int i = 0;
    while (i < 0x880) {
        *(unsigned short*)((int)&DAT_07ea7b88 + i) = 0xffff;
        *(unsigned short*)((int)&DAT_07ea5298 + i) = 0xffff;
        *(UINT*)((int)&DAT_07ea7bc0 + i) = 0;
        *(UINT*)((int)&DAT_07ea52d0 + i) = 0;
        if (DAT_07eaa0e8 == '\x01') {
            *(unsigned short*)((int)&DAT_07e11f78 + i) = 0xffff;
            *(UINT*)((int)&DAT_07e11fb0 + i) = 0;
        }
        i += 0x44;
    }
    DAT_07e11d28 = 0;
    FUN_00404bc0(0x19, 0, 0);
    FUN_00404bc0(0x1c, 0, 0);
}

// FUN_004cd3b0 @ 0x004CD3B0 — UI_ItemGrid_Fill
// Fills 2D grid buffers with current item slot data (DAT_07e91350) for equipment display.
// Dispatches by DAT_07ea9800; each grid entry = 0x11 dwords, selection flag at offset 0x38.
void __cdecl FUN_004cd3b0(void)
{
    if ((int)DAT_07e91388 < 1) return;

    int   iVar8 = *(int*)DAT_07e91350;
    int   iVar3 = iVar8 * 0x40 + (int)DAT_07d78068;
    unsigned int uVar5 = (unsigned int)*(unsigned char *)(iVar3 + 0x20); // item width
    unsigned int uVar9 = (unsigned int)*(unsigned char *)(iVar3 + 0x21); // item height
    unsigned int selCol = (unsigned int)DAT_07e9138e;
    unsigned int selRow = (unsigned int)DAT_07e9138f;

    // Position packed as col (bits 0-2) + row (bits 3+).
    // IDA usa Inventory[32].Type acá.  El comentario anterior decía que
    // DAT_07ea9844 era "our mirror" de eso, pero NO lo es: en el binario esa
    // direccion es un byte-flag aparte (bSell).  Ahora usamos el campo real.
    int posVal  = ItemPickedPos;
    if ((void*)DAT_07ea9800 == (void*)&OffsetInventoryItems[0] && posVal >= 12) {
        posVal -= 12;
    }
    int colBits = posVal & 7;
    int rowBits = posVal >> 3;

    // Helper lambda-style inline: fill grid block at gridBase (stride 8 cols)
    #define FILL_GRID(gridBase) do { \
        int *_gb = (int *)(gridBase); \
        for (unsigned int row = (unsigned int)rowBits; row < (unsigned int)rowBits + uVar9; row++) { \
            for (unsigned int col = (unsigned int)colBits; col < (unsigned int)colBits + uVar5; col++) { \
                int *cell = _gb + (col + row * 8) * 0x11; \
                int *src  = (int *)DAT_07e91350; \
                for (int k = 0; k < 0x11; k++) cell[k] = src[k]; \
                cell[0xe] = (row == selRow && col == selCol) ? 1 : 0; \
            } \
        } \
    } while(0)

    if ((void *)DAT_07ea9800 == (void *)&DAT_07ea7b88) { FILL_GRID(&DAT_07ea7bc0); return; }
    if ((void *)DAT_07ea9800 == (void *)&DAT_07ea5b30) { FILL_GRID(&DAT_07ea5b68); return; }
    if ((void *)DAT_07ea9800 == (void *)&DAT_07ea9848) { FILL_GRID(&DAT_07ea9880); return; }

    {
        BYTE* base = DAT_07ea8448;
        BYTE* end  = base + sizeof(DAT_07ea8448);
        while (base < end) {
            memset(base, 0, 0x44);
            *(short*)base = (short)0xFFFF;
            base += 0x44;
        }
    }

    if (posVal > 0xb) {
        FILL_GRID(&DAT_07ea8448);
        if (iVar8 == 0x1a0 || iVar8 == 0x1a2 || iVar8 == 0x1a3) {
            int spawnType = (iVar8 == 0x1a0) ? 0x330 : (iVar8 == 0x1a2) ? 0xc3 : 0x10b;
            FUN_004fffd0(spawnType, (void *)(DAT_07abf5d8 + 0x10), (void *)DAT_07abf5d8, 0);
        }
    }
    #undef FILL_GRID
    // (HashTable obfuscation blocks skipped — anti-tamper)
}

// FUN_004b0e80 @ 0x004B0E80 — Hotkey_ClassSync(void)
// Iterates hotkey table (DAT_07cf1ff4 +0x57+i, 0x14 entries):
//   compares entry's class byte at (classType*0x40 + DAT_07cf1ff4 + 0xd7 + i)
//   against classType (= 1, the pushed ESI at call site in Chat_InputTick).
//   On match, writes entity[0x391] = slot index (i).
// Also: if DAT_00559c5c set and sub-state != 6 and current hotkey class is 0x06 or 0x0f:
//   clears DAT_00559c50 / DAT_00559c58 to -1.
// unaff_EBP = GetAsyncKeyState ptr (HashTable only), unaff_retaddr = 1 (classType).
void __cdecl FUN_004b0e80(void)
{
    // classType = 1 (the pushed ESI value from Chat_InputTick)
    int classType = (int)DAT_005616ac;  // slot class index (confirmed: DAT_005616ac used as iVar7)
    char* charData = (char*)DAT_07cf1ff4; // CharData sub-pointer
    char* playerEnt = DAT_07abf5d8;      // player entity

    for (int i = 0; i < 0x14; i++) {
        // Check if slot is active and matches class
        if (*(charData + 0x57 + i) != '\0' &&
            (unsigned char)*(charData + classType * 0x40 + 0xd7 + i) == (unsigned char)1) {
            // Set entity hotkey slot index
            *(playerEnt + 0x391) = (char)i;
        }

        // If hover enabled, sub-state != 6, and current hotkey class is mage/elf
        if (DAT_00559c5c != '\0' && DAT_0055a7ac != 6) {
            int slotIdx = (unsigned char)*(playerEnt + 0x391);
            char hotkeyCls = *(charData + 0x57 + slotIdx);
            if (hotkeyCls == '\x06' || hotkeyCls == '\x0f') {
                DAT_00559c50 = 0xffffffff;
                DAT_00559c58 = 0xffffffff;
            }
        }
    }
}
// FUN_004e3d60 @ 0x004E3D60 — Connection_Check(ctx, cols, rows)
// Scans a 2D grid of short-based structs (stride 0x22 each element).
// Returns 1 (clear) if every entry is -1 or has ≤0 at offset+0x1c.
// Returns 0 (busy) if any entry has ≠-1 AND int at +0x1c > 0.
char __cdecl FUN_004e3d60(void *ctx, int p1, int p2) {
    char uVar1 = 1;
    short *psVar2 = (short*)ctx;
    for (int row = 0; row < p2; row++) {
        short *cur = psVar2;
        for (int col = 0; col < p1; col++) {
            if (*cur != -1 && *(int*)(cur + 0x1c) > 0)
                uVar1 = 0;
            cur += 0x22;
        }
        psVar2 += p1 * 0x22;
    }
    return uVar1;
}

// FUN_00494520 @ 0x00494520 — Auth_XorEncode(key_ctx, buf, flag)
// __thiscall in original (this=key_ctx). XOR-encodes buf[] using a key schedule derived
// from this->key_table (256-byte S-box from RC4 variant). flag=0: encode only; flag!=0: also validates.
// Uses SEH frame + 256-byte S-box + HashTable obfuscation. 3500+ lines in binary.
// STUB: SEH + unaff_ESI pattern prevent safe implementation.
unsigned long __cdecl FUN_00494520(void*, unsigned char*, char) { return 0; } // STUB: Auth_XorEncode — SEH+RC4

// FUN_00513440 @ 0x00513440 — Chat_Validate(buf)
// Returns 1 if buf (after stripping spaces) matches any word in the
// banned-keyword table at DAT_07d73104 (stride 0x14, count DAT_07d78070).
// Returns 0 if buf starts with '/' or no match found.
char __cdecl FUN_00513440(char *param_1) {
    if (*param_1 == '/') return 0;
    // strip spaces into local buf
    int len = (int)strlen(param_1);
    char local_100[256];
    int out = 0;
    for (int i = 0; i < len; i++) {
        if (param_1[i] != ' ') local_100[out++] = param_1[i];
    }
    local_100[out] = '\0';
    // search banned word table
    int count = (int)DAT_07d78070;
    char *entry = DAT_07d73104;
    for (int i = 0; i < count; i++) {
        if (FUN_004977f0(local_100, entry, '\0')) return 1;
        entry += 0x14;
    }
    return 0;
}
// FUN_00497c70 @ 0x00497C70 — Auth_KeySchedule(void)
// Initializes the RC4-variant S-box used by FUN_00494520.
// Reads seed data from DAT_07cf1ffc (CharData), permutes 256-byte key table.
// Called once during login handshake. Many unreachable blocks (dead code).
// NOP confirmed safe — key schedule runs in original binary only; stub here has no effect.
void __cdecl FUN_00497c70(void) {} // NOP — Auth_KeySchedule (no external side effects needed)
// FUN_004e9250 @ 0x004E9250 — SecondPassword_Shuffle(mode)
// Initializes a 10-element short array at DAT_07e91394 with values 0..9,
// then performs 20 random XOR swaps. Stores mode in DAT_07eaa14c.
// Returns last randomly-computed iVar4 (ignore value — callers discard).
int __cdecl FUN_004e9250(int mode) {
    // Fill 0..9
    short* arr = (short*)&DAT_07e91394;
    for (int i = 0; i < 10; i++) arr[i] = (short)i;
    // 20 random swaps
    int iVar4 = 0;
    for (int k = 0; k < 20; k++) {
        int a = rand() % 10;
        int b = rand() % 10;
        iVar4 = b / 10; // mirrors original (iVar3 = rand() % 10; iVar4 = iVar3/10)
        if (a != b) {
            arr[a] ^= arr[b];
            arr[b] ^= arr[a];
            arr[a] ^= arr[b];
        }
    }
    _DAT_07ea9814 = 0.0f;
    DAT_07ea9818  = 0;
    DAT_07eaa14c  = (DWORD)mode;
    DAT_07ea981c  = 0;
    DAT_07ea981e  = 0;
    return iVar4;
}


// Terrain / map loaders (called from FUN_0050e5a0 / Map_LoadResources in stubs.cpp)

// FUN_004f6f90 @ 0x004F6F90 — Terrain_LoadMap(path)
// Reads map file: skips 1 byte, copies 0x4000×4 bytes to DAT_080bb2b4 (tile map),
// next 0x4000×4 bytes to DAT_080ab2b4 (alt-tile), then 0x10000 height bytes → DAT_0834b608 as float.
//
// BUG-FIX 2026-05-01: el archivo `EncTerrain%d.map` está ENCRIPTADO con el mismo
// BuxConvert (3-byte XOR rolling) que usa OpenTerrainAttribute (.att). Sin
// descifrarlo, los bytes raw del file se interpretaban como tile-texture-IDs
// y heights → suelo render como mosaico de UI textures con quads de altura
// infinity (causa el triángulo cyan gigante). Aplicar BuxConvert antes de parsear.
void __cdecl FUN_004f6f90(const char *path) {
    FUN_004f6c60();
    FILE *f = FUN_0054173f(path, DAT_005580ac);
    if (!f) {
        char d[256]; wsprintfA(d, "TerrainMap LOAD FAIL: %s", path);
        DbgLogPublic(d);
        return;
    }
    FUN_00543037((int*)f, 0, 2);
    unsigned int sz = (unsigned int)FUN_00542eb4((char*)f);
    FUN_00543037((int*)f, 0, 0);
    char *buf = (char*)operator_new(sz);
    if (!buf) { FUN_0054150f(f); return; }
    FUN_00541597(buf, 1, sz, (int*)f);
    FUN_0054150f(f);

    // Decrypt with MapFileDecrypt (16-byte rolling key + running counter).
    // Format post-decrypt: byte 0 = magic, bytes 1+ = 3 layers of 0x10000 bytes.
    MapFileDecrypt((BYTE*)buf, (int)sz);

    // BUG-FIX 2026-05-01 (v3): formato Enc tiene BYTE EXTRA de version flag.
    // Verificación: archivo .map size = 0x30002 = 1(magic) + 1(version) + 3*0x10000(data).
    // Verificación: archivo .obj size = 64324 = 1+1+2(count short)+30*2144 → count=0x0860.
    // El parser 0.85 leía desde buf+1; en archivos Enc hay que leer desde buf+2.
    char *p = buf + 2;
    DWORD *dst = (DWORD*)DAT_080bb2b4;
    for (int i = 0; i < 0x4000; i++) { *dst++ = *(DWORD*)p; p += 4; }
    dst = (DWORD*)DAT_080ab2b4;
    for (int i = 0; i < 0x4000; i++) { *dst++ = *(DWORD*)p; p += 4; }
    // alpha bytes → float (per IDA 0.85: alpha[i] = byte * (1/255.0f))
    float *fDst = DAT_0834b608;
    unsigned char *hSrc = (unsigned char*)p;
    for (int i = 0; i < 0x10000; i++) *fDst++ = (float)*hSrc++ * _DAT_00552b70;
    operator_delete(buf);
}

// FUN_004f6ce0 @ 0x004F6CE0 — OpenTerrainAttribute(FileName)
// Per IDA decompile (raw/004F6CE0_OpenTerrainAttribute.c, 442 bytes).
//
// Reads a 65539-byte .att file into a temp buffer, runs BuxConvert (XOR
// descrambler with 3-byte key at DAT_0055a770), then memcpy bytes [+3..+65539]
// into the terrain wall array DAT_0838bc70 (256×256). First byte must be 0,
// next short must be 0xFFFF (file marker). Per-world magic-byte check at
// known offsets validates the right map. Any byte >= 0x80 triggers Error.
//
// BUG-FIX 2026-04-27: previously a no-op (signature was void(void), path
// param lost). Now properly loads the .att file via the same FUN_0054xxxx
// pipeline used by the other terrain loaders.
unsigned char* TerrainWall = (unsigned char*)&DAT_0838bc70;
int __cdecl FUN_004f6ce0(const char *FileName) {
    FILE *fp = fopen(FileName, "rb");
    if (!fp) {
        // 2026-05-04: silent fail — el caller (stubs.cpp:1922) prueba dos
        // formatos (Terrain*.att y EncTerrain*.att). MessageBox bloqueante +
        // WM_DESTROY harían imposible el fallback. Loggear y retornar.
        char dbg[260];
        wsprintfA(dbg, "OpenTerrainAttribute: file not found '%s'", FileName);
        DbgLogPublic(dbg);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    int iSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (iSize != 65539) {
        char dbg[260];
        wsprintfA(dbg, "OpenTerrainAttribute: '%s' wrong size %d (expected 65539)",
                  FileName, iSize);
        DbgLogPublic(dbg);
        fclose(fp);
        return 0;
    }

    static unsigned char attBuf[65539];
    fread(attBuf, 65539, 1, fp);
    FUN_004f6eb0((int)attBuf, 65539);   // BuxConvert (3-byte XOR key)

    memcpy(TerrainWall, attBuf + 3, 65536);

    bool Error = false;
    if (attBuf[0] != 0 || *(unsigned short*)(attBuf + 1) != 0xFFFF)
        Error = true;

    if (DAT_083a410c == '\0') {
        // Per-world magic byte check (sanity vs distributed .att files).
        switch (DAT_0055a7ac) {
        case 0: if (TerrainWall[31623] != 5) Error = true; break;
        case 1: if (TerrainWall[30947] != 4) Error = true; break;
        case 2: if (TerrainWall[14288] != 5) Error = true; break;
        case 3: if (TerrainWall[30650] != 5) Error = true; break;
        case 4: if (TerrainWall[19393] != 5) Error = true; break;
        }
    }
    for (int i = 0; i < 65536; ++i) {
        if (TerrainWall[i] >= 0x80) { Error = true; break; }
    }

    if (Error) {
        // 2026-05-04: silent fail — la valid magic-byte check del IDA original
        // mata el proceso si fallía. En nuestro build preferimos seguir con
        // walkable=0 implícito (mejor que crash duro).
        DbgLogPublic("OpenTerrainAttribute: validation Error (magic-byte mismatch or byte>=0x80)");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    {
        char dbg[160];
        wsprintfA(dbg, "OpenTerrainAttribute OK '%s' (loaded %d bytes into TerrainWall)",
                  FileName, 65536);
        DbgLogPublic(dbg);
    }
    return 1;
}

// FUN_004ffe70 @ 0x004FFE70 — Terrain_LoadObjects(path)
// Reads .obj file: 2-byte count, then count×0x1e entries → calls FUN_004ff5a0 for each.
//
// BUG-FIX 2026-05-01: el archivo `EncTerrain%d.obj` está ENCRIPTADO (mismo
// BuxConvert 3-byte XOR rolling key que .att). Sin descifrar, count y posiciones
// son basura → no se spawnean instancias de objetos del mundo (casas, NPCs
// estáticos, props) → mapa renderiza solo terreno + hero.
void __cdecl FUN_004ffe70(const char *path) {
    FILE *f = FUN_0054173f(path, DAT_005580ac);
    if (!f) {
        // CRITICAL BUG-FIX 2026-05-08: previously wrote the error string into
        // `(char*)&DAT_083a0218` — the bucket-grid cell[0] start in our build.
        // IDA's original used a stack-local `char Text[256]` that Ghidra
        // mis-decompiled as the global symbol. Writing "File not found: %s"
        // there overwrote cell[0].head/tail with garbage like 0x656c6946
        // ("File"), turning the bucket walker into a deref-into-unmapped
        // memory fault on the next frame (the AV chain
        // Object_MoveUpdate → MoveObjects → FUN_004fdc00 → FUN_0043e5c0).
        char Text[256];
        crt_sprintf(Text, "OpenObjectsEnc: file not found '%s'", path);
        DbgLogPublic(Text);
        return;
    }
    FUN_00543037((int*)f, 0, 2);
    unsigned int sz = (unsigned int)FUN_00542eb4((char*)f);
    FUN_00543037((int*)f, 0, 0);
    char *buf = (char*)operator_new(sz);
    FUN_00541597(buf, 1, sz, (int*)f);
    FUN_0054150f(f);

    // Decrypt with MapFileDecrypt; format Enc post-decrypt: byte 0 = magic,
    // byte 1 = version flag, short[2..3] = count, entries from byte 4 (stride 30B).
    MapFileDecrypt((BYTE*)buf, (int)sz);

    int count = (int)*(short*)(buf + 2);
    // BUG-FIX 2026-08-17: el guard era `count > 0 && count < 5000`, un tope
    // inventado por el port (IDA 0x4FFE70 sólo chequea `> 0`). Los conteos
    // reales del 0.97k son Lorencia 2870, Dungeon 4488, Atlans 5205,
    // LostTower 5380 y Noria 9399 — o sea el cap descartaba el archivo ENTERO
    // en los tres últimos y esos mapas quedaban sin un solo objeto (paredes,
    // puentes, props). Ahora el bound sale del tamaño real del buffer, que es
    // lo único que hace falta para no leer fuera: el header son 4 bytes y cada
    // entrada 30 (verificado: 4 + 30*count == filesize exacto en los 5 mapas).
    int maxByBuf = ((int)sz - 4) / 30;
    if (count > maxByBuf) count = maxByBuf;
    if (count > 0) {
        short *p = (short*)(buf + 4);
        for (int i = 0; i < count; i++, p += 0xf) {
            float pos[3]  = { *(float*)(p+1), *(float*)(p+3), *(float*)(p+5) };
            float tgt[3]  = { *(float*)(p+7), *(float*)(p+9), *(float*)(p+0xb) };
            // BUG-FIX 2026-05-03: el 4° arg de FUN_004ff5a0 es `float param_4`
            // (la SCALE del objeto en el .obj). Antes leíamos como `*(unsigned int*)`
            // y la conversión implícita int→float convertía el bit pattern de 1.0f
            // (= 0x3F800000 = 1065353216) en el float 1065353216.0f literal →
            // scale gigante → vertices transformados fuera del frustum → invisible.
            // El IDA original lee como `*(float*)` (bit-cast) preservando los bits.
            FUN_004ff5a0((int)*p, pos, tgt, *(float*)(p + 0xd));
        }
    }
    operator_delete(buf);
}

// FUN_004f7250 @ 0x004F7250 — Terrain_LoadLight(path)
// Loads TerrainLight.jpg into DAT_07eeb238 (RGB float buffer, 256x256x3),
// then processes via FUN_004f70b0 / FUN_004f71c0.
//
// BUG-FIX 2026-04-28: el decomp pasaba la dirección absoluta hardcodeada
// 0x7eeb238 que en el binario original es DAT_07eeb238. En nuestro proceso
// esa dirección no existe → AV al escribir. Ahora pasamos &DAT_07eeb238,
// que es el array real.
void __cdecl FUN_004f7250(const char *path) {
    FUN_00529360((char*)path, (int)(uintptr_t)DAT_07eeb238);
    FUN_004f70b0();
    FUN_004f71c0();
}

// FUN_004f7270 @ 0x004F7270 — Terrain_LoadHeight(path)
// Sets flag, loads height bitmap, flushes.
void __cdecl FUN_004f7270(const char *path) {
    DAT_0839bc84 = 1;
    FUN_004f7290((char*)path);
    FUN_004f9c20();
}

// FUN_00502b80 @ 0x00502B80 — ClearItems / Map_InitEntities
// Clears the "alive" flag (offset 0) for every slot in the GroundItem pool.
// Pool is at DAT_07e12840, 1000 slots × 0x204 bytes.
//
// BUG-FIX 2026-04-28: el decomp Ghidra hardcodeaba la dirección absoluta
// del binario original (0x07E12840 .. 0x07E907E0). En nuestro proceso esa
// dirección no existe → AV. Indexamos el array real ahora que está en
// globals.cpp con tamaño correcto.
// 2026-08-21: limpiaba el offset 0 de cada slot.  IDA arranca en
// `&Items[0][72]` — el flag activo vive en ip+72, que es el que leen
// Net_Process (0x20), FUN_005038e0 y MoveItems.  O sea ClearItems no borraba
// nada y los items del mapa anterior seguían "vivos" al cambiar de zona.
void __cdecl FUN_00502b80(void) {
    for (int i = 0; i < 1000; ++i) {
        DAT_07e12840[i * 0x204 + 72] = 0;
    }
}

// FUN_00509190 @ 0x00509190 — Terrain_InitLayers
// Frees tile model slots (0xf604..0x11710), then frees sound channels 0x78..0xa9.
void __cdecl FUN_00509190(void) {
    for (int i = 0xf604; i < 0x11710; i += 0xbc)
        FUN_00442090(i + DAT_05828d58);
    for (int i = 0x78; i < 0xaa; i++) FUN_00404ad0(i);
}

// FUN_00509880 @ 0x00509880 — Terrain_InitWater
// Frees water model slots (0xc648..0xf604), then frees sound channels 0xaa..0x1a3.
void __cdecl FUN_00509880(void) {
    for (int i = 0xc648; i < 0xf604; i += 0xbc)
        FUN_00442090(i + DAT_05828d58);
    for (int i = 0xaa; i < 0x1a4; i++) FUN_00404ad0(i);
}

// FUN_0050c4d0 @ 0x0050C4D0 — Map_LoadObjectModels
// Loads world-specific animated props + object models for current zone.
// (Scene_Objects.cpp tiene un port alternativo con strings distintos.)
void __cdecl FUN_0050c4d0(void) {
    char cVar2 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = '\0';
        DAT_0055a7ac = 7;
    }

    FUN_00529740("Object8_drop01.jpg", 0x4d9, 0x2600, 0x2900, 0, '\x01');

    if (DAT_0055a7c4 == '\0') {
        switch (DAT_0055a7ac) {
        case 0:
            FUN_00505e90((int)0xae, "Data2/Object1/Animal/", "bird.smd");
            FUN_00505e90((int)0xb5, "Data2/Object1/Animal/", "fish.smd");
            break;
        case 1:
        case 4:
            FUN_00505e90((int)0xd7, "Data2/Object2/", "DungeonStone.smd");
            FUN_00505e90((int)0xb0, "Data2/Object2/", "Bat.smd");
            FUN_00505e90((int)0xb1, "Data2/Object2/", "mouse.smd");
            break;
        case 3:
            FUN_00505e90((int)0xaf, "Data2/Object1/Animal/", "butterfly.smd");
            break;
        case 5:
            FUN_00505e90((int)0xe4, "Data2/Object6/", "Meteo.smd");
            FUN_00505e90((int)0xe5, "Data2/Object6/", "Meteo.smd");
            FUN_00505e90((int)0xe6, "Data2/Object6/", "Meteo.smd");
            FUN_00505e90((int)0xe7, "Data2/Object6/", "Meteo.smd");
            FUN_00505e90((int)0xe8, "Data2/Object6/", "Meteo.smd");
            FUN_00505e90((int)0xea, "Data2/Monster/", "BossHead.smd");
            FUN_00505e90((int)0xeb, "Data2/Object6/", "Princess.smd");
            break;
        case 6:
            FUN_00505e90((int)0xb2, "Data2/Object7/", "SummonMonster.smd");
            *(unsigned char *)(*(int *)(DAT_05828d58 + 0x82e8) + 0x1a) = 1;
            break;
        case 7:
            FUN_00505e90((int)0xb6, "Data2/Object8/", "WaterMill.smd");
            FUN_00505e90((int)0xb7, "Data2/Object8/", "BladedStatue.smd");
            FUN_00505e90((int)0xb8, "Data2/Object8/", "Stairway.smd");
            FUN_00505e90((int)0xb9, "Data2/Object8/", "Bridge.smd");
            FUN_00505e90((int)0xba, "Data2/Object8/", "Trap01.smd");
            FUN_00505e90((int)0xbb, "Data2/Object8/", "Trap02.smd");
            FUN_00505e90((int)0xbc, "Data2/Object8/", "Trap03.smd");
            FUN_00505e90((int)0xbd, "Data2/Object8/", "FireArrow.smd");
            for (int iVar7 = 0xbc; iVar7 < 0x69c; iVar7 += 0xbc)
                *(unsigned char *)(*(int *)(iVar7 + 0x851c + DAT_05828d58) + 10) = 1;
            break;
        case 8:
            FUN_00505e90((int)0xb3, "Data2/Object9/", "SandPillar.smd");
            break;
        case 10:
            FUN_00529740("Effect/clouds.jpg",      0x4f4, 0x2601, 0x2900, 0, '\x01');
            FUN_00505e90((int)0xb6, "Data2/Object11/", "cloud.smd");
            FUN_005060b0(0xb6, "Data/Object11/", "cloud", -1);
            FUN_00505c80(0xb6, "Object11/", 0x2600, '\x01');
            FUN_00529740("Effect/cloudLight.jpg",  0x4f5, 0x2601, 0x2900, 0, '\x01');
            break;
        case 0xb: case 0xc: case 0xd: case 0xe: case 0xf: case 0x10:
            FUN_00505e90((int)0xb8, "Data2/Object12/", "Angel.smd");
            FUN_00505e90((int)0x106, "Data2/Object12/", "gate_entrance.smd");
            FUN_00505e90((int)0x107, "Data2/Object12/", "gate_entrance2.smd");
            FUN_00505e90((int)0x104, "Data2/Object12/", "gate_left.smd");
            FUN_00505e90((int)0x105, "Data2/Object12/", "gate_right.smd");
            FUN_00505e90((int)0xb9, "Data2/Object12/", "shine.smd");
            FUN_00529740("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, '\x01');
            FUN_00404a10(0x6e, "Data/Sound/iBloodCastle.wav", 1, '\0');
            DAT_0055a7c4 = '\x01';
            break;
        }
    }

    // Object type texture/name registration (second pass, all maps)
    FUN_00505bd0(0x69);
    switch (DAT_0055a7ac) {
    case 0:
        FUN_005060b0(0xae, "Data/Object1/", "bird", 1);
        FUN_00505c80(0xae, "Object1/", 0x2600, '\x01');
        FUN_005060b0(0xb5, "Data/Object1/", "fish", 1);
        FUN_00505c80(0xb5, "Object1/", 0x2600, '\x01');
        break;
    case 1: case 4:
        FUN_005060b0(0xd7, "Data/Object2/", "DungeonStone", 1);
        FUN_00505c80(0xd7, "Object2/", 0x2600, '\x01');
        FUN_005060b0(0xb0, "Data/Object2/", "Bat", 1);
        FUN_00505c80(0xb0, "Object2/", 0x2600, '\x01');
        FUN_005060b0(0xb1, "Data/Object2/", "mouse", 1);
        FUN_00505c80(0xb1, "Object2/", 0x2600, '\x01');
        break;
    case 3:
        FUN_005060b0(0xaf, "Data/Object1/", "Butterfly", 1);
        FUN_00505c80(0xaf, "Object1/", 0x2600, '\x01');
        break;
    case 5:
        for (int i = 0xe4; i < 0xec; i++) {
            FUN_005060b0(i, "Data/Object6/", "Meteo", i - 0xe3);
            FUN_00505c80(i, "Object6/", 0x2600, '\x01');
        }
        FUN_005060b0(0xea, "Data/Object6/", "BossHead", 1);
        FUN_005060b0(0xeb, "Data/Object6/", "Princess", 1);
        break;
    case 6:
        FUN_005060b0(0xb2, "Data/Object7/", "SummonMonster", 1);
        FUN_00505c80(0xb2, "Object7/", 0x2600, '\x01');
        break;
    case 7:
        // BUG-FIX 2026-08-17: el basename era "Object8" → pedía Object802..Object810,
        // que no existen; los 9 peces de Atlans no cargaban. IDA 0050C4D0 L171:
        //   AccessModel(v3, "Data\Object8\", "Fish", v3 - 180)  para v3 = 182..190
        for (int i = 0xb6; i < 0xbf; i++) {
            FUN_005060b0(i, "Data/Object8/", "Fish", i - 0xb4);
            FUN_00505c80(i, "Object8/", 0x2600, '\x01');
        }
        // BUG-FIX 2026-08-17: faltaba entero el bloque de texturas de agua de
        // Atlans (IDA L175-199). Carga wt00..wt31 en Bitmaps[65..96] y además
        // copia el nombre corto en Bitmaps[n].FileName (offset 0 del slot,
        // stride 0x38), que es de donde lo lee el render de tiles de agua.
        // El "if (v5 >= &Bitmaps[75])" del decompile es simplemente v4 >= 10:
        // wt00..wt09 llevan cero a la izquierda, wt10..wt31 no.
        {
            char Buffer[64];
            for (int v4 = 0; v4 < 32; v4++) {
                const char* fmtFull = (v4 >= 10) ? "Object8/wt%d.jpg" : "Object8/wt0%d.jpg";
                _snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, fmtFull, v4);
                FUN_00529740(Buffer, v4 + 65, 0x2601, 0x2901, 0, '\0');

                const char* fmtLeaf = (v4 >= 10) ? "wt%d.jpg" : "wt0%d.jpg";
                _snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, fmtLeaf, v4);
                lstrcpynA(&g_BitmapsRaw[(v4 + 65) * 0x38], Buffer, 0x20);
            }
        }
        break;
    case 8:
        FUN_00529740("Object9/sand01.jpg",    0x494, 0x2601, 0x2901, 0, '\x01');
        FUN_00529740("Object9/sand02.jpg",    0x495, 0x2601, 0x2901, 0, '\x01');
        FUN_00529740("Object9/Impack03.jpg",  0x597, 0x2601, 0x2900, 0, '\x01');
        FUN_005060b0(0xb3, "Data/Object9/", "SandPillar", 2);
        FUN_00505c80(0xb3, "Object9/", 0x2600, '\x01');
        break;
    case 10:
        FUN_005060b0(0xb6, "Data/Object11/", "cloud", -1);
        FUN_00505c80(0xb6, "Object11/", 0x2600, '\x01');
        break;
    case 0xb: case 0xc: case 0xd: case 0xe: case 0xf: case 0x10:
        FUN_005060b0(0xb8, "Data/Object12/", "Angel", 1);
        FUN_00505c80(0xb8, "Object12/", 0x2600, '\x01');
        FUN_005060b0(0x106, "Data/Object12/", "gate_entrance", 1);
        FUN_005060b0(0x107, "Data/Object12/", "gate_entrance", 2);
        FUN_005060b0(0x104, "Data/Object12/", "StoneCoffin", 1);
        FUN_005060b0(0x105, "Data/Object12/", "StoneCoffin", 2);
        FUN_005060b0(0xb9, "Data/Object12/", "Shine", 1);
        FUN_00505c80(0xb9, "Object12/", 0x2600, '\x01');
        break;
    }

    // Object model loading for all maps (FUN_00505bd0(0x2ee) then per-map loading)
    FUN_00505bd0(0x2ee);
    if (DAT_0055a7ac == 0) {
        // Lorencia (Object1) — load SMD models on first call
        if (DAT_0055a7c4 == '\0') {
            FUN_00505e90((int)0x00, "Data2/Object1/", "treesmall.smd");
            FUN_00505e90((int)0x01, "Data2/Object1/", "treebig.smd");
            FUN_00505e90((int)0x02, "Data2/Object1/", "treea_01.smd");
            FUN_00505e90((int)0x03, "Data2/Object1/", "treea_02.smd");
            FUN_00505e90((int)0x04, "Data2/Object1/", "treea_03.smd");
            FUN_00505e90((int)0x05, "Data2/Object1/", "treea_04.smd");
            FUN_00505e90((int)0x06, "Data2/Object1/", "treea_05.smd");
            FUN_00505e90((int)0x07, "Data2/Object1/", "treea_06.smd");
            FUN_00505e90((int)0x08, "Data2/Object1/", "treea_07.smd");
            FUN_00505e90((int)0x09, "Data2/Object1/", "treea_08.smd");
            FUN_00505e90((int)0x0a, "Data2/Object1/", "treea_09.smd");
            FUN_00505e90((int)0x0b, "Data2/Object1/", "treea_10.smd");
            FUN_00505e90((int)0x0c, "Data2/Object1/", "treea_11.smd");
            FUN_00505e90((int)0x14, "Data2/Object1/", "grass_01.smd");
            FUN_00505e90((int)0x15, "Data2/Object1/", "grass_02.smd");
            FUN_00505e90((int)0x16, "Data2/Object1/", "grass_03.smd");
            FUN_00505e90((int)0x17, "Data2/Object1/", "grass_04.smd");
            FUN_00505e90((int)0x18, "Data2/Object1/", "grass_05.smd");
            FUN_00505e90((int)0x19, "Data2/Object1/", "grass_06.smd");
            FUN_00505e90((int)0x1e, "Data2/Object1/", "mushroom_01.smd");
            FUN_00505e90((int)0x1f, "Data2/Object1/", "mushroom_02.smd");
            FUN_00505e90((int)0x20, "Data2/Object1/", "Ston_01.smd");
            FUN_00505e90((int)0x21, "Data2/Object1/", "Ston_02.smd");
            FUN_00505e90((int)0x22, "Data2/Object1/", "Ston_03.smd");
            FUN_00505e90((int)0x23, "Data2/Object1/", "Ston_04.smd");
            FUN_00505e90((int)0x24, "Data2/Object1/", "Ston_05.smd");
            FUN_00505e90((int)0x28, "Data2/Object1/", "stone_statue01.smd");
            FUN_00505e90((int)0x29, "Data2/Object1/", "stone_statue02.smd");
            FUN_00505e90((int)0x2a, "Data2/Object1/", "Angel_Stone.smd");
            FUN_00505e90((int)0x2b, "Data2/Object1/", "steel_barred_door_side.smd");
            FUN_00505e90((int)0x2c, "Data2/Object1/", "Tomb_arc.smd");
            DAT_083a4100 = 1;
            FUN_00505e90((int)0x2d, "Data2/Object1/", "Tomb_cross.smd");
            FUN_00505e90((int)0x2e, "Data2/Object1/", "TombStone.smd");
            FUN_00505e90((int)0x32, "Data2/Object1/", "fire_light.smd");
            FUN_00505e90((int)0x33, "Data2/Object1/", "Fire_Light_01.smd");
            FUN_00505e90((int)0x34, "Data2/Object1/", "Fire.smd");
            FUN_00505e90((int)0x37, "Data2/Object1/", "dungeon_gate_01.smd");
            FUN_00505e90((int)0x3a, "Data2/Object1/", "Drum.smd");
            FUN_00505e90((int)0x3b, "Data2/Object1/", "Treasure_Chest.smd");
            FUN_00505e90((int)0x3c, "Data2/Object1/", "ship.smd");
            FUN_00505e90((int)0x38, "Data2/Object1/", "monster_a.smd");
            FUN_00505e90((int)0x39, "Data2/Object1/", "monster_b.smd");
            FUN_00505e90((int)0x41, "Data2/Object1/", "steel_barred_wall01.smd");
            FUN_00505e90((int)0x42, "Data2/Object1/", "steel_barred_wall02.smd");
            FUN_00505e90((int)0x43, "Data2/Object1/", "steel_barred_wall03.smd");
            FUN_00505e90((int)0x44, "Data2/Object1/", "steel_barred_door.smd");
            FUN_00505e90((int)0x45, "Data2/Object1/", "wall_01.smd");
            FUN_00505e90((int)0x46, "Data2/Object1/", "wall_02.smd");
            FUN_00505e90((int)0x47, "Data2/Object1/", "wall_03.smd");
            FUN_00505e90((int)0x48, "Data2/Object1/", "wall_04.smd");
            FUN_00505e90((int)0x49, "Data2/Object1/", "wall_05.smd");
            FUN_00505e90((int)0x4a, "Data2/Object1/", "wall_06.smd");
            FUN_00505e90((int)0x4b, "Data2/Object1/", "c_wall01.smd");
            FUN_00505e90((int)0x4c, "Data2/Object1/", "c_wall02.smd");
            FUN_00505e90((int)0x4d, "Data2/Object1/", "c_wall03.smd");
            FUN_00505e90((int)0x4e, "Data2/Object1/", "c_wall04.smd");
            FUN_00505e90((int)0x4f, "Data2/Object1/", "c_wall05.smd");
            FUN_00505e90((int)0x50, "Data2/Object1/", "bridge_01.smd");
            FUN_00505e90((int)0x51, "Data2/Object1/", "fence_01.smd");
            FUN_00505e90((int)0x52, "Data2/Object1/", "fence_02.smd");
            FUN_00505e90((int)0x53, "Data2/Object1/", "fence_03.smd");
            FUN_00505e90((int)0x54, "Data2/Object1/", "fence_04.smd");
            FUN_00505e90((int)0x55, "Data2/Object1/", "bridge_stone.smd");
            FUN_00505e90((int)0x5a, "Data2/Object1/", "StreetLight.smd");
            FUN_00505e90((int)0x5b, "Data2/Object1/", "cannon_01.smd");
            FUN_00505e90((int)0x5c, "Data2/Object1/", "cannon_02.smd");
            FUN_00505e90((int)0x5d, "Data2/Object1/", "cannon_03.smd");
            FUN_00505e90((int)0x5f, "Data2/Object1/", "badge_01.smd");
            FUN_00505e90((int)0x60, "Data2/Object1/", "signboard_01.smd");
            FUN_00505e90((int)0x61, "Data2/Object1/", "signboard_02.smd");
            FUN_00505e90((int)0x62, "Data2/Object1/", "carriage_01.smd");
            FUN_00505e90((int)0x63, "Data2/Object1/", "carriage_02.smd");
            FUN_00505e90((int)0x64, "Data2/Object1/", "carriage_03.smd");
            FUN_00505e90((int)0x65, "Data2/Object1/", "carriage_04.smd");
            FUN_00505e90((int)0x66, "Data2/Object1/", "straw_01.smd");
            FUN_00505e90((int)0x67, "Data2/Object1/", "straw_02.smd");
            FUN_00505e90((int)0x69, "Data2/Object1/", "waterspout.smd");
            FUN_00505e90((int)0x6a, "Data2/Object1/", "jar_01.smd");
            FUN_00505e90((int)0x6b, "Data2/Object1/", "jar_02.smd");
            FUN_00505e90((int)0x6c, "Data2/Object1/", "jar_03.smd");
            FUN_00505e90((int)0x6d, "Data2/Object1/", "jar_04.smd");
            FUN_00505e90((int)0x6e, "Data2/Object1/", "hanging_01.smd");
            FUN_00505e90((int)0x6f, "Data2/Object1/", "stair_01.smd");
            FUN_00505e90((int)0x73, "Data2/Object1/", "house_01.smd");
            FUN_00505e90((int)0x74, "Data2/Object1/", "house_02.smd");
            FUN_00505e90((int)0x75, "Data2/Object1/", "house_03.smd");
            FUN_00505e90((int)0x76, "Data2/Object1/", "house_04.smd");
            FUN_00505e90((int)0x77, "Data2/Object1/", "house_05.smd");
            FUN_00505e90((int)0x78, "Data2/Object1/", "tent_01.smd");
            FUN_00505e90((int)0x79, "Data2/Object1/", "house_wall_01.smd");
            FUN_00505e90((int)0x7a, "Data2/Object1/", "house_wall_02.smd");
            FUN_00505e90((int)0x7b, "Data2/Object1/", "house_wall_03.smd");
            FUN_00505e90((int)0x7c, "Data2/Object1/", "house_wall_04.smd");
            FUN_00505e90((int)0x7d, "Data2/Object1/", "house_wall_05.smd");
            FUN_00505e90((int)0x7e, "Data2/Object1/", "house_wall_06.smd");
            FUN_00505e90((int)0x7f, "Data2/Object1/", "house_etc_01.smd");
            FUN_00505e90((int)0x80, "Data2/Object1/", "house_etc_02.smd");
            FUN_00505e90((int)0x81, "Data2/Object1/", "house_etc_03.smd");
            FUN_00505e90((int)0x82, "Data2/Object1/", "light_01.smd");
            FUN_00505e90((int)0x83, "Data2/Object1/", "light_02.smd");
            FUN_00505e90((int)0x84, "Data2/Object1/", "light_03.smd");
            FUN_00505e90((int)0x85, "Data2/Object1/", "posebox_01.smd");
            FUN_00505e90((int)0x8c, "Data2/Object1/", "furniture_01.smd");
            FUN_00505e90((int)0x8d, "Data2/Object1/", "furniture_02.smd");
            FUN_00505e90((int)0x8e, "Data2/Object1/", "furniture_03.smd");
            FUN_00505e90((int)0x8f, "Data2/Object1/", "furniture_04.smd");
            FUN_00505e90((int)0x90, "Data2/Object1/", "furniture_05.smd");
            FUN_00505e90((int)0x91, "Data2/Object1/", "furniture_06.smd");
            FUN_00505e90((int)0x92, "Data2/Object1/", "furniture_07.smd");
            FUN_00505e90((int)0x96, "Data2/Object1/", "candle.smd");
            FUN_00505e90((int)0x97, "Data2/Object1/", "beer_01.smd");
            FUN_00505e90((int)0x98, "Data2/Object1/", "beer_02.smd");
            FUN_00505e90((int)0x99, "Data2/Object1/", "beer_03.smd");
        }
        // BUG-FIX 2026-05-04: agregar load explícito de BMDs Object1.
        // El bloque SMD arriba está gated por `DAT_0055a7c4 == 0` que en nuestro
        // build SIEMPRE es 1 (default = Data mode, no Data2/), así que las SMDs
        // nunca se cargaban. Como la distribución solo trae BMDs con nombres
        // PascalCase (House01.bmd, Tree01.bmd, Bridge01.bmd, etc.), aquí mapeamos
        // explícitamente cada slot SMD a su BMD equivalente.
        struct LorenciaSlot { int slot; const char* bmd; };
        static const LorenciaSlot lorenciaSlots[] = {
            // Trees (slots 0x00..0x0c → Tree01..Tree13)
            { 0x00, "Tree01" }, { 0x01, "Tree02" },
            { 0x02, "Tree03" }, { 0x03, "Tree04" }, { 0x04, "Tree05" },
            { 0x05, "Tree06" }, { 0x06, "Tree07" }, { 0x07, "Tree08" },
            { 0x08, "Tree09" }, { 0x09, "Tree10" }, { 0x0a, "Tree11" },
            { 0x0b, "Tree12" }, { 0x0c, "Tree13" },
            // Grass (0x14..0x19 → Grass01..Grass06)
            { 0x14, "Grass01" }, { 0x15, "Grass02" }, { 0x16, "Grass03" },
            { 0x17, "Grass04" }, { 0x18, "Grass05" }, { 0x19, "Grass06" },
            // Mushrooms not distributed as BMDs (only OZJ texture)
            // Stones
            { 0x20, "Stone01" }, { 0x21, "Stone02" }, { 0x22, "Stone03" },
            { 0x23, "Stone04" }, { 0x24, "Stone05" },
            // Statues / Tomb
            { 0x28, "StoneStatue01" }, { 0x29, "StoneStatue02" },
            { 0x2a, "StoneStatue03" },
            { 0x2c, "Tomb01" }, { 0x2d, "Tomb02" }, { 0x2e, "Tomb03" },
            // Fire / Light
            { 0x32, "FireLight01" }, { 0x33, "FireLight02" },
            { 0x34, "Bonfire01" },
            // Merchant animals
            { 0x38, "MerchantAnimal01" }, { 0x39, "MerchantAnimal02" },
            // Doungeon / drum / chest / ship
            { 0x37, "DoungeonGate01" },
            { 0x3a, "TreasureDrum01" }, { 0x3b, "TreasureChest01" },
            { 0x3c, "Ship01" },
            // Steel barred wall variants → SteelWall + SteelDoor in distrib
            { 0x41, "SteelWall01" }, { 0x42, "SteelWall02" },
            { 0x43, "SteelWall03" }, { 0x44, "SteelDoor01" },
            // Town walls → StoneWall in distrib (no plain Wall*.bmd)
            { 0x45, "StoneWall01" }, { 0x46, "StoneWall02" }, { 0x47, "StoneWall03" },
            { 0x48, "StoneWall04" }, { 0x49, "StoneWall05" }, { 0x4a, "StoneWall06" },
            // Castle/city walls → StoneMuWall in distrib (only 4 exist)
            { 0x4b, "StoneMuWall01" }, { 0x4c, "StoneMuWall02" },
            { 0x4d, "StoneMuWall03" }, { 0x4e, "StoneMuWall04" },
            // Bridge / fence
            { 0x50, "Bridge01" }, { 0x55, "BridgeStone01" },
            { 0x51, "Fence01" }, { 0x52, "Fence02" },
            { 0x53, "Fence03" }, { 0x54, "Fence04" },
            // Streetlight / cannon
            { 0x5a, "StreetLight01" },
            { 0x5b, "Cannon01" }, { 0x5c, "Cannon02" }, { 0x5d, "Cannon03" },
            { 0x5f, "Curtain01" },
            // Signboard → Sign in distrib
            { 0x60, "Sign01" }, { 0x61, "Sign02" },
            // Carriage
            { 0x62, "Carriage01" }, { 0x63, "Carriage02" },
            { 0x64, "Carriage03" }, { 0x65, "Carriage04" },
            // Straw / waterspout (Jar01..04 not distributed)
            { 0x66, "Straw01" }, { 0x67, "Straw02" },
            { 0x69, "Waterspout01" },
            // Hanging / stair
            { 0x6e, "Hanging01" }, { 0x6f, "Stair01" },
            // Houses (0x73..0x77 → House01..House05)
            { 0x73, "House01" }, { 0x74, "House02" }, { 0x75, "House03" },
            { 0x76, "House04" }, { 0x77, "House05" },
            { 0x78, "Tent01" },
            // House walls
            { 0x79, "HouseWall01" }, { 0x7a, "HouseWall02" }, { 0x7b, "HouseWall03" },
            { 0x7c, "HouseWall04" }, { 0x7d, "HouseWall05" }, { 0x7e, "HouseWall06" },
            // House etc
            { 0x7f, "HouseEtc01" }, { 0x80, "HouseEtc02" }, { 0x81, "HouseEtc03" },
            // Light / posebox per IDA AccessModel slots
            { 0x82, "Light01" }, { 0x83, "Light02" }, { 0x84, "Light03" },
            { 0x85, "PoseBox01" },
            // Furniture lives in 0x8c..0x92 in the original Object1 table
            { 0x8c, "Furniture01" }, { 0x8d, "Furniture02" }, { 0x8e, "Furniture03" },
            { 0x8f, "Furniture04" }, { 0x90, "Furniture05" }, { 0x91, "Furniture06" },
            { 0x92, "Furniture07" },
            // Beer / Candle / Bonfire
            { 0x96, "Candle01" },
            { 0x97, "Beer01" }, { 0x98, "Beer02" }, { 0x99, "Beer03" },
            // Bird/Fish/Butterfly handled by switch case 0 explicit calls above
        };
        for (size_t i = 0; i < sizeof(lorenciaSlots)/sizeof(lorenciaSlots[0]); ++i) {
            int slotIdx = lorenciaSlots[i].slot;
            char* slotPtr = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * slotIdx);
            if (*(short*)(slotPtr + 0x22) > 0) continue;  // already loaded (e.g. SMD worked)
            FUN_005060b0(slotIdx, "Data/Object1/", lorenciaSlots[i].bmd, -1);
            // Post-load verification log
            short nMesh   = *(short*)(slotPtr + 0x24);
            short nAction = *(short*)(slotPtr + 0x22);
            short nBone   = *(short*)(slotPtr + 0x26);
            char dbg[160];
            _snprintf_s(dbg, sizeof(dbg), _TRUNCATE,
                "Lorencia BMD slot=0x%02x '%s.bmd' nMesh=%d nAction=%d nBone=%d %s",
                slotIdx, lorenciaSlots[i].bmd, (int)nMesh, (int)nAction, (int)nBone,
                (nMesh > 0 ? "OK" : "FAIL"));
            DbgLogPublic(dbg);
        }

        // Register all Object1 slots 0..0x9f
        for (int i = 0; i < 0xa0; i++)
            FUN_00505c80(i, "Object1/", 0x2600, '\x01');
    } else {
        // Dynamic map: load from per-map object file
        if (DAT_0055a7c4 == '\0') {
            char local_384[32];
            crt_sprintf(local_384, "Data2/Object%d/", DAT_0055a7ac + 1);
            DAT_083a40fc = fopen(local_384, "rt");
            if (DAT_083a40fc != nullptr) {
                char local_300[256], local_200[256], local_100[256];
                while (FUN_0050e2c0() != 2) {
                    int objIdx = _DAT_083a40f4;
                    FUN_0050e2c0(); strncpy(local_300, DAT_083a3ff4, 255);
                    FUN_0050e2c0(); strncpy(local_200, DAT_083a3ff4, 255);
                    FUN_0050e2c0(); strncpy(local_100, DAT_083a3ff4, 255);
                    char pathBuf[32];
                    crt_sprintf(pathBuf, "Data2/Object%d/", DAT_0055a7ac + 1);
                    FUN_00505e90(objIdx, pathBuf, local_300);
                }
                fclose(DAT_083a40fc);
            }
        }
        // Register data paths for all 0xa0 object slots
        char local_384[32];
        crt_sprintf(local_384, "Data/Object%d/", DAT_0055a7ac + 1);
        for (int i = 0; i < 0xa0; i++)
            FUN_005060b0(i, local_384, "Object", i + 1);
        FUN_00505bd0(0x2ee);
        crt_sprintf(local_384, "Object%d/", DAT_0055a7ac + 1);
        for (int i = 0; i < 0xa0; i++)
            FUN_00505c80(i, local_384, 0x2600, '\x01');
        // Map-specific post-load fixups
        if (DAT_0055a7ac == 1)
            *(unsigned int *)(*(int *)(DAT_05828d58 + 0x1d90) + 0x14) = 0x3ecccccd; // 0.4f
        else if (DAT_0055a7ac == 8) {
            *(unsigned char *)(DAT_05828d58 + 0x89c)  = 0;
            *(unsigned char *)(DAT_05828d58 + 0x958)  = 0;
            *(unsigned char *)(DAT_05828d58 + 0xa14)  = 0;
            *(unsigned char *)(DAT_05828d58 + 0x3624) = 0;
            *(unsigned char *)(DAT_05828d58 + 0x379c) = 0;
            *(unsigned char *)(DAT_05828d58 + 0x3a8c) = 0;
        }
    }

    if (DAT_083a410c != '\0')
        DAT_0055a7c4 = cVar2;
}

// Font helpers (called from FUN_0050f690 in stubs.cpp)
// FUN_0043f2d0 @ 0x0043F2D0 — Pathfinder_Reset: frees + re-initialises A* context at DAT_05826df4.
// Despite the "Font_Reset" comment in functions.h, this is clearly the A* grid init.
void __cdecl FUN_0043f2d0(void) {
    DWORD *puVar4 = (DWORD*)DAT_05826df4;
    if ((void*)puVar4[0xff]  != nullptr) { operator_delete((void*)puVar4[0xff]);  puVar4[0xff]  = 0; }
    if ((void*)puVar4[0x102] != nullptr) { operator_delete((void*)puVar4[0x102]); puVar4[0x102] = 0; }
    if ((void*)puVar4[0x103] != nullptr) { operator_delete((void*)puVar4[0x103]); puVar4[0x103] = 0; }
    if ((void*)puVar4[0x104] != nullptr) { operator_delete((void*)puVar4[0x104]); puVar4[0x104] = 0; }
    puVar4[0x100] = 1950000000;
    puVar4[0x101] = 0xffffffff;
    puVar4[0]     = 0x100;
    puVar4[1]     = 0x100;
    puVar4[3]     = (DWORD)&DAT_0838bc70;
    puVar4[2]     = 0x10000;
    puVar4[0xff]  = (DWORD)operator_new(0x10000);
    puVar4[0x102] = (DWORD)operator_new(puVar4[2] << 2);
    puVar4[0x103] = (DWORD)operator_new(puVar4[2] << 2);
    void *pvVar1  = operator_new(puVar4[2] << 2);
    DWORD uVar3   = puVar4[2];
    puVar4[0x104] = (DWORD)pvVar1;
    DWORD *p      = (DWORD*)puVar4[0xff];
    for (DWORD i = uVar3 >> 2; i != 0; i--) { *p = 0; p++; }
    for (DWORD r = uVar3 &  3; r != 0; r--) { *(BYTE*)p = 0; p = (DWORD*)((BYTE*)p + 1); }
}
// FUN_0050f5f0 — implemented in src/Render/Font_Layout.cpp
// FUN_0040f570 @ 0x0040F570 — Font_BuildCharMap(this, type, dc)
// Creates a font rendering object: if type==1, allocates a 0x2C4-byte widget (FUN_0040f730),
// otherwise allocates a 4-byte simple widget (FUN_00410a90). Stores at this+4,
// sets this+8 = type, then calls the vtable's first virtual method with dc.
void __cdecl FUN_0040f570(int self, int param_1, int param_2)
{
    void* pObj = NULL;
    if (param_1 == 1) {
        void* mem = operator_new(0x2C4);
        if (mem) {
            pObj = (void*)FUN_0040f730(mem);
        }
    } else {
        int* mem = (int*)operator_new(4);
        if (mem) {
            FUN_00410a90(mem);
            pObj = (void*)mem;
        }
    }
    *(void**)(self + 4) = pObj;
    *(int*)(self + 8) = param_1;
    // Call vtable method 0: (*(code**)*pObj)(param_2)
    if (pObj) {
        typedef void (__cdecl *VtblFn)(int);
        VtblFn fn = *(VtblFn*)(*(DWORD*)pObj);
        if (fn) fn(param_2);
    }
}

// Model data loaders — implemented in src/Model/
// FUN_00506170 — implemented in src/Model/Model_Items.cpp
// FUN_00507610 — implemented in src/Model/Model_Monsters.cpp
// FUN_005079d0 — implemented in src/Model/Model_Players.cpp
// FUN_00508d10 — implemented in src/Model/Model_Effects.cpp
// FUN_0050b710 — implemented in src/Model/Model_Misc.cpp
// FUN_0050eb80 — implemented in src/Model/Model_SkillEffects.cpp
// FUN_0050f030 — implemented in src/Model/Model_Gates.cpp
// Item data loaders — implemented in src/Item/
// FUN_0047b130 — implemented in src/Item/Item_Data.cpp    (Item_LoadData)
// FUN_0047b650 — implemented in src/Item/Item_Data.cpp    (Item_SaveBMD)
// FUN_0047b740 — implemented in src/Item/Item_Data.cpp    (Item_LoadBMD)
// FUN_0047a5b0 — implemented in src/Item/Skill_Data.cpp   (Skill_LoadData)
// FUN_0047a970 — implemented in src/Item/Skill_Data.cpp   (Skill_SaveBMD)
// FUN_0047ac50 — implemented in src/Item/Skill_Data.cpp   (Skill_LoadBMD)
// FUN_0047a010 — implemented in src/Item/Gate_Data.cpp    (Gate_LoadData)
// FUN_0047a170 — implemented in src/Item/Gate_Data.cpp    (Gate_SaveBMD)
// FUN_0047a4d0 — implemented in src/Item/Gate_Data.cpp    (Gate_LoadBMD)
// FUN_004799d0 — implemented in src/Item/Filter_Data.cpp  (Filter_LoadData)
