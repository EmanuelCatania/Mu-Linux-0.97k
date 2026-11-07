// PacketFrame.cpp
// Ver PacketFrame.h. La tabla se genera desde la columna `Encrypt` de
// `MuServer/Data/Hack/HackPacketCheck.txt`; si ese archivo cambia del lado
// del server, hay que regenerarla.

#include "stdafx.h"
#include "Net/PacketFrame.h"

// Tabla generada desde MuServer/Data/Hack/HackPacketCheck.txt (columna Encrypt).
// -2 = el opcode no tiene fila generica: solo valen los subopcodes de abajo
//      (el server responde "Packet unknown error" y cierra la conexion).
// -1 = '*' en el archivo: el server acepta cualquier frame.
//  0 = el frame TIENE que ser C1/C2.   1 = tiene que ser C3/C4.
static const signed char s_FrameByOpcode[256] = {
     0,  0,  0,  1,  0,  0,  0,  0,   // 0x00 - 0x07
     0,  0,  0,  0,  0,  0,  1,  0,   // 0x08 - 0x0F
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x10 - 0x17
     0,  1,  0,  1,  1,  1,  1,  0,   // 0x18 - 0x1F
     0,  0,  1,  1,  1,  0,  1,  0,   // 0x20 - 0x27
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x28 - 0x2F
     1,  0,  1,  1,  1,  0,  1,  0,   // 0x30 - 0x37
     0,  0,  0,  0,  1,  1,  0,  1,   // 0x38 - 0x3F
     1,  1,  0,  0,  0,  0,  0,  0,   // 0x40 - 0x47
     0,  0,  1,  0,  0, -2, -2,  0,   // 0x48 - 0x4F
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x50 - 0x57
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x58 - 0x5F
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x60 - 0x67
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x68 - 0x6F
     0,  0,  1,  1,  0,  0,  0,  0,   // 0x70 - 0x77
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x78 - 0x7F
     0,  0,  0,  1,  0,  0,  0,  0,   // 0x80 - 0x87
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x88 - 0x8F
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x90 - 0x97
     0,  0,  0,  0,  0,  0,  0,  0,   // 0x98 - 0x9F
     1,  0,  1,  0,  0,  0,  0,  0,   // 0xA0 - 0xA7
     0,  0,  1,  1,  1,  0,  0,  0,   // 0xA8 - 0xAF
     1,  1,  0,  0,  0,  0,  0,  0,   // 0xB0 - 0xB7
     0,  0,  0,  0,  0,  0,  0,  0,   // 0xB8 - 0xBF
     0,  0,  0,  0,  0,  1,  0,  0,   // 0xC0 - 0xC7
     0,  0,  0,  0,  0,  0,  0,  0,   // 0xC8 - 0xCF
     0,  0,  1,  0,  0,  0,  0,  0,   // 0xD0 - 0xD7
     1,  0,  0,  0,  0,  0,  0,  0,   // 0xD8 - 0xDF
     0,  0,  0,  0,  0,  0,  0,  0,   // 0xE0 - 0xE7
     0,  0,  0,  0, -2,  0,  0,  0,   // 0xE8 - 0xEF
     0,  1,  0, -1,  0,  0,  0,  0,   // 0xF0 - 0xF7
     0,  0,  0,  0,  0,  0,  0,  0,   // 0xF8 - 0xFF
};

// Opcodes cuyo frame depende del subopcode (byte 3 del paquete).
struct PF_SubOverride { unsigned char op; unsigned char sub; signed char enc; };
static const PF_SubOverride s_SubOverrides[] = {
    { 0x4D,   0,  1 },
    { 0x4D,   1,  1 },
    { 0x4D,  15,  0 },
    { 0x4D,  16,  0 },
    { 0x4D,  17,  0 },
    { 0x4D,  18,  0 },
    { 0x4D,  19,  0 },
    { 0x4D,  20,  0 },
    { 0x4D,  21,  0 },
    { 0x4E,   0,  1 },
    { 0x4E,   1,  0 },
    { 0x4E,   8,  0 },
    { 0x4E,   9,  1 },
    { 0xEC,   0,  0 },
    { 0xEC,   1,  0 },
    { 0xEC,   2,  0 },
    { 0xEC,   3,  0 },
    { 0xEC,  49,  0 },
    { 0xEC,  51,  1 },
};

int PacketFrame_WantsEncrypt(BYTE opcode, BYTE subop)
{
    // Los overrides mandan sobre la fila generica: el server busca primero la
    // fila (index, value) exacta y recien despues la de value = '*'.
    for (int i = 0; i < (int)(sizeof(s_SubOverrides) / sizeof(s_SubOverrides[0])); ++i) {
        if (s_SubOverrides[i].op == opcode && s_SubOverrides[i].sub == subop)
            return s_SubOverrides[i].enc;
    }
    return s_FrameByOpcode[opcode];
}
