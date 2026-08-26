// Item_EquipmentQueries.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_004824c0 @ 0x004824C0 (~266 lines) — get equipped weapon type (primary)
// Decrypts CharacterMachine, checks class!=elf, returns weapon type index or -1.
int __stdcall FUN_004824c0_stub(void) {
    // 0x004824C0 — Get equipped weapon type (primary hand)
    // anti-tamper hash table — skipped (encrypt/decrypt CharacterMachine)

    DWORD* cm = (DWORD*)DAT_07cf1ffc;  // CharacterMachine
    DWORD* ca = (DWORD*)DAT_07cf1ff4;  // CharacterAttribute

    // If class != elf (2), return -1
    if ((*(BYTE*)((DWORD)ca + 0x00) & 7) != 2) {
        // anti-tamper hash table — skipped
        return -1;
    }

    // Elf path: read equipped item type from CharacterMachine
    int itemType = (int)*(short*)((DWORD)cm + 0x97 * 2);   // slot 0x97 (weapon type short)
    int weaponClass = (int)*(short*)((DWORD)cm + 0x86 * 2); // slot 0x86 (weapon class short)

    // anti-tamper hash table — skipped

    // Classify weapon type into weapon group
    int weaponGroup;
    if ((weaponClass >= 0x80 && weaponClass <= 0x86) || weaponClass == 0x91) {
        weaponGroup = 0x8f;
    } else if ((weaponClass > 0x87 && weaponClass < 0x8f) || (weaponClass > 0x8f && weaponClass < 0xa0)) {
        weaponGroup = 0x87;
    } else {
        weaponGroup = weaponClass;
    }

    // Scan equipment table from DAT_07ea9504 downward (stride 0x11 dwords = 0x44 bytes per slot)
    // 8 rows x 8 columns, looking for first slot matching weaponGroup with durability > 0
    //
    // BUG-FIX 2026-05-03: el original usa `if ((int)piRow < 0x7ea9328) return -1;`
    // — una direccion absoluta del binario fuente, que en nuestro build no
    // significa nada.  Reemplazado por un contador explicito.
    // 2026-08-22: ese contador era de 7 y son 8 columnas
    // ((0x7EA9504 - 0x7EA9328) / 68 + 1 = 8).  Se nota tambien en `col`, que
    // arranca en 7 y tiene que llegar hasta 0.
    int* piRow = &DAT_07ea9504;
    int col = 7;
    for (int outer = 0; outer < 8; ++outer) {
        int slotIdx = col + 0x38;
        int* piSlot = piRow;
        for (int row = 7; row >= 0; --row) {
            if ((short)*(piSlot - 0x0E) == (short)weaponGroup && *piSlot > 0) {
                return slotIdx;
            }
            piSlot -= 0x88;
            slotIdx -= 8;
        }
        piRow -= 0x11;
        col--;
    }
    return -1;
}

// FUN_00482850 @ 0x00482850 (~260 lines) — get equipped weapon type (secondary/shield)
int __stdcall FUN_00482850_stub(void) {
    // 0x00482850 — Get equipped weapon type (secondary/shield)
    // anti-tamper hash table — skipped (encrypt/decrypt CharacterMachine)

    DWORD* cm = (DWORD*)DAT_07cf1ffc;  // CharacterMachine
    DWORD* ca = (DWORD*)DAT_07cf1ff4;  // CharacterAttribute

    // If class != elf (2), count matching items and return count
    if ((*(BYTE*)((DWORD)ca + 0x00) & 7) != 2) {
        // anti-tamper hash table — skipped
        // Ghidra returns unaff_EBP here (uninitialized/phantom) — return -1 for non-elf
        return -1;
    }

    // Elf path: read equipped item type from CharacterMachine
    int itemType = (int)*(short*)((DWORD)cm + 0x97 * 2);   // slot 0x97
    int weaponClass = (int)*(short*)((DWORD)cm + 0x86 * 2); // slot 0x86

    // anti-tamper hash table — skipped

    // Classify weapon type into weapon group
    int weaponGroup;
    if ((weaponClass >= 0x80 && weaponClass <= 0x86) || weaponClass == 0x91) {
        weaponGroup = 0x8f;
    } else if ((weaponClass > 0x87 && weaponClass < 0x8f) || (weaponClass > 0x8f && weaponClass < 0xa0)) {
        weaponGroup = 0x87;
    } else {
        weaponGroup = weaponClass;
    }

    // Recorre la grilla desde DAT_07ea9504 hacia atras contando coincidencias.
    // 2026-08-22: el bound seguia siendo el literal del binario fuente
    // (`while ((int)piRow > 0x7ea9327)`).  En nuestro build piRow es una
    // direccion de BSS mucho mas baja que 0x7EA9327, asi que la condicion daba
    // falsa en la primera vuelta y el do-while contaba UNA sola columna de las
    // 8.  Contador explicito, igual que los otros dos walkers de esta grilla.
    int count = 0;
    int* piRow = &DAT_07ea9504;
    for (int outer = 0; outer < 8; ++outer) {
        int* piSlot = piRow;
        for (int row = 8; row != 0; --row) {
            if ((short)*(piSlot - 0x0E) == (short)weaponGroup && *piSlot > 0) {
                count++;
            }
            piSlot -= 0x88;
        }
        piRow -= 0x11;
    }
    return count;
}

// FUN_00482e40 @ 0x00482E40 (~109 lines) — count equipped items of weapon group
// param_1: 0=right, 1=left, 2=crossbow. Reads DAT_00559c60/c64/c68.
// Scans 8x4 inventory grid counting matching weapon types.
// FUN_00482e40 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00482e40(int a1)
{
  char v1; // bl
  int v2; // ecx
  int v3; // ebp
  int v4; // edi
  int *v5; // esi
  int *v6; // ecx
  int v7; // edx
  int v9; // [esp+10h] [ebp+4h]

  v1 = 0;
  if ( a1 )
  {
    if ( a1 == 1 )
    {
      v2 = DAT_00559c64;
      switch ( DAT_00559c64 )
      {
        case 456:
          goto LABEL_17;
        case 457:
        case 468:
          goto LABEL_16;
        default:
          if ( DAT_00559c64 >= 448 && DAT_00559c64 <= 451 )
          {
            goto LABEL_20;
          }
          goto LABEL_13;
      }
    }
    if ( a1 == 2 )
    {
      v2 = DAT_00559c68;
      if ( DAT_00559c68 == 457 || DAT_00559c68 == 468 )
      {
        v3 = DAT_00559c68;
        v1 = 1;
      }
      else if ( DAT_00559c68 < 448 || DAT_00559c68 > 451 )
      {
        if ( DAT_00559c68 < 452 || DAT_00559c68 > 454 )
        {
          v2 = 456;
          v3 = 456;
        }
        else
        {
LABEL_13:
          v2 = 454;
          v3 = 452;
        }
      }
      else
      {
        v2 = 451;
        v3 = 448;
      }
    }
    else
    {
      v2 = a1;
      v3 = a1;
    }
  }
  else
  {
    v2 = DAT_00559c60;
    switch ( DAT_00559c60 )
    {
      case 0x1C8:
        goto LABEL_17;
      case 0x1C9:
      case 0x1D4:
LABEL_16:
        v1 = 1;
LABEL_17:
        v3 = v2;
        break;
      default:
        if ( DAT_00559c60 < 452 || DAT_00559c60 > 454 )
        {
LABEL_20:
          v2 = 451;
          v3 = 448;
        }
        else
        {
          v2 = 454;
          v3 = 452;
        }
        break;
    }
  }
  v4 = 0;
  v9 = v2;
  if ( v2 < v3 )
  {
    return 0;
  }
  do
  {
    v5 = (int *)&DAT_07ea9504;
    do
    {
      v6 = v5;
      v7 = 8;
      do
      {
        if ( *((short *)v6 - 28) == v9 && *v6 > 0 )
        {
          if ( v1 )
          {
            ++v4;
          }
          else
          {
            v4 += *((unsigned char *)v6 - 30);
          }
        }
        v6 -= 136;
        --v7;
      }
      while ( v7 );
      v5 -= 17;
    }
    while ( (int)v5 >= (int)&DAT_07ea9328 );
    --v9;
  }
  while ( v9 >= v3 );
  return v4;
}
