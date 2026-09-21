// Net_PacketSession.cpp
// Packet session management: per-frame packet receive/dispatch and slot cleanup.
//
// IDA: ClearInventory (0x004CBDF0)
// IDA: UpdateWindowsMouse (0x004ECB00)

#include "stdafx.h"


// IDA: ClearInventory (0x004CBDF0).  El nombre viejo del port
// ("Net_ProcessReceiveQueue") era una misidentificacion, y el cuerpo estaba
// anulado con un `return` ("AUTO-SKIP"), asi que al entrar al mundo no se
// limpiaba ningun pool.  Lo que queda fuera es el cifrado anti-tamper de
// CharacterMachine.
extern "C" BYTE Inventory[];
extern "C" BYTE OffsetInventoryItems[];
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];

static void ClearItemPool97k(BYTE* pool, int count)
{
    for (int i = 0; i < count; ++i) {
        *(WORD*)(pool + i * 68) = 0xFFFF;         // Type
        *(DWORD*)(pool + i * 68 + 56) = 0;        // Key
    }
}

// IDA: ClearInventory (0x004CBDF0)
void ClearInventory(void)
{
    // Los 12 slots de equipo de CharacterMachine (+536, stride 68).
    if (CharacterMachine)
        ClearItemPool97k((BYTE*)CharacterMachine + 536, 12);

    ClearItemPool97k(OffsetInventoryItems, 64);    // 0x7EA8410 .. 0x7EA9510
    ClearItemPool97k(OffsetWarehouseItems, 120);   // 0x7EA5B30 .. TextBold[14]
    ClearItemPool97k(OffsetMixItems, 32);          // 0x7EA9848 .. m_nYourTradeWait
    *(BYTE*)&DAT_07eaa0e8 = 0;                     // byte_7EAA0E8

    // for (i = 0; i < 2176; i += 68): Inventory, OffsetTradeItems y el
    // historial de trade (word_7E11F78), 32 entradas cada uno.
    ClearItemPool97k(Inventory, 32);
    ClearItemPool97k(OffsetTradeItems, 32);
    ClearItemPool97k(DAT_07e11f78, 32);
}


// IDA: UpdateWindowsMouse (0x004ECB00)
// Processes queued incoming network packets for the current scene.
// Guards against map 0x6e. Calls sub-scene packet handlers in sequence:
//   FUN_004e4760, FUN_004e5500, FUN_004e5de0 (always)
//   FUN_004e6550..FUN_004ec330 (if DAT_07eaa117 != 0 — extended scene)
//   Inventory_DropDispatch (if DAT_07eaa164 == 0 — not in special mode)
//   CheckGoldenArcherWindow, FUN_004e8b70 (always)
void UpdateWindowsMouse(void)
{
  undefined4 uVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  uint extraout_EDX;
  uint extraout_EDX_00;
  uint uVar2;

  if (DAT_083a7c24 == 0x6e) {
    return;
  }
  uVar1 = SecondPassword_Handler();
  if (((((char)uVar1 != '\0') || (DAT_083a7c24 != 0)) || (DAT_055c9b7c != 0)) || (DAT_055c9b80 != 0)
     ) {
    if (DAT_055c9b7c != *(int *)(DAT_055c9ff4 + 0x1c)) {
      return;
    }
    DAT_055c9b7c = 0;
  }
  FUN_004e4760();
  FUN_004e5500();
  FUN_004e5de0();
  uVar1 = extraout_ECX;
  uVar2 = extraout_EDX;
  if (DAT_07eaa117 != '\0') {
    FUN_004e6550();
    FUN_004e6c40();
    FUN_004e9050();
    FUN_004eb5d0();
    FUN_004eb7f0();
    FUN_004ec330();
    uVar1 = extraout_ECX_00;
    uVar2 = extraout_EDX_00;
  }
  if (DAT_07eaa164 == '\0') {
    Inventory_DropDispatch(uVar1,uVar2);
  }
  CheckGoldenArcherWindow();
  FUN_004e8b70();
  return;
}
