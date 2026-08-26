#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_0047d120 @ 0x0047D120 — NPCName_Load(path) ───────────────────────────
// Reads a text-format NPC name data file.
// Parser uses TextParser_GetToken (type 0=section/END, 1=record, 2=EOF).
// For each non-section record:
//   - First field (from initial TextParser_GetToken before the loop): NPC type_id (float→int)
//   - Two unused fields skipped (TextParser_GetToken × 2)
//   - Name string from TextParserTokenString copied into NPC name table
//
// NPC name table layout:
//   Base: &DAT_07cf2000  (actually byte array; DAT_07d78078 = current count)
//   Each entry: stride 0x36 (54 bytes)
//     [0x00] = type_id (char from first float read before loop)
//     [0x01..] = name string (memcpy from TextParserTokenString)
// Count (DAT_07d78078) incremented after each entry write.
//
// Sentinel: when TextParser_GetToken returns 0, compare TextParserTokenString with DAT_00559088
//   (the "END" marker); if equal, break inner loop and process next section.
// PORT FIEL de OpenMonsterScript (0x47D120) — 2026-07-24.
// Antes escribia un type_id placeholder 0 y leia el nombre de TextParserTokenString
// (buffer equivocado) → tabla inutil.  Layout real por entrada (stride 0x36):
//   [0]     Type   = 1er token (columna 1 del archivo = entity type)
//   [1..32] Name   = 3er token (columna 3; el 2do se saltea)
// Formato NPCName.txt: "<Type> <idx> \"<Name>\"" por linea, hasta "end"/EOF.
// GetToken (TextParser_GetToken) saltea el header "//..." y las comillas.
void __cdecl FUN_0047d120(const char *path)
{
    DAT_07d7806c = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    while (1) {
        int tok = TextParser_GetToken();                    // GetToken → Type token
        if (tok == 2) break;                          // EOF
        if (tok == 0 && strcmp("end", TokenString) == 0) break;  // sentinel
        if (DAT_07d78078 >= 512) break;               // tabla llena

        BYTE *m = &MonsterScript[DAT_07d78078 * 0x36];
        DAT_07d78078++;
        m[0] = (BYTE)(int)ParserTokenNumber;              // Type = (int)TokenNumber

        TextParser_GetToken();                               // skip columna 2 (idx)
        TextParser_GetToken();                               // columna 3 = Name (TokenString)

        char *dst = (char *)(m + 1);                  // Name en [1]
        const char *src = TokenString;
        int n = 0;
        while (src[n] != '\0' && n < 31) { dst[n] = src[n]; n++; }
        dst[n] = '\0';
    }
    FUN_0054150f(DAT_07d7806c);
}
