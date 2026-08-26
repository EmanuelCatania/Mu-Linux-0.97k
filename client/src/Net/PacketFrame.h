// PacketFrame.h
// Tabla de frames por opcode — replica lo que el GameServer valida en
// CHackPacketCheck::CheckPacketHack (columna `Encrypt` de
// `Data/Hack/HackPacketCheck.txt`).
//
// El problema que resuelve
// -----------------------
// El server exige por opcode un frame concreto:
//
//     Encrypt = 0  ->  el paquete DEBE venir como C1/C2 (plano + chain-XOR)
//     Encrypt = 1  ->  DEBE venir como C3/C4 (serial + chain-XOR + CSimpleModulus)
//     Encrypt = *  ->  cualquiera
//
// Si no coincide: "Packet encryption error" -> CloseClient, o sea el cliente
// se desconecta ~1 s despues de la accion, sin ningun mensaje. Y si el opcode
// no figura en el archivo: "Packet unknown error" -> CloseClient tambien.
//
// Hasta 2026-08-26 el cliente elegia el frame A MANO en cada call site (74
// repartidos entre 5 helpers de envio), asi que cada opcode nuevo era una
// chance de repetir el bug. Ya paso varias veces: reparar (0x34),
// trade-unconfirm (0x3C), guild (0x50/0x51/0x52/0x57) y los botones [+] de
// stats (F3/06) se mandaban con el frame equivocado y desconectaban.
//
// Ahora los helpers consultan esta tabla y corrigen el frame solos.
#pragma once
#include <windows.h>

// Valores devueltos por PacketFrame_WantsEncrypt.
#define PACKETFRAME_UNKNOWN  (-2)   // el opcode no existe para el server
#define PACKETFRAME_ANY      (-1)   // '*' — sirve cualquier frame
#define PACKETFRAME_PLAIN      0    // C1/C2
#define PACKETFRAME_ENCRYPTED  1    // C3/C4

// `opcode` es el byte 2 del paquete (head) y `subop` el byte 3 (value), que es
// el par con el que el server indexa la tabla. Para paquetes de 3 bytes (sin
// subopcode) pasar 0: ningun override de los que existen usa esa combinacion.
int PacketFrame_WantsEncrypt(BYTE opcode, BYTE subop);
