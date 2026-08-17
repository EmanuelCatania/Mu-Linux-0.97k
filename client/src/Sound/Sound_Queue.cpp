// Sound_Queue.cpp
// FUN_00479730 @ 0x00479730
//
// Sound_UpdateQueue — processes the pending sound-play queue each frame.
//
// The queue is a flat array starting at DAT_07c85894 with stride 0x6f (int-words),
// so each entry is 0x1bc = 444 bytes. The array ends at 0x7CF1EF4.
// Each entry layout (relative to piVar2, which points at field +4):
//   piVar2[-1]  (byte)   — active flag (non-zero = pending)
//   piVar2[0]   (int)    — sound type:
//                            0 = FUN_00511710 (play once)
//                            1 = FUN_00511790 (play looped / 3D)
//                            2 = FUN_00511680(1) (play with param)
//   piVar2[...]          — remaining sound params (passed to FUN_00479670)
//
// After dispatching the play call, FUN_00479670 is invoked on the entry
// (likely to advance or clear the queue slot).
//
// Sub-functions:
//   FUN_00511710 — Sound_Play (type 0: one-shot)
//   FUN_00511790 — Sound_PlayLoop (type 1: looped/3D)
//   FUN_00511680 — Sound_PlayParam (type 2: param variant)
//   FUN_00479670 — Sound_Queue_Advance / clear slot

#include "stdafx.h"

// FUN_00479730 = RenderSprites (verificado vía Ghidra). Recorre el effect pool
// y por cada slot activo:
//   - dispatch GL state según blend mode en +4 (FUN_00511710/90/80)
//   - llama FUN_00479670 (RenderSprite) para dibujar el quad
//   - clear active flag
// Llamada desde Scene_CharSelect.cpp:325 (mal-comentada como "Portal_Render"),
// Scene_Login.cpp:100. Es la función que dibuja TODOS los sprites/glows/sparkles
// del pool (glow +9 set, wing FX, weapon FX, particles, etc.)
// Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODO el render — ahora itera
// por índice acotado a 1002 slots.
extern "C" void DbgLogPublic(const char*);
void __cdecl FUN_00479730(void)
{
    char *pcVar2 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar2 += 0x1bc) {
        if (*pcVar2 != '\0') {
            int blend = *(int*)(pcVar2 + 4);
            if      (blend == 0) FUN_00511710();
            else if (blend == 1) FUN_00511790();
            else if (blend == 2) FUN_00511680('\x01');
            FUN_00479670((int)pcVar2);
            *pcVar2 = 0;
        }
    }
}


// FUN_00479670 @ 0x00479670
//
// Sound_Queue_Advance — per-frame update for a sound queue slot.
//
// Fades the slot's volume field (+0x108) up or down based on the
// direction flag at +0x160:
//   0 — fade out: subtract _DAT_005524f4 per frame; clamp to 0.2 (0x3e4ccccd)
//   1 — fade in:  add    _DAT_005524f4 per frame; clamp to 1.0 (0x3f800000)
//
// After adjusting volume, calls FUN_00511d00 to submit the sound update:
//   channel  = *(short*)(param_1 + 2)
//   position = (float*)(param_1 + 0x10)
//   volume   = slot_volume * channel_volume_R * channel_volume_G (from DAT_083a7cc0/DAT_083a7cc4)
//   roll-off = *(float*)(param_1 + 0xe8)
//   distance = *(float*)(param_1 + 0x24)
//
// Globals:
//   _DAT_005524f4  — fade step delta
//   _DAT_005526e4  — minimum fade-out floor (0.2)
//   _DAT_0055256c  — float 1.0
//   DAT_083a7cc0   — per-channel volume table (R component, stride 0x38)
//   DAT_083a7cc4   — per-channel volume table (G component, stride 0x38)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_00479670(int param_1)

{
  float fVar1;
  int iVar2;

  // BUG-FIX 2026-04-27: esta función NO es Sound_Queue_Advance — es **RenderSprite**
  // (verificado vía Ghidra, addr 0x00479670 → "RenderSprite"). El comentario y el
  // archivo Sound_Queue.cpp tenían el nombre wrong. Saca cada slot del effect pool
  // y lo dibuja como billboard cuadrado vía FUN_00511d00 (Sprite_DrawTexturedQuad).
  // El return; previo bloqueaba TODO el render de sprites del juego (glow +9, wing
  // FX, weapon sparkles, lightning, particles) — combinado con el AUTO-SKIP del
  // pool en FUN_004795c0, NADA spawneaba ni se dibujaba.
  // DAT_083a7cc0/cc4 = Bitmaps[type] tabla de texturas (stride 0x38), fields +0/+4
  // = width/height usados para scale del quad.

  if (*(char *)(param_1 + 0x160) == '\0') {
    fVar1 = *(float *)(param_1 + 0x108) - _DAT_005524f4;
    *(float *)(param_1 + 0x108) = fVar1;
    if (fVar1 < _DAT_005526e4) {
      *(undefined4 *)(param_1 + 0x108) = 0x3e4ccccd;
    }
  }
  else {
    fVar1 = *(float *)(param_1 + 0x108) + _DAT_005524f4;
    *(float *)(param_1 + 0x108) = fVar1;
    if (_DAT_0055256c < fVar1) {
      *(undefined4 *)(param_1 + 0x108) = 0x3f800000;
    }
  }
  fVar1 = *(float *)(param_1 + 0xc) * *(float *)(param_1 + 0x108);
  iVar2 = *(short *)(param_1 + 2) * 0x38;
  FUN_00511d00((int)*(short *)(param_1 + 2),(float *)(param_1 + 0x10),
               fVar1 * *(float *)((char*)&DAT_083a7cc0 + iVar2),fVar1 * *(float *)((char*)&DAT_083a7cc4 + iVar2),
               (float *)(param_1 + 0xe8),*(float *)(param_1 + 0x24),0.0,0.0,1.0,1.0);
  return;
}


// FUN_0047fcb0 @ 0x0047fcb0 — Sound_Countdown1
// Decrements counter DAT_00559cdc each frame.
// When it underflows below 1, resets to 300 and calls FUN_0047fae0 (queue advance).
void FUN_0047fcb0(void)
{
  bool bVar1;

  bVar1 = DAT_00559cdc < 1;
  DAT_00559cdc = DAT_00559cdc + -1;
  if (bVar1) {
    DAT_00559cdc = 300;
    FUN_0047fae0((char*)DAT_07e11dd0, (unsigned char)0);   // ahora es char[256]
  }
  return;
}


// FUN_00480950 @ 0x00480950 — Sound_Countdown2
// Decrements counter DAT_00559ce4 each frame.
// When it underflows below 1, resets to 0x96 (150) and calls FUN_00480620.
void FUN_00480950(void)
{
  bool bVar1;

  bVar1 = DAT_00559ce4 < 1;
  DAT_00559ce4 = DAT_00559ce4 + -1;

  // Verificado runtime 2026-07-19: el tick dispara cada 150 frames y agrega la
  // linea vacia que hace scrollear el historial superior-izquierdo.
  if (bVar1) {
    DAT_00559ce4 = 0x96;
    // 2026-07-27 FIX (mensajes "whisper" fantasma cada ~6s): este tick re-emite
    // un mensaje periódico guardado en DAT_07e11dd8 (texto) / DAT_07e11ddc
    // (sender) con mode=0 (estilo whisper). En nuestro build NADA puebla esos
    // buffers, así que contenían basura → cada 150 frames aparecía un mensaje
    // con un carácter no-ASCII suelto arriba a la izquierda (confirmado por el
    // diag: "CHATADD mode=0 ret=004E5E13 msg='?'" cada 6.4 s, ret =
    // FUN_00480950+0x63). Sólo re-emitimos si el buffer tiene texto imprimible.
    const char* pMsg = (const char*)&DAT_07e11dd8;
    bool bValid = false;
    for (int i = 0; i < 32; ++i) {
      unsigned char ch = (unsigned char)pMsg[i];
      if (ch == 0) break;                 // fin de string
      if (ch >= 0x20 && ch < 0x7F) { bValid = true; break; }  // ASCII imprimible
    }
    if (bValid) {
      FUN_00480620(&DAT_07e11ddc,&DAT_07e11dd8,0);
    }
  }
  return;
}


// FUN_004fffd0 — implemented in src/stubs.cpp (Sound_SpawnEmitter, cleaner version)
