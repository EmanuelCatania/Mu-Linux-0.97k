#pragma once

// DESVIACION (Fase 4): historial de los últimos mensajes y comandos enviados
// desde el campo del mensaje del chat, recorrible con las flechas. El 0.97k
// sólo guarda los nombres de susurro (anillo de 0x07E113E4, campo 1).
namespace ChatHistory {
// Guarda una línea enviada (vacía o igual a la última: no se repite).
void Push(const char* line);
// Vuelve al texto que se estaba escribiendo (al abrir el chat o al enviar).
void ResetBrowse();
// `older` = flecha arriba. Carga la entrada en el campo 0 del input; la
// primera vez guarda lo que se estaba escribiendo para poder volver.
// Devuelve false si ya no hay más entradas en esa dirección.
bool Browse(bool older);
}
