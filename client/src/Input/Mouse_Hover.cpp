// Mouse_Hover.cpp — IDA: FUN_004b0310 — Mouse_UpdateHoverTargets
// Per-frame mouse cursor billboard render + hover target determination.
//
// Called every frame. Two responsibilities:
//   1. Render the cursor as a 2D billboard sprite.
//   2. Determine which entity/item is under the cursor and store hover targets.
//
// ── Cursor billboard ──────────────────────────────────────────────────────────
// RenderTerrainTile(DAT_080ab288, DAT_080ab28c, screenX, screenY, 1.0f, 1, 1):
//   Returns nonzero if cursor is visible/active.
// If visible, calls GL_SetBlendAdditive() (hide char anim sprite for cursor area),
// then RenderTerrainAlphaBitmap(type=8, x, y, sx, sy, color, 0, alpha) to draw the quad.
//   - States 2/4/5 (login/charselect/ingame): fixed size based on DAT_07e11d5c
//   - States 1/3 (intro/loading): animated size using DAT_07e11d5c oscillation
//
// ── Hover targets ─────────────────────────────────────────────────────────────
// After cursor render, sets hover indices:
//   SelectedItem = item on ground (-1=none)
//   SelectedNpc = NPC / shop entity (-1=none)
//   SelectedCharacter = mob or player entity (-1=none)
//   SelectedOperate = special object (-1=none)
//   Attacking = secondary hover (cleared if SelectedCharacter resets)
//
// Priority with Alt held (VK_MENU):
//   item-ground (ItemOnGround_HoverTest, IDA: FUN_004afa40) → NPC type 4 → mob type 0x22 → player type 1 → special
//
// Priority without Alt + swim/idle:
//   mob/player type 0x22 → type 1 → NPC type 4 → item-ground → special
//
// HashTable section (lines ~131-350 in original):
//   Anti-tamper XOR encode/decode block operating on DAT_07cf1ffc (0x584-byte
//   char-data buffer). Per CLAUDE.md policy, hash table operations are
//   reference-count obfuscation — not game logic. Omitted from implementation.
//
// After hover detection: if hover target found and Attacking != -1,
//   calls FUN_004afb00() to process the pending click action.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <windows.h>

extern "C" void DbgLogPublic(const char* msg);


// ─────────────────────────────────────────────────────────────────────────────

// IDA: FUN_004b0310
void Mouse_UpdateHoverTargets(void)
{
    // ── 1. Cursor billboard render ────────────────────────────────────────────
    // 2026-04-29 DISABLED: el cursor billboard 3D (sprite en el suelo del tile
    // hovered) requiere DAT_07eab24c (BackTerrainHeight) que no se inicializa
    // en nuestro port. Crash AV en FUN_004f8740 al acceder al buffer null.
    // El cursor 2D (Cursor_Render) sigue funcionando normalmente.
    #if 0
    if (SceneFlag == 2 || SceneFlag == 4 || SceneFlag == 5)
    {
        float color[3] = { 1.0f, 0.766f, 0.0f };
        char visible = RenderTerrainTile(DAT_080ab288, DAT_080ab28c, 0, 0, 1.0f, 1, 1);
        if (visible != '\0') {
            GL_SetBlendAdditive();
            int frame = (SceneFlag == 2) ? 1 : (DAT_07e11d5c + 1);
            float sz = (float)frame;
            RenderTerrainAlphaBitmap(8, DAT_083a4130, DAT_083a4134, sz, sz, color, 0, 1.0f);
        }
    }
    if (SceneFlag == 1 || SceneFlag == 3) {
        float color[3] = { 1.0f, 0.766f, 0.0f };
        char visible = RenderTerrainTile(DAT_080ab288, DAT_080ab28c, 0, 0, 1.0f, 1, 1);
        if (visible != '\0') {
            GL_SetBlendAdditive();
            float base = (float)DAT_07e11d5c + (float)DAT_07e11d5c + _DAT_0055256c;
            float szX = ((int)base / 100) * 100 + _DAT_00552598;
            float szY = ((int)base / 100) * 100 + _DAT_00552598;
            RenderTerrainAlphaBitmap(8, szX, szY, base, base, color, 0, 1.0f);
        }
    }
    #endif

    // ── 2. Reset hover targets ────────────────────────────────────────────────

    // 2026-05-06: añadido guard `c50 >= 0` para evitar OOB read cuando
    // SelectedCharacter == -1 (initial state). Antes se leía entity[+0x2fd] con
    // c50=-1 → puntero negativo → crash latente.
    if (m_bAutoAttack == '\0' || World == 6) {
        // Cursor disabled or spectator state
        SelectedCharacter = -1;
        Attacking = -1;
    } else if (SelectedCharacter >= 0 &&
               *(char *)(DAT_07abf5d0 + 0x2fd + SelectedCharacter * 0x394) == '\0' &&
               *(char *)(DAT_07abf5d0 + SelectedCharacter * 0x394 + 0x84) == '\x02') {
        // Current hover target is a valid alive monster.
        // 2026-05-06: REMOVED reset on IsClickPushed/DAT_083a42c4. La lógica
        // original IDA reseteaba aquí porque la detect que sigue inmediato
        // re-poblaría. Pero en nuestro port el detect a veces falla (terrain
        // filter, etc) → c50 quedaba -1 al click time → mob attack handler
        // no disparaba. Mantener el target HASTA que detect lo reemplace.
        // Keep the original transient hover state.  Leaving this selected
        // after the cursor moves away turns later ground clicks into a basic
        // attack against the stale mob.
        // IDA sub_4B0310 L85-106 — el bloque de reset completo:
        //   if ( !m_bAutoAttack || World == 6 )      { SelectedCharacter = -1; Attacking = -1; }
        //   else if ( !target->Dead && target->Kind == 2 )
        //   {
        //       if ( Attacking == -1 || MouseLButton || MouseLButtonPush
        //         || MouseRButton || MouseRButtonPush || Hero->Dead )
        //           SelectedCharacter = -1;
        //   }
        //   else { Attacking = -1; SelectedCharacter = -1; }
        //
        // El clear NO es destructivo: el detect que viene justo despues
        // (`if (SelectedCharacter == -1) SelectedCharacter = sub_4AFDC0(...)`,
        // L326) lo vuelve a poblar con lo que haya bajo el cursor.  Por eso el
        // objetivo sigue al mouse en el original.
        //
        // El port tenia SOLO los dos flags del boton DERECHO
        // (DAT_083a42ac / MouseRButtonPush), asi que clickeando con el IZQUIERDO el
        // target nunca se limpiaba: quedaba pegado el primer mob que hubiera
        // pasado por debajo del cursor.  Direcciones confirmadas con
        // ida_xrefs_to:  MouseLButton = 0x083A42C4 · MouseLButtonPush = 0x083A4124
        //                MouseRButton = 0x083A42AC · MouseRButtonPush = 0x083A42D0
        //                m_bAutoAttack = 0x00559C5C · Attacking = 0x00559C58
        if (Attacking == -1 ||
            DAT_083a42c4 != '\0' || DAT_083a4124 != 0 ||
            DAT_083a42ac != '\0' || MouseRButtonPush != '\0' ||
            *(char *)(DAT_07abf5d8 + 0x2fd) != '\0')
        {
            SelectedCharacter = -1;
        }
    } else if (SelectedCharacter >= 0) {
        // Current target is no longer valid (died or kind changed).
        Attacking = -1;
        SelectedCharacter = -1;
    }

    SelectedItem = -1;
    SelectedNpc = -1;
    SelectedOperate = -1;

    // ── 3. Hover target detection ─────────────────────────────────────────────

    if (DAT_07d78094 != '\0') goto process_click;

    {
        SHORT altState = GetAsyncKeyState(0x12);  // VK_MENU = Alt
        if ((char)((unsigned short)altState >> 8) == (char)0x80)
        {
            // Alt held: item-on-ground first, then NPC, mob, player
            if (DAT_07e91388 == 0)
                SelectedItem = ItemOnGround_HoverTest();
            if (SelectedItem == -1) {
                SelectedNpc = Entity_SelectNearest(4);
                if (SelectedNpc == -1) {
                    SelectedCharacter = Entity_SelectNearest(0x22);
                    if (SelectedCharacter != -1) goto done;
                    SelectedCharacter = Entity_SelectNearest(1);
                    if (SelectedCharacter != -1) goto done;
                    SelectedOperate = SpecialObject_HoverTest();
                }
                goto check_click;
            }
            goto process_click;
        }

        // No Alt: check swim/idle move state
        unsigned char moveFlags = *(unsigned char *)(DAT_07abf5d8 + 0x1bc) & 7;
        if (moveFlags == 2 || moveFlags == 0)
        {
            // HashTable encode/decode block — anti-tamper obfuscation.
            // Operates on DAT_07cf1ffc (XOR-encoded char data, 0x584 bytes).
            // Omitted per project policy (hash table operations are not game logic).
        }

        // Fallthrough to secondary hover scan
        {
            // Item range check (iStack_10 between 0x19 and 0x1c — from hash table result)
            // local_20 / local_1c adjust entity type priority based on equip slot range
        }

        // ── Secondary hover without second password ───────────────────────────
        if (FUN_004e5980() == '\0') {
            // IDA sub_4B0310 L315-351 (Alt SIN apretar): cadena de descarte
            // estricta, personaje -> personaje -> NPC -> ITEM -> mobiliario.  El
            // item solo se elige si el cursor no esta sobre ningun personaje ni NPC.
            //
            // 2026-09-21: aca habia una inversion puesta el 2026-07-27 que miraba
            // el item PRIMERO, porque "cualquier mob cercano en pantalla robaba el
            // hover".  Esa causa desaparecio el 2026-09-16 (2f83d26): desde ahi
            // Entity_SelectNearest usa el rayo contra la OBB, como IDA, y solo
            // elige al que esta realmente bajo el cursor.  La inversion quedo
            // compensando un problema que ya no existia, y su efecto era el
            // reporte del tester: con un item debajo del monstruo el cursor
            // quedaba en el de levantar en vez del de ataque (RenderCursor le da
            // prioridad a SelectedItem).  Con Alt APRETADO los items si van
            // primero -- esa rama de arriba es la de IDA y no se toca.
            //
            // Orden de los dos tipos de personaje (IDA L117-118 y L318-322): por
            // defecto monstruos (0x22) y despues jugadores (1); con un buff de
            // elfa activo (skills 26-28: curar, mas defensa, mas dano) se invierte,
            // para poder apuntarle a un jugador que tiene un monstruo detras.
            int firstKind = 0x22, secondKind = 1;
            if (DAT_07abf5d8 && CharacterAttribute) {
                const BYTE slot  = *(BYTE*)((BYTE*)DAT_07abf5d8 + 913);
                const BYTE skill = ((BYTE*)CharacterAttribute)[87 + slot];
                if (skill >= 26 && skill <= 28) { firstKind = 1; secondKind = 0x22; }
            }
            if (SelectedCharacter == -1) {
                SelectedCharacter = Entity_SelectNearest(firstKind);
                if (SelectedCharacter == -1) {
                    SelectedCharacter = Entity_SelectNearest(secondKind);
                    if (SelectedCharacter != -1) goto done;
                    SelectedNpc = Entity_SelectNearest(4);  // NPC
                    if (SelectedNpc == -1) {
                        if (DAT_07e91388 == 0)
                            SelectedItem = ItemOnGround_HoverTest();
                        if (SelectedItem == -1)
                            SelectedOperate = SpecialObject_HoverTest();
                    }
                    goto check_click;
                }
            }
            if (Attacking != -1) goto process_click;
        }
        else goto process_click;
    }

check_click:
    if (SelectedCharacter == -1) {
        Attacking = -1;
    }
    goto done;

process_click:
    Party_MatchEntityNames();

done:
    if (SelectedCharacter == -1)
        Attacking = -1;

}

// ── Additional helpers extracted from stubs_mouse_hover.cpp ─────────────────
// ── Mouse hover helpers ────────────────────────────────────────────────────────
// RenderTerrainTile @ 0x004F8480 — Terrain_TilePick(x,y,row,col,unused,stride,flag)
// Stores world coords + tile index, optionally renders a debug outline (GL_LINE_STRIP).
// For state 3 (combat target select), draws a filled quad and does mouse-ray intersection.
// Returns 1 if mouse ray intersects tile, 0 otherwise.
// IDA: RenderTerrainTile (0x004F8480)
int __cdecl RenderTerrainTile(int iparam_1, int iparam_2, int param_3, int param_4, float param_5, int param_6, int param_7) {
    float param_1 = *(float*)&iparam_1;
    float param_2 = *(float*)&iparam_2;
    *(float*)&DAT_07feb258 = param_1 * _DAT_005524f0;
    *(float*)&DAT_07feb25c = param_2 * _DAT_005524f0;
    DAT_07eab1ec = param_4 * 0x100 + param_3;
    DAT_07eab1f0 = DAT_07eab1ec + param_6;
    *(float*)&DAT_07feb260 = (float)DAT_080cb2cc[DAT_07eab1ec];
    _DAT_07feb264 = *(float*)&DAT_07feb258 + _DAT_005524f0;
    DAT_07eab1f8 = (param_4 + param_6) * 0x100 + param_3;
    DAT_07eab1f4 = param_6 + DAT_07eab1f8;
    _DAT_07feb26c = (float)DAT_080cb2cc[DAT_07eab1f0];
    _DAT_07feb274 = *(float*)&DAT_07feb25c + _DAT_005524f0;
    _DAT_07feb278 = (float)DAT_080cb2cc[DAT_07eab1f4];
    _DAT_07feb284 = (float)DAT_080cb2cc[DAT_07eab1f8];
    _DAT_07feb268 = *(float*)&DAT_07feb25c;
    _DAT_07feb270 = _DAT_07feb264;
    _DAT_07feb27c = *(float*)&DAT_07feb258;
    _DAT_07feb280 = _DAT_07feb274;
    if (param_7 == 0) {
        if (((unsigned char)DAT_0838bc70[DAT_07eab1ec] & 8) != 8)
            RenderTerrainFace(param_1, param_2, param_3, param_4, param_5);   // RenderTerrainFace(xf,yf,xi,yi,lodf)
        return 0;
    }
    char cVar1 = '\0';

    // [DIAG] one-shot per ~half second to see camera/ray
    {
        static DWORD s_lastTilePick = 0;
        DWORD now = GetTickCount();
        if (now - s_lastTilePick > 500) {
            s_lastTilePick = now;
            float* eye = (float*)&CameraRayOriginX_arr[0];
            float* tgt = (float*)&DAT_083a4110_arr[0];
            char d[256];
            wsprintfA(d, "TilePick(%d,%d) eye=(%d,%d,%d) tgt=(%d,%d,%d)",
                param_3, param_4,
                (int)eye[0], (int)eye[1], (int)eye[2],
                (int)tgt[0], (int)tgt[1], (int)tgt[2]);
            DbgLogPublic(d);
        }
    }
    if (DAT_07e11d30 != 5) {
        GL_SetAlphaTest('\0');
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(3);
        // BUG-FIX 2026-04-28: bound era 0x7feb288 (addr abs del binario original).
        // Pool real es g_TilePickBuf[12] = 4 vec3 corners. Iterar 4.
        for (int i = 0; i < 4; ++i) {
            glVertex3fv(&g_TilePickBuf[i * 3]);
        }
        glEnd();
        GL_ResetState();
    }
    float local_c[3];
    Triangle_ComputeNormal((float*)&DAT_07feb258, &_DAT_07feb264, &_DAT_07feb270, local_c);
    unsigned int uVar2 = FUN_00512d40((float*)&CameraRayOriginX, (float*)&DAT_083a4110, 3,
                                       (float*)&DAT_07feb258, &_DAT_07feb264, &_DAT_07feb270,
                                       &_DAT_07feb27c, local_c, '\x01');
    cVar1 = (char)uVar2;
    if (cVar1 == '\0') {
        Triangle_ComputeNormal((float*)&DAT_07feb258, &_DAT_07feb270, &_DAT_07feb27c, local_c);
        uVar2 = FUN_00512d40((float*)&CameraRayOriginX, (float*)&DAT_083a4110, 3,
                              (float*)&DAT_07feb258, &_DAT_07feb270, &_DAT_07feb27c,
                              &_DAT_07feb264, local_c, '\x01');
        cVar1 = (char)uVar2;
    }
    if (cVar1 == '\x01') {
        *(float*)&DAT_080ab288 = param_1;
        *(float*)&DAT_080ab28c = param_2;
        DAT_07eab1fc = 1;
    }
    if (DAT_07e11d30 == 3) {
        bool draw = false;
        if (DAT_07e11d44 == 0) {
            if (((unsigned char)DAT_0838bc70[DAT_07eab1ec] & 4) == 4) draw = true;
        } else if (DAT_07e11d44 == 2) {
            if (((unsigned char)DAT_0838bc70[DAT_07eab1ec] & 1) == 1) draw = true;
        }
        if (draw) {
            GL_DisableDepthTest();
            GL_SetBlendSrcOver('\x01');
            GL_SetAlphaTest('\0');
            glBegin(6);
            glColor4f(1.0f, 0.0f, 0.0f, 0.3f);
            // BUG-FIX 2026-05-03: was `while (puVar3 < 0x7feb288)` — absolute
            // source-binary bound, junk in our build. g_TilePickBuf[12] holds
            // exactly 4 vec3 corners (matching the lines 924 fix above).
            for (int i = 0; i < 4; ++i) {
                glVertex3fv(&g_TilePickBuf[i * 3]);
            }
            glEnd();
            GL_ResetState();
        }
    }
    (void)param_5;
    return (int)(unsigned char)cVar1;
}

// FUN_00512d30 @ 0x00512D30 — Map_InitRayCast: init ray t_max to ~1.03e7
// IDA-ported: single store. DAT_083a4120 is the raycast t_max sentinel
// used by FUN_00512d40 (CollisionDetectLineToFace) to accept nearer hits only.
// Original binary stores raw bits 0x4B1DCD65 (= 10367333.0f) into the float.
void FUN_00512d30()
{
    *(DWORD*)&DAT_083a4120 = 0x4B1DCD65;  // ≈ 1.0367e7f — far-plane sentinel
}


// Entity_SelectNearest @ 0x004AFDC0
// Two-pass entity scan: (1) sets +0x58 visible-flag and +0x64..6f RGB tint per entity type;
// (2) finds the nearest entity (matching mask bits at +0x84) to the camera and syncs
// party HP bar ID arrays.  Returns the entity index of the nearest match, or -1.
// IDA: FUN_004afdc0
int __cdecl Entity_SelectNearest(int param_1_int)
{
    byte param_1 = (byte)param_1_int;
    bool bVar17 = (SceneFlag == 4);  // SceneFlag == CharSelect

    // Pass 1: set highlight flags and color tints for visible entities
    byte *pbVar4 = (byte *)(DAT_07abf5d0 + 0x84);
    for (int loop1 = 400; loop1 != 0; loop1--, pbVar4 += 0x394) {
        if (((*pbVar4 & param_1) == *pbVar4) && (pbVar4[-0x84] != 0) && (pbVar4[0xdc] != 0) &&
            (_DAT_00552580 < *(float *)(pbVar4 + 0xe4))) {
            pbVar4[0x58] = 1;
            float r, g, b;
            switch (*(short *)(pbVar4 + 0x13a)) {
            case 0:  r =  0.2f; g =  0.2f; b =  0.2f; break;
            case 1:  r = -0.4f; g = -0.4f; b = -0.4f; break;
            case 2:  r =  0.2f; g = -0.6f; b = -0.6f; break;
            case 3:  r =  1.5f; g =  1.5f; b =  1.5f; break;
            case 4:  r =  0.3f; g =  0.2f; b = -0.5f; break;
            default: r =  0.0f; g =  0.0f; b =  0.0f; break;
            }
            if (pbVar4[0x266] > 5) { r = -0.4f; g = -0.4f; b = -0.4f; }
            *(float *)(pbVar4 + 0x64) = r;
            *(float *)(pbVar4 + 0x68) = g;
            *(float *)(pbVar4 + 0x6c) = b;
        }
    }

    // Pass 2: find nearest entity to MOUSE-RAY (perpendicular distance), not camera.
    // Antes: usábamos distancia a cámara con FUN_00513260 stub → siempre return 1
    // → ganaba el más cercano a cámara siempre, que es slot 1 (elfa) por geometría.
    // Ahora: gana el char cuyo centro de masa está más cerca del ray del mouse.
    float best_perp = 1e12f;
    int   best_idx  = -1;
    int   ent_idx   = 0;
    // DIAG: rate-limited per slot, log filter rejection reasons
    static DWORD s_lastFilt[5] = {0,0,0,0,0};
    bool diagFilt = (SceneFlag == 4);

    for (int ofs = 0; ofs < 0x59740; ofs += 0x394, ent_idx++) {
        char *ent = (char *)(ofs + DAT_07abf5d0);
        if (diagFilt && ent_idx < 5) {
            DWORD now = GetTickCount();
            if (now - s_lastFilt[ent_idx] > 1000) {
                s_lastFilt[ent_idx] = now;
                char b[200];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "FILT slot=%d mask=%d act=%d vis@160=%d dist@168=%.3f thr@552580=%.3f flag@2fd=%d flag@84=%d",
                    ent_idx, (int)param_1, (int)*ent, (int)ent[0x160],
                    *(float*)(ent+0x168), _DAT_00552580,
                    (int)ent[0x2fd], (int)ent[0x84]);
                DbgLogPublic(b);
            }
        }
        if ((*ent == '\0') || (ent[0x160] == '\0') ||
            (!(_DAT_00552580 < *(float *)(ent + 0x168))) || (ent[0x2fd] != '\0'))
            continue;

        // Party name matching for DarkLord entities
        if ((*(short *)(ent + 2) == 0x186) && (ent[0x84] == '\x01') && (0 < (int)DAT_07eaa0e0)) {
            char *slot = (char *)&DAT_07e11e80;
            for (int pi = 0; pi < (int)DAT_07eaa0e0; pi++, slot += 0x24) {
                if (*(int *)(slot + 0x1c) == -2) {
                    const char *eName = ent + 0x1c1;
                    size_t sVar7 = strlen(eName);
                    size_t m     = strlen(slot);
                    if (m > sVar7) sVar7 = m;
                    if (sVar7 == 0) sVar7 = 1;
                    *(int *)(slot + 0x20) = 0;
                    if (strncmp(slot, eName, sVar7) == 0) {
                        *(int  *)(slot + 0x1c) = ent_idx;
                        *(DWORD *)(slot + 0x20) = *(DWORD *)(ent + 0x78);
                        break;
                    }
                }
            }
        }

        // Visibility + nearest check
        if ((!bVar17) && (ent == (char *)DAT_07abf5d8)) continue;
        if ((ent[0x84] & param_1) != ent[0x84]) continue;

        // IDA sub_4AFDC0 L38-52: test del RAYO del mouse contra la OBB de la
        // entidad (o+0x130, 12 floats que llena Calc_RenderObject) con
        // sub_513260, y gana la MAS CERCANA A LA CAMARA.
        //
        // 2026-09-16: aca habia una reimplementacion en pantalla (gluProject de
        // tres puntos a 10/40/70 de altura sobre los pies y un radio fijo de 32
        // px).  Apuntando a la parte alta del cuerpo, o con el mob inclinado en
        // su animacion, el cursor quedaba fuera de esos circulos y el click caia
        // al suelo (SelectedCharacter = -1): los "clicks que no atacan".  El
        // motivo por el que se habia reemplazado (FUN_00513260 era un stub que
        // devolvia 1) ya no aplica: quedo portado el 2026-09-04.
        {
            float box[12];
            memcpy(box, (const void*)(ent + 0x130), sizeof(box));
            if (!FUN_00513260((float*)&CameraRayOriginX, (float*)&DAT_083a4110, box))
                continue;

            const float dy = *(float*)(ent + 0x14) - _DAT_083a42d8;   // CameraPosition[1]
            const float dz = *(float*)(ent + 0x18) - _DAT_083a42dc;   // CameraPosition[2]
            const float dx = *(float*)(ent + 0x10) - _DAT_083a42d4;   // CameraPosition[0]
            const float d2 = dz * dz + dy * dy + dx * dx;
            if (!(d2 < best_perp)) continue;

            // Filtro de techos (IDA L~115-131): en Lorencia (World 0) una entidad
            // sobre un tile 4, y en Devias (World 2) sobre un tile 3, solo se
            // puede elegir si el heroe esta en ese mismo tipo de tile.
            // `World` es el indice de mapa (el macro `World` que lo
            // nombraba World mentia; la nota vieja que deshabilito este
            // filtro partia de esa etiqueta).
            const int map = (int)World;
            if (map == 0 || map == 2) {
                int tx = (int)*(float*)(ent + 0x10) / 100;
                int ty = (int)*(float*)(ent + 0x14) / 100;
                if (tx < 0) tx = 0; if (tx > 255) tx = 255;
                if (ty < 0) ty = 0; if (ty > 255) ty = 255;
                const unsigned char tile = TerrainMappingLayer1[tx + (ty << 8)];   // TerrainMappingLayer1
                const unsigned char roof = (map == 0) ? 4 : 3;
                if (tile == roof && (DWORD)tile != DAT_07e118e8)           // HeroTile
                    continue;
            }
            best_perp = d2;
        }

        best_idx  = ent_idx;
    }

    // Pass 3: sync party HP bar entity IDs by player name
    if (0 < (int)DAT_07eaa0e0) {
        int *piVar12 = (int *)((char *)&DAT_07e11e80 + 0x1c);
        for (int pi = 0; pi < (int)DAT_07eaa0e0; pi++, piVar12 += 9) {
            if (*piVar12 < 0) {
                const char *playerName = (char *)DAT_07abf5d8 + 0x1c1;
                const char *partyName  = (char *)(piVar12 - 7);
                size_t n = strlen(playerName);
                size_t m = strlen(partyName);
                size_t sVar7 = (n < m) ? m : n;
                if (sVar7 == 0) sVar7 = 1;
                piVar12[1] = 0;
                int cmp = strncmp(partyName, playerName, sVar7);
                *piVar12 = (cmp != 0) ? -3 : -1;
            }
        }
    }
    return best_idx;
}

// ItemOnGround_HoverTest @ 0x004AFA40
// 2026-09-04: el encabezado decia "NEUTRALIZADO (2026-04-26)", pero eso quedo
// viejo -- la funcion se reimplemento el 2026-07-27 y anda (pickup confirmado en
// runtime).  Se conserva la nota historica porque explica la DESVIACION que sigue
// vigente:
//
//   El path fiel (sub_4AFA40) hace un test de rayo contra la OBB del item con
//   `sub_513260`; aca se usa proximidad world-space -- se compara el tile del item
//   con el tile del terreno bajo el mouse (el mismo picker del click-to-move,
//   RenderTerrain -> DAT_080ab288/28c).
//
//   El motivo que se anotaba para no portarlo ("FUN_00513260 depende de macros
//   Hex-Rays sin portar") YA NO APLICA: ese test quedo portado el 2026-09-04 al
//   arreglar el pick de objetos interactuables.  Si algun dia el hover de items se
//   comporta distinto al original, ese es el cambio a hacer -- pero hoy funciona y
//   tocarlo es riesgo sin beneficio reportado.
//
// El pool DAT_07e12840 es 1000x0x204; layout por slot (base = pool + i*0x204):
//   base+72   active flag
//   base+424  visible flag (lo setea el render)
//   base+16/20  world X/Y del item
//   base+304/308/312  light color (0.2 normal, 1.5 al hover)
// IDA: FUN_004afa40
int __cdecl ItemOnGround_HoverTest(void)
{
    // 2026-07-27: hover de items en el suelo. El path FIEL (sub_4AFA40) usa un
    // point-in-quad screen-space (FUN_00513260, 12-arg) que depende de macros
    // Hex-Rays sin portar. En su lugar usamos proximidad world-space: comparar
    // el tile del item con el tile del terreno bajo el mouse (el mismo picker
    // que usa el click-to-move, RenderTerrain → DAT_080ab288/28c).
    // El pool DAT_07e12840 es 1000×0x204; layout por slot (base = pool+i*0x204):
    //   base+72   active flag
    //   base+424  visible flag (lo setea el render)
    //   base+16/20  world X/Y del item
    //   base+304/308/312  light color (0.2 normal, 1.5 al hover)
    BYTE* pool = (BYTE*)&DAT_07e12840[0];

    // Pass 1: atenuar todos los items activos + visibles.
    for (int i = 0; i < 1000; ++i) {
        BYTE* base = pool + i * 0x204;
        if (base[72] && base[424]) {
            *(DWORD*)(base + 304) = 0x3E4CCCCD;   // 0.2f
            *(DWORD*)(base + 308) = 0x3E4CCCCD;
            *(DWORD*)(base + 312) = 0x3E4CCCCD;
        }
    }

    // Pass 2: item cuya posición de pantalla (v1+92/94, la computa el render con
    // World_ToScreen) esté cerca del cursor. Screen-space, sin picker de terreno
    // (que dibujaba la cuadrícula). Umbral generoso ~24 px (el modelo del item
    // es chico en el suelo).
    int mouseX = (int)DAT_083a427c;
    int mouseY = (int)DAT_083a4278;
    int best = -1, bestD = 24 * 24 + 1;
    for (int i = 0; i < 1000; ++i) {
        BYTE* base = pool + i * 0x204;
        if (base[72] && base[424]) {
            int sx = *(short*)(base + 72 + 92);   // = v1+92
            int sy = *(short*)(base + 72 + 94);
            int dx = sx - mouseX, dy = sy - mouseY;
            int d = dx * dx + dy * dy;
            if (d < bestD) { bestD = d; best = i; }
        }
    }
    if (best >= 0) {
        BYTE* base = pool + best * 0x204;
        *(DWORD*)(base + 304) = 0x3FC00000;   // 1.5f (highlight)
        *(DWORD*)(base + 308) = 0x3FC00000;
        *(DWORD*)(base + 312) = 0x3FC00000;
    }
    return best;
}

// SpecialObject_HoverTest @ 0x004B0240 (sub_4B0240)
// Pick de los objetos "operables" del mundo -- sillas, bancos, barandas y los
// orbes de Noria.  La lista la arma `sub_4FF580` desde `CreateObject` (200
// entradas de 12 bytes en DAT_083A2370: [0] activo, [2] puntero al objeto) y el
// indice que devuelve esta funcion va a `SelectedOperate`, que leen
// `RenderCursor` (para cambiar el cursor) y `Player_InputTick` (para encolar
// MOVEMENT_OPERATE = sentarse / apoyarse / flotar).
//
// Pasada 1: baja la luz de todos los operables visibles a 0.2.
// Pasada 2: el primero cuya OBB (objeto+0x130, la que deja Calc_RenderObject)
//           corte el rayo del mouse se ilumina a 1.5 y se devuelve su indice.
//
// 2026-09-04: estaba NEUTRALIZADO (`return -1`) desde 2026-04-26 porque el port
// original iteraba con el bound absoluto 0x83A2CD0 del binario fuente.  El array
// ya esta bien dimensionado en globals.cpp (0x960 = 200 x 12), asi que se acota
// con `sizeof`.  Mientras estuvo neutralizado NADA del mundo era interactuable.
int __cdecl SpecialObject_HoverTest(void)
{
    const int stride = 0xc;
    const int slots  = (int)(sizeof(DAT_083a2370) / stride);

    // Pasada 1 - apagar el resalte de todos.
    for (int i = 0; i < slots; ++i) {
        char *e   = &DAT_083a2370[i * stride];
        int   obj = *(int *)(e + 8);
        if (e[0] == 0 || obj == 0) continue;
        if (*(char *)(obj + 0x160) == 0) continue;   // no visible este frame
        *(DWORD *)(obj + 0xe8) = 0x3e4ccccd;             // 0.2f
        *(DWORD *)(obj + 0xec) = 0x3e4ccccd;
        *(DWORD *)(obj + 0xf0) = 0x3e4ccccd;
    }

    // Pasada 2 - el primero que corte el rayo del mouse.
    for (int i = 0; i < slots; ++i) {
        char *e   = &DAT_083a2370[i * stride];
        int   obj = *(int *)(e + 8);
        if (e[0] == 0 || obj == 0) continue;
        if (*(char *)(obj + 0x160) == 0) continue;

        float box[12];
        memcpy(box, (const void *)(obj + 0x130), sizeof(box));

        if (FUN_00513260((float *)&CameraRayOriginX, (float *)&DAT_083a4110, box)) {
            *(DWORD *)(obj + 0xe8) = 0x3fc00000;         // 1.5f -- resalte
            *(DWORD *)(obj + 0xec) = 0x3fc00000;
            *(DWORD *)(obj + 0xf0) = 0x3fc00000;
            return i;
        }
    }
    return -1;
}
// FUN_004afb00 — implemented in src/Game/Party_NameMatch.cpp (Party_MatchEntityNames)
// FUN_004e5980 @ 0x004E5980 — Party_HPBar_HoverCheck(void)
// Iterates the party HP bar array (DAT_07e11e9c, stride 0x24 = 9 uints) and checks
// if the mouse cursor (DAT_083a427c, DAT_083a4278) is within any party member's
// screen rect. Sets SelectedCharacter (hover entity index) and returns 1 if hovering.
// SecondPassword UI state flags (DAT_07eaa115..130) control which X position band is used.
// Anti-tamper HashTable blocks in the loop are skipped — only position comparison kept.
char __cdecl FUN_004e5980(void)
{
    if (DAT_07eaa115 != '\0') return 0;

    // Determine X band based on SecondPassword UI state
    UINT local_20;
    if (DAT_07eaa117 != '\0') {
        if (DAT_07eaa116 != '\0') {
            local_20 = 0x104;
        } else {
            // Check secondary password active flags
            if ((DAT_07eaa118 == '\0') && (DAT_07eaa119 == '\0') && (DAT_07eaa11a == '\0') &&
                (DAT_07eaa11b == '\0') && (DAT_07eaa11c == '\0')) {
                local_20 = 0x1c2;
            } else {
                local_20 = 0x104;
            }
        }
    } else {
        // All clear: check if all flags inactive
        if ((DAT_07eaa116 == '\0') && (DAT_07eaa115 == '\0') &&
            (DAT_07eaa114 == '\0') && (DAT_07eaa124 == 0) &&
            (GoldenArcherOpenType == 0) && (*(char*)((uintptr_t)DAT_00583d8c + 0x1c87f) == '\0') &&
            (ServerDivisionOpened == '\0')) {
            local_20 = 0x280;
        } else {
            local_20 = 0x1c2;
        }
    }

    float local_14 = (float)local_20 - _DAT_00552598;   // left edge of HP bar column
    float local_24 = 5.0f;                              // top of first bar (Y)
    char result = 0;

    UINT  uVar7 = *(BYTE*)(DAT_07abf5d8 + 0x1bc) & 7;  // player move_type_flags & 7
    UINT *puVar5 = (UINT*)&DAT_07e11e9c;                // party slot base (stride 9 uints = 0x24)

    for (int i = 0; i < (int)DAT_07eaa0e0; i++, puVar5 += 9) {
        if (*puVar5 < 0x80000000) {
            // [HashTable obfuscation block skipped — anti-tamper, not game logic]

            // Get entity stat ID for this party slot
            UINT *puVar12 = (UINT*)(UINT)*(BYTE*)((UINT)*(BYTE*)(DAT_07abf5d8 + 0x391) + 0x57 + (char*)DAT_07cf1ff4);

            // Check if entity type is a hoverable icon (0x1a/0x1b/0x1c/0xf/0x10)
            UINT pVal = (UINT)(uintptr_t)puVar12;
            bool bTypeOk = (pVal == 0x1a || pVal == 0x1b || pVal == 0x1c ||
                            pVal == 0xf  || pVal == 0x10);

            // Check mouse position within screen rect
            float mouseX = (float)DAT_083a427c;
            float mouseY = (float)DAT_083a4278;
            bool bPosOk = (mouseX >= local_14 - _DAT_00552540) &&
                          (mouseX <= local_14 + _DAT_00552598) &&
                          (mouseY >= local_24) &&
                          (mouseY <= local_24 + _DAT_005527d4);

            if (bTypeOk && bPosOk) {
                result = 1;
                SelectedCharacter = *puVar5;
            }
        }
        local_24 += _DAT_005527d4;
        *puVar5 = 0xfffffffe;
    }
    return result;
}
// Chat_InputTick — implemented in src/UI/Chat_InputTick.cpp
