#pragma once

// Los dos tramos compartían temporales; mantenerlos evita cambiar offsets
// heredados entre emisores al separar sus pasadas de dibujo.
struct CharacterEffectScratch {
    float light[3] = {1.0f, 1.0f, 1.0f};
    float offset[3] = {}, position[3] = {}, previous[3] = {}, extra[3] = {};
};
constexpr int CharacterEffects_EarlyStage = 3;
constexpr int CharacterEffects_LateStage = 4;
void CharacterEffects_TickEarly(int entity, CharacterEffectScratch &scratch);
void *CharacterEffects_TickLate(int entity, CharacterEffectScratch &scratch);
