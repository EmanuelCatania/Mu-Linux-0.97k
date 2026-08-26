// Net_Bux.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_004f6eb0 @ 0x004F6EB0 — BuxConvert(BYTE* Buffer, int Size)
// XOR-obfuscates Buffer in place using a 3-byte rotating key at DAT_0055a770.
// Used for terrain block (.bux) encryption/decryption.
void __cdecl FUN_004f6eb0(int data, int size)
{
    BYTE* Buffer = (BYTE*)data;
    BYTE* key = &DAT_0055a770;   // 3 consecutive bytes at 0x0055a770..2
    for (int i = 0; i < size; i++)
        Buffer[i] ^= key[i % 3];
}

// MapFileDecrypt — decrypt EncTerrain%d.{map,obj} with MU map-file algorithm.
// Reference: público en source leaks de Mu Online Season clients (válido para 0.97.x).
// Algorithm: each byte is XOR'd with a 16-byte rolling key, minus a per-byte
// running counter (wKey) that is updated based on the original ciphertext byte.
//   plain[i] = (cipher[i] ^ MapFileKey[i & 0x0F]) - wKey
//   wKey = cipher[i] + 0x3D    (note: cipher byte, not plain)
// Initial wKey = 0x5E.
//
// BUG-FIX 2026-05-01: BuxConvert (3-byte XOR) NO sirve para .map/.obj — esos
// archivos usan un algorithm distinto (de ahí el prefix "Enc" mientras que .att
// usa BuxConvert simple).
void MapFileDecrypt(BYTE* buf, int size)
{
    static const BYTE MapFileKey[16] = {
        0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
        0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2
    };
    BYTE wKey = 0x5E;
    for (int i = 0; i < size; i++) {
        BYTE enc = buf[i];
        buf[i]   = (BYTE)((enc ^ MapFileKey[i & 0x0F]) - wKey);
        wKey     = (BYTE)(enc + 0x3D);
    }
}

