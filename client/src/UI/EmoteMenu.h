#pragma once
// Menus del chat con los emotes: el boton de la derecha del popup del chat abre la lista de
// emotes de CustomAnim.bmd (Fortnite) y el de la izquierda la de gestos clasicos del juego
// (saludar, aplaudir... mas Hustle, Dance y los que se agregaron). Un click lanza el emote
// con el heroe de frente a la camara. Todo en el espacio 640x480 del HUD, como el resto del chat.
namespace EmoteMenu {
enum Which { Fortnite = 0, Classic = 1 };
// Abre o cierra una lista (cierra la otra).
void Toggle(Which which);
bool IsOpen(Which which);
bool AnyOpen();
// Aplica el emote elegido en una lista. Se llama al empezar cada paso de la simulacion:
// las listas se dibujan en el render, que restaura el estado del heroe al terminar el
// cuadro, asi que lanzar el emote desde ahi se perdia.
void ApplyPending();
// Dibuja la lista abierta y atiende sus clicks. `bottomY` es el borde superior de la
// franja de botones: la lista se apoya justo encima.
void Render(int bottomY);
// Rueda del mouse sobre una lista abierta: cambia de pagina (arriba = anterior). Devuelve
// true si la consumio, para que no llegue al zoom de la camara ni al scroll del chat.
bool Wheel(int notches);
// true si (x, y) cae sobre una lista: el click no es del mundo.
bool Contains(int x, int y);
}
