// Text_Data.cpp — localized string pool loader.
// Port of:
//   OpenTextData   @ 0x00510FE0
//   sub_479830     @ 0x00479830  (binary Text.bmd loader)
//
// The binary Text.bmd is 300000 bytes = 1000 entries × 300 bytes, XOR'd with
// the 3-byte key {0xFC, 0xCF, 0xAB} (BuxConvert_0 / FUN_00479910).
//
// Call tree:
//   WinMain → OpenTextData() → LoadTextData_Bin("Data\\Local\\Text.bmd")
//                           → fopen → fread(300000) → BuxConvert_0 → memcpy → fclose
//
// If the file is missing, original sends WM_QUIT.  Here we log + continue with
// an empty pool (UI strings just render blank rather than killing the client).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

#include <cstdio>
#include <cstring>

extern "C" { void DbgLogPublic(const char *msg); }

// ─────────────────────────────────────────────────────────────────────────────
// LoadTextData_Bin — port of sub_479830 @ 0x00479830
// ─────────────────────────────────────────────────────────────────────────────
int __cdecl LoadTextData_Bin(const char *FileName)
{
    FILE *fp = fopen(FileName, "rb");
    if (!fp) {
        char msg[256];
        wsprintfA(msg, "OpenTextData: %s file not found", FileName);
        DbgLogPublic(msg);
        return 0;
    }

    // Read the full 300000-byte blob into a scratch buffer, XOR-decrypt, then
    // copy into GlobalText[].  Mirrors the IDA layout: the .bmd is literally
    // the in-memory GlobalText array XOR'd with BuxConvert_0.
    // OJO: NO usar sizeof(GlobalText) — el array esta sobredimensionado a
    // GLOBALTEXT_ROWS para absorber indices heredados; el archivo solo trae
    // GLOBALTEXT_ROWS_FILE filas.
    const size_t poolBytes = (size_t)GLOBALTEXT_ROWS_FILE * 300;   // 300000
    BYTE *buf = (BYTE *)malloc(poolBytes);
    if (!buf) {
        DbgLogPublic("OpenTextData: malloc failed");
        fclose(fp);
        return 0;
    }

    size_t got = fread(buf, 1, poolBytes, fp);
    if (got != poolBytes) {
        char msg[256];
        wsprintfA(msg, "OpenTextData: short read (%u/%u)", (unsigned)got, (unsigned)poolBytes);
        DbgLogPublic(msg);
        free(buf);
        fclose(fp);
        return 0;
    }

    // XOR-decrypt in place with the 3-byte rolling key (BuxConvert_0).
    FUN_00479910((int)buf, (int)poolBytes);

    // Copy decrypted pool into GlobalText.  Each 300-byte row is already a
    // null-terminated string in the file; we don't truncate or reformat.
    memcpy(GlobalText, buf, poolBytes);

    free(buf);
    fclose(fp);

    char msg[128];
    wsprintfA(msg, "OpenTextData: loaded %s (1000 x 300)", FileName);
    DbgLogPublic(msg);
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// OpenTextData — port of 0x00510FE0
// ─────────────────────────────────────────────────────────────────────────────
// Original:
//   if (unk_55A7C4)            // == DAT_0055a7c4, set to 1 in our build
//       sub_479830("Data\\Local\\Text.bmd");
//       OpenMacro("Data\\Macro.txt");     // key-macros file
//   else
//       sub_4797B0("Data2\\Local\\Text(%s).txt", dword_55C9E50);   // text form
//       sub_479950(...);
//
// DAT_0055a7c4 == 1 in our build (see globals.cpp:1248), so we take the
// binary-file branch.  OpenMacro is not yet ported; skipping it leaves the
// user's custom key bindings empty (defaults still work).
void __cdecl OpenTextData(void)
{
    if (DAT_0055a7c4) {
        LoadTextData_Bin("Data\\Local\\Text.bmd");
        // TODO: OpenMacro("Data\\Macro.txt") — key-macro file loader.
    } else {
        // Plain-text branch — not taken in our build, but implement for parity.
        // sub_4797B0 tokenises "index value" pairs out of Text(xxx).txt.
        DbgLogPublic("OpenTextData: plain-text branch requested but not active");
    }

    // ── ESC-menu string buffer sync (IDA 0051AF50 L175-212 / L233-262) ───────
    // The original binary resolves the ESC-menu button labels inline from
    // GlobalText[381..388].  Our Ghidra port materialised them as six
    // distinct 300-byte buffers (lpString_07d45ba0..07d463d4) with Spanish
    // placeholders.  After loading Text.bmd we copy the real localized
    // strings into those buffers so the menu picks them up unchanged.
    if (GlobalText[381][0]) lstrcpynA(lpString_07d45ba0, GlobalText[381], 300);  // title ("Salir del juego")
    if (GlobalText[382][0]) lstrcpynA(lpString_07d45ccc, GlobalText[382], 300);  // "Elegir servidor"
    if (GlobalText[383][0]) lstrcpynA(lpString_07d45df8, GlobalText[383], 300);  // "Cambiar personaje"
    if (GlobalText[384][0]) lstrcpynA(lpString_07d45f24, GlobalText[384], 300);  // "Cancelar"
    if (GlobalText[385][0]) lstrcpynA(lpString_07d46050, GlobalText[385], 300);  // "Opciones"
    if (GlobalText[388][0]) lstrcpynA(lpString_07d463d4, GlobalText[388], 300);  // "Cerrar"

    // Options-panel On/Off labels — original formats them at render time via
    //   sprintf(buf, "%s  On"/"%s  Off", GlobalText[386/387]).
    // Our buffers s__s_On_0056184c / s__s_Off_00561854 etc are 32 bytes each,
    // so pre-compose once to keep the render code path unchanged.
    if (GlobalText[386][0]) {
        wsprintfA(s__s_On_0056184c,  "%s  On",  GlobalText[386]);  // Auto-attack
        wsprintfA(s__s_Off_00561854, "%s  Off", GlobalText[386]);
    }
    if (GlobalText[387][0]) {
        wsprintfA(s__s_On_0056185c,  "%s  On",  GlobalText[387]);  // Whisper sound
        wsprintfA(s__s_Off_00561864, "%s  Off", GlobalText[387]);
    }

    // ── Dialog/error message strings (IDA 0x07d4662c .. 0x07d68cf4) ──────────
    // En el binario original estas direcciones son slots de 300 bytes dentro
    // del pool GlobalText[] (cargado desde Text.bmd). Ghidra las partió en
    // `char X = 0;` independientes → cartel de error de login salía VACÍO
    // (UI_StatsPanel_Render lee `&lpString_07dXXXX` como LPCSTR y siempre
    // encontraba 0x00 → strlen=0 → no renderizaba texto).
    //
    // Base=0x07d45ba0 = GlobalText[381]; stride=300 (0x12c).
    // Índice GT = 381 + (addr - 0x07d45ba0) / 300.
    if (GlobalText[390][0]) lstrcpynA(lpString_07d4662c, GlobalText[390], 300);
    if (GlobalText[391][0]) lstrcpynA(lpString_07d46758, GlobalText[391], 300);
    if (GlobalText[392][0]) lstrcpynA(lpString_07d46884, GlobalText[392], 300);
    if (GlobalText[393][0]) lstrcpynA(lpString_07d469b0, GlobalText[393], 300);
    if (GlobalText[394][0]) lstrcpynA(lpString_07d46adc, GlobalText[394], 300);
    if (GlobalText[395][0]) lstrcpynA(lpString_07d46c08, GlobalText[395], 300);
    if (GlobalText[396][0]) lstrcpynA(lpString_07d46d34, GlobalText[396], 300);
    if (GlobalText[398][0]) lstrcpynA(lpString_07d46f8c, GlobalText[398], 300);
    if (GlobalText[399][0]) lstrcpynA(lpString_07d470b8, GlobalText[399], 300);
    if (GlobalText[400][0]) lstrcpynA(lpString_07d471e4, GlobalText[400], 300);
    if (GlobalText[401][0]) lstrcpynA(lpString_07d47310, GlobalText[401], 300);
    if (GlobalText[402][0]) lstrcpynA(lpString_07d4743c, GlobalText[402], 300);
    if (GlobalText[403][0]) lstrcpynA(lpString_07d47568, GlobalText[403], 300);
    if (GlobalText[404][0]) lstrcpynA(lpString_07d47694, GlobalText[404], 300);
    if (GlobalText[405][0]) lstrcpynA(lpString_07d477c0, GlobalText[405], 300);
    if (GlobalText[406][0]) lstrcpynA(lpString_07d478ec, GlobalText[406], 300);
    if (GlobalText[407][0]) lstrcpynA(lpString_07d47a18, GlobalText[407], 300);
    if (GlobalText[408][0]) lstrcpynA(lpString_07d47b44, GlobalText[408], 300);
    if (GlobalText[409][0]) lstrcpynA(lpString_07d47c70, GlobalText[409], 300);
    if (GlobalText[410][0]) lstrcpynA(lpString_07d47d9c, GlobalText[410], 300);
    if (GlobalText[411][0]) lstrcpynA(lpString_07d47ec8, GlobalText[411], 300);
    if (GlobalText[412][0]) lstrcpynA(lpString_07d47ff4, GlobalText[412], 300);
    if (GlobalText[413][0]) lstrcpynA(lpString_07d48120, GlobalText[413], 300);
    if (GlobalText[414][0]) lstrcpynA(lpString_07d4824c, GlobalText[414], 300);
    if (GlobalText[415][0]) lstrcpynA(lpString_07d48378, GlobalText[415], 300);
    if (GlobalText[416][0]) lstrcpynA(lpString_07d484a4, GlobalText[416], 300);
    if (GlobalText[417][0]) lstrcpynA(lpString_07d485d0, GlobalText[417], 300);
    if (GlobalText[418][0]) lstrcpynA(DAT_07d486fc, GlobalText[418], 300);
    if (GlobalText[420][0]) lstrcpynA(lpString_07d48954, GlobalText[420], 300);
    if (GlobalText[421][0]) lstrcpynA(lpString_07d48a80, GlobalText[421], 300);
    if (GlobalText[422][0]) lstrcpynA(lpString_07d48bac, GlobalText[422], 300);
    if (GlobalText[423][0]) lstrcpynA(lpString_07d48cd8, GlobalText[423], 300);
    if (GlobalText[424][0]) lstrcpynA(lpString_07d48e04, GlobalText[424], 300);
    if (GlobalText[425][0]) lstrcpynA(DAT_07d48f30, GlobalText[425], 300);
    if (GlobalText[426][0]) lstrcpynA(lpString_07d4905c, GlobalText[426], 300);
    if (GlobalText[427][0]) lstrcpynA(lpString_07d49188, GlobalText[427], 300);
    if (GlobalText[428][0]) lstrcpynA(lpString_07d492b4, GlobalText[428], 300);
    if (GlobalText[433][0]) lstrcpynA(lpString_07d49890, GlobalText[433], 300);
    if (GlobalText[434][0]) lstrcpynA(lpString_07d499bc, GlobalText[434], 300);
    if (GlobalText[435][0]) lstrcpynA(lpString_07d49ae8, GlobalText[435], 300);
    if (GlobalText[438][0]) lstrcpynA(lpString_07d49e6c, GlobalText[438], 300);
    if (GlobalText[439][0]) lstrcpynA(lpString_07d49f98, GlobalText[439], 300);
    if (GlobalText[440][0]) lstrcpynA(lpString_07d4a0c4, GlobalText[440], 300);
    if (GlobalText[441][0]) lstrcpynA(lpString_07d4a1f0, GlobalText[441], 300);
    if (GlobalText[442][0]) lstrcpynA(lpString_07d4a31c, GlobalText[442], 300);
    if (GlobalText[443][0]) lstrcpynA(lpString_07d4a448, GlobalText[443], 300);
    if (GlobalText[444][0]) lstrcpynA(lpString_07d4a574, GlobalText[444], 300);
    if (GlobalText[445][0]) lstrcpynA(lpString_07d4a6a0, GlobalText[445], 300);
    if (GlobalText[597][0]) lstrcpynA(lpString_07d558c0, GlobalText[597], 300);
    if (GlobalText[598][0]) lstrcpynA(lpString_07d559ec, GlobalText[598], 300);
    if (GlobalText[599][0]) lstrcpynA(lpString_07d55b18, GlobalText[599], 300);
    if (GlobalText[734][0]) lstrcpynA(lpString_07d5f94c, GlobalText[734], 300);
    if (GlobalText[857][0]) lstrcpynA(lpString_07d68970, GlobalText[857], 300);
    if (GlobalText[858][0]) lstrcpynA(lpString_07d68a9c, GlobalText[858], 300);
    if (GlobalText[859][0]) lstrcpynA(lpString_07d68bc8, GlobalText[859], 300);
    if (GlobalText[860][0]) lstrcpynA(lpString_07d68cf4, GlobalText[860], 300);
}
