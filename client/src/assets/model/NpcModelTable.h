#pragma once

#include <string>
#include <utility>
#include <vector>

// Cómo se carga cada modelo de NPC (Content/npc/models.toml).

struct NpcModel {
    std::string Model;      // Data/Npc/<Model><Index>.bmd
    int         Index = 1;
    int         PartCount = 0;
    // (archivo, slot base): la variante i de la parte va al slot base + i.
    std::vector<std::pair<std::string, int>> Parts;
    bool        Texture = false;       // OpenTexture del propio modelo desde Data/Npc
    bool        MarksLoaded = false;   // DAT_0055a7c4 = 1
};

// Definición del modelo de NPC, o nullptr si ese slot no tiene sección.
const NpcModel* NpcModel_Get(int model);
