#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_0047d120 @ 0x0047D120 — NPCName_Load(path) ───────────────────────────
// Reads a text-format NPC name data file.
// Parser uses FUN_0047a1f0 (type 0=section/END, 1=record, 2=EOF).
// For each non-section record:
//   - First field (from initial FUN_0047a1f0 before the loop): NPC type_id (float→int)
//   - Two unused fields skipped (FUN_0047a1f0 × 2)
//   - Name string from DAT_07cf1ef0 copied into NPC name table
//
// NPC name table layout:
//   Base: &DAT_07cf2000  (actually byte array; DAT_07d78078 = current count)
//   Each entry: stride 0x36 (54 bytes)
//     [0x00] = type_id (char from first float read before loop)
//     [0x01..] = name string (memcpy from DAT_07cf1ef0)
// Count (DAT_07d78078) incremented after each entry write.
//
// Sentinel: when FUN_0047a1f0 returns 0, compare DAT_07cf1ef0 with DAT_00559088
//   (the "END" marker); if equal, break inner loop and process next section.
// PORT FIEL de OpenMonsterScript (0x47D120) — 2026-07-24.
// Antes escribia un type_id placeholder 0 y leia el nombre de DAT_07cf1ef0
// (buffer equivocado) → tabla inutil.  Layout real por entrada (stride 0x36):
//   [0]     Type   = 1er token (columna 1 del archivo = entity type)
//   [1..32] Name   = 3er token (columna 3; el 2do se saltea)
// Formato NPCName.txt: "<Type> <idx> \"<Name>\"" por linea, hasta "end"/EOF.
// GetToken (FUN_0047a1f0) saltea el header "//..." y las comillas.
void __cdecl FUN_0047d120(const char *path)
{
    DAT_07d7806c = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    while (1) {
        int tok = FUN_0047a1f0();                    // GetToken → Type token
        if (tok == 2) break;                          // EOF
        if (tok == 0 && strcmp("end", (char *)&DAT_083a3ff4) == 0) break;  // sentinel
        if (DAT_07d78078 >= 512) break;               // tabla llena

        BYTE *m = &MonsterScript[DAT_07d78078 * 0x36];
        DAT_07d78078++;
        m[0] = (BYTE)(int)_DAT_083a40f8;              // Type = (int)TokenNumber

        FUN_0047a1f0();                               // skip columna 2 (idx)
        FUN_0047a1f0();                               // columna 3 = Name (TokenString)

        char *dst = (char *)(m + 1);                  // Name en [1]
        const char *src = (const char *)&DAT_083a3ff4; // TokenString
        int n = 0;
        while (src[n] != '\0' && n < 31) { dst[n] = src[n]; n++; }
        dst[n] = '\0';
    }
    FUN_0054150f(DAT_07d7806c);
}
