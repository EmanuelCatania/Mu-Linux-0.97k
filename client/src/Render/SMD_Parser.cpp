// SMD_Parser.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 4346-5298 (953 lines).
//
// SMD (Half-Life skeletal model format) parsers and SMD2BMD converters.
// El cliente 0.97k usa BMD compresado para distribución, pero el código
// de loader SMD original está aquí como referencia + fallback path.
//
//   ParseNodes / ParseSkeleton / ParseTriangles — SMD section parsers
//   FUN_0040b350 — SMD tokenizer (read next token)
//   FixupSMD     — post-process skeleton + triangles
//   SMD2BMDModel / SMD2BMDAnimation — convert SMD parsed data to BMD slots
//   OpenSMDFile (probe stub)

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

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

// ── SMD parser stubs ─────────────────────────────────────────────────────────
// Los archivos SMD (Half-Life skeletal model format) NO se distribuyen con el
// BUG-FIX 2026-05-03: previously this stub returned false unconditionally,
// shadowing the real implementation at line 18532 which has the parser logic.
// The shadow happened because both definitions exist (C-linkage here +
// C++-linkage further below); call sites in this file resolve to the
// extern "C" stub via the forward decl at line 3943 → SMD models never load.
// Now we delegate to the parser proper. Cast the const-char arg since the
// real impl mutates the input via fopen handle but signature is `char*`.
extern bool __cdecl OpenSMDFile(char *FileName, int Type, bool Flip);  // C++ impl, line 18532
extern "C" bool __cdecl OpenSMDFile(const char* FileName, int Type, char Flip) {
    return OpenSMDFile(const_cast<char*>(FileName), Type, (bool)Flip);
}
extern "C" void __cdecl FixupSMD(void) { /* SMD post-process — not ported */ }
extern "C" void __cdecl SMD2BMDModel(int /*ID*/, int /*Actions*/) { /* convert — not ported */ }
extern "C" void __cdecl SMD2BMDAnimation(int /*ID*/, char /*LockPosition*/) { /* convert anim — not ported */ }
// FUN_00506050 @ 0x00506050 — OpenModels(Model, FileName, i)
// Port FIEL del IDA: construye "prefix01.smd" (i<10) o "prefix11.smd" (i>=10),
// llama OpenSMDModel + OpenSMDAnimation. Mismo no-op silencioso si SMD no
// existe en filesystem.
void __cdecl FUN_00506050(int Model, const char* FileName, int i) {
    char Buffer[256];
    if (i >= 10) {
        crt_sprintf(Buffer, "%s%d.smd", FileName, i);
    } else {
        crt_sprintf(Buffer, "%s0%d.smd", FileName, i);
    }
    FUN_0040b280(Model, Buffer, 1, 0);
    FUN_0040b310(Model, Buffer, 0);
}

// CRT file helpers
FILE* __cdecl FUN_0054173f(const char* path, const void* mode) {
    return fopen(path, (const char*)mode);
}
void __cdecl FUN_0054150f(FILE* f) {
    if (f) fclose(f);
}
void __cdecl FUN_00543264(int ch, int *fp) {
    if (fp) fputc(ch, (FILE*)fp);
}

// FUN_00529740 (Texture_Load OZJ), FUN_00529bd0 (OpenTGA), FUN_0052a050 (Texture_FreeSlot)
// moved to src/Render/Texture/Texture.cpp (B3 refactor 2026-05-07, 395 lines).

// FUN_0043db30 @ 0x0043DB30 — Net_WSAStartup(__fastcall int param_1)
// Initialises WinSock 2.2. On success: stores wVersion low-word at param_1+4,
// clears param_1+8, returns 1. On failure: logs error, shows MessageBox, returns 0.
void __cdecl FUN_0043db30(int param_1) {
    WSADATA wsaData;
    int r = WSAStartup(0x202, &wsaData);
    if (r != 0) {
        FUN_00405540(&DAT_055c9bf0, "Winsock DLL Initialize error");
        MessageBoxA(NULL, "Winsock error", "IError", 0);
        return;
    }
    if (((char)wsaData.wVersion == '\x02') && ((char)(wsaData.wVersion >> 8) == '\x02')) {
        *(unsigned int*)(param_1 + 8) = 0;
        *(unsigned int*)(param_1 + 4) = wsaData.wVersion & 0xffff;
        FUN_00403a30();
    } else {
        WSACleanup();
        FUN_00405540(&DAT_055c9bf0, "Winsock version low");
        MessageBoxA(NULL, "Winsock version error", "IError", 0);
    }
}

// FUN_0043dbf0 @ 0x0043DBF0 — Net_CreateSocket(__thiscall void *this, int param_1)
// Creates TCP socket, stores in *(this+8). Logs and shows MessageBox on failure.
void __cdecl FUN_0043dbf0(void* ctx, int param_1) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    *(SOCKET*)((char*)ctx + 8) = s;
    DAT_05826cf0 = 0;
    if (s == INVALID_SOCKET) {
        char buf[128];
        int err = WSAGetLastError();
        wsprintfA(buf, "Socket error: %d", err);
        FUN_00405540(&DAT_055c9bf0, buf);
        MessageBoxA(NULL, buf, "IError", 0);
        return;
    }
    *(DWORD*)ctx = (DWORD)param_1;
}

// FUN_0045c050 @ 0x0045C050 — Entity_SetMoveSpeed(entity_ptr)
// Sets entity speed (+0x0c) based on move_type_flags (+0x1bc) and swim flag (+0x1bd).
// Also sets +0x1e0 to anim speed table index for vehicle entities.
void __cdecl FUN_0045c050(int param_1) {
    if (*(char*)(param_1 + 0x34f) == '\0') {
        short sVar1 = *(short*)(param_1 + 0x1f8);
        if (((sVar1 == 0x270) || (sVar1 == 0x272)) || ((0x279 < sVar1 && (sVar1 < 0x27e)))) {
            *(unsigned short*)(param_1 + 0x1e0) =
                (*(unsigned char*)(param_1 + 0x1bc) & 7) + 0x390 +
                (unsigned short)(*(unsigned char*)(param_1 + 0x1bc) >> 3) * 4;
        } else {
            *(unsigned short*)(param_1 + 0x1e0) = 0xffff;
        }
        if (*(char*)(param_1 + 0x1bd) != '\0') {
            switch (*(unsigned char*)(param_1 + 0x1bc) & 7) {
            case 0: case 1: *(unsigned int*)(param_1 + 0x0c) = 0x3f6e147b; return; // 0.93f
            case 2:         *(unsigned int*)(param_1 + 0x0c) = 0x3f5c28f6; return; // 0.86f
            case 3:         goto case3;
            default:        return;
            }
        }
        switch (*(unsigned char*)(param_1 + 0x1bc) & 7) {
        case 0: case 1: *(unsigned int*)(param_1 + 0x0c) = 0x3f666666; return; // 0.9f
        case 2:         *(unsigned int*)(param_1 + 0x0c) = 0x3f6147ae; return; // 0.88f
        case 3:
        case3:          *(unsigned int*)(param_1 + 0x0c) = 0x3f733333; return; // 0.95f
        }
    }
}

// FUN_00543274 @ 0x00543274 — fprintf wrapper.
// IDA: int fprintf(FILE* Stream, const char* Format, ...). Original wraps
// _lock_file/_stbuf/_output/_ftbuf/_unlock_file. Equivalent to plain fprintf.
// Only call site (line ~1835) passes 2 args (file, format string with no
// variadic args), so the simple 2-arg form is safe.
void __cdecl FUN_00543274(void* param_1, void* param_2) {
    if (!param_1 || !param_2) return;
    fprintf((FILE*)param_1, "%s", (const char*)param_2);
}

// FUN_00442090 @ 0x00442090 — BMD_FreeModel(model_ptr)
// Frees all bone mesh/action/texture data from a model slot at param_1.
// Bones: stride 0x8c, sub-meshes: stride 0x28, actions: stride 0x10, textures via Texture_Unload.
void __cdecl FUN_00442090(int param_1) {
    short numBones   = *(short*)(param_1 + 0x22);
    short numMeshes  = *(short*)(param_1 + 0x26);
    short numActions = *(short*)(param_1 + 0x24);
    if (numBones == 0) return;
    // free per-bone sub-mesh vertex arrays
    for (int bi = 0; bi < numBones; bi++) {
        int bonePtr = *(int*)(param_1 + 0x2c) + bi * 0x8c;
        if (*(char*)(bonePtr + 0x22) == '\0') {
            if (numMeshes > 0) {
                int meshBase = *(int*)(bonePtr + 0x24);
                for (int mi = 0; mi < numMeshes; mi++) {
                    int mPtr = meshBase + mi * 0xc;
                    operator_delete(*(void**)(mPtr + 0));
                    operator_delete(*(void**)(mPtr + 4));
                    operator_delete(*(void**)(mPtr + 8));
                }
            }
            operator_delete(*(void**)(bonePtr + 0x24));
        }
    }
    // free action keyframe data
    for (int ai = 0; ai < numMeshes; ai++) {
        int aPtr = *(int*)(param_1 + 0x30) + ai * 0x10;
        if (*(char*)(aPtr + 10) != '\0')
            operator_delete(*(void**)(aPtr + 0xc));
    }
    // free mesh vertex/normal/UV buffers + unload textures
    for (int mi = 0; mi < numActions; mi++) {
        int mBase = *(int*)(param_1 + 0x28) + mi * 0x28;
        operator_delete(*(void**)(mBase + 0x10));
        operator_delete(*(void**)(mBase + 0x14));
        operator_delete(*(void**)(mBase + 0x18));
        operator_delete(*(void**)(mBase + 0x1c));
        if (*(void**)(mBase + 0x24) != nullptr) {
            operator_delete(*(void**)(mBase + 0x24));
            *(DWORD*)(mBase + 0x24) = 0;
        }
        int texId = (int)*(short*)(*(int*)(param_1 + 0x38) + *(short*)(mBase + 2) * 2);
        if (texId != 0x12d)
            FUN_0052a050(texId);
    }
    if (*(void**)(param_1 + 0x28) != nullptr) { operator_delete(*(void**)(param_1 + 0x28)); *(DWORD*)(param_1 + 0x28) = 0; }
    if (*(void**)(param_1 + 0x2c) != nullptr) { operator_delete(*(void**)(param_1 + 0x2c)); *(DWORD*)(param_1 + 0x2c) = 0; }
    if (*(void**)(param_1 + 0x30) != nullptr) { operator_delete(*(void**)(param_1 + 0x30)); *(DWORD*)(param_1 + 0x30) = 0; }
    if (*(void**)(param_1 + 0x34) != nullptr) { operator_delete(*(void**)(param_1 + 0x34)); *(DWORD*)(param_1 + 0x34) = 0; }
    if (*(void**)(param_1 + 0x38) != nullptr) { operator_delete(*(void**)(param_1 + 0x38)); *(DWORD*)(param_1 + 0x38) = 0; }
    *(short*)(param_1 + 0x22) = 0;
    *(short*)(param_1 + 0x26) = 0;
    *(short*)(param_1 + 0x24) = 0;
}

// FUN_004ffd50 @ 0x004FFD50 — Terrain_ResetObjects
// Calls BMD_FreeModel on every model slot (stride 0xbc, count 0x7580/0xbc).
// Then walks the 16x16 scene-entity grid (DAT_083a0218, stride 0x10) freeing nodes via FUN_004ffcc0.
// Finally unloads tile textures 0x23-0x67, clears particle/effect/entity pools.
void __cdecl FUN_004ffd50(void) {
    // free all model slots
    for (int i = 0; i < 0x7580; i += 0xbc)
        FUN_00442090(i + DAT_05828d58);

    // free scene entity grid (16x16, stride 0x10)
    // Original binary terminated when puVar5 > 0x83a1217 (grid_base 0x83a0218 + 0xFFF).
    // In our build grid is g_ObjectBucketGrid (linker-placed); use end-pointer instead.
    char* puVar5 = (char*)&DAT_083a0218;
    char* gridEnd = (char*)g_ObjectBucketGrid + 0xFFF;
    do {
        for (int cell = 0; cell < 0x10; cell++) {
            char* head = (char*)*(DWORD*)(puVar5 + 8);
            while (head != nullptr) {
                char* next = (char*)*(DWORD*)(head + 0x1b4);
                FUN_004ffcc0((void*)head, (int)puVar5);
                head = next;
            }
            *(DWORD*)(puVar5 + 4) = 0;
            *(DWORD*)(puVar5 + 8) = 0;
            puVar5 += 0x10;
        }
        if (gridEnd < puVar5) {
            // unload tile textures
            for (int ti = 0x23; ti < 0x68; ti++) FUN_0052a050(ti);
            // ── Pool zero-clear loops (DESACTIVADOS) ─────────────────────────────
            // El binario original limpiaba 9 pools de partículas/efectos/entidades
            // usando direcciones ABSOLUTAS del .bss original (rangos 0x07c85890..0x83a3fe8).
            // En nuestro port:
            //   • DAT_07c85890, _0839bcb0, _07abf5f0, _07c80110, _07b27150, _07b11670
            //     son `char = 0` stubs de 1 byte — iterar con stride 0x1bc/0x70 escribe
            //     cientos de KB hacia globals adyacentes y luego en memoria no mapeada.
            //   • Los loops `(char*)0x83a2e90` y `(char*)0x7c5ab30` arrancan en literal
            //     pointers que en nuestro binario están sin commit → segfault inmediato.
            //   • DAT_083a2370 sí es un array real (0x960 bytes) pero el bound 0x83a3ae0
            //     también es absoluto.
            // Estas pools están vacías (no se llenan en login/char-select) así que
            // saltearlas es seguro hasta que migremos cada uno a símbolos con tamaño.
            return;
        }
    } while (true);
}

// FUN_0045abb0 @ 0x0045ABB0 — Entity_ClearByType(map_id)
// Loops over entity array (base DAT_07abf5d0, stride 0x394).
// For each active entity whose type (+0x1dc) != map_id: clears active flag,
// also clears matching emitter pool entries (DAT_083a1218, stride 0x1bc).
// Then calls FUN_00449840 on every slot.
//
// Inner loop bound: el binario original usaba el literal 0x83a2370 (= DAT_083a1218
// + 0x1158, fin del array Butterfles). En nuestro port DAT_083a1218 es un array
// real (10 × 0x1bc = 0x1158 bytes) pero el linker lo coloca en otra dirección,
// así que el literal es basura — pcVar2 sigue iterando hasta crashear.
// Se reemplaza por DAT_083a1218 + 0x1158 (end-pointer real).
void __cdecl FUN_0045abb0(int param_1) {
    char* butterflesEnd = DAT_083a1218 + 0x1158;
    for (int i = 0; i < 0x59740; i += 0x394) {
        char* puVar1 = (char*)(i + DAT_07abf5d0);
        if (*puVar1 != '\0' && *(short*)(puVar1 + 0x1dc) != (short)param_1) {
            *puVar1 = 0;
            char* pcVar2 = DAT_083a1218;
            do {
                if (*pcVar2 != '\0' && *(void**)(pcVar2 + 0xfc) == puVar1)
                    *pcVar2 = '\0';
                pcVar2 += 0x1bc;
            } while (pcVar2 < butterflesEnd);
        }
        FUN_00449840((int)puVar1, (int)puVar1, 0);
    }
}

// Effect/particle
// FUN_00466ad0 @ 0x00466AD0 — MoveEffect: implemented in Render/MoveEffect.cpp
// FUN_004660f0 @ 0x004660F0 — Effect_SmokeBurst: implemented in Render/MoveEffect_Helpers.cpp
// FUN_004661f0 @ 0x004661F0 — Effect_SmokeExplosion: implemented in Render/MoveEffect_Helpers.cpp
// FUN_00460c30 @ 0x00460C30 — Effect_LightningBurst: implemented in Render/MoveEffect_Helpers.cpp
// FUN_00465e60 @ 0x00465E60 — Effect_OnHitProximity: implemented in Render/MoveEffect_Helpers.cpp
// FUN_00473d90 @ 0x00473D90 — Ring_ComputeOrbit: implemented in Render/MoveEffect_Helpers.cpp
// STUB: Effect_AutoAttack — proximity-check all entities against param_1, fire
// attack effect (CreateBomb/CreateJoint 0x4E1) at nearby targets.
// Real logic: iterates CharactersClient[0..399], distance check <= DAT_005524f0,
// sub_466440 @ 0x00466440 — Effect/projectile collision/trigger handler.
// Port FIEL desde IDA decompile (2026-05-02). Anti-tamper hash table noise
// (CharacterMachine encrypt/decrypt wrappers) skipped per project policy.
//
// Called per-frame from MoveEffect (3 sites) and MoveJoint (1 site) when
// a projectile/effect entity is moving. Two paths:
//   A) Target[+132] != 0: targeted skill — 50% rand check, hero-only,
//      cooldown decrement (skill 52), radius 100 collision, then dispatch
//      CreateJoint (skill 52 chain) / CreateBomb (skill 51 explosion) /
//      sound + bomb (other).
//   B) Target[+132] == 0: AOE — scan CharactersClient for any non-self
//      visible entity within 100u, then dispatch similar.
//
// Used skill IDs (read from CharacterAttribute[+87 + Target[+133]]):
//   51 = explosion / bomb
//   52 = chain / joint hit
//
// Entity types triggering bomb FX:
//   223 (0xDF), 243 (0xF3) — produce CreateBomb on hit
// FUN_004660f0 declared in functions.h as (float*, char). Using through normal
// linkage (no extern decl needed here).

void __cdecl FUN_00466440(int Target) {
    char* T = (char*)(uintptr_t)Target;
    if (!T) return;
    DWORD ca = (DWORD)DAT_07cf1ff4;  // CharacterAttribute
    if (ca == 0) return;

    if (*(unsigned char*)(T + 132) != 0) {
        // ── Path A: targeted skill ────────────────────────────────────────
        int rcheck = rand() & 0x80000001;
        // IDA test reduces to: rcheck == 0 || (rcheck has wrap-around). The
        // intent is a ~50% pass with sign-bit handling for negative rand.
        bool randPass;
        if (rcheck < 0) {
            randPass = ((((unsigned char)rcheck - 1) | 0xFFFFFFFE) == 0xFFFFFFFF);
        } else {
            randPass = (rcheck == 0);
        }
        if (!randPass) return;
        if (*(int*)(T + 252) != (int)(uintptr_t)DAT_07abf5d8) return;  // hero only

        // Anti-tamper hash table — skipped

        // Read skill ID from CharacterAttribute[+87 + Target[+133]]
        unsigned char skillIdx = *(unsigned char*)(T + 133);
        DWORD v36 = (DWORD)*(unsigned char*)(skillIdx + ca + 87);

        // Anti-tamper hash table — skipped

        if (v36 == 52) {
            int cd = *(int*)(T + 244);
            if (cd > 0) {
                *(int*)(T + 244) = cd - 1;
                return;
            }
        }
        float* posPtr = (float*)(T + 16);
        if (FUN_0045fec0((unsigned int)skillIdx, posPtr, 100.0f,
                         *(unsigned char*)(T + 136),
                         *(short*)(T + 134))) {
            short type = *(short*)(T + 2);
            if (v36 == 51) {
                *T = 0;
                if (type != 223 && type != 243) return;
            } else {
                if (v36 == 52) {
                    if (*(int*)(T + 4) == 2) {
                        if (type == 243 && *(int*)(T + 96) > 14) {
                            *(int*)(T + 244) = 5;
                        } else if (type == 223) {
                            *(int*)(T + 244) = 5;
                        } else {
                            *(int*)(T + 244) = 2;
                        }
                        FUN_0046d840(1249, posPtr, posPtr,
                                     (float*)(T + 28), 6, Target, 30.0f, -1, 0);
                    } else {
                        *T = 0;
                        FUN_004660f0(posPtr, (char)1);
                    }
                    return;
                }
                // v36 != 51 and v36 != 52 — generic hit
                *T = 0;
                int rs = rand();
                PlayBuffer(rs % 7 + 50, (DWORD)Target, 0);
                if (type != 223 && type != 243) return;
            }
            FUN_004660f0(posPtr, (char)1);
        }
    }
    else {
        // ── Path B: AOE — scan all entities within 100 units ──────────────
        int owner = *(int*)(T + 252);
        char* charBase = (char*)(uintptr_t)DAT_07abf5d0;  // CharactersClient
        const int kEntStride = 916;
        char* found = nullptr;
        for (int idx = 0; idx < 400; ++idx) {
            char* ent = charBase + idx * kEntStride;
            if ((int)(uintptr_t)ent == owner) continue;
            if (*ent == 0) continue;
            // visibility flag at +332
            if (*(unsigned char*)(ent + 332 + 20) == 0) continue;
            // skip hero
            if (ent == (char*)(uintptr_t)DAT_07abf5d8) continue;
            // skip if "dead" flag at +745+20
            if (*(unsigned char*)(ent + 745 + 20) != 0) continue;
            // distance check (XY only, sqrt <= 100)
            float dx = *(float*)(T + 16) - *(float*)(ent + 16);
            float dy = *(float*)(T + 20) - *(float*)(ent + 20);
            if (sqrtf(dx*dx + dy*dy) > 100.0f) continue;
            found = ent;
            break;
        }
        if (!found) return;

        // Anti-tamper hash table — skipped

        unsigned char skillIdx = *(unsigned char*)(T + 133);
        DWORD v39 = (DWORD)*(unsigned char*)(skillIdx + ca + 87);

        // Anti-tamper hash table — skipped

        float* posPtr = (float*)(T + 16);
        if (v39 != 51 && v39 == 52) {
            if (*(int*)(T + 4) == 2) {
                FUN_0046d840(1249, posPtr, posPtr,
                             (float*)(T + 28), 6, Target, 30.0f, -1, 0);
            } else {
                *T = 0;
                FUN_004660f0(posPtr, (char)1);
            }
        } else {
            short type = *(short*)(T + 2);
            *T = 0;
            if (type == 223 || type == 243) {
                FUN_004660f0(posPtr, (char)1);
            }
        }
    }
}

// FUN_00470030 @ 0x00470030 — MoveJoint(entity_ptr, frame_id)    [Kayito: MoveJoint]
// Implemented in Render/MoveJoint.cpp

// FUN_00511bf0 @ 0x00511BF0 — sets *param_1 = param_2, param_1[1] = (DWORD)param_3
// Signature from decompile: (undefined4 *param_1, undefined4 param_2, undefined4 param_3)
void __cdecl FUN_00511bf0(float* param_1, float param_2, int param_3) {
    // Match Ghidra: *param_1 = param_2; param_1[1] = param_3
    *param_1 = param_2;
    *(int*)(param_1 + 1) = param_3;
}

// FUN_004f9e90 — implemented below (EulerToMatrix)
// FUN_00465fe0 @ 0x00465FE0 — Joint_BoneOffsetApply(entity_ptr, flag)
// If flag != 0: builds rotation matrix from euler (+0x1c..0x24), transforms +0xc0 offset,
//   adds result to world pos (+0x10/+0x14/+0x18).
// If flag == 0: directly adds +0xc0/+0xc4/+0xc8 to world pos.
void __cdecl FUN_00465fe0(int param_1, int param_2) {
    // PORT FIX: Ghidra decompile split a contiguous float[3] output buffer into
    // three separate locals (local_3c/38/34). MSVC does not guarantee they're
    // adjacent in memory, so FUN_004fa0b0 (which writes 3 contiguous floats)
    // only landed in local_3c and the other two reads picked up uninitialised
    // stack. Use a proper array to guarantee contiguity. Same pattern as the
    // Terrain_Light.cpp FUN_004fa930 fix.
    float out[3] = {0.0f, 0.0f, 0.0f};
    float local_30[12];
    if (param_2 != 0) {
        float local_48 = *(float*)(param_1 + 0x1c);
        // local_44 = *(float*)(param_1 + 0x20); local_40 = *(int*)(param_1 + 0x24);
        FUN_004f9db0((float*)(param_1 + 0x1c), local_30);
        FUN_004fa0b0((float*)(param_1 + 0xc0), local_30, out);
        *(float*)(param_1 + 0x10) += out[0];
        *(float*)(param_1 + 0x14) += out[1];
        *(float*)(param_1 + 0x18) += out[2];
    } else {
        *(float*)(param_1 + 0x10) += *(float*)(param_1 + 0xc0);
        *(float*)(param_1 + 0x14) += *(float*)(param_1 + 0xc4);
        *(float*)(param_1 + 0x18) += *(float*)(param_1 + 200);
    }
}

// FUN_004f76c0 @ 0x004F76C0 — Terrain_BlendLightSphere(cx, cy, src3f, radius, dst_buf)
// Blends a spherical gradient from a 3-float source colour into dst_buf
// (indexed as [row&0xff * 0x100 + col&0xff] * 0xc, i.e. 3×float per tile).
void __cdecl FUN_004f76c0(float param_1, float param_2, int param_3, int param_4, int param_5) {
    float cx   = param_1 * _DAT_00552594;
    float cy   = param_2 * _DAT_00552594;
    int   icx  = (int)cx;
    int   icy  = (int)cy;
    int   rMin = icy - param_4,  rMax = icy + param_4;
    if (rMin > rMax) return;
    unsigned int uRow = (unsigned int)rMin;
    for (int row = rMin; row <= rMax; row++, uRow++) {
        float fRow = (float)row;
        float fDy  = cy - fRow;
        int   cMin = icx - param_4, cMax = param_4 + icx;
        for (int col = cMin; col <= cMax; col++) {
            float fDx  = cx - (float)col;
            float fVal = ((float)param_4 - sqrtf(fDx * fDx + fDy * fDy)) / (float)param_4;
            if (_DAT_00552580 >= fVal) continue;
            float *dst = (float*)(param_5 + ((int)((uRow & 0xff) * 0x100 + ((unsigned int)col & 0xff))) * 0xc);
            int   off  = param_3 - (int)dst;
            for (int k = 0; k < 3; k++) {
                float fv = fVal * *(float*)((int)dst + off) + *dst;
                *dst = (fv < _DAT_00552580) ? 0.0f : fv;
                dst++;
            }
        }
    }
}
// FUN_0045fec0 @ 0x0045FEC0 — Entity_FindNearby_SendPacket
// Scans up to 400 entities for those within radius param_3 of world pos param_2.
// Collects up to 5 entity IDs matching type/team filter (param_4/param_5).
// If any found, sends C1-0x1d packet with entity list XOR-encrypted.
// param_1  = char data stat slot index (indexes into DAT_07cf1ff4+0x57)
// param_2  = float[2] world position to search from
// param_3  = search radius
// param_4  = zone/flag byte written into packet
// param_5  = team/guild ID (for player-type entity matching)
// Returns 0 on no entities, 1 on packet sent or send error.
float* __cdecl FUN_0045fec0(unsigned int param_1, float* param_2, float param_3, int param_4, short param_5)
{
    static const BYTE xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    // 1. Read character stat byte for this slot
    BYTE uVar19 = *(BYTE*)((char*)DAT_07cf1ff4 + 0x57 + param_1);

    // 2. Scan entity array for nearby entities (max 400, collect up to 5)
    int count = 0;
    short nearbyIds[5] = {};
    char *pcEnt = (char *)DAT_07abf5d0;
    for (int i = 0; i < 400 && count < 5; i++, pcEnt += 0x394) {
        // Must be valid (byte[0] != 0) and visible (byte[0x160] != 0)
        if (pcEnt[0] == '\0') continue;
        if (pcEnt[0x160] == '\0') continue;
        // Must not be local player entity
        if (pcEnt == DAT_07abf5d8) continue;
        // Must have no active targeting lock (byte[0x12] == 0)
        if (pcEnt[0x12] != '\0') continue;

        // Distance check
        float dx = param_2[0] - *(float*)(pcEnt + 0x10);
        float dy = param_2[1] - *(float*)(pcEnt + 0x14);
        float dist = SQRT(dx*dx + dy*dy);
        if (dist >= param_3) continue;

        // Type filter: byte[0x84] == 2 (NPC/monster, always include)
        //              byte[0x84] == 1 (player, include only if team matches)
        char cType = pcEnt[0x84];
        short sTeam = *(short*)(pcEnt + 0x1dc);
        if (cType != '\x02' && !(cType == '\x01' && sTeam == param_5))
            continue;

        // Zone 8/9: mark entity for special AoE targeting
        if ((uVar19 == 8) || (uVar19 == 9)) {
            char anim = pcEnt[0x105];
            if (anim != 'M' && anim != 'K' && anim != 'I' &&
                anim != (char)-0x7d && anim != (char)-0x7c &&
                anim != (char)-0x7b && anim != (char)-0x7a)
                pcEnt[0x304] = '\n';
        }

        nearbyIds[count++] = *(short*)(pcEnt + 0x1dc);
    }

    if (count < 1)
        return (float*)(uintptr_t)0;

    // 3. Build XOR-encrypted packet [0xC1][len][0x1d][rand][seq][flags][count][ids...]
    BYTE pktBuf[32] = {};
    int pos = 0;
    auto xorAppend = [&](BYTE b) {
        if (pos < (int)sizeof(pktBuf)) {
            pktBuf[pos] = b ^ xorKey[pos & 0x1f] ^ (pos > 0 ? pktBuf[pos-1] : 0);
            pos++;
        }
    };
    pktBuf[0] = 0xC1;  pos = 1;  // header (plain)
    pktBuf[1] = 0;     pos = 2;  // length placeholder (plain)
    pktBuf[2] = 0x1d;  pos = 3;  // opcode (plain)
    xorAppend((BYTE)rand());           // rand byte
    xorAppend(DAT_05826ceb++);        // sequence counter
    xorAppend((BYTE)param_4);         // zone/flags
    xorAppend((BYTE)count);           // entity count
    for (int i = 0; i < count; i++)
        xorAppend((BYTE)(nearbyIds[i] >> 8));  // entity ID high byte

    pktBuf[1] = (BYTE)pos;  // fill length

    // 4. Encode and send
    BYTE sendBuf[64] = {};
    int encLen = FUN_0053cc30(0, pktBuf + 3, pos - 3);  // encode payload
    if (encLen < 0x100) {
        sendBuf[0] = 0xC3;
        sendBuf[1] = (BYTE)(encLen + 2);
        FUN_0053cc30((int)(sendBuf + 2), (BYTE*)(pktBuf + 3), pos - 3);
        int total = encLen + 2;
        if (DAT_055ca168 != INVALID_SOCKET) {
            int sent = send(DAT_055ca168, (char*)sendBuf, total, 0);
            if (sent == -1) {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK) { Net_Disconnect(((int)(uintptr_t)DAT_055ca160)); return (float*)(uintptr_t)1; }
                if ((int)(total + DAT_055cc16c) > 0x2000) { Net_Disconnect(((int)(uintptr_t)DAT_055ca160)); return (float*)(uintptr_t)1; }
                memcpy(DAT_055ca16c + DAT_055cc16c, sendBuf, total);
                DAT_055cc16c += total;
            }
        }
    }
    if (DAT_055ce174 != 0) FUN_0043de60();
    return (float*)(uintptr_t)1;
}

// FUN_0046fe90 @ 0x0046FE90 — Joint_SegmentTick(joint_ptr, mat)
// Pushes all existing vertex segments one position forward (scroll back),
// then computes 4 new billboard vertices (at ±half-width perpendicular offsets)
// using FUN_004fa0b0 and the weapon-scale constants _DAT_00552a14/_DAT_00552504.
void __cdecl FUN_0046fe90(int param_1, float *param_2) {
    // PORT FIX: Ghidra decompile produced `float local_18[4], local_8, local_4;`
    // and wrote FUN_004fa0b0's 3-float output at `local_18 + 3`, expecting
    // local_18[4]==local_8 and local_18[5]==local_4. MSVC doesn't guarantee that
    // layout, so local_8/local_4 reads picked up uninitialised stack. Expanding
    // the array to 6 elements makes the 3 output slots genuinely contiguous.
    // Slots used: local_18[0..2] = input vec, local_18[3..5] = output vec.
    float local_18[6] = {0};
    #define local_8 local_18[4]
    #define local_4 local_18[5]
    // scroll: shift all existing segments one step back
    int iVar4 = *(int*)(param_1 + 0x50) + 1;
    int iVar1 = *(int*)(param_1 + 0x54) - 1;
    *(int*)(param_1 + 0x50) = iVar4;
    if (iVar1 < iVar4) *(int*)(param_1 + 0x50) = iVar1;
    iVar1 = *(int*)(param_1 + 0x50);
    if (0 <= iVar1 - 1) {
        // 2026-08-10 FIX (haces oscuros saliendo del personaje): el port había
        // COLAPSADO los dos punteros del IDA en uno solo. El original lleva
        // `v6` = base de la fila (retrocede 0x30 por segmento) y `v7` = cursor
        // que camina esa fila; `v7` se RE-INICIALIZA desde `v6` en cada vuelta:
        //     v6 = 48*(count-1) + a1 + 136;
        //     do { v7 = v6; <4 × copiar vec3, v7 += 3>; v6 -= 48; } while(--v5);
        // Nosotros hacíamos `puVar3 += 48` en el inner y después `-= 0x78`,
        // o sea un paso neto de -72 en vez de -48: el cursor se corría 24 bytes
        // por vuelta y terminaba escribiendo POR DEBAJO del array de segmentos
        // (0x58), encima de la cabecera — segCount (0x50) y segMax (0x54)
        // quedaban con floats, y el render dibujaba vértices basura = los haces.
        char* rowBase = (char*)param_1 + 0x88 + (iVar1 - 1) * 0x30;
        do {
            unsigned int* puVar3 = (unsigned int*)rowBase;
            for (int k = 0; k < 4; k++) {
                puVar3[0] = puVar3[-0xc];
                puVar3[1] = puVar3[-0xb];
                puVar3[2] = puVar3[-10];
                puVar3 += 3;
            }
            rowBase -= 0x30;
            iVar1--;
        } while (iVar1 != 0);
    }
    // compute new tip vertices
    local_18[0] = *(float*)(param_1 + 0x0c) * _DAT_00552a14;
    local_18[1] = 0.0f; local_18[2] = 0.0f;
    FUN_004fa0b0(local_18, param_2, local_18 + 3);
    local_18[1] = 0.0f; local_18[2] = 0.0f;
    *(float*)(param_1 + 0x58) = local_18[3] + *(float*)(param_1 + 0x10);
    *(float*)(param_1 + 0x5c) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x60) = local_4    + *(float*)(param_1 + 0x18);
    local_18[0] = *(float*)(param_1 + 0x0c) * _DAT_00552504;
    FUN_004fa0b0(local_18, param_2, local_18 + 3);
    local_18[0] = 0.0f;
    *(float*)(param_1 + 100) = local_18[3] + *(float*)(param_1 + 0x10);
    local_18[1] = 0.0f;
    *(float*)(param_1 + 0x68) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x6c) = local_4    + *(float*)(param_1 + 0x18);
    local_18[2] = *(float*)(param_1 + 0x0c) * _DAT_00552a14;
    FUN_004fa0b0(local_18, param_2, local_18 + 3);
    local_18[0] = 0.0f;
    *(float*)(param_1 + 0x70) = local_18[3] + *(float*)(param_1 + 0x10);
    local_18[1] = 0.0f;
    *(float*)(param_1 + 0x74) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x78) = local_4    + *(float*)(param_1 + 0x18);
    local_18[2] = *(float*)(param_1 + 0x0c) * _DAT_00552504;
    FUN_004fa0b0(local_18, param_2, local_18 + 3);
    *(float*)(param_1 + 0x7c) = local_18[3] + *(float*)(param_1 + 0x10);
    *(float*)(param_1 + 0x80) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x84) = local_4    + *(float*)(param_1 + 0x18);
    #undef local_8
    #undef local_4
}

// FUN_0043e4a0 @ 0x0043E4A0 — MoveHumming(Position, Angle, TargetPosition, Turn)
// Gira Angle hacia el target y **devuelve la distancia** al target.
// NO mueve la posicion (de eso se encarga el tick generico del joint).
//
// 2026-08-16: el retorno FALTABA. Hex-Rays la tipa `void` porque el valor sale
// en st0 y no lo detecta — el mismo artefacto de FPU que ya mordio antes. El
// source original de MU 5.2 (ZzzAI.cpp:131) lo deja explicito:
//     float MoveHumming(...) { ...; return VectorLength(Range); }
// y el consumidor lo usa como distancia (ZzzEffectJoint.cpp:3368):
//     Distance = MoveHumming(...);
//     if (Distance <= 35.f)  { o->Live = false; ... }        // absorber
//     else if (Distance <= 70.f && ...) { Velocity -= 10; }  // frenar
// Sin el retorno, `MoveJoint` case 0x4ea comparaba la Z ABSOLUTA del target
// (ownerZ + 120, siempre > 35) → las esferas de EXP nunca se absorbian y
// quedaban orbitando al personaje acumulandose.
float __cdecl FUN_0043e4a0(float *param_1, float *param_2, float *param_3, float param_4)
{
    // Horizontal angle: from (pos.x, pos.y) to (target.x, target.y)
    float horizAngle = FUN_0043e050(param_1[0], param_1[1], param_3[0], param_3[1]);
    // Interpolate rot[2] (yaw) toward horizontal angle
    param_2[2] = FUN_0043e1b0(param_2[2], horizAngle, param_4);

    // Compute delta vector for vertical angle
    float dx = param_1[0] - param_3[0];
    float dy = param_1[1] - param_3[1];
    float dz = param_1[2] - param_3[2];
    float horizDist = sqrtf(dx * dx + dy * dy);

    // Vertical angle: from (pos.z, horizDist) to (target.z, 0)
    float vertAngle = FUN_0043e050(param_1[2], horizDist, param_3[2], 0.0f);
    // Interpolate rot[0] (pitch) toward (360 - vertAngle)
    param_2[0] = FUN_0043e1b0(param_2[0], _DAT_0055286c - vertAngle, param_4);

    // VectorLength(Range) — el valor de retorno de la funcion.
    float local[3] = { dx, dy, dz };
    return FUN_004f9c40(local);
}

// FUN_004e1be0 @ 0x004E1BE0 — RenderItem3D
//
// 2026-04-30: la versión anterior estaba MAL identificada como
// `ItemDrop_SpawnEffect` y llamaba `FUN_004e13a0(type+400, ...)` (RenderObjectScreen)
// con effect-ids inventados.  Para items "normales" (helmet=0x4E1, etc.) eso
// resolvía a un BMD inexistente y crasheaba en BMD_Animation con AV.
//
// El IDA companion confirma 0x004E1BE0 = RenderItem3D
// `(float sx, sy, Width, Height, int Type, Level, Option1, bool PickUp)`.
//
// Mientras no portemos el render 3D real con BMD models, redirigimos al
// placeholder 2D que vive en RenderItem3D (línea 26253 abajo) — dibuja un
// quad texturado con el icono del item type en la posición pasada.
// RenderItem3D forward decl already in functions.h (non-extern-C C++ linkage).

// 2026-05-08 BUG-FIX (item +N glow): el wrapper estaba pre-shifting `Level`
// con `>> 3 & 0x0F` antes de llamar `RenderItem3D`, pero per IDA
// `RenderItem3D` (0x004E1BE0) toma el RAW Level byte y lo pasa así a
// `RenderObjectScreen` (0x004E13A0) que internamente hace `Level = (ItemLevel
// >> 3) & 0xF`. Pre-shifting acá producía un DOUBLE-shift → para items +9
// (Level byte = 0x48), el valor llegaba a Entity_DrawSetup como 1 en vez
// de 9 → ItemLevel<3 → no entra en la rama de glow +9/+11 → items en
// inventario sin halo dorado/azul.
//
// Per IDA: pasamos raw Level. Entity_DrawSetup (línea 52 de su archivo)
// hace el shift una sola vez (la cadena solo shifteaba después).
void __cdecl FUN_004e1be0(float param_1, float param_2, float param_3, float param_4,
                          int param_5, unsigned int param_6, unsigned char param_7, char param_8)
{
    // Forward to RenderItem3D with RAW Level — downstream extracts the
    // ItemLevel via (Level >> 3) & 0xF in RenderObjectScreen / Entity_DrawSetup.
    RenderItem3D(param_1, param_2, param_3, param_4,
                 param_5, (int)param_6, (int)param_7, 0, param_8 != 0);
}

// FUN_00441e00 @ 0x00441E00 — BMD::RenderBodyTranslate
// Signature IDA: __thiscall(this, Flag, Alpha, BlendMesh, BlendMeshLight,
//                           BlendMeshTexCoordU, BlendMeshTexCoordV, HiddenMesh, Texture8)
// BlendMesh y HiddenMesh son INT pero los callers nuestros pasan como float
// (bit-pattern reinterpreted). Por eso usamos union para reinterpretar bits sin
// romper signatures.
//
// BUG-FIX 2026-04-28: lógica del branch NULL estaba INVERTIDA (skipping cuando
// debería render). IDA: `if (NULL && i != HiddenMesh) goto render;`. Y la
// comparación `i != HiddenMesh` debe ser INT, no float (NaN para -1, etc.).
void __cdecl FUN_00441e00(void *model, int flags, float f1, float f2, float f3, float f4, float f5, float f6, int rgba) {
    // BUG-FIX 2026-04-29: validar model + meshBase antes de iterar. Crash AV en
    // glPopMatrix con stack KernelBase+opengl32 venía de un FUN_00440d50 que
    // dereferenciaba un mesh pointer wild (VBO inválido).
    // BUG-FIX 2026-05-01: range check del pointer model. Algún caller pasa
    // direcciones tipo 0xE5E90005 (kernel space) → AV en glDrawElements / lectura
    // de model+0x24. User-space address válido es < 0x80000000 y > 0x100000.
    if (model == nullptr) return;
    if ((uintptr_t)model < 0x100000 || (uintptr_t)model >= 0x80000000) return;
    if (*(short*)((char*)model + 0x24) == 0) return;
    int meshBase_check = *(int*)((char*)model + 0x28);
    if (meshBase_check == 0 || (uintptr_t)meshBase_check < 0x100000) return;
    FUN_00440d30();
    if (*(char*)((char*)model + 0x44) == '\0') {
        // BUG-FIX 2026-04-26: IDA usa < 0.99f (_DAT_00552544), no < 1.0f.
        if (f1 < _DAT_00552544) glColor4f(*(float*)((char*)model+0x48),*(float*)((char*)model+0x4c),*(float*)((char*)model+0x50),f1);
        else glColor3fv((GLfloat*)((char*)model + 0x48));
    }
    // Reinterpret f6 bits as int for HiddenMesh comparison (IDA reads as int).
    int HiddenMesh = *(int*)&f6;
    int meshBase = *(int*)((char*)model + 0x28);
    int numMesh = (int)*(short*)((char*)model+0x24);
    for (int i = 0; i < numMesh; i++) {
        char* pcVar1 = *(char**)(meshBase + i * 0x28 + 0x24);
        float fVar3 = f2;  // BlendMesh (kept as float bit-pattern)
        bool render = false;
        if (pcVar1 == nullptr) {
            // IDA: NULL && i != HiddenMesh → render
            if (i != HiddenMesh) render = true;
        }
        else if ((pcVar1[1] == '\0') && (i != HiddenMesh)) {
            // IDA passes the override mesh index as a raw INT argument to sub_440D50.
            // Passing 1.0f/2.0f here breaks the later int-style blend comparison path.
            if (*pcVar1 != '\0') {
                unsigned int meshOverrideBits = (unsigned int)i;
                memcpy(&fVar3, &meshOverrideBits, sizeof(fVar3));
            }
            render = true;
        }
        // else: skip (mesh marked hidden or has [1]!='\0')
        if (render) {
            FUN_00440d50(model, (float)i, flags, f1, fVar3, f3, f4, f5, (unsigned int)rgba);
        }
    }
    glPopMatrix();
}

// FUN_00509810 @ 0x00509810 — Model_SetAnimSlots(slot_idx, s0, s1, s2, s3, s4)
// Writes 5 shorts into model slot at DAT_05828d58 + slot_idx * 0xbc + 0xaa.
void __cdecl FUN_00509810(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    int base = param_1 * 0xbc + DAT_05828d58;
    *(short*)(base + 0xaa) = (short)param_2;
    *(short*)(base + 0xac) = (short)param_3;
    *(short*)(base + 0xae) = (short)param_4;
    *(short*)(base + 0xb0) = (short)param_5;
    *(short*)(base + 0xb2) = (short)param_6;
}

// FUN_0045bfa0 (CreateCharacter), FUN_0045ccf0 (CreateMonster) moved to
// src/Monster/Monster.cpp (B3 refactor 2026-05-07, 925 lines).

// Sound
// FUN_00404bb0 @ 0x00404BB0 — returns 1 (Sound_BufferUnlock helper)
// Ghidra: undefined4 FUN_00404bb0(void) { return 1; }
// signature in functions.h: void (void) — no return used at call site, treat as void.
void __cdecl FUN_00404bb0(void) { /* NOP — original returns 1 but callers ignore it */ }

// FUN_00483160 @ 0x00483160 — Combat_GetTargetResult: resolves the current hovered entity result.
// Returns: high=entity slot (DAT_00559c50), low=action type (0=none, 1=attack, 2=trade).
// Also handles trade-mode comparisons (name vs local player name).
unsigned int __cdecl FUN_00483160(void) {
    unsigned int uVar6 = DAT_00559c50;
    if (DAT_00559c50 == 0xffffffff)
        return uVar6 & 0xffffff00u;
    int iVar1 = DAT_07abf5d0 + DAT_00559c50 * 0x394;
    char cVar2 = *(char*)(DAT_07abf5d0 + 0x84 + DAT_00559c50 * 0x394);
    uVar6 = (DAT_00559c50 * 0xe5u >> 8u) << 8u | (unsigned char)cVar2;
    if (cVar2 == '\x02')
        return ((DAT_00559c50 * 0xe5u >> 8u) << 8u) | 1u;
    if (cVar2 == '\x01') {
        unsigned char bVar3 = *(unsigned char*)(iVar1 + 0x2ea);
        unsigned int uVar8 = ((DAT_00559c50 * 0xe5u >> 8u) << 8u) | bVar3;
        if (DAT_05826d30 != '\0') {
            if (bVar3 > 5) {
                short sVar4 = *(short*)(iVar1 + 0x1da);
                uVar8 = (unsigned short)((DAT_00559c50 * 0xe5u) >> 0x10u) << 16u | (unsigned short)sVar4;
                if (sVar4 != -1) {
                    unsigned char *pbVar10 = (unsigned char*)&DAT_07e919bc + sVar4 * 0x50;
                    unsigned char *pbVar7  = (unsigned char*)&DAT_07e919bc + *(short*)((char*)DAT_07abf5d8 + 0x1da) * 0x50;
                    int cmp = 0;
                    while (true) {
                        unsigned char b1 = *pbVar7, b2 = *pbVar10;
                        if (b1 != b2) { cmp = (b1 < b2) ? -1 : 1; break; }
                        if (b1 == 0) { cmp = 0; break; }
                        pbVar7++; pbVar10++;
                    }
                    uVar8 = (unsigned int)cmp;
                    uVar6 = 0;
                    if (cmp == 0) goto LAB_0048324d;
                }
            }
            if (*(char*)(iVar1 + 0x2e9) != '\x02')
                return uVar8;
            if (iVar1 == (int)(uintptr_t)DAT_07abf5d8)
                return uVar8;
            return ((uVar8 >> 8u) << 8u) | 1u;
        }
        uVar6 = uVar8;
        if (bVar3 < 6) {
            SHORT SVar5 = GetAsyncKeyState(0x11);
            uVar6 = (unsigned int)SVar5;
            if (((char)(((unsigned short)SVar5) >> 8u) != (char)0x80) ||
                (iVar1 == (int)(uintptr_t)DAT_07abf5d8)) goto LAB_0048324d;
        }
        return ((uVar6 >> 8u) << 8u) | 1u;
    }
LAB_0048324d:
    return uVar6 & 0xffffff00u;
}
// FUN_004cb520 @ 0x004CB520 — `GetScreenWidth` per IDA companion (Offsets.h).
// Returns the "logical width" of the 3D world viewport based on which UI
// panel is open: 260 (right pane open) / 450 (right pane open, narrower
// content) / 640 (no panel — full width).
//
// 2026-04-30: Ghidra labelled this `SecondPassword_GetAnimFrame` because
// it reads DAT_07eaa117/116/118/119/11a/11b/11c — but per IDA's
// Offsets.h:59-69 those addresses ARE the UI-panel flags
// (InventoryOpened / CharacterOpened / ShopOpened / WarehouseOpened /
//  ChaosMixOpened / TradeOpened / EventWindowOpened). The magic values
// 0x280=640, 0x1c2=450, 0x104=260 confirm screen-width semantics.
//
// Body kept verbatim to original IDA decompile (matches the safe path
// of the anti-tamper hash-table-decorated original).
extern "C" int __cdecl GetScreenWidth(void);
int __cdecl FUN_004cb520(void) {
    return GetScreenWidth();
}

// Net PacketSession helpers
// SecondPassword screens (FUN_004e93a0 / 004df410 / 004e4760-004ec330) moved to
// src/Net/SecondPassword.cpp (B3 refactor 2026-05-07, ~1535 lines).

// Net_Connect @ 0x0043DC70 — connect socket to server (TCP) + arm WSAAsyncSelect.
// ctx layout:  +0x00 = HWND (msg target)   +0x08 = SOCKET
// wMsg        = Windows message ID for WSAAsyncSelect (WinMain/0x423920 pass 0x400 = WM_USER)
// Returns: 1 on success, 0 on failure (matches caller in Net_Connect.cpp:46).
int __cdecl Net_Connect(void* ctx, char* ip, unsigned short port, unsigned int wMsg)
{
    if (ctx == nullptr || ip == nullptr) return 0;

    // 1. Resolve IP (dotted or hostname)
    unsigned long addr = inet_addr(ip);
    if (addr == INADDR_NONE) {
        HOSTENT* h = gethostbyname(ip);
        if (h == nullptr || h->h_addr_list == nullptr || h->h_addr_list[0] == nullptr) {
            char buf[160];
            wsprintfA(buf, "gethostbyname failed for %.64s (WSA=%d)", ip, WSAGetLastError());
            FUN_00405540(&DAT_055c9bf0, buf);
            return 0;
        }
        addr = *(unsigned long*)h->h_addr_list[0];
    }

    // 2. Ensure socket exists (FUN_0043dbf0 already created one into ctx+8)
    SOCKET s = *(SOCKET*)((char*)ctx + 8);
    if (s == INVALID_SOCKET || s == 0) {
        s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) {
            FUN_00405540(&DAT_055c9bf0, "socket() failed");
            return 0;
        }
        *(SOCKET*)((char*)ctx + 8) = s;
    }

    // 3. Put socket in non-blocking mode so connect() returns WSAEWOULDBLOCK
    //    instead of blocking the game thread. WSAAsyncSelect implicitly switches
    //    to non-blocking; we keep the ordering the binary uses (connect first, then async).
    sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    sa.sin_addr.S_un.S_addr = addr;

    // Arm async notifications BEFORE connect so FD_CONNECT is delivered.
    HWND hWnd = *(HWND*)ctx;
    if (hWnd != nullptr) {
        WSAAsyncSelect(s, hWnd, wMsg, FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE);
    }

    int r = connect(s, (sockaddr*)&sa, sizeof(sa));
    if (r == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            char buf[160];
            wsprintfA(buf, "connect(%.64s:%u) failed (WSA=%d)", ip, (unsigned)port, err);
            FUN_00405540(&DAT_055c9bf0, buf);
            return 0;
        }
        // WSAEWOULDBLOCK = connect in progress; FD_CONNECT will fire later.
    }

    // NetCtx.socket_is_valid = 1 (the original sets a flag at +0xC or so)
    // Leave as-is; Net_Recv / Net_Process already drive the socket.
    return 1;
}
