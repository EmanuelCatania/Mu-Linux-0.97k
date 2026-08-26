// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

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


// FUN_0050b510 — implemented in src/Monster/Monster_Data.cpp
// FUN_0047d020 — implemented in src/Monster/Monster_Data.cpp
// FUN_00505bd0 @ 0x00505BD0 — Model_SetSlotIndex(index): sets active model slot index.
void __cdecl FUN_00505bd0(int param_1) {
    DAT_083a4104 = 0;
    DAT_083a4108 = param_1;
}
// FUN_00505e90 @ 0x00505E90 — OpenModel(Type, Dir, ModelFileName, ...).
// Port FIEL del IDA (raw 0x505E90):
//   1. FileName = Dir + ModelFileName
//   2. Itera variadic args (extra animation .smd paths) hasta NULL o "end"
//   3. Si v11>0: OpenSMDModel(Type, FileName, v11, unk_83A4100) +
//      OpenSMDAnimation(Type, FileNameN, lockFlag) por cada N
//   4. Si v11==0: OpenSMDModel(Type, FileName, 1, unk_83A4100) +
//      OpenSMDAnimation(Type, FileName, 0)
//
// NOTA 2026-05-01: los archivos Data2/Item/<class>/<file>.smd NO existen en el
// filesystem distribuido (solo Data/Item/<file>.bmd está). Las llamadas a
// fopen dentro de FUN_0040b280/FUN_0040b310 retornarán NULL → early return →
// no-op silencioso. El path BMD (FUN_005060b0) cubre la carga real de items.
// (FUN_0040b280, FUN_0040b310 — declared via functions.h.
//  DAT_083a4100 — declared in globals.h.)

// Note: la signature original es variadic (`...` para extra anim paths) pero
// TODOS los call sites en nuestro source pasan exactamente 3 args (sin anims
// extras). Mantener 3 args para compat con functions.h.
void __cdecl FUN_00505e90(int Type, const char* Dir, const char* ModelFileName) {
    char FileName[200];
    // Build base FileName = Dir + ModelFileName
    crt_sprintf(FileName, "%s%s", Dir ? Dir : "", ModelFileName ? ModelFileName : "");
    // Sin variadic args, v11=0 → call OpenSMDModel + OpenSMDAnimation con
    // FileName base solamente.
    FUN_0040b280(Type, FileName, 1, (char)DAT_083a4100);
    FUN_0040b310(Type, FileName, 0);
    DAT_083a4100 = 0;

    // BUG-FIX 2026-05-04: el cliente 0.97k distribuido NO tiene Data2/Object*/
    // (solo Data/Object*/ con archivos .bmd). Las SMDs no cargan → Lorencia
    // queda sin casas/decoraciones porque su path de OpenWorldModels usa SMDs
    // exclusivamente. Como fallback, si después del SMD load el slot sigue
    // vacío (mesh count = 0), intentamos varias transformaciones del nombre
    // .smd → .bmd para encontrar el archivo real (case-insensitive en Win32).
    char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * Type);
    short meshCount = *(short*)(slot + 0x22);  // model[+0x22] = mesh count
    if (meshCount > 0) return;  // SMD load worked

    if (!Dir || !ModelFileName) return;
    // Strip "Data2/" prefix → "Data/" prefix for BMD fallback
    char bmdDir[128];
    if (strncmp(Dir, "Data2/", 6) == 0)
        crt_sprintf(bmdDir, "Data/%s", Dir + 6);
    else
        crt_sprintf(bmdDir, "%s", Dir);

    // Strip ".smd" extension from filename
    char baseName[128];
    crt_sprintf(baseName, "%s", ModelFileName);
    int blen = (int)strlen(baseName);
    if (blen > 4 && _stricmp(baseName + blen - 4, ".smd") == 0)
        baseName[blen - 4] = 0;

    // Hardcoded SMD→BMD name remaps for known mismatches in Lorencia
    // (smd uses snake_case named files; bmd uses PascalCase with different names)
    struct NameRemap { const char* smd; const char* bmd; };
    static const NameRemap remaps[] = {
        // Trees: 13 SMDs → 13 BMDs (Tree01..Tree13)
        {"treesmall",  "Tree01"}, {"treebig",    "Tree02"},
        {"treea_01",   "Tree03"}, {"treea_02",   "Tree04"},
        {"treea_03",   "Tree05"}, {"treea_04",   "Tree06"},
        {"treea_05",   "Tree07"}, {"treea_06",   "Tree08"},
        {"treea_07",   "Tree09"}, {"treea_08",   "Tree10"},
        {"treea_09",   "Tree11"}, {"treea_10",   "Tree12"},
        {"treea_11",   "Tree13"},
        // Stones (typo "Ston" → "Stone")
        {"Ston_01",    "Stone01"}, {"Ston_02", "Stone02"},
        {"Ston_03",    "Stone03"}, {"Ston_04", "Stone04"},
        {"Ston_05",    "Stone05"},
        // Mushrooms (drop underscore + numbering different)
        {"mushroom_01","Mushroom01"}, {"mushroom_02","Mushroom02"},
        // Walls
        {"wall_01",    "Wall01"}, {"wall_02", "Wall02"},
        {"wall_03",    "Wall03"}, {"wall_04", "Wall04"},
        {"wall_05",    "Wall05"}, {"wall_06", "Wall06"},
        {"c_wall01",   "CWall01"}, {"c_wall02","CWall02"},
        {"c_wall03",   "CWall03"}, {"c_wall04","CWall04"},
        {"c_wall05",   "CWall05"},
        // Tomb / TombStone
        {"Tomb_arc",   "Tomb01"}, {"Tomb_cross","Tomb02"},
        {"TombStone",  "TombStone01"},
        // Bridge
        {"bridge_01",      "Bridge01"},
        {"bridge_stone",   "BridgeStone01"},
        // Carriage / fence / etc. (already PascalCase variant works)
    };
    for (size_t i = 0; i < sizeof(remaps)/sizeof(remaps[0]); ++i) {
        if (_stricmp(baseName, remaps[i].smd) == 0) {
            FUN_005060b0(Type, bmdDir, remaps[i].bmd, -1);
            meshCount = *(short*)(slot + 0x22);
            if (meshCount > 0) return;
            break;
        }
    }

    // Build candidate variations to try in order:
    //   1. baseName as-is (e.g. "house_01" → "house_01.bmd")
    //   2. PascalCase no underscores: capitalize first letter of each segment
    //      between underscores, drop underscores. ("house_01" → "House01")
    //   3. Capitalized with underscores preserved. ("house_01" → "House_01")
    char cand[3][128];
    crt_sprintf(cand[0], "%s", baseName);
    // PascalCase no underscores
    {
        int j = 0;
        bool capitalize = true;
        for (int i = 0; baseName[i] && j < 126; ++i) {
            char c = baseName[i];
            if (c == '_') { capitalize = true; continue; }
            if (capitalize) {
                cand[1][j++] = (char)toupper((unsigned char)c);
                capitalize = false;
            } else {
                cand[1][j++] = c;
            }
        }
        cand[1][j] = 0;
    }
    // Capitalized with underscores
    {
        int j = 0;
        bool capitalize = true;
        for (int i = 0; baseName[i] && j < 126; ++i) {
            char c = baseName[i];
            if (c == '_') { cand[2][j++] = c; capitalize = true; continue; }
            if (capitalize) {
                cand[2][j++] = (char)toupper((unsigned char)c);
                capitalize = false;
            } else {
                cand[2][j++] = c;
            }
        }
        cand[2][j] = 0;
    }

    // Try each variant. FUN_005060b0 with idx=-1 appends ".bmd" → "<name>.bmd".
    // After each attempt check mesh count; bail when slot is populated.
    for (int v = 0; v < 3; ++v) {
        FUN_005060b0(Type, bmdDir, cand[v], -1);
        meshCount = *(short*)(slot + 0x22);
        if (meshCount > 0) return;
    }
}
// FUN_005098c0 — implemented in src/Monster/Monster_Data.cpp
// FUN_0047A1F0 @ 0x0047A1F0 — TextParser_GetToken: tokenizer for all game data files
// (Monster.txt / Item.txt / Skill.txt / NPC.txt / Gate.txt / Filter.txt).
//
// Ported from IDA Hex-Rays (610 bytes). Reads from `SMDFile` (DAT_0055c0a0),
// stores result in TokenString / TokenNumber / CurrentToken globals.
//
// Returns:
//   0   = identifier (alpha[alnum/./_]*) — stored in TokenString
//   1   = number (atof(buf)) — stored in TokenNumber (ParserTokenNumber)
//   2   = EOF (0xFF byte read)
//   '#' = literal hash
//   ',' = literal comma
//   ';' = literal semicolon
//   '{' / '}' = brace literals
//   '<' (60) = unrecognized character
//   string-literal "…" handled like identifier (sets CurrentToken=0, returns 0)
//
// Uses CRT helpers fgetc/getc/ungetc/isspace/isalpha/isalnum/isdigit/atof —
// all available in stdafx-included <cctype>/<cstdio>/<cstdlib>.
int __cdecl TextParser_GetToken(void)
{
    // 2026-08-22 FIX: escribia en ParserTokenString, que es el buffer del OTRO
    // tokenizer (Parse_NextToken / OpenWorldModels).  TokenString es 0x07CF1EF0.
    char* TokenStringBuf = (char*)&TextParserTokenString[0];
    int&   CurrentToken  = ParserCurrentToken;
    float& TokenNumber   = ParserTokenNumber;

    // CRITICAL 2026-05-03: data parsers (Item_Data, Monster_Data, Skill_Data,
    // Filter_Data, NPC_Data, Gate_Data) all open their file via DAT_07d7806c
    // (= IDA's SMDFile_0). The other "SMDFile" symbol at DAT_0055c0a0 is a
    // separate misnamed global from an early port pass — unused here.
    FILE* fp = DAT_07d7806c;

    char  TempString[100];
    int   cha;
    char* p;

    TokenStringBuf[0] = 0;
    if (!fp) return 2;

    cha = fgetc(fp);
    if ((char)cha == (char)0xFF || cha == EOF) return 2;

    // Skip comments (// … \n) and whitespace
    while (1) {
        if ((char)cha == '/') {
            cha = fgetc(fp);
            if ((char)cha == '/') {
                do { cha = fgetc(fp); }
                while ((char)cha != '\n' && cha != EOF);
            }
        }
        if (cha == EOF) return 2;
        if (!isspace((unsigned char)cha)) break;
        cha = fgetc(fp);
        if ((char)cha == (char)0xFF || cha == EOF) return 2;
    }

    int result;
    switch ((char)cha) {
    case '"':
        // Quoted string — read until closing quote, save into TokenString
        p = TokenStringBuf;
        for (cha = getc(fp); cha != EOF && (char)cha != (char)0xFF; cha = getc(fp)) {
            if ((char)cha == '"') break;
            *p++ = (char)cha;
        }
        if ((char)cha == (char)0xFF || cha == EOF) ungetc(cha, fp);
        *p = 0;
        CurrentToken = 0;
        return 0;

    case '#': result = 35;  CurrentToken = 35;  break;
    case ',': result = 44;  CurrentToken = 44;  break;
    case ';': result = 59;  CurrentToken = 59;  break;
    case '{': result = 123; CurrentToken = 123; break;
    case '}': result = 125; CurrentToken = 125; break;

    case '-': case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        ungetc(cha, fp);
        p = TempString;
        for (cha = getc(fp); cha != EOF && (char)cha != (char)0xFF; cha = getc(fp)) {
            if ((char)cha != '.' && !isdigit((unsigned char)cha) && (char)cha != '-') break;
            *p++ = (char)cha;
        }
        ungetc(cha, fp);
        *p = 0;
        TokenNumber = (float)atof(TempString);
        CurrentToken = 1;
        return 1;

    default:
        if (isalpha((unsigned char)cha)) {
            TokenStringBuf[0] = (char)cha;
            p = &TokenStringBuf[1];
            for (cha = getc(fp); cha != EOF && cha != -1; cha = getc(fp)) {
                char c2 = (char)cha;
                if (c2 != '.' && c2 != '_' && !isalnum((unsigned char)cha)) break;
                *p++ = c2;
            }
            ungetc(cha, fp);
            *p = 0;
            CurrentToken = 0;
            return 0;
        } else {
            CurrentToken = 60;
            return 60;
        }
    }
    return result;
}

// FUN_0047ea70 @ 0x0047EA70 — Skill_HashTable_SerializeEntry: encode 0x28-byte
// entry via rolling XOR/sub cipher and insert into hash table.
void __cdecl FUN_0047ea70(void *dst, void *src) { /* hash table serialize stub */ }
// FUN_0047eaf0 @ 0x0047EAF0 — Skill_HashTable_FreeEntry: decode entry and remove.
void __cdecl FUN_0047eaf0(void *entry, void *key) { /* hash table free stub */ }
// FUN_005430f0 @ 0x005430F0 — fwrite wrapper (with lock).
uint __cdecl FUN_005430f0(char *buf, uint size, uint count, int *fp) {
    return (uint)fwrite(buf, size, count, (FILE *)fp);
}
// FUN_005060b0 @ 0x005060B0 — Model_LoadBMD_ByIdx(slot, dir, basename, idx): loads BMD file at slot.
// Construye leafname "basename.bmd" (idx==-1), "basename0N.bmd" (idx<10) o
// "basenameNN.bmd" (idx>=10), usando param_3 (BASENAME) — NO param_2 (directorio).
// param_2 (directorio "Data/Logo/") se pasa aparte al loader BMD.
// BUG PREVIO: el sprintf pasaba param_2 en vez de param_3 → el filename quedaba
// "Data/Logo/01.bmd" en vez de "Logo01.bmd", los modelos de login/select nunca
// cargaban y el fondo 3D del server select quedaba vacío.
void __cdecl FUN_005060b0(int param_1, const char *param_2, const char *param_3, int param_4) {
    // BUG-FIX 2026-04-29: pump message queue cada N llamadas para evitar que
    // OpenWorld (que llama esta func ~hundreds de veces) bloquee el message
    // pump por 2+ segundos. El server MuEmu nos kickea por backpressure si
    // no consumimos los packets que envía después del JoinMapServer.
    {
        static int s_pumpCounter = 0;
        if ((s_pumpCounter++ & 0x07) == 0) {  // cada 8 BMDs
            MSG msg;
            for (int i = 0; i < 16 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); ++i) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    char local_40[64];
    if (param_4 == -1) {
        crt_sprintf(local_40, s__s_bmd_0055a7f8, param_3);
    } else {
        const char *pcVar1 = (param_4 < 10) ? s__s0_d_bmd_0055a7ec : s__s_d_bmd_0055a7e0;
        crt_sprintf(local_40, pcVar1, param_3, param_4);
    }
    int numBonesInSlot = *(short*)(DAT_05828d58 + 0x22 + param_1 * 0xbc);
    {
        char diag[200];
        _snprintf_s(diag, sizeof(diag), _TRUNCATE,
            "FUN_005060b0: slot=0x%x dir='%s' leaf='%s' idx=%d bones=%d hqMode=%d",
            param_1, param_2 ? param_2 : "(null)", local_40, param_4,
            numBonesInSlot, (int)(DAT_0055a7c4 == '\0'));
        DbgLogPublic(diag);
    }
    if (DAT_0055a7c4 == '\0') {
        // HQ path original: si el SMD ya cargó bones, FUN_00442a60 agrega la anim BMD.
        // PORT FALLBACK: como nuestro SMD loader (FUN_00505e90) es stub y nunca
        // popula bones, caemos al loader completo FUN_004423e0 para al menos traer
        // la geometría BMD y ver algo del background 3D.
        if (numBonesInSlot > 0)
            FUN_00442a60((int)(DAT_05828d58 + param_1 * 0xbc), (char*)param_2, local_40);
        else
            FUN_004423e0((int)(DAT_05828d58 + param_1 * 0xbc), (int)param_2, (int)local_40, 0);
    } else {
        FUN_004423e0((int)(DAT_05828d58 + param_1 * 0xbc), (int)param_2, (int)local_40, 0);
    }

    // Post-load defensive init: ensure bodyLight is (1,1,1) even if the BMD
    // load path bypassed BMD::Open (e.g. SMD-first HQ path). Without this the
    // login-scene slots retain the 0xF5F5F5F5 pool-fill garbage pattern at
    // +0x50, which manifests as white/yellow/corrupt triangles on Ship/Logos.
    {
        int slotBase = (int)(DAT_05828d58 + param_1 * 0xbc);
        *(float *)(slotBase + 0x48) = 1.0f;
        *(float *)(slotBase + 0x4c) = 1.0f;
        *(float *)(slotBase + 0x50) = 1.0f;
    }
}
// Forward-declare FindTextureByName (real implementation at ~line 12786 below).
int __cdecl FindTextureByName(char *Name, DWORD *dwTexture);

// ── FUN_00505c80 @ 0x00505C80 — OpenTexture (Model_LoadTextures) ────────────
// Para cada mesh del modelo en slot [Model]:
//   1) Lee el nombre de textura (32 bytes) desde pBMD->Data[+0x34] + i*0x20.
//   2) FindTextureByName — si ya está cargada, reusa e incrementa ref-count.
//   3) Si no: construye "SubFolder + name", llama OpenTGA (.t*) u OpenJPG (.*),
//      copia el nombre al slot de Bitmaps, e incrementa TextureCurrent.
//   4) Guarda el slot resuelto en pBMD->Data[+0x38] + i*2 (IndexTexture[i]).
//   5) Overrides por prefijo de filename: "ski*"/"level*" → 0x12d (BITMAP_SKIN),
//      "hid*" → 300 (BITMAP_HIDE).
//
// Sin esta función, IndexTexture queda sin inicializar (0xCDCD en MSVC Debug)
// y BMD_DrawMesh hace BindTexture con un id basura → naves/logos aparecen
// blancos o con textura corrupta. Verificado via debug.log (tex=0xffffcdcd).
//
// Globals:
//   Models            = DAT_05828d58 (BMD table, stride 0xBC; Data ptr at +0x00)
//   TextureBegin      = DAT_083a4104 (int, lower bound del scan)
//   TextureCurrent    = DAT_083a4108 (int, next-free slot)
//   Bitmaps[]         = g_BitmapsRaw (stride 0x38; filename en [+0x00..+0x1F])
//   DAT_0055a7a4      = base path "Data2\"   (Data2/pak mode)
//   DAT_0055a79c      = base path "Data\"    (Data mode)
void __cdecl FUN_00505c80(int Model, const char* SubFolder, int Type, char Check) {
    // ── BUG fix (crash 0xC0000005 @ 0x61746168 "ataH"): el Model slot ES la
    //    estructura BMD completa (stride 0xBC), NO un puntero a datos. Los
    //    primeros 32 bytes del slot son el Name (string), no un data ptr.
    //    Los contadores y tablas están inline:
    //       slot +0x24 short  numMeshes
    //       slot +0x34 char*  texNameTable (char[n][0x20])
    //       slot +0x38 short* indexTexture (short[n])
    //    Verificado en Ghidra FUN_004423e0 (BMD::Open): this[0x24]=numMeshes,
    //    this[0x34]=texName[] y this[0x38]=indexTex[] se asignan directamente.
    char* slot = (char*)(DAT_05828d58 + Model * 0xBC);
    short numMeshes = *(short*)(slot + 0x24);
    // DIAG-canary inconditional: confirmar que llegamos a la función para Ship/Logo
    {
        static int s_oc_any = 0;
        if (s_oc_any < 8) {
            char b[160];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "FUN_00505c80 CALL Model=0x%x sub='%s' nMesh=%d TextureCurrent=0x%x",
                Model, SubFolder ? SubFolder : "(null)", (int)numMeshes, (unsigned)DAT_083a4108);
            DbgLogPublic(b);
            s_oc_any++;
        }
    }
    // DIAG: log ENTRY for login-scene slots (Ship=0x3c, Logos=0xa0..0xa3).
    {
        static int s_oc = 0;
        bool isLoginSlot = (Model == 0x3c || (Model >= 0xa0 && Model <= 0xa3));
        if (isLoginSlot && s_oc < 20) {
            char* texNameTable0 = *(char**)(slot + 0x34);
            short* indexTexture0 = *(short**)(slot + 0x38);
            char firstName[33] = {0};
            if (texNameTable0 && numMeshes > 0) {
                memcpy(firstName, texNameTable0, 32);
                firstName[32] = 0;
            }
            char b[300];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "OpenTex ENTER Model=0x%x sub='%s' nMesh=%d texNames=%p idxTex=%p firstName='%s' TextureCurrent=0x%x",
                Model, SubFolder, (int)numMeshes, texNameTable0, indexTexture0,
                firstName, (unsigned)DAT_083a4108);
            DbgLogPublic(b);
            s_oc++;
        }
    }
    if (numMeshes <= 0) return;

    char* texNameTable  = *(char**)(slot + 0x34);
    short* indexTexture = *(short**)(slot + 0x38);
    if (!texNameTable || !indexTexture) return;

    for (int i = 0; i < numMeshes; i++) {
        char* Name = texNameTable + i * 0x20;
        DWORD pSlot = 0;
        int foundIdx = FindTextureByName(Name, &pSlot);
        int resolvedIdx;

        if (foundIdx < 0) {
            // Miss: load texture from disk into new slot
            // Locate '.' to detect extension
            int nameLen = (int)strnlen(Name, 32);
            int dotPos = 0;
            for (int k = 0; k < nameLen; k++) { if (Name[k] == '.') { dotPos = k; break; } }

            // Build full path: SubFolder + Name  (into local_40)
            // 2026-05-05: Si Name ya contiene un path (ej. "Data\Npc\foo.OZT"
            // como guardan algunos BMDs de NPC), NO concatenar SubFolder —
            // sino que sale "Data\Npc\Data\Npc\foo.OZT" → fopen FAIL.
            // Detectamos path absoluto: arranca con "Data\" o "Data/" o
            // contiene '\\' o '/' antes del primer '.'.
            char local_40[128];
            size_t nlen = strnlen(Name, 32);
            bool nameHasPath = false;
            for (size_t k = 0; k < nlen; ++k) {
                char ch = Name[k];
                if (ch == '\\' || ch == '/') { nameHasPath = true; break; }
                if (ch == '.') break;   // extension; stop scanning
            }
            if (nameHasPath) {
                if (nlen > sizeof(local_40) - 1) nlen = sizeof(local_40) - 1;
                memcpy(local_40, Name, nlen);
                local_40[nlen] = 0;
            } else {
                size_t slen = strlen(SubFolder);
                if (slen > 63) slen = 63;
                memcpy(local_40, SubFolder, slen);
                local_40[slen] = 0;
                if (slen + nlen > sizeof(local_40) - 1) nlen = sizeof(local_40) - 1 - slen;
                memcpy(local_40 + slen, Name, nlen);
                local_40[slen + nlen] = 0;
            }

            // Dispatch by extension character (tolower'd)
            int extChar = (dotPos + 1 < nameLen) ? tolower((unsigned char)Name[dotPos + 1]) : 'j';
            int slot = (int)DAT_083a4108;  // TextureCurrent
            if (extChar == 't')
                FUN_00529bd0(local_40, slot, 0x2600, 0x2901, 0, Check);   // OpenTGA
            else
                FUN_00529740(local_40, slot, Type,   0x2901, 0, Check);   // OpenJPG/OZJ

            // Store filename into the Bitmaps slot (first 32 bytes)
            char* slotBase = &g_BitmapsRaw[slot * 0x38];
            size_t fnLen = strnlen(Name, 32);
            memcpy(slotBase, Name, fnLen);
            if (fnLen < 32) memset(slotBase + fnLen, 0, 32 - fnLen);

            resolvedIdx = slot;
            DAT_083a4108 = slot + 1;   // TextureCurrent++
        } else {
            // Hit: reuse existing slot, bump ref count at +0x30
            if (pSlot != 0) {
                *(unsigned char*)(pSlot + 0x30) += 1;
            }
            resolvedIdx = foundIdx;
        }

        // Write resolved index into IndexTexture[i]
        indexTexture[i] = (short)resolvedIdx;

        // Special name overrides
        char c0 = Name[0], c1 = Name[1], c2 = Name[2];
        if (c0 == 's' && c1 == 'k' && c2 == 'i') {
            indexTexture[i] = 0x12d;   // BITMAP_SKIN
        } else if (c0 == 'l' && c1 == 'e' && c2 == 'v' && Name[3] == 'e' && Name[4] == 'l') {
            indexTexture[i] = 0x12d;   // BITMAP_SKIN (level variant)
        } else if (c0 == 'h' && c1 == 'i' && c2 == 'd') {
            indexTexture[i] = 300;     // BITMAP_HIDE
        }

        // DIAG: log Ship (0x3c) mesh→texture mapping so we can see
        // which mesh gets which texture slot and whether the BMD's texture
        // name matched an existing slot or triggered a disk load.
        if (Model == 0x3c) {
            char safe[33] = {0};
            memcpy(safe, Name, 32);
            safe[32] = 0;
            // sanitize non-printables
            for (int q = 0; q < 32; q++) {
                if (safe[q] && (safe[q] < 0x20 || safe[q] > 0x7e)) safe[q] = '?';
            }
            BITMAP_t* b = &Bitmaps[indexTexture[i]];
            char m[320];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "Ship mesh[%d] name='%s' foundIdx=%d resolvedIdx=0x%x idxTex=0x%x "
                "BMP: W=%.0f H=%.0f Comp=%d GL=%u Ref=%u Buf=%p",
                i, safe, foundIdx, (unsigned)resolvedIdx, indexTexture[i],
                (double)b->Width, (double)b->Height, (int)b->Components,
                (unsigned)b->TextureNumber, (unsigned)b->Ref, b->Buffer);
            DbgLogPublic(m);
        }
    }
}

// Forward decls for the SMD parsing chain (stubs below — files no existen
// en filesystem, retornan false; mantienen estructura del binario).
extern "C" bool __cdecl OpenSMDFile(const char* FileName, int Type, char Flip);
extern "C" void __cdecl FixupSMD(void);
extern "C" void __cdecl SMD2BMDModel(int ID, int Actions);
extern "C" void __cdecl SMD2BMDAnimation(int ID, char LockPosition);

// FUN_0040b280 @ 0x0040b280 — OpenSMDModel(ID, FileName, Actions, Flip)
// Port FIEL del IDA (raw 0x40B280):
//   if (Models[id].numMesh <= 0) {
//     if (OpenSMDFile(FileName, 0, Flip)) {
//       strcpy(Models[id], FileName);
//       Models[id].byte32 = 10;
//       FixupSMD();
//       SMD2BMDModel(ID, Actions);
//     }
//   }
void __cdecl FUN_0040b280(int ID, const char* FileName, int Actions, char Flip) {
    char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * ID);  // stride 188 = 0xbc
    if (*(short*)(slot + 36) > 0) return;  // already loaded
    if (!OpenSMDFile(FileName, 0, Flip)) return;
    strncpy(slot, FileName, 31);
    slot[31] = 0;
    *(unsigned char*)(slot + 32) = 10;
    FixupSMD();
    SMD2BMDModel(ID, Actions);
}

// FUN_0040b310 @ 0x0040b310 — OpenSMDAnimation(ID, FileName, LockPosition)
// Port FIEL del IDA (raw 0x40B310):
//   if (Models[id].numAnims > 0) {
//     OpenSMDFile(FileName, 1, 0);
//     SMD2BMDAnimation(ID, LockPosition);
//   }
void __cdecl FUN_0040b310(int ID, const char* FileName, char LockPosition) {
    char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * ID);
    if (*(short*)(slot + 34) <= 0) return;  // mesh slot not initialized
    OpenSMDFile(FileName, 1, 0);
    SMD2BMDAnimation(ID, LockPosition);
}
