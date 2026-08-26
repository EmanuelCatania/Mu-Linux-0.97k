// GameGuard_Packet.cpp
// Extracted from stubs_bulk_misc.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// CSimpleModulus key-load wrappers, extracted with the GameGuard helpers.
BOOL __cdecl CSimpleModulus_LoadEncryptionKey(DWORD *self, const char *fn) {
    return FUN_0053d1c0(self, fn, 0x1112, 1, 1, 0, 1);
}

BOOL __cdecl CSimpleModulus_LoadDecryptionKey(DWORD *self, const char *fn) {
    return FUN_0053d1c0(self, fn, 0x1112, 1, 0, 1, 1);
}

// FUN_004070d0 @ 0x004070D0 (~379 lines) — GameGuard packet handler (anti-tamper dispatch)
// Handles GameGuard message IDs 0x3E9..0x3F8. Key behaviors:
//   0x3E9/0x3EA: format param_2 into error string, show MessageBox
//   0x3F3: show error MessageBox, call CloseHack
//   0x3F4/0x3F5: format error with param_1, MessageBox, CloseHack
//   0x3F6: format error with param_2, MessageBox, CloseHack
//   0x3F8: XOR-encrypt packet with 32-byte hardcoded key {0xe7,0x6d,0x3a,...},
//           build C1 packet, lookup serial in MAIN_HASH_CLASS, send via socket
//   default: return 1
// Contains anti-tamper patterns: repeated key init forward+reverse, dead code.
int __stdcall FUN_004070d0(int param_1, int param_2) {
    (void)param_1; (void)param_2;
    // switch(param_1):
    //   case 0x3E9/0x3EA: wsprintfA(buf, param_2_template, param_2); MessageBoxA; CloseHack
    //   case 0x3F3: MessageBoxA(error); CloseHack(g_hWnd, true)
    //   case 0x3F4: wsprintfA(buf, template, param_1); MessageBoxA; CloseHack
    //   case 0x3F5: wsprintfA(buf, template, param_2)
    //   case 0x3F6: wsprintfA(buf, param_2_template, param_2); MessageBoxA; CloseHack
    //   case 0x3F8: build XOR-encrypted C1 packet:
    //     32-byte key = {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,...}
    //     XOR payload with key[i%32] ^ prev_byte
    //     Lookup serial in MAIN_HASH_CLASS, send packet via socket
    //   default: return 1
    return 1;
}
