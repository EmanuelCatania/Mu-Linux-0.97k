// Sound_Queue.cpp
// Render_DrawSpritePool @ 0x00479730
//
// Pese al nombre del archivo, acá no hay sonido: son RenderSprites
// (Render_DrawSpritePool) y RenderSprite (Render_DrawSprite), que recorren y
// dibujan el pool de sprites en DAT_07c85890 (1002 slots de 0x1bc bytes;
// +0 flag activo, +4 modo de blend), más los timers de avisos y de chat.

#include "stdafx.h"

// Render_DrawSpritePool = RenderSprites. Recorre el effect pool
// y por cada slot activo:
//   - dispatch GL state según blend mode en +4 (GL_SetBlendAdditive/90/80)
//   - llama Render_DrawSprite (RenderSprite) para dibujar el quad
//   - clear active flag
// Es la función que dibuja todos los sprites/glows/sparkles del pool (glow +9
// set, wing FX, weapon FX, particles, etc.). Itera los 1002 slots por índice.
// IDA: RenderSprites
void __cdecl Render_DrawSpritePool(void)
{
    char *pcVar2 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar2 += 0x1bc) {
        if (*pcVar2 != '\0') {
            int blend = *(int*)(pcVar2 + 4);
            if      (blend == 0) GL_SetBlendAdditive();
            else if (blend == 1) GL_SetBlendSrcAlpha();
            else if (blend == 2) GL_SetBlendSrcOver('\x01');
            Render_DrawSprite((int)pcVar2);
            *pcVar2 = 0;
        }
    }
}


// Render_DrawSprite @ 0x00479670
//
// RenderSprite: dibuja un slot del pool de sprites como billboard cuadrado vía
// RenderSprite_0 (Sprite_DrawTexturedQuad).
//
// Primero ajusta el factor +0x108 según el flag +0x160:
//   0 — resta _DAT_005524f4 por frame; piso 0.2 (0x3e4ccccd)
//   1 — suma  _DAT_005524f4 por frame; tope 1.0 (0x3f800000)
//
// Argumentos de RenderSprite_0:
//   textura  = *(short*)(param_1 + 2)
//   posición = (float*)(param_1 + 0x10)
//   ancho/alto = *(float*)(param_1 + 0xc) * factor * Bitmaps[tipo] (+0/+4)
//   luz      = (float*)(param_1 + 0xe8)
//   rotación = *(float*)(param_1 + 0x24)
//
// Globals:
//   _DAT_005524f4  — paso del fade
//   _DAT_005526e4  — piso del fade (0.2)
//   _DAT_0055256c  — float 1.0
//   DAT_083a7cc0/cc4 — Bitmaps[tipo]: ancho/alto de la textura (stride 0x38)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// IDA: RenderSprite
void __cdecl Render_DrawSprite(int param_1)

{
  float fVar1;
  int iVar2;

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
  RenderSprite_0((int)*(short *)(param_1 + 2),(float *)(param_1 + 0x10),
               fVar1 * *(float *)((char*)&DAT_083a7cc0 + iVar2),fVar1 * *(float *)((char*)&DAT_083a7cc4 + iVar2),
               (float *)(param_1 + 0xe8),*(float *)(param_1 + 0x24),0.0,0.0,1.0,1.0);
  return;
}


// Chat_TickNoticeTimer @ 0x0047fcb0 — Sound_Countdown1
// Decrements counter DAT_00559cdc each frame.
// When it underflows below 1, resets to 300 and calls UI_AddNotice (queue advance).
// IDA: MoveNotices
void Chat_TickNoticeTimer(void)
{
  bool bVar1;

  bVar1 = DAT_00559cdc < 1;
  DAT_00559cdc = DAT_00559cdc + -1;
  if (bVar1) {
    DAT_00559cdc = 300;
    UI_AddNotice((char*)DAT_07e11dd0, (unsigned char)0);   // ahora es char[256]
  }
  return;
}


// Chat_TickMessageTimer @ 0x00480950 — Sound_Countdown2
// Decrements counter DAT_00559ce4 each frame.
// When it underflows below 1, resets to 0x96 (150) and calls UIChatLogWindow_AddText.
// IDA: FUN_00480950
void Chat_TickMessageTimer(void)
{
  bool bVar1;

  bVar1 = DAT_00559ce4 < 1;
  DAT_00559ce4 = DAT_00559ce4 + -1;

  // El tick dispara cada 150 frames y agrega la línea vacía que hace
  // scrollear el historial superior izquierdo.
  if (bVar1) {
    DAT_00559ce4 = 0x96;
      // Este es el ENVEJECEDOR del historial de chat, no un "mensaje periodico".
      //
      // IDA 0x480950 llama incondicionalmente con strText (0x07E11DD8) y
      // byte_7E11DDC (0x07E11DDC), y a esos dos globals NO LOS ESCRIBE NADIE en
      // todo el binario: tienen un unico xref cada uno, que es esta misma
      // lectura.  O sea son cadenas VACIAS siempre, y ese es el punto.
      //
      // El primer branch de ChatLB_AddText (sub_40C940) es justamente
      // `if (!*src && !*msg)`: recorre la lista y hace ++nodo[+0x114] en cada
      // entrada.  El render de la linea (slot 23) lee ese contador y empuja la
      // fila hacia arriba, dejando de dibujarla cuando pasa el tope de filas
      // visibles.  O sea la caducidad del historial la produce esta llamada,
      // cada 150 frames.  MoveNotices (0x47FCB0) es el mismo patron para los
      // avisos: CreateNotice(byte_7E11DD0, 0) cada 300, con otro buffer que
      // tampoco escribe nadie.
      //
      // No condicionar la llamada al contenido de los buffers: con buffers vacíos
      // (el caso normal) el historial dejaría de avanzar.
    UIChatLogWindow_AddText(DAT_07e11ddc, DAT_07e11dd8, 0);
  }
  return;
}

