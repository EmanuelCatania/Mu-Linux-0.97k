// SMD_Parser.cpp
//
// Pese al nombre, es un módulo heterogéneo: el puente OpenSMDFile y los stubs
// de conversión SMD (FixupSMD, SMD2BMDModel, SMD2BMDAnimation; el 0.97k sólo
// trae .bmd), OpenModels, y helpers sueltos de render/efectos/red
// (ClearCharacters, Effect_CollisionCheck, AddTerrainLight, Joint_SegmentTick,
// MoveHumming, BMD__RenderBody, CheckAttack, Net_Connect, ...).

#include "stdafx.h"
#include "UI/HealthBar.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include "Party/Party.h"

extern void __cdecl operator_delete(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
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
// Puente C-linkage hacia OpenSMDFile (C++), implementado en
// Core/Runtime_Medium.cpp (devuelve false: los SMD no se distribuyen con el
// 0.97k).  Se castea el const char* porque la firma real es `char*`.
extern bool __cdecl OpenSMDFile(char *FileName, int Type, bool Flip);  // impl C++ en Core/Runtime_Medium.cpp
extern "C" bool __cdecl OpenSMDFile(const char* FileName, int Type, char Flip) {
    return OpenSMDFile(const_cast<char*>(FileName), Type, (bool)Flip);
}
extern "C" void __cdecl FixupSMD(void) { /* SMD post-process — not ported */ }
extern "C" void __cdecl SMD2BMDModel(int /*ID*/, int /*Actions*/) { /* convert — not ported */ }
extern "C" void __cdecl SMD2BMDAnimation(int /*ID*/, char /*LockPosition*/) { /* convert anim — not ported */ }
// OpenModels @ 0x00506050 — OpenModels(Model, FileName, i)
// Port FIEL del IDA: construye "prefix01.smd" (i<10) o "prefix11.smd" (i>=10),
// llama OpenSMDModel + OpenSMDAnimation. Mismo no-op silencioso si SMD no
// existe en filesystem.
void __cdecl OpenModels(int Model, const char* FileName, int i) {
    char Buffer[256];
    if (i >= 10) {
        crt_sprintf(Buffer, "%s%d.smd", FileName, i);
    } else {
        crt_sprintf(Buffer, "%s0%d.smd", FileName, i);
    }
    OpenSMDModel(Model, Buffer, 1, 0);
    OpenSMDAnimation(Model, Buffer, 0);
}

// CRT file helpers
FILE* __cdecl crt_fopen(const char* path, const void* mode) {
    return fopen(path, (const char*)mode);
}
void __cdecl crt_fclose(FILE* f) {
    if (f) fclose(f);
}
void __cdecl putc(int ch, int *fp) {
    if (fp) fputc(ch, (FILE*)fp);
}

// OpenJPG (Texture_Load OZJ), OpenTGA (OpenTGA), UnloadImage (Texture_FreeSlot)
// viven en src/Render/Texture/Texture.cpp.

// CWsctlc_Startup @ 0x0043DB30 — Net_WSAStartup(__fastcall int param_1)
// Initialises WinSock 2.2. On success: stores wVersion low-word at param_1+4,
// clears param_1+8, returns 1. On failure: logs error, shows MessageBox, returns 0.
// IDA: CWsctlc::Startup (0x0043DB30)
void __cdecl CWsctlc_Startup(int param_1) {
    WSADATA wsaData;
    int r = WSAStartup(0x202, &wsaData);
    if (r != 0) {
        CErrorReport_Write(&DAT_055c9bf0, "Winsock DLL Initialize error");
        MessageBoxA(NULL, "Winsock error", "IError", 0);
        return;
    }
    if (((char)wsaData.wVersion == '\x02') && ((char)(wsaData.wVersion >> 8) == '\x02')) {
        *(unsigned int*)(param_1 + 8) = 0;
        *(unsigned int*)(param_1 + 4) = wsaData.wVersion & 0xffff;
        CWsctlc__LogPrintOn();
    } else {
        WSACleanup();
        CErrorReport_Write(&DAT_055c9bf0, "Winsock version low");
        MessageBoxA(NULL, "Winsock version error", "IError", 0);
    }
}

// CWsctlc_Create @ 0x0043DBF0 — Net_CreateSocket(__thiscall void *this, int param_1)
// Creates TCP socket, stores in *(this+8). Logs and shows MessageBox on failure.
// IDA: CWsctlc::Create (0x0043DBF0)
void __cdecl CWsctlc_Create(void* ctx, int param_1) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    *(SOCKET*)((char*)ctx + 8) = s;
    g_bGameServerConnected = 0;
    if (s == INVALID_SOCKET) {
        char buf[128];
        int err = WSAGetLastError();
        wsprintfA(buf, "Socket error: %d", err);
        CErrorReport_Write(&DAT_055c9bf0, buf);
        MessageBoxA(NULL, buf, "IError", 0);
        return;
    }
    *(DWORD*)ctx = (DWORD)param_1;
}

// IDA: SetCharacterScale (0x0045C050)
// Sets entity speed (+0x0c) based on move_type_flags (+0x1bc) and swim flag (+0x1bd).
// Also sets +0x1e0 to anim speed table index for vehicle entities.
void __cdecl SetCharacterScale(int param_1) {
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

// crt_fprintf @ 0x00543274 — fprintf wrapper.
// IDA: int fprintf(FILE* Stream, const char* Format, ...). Original wraps
// _lock_file/_stbuf/_output/_ftbuf/_unlock_file. Equivalent to plain fprintf.
// Only call site (line ~1835) passes 2 args (file, format string with no
// variadic args), so the simple 2-arg form is safe.
void __cdecl crt_fprintf(void* param_1, void* param_2) {
    if (!param_1 || !param_2) return;
    fprintf((FILE*)param_1, "%s", (const char*)param_2);
}

// BMD__Release @ 0x00442090 — BMD_FreeModel(model_ptr)
// Frees all bone mesh/action/texture data from a model slot at param_1.
// Bones: stride 0x8c, sub-meshes: stride 0x28, actions: stride 0x10, textures via Texture_Unload.
void __cdecl BMD__Release(int param_1) {
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
            UnloadImage(texId);
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

// DeleteObjects @ 0x004FFD50 — Terrain_ResetObjects
// Calls BMD_FreeModel on every model slot (stride 0xbc, count 0x7580/0xbc).
// Then walks the 16x16 scene-entity grid (DAT_083a0218, stride 0x10) freeing nodes via Entity_GridUnlink.
// Finally unloads tile textures 0x23-0x67, clears particle/effect/entity pools.
void __cdecl DeleteObjects(void) {
    // free all model slots
    for (int i = 0; i < 0x7580; i += 0xbc)
        BMD__Release(i + DAT_05828d58);

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
                Entity_GridUnlink((void*)head, (int)puVar5);
                head = next;
            }
            *(DWORD*)(puVar5 + 4) = 0;
            *(DWORD*)(puVar5 + 8) = 0;
            puVar5 += 0x10;
        }
        if (gridEnd < puVar5) {
            // unload tile textures
            for (int ti = 0x23; ti < 0x68; ti++) UnloadImage(ti);
            // ── Pool zero-clear loops (DESACTIVADOS, salvo Operates) ─────────────────
            // DESVIACION: el binario limpia acá 9 pools de partículas/efectos/entidades
            // recorriendo direcciones ABSOLUTAS de su .bss (0x07c85890..0x83a3fe8, más
            // literales como `(char*)0x83a2e90` y `(char*)0x7c5ab30`), que en este build
            // no son válidas.  Esos clears no se portaron; si se portan, acotar con el
            // sizeof de cada símbolo.
            //
            // La de `Operates` (DAT_083a2370) SI hay que limpiarla.
            // IDA la borra aca (`v12 = &unk_83A2370; do { *v12 = 0; v12 += 12; }`)
            // y es la lista de objetos interactuables que arma `sub_4FF580` desde
            // CreateObject.  Sin el clear, al cambiar de mapa quedan punteros a
            // objetos ya liberados y el picker (sub_4B0240) los deferencia; ademas
            // la lista se llena y los objetos del mapa nuevo no entran.
            // El array es real y esta dimensionado, asi que se acota con sizeof en
            // vez del bound absoluto del binario.
            memset(DAT_083a2370, 0, sizeof(DAT_083a2370));
            return;
        }
    } while (true);
}

// ClearCharacters @ 0x0045ABB0 — Entity_ClearByType(map_id)
// Loops over entity array (base DAT_07abf5d0, stride 0x394).
// For each active entity whose type (+0x1dc) != map_id: clears active flag,
// also clears matching emitter pool entries (DAT_083a1218, stride 0x1bc).
// Then calls DeleteCloth on every slot.
//
// Cota del bucle interno: el binario usa el literal 0x83a2370 (= DAT_083a1218
// + 0x1158, fin del array Butterfles); acá el array vive en otra dirección,
// así que se usa DAT_083a1218 + 0x1158 (end-pointer real).
void __cdecl ClearCharacters(int param_1) {
    gHealthBar.Clear();
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
        DeleteCloth((int)puVar1, (int)puVar1, 0);
    }

    // Gate/map transition removes the viewport but not Party membership.
    Party_RefreshViewportLinks();
}

// Effect/particle
// MoveEffect @ 0x00466AD0 — MoveEffect: implemented in Render/MoveEffect.cpp
// Effect_SpawnSmokeBurst @ 0x004660F0 — Effect_SmokeBurst: implemented in Render/MoveEffect_Helpers.cpp
// Effect_SpawnSmokeExplosion @ 0x004661F0 — Effect_SmokeExplosion: implemented in Render/MoveEffect_Helpers.cpp
// Effect_SpawnLightningBurst @ 0x00460C30 — Effect_LightningBurst: implemented in Render/MoveEffect_Helpers.cpp
// Effect_SpawnProximityHit @ 0x00465E60 — Effect_OnHitProximity: implemented in Render/MoveEffect_Helpers.cpp
// Ring_ComputeOrbit @ 0x00473D90 — Ring_ComputeOrbit: implemented in Render/MoveEffect_Helpers.cpp
//
// sub_466440 @ 0x00466440 — Effect/projectile collision/trigger handler.
// Port fiel del decompile de IDA; se omite el ruido anti-tamper de hash-table
// (wrappers encrypt/decrypt de CharacterMachine).
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
// Effect_SpawnSmokeBurst declared in functions.h as (float*, char). Using through normal
// linkage (no extern decl needed here).

void __cdecl Effect_CollisionCheck(int Target) {
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
        if (Entity_FindNearby_SendPacket((unsigned int)skillIdx, posPtr, 100.0f,
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
                        Joint_Create(1249, posPtr, posPtr,
                                     (float*)(T + 28), 6, Target, 30.0f, -1, 0);
                    } else {
                        *T = 0;
                        Effect_SpawnSmokeBurst(posPtr, (char)1);
                    }
                    return;
                }
                // v36 != 51 and v36 != 52 — generic hit
                *T = 0;
                int rs = rand();
                PlayBuffer(rs % 7 + 50, (DWORD)Target, 0);
                if (type != 223 && type != 243) return;
            }
            Effect_SpawnSmokeBurst(posPtr, (char)1);
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
                Joint_Create(1249, posPtr, posPtr,
                             (float*)(T + 28), 6, Target, 30.0f, -1, 0);
            } else {
                *T = 0;
                Effect_SpawnSmokeBurst(posPtr, (char)1);
            }
        } else {
            short type = *(short*)(T + 2);
            *T = 0;
            if (type == 223 || type == 243) {
                Effect_SpawnSmokeBurst(posPtr, (char)1);
            }
        }
    }
}

// MoveJoint @ 0x00470030 — MoveJoint(entity_ptr, frame_id)    [Kayito: MoveJoint]
// Implemented in Render/MoveJoint.cpp

// TEXCOORD @ 0x00511BF0 — sets *param_1 = param_2, param_1[1] = (DWORD)param_3
// Signature from decompile: (undefined4 *param_1, undefined4 param_2, undefined4 param_3)
void __cdecl TEXCOORD(float* param_1, float param_2, int param_3) {
    // Match Ghidra: *param_1 = param_2; param_1[1] = param_3
    *param_1 = param_2;
    *(int*)(param_1 + 1) = param_3;
}

// EulerToMatrix — implemented below (EulerToMatrix)
// Joint_BoneOffsetApply @ 0x00465FE0 — Joint_BoneOffsetApply(entity_ptr, flag)
// If flag != 0: builds rotation matrix from euler (+0x1c..0x24), transforms +0xc0 offset,
//   adds result to world pos (+0x10/+0x14/+0x18).
// If flag == 0: directly adds +0xc0/+0xc4/+0xc8 to world pos.
void __cdecl Joint_BoneOffsetApply(int param_1, int param_2) {
    // Ghidra separó un float[3] de salida en tres locales (local_3c/38/34) y
    // Vector_Rotate escribe 3 floats contiguos: se usa un array real.
    float out[3] = {0.0f, 0.0f, 0.0f};
    float local_30[12];
    if (param_2 != 0) {
        float local_48 = *(float*)(param_1 + 0x1c);
        // local_44 = *(float*)(param_1 + 0x20); local_40 = *(int*)(param_1 + 0x24);
        Matrix_BuildFromEuler((float*)(param_1 + 0x1c), local_30);
        Vector_Rotate((float*)(param_1 + 0xc0), local_30, out);
        *(float*)(param_1 + 0x10) += out[0];
        *(float*)(param_1 + 0x14) += out[1];
        *(float*)(param_1 + 0x18) += out[2];
    } else {
        *(float*)(param_1 + 0x10) += *(float*)(param_1 + 0xc0);
        *(float*)(param_1 + 0x14) += *(float*)(param_1 + 0xc4);
        *(float*)(param_1 + 0x18) += *(float*)(param_1 + 200);
    }
}

// AddTerrainLight @ 0x004F76C0 — suma una esfera de luz al buffer de luz del
// terreno.  dst se indexa como [(row & 0xff) * 0x100 + (col & 0xff)] * 3 floats.
//
// A diferencia de AddTerrainLightClip (0x004F7800) esta NO clampea a 1.0: solo
// evita valores negativos, que es lo que produce el resplandor del fuego.
void __cdecl AddTerrainLight(float xf, float yf, float *Light, int Range, float *Buffer) {
    float cx   = xf * _DAT_00552594;
    float cy   = yf * _DAT_00552594;
    int   icx  = (int)cx;
    int   icy  = (int)cy;
    int   rMin = icy - Range,  rMax = icy + Range;
    if (rMin > rMax) return;
    unsigned int uRow = (unsigned int)rMin;
    for (int row = rMin; row <= rMax; row++, uRow++) {
        float fRow = (float)row;
        float fDy  = cy - fRow;
        int   cMin = icx - Range, cMax = Range + icx;
        for (int col = cMin; col <= cMax; col++) {
            float fDx  = cx - (float)col;
            float fVal = ((float)Range - sqrtf(fDx * fDx + fDy * fDy)) / (float)Range;
            if (_DAT_00552580 >= fVal) continue;
            float *dst = Buffer + ((uRow & 0xff) * 0x100 + ((unsigned int)col & 0xff)) * 3;
            for (int k = 0; k < 3; k++) {
                float fv = fVal * Light[k] + dst[k];
                dst[k] = (fv < _DAT_00552580) ? 0.0f : fv;
            }
        }
    }
}
// Entity_FindNearby_SendPacket @ 0x0045FEC0 — Entity_FindNearby_SendPacket
// Scans up to 400 entities for those within radius param_3 of world pos param_2.
// Collects up to 5 entity IDs matching type/team filter (param_4/param_5).
// If any found, sends C1-0x1d packet with entity list XOR-encrypted.
// param_1  = char data stat slot index (indexes into DAT_07cf1ff4+0x57)
// param_2  = float[2] world position to search from
// param_3  = search radius
// param_4  = zone/flag byte written into packet
// param_5  = team/guild ID (for player-type entity matching)
// Returns 0 on no entities, 1 on packet sent or send error.
float* __cdecl Entity_FindNearby_SendPacket(unsigned int param_1, float* param_2, float param_3, int param_4, short param_5)
{
    // 1. Read character stat byte for this slot
    BYTE uVar19 = *(BYTE*)((char*)DAT_07cf1ff4 + 0x57 + param_1);

    // IDA sub_45FEC0 L157-195.  El bucle ancla en `v16 = CharactersClient + 747`
    // y todos los campos salen relativos a ese puntero:
    //     *(v16 - 747)          -> ent + 0     (activo)
    //     *(float *)(v16 - 731) -> ent + 16    (x)
    //     *(float *)(v16 - 727) -> ent + 20    (y)
    //     *(v16 - 395)          -> ent + 352   (visible, 0x160)
    //     v16 - 747 != Hero
    //     v16[18]               -> ent + 765   (0x2FD, dead_flag)
    //     *(v16 - 615)          -> ent + 132   (Kind: 2 = monstruo, 1 = jugador)
    //     *(_WORD *)(v16 - 271) -> ent + 476   (Key)
    //     *v16                  -> ent + 747   (tipo de monstruo)
    //     v16[25]               -> ent + 772
    //
    // El port habia tomado `v16[18]` como offset 18 desde la BASE de la entidad
    // en vez de desde `v16` (= base + 747), o sea leia `ent + 18`, que cae en
    // medio del float Position[0].  Para cualquier X de mundo realista ese byte
    // es != 0, asi que el `continue` descartaba TODAS las entidades y el barrido
    // devolvia count = 0 siempre — por eso el 0x1D no salia nunca y ningun skill
    // multi-objetivo hacia dano.  Mismo error en el bloque 8/9, que leia
    // `ent + 0x105` (CurrentAction) donde IDA lee `*v16` = ent + 747.
    int count = 0;
    short nearbyIds[5] = {};
    char *pcEnt = (char *)DAT_07abf5d0;
    for (int i = 0; i < 400 && count < 5; i++, pcEnt += 0x394) {
        if (pcEnt[0] == '\0') continue;                    // IDA: *(v16 - 747)
        if (pcEnt[0x160] == '\0') continue;                // IDA: *(v16 - 395)
        if (pcEnt == DAT_07abf5d8) continue;               // IDA: v16 - 747 != Hero
        if (pcEnt[765] != '\0') continue;                   // IDA: v16[18] = ent + 765

        // IDA L160-163: v17 = a2[1] - ent.y ; v19 = a2[0] - ent.x ; sqrt <= a3
        float dx = param_2[0] - *(float*)(pcEnt + 0x10);
        float dy = param_2[1] - *(float*)(pcEnt + 0x14);
        if (SQRT(dx*dx + dy*dy) > param_3) continue;

        // IDA L170-171: Kind == 2 (monstruo) o Kind == 1 (jugador) con Key == a5
        char cType = pcEnt[0x84];                          // IDA: *(v16 - 615)
        short sKey  = *(short*)(pcEnt + 0x1dc);            // IDA: *(_WORD *)(v16 - 271)
        if (cType != '\x02' && !(cType == '\x01' && sKey == param_5))
            continue;

        // IDA L173-180: marca de AOE para los grupos de skill 8 y 9.
        if ((uVar19 == 8) || (uVar19 == 9)) {
            char monsterType = pcEnt[747];                 // IDA: *v16
            if (monsterType != 77 && monsterType != 75 && monsterType != 73 &&
                monsterType != (char)-125 && monsterType != (char)-124 &&
                monsterType != (char)-123 && monsterType != (char)-122)
                pcEnt[772] = '\n';                         // IDA: v16[25] = 10
        }

        nearbyIds[count++] = sKey;                         // IDA L181
    }

    if (count < 1)
        return (float*)(uintptr_t)0;

    // IDA L197-218: si el nombre del heroe contiene "webzen" (aWebzen_2,
    // 0x559B80) sale sin mandar el 0x1D.  Exencion de las cuentas de Webzen.
    if (DAT_07abf5d8 && strstr((const char*)DAT_07abf5d8 + 449, "webzen"))
        return (float*)(uintptr_t)1;

    // 3. Paquete C1:1D — PMSG_MULTI_SKILL_ATTACK_RECV.
    //
    // IDA sub_45FEC0 anexa, en este orden (L228, 277, 326, 374, 427 y el bucle
    // de L476/L525):
    //     [0x1D]
    //     v111 = *(BYTE *)(CharacterAttribute + a1 + 87)   // skill
    //     v29  = (int)(a2[0] * 0.01)                       // x  (grilla)
    //     v32  = (int)(a2[1] * 0.01)                       // y
    //     a4                                               // serial
    //     LOBYTE(v109) = count
    //     por entidad:  [id >> 8][(BYTE)id]                // index[2] big-endian
    //
    // Coincide 1:1 con el server (GameServer/SkillManager.h:67):
    //     struct PMSG_MULTI_SKILL_ATTACK_RECV { PBMSG_HEAD header; BYTE skill;
    //                                           BYTE x; BYTE y; BYTE serial; BYTE count; };
    //     struct PMSG_MULTI_SKILL_ATTACK      { BYTE index[2]; };
    //
    // Este es el paquete que cierra los skills multi-objetivo (Penetration,
    // Twisting Slash, Rageful Blow, Death Stab, Hell Fire, Twister, Evil Spirit,
    // Aqua Beam, Blast, Inferno, Flame, Fire Slash): el C3:1E solo arma
    // `MultiSkillIndex` y es el 0x1D el que trae la lista de blancos y dispara
    // gAttack.Attack().
    BYTE pkt[3 + 5 + 5 * 2];
    int len = 0;
    pkt[len++] = 0xC1;
    pkt[len++] = 0;                                   // tamanio, se rellena abajo
    pkt[len++] = 0x1D;
    pkt[len++] = uVar19;                              // IDA: v111 (skill del slot)
    pkt[len++] = (BYTE)(int)(param_2[0] * 0.01f);     // IDA: v29
    pkt[len++] = (BYTE)(int)(param_2[1] * 0.01f);     // IDA: v32
    pkt[len++] = (BYTE)param_4;                       // IDA: a4 (serial)
    pkt[len++] = (BYTE)count;                         // IDA: LOBYTE(v109)
    for (int i = 0; i < count; i++) {
        pkt[len++] = (BYTE)((nearbyIds[i] >> 8) & 0xFF);   // IDA: v42
        pkt[len++] = (BYTE)(nearbyIds[i] & 0xFF);          // IDA: v45
    }
    pkt[1] = (BYTE)len;

    // El original arma la trama a mano (chain-XOR + serial + CSimpleModulus).
    // gNetwork.Send hace exactamente eso y ademas corrige el frame contra
    // HackPacketCheck.txt, que es el camino que ya usan todos los demas opcodes.
    gNetwork.Send(pkt, len);
    return (float*)(uintptr_t)1;
}

// Joint_SegmentTick @ 0x0046FE90 — Joint_SegmentTick(joint_ptr, mat)
// Pushes all existing vertex segments one position forward (scroll back),
// then computes 4 new billboard vertices (at ±half-width perpendicular offsets)
// using Vector_Rotate and the weapon-scale constants _DAT_00552a14/_DAT_00552504.
void __cdecl Joint_SegmentTick(int param_1, float *param_2) {
    // Ghidra produjo `float local_18[4], local_8, local_4;` y escribía la salida
    // de Vector_Rotate en `local_18 + 3`, esperando local_18[4]==local_8 y
    // local_18[5]==local_4: se expande el array a 6 para que sea contiguo.
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
        // IDA lleva DOS punteros: `v6` = base de la fila (retrocede 0x30 por
        // segmento) y `v7` = cursor que camina esa fila; `v7` se RE-INICIALIZA desde
        // `v6` en cada vuelta:
        //     v6 = 48*(count-1) + a1 + 136;
        //     do { v7 = v6; <4 × copiar vec3, v7 += 3>; v6 -= 48; } while(--v5);
        // No colapsarlos en uno: el paso neto por vuelta tiene que ser -48.
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
    Vector_Rotate(local_18, param_2, local_18 + 3);
    local_18[1] = 0.0f; local_18[2] = 0.0f;
    *(float*)(param_1 + 0x58) = local_18[3] + *(float*)(param_1 + 0x10);
    *(float*)(param_1 + 0x5c) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x60) = local_4    + *(float*)(param_1 + 0x18);
    local_18[0] = *(float*)(param_1 + 0x0c) * _DAT_00552504;
    Vector_Rotate(local_18, param_2, local_18 + 3);
    local_18[0] = 0.0f;
    *(float*)(param_1 + 100) = local_18[3] + *(float*)(param_1 + 0x10);
    local_18[1] = 0.0f;
    *(float*)(param_1 + 0x68) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x6c) = local_4    + *(float*)(param_1 + 0x18);
    local_18[2] = *(float*)(param_1 + 0x0c) * _DAT_00552a14;
    Vector_Rotate(local_18, param_2, local_18 + 3);
    local_18[0] = 0.0f;
    *(float*)(param_1 + 0x70) = local_18[3] + *(float*)(param_1 + 0x10);
    local_18[1] = 0.0f;
    *(float*)(param_1 + 0x74) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x78) = local_4    + *(float*)(param_1 + 0x18);
    local_18[2] = *(float*)(param_1 + 0x0c) * _DAT_00552504;
    Vector_Rotate(local_18, param_2, local_18 + 3);
    *(float*)(param_1 + 0x7c) = local_18[3] + *(float*)(param_1 + 0x10);
    *(float*)(param_1 + 0x80) = local_8    + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x84) = local_4    + *(float*)(param_1 + 0x18);
    #undef local_8
    #undef local_4
}

// MoveHumming @ 0x0043E4A0 — MoveHumming(Position, Angle, TargetPosition, Turn)
// Gira Angle hacia el target y **devuelve la distancia** al target.
// NO mueve la posicion (de eso se encarga el tick generico del joint).
//
// Hex-Rays la tipa `void` porque el valor sale en st0. El source original de
// MU 5.2 (ZzzAI.cpp:131) lo deja explicito:
//     float MoveHumming(...) { ...; return VectorLength(Range); }
// y el consumidor lo usa como distancia (ZzzEffectJoint.cpp:3368):
//     Distance = MoveHumming(...);
//     if (Distance <= 35.f)  { o->Live = false; ... }        // absorber
//     else if (Distance <= 70.f && ...) { Velocity -= 10; }  // frenar
float __cdecl MoveHumming(float *param_1, float *param_2, float *param_3, float param_4)
{
    // Horizontal angle: from (pos.x, pos.y) to (target.x, target.y)
    float horizAngle = CreateAngle(param_1[0], param_1[1], param_3[0], param_3[1]);
    // Interpolate rot[2] (yaw) toward horizontal angle
    param_2[2] = TurnAngle2(param_2[2], horizAngle, param_4);

    // Compute delta vector for vertical angle
    float dx = param_1[0] - param_3[0];
    float dy = param_1[1] - param_3[1];
    float dz = param_1[2] - param_3[2];
    float horizDist = sqrtf(dx * dx + dy * dy);

    // Vertical angle: from (pos.z, horizDist) to (target.z, 0)
    float vertAngle = CreateAngle(param_1[2], horizDist, param_3[2], 0.0f);
    // Interpolate rot[0] (pitch) toward (360 - vertAngle)
    param_2[0] = TurnAngle2(param_2[0], _DAT_0055286c - vertAngle, param_4);

    // VectorLength(Range) — el valor de retorno de la funcion.
    float local[3] = { dx, dy, dz };
    return Vec3_Length(local);
}

// RenderItem3D @ 0x004E1BE0 vive en src/Render/Render_LegacyLinker.cpp.

// BMD__RenderBody @ 0x00441E00 — BMD::RenderBodyTranslate
// Signature IDA: __thiscall(this, Flag, Alpha, BlendMesh, BlendMeshLight,
//                           BlendMeshTexCoordU, BlendMeshTexCoordV, HiddenMesh, Texture8)
// Port parcial: valida el modelo, hace BMD__BeginRender y fija el color
// (glColor4f si Alpha < 0.99); NO recorre ni dibuja las mallas.  En IDA el
// loop de mallas hace `if (NULL && i != HiddenMesh) goto render;`, con la
// comparación `i != HiddenMesh` como INT (no float).
void __cdecl BMD__RenderBody(void *model, int flags, float f1, int f2, float f3, float f4, float f5, int f6, int rgba) {
    // DESVIACION: validar el puntero model (rango user-space 0x100000..0x80000000)
    // y meshBase antes de usarlo; algunos callers pasaban punteros inválidos.
    if (model == nullptr) return;
    if ((uintptr_t)model < 0x100000 || (uintptr_t)model >= 0x80000000) return;
    if (*(short*)((char*)model + 0x24) == 0) return;
    int meshBase_check = *(int*)((char*)model + 0x28);
    if (meshBase_check == 0 || (uintptr_t)meshBase_check < 0x100000) return;
    BMD__BeginRender();
    if (*(char*)((char*)model + 0x44) == '\0') {
        // IDA usa < 0.99f (_DAT_00552544), no < 1.0f.
        if (f1 < _DAT_00552544) glColor4f(*(float*)((char*)model+0x48),*(float*)((char*)model+0x4c),*(float*)((char*)model+0x50),f1);
        else glColor3fv((GLfloat*)((char*)model + 0x48));
    }
    // HiddenMesh: entero (indice de malla a ocultar, o -1), como el a8 de IDA.
    int HiddenMesh = f6;
    int meshBase = *(int*)((char*)model + 0x28);
    int numMesh = (int)*(short*)((char*)model+0x24);
    for (int i = 0; i < numMesh; i++) {
        char* pcVar1 = *(char**)(meshBase + i * 0x28 + 0x24);
        int fVar3 = f2;  // BlendMesh (entero, como el a4 de IDA)
        bool render = false;
        if (pcVar1 == nullptr) {
            // IDA: NULL && i != HiddenMesh → render
            if (i != HiddenMesh) render = true;
        }
        else if ((pcVar1[1] == '\0') && (i != HiddenMesh)) {
            // IDA sub_441E00: `if (*v12) v11 = v10;` -- el indice de malla
            // pisa el BlendMesh recibido.
            if (*pcVar1 != 0) fVar3 = i;
            render = true;
        }
        // else: skip (mesh marked hidden or has [1]!='\0')
        if (render) {
            BMD__RenderMesh(model, (float)i, flags, f1, fVar3, f3, f4, f5, (unsigned int)rgba);
        }
    }
    glPopMatrix();
}

// SetMonsterSound @ 0x00509810 — Model_SetAnimationSlots(slot_idx, s0, s1, s2, s3, s4)
// Writes 5 shorts into model slot at DAT_05828d58 + slot_idx * 0xbc + 0xaa.
void __cdecl Model_SetAnimationSlots(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    int base = param_1 * 0xbc + DAT_05828d58;
    *(short*)(base + 0xaa) = (short)param_2;
    *(short*)(base + 0xac) = (short)param_3;
    *(short*)(base + 0xae) = (short)param_4;
    *(short*)(base + 0xb0) = (short)param_5;
    *(short*)(base + 0xb2) = (short)param_6;
}

// CreateCharacter, CreateMonster viven en src/Monster/Monster.cpp.

// SetHall (0x00404BB0) vive en src/Sound/Sound_DS3D.cpp.  Aca habia una segunda
// copia bajo el nombre FUN_00404bb0, sin callers: las dos son fieles (en el
// binario la funcion es un stub que devuelve 1), asi que solo sobraba el nombre.

// IDA: CheckAttack (0x00483160)
//
// This is deliberately a boolean predicate, despite the historical unsigned
// return type in functions.h.  Every caller uses it as one: the combat paths
// decide whether to retain a target key, and Cursor_Render selects the attack
// cursor.  The guild-war branch is controlled by EnableGuildWar and the
// entity's relation byte at +745 (2 = current war opponent), exactly as in
// the original client.
unsigned int __cdecl CheckAttack(void) {
    if (SelectedCharacter == -1) {
        return 0;
    }

    BYTE* target = (BYTE*)(uintptr_t)DAT_07abf5d0 + 916 * SelectedCharacter;
    const BYTE kind = target[132];
    if (kind == 2) {
        return 1;
    }
    if (kind != 1) {
        return 0;
    }

    const BYTE targetPkLevel = target[746];
    if (!EnableGuildWar) {
        return targetPkLevel >= 6 ||
               (((unsigned short)GetAsyncKeyState(VK_CONTROL) >> 8) == 0x80 &&
                target != (BYTE*)(uintptr_t)DAT_07abf5d8);
    }

    // In a war, a PK target from our own guild remains protected before the
    // opponent-relation test.  The original indexes the same 80-byte guild
    // name table used by GuildMark_AssociateEntities.
    if (targetPkLevel >= 6) {
        const short targetGuild = *(short*)(target + 474);
        if (targetGuild != -1) {
            const short heroGuild = *(short*)((BYTE*)(uintptr_t)DAT_07abf5d8 + 474);
            if (strcmp(DAT_07e919bc + 80 * heroGuild,
                       DAT_07e919bc + 80 * targetGuild) == 0) {
                return 0;
            }
        }
    }

    if (target[745] == 2 && target != (BYTE*)(uintptr_t)DAT_07abf5d8) {
        return 1;
    }

    return targetPkLevel;
}
// GetScreenWidth @ 0x004CB520 vive en src/Render/HUD_Pass2.cpp.
// SecondPassword screens (SecondPassword_Handler / 004df410 / 004e4760-004ec330)
// viven en src/Net/SecondPassword.cpp.

// Net_Connect @ 0x0043DC70 — connect socket to server (TCP) + arm WSAAsyncSelect.
// ctx layout:  +0x00 = HWND (msg target)   +0x08 = SOCKET
// wMsg        = Windows message ID for WSAAsyncSelect (WinMain/0x423920 pass 0x400 = WM_USER)
// Returns: 1 on success, 0 on failure (matches caller in Net/Net_Connect.cpp).
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
            CErrorReport_Write(&DAT_055c9bf0, buf);
            return 0;
        }
        addr = *(unsigned long*)h->h_addr_list[0];
    }

    // 2. Ensure socket exists (CWsctlc_Create already created one into ctx+8)
    SOCKET s = *(SOCKET*)((char*)ctx + 8);
    if (s == INVALID_SOCKET || s == 0) {
        s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) {
            CErrorReport_Write(&DAT_055c9bf0, "socket() failed");
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
        WSAAsyncSelect(s, hWnd, wMsg, FD_READ | FD_WRITE | FD_CLOSE);   // IDA 0x43DCD0: mascara 35 (0x23), sin FD_CONNECT
    }

    int r = connect(s, (sockaddr*)&sa, sizeof(sa));
    if (r == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            char buf[160];
            wsprintfA(buf, "connect(%.64s:%u) failed (WSA=%d)", ip, (unsigned)port, err);
            CErrorReport_Write(&DAT_055c9bf0, buf);
            return 0;
        }
        // WSAEWOULDBLOCK = connect in progress; FD_CONNECT will fire later.
    }

    // NetCtx.socket_is_valid = 1 (the original sets a flag at +0xC or so)
    // Leave as-is; Net_Recv / Net_Process already drive the socket.
    return 1;
}
