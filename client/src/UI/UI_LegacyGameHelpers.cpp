// UI_LegacyGameHelpers.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

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


// UI/game helpers
// Item_ReturnPickedItem @ 0x004CD3B0 — UI_ItemGrid_Fill
// Fills 2D grid buffers with current item slot data (DAT_07e91350) for equipment display.
// Dispatches by DAT_07ea9800; each grid entry = 0x11 dwords, selection flag at offset 0x38.
//
// 2026-09-11: reescrita contra IDA.  La version anterior escribia en bases
// DAT_ corridas 0x38 que en este build son OTRA memoria (no los Offset*Items),
// borraba con memset un buffer de 64 celdas que el original no toca, tomaba la
// posicion de ItemPickedPos y no soltaba el item de la mano: el item "levantado
// y devuelto" nunca volvia a su celda.
//
// IDA sub_4CD3B0:
//   pos = Inventory[32].Type (= DAT_07ea5b18, slot de origen)
//   pool de origen (dword_7EA9800): trade / baul / mix -> celda = pos
//                                   inventario        -> celda = pos - 12
//                                   pos < 12          -> slot de equipo
//   cada celda del footprint = pPickedItem, Key = 1 solo en la primaria
//   (byte_7E9138E/F = columna/fila de la primaria)
//   al final: dword_7E91388 = 0; SetCharacterClass(Hero); PlayBuffer(29)
// IDA: sub_4CD3B0 (0x004CD3B0)
void __cdecl Item_ReturnPickedItem(void)
{
    if ((int)DAT_07e91388 < 1) return;

    const int type = (int)*(short*)DAT_07e91350;           // (__int16)pPickedItem
    const BYTE* attr = (const BYTE*)(uintptr_t)DAT_07d78068 + type * 0x40;
    const int w = attr[0x20];
    const int h = attr[0x21];
    const int selCol = (int)(BYTE)DAT_07e9138e;
    const int selRow = (int)(BYTE)DAT_07e9138f;
    const int pos    = (int)DAT_07ea5b18;
    BYTE* const src  = (BYTE*)(uintptr_t)DAT_07ea9800;

    auto fill = [&](BYTE* pool, int cell0) {
        const int col0 = cell0 % 8, row0 = cell0 / 8;
        for (int row = row0; row < row0 + h; ++row)
            for (int col = col0; col < col0 + w; ++col) {
                BYTE* cell = pool + (col + row * 8) * 0x44;
                memcpy(cell, DAT_07e91350, 0x44);
                *(int*)(cell + 0x38) = (row == selRow && col == selCol) ? 1 : 0;
            }
    };

    if (src == &OffsetTradeItems[0])          fill(OffsetTradeItems, pos);
    else if (src == &OffsetWarehouseItems[0]) fill(OffsetWarehouseItems, pos);
    else if (src == &OffsetMixItems[0])       fill(OffsetMixItems, pos);
    else if (pos >= 12)                       fill(OffsetInventoryItems, pos - 12);
    else if (pos >= 0 && CharacterMachine) {
        if (pos == 8 && DAT_07abf5d8) {
            int bug = (type == 416) ? 816 : (type == 418) ? 195 : (type == 419) ? 267 : 0;
            if (bug)
                CreateBug(bug, (void*)(DAT_07abf5d8 + 0x10), (void*)DAT_07abf5d8, 0);
        }
        memcpy((BYTE*)CharacterMachine + 68 * pos + 536, DAT_07e91350, 0x44);
    }

    DAT_07e91388 = 0;
    if (DAT_07abf5d8) SetCharacterClass((int)DAT_07abf5d8);      // SetCharacterClass(Hero)
    PlayBuffer(29, 0, 0);                                  // PlayBuffer(29)
    // (bloques de hash-table anti-tamper omitidos)
}

// SelectSkillByHotkey @ 0x004B0E80 — SelectSkillByHotkey(int number)
//
// Elige la skill activa a partir del numero de hotkey que el jugador acaba de
// apretar.  Recorre las 20 ranuras de skill y, para la que tenga asignado ese
// numero, escribe su indice en `Hero + 913` (la skill en uso).
//
//   for (i = 0; i < 20; i++) {
//       if (CharacterAttribute[i + 87] &&
//           CharacterAttribute[(SelectedHero << 6) + i + 215] == a1) {
//           Hero[913] = i;  found = 1;
//       }
//       if (m_bAutoAttack && World != 6) {
//           v9 = CharacterAttribute[Hero[913] + 87];
//           if (v9 == 6 || v9 == 15) { SelectedCharacter = -1; Attacking = -1; }
//       }
//   }
//
// 2026-09-04 -- BUG-FIX ("asigno el skill con Ctrl+N pero al apretar el numero
// no cambia").  El port tenia la firma `void SelectSkillByHotkey(void)`: Ghidra perdio
// el argumento (viaja en registro) y quien lo porteo comparo la tabla de
// asignaciones contra la CONSTANTE 1 en vez de contra el numero apretado.  O sea
// solo podia seleccionar la skill asignada al 1 -- y como los dos call sites
// llamaban sin argumento, cualquier tecla 0..9 hacia lo mismo.
//
// Los globals si estaban bien mapeados (verificado con ida_get_function):
// SelectedHero = 0x5616AC, m_bAutoAttack = 0x559C5C, Attacking = 0x559C58,
// CharacterAttribute = 0x7CF1FF4;  +87 = tipo de skill, +215 = numero de hotkey
// (la tabla es por personaje: SelectedHero << 6).
// IDA: sub_4B0E80 (0x004B0E80)
char __cdecl SelectSkillByHotkey(int a1)
{
    char found = 0;
    char* CA = (char*)CharacterAttribute;
    char* playerEnt = (char*)DAT_07abf5d8;
    if (!CA || !playerEnt) return 0;

    const int hero = (int)DAT_005616ac;   // SelectedHero
    for (int i = 0; i < 20; i++) {
        if (CA[i + 87] != 0 &&
            (unsigned char)CA[(hero << 6) + i + 215] == (unsigned char)a1) {
            playerEnt[913] = (char)i;
            found = 1;
        }
        if (DAT_00559c5c != 0 && World != 6) {
            const char skillType = CA[(unsigned char)playerEnt[913] + 87];
            if (skillType == 6 || skillType == 15) {
                SelectedCharacter = 0xffffffff;
                Attacking      = 0xffffffff;   // Attacking = -1
            }
        }
    }
    return found;
}

