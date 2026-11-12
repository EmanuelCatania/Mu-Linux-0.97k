#pragma once

// Datos extra de los items en TOML, un archivo por sección (Fase 5, grupo B).
//
//     Data/Items/00-swords.toml       [020] = sección 0, índice 20
//     Data/Items/13-helpers.toml      [400] = sección 13, índice 400
//     ...
//
// Item/Item.txt sigue siendo la tabla de stats, clases, tamaño y nombre. Estos
// archivos llevan todo lo demás de un item: modelo, glow, pose del inventario,
// arco/ballesta, constantes del ala, efectos, tooltip, pet, apilado y precio.
// Reemplazan a los .txt del Encoder (CustomItem, CustomGlow, CustomBow,
// ItemPosition), CustomWing.txt, ItemStack.txt, ItemValue.txt y los JSON de
// Custom/Items y Custom/Pets. El nombre de la sección en el archivo es sólo
// para leerlo: manda el número.
//
// Qué se manda al cliente lo decide el GameServer (catálogo); el operador no
// tiene que pensar en eso. Sólo del server: stack, value y el pickup del pet.
//
// Este módulo no depende del GameServer: lo usan el GameServer y mu-data.

#include <string>
#include <vector>

#include "../GameServer/JsonLite.h"

#define ITEM_EXTRAS_SECTIONS 16

// Índices válidos dentro de una sección (los vanilla son 0..31; el resto son
// items agregados, como en Item.txt).
#define ITEM_EXTRAS_MAX_INDEX 512

struct ItemExtraStack
{
	int Level = -1;             // -1 = cualquier nivel
	int MaxStack = 0;
	int CreateSection = -1;     // -1 = no crea otro item al llenarse
	int CreateIndex = -1;
};

struct ItemExtraValue
{
	int Level = -1;             // -1 = cualquier nivel
	int Buy = -1;               // -1 = el precio de siempre
	int Sell = -1;
};

struct ItemExtraDef
{
	int Section = 0;
	int Index = 0;

	// Modelo propio (carpeta bajo Data\ del cliente, con '\'). Vacío = el de
	// Item.txt o el vanilla.
	std::string ModelFolder;
	std::string ModelName;

	int Glow = -1;              // 0xRRGGBB, -1 = vanilla
	int Bow = -1;               // 0 = arco, 1 = ballesta, -1 = no es arco custom

	bool HasWing = false;       // constantes del ala custom
	int WingDefenseA = 0;
	int WingIncDamageA = 0;
	int WingIncDamageB = 0;
	int WingDecDamageA = 0;
	int WingDecDamageB = 0;

	// Lo mismo que iba en los JSON de Custom/Items y Custom/Pets; el GameServer
	// lo valida con las reglas de siempre. Null = no tiene.
	JsonValue Inventory;        // { position = [x, y], rotation = [x, y, z], scale = s }
	JsonValue Effects;          // array
	JsonValue Tooltip;          // array
	JsonValue Pet;              // objeto (sin "item")

	std::vector<ItemExtraStack> Stack;
	std::vector<ItemExtraValue> Value;

	std::string Comment;        // comentario sobre el item (líneas)
	std::string Source;         // archivo, para los mensajes de error
};

// Nombre de la sección para el archivo ("swords", "bows", ...).
const char* ItemExtras_SectionName(int section);

// Data/Items/NN-nombre.toml de esa sección (la que ya exista o una nueva).
std::string ItemExtras_SectionPath(const std::string& dataDir, int section);

// true si existe algún Data/Items/NN-nombre.toml.
bool ItemExtras_HasData(const std::string& dataDir);

// Lee todos los archivos de sección, ordenados por sección e índice. Devuelve
// false con el motivo (archivo e item) en `error`.
bool ItemExtras_Load(const std::string& dataDir, std::vector<ItemExtraDef>& out, std::string& error);

// Lee los formatos viejos desde Data/ (los que existan): Custom/Encoder
// (CustomItem, CustomGlow, CustomBow, ItemPosition), Item/CustomWing.txt,
// Item/ItemStack.txt, Item/ItemValue.txt, Custom/Items/*.json y
// Custom/Pets/*.json. Sirve para importar y para comparar contra el TOML.
bool ItemExtras_LoadLegacy(const std::string& dataDir, std::vector<ItemExtraDef>& out, std::string& error);

// Archivos viejos que reemplaza el TOML (rutas relativas a Data/), para que
// mu-data diga cuáles se pueden borrar.
std::vector<std::string> ItemExtras_LegacyFiles(const std::string& dataDir);

// Nombres de Item/Item.txt (GET_ITEM -> nombre), para los comentarios.
void ItemExtras_ReadItemNames(const std::string& itemTxtPath, std::vector<std::string>& names);

// Escribe el archivo de una sección con sus items (los de otra sección se
// ignoran). `names` es opcional (comentario con el nombre de cada item).
bool ItemExtras_WriteSection(const std::string& path, int section, const std::vector<ItemExtraDef>& items, const std::vector<std::string>& names, std::string& error);

// Compara dos listas sin importar el orden entre items. Si difieren deja la
// primera diferencia en `diff`.
bool ItemExtras_Equal(const std::vector<ItemExtraDef>& a, const std::vector<ItemExtraDef>& b, std::string& diff);

// GET_ITEM de MuEmu: los índices >= 32 van al bloque de agregados.
int ItemExtras_GetItem(int section, int index);
