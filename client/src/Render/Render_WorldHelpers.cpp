// Render_WorldHelpers.cpp
//
// Formerly stubs_render_helpers.cpp.  This module owns the world-render
// helpers called from Render_Scene3D and adjacent render passes.
//
// IDA provenance is intentionally retained at every entry point as
// `FUN_XXXXXXXX @ 0xXXXXXXXX`.  Functions are not renamed until their
// contracts are confirmed against the 0.97k binary; do not use 5.2 code to
// infer behaviour.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
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

// ── In-game render helpers (declared in functions.h, called from Render_Scene3D) ──
// 2026-04-28: minimal-impl stubs so we can wire them up in Render_Scene3D without
// link errors. Each will be ported per-IDA when the corresponding pool/entity
// system gets activated.
//
// FUN_00500aa0 @ 0x00500AA0 — RenderBoids
// Port FIEL del IDA: itera DAT_0839BE18 pool stride 444B (40 entries).
// Decoración de fauna/efectos del mapa: birds, fish, butterflies, magic gates.
//
// Pool layout (per IDA): cada entry = 111 floats = 444 bytes.
// v0 inicia en `(float*)dword_839BE18` y avanza 111 floats por iter, hasta unk_83A0378.
// Offsets relevantes (relativos a v0):
//   *((WORD*)v0 - 179)     type code (offset -358 from v0)
//   *((BYTE*)v0 - 360)     active flag
//   v0[-86..-84]            world pos x/y/z
//   v0[-83..-81]            pos2
//   v0[-81]                 angle Y (+360 mod adjust)
//   v0[-90..]               entity start
//   v0[+0]                  scale
//   v0[+48..50]             color RGB
// Externs ya en functions.h: FUN_004f8ff0, FUN_004fc030, FUN_00475220,
// FUN_004f7500, FUN_004409a0, FUN_00503cf0, FUN_00441f00

void __cdecl FUN_00500aa0(void)
{
    if (!DAT_0839be18 || !DAT_05828d58) return;

    float* v0 = (float*)((uintptr_t)DAT_0839be18);
    float* poolEnd = (float*)((uintptr_t)DAT_0839be18 + (40 * 444));

    while (v0 < poolEnd) {
        WORD entType = *((WORD*)v0 - 179);
        // Pre-tick: angle adjust (+90° rotation per frame for non-266)
        if (entType != 266) {
            v0[-81] = v0[-81] + 90.0f;
        }

        // Active flag
        if (*((BYTE*)v0 - 360)) {
            float xf = v0[-86] * 0.01f;
            float yf = v0[-85] * 0.01f;
            char vis = (char)FUN_004f8ff0(xf, yf, -20.0f);
            *((BYTE*)v0 - 8) = vis;

            if (vis) {
                if (entType == 266) {
                    // Skip pre-tick reverse + Entity_PrepareRender for type 266 (gate effect).
                    goto next_iter;
                }

                // Standard render path
                FUN_004fc030((unsigned char*)(v0 - 90), 1, 0, 0);

                // Type 301: special action — render hero body + secondary at pose offsets.
                if (entType == 301 && *((int*)v0 - 89) == 1) {
                    float light[3] = { 1.0f, 1.0f, 1.0f };
                    FUN_00503cf0(301, *v0, 1.0f, light, true);  // Hero body color
                }

                // Type 175: random fire-cloud sparkle.
                if (entType == 175) {
                    float scale = (float)((rand() % 32 + 64) * 0.01);
                    float color[3] = { scale * 0.2f, scale * 0.4f, scale * 0.4f };
                    FUN_00475220(1150, nullptr, v0 - 86, color, (int)(uintptr_t)(v0 - 90), 1.0f, 0);
                }

                // Type 184: dual-side flame jets.
                if (entType == 184) {
                    BYTE* model = (BYTE*)((uintptr_t)DAT_05828d58 + 188 * entType);
                    float locOffsetL[3] = { -5.0f, 0.0f, 0.0f };
                    float locOffsetR[3] = {  5.0f, 0.0f, 0.0f };
                    float Position[3];
                    float scale  = (float)((rand() % 32 + 128) * 0.01);
                    float color[3] = { scale, scale * 0.2f, 0.0f };

                    // Left jet
                    FUN_004409a0(model, nullptr, locOffsetL, Position, 1);
                    FUN_00475220(1150, nullptr, Position, color, (int)(uintptr_t)(v0 - 90), 0.1f, 0);
                    // Right jet
                    FUN_004409a0(model, nullptr, locOffsetR, Position, 1);
                    FUN_00475220(1150, nullptr, Position, color, (int)(uintptr_t)(v0 - 90), 0.1f, 0);
                }

                // World != 10: render shadow on terrain.
                if (DAT_0055a7ac != 10) {
                    BYTE* model = (BYTE*)((uintptr_t)DAT_05828d58 + 188 * entType);
                    glEnable(GL_BLEND);
                    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
                    float wx = v0[-86];
                    float wy = v0[-85];
                    float wz = FUN_004f7500(wx, wy);
                    *(float*)(model + 108) = wx;
                    *(float*)(model + 112) = wy;
                    *(float*)(model + 116) = wz;
                    FUN_00441f00(model, -1, -1);
                }
            }
        }

        // Post-tick: undo angle adjust for non-266
        if (entType != 266) {
            v0[-81] = v0[-81] - 90.0f;
        }
    next_iter:
        v0 += 111;
    }
}

// FUN_005038e0 @ 0x005038E0 — Entity_Render. Port FIEL del IDA.
// Itera Items[] pool (DAT_07e12840, 1000 entries × 516 bytes), per-entry:
//   1. Frustum-cull con sub_4F9590 (=FUN_004f9590) radio 400.
//   2. Resolve model slot por type (special handling 624..783, 860).
//   3. BMD_Animation (FUN_00440060) con bone matrix.
//   4. RequestTerrainLight + offset by entity color.
//   5. Special bombs (type 863): scatter pattern con N copies.
//   6. RenderPartObject (FUN_00505a10) en posición.
//   7. Projection a screen → save sx/sy en entity[+0xB8/+0xBA].
// RandomTable (0x055C9E58) la siembra WinMain con `rand() % 360`; la usa el
// montón de monedas del tipo 863 (Zen) para repartirlas en círculo.
// 2026-08-21: acá había una tabla local inventada con valores 0..99, así que
// `% 360` daba ángulos de sólo 0..99° → las monedas salían en una cuña en vez
// de en círculo, y siempre en el mismo patrón.

void __cdecl FUN_005038e0(void)
{
    int v14 = 1, v15 = 0, v16 = 0;
    BYTE* Items = (BYTE*)DAT_07e12840;
    BYTE* end   = Items + 1000 * 0x204;
    BYTE* v0    = Items + 333;

    while (v0 < end) {
        BYTE* v1 = v0 - 261;
        if (*v1) {
            // Frustum cull radio 400 alrededor de entity pos (v0-245 = pos vec3).
            float* v2 = (float*)(v0 - 245);
            char vis = (char)FUN_004f9590(v2, 400.0f);
            v0[91] = vis;
            if (vis) {
                short type = *(short*)(v0 - 259);
                int v5 = type;
                if (type < 624 || type >= 784) {
                    if (type == 860) {
                        int variant = (*(int*)(v0 - 325) >> 3) & 0xF;
                        if (variant == 0)      v5 = 947;
                        else if (variant == 2) v5 = 948;
                    }
                } else {
                    v5 = 390;  // player class fallback
                }
                if (v5 < 0 || v5 >= 1200) goto next_entry;

                BYTE* model = (BYTE*)((uintptr_t)DAT_05828d58 + (uintptr_t)v5 * 0xbc);
                model[160] = 0;
                model[152] = (BYTE)((*(BYTE*)((uintptr_t)DAT_07abf5d8 + 444)) & 7);
                model[160] = *v0;
                *(float*)(model + 108) = *v2;
                *(DWORD*)(model + 112) = *(DWORD*)(v0 - 241);
                *(DWORD*)(model + 116) = *(DWORD*)(v0 - 237);

                FUN_00503830(type, (int)model);

                // BMD_Animation
                float angles_in[3]  = { *(float*)(v0 - 233), *(float*)(v0 - 229), *(float*)(v0 - 225) };
                float headA[3]      = { *(float*)(v0 - 221), *(float*)(v0 - 217), *(float*)(v0 - 213) };
                // AnimationFrame / PriorFrame: IDA los toma en v0+3 y v0+7 —
                // offsets de BYTE (disasm 0x5039E9/0x5039ED: `mov ecx,[esi+7]`,
                // `mov edx,[esi+3]`).  2026-08-21: el port usaba v0+12 y v0+28,
                // que es la misma confusión float*/BYTE* que el Alpha de arriba.
                FUN_00440060(model, (int)&DAT_06970a9c,
                             *(float*)(v0 + 3), *(unsigned int*)(v0 + 7),
                             v0[1], (unsigned int*)angles_in, headA, 0, 0);

                // Compute lighting from terrain + entity color offset.
                float Light[3];
                FUN_004f7960(*v2, *(float*)(v0 - 241), Light);
                Light[0] += *(float*)(v0 - 29);
                Light[1] += *(float*)(v0 - 25);
                Light[2] += *(float*)(v0 - 21);

                // Type 863: scatter explosion with multiple copies.
                if (type == 863) {
                    float sx_orig = *v2;
                    float sy_orig = *(float*)(v0 - 241);
                    float sz_orig = *(float*)(v0 - 237);
                    int   pts     = (int)sqrt((double)*(int*)(v0 - 325)) / 2;
                    if (pts < 3)  pts = 3;
                    if (pts > 80) pts = 80;
                    int v9 = 1, v10 = v14;
                    while (v9 < pts) {
                        float angles_r[3] = {0.0f, 0.0f,
                            (float)(RandomTable[v10 % 100] % 360)};
                        float in1[3] = {
                            (float)(RandomTable[(v9 + v16) % 100] % (pts + 20)),
                            0.0f, 0.0f
                        };
                        float in2[3][4];
                        AngleMatrix(angles_r, in2);
                        float out[3];
                        FUN_004fa0b0(in1, (float*)in2, out);
                        *(float*)(v0 - 245) = out[0] + sx_orig;
                        *(float*)(v0 - 241) = out[1] + sy_orig;
                        *(float*)(v0 - 237) = out[2] + sz_orig;
                        // Alpha: IDA `*(float *)(v0 + 99)` — offset de BYTE
                        // (disasm 0x503B4C: `mov eax, [esi+63h]`), o sea Items+432,
                        // el mismo campo que usa el draw principal (v1 + 360).
                        // 2026-08-21: el port tenía v0 + 396 (Items+729).
                        FUN_00505a10((int)v1, type, 0, Light, *(float*)(v0 + 99),
                                     *(DWORD*)(v0 - 325), *(v0 - 302),
                                     1, 1, 1, 0, 2);
                        ++v9; ++v10;
                    }
                    *(float*)(v1 + 16) = sx_orig;
                    *(float*)(v1 + 20) = sy_orig;
                    *(float*)(v1 + 24) = sz_orig;
                }

                float v20 = *(float*)(v1 + 16);
                float v21 = *(float*)(v1 + 20);
                float v22 = *(float*)(v1 + 24);

                if (DAT_0055a7ac == 10) {  // World 10 (Icarus) — bamboleo
                    // IDA: sin((v15 + WorldTime) * flt_5528E0) * flt_552488 + z
                    *(float*)(v1 + 24) = (float)(sin(((double)v15 + (double)DAT_05826e08)
                                                     * (double)_DAT_005528e0)
                                                 * (double)_DAT_00552488 + v22);
                }

                // 2026-07-27 FIX (item del suelo renderizaba mal, "árbol"):
                // el IDA (L161-163) pasa `*((short*)v1 + 1)` = v1+2 = el TYPE
                // del entity (= model del item, ej 662) a RenderPartObject.
                // El port usaba v1+4 (= el flag "1" que escribe CreateItem en
                // ip+76) → renderizaba el modelo equivocado.
                FUN_00505a10((int)v1, *(short*)(v1 + 2), 0, Light,
                             *(float*)(v1 + 360),
                             *(DWORD*)(v0 - 325), *(v0 - 302),
                             1, 1, 1, 0, 2);

                *(float*)(v1 + 24) = v22;
                *(float*)(v1 + 16) = v20;
                *(float*)(v1 + 20) = v21;

                // 2026-07-27 FIX: proyectar la posición del item a pantalla y
                // guardarla en v1+92/94 (= word idx 46/47, IDA Entity_Render
                // L182-183). RenderItemName lee esa pos en o+0x5c/0x5e (= base+
                // 164/166 = v1+92/94) para dibujar el nombre SOBRE el item. El
                // port la zereaba en v1+184 (offset equivocado) → nombre en
                // (0,0). FUN_005113f0 (World_ToScreen) sí está implementado.
                {
                    float scr[3];
                    scr[0] = v20;                              // world X
                    scr[1] = v21;                              // world Y
                    scr[2] = v22 + _DAT_0055284c;              // world Z + name-tag offset
                    int sx = 0, sy = 0;
                    extern void __cdecl FUN_005113f0(float*, int*, int*);
                    FUN_005113f0(scr, &sx, &sy);
                    *(short*)(v1 + 92) = (short)sx;
                    *(short*)(v1 + 94) = (short)sy;
                }
                (void)v20; (void)v21; (void)v22;
            }
        }
    next_entry:
        v0  += 0x204;
        ++v16;
        v14 += 20;
        v15 += 1237;
    }
}

// FUN_00473710 @ 0x00473710 — Trail/Particle Effects Render
// Port FIEL del IDA decompile: itera unk_7B27B08 pool stride 2520B (= 630 ints).
// El pool está dentro del Joint pool DAT_07b27150 (200 × 0x9d8B), offset +0x9b8
// por entry. Itera 84 entries.
//
// Per entry, render trail con:
//   - EnableAlphaBlend (o Minus para types específicos)
//   - BindTexture(type), excepto type 266 (subtype lookup → 1253/1277/1278)
//   - Loop por segment con UV scrolling animado por WorldTime
//   - GL_QUADS por segment con interpolación entre vertices vecinos
// FUN_00511680/790/600/480 — declared en functions.h, sin extern "C" duplicate.

void __cdecl FUN_00473710(void)
{
    // Pool: DAT_07b27150 (Joint pool) + 0x9b8 stride 0x9d8.
    int* poolBase = (int*)((char*)DAT_07b27150 + 0x9b8);
    const int kStride = 0x9d8 / 4;   // 630 ints = 2520 bytes
    // El binario itera de &unk_7B27B08 a &unk_7C5B4E8 con stride 0x9d8 = 500
    // entradas; nuestro pool DAT_07b27150 está dimensionado a 200 slots, así que
    // iteramos los que entran (antes eran 84 fijos → los trails de los slots
    // 84..199 nunca se dibujaban).
    const int kEntries = (int)(sizeof(DAT_07b27150) / 0x9d8);   // 200
    const char* poolEnd = (const char*)DAT_07b27150 + sizeof(DAT_07b27150);

    for (int eIdx = 0; eIdx < kEntries; ++eIdx) {
        int* v0 = poolBase + eIdx * kStride;
        // active flag at byte offset -2488 (negative = before v0 base)
        if (!*((BYTE*)v0 - 2488)) continue;

        int type = *(v0 - 621);    // type code at offset -621*4 = -2484

        // Decide blend mode per type/subtype.
        bool useMinus = (type == 1253 || type == 1250);
        if (type == 1253) {
            int sub = *(v0 - 620);
            if (sub == 2 || sub == 3 || sub == 4 || sub == 5) useMinus = false;
        }
        if (type == 1250) {
            if (*(v0 - 620) != 4) useMinus = false;
        }
        // 2026-08-10 FIX (picos duros / líneas negras de los joints): el IDA
        // llama `EnableAlphaBlend()` en la rama else, que es **0x00511710 =
        // GL_SetBlendAdditive** (blend tipo 3). El port llamaba `FUN_00511680`
        // (GL_SetBlendSrcOver, alpha normal SRC_ALPHA/ONE_MINUS_SRC_ALPHA +
        // depth-mask ON). Con alpha normal los téxeles OSCUROS de la textura de
        // glow se pintan negros y opacos en vez de no sumar nada → el rayo
        // aparecía como una forma sólida de bordes duros (picos azules del MG)
        // y como líneas negras (mago). `EnableAlphaBlendMinus()` = 0x00511790,
        // que sí estaba bien.
        if (useMinus) FUN_00511790(); else FUN_00511710();

        // Texture bind per type.
        if (type == 266) {
            int sub = *(v0 - 620);
            if (sub == 0 || sub == 4)      FUN_00511480(1277);
            else if (sub == 1)             FUN_00511480(1253);
            else if (sub == 2)             FUN_00511480(1278);
            else if (sub == 3)             FUN_00511480(1253);
            // type-266 color modulation: clamp count, scale color
            int v30 = *v0;
            if (v30 > 20) v30 = 20;
            float scale = (float)v30 * 0.05f;
            float r = scale * *((float*)v0 - 609);
            float g = scale * *((float*)v0 - 608);
            float b = scale * *((float*)v0 - 607);
            glColor3f(r, g, b);
        } else {
            int texId = (type == 1255) ? 1254 : type;
            FUN_00511480(texId);
            glColor3fv((const GLfloat*)v0 - 609);
        }

        // Iterate segments.
        int segCount = *(v0 - 602);
        if (segCount <= 0) continue;

        int segMax  = *(v0 - 601);
        int* v6     = v0 - 599;   // segment data pointer
        // 2026-08-08 GUARD (crash 0xC0000005 dentro del driver GL, llamado desde
        // acá): segCount sale del slot del pool; si un slot queda con basura, el
        // loop avanza v6 de a 12 ints sin techo y termina pasándole a
        // glVertex3fv un puntero fuera de todo lo mapeado. El binario original
        // tampoco acota, pero acá el pool se corrompe por otros bugs de port, así
        // que clampeamos al espacio de datos del propio slot (cada entrada son
        // 630 ints; desde v0-599 entran (630-599+31)/12 segmentos con margen).
        {
            const char* slotEnd = (const char*)v0 + (kStride - 599) * 4;
            if (slotEnd > poolEnd) slotEnd = poolEnd;
            int maxSeg = (int)((slotEnd - (const char*)v6) / (12 * 4)) - 2;
            if (maxSeg < 0) maxSeg = 0;
            if (segCount > maxSeg) segCount = maxSeg;
            if (segCount <= 0) continue;
        }

        for (int segIdx = 0; segIdx < segCount; ++segIdx) {
            float s, v28;
            bool stepPow2 = (type == 1260 || type == 1261 ||
                             (type == 1254 && (*(v0 - 620) == 7 || *(v0 - 620) == 10)));
            if (stepPow2) {
                s   = (float)(segCount - (segIdx == 0 ? 0 : segIdx - 1)) * 0.0625f;
                v28 = (float)((segCount - (segIdx == 0 ? 0 : segIdx - 1)) - 1) * 0.0625f;
            } else if (segMax > 1) {
                float denom = (float)(segMax - 1);
                s   = (float)(segCount - segIdx) / denom;
                v28 = (float)((segCount - segIdx) - 1) / denom;
            } else {
                s = 1.0f; v28 = 0.0f;
            }

            // WorldTime modulation for types 1254/1255.
            float v29 = (float)((DWORD)DAT_05826e08 % 1000) * 0.001f;
            if (type == 1254 || type == 1255) {
                s   += s   - v29;
                v28 += v28 - v29;
            }

            // Render quad: 4 vertices forming a strip segment.
            // (v6 + offsets de vertices del segmento previo y actual)
            glBegin(GL_QUADS);
            glTexCoord2f(s,   0.0f); glVertex3fv((const GLfloat*)(v6 - 1));
            glTexCoord2f(s,   1.0f); glVertex3fv((const GLfloat*)(v6 + 2));
            glTexCoord2f(v28, 1.0f); glVertex3fv((const GLfloat*)(v6 + 14));
            glTexCoord2f(v28, 0.0f); glVertex3fv((const GLfloat*)(v6 + 11));
            glEnd();

            v6 += 12;   // advance to next segment (12 ints per segment data)
        }
    }

    FUN_00511600();   // DisableAlphaBlend after all
}


// 2026-05-07: NPC interaction packet helpers
// =============================================
// Wire formats per server source Mu-linux-97K/Source/MuServer/GameServer/
// {NpcTalk.h, ItemManager.h, Warehouse.h}.
// Cada send: plain C1 + chain XOR forward + MuEmu byte XOR + raw socket send.

namespace {
    static const BYTE s_NpcKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    void SendNpcPacket(BYTE* pkt, int totalLen) {
        for (int i = 3; i < totalLen; ++i) {
            pkt[i] ^= pkt[i - 1] ^ s_NpcKey[i & 0x1f];
        }
        MuEmu::EncryptSend(pkt, totalLen);
        if (DAT_055ca168 != 0xFFFFFFFF) {
            ::send(DAT_055ca168, (const char*)pkt, totalLen, 0);
        }
    }
}

// 0x31 PMSG_NPC_TALK_CLOSE_RECV — sin payload
// Wire: [C1][03][31]
extern "C" void Net_SendNpcTalkClose(void) {
    BYTE pkt[4] = { 0xC1, 0x03, 0x31, 0 };
    SendNpcPacket(pkt, 3);
}

// 0x97 — cerrar ventana de evento / Golden Archer
// Wire: [C1][03][97]  (Protocol.cpp case 0x97; lo manda
// CheckGoldenArcherWindow 0x4E7AC0 al click en el botón de cerrar)
extern "C" void Net_SendEventWindowClose(void) {
    BYTE pkt[4] = { 0xC1, 0x03, 0x97, 0 };
    SendNpcPacket(pkt, 3);
}

// 0x32 PMSG_ITEM_BUY_RECV — buy item from shop
// Wire: [C1][04][32][slot]
extern "C" void Net_SendItemBuy(BYTE shopSlot) {
    BYTE pkt[5] = { 0xC1, 0x04, 0x32, shopSlot, 0 };
    SendNpcPacket(pkt, 4);
}

// 0x33 PMSG_ITEM_SELL_RECV — sell item from inventory
// Wire: [C1][04][33][slot]
extern "C" void Net_SendItemSell(BYTE invSlot) {
    BYTE pkt[5] = { 0xC1, 0x04, 0x33, invSlot, 0 };
    SendNpcPacket(pkt, 4);
}

// 0x34 PMSG_ITEM_REPAIR_RECV — repair item
// Wire: [C1][05][34][slot][type]
//   type: 0=npc-repair, 1=self-repair
extern "C" void Net_SendItemRepair(BYTE invSlot, BYTE repairType) {
    BYTE pkt[6] = { 0xC1, 0x05, 0x34, invSlot, repairType, 0 };
    SendNpcPacket(pkt, 5);
}

// 0x23 PMSG_ITEM_DROP_RECV — drop item to ground
// Wire: [C1][06][23][x][y][slot]
extern "C" void Net_SendItemDrop(BYTE gridX, BYTE gridY, BYTE invSlot) {
    BYTE pkt[7] = { 0xC1, 0x06, 0x23, gridX, gridY, invSlot, 0 };
    SendNpcPacket(pkt, 6);
}

// 0x26 PMSG_ITEM_USE_RECV — use item (potion/scroll)
// Wire: [C1][05][26][srcSlot][tgtSlot]
extern "C" void Net_SendItemUse(BYTE srcSlot, BYTE tgtSlot) {
    BYTE pkt[6] = { 0xC1, 0x05, 0x26, srcSlot, tgtSlot, 0 };
    SendNpcPacket(pkt, 5);
}

// 0x81 PMSG_WAREHOUSE_MONEY_RECV — deposit/withdraw money
// Wire: [C1][08][81][type][moneyL][moneyM1][moneyM2][moneyH]
//   type: 0=deposit, 1=withdraw
extern "C" void Net_SendWarehouseMoney(BYTE type, DWORD money) {
    BYTE pkt[10];
    pkt[0] = 0xC1; pkt[1] = 0x08; pkt[2] = 0x81;
    pkt[3] = type;
    pkt[4] = (BYTE)(money & 0xFF);
    pkt[5] = (BYTE)((money >> 8) & 0xFF);
    pkt[6] = (BYTE)((money >> 16) & 0xFF);
    pkt[7] = (BYTE)((money >> 24) & 0xFF);
    SendNpcPacket(pkt, 8);
}

// 0x83 PMSG_WAREHOUSE_PASSWORD_RECV — set/change/verify password
// Wire: [C1][0F][83][type][passLow][passHigh][personalCode 10B]
//   type: 0=verify, 1=set, 2=change-old, 3=change-new
extern "C" void Net_SendWarehousePassword(BYTE type, WORD password,
                                           const char* personalCode) {
    BYTE pkt[16];
    pkt[0] = 0xC1; pkt[1] = 0x0F; pkt[2] = 0x83;
    pkt[3] = type;
    pkt[4] = (BYTE)(password & 0xFF);
    pkt[5] = (BYTE)((password >> 8) & 0xFF);
    for (int i = 0; i < 10; ++i) {
        pkt[6 + i] = personalCode ? (BYTE)personalCode[i] : 0;
    }
    SendNpcPacket(pkt, 0x0F);
}

// 0x86 PMSG_CHAOS_BOX_ITEM_PROC — chaos mix execute
// Wire: [C1][03][86]
extern "C" void Net_SendChaosBoxMix(void) {
    BYTE pkt[4] = { 0xC1, 0x03, 0x86, 0 };
    SendNpcPacket(pkt, 3);
}

// 0x87 PMSG_CHAOS_BOX_CLOSE — close chaos box
// Wire: [C1][03][87]
extern "C" void Net_SendChaosBoxClose(void) {
    BYTE pkt[4] = { 0xC1, 0x03, 0x87, 0 };
    SendNpcPacket(pkt, 3);
}

// FUN_004CB6F0 @ 0x004CB6F0 — Target_Render (sub_4CB6F0)
// 2026-05-07: port FIEL desde IDA mu97k-src-IDA/raw/004CB6F0_sub_4CB6F0.c.
// Renderiza el nombre del target hovered (NPC/mob/item) sobre la HUD 2D.
//
// IDA flow:
//   - Si SelectedItem != -1: RenderItemName del item
//   - Si SelectedNpc != -1: CreateChat con nombre del NPC (chat log)
//   - Si SelectedCharacter != -1 y not Attacking:
//     - Si target Kind == 2 (monster): RenderCenteredText centrado arriba con
//       el nombre del mob (color cyan). Posicion X depende de UI panels abiertos
//       (260/450/640).
//     - Else (player): CreateChat con nombre del player.
//   - Si Alt held: render nombre de TODOS los items dropeados visibles.
//
// Anti-tamper hash table noise (PACKET_ENCRYPT etc) skipped per project policy.
extern "C" SIZE* __cdecl RenderCenteredText(int iPos_x, int iPos_y, const char* pszText);
extern "C" int __cdecl GetScreenWidth(void);
void __cdecl FUN_004cb6f0(int /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/)
{
    DAT_07e11d6e = 1;
    glColor3f(1.0f, 1.0f, 1.0f);
    FUN_00511600();  // DisableAlphaBlend

    // 2026-05-07: solo activo in-world. CharSelect tiene su propio path con
    // entity pool poblado de chars; queremos que Target_Render solo procese
    // mob/NPC/player hovers en el mundo de juego.
    if (DAT_005615c0 != 5) return;

    // 2026-07-27: render de nombres de items en el suelo (port sub_4CB6F0
    // L61-68 + L158-177). Antes se skipeaba → nunca aparecía el nombre.
    //   o = &Items[i][72] (= slot base + 72, donde RenderItemName lee o+2 = model
    //   y hace -400 para el ItemAttribute). ItemLevel = ip+8, ItemOption = ip+31.
    {
        extern void __cdecl RenderItemName_stub(int, DWORD, int, int, bool);
        BYTE* itemPool = (BYTE*)&DAT_07e12840[0];
        int hovered = (int)SelectedItem;   // SelectedItem (item bajo el cursor)

        // 1. Nombre del item hovereado (Sort=0, se dibuja sobre el item).
        if (hovered >= 0 && hovered < 1000) {
            BYTE* o = itemPool + hovered * 0x204 + 72;
            if (o[0] && o[352]) {
                RenderItemName_stub(hovered, (DWORD)(uintptr_t)o,
                                    *(int*)(o - 64), (int)*(char*)(o - 41), 0);
            }
        }

        // 2. Alt como TOGGLE (IDA sub_4CB6F0 L159-163: PressKey(VK_MENU) togglea
        //    byte_7EAA15C; se muestran si el toggle está on O Alt está mantenido).
        //    Una pulsación de Alt alterna mostrar/ocultar todos los nombres.
        static int s_altNameToggle = 0;
        if (FUN_0047ec20(VK_MENU))               // Alt recién pulsado (edge)
            s_altNameToggle = !s_altNameToggle;
        if (s_altNameToggle || (GetAsyncKeyState(VK_MENU) & 0x8000) != 0) {
            for (int i = 0; i < 1000; ++i) {
                if (i == hovered) continue;
                BYTE* o = itemPool + i * 0x204 + 72;
                if (o[0] && o[352]) {
                    RenderItemName_stub(i, (DWORD)(uintptr_t)o,
                                        *(int*)(o - 64), (int)*(char*)(o - 41), 1);
                }
            }
        }
    }

    if (SelectedCharacter == -1 && SelectedNpc == -1) {
        return;
    }

    if (SelectedNpc != -1) {
        // NPC hovered — chat bubble per IDA CreateChat (FUN_00481ba0).
        char* base = (char*)(uintptr_t)DAT_07abf5d0;
        char* ent  = base + (int)SelectedNpc * 0x394;
        if (ent[0] != 0) {
            const char* name = (const char*)(ent + 0x1C1);
            if (name[0]) {
                FUN_00481ba0((char*)name, (char*)"", (DWORD)ent, 0, -1);
            }
        }
        return;
    }

    if (SelectedCharacter == -1) return;

    char* base = (char*)(uintptr_t)DAT_07abf5d0;
    char* ent  = base + (int)SelectedCharacter * 0x394;
    if (ent[0] == 0) return;

    BYTE kind = *(BYTE*)(ent + 0x84);  // 1=player, 2=monster, 4=npc
    const char* name = (const char*)(ent + 0x1C1);
    if (!name[0]) return;

    if (kind == 2) {
        // Monstruo: el nombre va arriba del todo, centrado.
        DAT_00559c80 = 0xFF000064;  // m_dwBackColor (azul oscuro)
        DAT_00559c78 = 0xFFC8E6FF;  // m_dwTextColor (celeste)
        // IDA sub_4CB6F0 LABEL_35: `RenderCenteredText(v13 / 2, 10, v3)`, donde
        // v13 se calcula con EL MISMO arbol de decision que GetScreenWidth
        // (0x4CB520, verificado linea por linea): 260 con inventario + panel
        // lateral, 450 con cualquier panel abierto, 640 con ninguno.
        //
        // 2026-08-22: aca habia un criterio inventado — leia un byte de
        // CharacterAttribute + 0x14E como si fuera "inventario abierto".  Ese
        // offset es un campo cualquiera del struct del personaje, asi que en
        // cuanto valia != 0 el nombre quedaba centrado en 225 (= el caso 450)
        // de forma permanente, sin ningun panel abierto.  Ademas faltaba el
        // caso 260.  Ahora sale de GetScreenWidth, que es la misma fuente.
        RenderCenteredText(GetScreenWidth() / 2, 10, name);
    } else if (kind == 1) {
        // Player: chat bubble per IDA (NOT chat log).
        FUN_00481ba0((char*)name, (char*)"", (DWORD)ent, 0, -1);
    }
}

// FUN_00502200 @ 0x00502200 — RenderFishs
// 2026-05-07: port FIEL desde IDA mu97k-src-IDA/raw/00502200_RenderFishs.c.
// Renderiza peces decorativos (Lorencia ponds, Devias mountains, etc).
// Pool: DAT_083a2e90 (10 entries × 0x1BC bytes = 4440 bytes total).
// Cada slot:
//   +0     active flag (byte)
//   +2     type code (WORD)
//   +16    pos.x (float)
//   +20    pos.y (float)
//   +36    rotation (float, accumulated +90/-90 per frame)
//   +352   visible flag (byte, set by frustum test)
//
// Logic:
//   1. Increment rotation +90 (then -90 at end → net 0, but used by render)
//   2. If active, test frustum at pos
//   3. If visible: PrepareRender, then if type != 188/189 and World != 10:
//      enable alpha test, set black alpha=0.2 color, sync model render slot
//      to pos with terrain height, call sub_441F00 (shadow render).
void __cdecl FUN_00502200(int /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/)
{
    char* base = (char*)&DAT_083a2e90[0];
    for (int i = 0; i < 10; ++i) {
        char* slot = base + i * 0x1BC;
        // Increment rotation +90 (per IDA, applied unconditionally).
        *(float*)(slot + 36) += 90.0f;

        if (slot[0] != 0) {
            float posX = *(float*)(slot + 16);
            float posY = *(float*)(slot + 20);
            float xGrid = posX * 0.01f;
            float yGrid = posY * 0.01f;
            // Wrapper compatible: FUN_004f8ff0 returns short (visible flag).
            // IDA's TestFrustrum2D returns bool.
            unsigned short vis = FUN_004f8ff0(xGrid, yGrid, -20.0f);
            slot[352] = (char)(vis != 0);
            if (vis) {
                FUN_004fc030((unsigned char*)slot, 0u, 0, 0);
                short typeCode = *(short*)(slot + 2);
                int World = (int)DAT_0055a7ac;
                if (typeCode != 188 && typeCode != 189 && World != 10) {
                    FUN_00511680('\x01');                  // EnableAlphaTest(1)
                    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);     // shadow color
                    float* modelData = (float*)(DAT_05828d58 + 188 * (int)typeCode);
                    if (modelData) {
                        modelData[29] = FUN_004f7500(posX, posY);  // height
                        modelData[27] = posX;
                        modelData[28] = posY;
                        FUN_00441f00((void*)modelData, -1, -1);    // shadow draw
                    }
                }
            }
        }
        // IDA does v0 += 222 then writes v0[-205] = rot - 90 — i.e. restore
        // rotation to net 0 in NEXT slot's offset. In our cleaner walker we
        // just decrement back here.
        *(float*)(slot + 36) -= 90.0f;
    }
}

// FUN_0046bba0 @ 0x0046BBA0 — RenderEffects
// Port FIEL del IDA: itera HeadAngle pool stride 111 floats (444 bytes).
// Dispatcher por type code para spell effects, weapon glow, gates.
//
// Pool: HeadAngle (start), unk_7B27178 (end), 111 floats stride.
// Offsets críticos (relativos a v0 = float*):
//   *((BYTE*)v0 - 40)        active flag
//   v0[-6..-4]               world position xyz
//   *((BYTE*)v0 + 312)       visibility flag (output)
//   *((WORD*)v0 - 19)        type code
//   v0[+38..40]              dir velocity
//   v0[+48..50]              color RGB
//   v0[-3..-1]               angles
//   v0[+56], v0[+57]         anim/prevAnim
//   *((BYTE*)v0 + 222)       prev action
//   v0[+16]                  scale
//   *((int*)v0 + 14)         counter (timeout)
// FUN_0046b980 ya está en functions.h.  (El comentario anterior decía que
// RenderWheelWeapon era 0x46B8C0: es **0x46B7C0**, y no estaba portada.)

// ── RenderWheelWeapon @ 0x0046B7C0 ────────────────────────────────────────────
// Port FIEL de IDA. Dibuja el efecto de "arma girando" (tipo de efecto 239):
// toma el arma del owner, la hace rotar sobre sí misma y la sube 100 unidades,
// la renderiza, y RESTAURA todos los campos que piso.
//
// El truco del binario es que reusa el propio slot del efecto como si fuera la
// entidad del arma: le cambia `Type` (o+2) al modelo del item y al final lo
// devuelve. Por eso guarda/restaura pos (16/20/24), angle (28/32/36) y type.
void __cdecl FUN_0046b7c0(DWORD o)
{
    if (!o || !DAT_05828d58 || !DAT_07abf5d8) return;

    // ── guardar el estado original del slot ──
    const int savedX     = *(int*)(o + 16);
    const int savedY     = *(int*)(o + 20);
    const int savedZ     = *(int*)(o + 24);
    const int savedAng0  = *(int*)(o + 28);
    const int savedAng1  = *(int*)(o + 32);
    const int savedAng2  = *(int*)(o + 36);
    const short savedType = *(short*)(o + 2);

    const DWORD owner = *(DWORD*)(o + 252);
    if (!owner) return;

    // Modelo del arma = itemType + 400 (owner+136 = índice del item).
    const int model_id = *(unsigned char*)(owner + 136) + 400;
    if (model_id < 0 || model_id >= 1024) return;

    // Giro: o+200 acumula -30°/frame y se suma al angle[2]; el arma además
    // flota 100 unidades sobre su posición y fija angle[1] = 90°.
    const float spin = *(float*)(o + 200) - 30.0f;
    *(float*)(o + 200) = spin;
    *(float*)(o + 36)  = spin + *(float*)(o + 36);
    *(int*)  (o + 32)  = 1119092736;              // 90.0f
    const float zUp    = *(float*)(o + 24) + 100.0f;
    *(float*)(o + 24)  = zUp;

    const float alpha = *(float*)(o + 360);

    // Volcar posición/acción/skin al slot del modelo compartido.
    BYTE* mdl = (BYTE*)(uintptr_t)DAT_05828d58 + 188 * model_id;
    *(float*)(mdl + 116) = zUp;
    *(BYTE*) (mdl + 160) = *(BYTE*)(o + 261);                       // CurrentAction
    *(BYTE*) (mdl + 152) = (BYTE)(*(BYTE*)((uintptr_t)DAT_07abf5d8 + 444) & 7); // clase
    *(int*)  (mdl + 108) = *(int*)(o + 16);
    *(int*)  (mdl + 112) = *(int*)(o + 20);

    *(short*)(o + 2) = (short)model_id;
    FUN_00502ba0((int)o);                          // ItemObjectAttribute

    float angles_in[3] = { *(float*)(o + 28), *(float*)(o + 32), *(float*)(o + 36) };
    FUN_00440060(mdl, (int)&DAT_06970a9c,
                 *(float*)(o + 264), *(unsigned int*)(o + 268),
                 *(BYTE*)(o + 262),
                 (unsigned int*)angles_in, (float*)(o + 40), 0, 0);

    float Light[3];
    FUN_004f7960(*(float*)(o + 16), *(float*)(o + 20), Light);
    Light[0] += *(float*)(o + 232);
    Light[1] += *(float*)(o + 236);
    Light[2] += *(float*)(o + 240);

    FUN_00505a10((int)o, model_id, 0, Light, alpha,
                 (unsigned int)(8 * *(unsigned char*)(owner + 137)),
                 0, 1, 1, 1, 0, 2);

    // ── restaurar ──
    *(short*)(o + 2)  = savedType;
    *(int*)  (o + 16) = savedX;
    *(int*)  (o + 20) = savedY;
    *(int*)  (o + 24) = savedZ;
    *(int*)  (o + 28) = savedAng0;
    *(int*)  (o + 32) = savedAng1;
    *(int*)  (o + 36) = savedAng2;
}

void __cdecl FUN_0046bba0(void)
{
    // BUG-FIX 2026-05-01: HeadAngle (0x07B11698) está en offset +40 dentro del
    // effect pool DAT_07b11670 (200 entries × 0x1bc bytes = 0x1bc stride = 444B).
    // En IDA: HeadAngle iter es float*, offsets negativos cubren la cabecera del
    // entry. v0 inicia en (float*)(pool + 40), recorre 200 entries de 111 floats.
    if (!DAT_05828d58) return;
    float* HeadAngle = (float*)((char*)DAT_07b11670 + 40);
    if (!HeadAngle) return;

    // Pool: 200 entries x 0x1bc bytes = 444B stride = 111 floats por entry.
    float* v0 = HeadAngle;
    const int kMaxEntries = 200;

    for (int i = 0; i < kMaxEntries; ++i, v0 += 111) {
        if (!*((BYTE*)v0 - 40)) continue;

        // Frustum cull radio 400 con pos offset (v0-6 = world pos)
        char vis = (char)FUN_004f9590((float*)(v0 - 6), 400.0f);
        *((BYTE*)v0 + 312) = vis;
        if (!vis) continue;

        WORD type = *((WORD*)v0 - 19);

        // Type 259: timed alpha entity (counter ramps from 0 to 10).
        if (type == 259) {
            int counter = *((int*)v0 + 14);
            if (counter >= 10) continue;
            v0[16] = (float)counter * 0.1f;
            FUN_004fc030((unsigned char*)(v0 - 10), 0, 0, 0);
            continue;
        }

        // ── 2026-08-15: CAUSA DE LOS "CUADROS BLANCOS" ───────────────────────
        // El binario NO manda todo el rango 190..268 a Entity_PrepareRender: su
        // switch (IDA L110-137) aparta cuatro tipos ANTES de caer al rango:
        //     case 238: case 243:  break;                  // no se dibujan
        //     case 239:  RenderWheelWeapon(o);             // renderer propio
        //     case 244:  sub_46B980(o);                    // renderer propio
        //     default:   if (190 <= t < 269) Entity_PrepareRender(o);
        // Esos cuatro slots NO tienen modelo cargado por `OpenSkills` (que sólo
        // llena 190-237, 240-242, 245-255, 259, 266-268), asi que dibujarlos
        // como entidad normal produce un quad sin geometria/textura = el
        // cuadrado blanco que se ve al lanzar skills.
        if (type == 238 || type == 243) {
            continue;                       // IDA: `break` — no renderiza nada
        }
        if (type == 239) {
            FUN_0046b7c0((DWORD)(uintptr_t)(v0 - 10));   // RenderWheelWeapon
            continue;
        }
        if (type == 244) {
            FUN_0046b980((int)(uintptr_t)(v0 - 10));
            continue;
        }

        // Types 182, 185, 260-263, 268, 190-268, 568: standard entity render.
        if (type == 182 || type == 185 || type == 568 ||
            (type >= 260 && type <= 263) || type == 268 ||
            (type >= 190 && type < 269 && type != 266)) {
            FUN_004fc030((unsigned char*)(v0 - 10), 0, 0, 0);
            continue;
        }

        // ── Tipos 1210-1212: estela de 30 sprites (IDA L41-64) ───────────────
        // Faltaba por completo. Emite 30 sprites 1176 a lo largo del vector de
        // avance del efecto (v0[38..40]), iluminando el terreno en cada paso.
        if (type > 1209 && type <= 1212) {
            float xf = v0[-6], yf = v0[-5], zf = v0[-4];
            for (int n = 0; n < 30; ++n) {
                float pos[3] = { xf, yf, zf };
                FUN_004795c0(1176, pos, v0[-7], v0 + 48,
                             (int)(uintptr_t)(v0 - 10), 0.0f, 0);
                xf += v0[38];
                yf += v0[39];
                zf += v0[40];
                AddTerrainLight(xf, yf, v0 + 48, 2, PrimaryTerrainLight[0]);
            }
            continue;
        }

        // Type 266: gate effect with BMD_Animation + RenderPartObject.
        if (type == 266) {
            BYTE* model = (BYTE*)((uintptr_t)DAT_05828d58 + 50008);
            float angles_in[3]  = { v0[-3], v0[-2], v0[-1] };
            float headA[3]      = { v0[0],  v0[1],  v0[2]  };
            FUN_00440060(model, (int)&DAT_06970a9c,
                         v0[56], *(unsigned int*)&v0[57],
                         *((BYTE*)v0 + 222),
                         (unsigned int*)angles_in, headA, 0, 0);

            v0[16] = 1.0f;
            float intensity = (float)*((int*)v0 + 14) * 0.05f;
            float Light[3] = {
                v0[48] * intensity,
                v0[49] * intensity,
                v0[50] * intensity
            };
            FUN_00505a10((int)(v0 - 10), 266, 0, Light, 1.0f, 0, 0, 1, 1, 1, 0, 66);
            continue;
        }

        // Types 1210-1212: scatter sprite chain (30 copies along velocity vector).
        if (type >= 1210 && type <= 1212) {
            float xf = v0[-6], yf = v0[-5], zf = v0[-4];
            for (int k = 0; k < 30; ++k) {
                float pos[3] = { xf, yf, zf };
                // IDA 0046BBA0 calls CreateSprite here.  This is the
                // independent sprite pool (004795C0), not CreateParticle
                // (00475220); the latter interprets its second argument as
                // a position and dereferences the previous nullptr.
                FUN_004795c0(1176, pos, v0[-7], v0 + 48,
                              (int)(uintptr_t)(v0 - 10), 0.0f, 0);
                xf += v0[38];
                yf += v0[39];
                zf += v0[40];
            }
            continue;
        }

        // Type 239: rotating weapon trail. FUN_0046b8c0 (RenderWheelWeapon)
        // no está portado todavía → skip silente.
        if (type == 239) {
            continue;
        }

        // Type 244: special handler.
        if (type == 244) {
            FUN_0046b980((int)(uintptr_t)(v0 - 10));
            continue;
        }
    }
}

// FUN_0046cb70 @ 0x0046CB70 — SkillEffect_Render (48 lines IDA).
//   Real implementation lives in src/Render/SkillEffect_Render.cpp::SkillEffect_Render.
//   Wrapper here so functions.h declaration matches a definition.
extern void SkillEffect_Render(void);
void __cdecl FUN_0046cb70(void) { SkillEffect_Render(); }

// FUN_00524cb0 @ 0x00524CB0 — MoveMainCamera  (port 1:1 desde IDA, 2026-06-27)
// Setea los parámetros de cámara que consume Camera_SetupFrustum:
//   CameraFOV = 35.0  (antes el port no lo seteaba → quedaba stale 45/55/10)
//   CameraViewFar = 2000 (o 3200 en topview)
//   CameraDistance = 1000 + smoothing (CameraDistanceTarget)
//   CameraPosition vía AngleMatrix(CameraAngle)+VectorIRotate del offset (0,-1000,0)
//   CameraAngle[0] = EarthQuake - 48.5  (pitch SET después de la posición)
// Símbolos IDA: CameraTopViewEnable=DAT_083a42e9, CameraDistance=DAT_083a45d0,
//   CameraDistanceTarget=DAT_005616b4. Retorna 0 (no-spectator) como IDA.
// Sin force-yaw ni DIAG: el yaw lo preserva el estado de cámara, igual que IDA.
bool __cdecl FUN_00524cb0(void) {
    float in1[3];
    float out[3];
    float matrix[3][4];

    CameraFOV = 35.0f;
    CameraAngle[0] = 0.0f;
    CameraAngle[1] = 0.0f;
    if (World == 10) {
        CameraAngle[0] = 0.0f;
        CameraAngle[1] = 0.0f;
    }
    if (DAT_083a42e9) {                            // CameraTopViewEnable
        CameraViewFar = 3200.0f;
        CameraPosition[2] = 3200.0f;
        CameraPosition[0] = *(float*)(Hero + 16);
        CameraPosition[1] = *(float*)(Hero + 20);
    } else {
        CameraViewFar = 2000.0f;
        DAT_083a45d0 = 1000.0f;                    // CameraDistance
        in1[0] = 0.0f;
        in1[1] = -1000.0f;
        in1[2] = 0.0f;
        AngleMatrix(CameraAngle, matrix);
        VectorIRotate(in1, matrix, out);
        CameraPosition[0] = out[0] + *(float*)(Hero + 16);
        CameraPosition[1] = out[1] + *(float*)(Hero + 20);
        CameraPosition[2] = *(float*)(Hero + 24) + DAT_083a45d0 - 150.0f;
        CameraAngle[0] = EarthQuake - 48.5f;       // pitch después de la posición
    }
    if (World == 5) {
        float v1 = (float)sin((double)WorldTime * 0.00050000002);
        CameraAngle[0] = v1 + v1 + CameraAngle[0];
        CameraAngle[1] = (float)sin((double)WorldTime * 0.00079999998) * 2.5f + CameraAngle[1];
    }
    // Smoothing de CameraDistance hacia CameraDistanceTarget (IDA final).
    DAT_083a45d0 = (DAT_005616b4 - DAT_083a45d0) * 0.33333334f + DAT_083a45d0;
    return false;
}


void __cdecl StopBuffer(int Buffer, int /*Object*/) { FUN_00404c60(Buffer); }



// FUN_00406f50 @ 0x00406F50 — Resource_LoadOrFatal(filename).
// Original: calls FUN_0053D5A0 (Resource_Load). On failure: shows "IError"
// MessageBox + Window_FatalError to terminate.
//
// PORT FIX (2026-04-25): the resource manager context (DAT_083bbb14) is never
// initialized in our port — FUN_0053d5a0 always returns 0, which would make
// every caller fatal-error. The most visible offender is Game_SceneUpdate.cpp
// case 0x14 (post-login Character list ready) which passes the username
// "tester" as a filename → IError MessageBox blocks user from ever reaching
// char-select, which is what the user reports happens "siempre".
//
// Neutralized: still calls Resource_Load (so any future side effects remain
// once the manager is wired up) but suppresses the modal + fatal exit. Once
// resource loading is fully ported this guard can be removed.
void __cdecl FUN_00406f50(char* param_1) {
    (void)FUN_0053d5a0(param_1);
    // Suppressed:
    // if ((char)uVar1 == '\0') {
    //     MessageBoxA((HWND)0, (LPCSTR)&lpText_07d63aec, "IError", 0);
    //     Window_FatalError(DAT_055c9ffc, '\x01');
    // }
}


// FUN_00440a30 @ 0x00440a30 — implemented in Render/BMD_SetupRender.cpp
// FUN_00502ba0 @ 0x00502ba0 — implemented in Entity/Entity_Reset.cpp
// FUN_004553c0 @ 0x004553c0 — implemented in Render/BMD_SetupRender.cpp
// FUN_00454fc0 — implemented in src/Render/Entity_Render.cpp
// FUN_00456770 — implemented in src/Render/Entity_UpdateRender.cpp (Entity_UpdateRender, 2195 lines)
// FUN_0045ab00 — implemented in src/Render/Entity_Render.cpp
// FUN_0045adc0 — implemented in src/Entity/Entity_Spawn.cpp (Entity_Spawn, 797 lines)
// FUN_0045f930 — implemented in src/Entity/Entity_Init.cpp
// FUN_0045fa20 — implemented in src/Entity/Entity_Init.cpp
// FUN_0046b790 — implemented in src/Render/Effect_Tick.cpp
// FUN_0046c3e0 — implemented in src/Render/Joint_Render.cpp
// FUN_0046cc80 — implemented in src/Render/Weather_Particles.cpp (weather particle tick, 399 lines)
// FUN_004736e0 — implemented in src/Render/Effect_Tick.cpp
// FUN_00473ea0 — implemented in src/Render/Effect_Tick.cpp
// FUN_00474f90 — implemented in src/Render/Effect_Tick.cpp
// FUN_00475090 — implemented in src/Render/Effect_Tick.cpp
// FUN_00478c00 — implemented in src/Render/ItemDrop_Render2.cpp (sprite pool render, 244 lines)
// FUN_00479380 — implemented in src/Render/Effect_Tick.cpp
// FUN_004794a0 — implemented in src/Render/Effect_Tick.cpp
// FUN_00479730 — implemented in src/Sound/Sound_Queue.cpp (Sound_UpdateQueue)
// FUN_0047ec60 — implemented in src/Input/Input.cpp
// FUN_0047f0b0 — implemented in src/UI/Chat.cpp
// FUN_0047f650 — implemented in src/UI/Chat.cpp
// FUN_0047fcb0 — implemented in src/Sound/Sound_Queue.cpp
// FUN_0047fce0 — implemented in src/UI/Chat.cpp
// FUN_00480950 — implemented in src/Sound/Sound_Queue.cpp
// FUN_00480980 — implemented in src/UI/Chat.cpp
// FUN_004821a0 — implemented in src/UI/Chat.cpp
// FUN_004acef0 — implemented in src/Game/Player_InputTick.cpp
// FUN_004b0310 — implemented in src/Input/Mouse_Hover.cpp (mouse hover/cursor tick, 523 lines)
