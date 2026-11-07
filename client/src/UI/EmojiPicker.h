#pragma once
// Selector de emojis del chat (Fase 4): un botón junto al campo de texto que
// abre una grilla; un click agrega el emoji al mensaje que se está escribiendo.
// Todo en el espacio 640x480 del HUD, como el resto de la interfaz clásica.
namespace EmojiPicker {
// Dibuja el botón y, abierto, la grilla; atiende los clicks. Lo llama el
// render del chat mientras el campo está activo.
void Render();
// El campo de chat se cerró: el selector también.
void OnChatClosed();
// true si (x, y) cae sobre el botón o la grilla: el click no es del mundo.
bool Contains(int x, int y);
}
