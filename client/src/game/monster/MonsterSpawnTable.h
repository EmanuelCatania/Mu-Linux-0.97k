#pragma once

#include <string>

// Cómo se crea cada tipo de monstruo o NPC (Content/monster/spawn.toml).
//
// Cada tipo es un índice de Monster.txt. Los campos que quedan en su valor
// por defecto no se escriben sobre el CHARACTER: conservan lo que dejó
// CreateCharacter.

enum class SpawnLoader { None, Monster, Npc };

constexpr int KeepSpawnValue = -32768;

struct MonsterSpawn {
    int         Model = 270;
    SpawnLoader Loader = SpawnLoader::Monster;
    int         Resource = 0;
    int         ExtraResource = -1;
    float       Scale = 0.0f;
    int         Right = -1, Left = -1;          // modelo del item de cada mano
    int         RightLevel = -1, LeftLevel = -1, RightExcellent = -1;
    int         BlendMesh = KeepSpawnValue;
    float       BlendLight = -1.0f;
    int         Subtype = -1, HiddenMesh = KeepSpawnValue;
    int         State446 = -1, Flag766 = -1;    // CHARACTER::Level y CHARACTER::Blood
    std::string Init;                           // ajuste extra: una función de Monster.cpp
};

// Definición del tipo; los tipos sin sección devuelven la de por defecto.
const MonsterSpawn& MonsterSpawn_Get(int type);
