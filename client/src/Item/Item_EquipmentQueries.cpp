// Item_EquipmentQueries.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_004824c0 @ 0x004824C0 (~266 lines) — get equipped weapon type (primary)
// Decrypts CharacterMachine, checks class!=elf, returns weapon type index or -1.
// IDA: FUN_004824c0
int __stdcall Item_FindElfWeaponInventorySlot(void) {
    // 0x004824C0 — Get equipped weapon type (primary hand)
    // anti-tamper hash table — skipped (encrypt/decrypt CharacterMachine)

    // IDA sub_4824C0 L41-88.  Los tres accesos estaban mal:
    //
    //   IDA                                        port anterior
    //   CharacterAttribute + 11                    ca + 0x00
    //   v6  = *(__int16 *)(CharacterMachine + 536) cm + 0x86*2 = +268
    //   v28 = *(__int16 *)(CharacterMachine + 604) cm + 0x97*2 = +302
    //
    // 268 y 302 son los INDICES de short (536/2 y 604/2) usados como offset de
    // BYTE: los dos accesos leian a la mitad de la direccion correcta, asi que
    // el tipo de arma salia basura, el scan no encontraba nada y la funcion
    // devolvia -1 siempre — por eso la municion no se auto-equipaba.
    //
    // Reparto de slots (confirmado por sub_4824C0 y por CreateArrow 0x474370):
    //   CharacterMachine + 536 (slot 0) -> BALLESTA (136-142, 144, 146)
    //   CharacterMachine + 604 (slot 1) -> ARCO     (128-134, 145)
    // y la municion va al slot que queda libre: Arrows (143) con arco, Bolts
    // (135) con ballesta.
    const char* const cmBytes = (const char*)(uintptr_t)DAT_07cf1ffc;
    const char* const caBytes = (const char*)(uintptr_t)DAT_07cf1ff4;
    if (!cmBytes || !caBytes)
        return -1;

    if ((caBytes[11] & 7) != 2)                       // IDA: CharacterAttribute + 11
        return -1;

    const short slot0 = *(const short*)(cmBytes + 536);   // IDA: v6
    const short slot1 = *(const short*)(cmBytes + 604);   // IDA: v28

    int weaponGroup;                                       // IDA: v12
    if ((slot1 >= 128 && slot1 < 135) || slot1 == 145)
        weaponGroup = 143;                                 // arco  -> Arrows
    else if ((slot0 < 136 || slot0 >= 143) && (slot0 < 144 || slot0 >= 160))
        weaponGroup = slot1;                               // ni arco ni ballesta
    else
        weaponGroup = 135;                                 // ballesta -> Bolts


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

// FUN_00482850 @ 0x00482850 - sub_482850: cuenta la municion del inventario
// IDA: FUN_00482850
// La copia VIVA de sub_482850 es `FUN_00482850_` (src/Render/HUD_Pass3.cpp),
// que es la que llama RenderNumArrow -- el unico caller que tiene la funcion en
// el binario (dos veces, 0x4BF77D y 0x4BF821).  Esta entrada existia como una
// segunda implementacion del mismo simbolo y estaba mal en tres accesos
// (`ca + 0` por `ca + 11`, y los dos slots de mano leidos como indices de short
// en vez de offsets de byte), ademas de cruzar las dos manos en el arbol de
// decision y devolver -1 donde el binario devuelve 0.  Como no tenia callers,
// nadie lo notaba: era exactamente el patron de simbolo duplicado que ya mordio
// con OpenSMDFile, RenderText y SetPlayerStop.  Ahora delega, asi las dos no
// pueden volver a divergir.
extern "C" int __cdecl FUN_00482850_(void);
int __stdcall Item_CountElfWeaponInventorySlots(void) {
    return FUN_00482850_();
}

// FUN_00482e40 @ 0x00482E40 (~109 lines) — count equipped items of weapon group
// param_1: 0=right, 1=left, 2=crossbow. Reads DAT_00559c60/c64/c68.
// Scans 8x4 inventory grid counting matching weapon types.
// IDA: FUN_00482e40
int __cdecl Item_CountWeaponGroupItems(int a1)
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
