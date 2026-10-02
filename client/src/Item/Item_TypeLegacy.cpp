#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void __cdecl operator_delete(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
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

// ConvertItemType @ 0x0047B110 — ConvertItemType(BYTE* Item)  (21 bytes)
// Port FIEL desde IDA: returns Item[0] + 2*(Item[3] & 0x80). The high bit of
// Item[3] is the "Type Hi" flag used to distinguish item categories beyond
// 256 entries (e.g. shields >= 0x100).
extern "C" int __cdecl ConvertItemType(BYTE* Item)
{
    return (int)Item[0] + 2 * (int)(Item[3] & 0x80);
}
