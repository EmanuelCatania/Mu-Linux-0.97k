// Config_TokenParser.cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.

// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;
extern void __cdecl FUN_0054158c(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

// IDA Hex-Rays intrinsic shims.
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


// FUN_0050E2C0 @ 0x0050E2C0 — ParseNextToken
// Text/config file tokenizer. Reads from ParserFileHandle; output to ParserTokenString.
// Returns: 0=string/identifier, 1=numeric, 2=EOF, or single-char code for {,};#.
// Skips whitespace and '//' line comments. Stores token type in ParserCurrentToken,
// numeric value in ParserTokenNumber.
int __cdecl ParseNextToken(void) {
    ParserTokenString[0] = '\0';
    int c = fgetc(ParserFileHandle);
    // Skip whitespace and '//' line comments
    while (c != -1) {
        if (c == '/') {
            c = fgetc(ParserFileHandle);
            if (c == '/') {
                do { c = fgetc(ParserFileHandle); } while (c != '\n' && c != -1);
                if (c != -1) c = fgetc(ParserFileHandle);
                continue;
            }
            // single '/' — not a comment, handle below
            break;
        }
        if (!isspace(c)) break;
        c = fgetc(ParserFileHandle);
    }
    if (c == -1) return 2;

    switch (c) {
    case '"': {
        char *p = ParserTokenString;
        while ((c = fgetc(ParserFileHandle)) != -1 && c != '"')
            *p++ = (char)c;
        *p = '\0';
        ParserCurrentToken = 0;
        return 0;
    }
    case '#': ParserCurrentToken = '#'; return '#';
    case ',': ParserCurrentToken = ','; return ',';
    case ';': ParserCurrentToken = ';'; return ';';
    case '{': ParserCurrentToken = '{'; return '{';
    case '}': ParserCurrentToken = '}'; return '}';
    }
    // Numeric: digit, '-', or '.'
    if (isdigit(c) || c == '-' || c == '.') {
        char buf[100];
        char *p = buf;
        *p++ = (char)c;
        int nc;
        while ((nc = fgetc(ParserFileHandle)) != -1 && (isdigit(nc) || nc == '.' || nc == '-'))
            *p++ = (char)nc;
        *p = '\0';
        if (nc != -1) ungetc(nc, ParserFileHandle);
        ParserTokenNumber = (float)atof(buf);
        ParserCurrentToken = 1;
        return 1;
    }
    // Alpha or underscore → identifier
    if (!isalpha(c)) {
        ParserCurrentToken = '<';
        return '<';
    }
    char *p = ParserTokenString;
    *p++ = (char)c;
    int nc;
    while ((nc = fgetc(ParserFileHandle)) != -1 && (isalnum(nc) || nc == '.' || nc == '_'))
        *p++ = (char)nc;
    if (nc != -1) ungetc(nc, ParserFileHandle);
    *p = '\0';
    ParserCurrentToken = 0;
    return 0;
}
