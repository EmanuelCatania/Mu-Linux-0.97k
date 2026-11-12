#pragma once
// Acciones propias agregadas al modelo del jugador (Data/Player/CustomAnim.bmd).
// No pisan player.bmd: se anexan al final de su tabla de acciones al cargarlo,
// de modo que sobreviven a un cambio de version de player.bmd.

// Anexa las acciones del archivo al modelo del jugador (slot 390).
// Devuelve la cantidad anexada (0 si no hay archivo o no es compatible).
int  CustomAnim_AppendToPlayer();
// Indice de la primera accion propia dentro del modelo (-1 si no hay).
int  CustomAnim_FirstAction();
// Enlaza el emote `emote` a un canal libre del modelo y devuelve el indice de
// accion a reproducir (-1 si no existe). Ver Model_CustomAnim.cpp.
int  CustomAnim_Acquire(int emote);
int  CustomAnim_Count();
// Datos del sidecar CustomAnim.txt (una linea por accion: nombre loop velocidad).
const char* CustomAnim_Name(int n);
// true si la accion absoluta (indice del modelo) es propia y termina sola al
// acabar la animacion (loop=0); las de loop=1 se repiten hasta que se cancelan.
bool CustomAnim_EndsOnFinish(int action);
// Emote enlazado a la accion absoluta `action` (-1 si no es un canal de emote).
int  CustomAnim_EmoteOfAction(int action);
// Sonidos candidatos del emote, separados por '|' (vacio si no tiene) y si repite.
const char* CustomAnim_Sound(int emote);
bool CustomAnim_IsLoop(int emote);
// Fuerza (1) o quita (-1) el loop de un emote mientras dura su audio; ver EmoteSound.cpp.
void CustomAnim_SetLoopOverride(int emote, int value);
// Duracion en segundos de una pasada del emote (a 25 ticks por segundo).
float CustomAnim_Duration(int emote);
