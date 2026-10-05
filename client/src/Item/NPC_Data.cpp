#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// IDA: FUN_0047D120 — OpenMonsterScript (0x47D120), port fiel.
// Lee NPCName.txt: "<Type> <idx> \"<Name>\"" por linea, hasta "end"/EOF.
// GetToken (TextParser_GetToken) saltea el header "//..." y las comillas.
//
// Tabla en &DAT_07cf2000 (EditMonsterNumber = cantidad), stride 0x36 por entrada:
//   [0]     Type   = 1er token (columna 1 del archivo = entity type)
//   [1..32] Name   = 3er token (columna 3; el 2do se saltea)
void __cdecl NPCName_LoadTextData(const char *path)
{
    DAT_07d7806c = (FILE *)crt_fopen(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    while (1) {
        int tok = TextParser_GetToken();                    // GetToken → Type token
        if (tok == 2) break;                          // EOF
        if (tok == 0 && strcmp("end", TokenString) == 0) break;  // sentinel
        if (EditMonsterNumber >= 512) break;               // tabla llena

        BYTE *m = &MonsterScript[EditMonsterNumber * 0x36];
        EditMonsterNumber++;
        m[0] = (BYTE)(int)ParserTokenNumber;              // Type = (int)TokenNumber

        TextParser_GetToken();                               // skip columna 2 (idx)
        TextParser_GetToken();                               // columna 3 = Name (TokenString)

        char *dst = (char *)(m + 1);                  // Name en [1]
        const char *src = TokenString;
        int n = 0;
        while (src[n] != '\0' && n < 31) { dst[n] = src[n]; n++; }
        dst[n] = '\0';
    }
    crt_fclose(DAT_07d7806c);
}
