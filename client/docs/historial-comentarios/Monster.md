# Historial de comentarios: `src/Monster/`

Comentarios de desarrollo movidos desde `src/Monster/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Monster/Monster.cpp`

### Línea 486 — antes de `extern "C++" {`

```cpp
// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 5239-6163 (925 lines)
// CreateCharacter, CreateMonster (CreateMonster — big switch)
// =============================================================================
// CreateMonster @ 0x0045CCF0 — CreateMonster(Type, PositionX, PositionY, Key, [phantom])
// Ported from IDA Hex-Rays decompile (10619 bytes).
//
// Spawns a monster/NPC entity by Type ID:
//   1. Loads the BMD model via OpenMonsterModel/OpenNpc.
//   2. Calls CreateCharacter to allocate/find an entity slot keyed by Key.
//   3. Writes per-Type scale (+0x0C), weapon item IDs (+624,+648), animation
//      bases (+504..+600), facing/extra (+446), action flags (+100/+104), etc.
//   4. For "world-tier" monsters (84..136 in worlds 9..16) bumps scale by world group.
//   5. Scans MonsterScript table for a matching Type → overrides display name.
//   6. Tags entity_type at +0x2EB (747) and HeroIndex copy at +0x310 (784).
//   7. Sets entity flags byte (+0x84/+132): 2=normal monster, 4=NPC, 8=ground item.
//
// strcpy(c+449, …) calls in the original switch have been omitted — the trailing
// MonsterScript scan overrides the name field anyway, and the original byte_5599xx
// addresses are Korean strings in the data segment we don't reproduce.
//
// CreateCharacter is also implemented here (was a 3-arg stub).
//
// Helpers used (all already implemented in our codebase):
//   OpenMonsterModel (OpenMonsterModel)  — Monster_Data.cpp
//   CreateCharacterPointer — Entity_Spawn.cpp
//   DeleteCloth (DeleteCloth/Entity_ClearBoneLinks) — stubs.cpp
//   SetCharacterScale — alias macro
//   SetAction
//   Joint_Create (CreateJoint)
//   RequestTerrainHeight
//   OpenNpc (0x005091D0)
```

### Línea 1395 en `CreateMonster` — antes de `{`

```cpp
        // RE-ACTIVADO 2026-07-24: copiar el nombre desde la tabla MonsterScript/
        // NPCName (getMonsterName por Type).  Antes se salteaba porque la tabla
        // era un global de 1 byte; ahora esta bien dimensionada y cargada desde
        // NPCName.txt.  El nombre va a c+0x1c1 (449) — lo lee Target_Render al
        // hacer hover sobre el NPC/mob.  Usa el Type ORIGINAL del packet (el
        // blacksmith es 251), no el model type resuelto.
```
