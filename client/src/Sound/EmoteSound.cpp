// EmoteSound.cpp — sonido de los emotes propios. Ver EmoteSound.h.
#include "stdafx.h"
#include "Sound/EmoteSound.h"
#include "Sound/SoundManager.h"
#include "Model/Model_CustomAnim.h"
#include "globals.h"
#include "functions.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>

namespace {
constexpr float kBaseGain = 0.30f;       // "no tan fuerte": un tercio del volumen del juego
constexpr float kNearDistance = 800.0f;  // 8 casillas: volumen completo
constexpr float kFarDistance  = 2500.0f; // desde aca, silencio

struct Active {
    unsigned char* character;
    int emote;
    int voice;
    std::chrono::steady_clock::time_point start;
    float duration;   // segundos de una pasada del emote
};
std::vector<Active> s_active;

float DistanceGain(unsigned char* character)
{
    unsigned char* hero = (unsigned char*)(uintptr_t)DAT_07abf5d8;
    if (!hero || hero == character) return 1.0f;
    const float dx = AsObject(character)->Position[0] - AsObject(hero)->Position[0];
    const float dy = AsObject(character)->Position[1] - AsObject(hero)->Position[1];
    const float d = sqrtf(dx * dx + dy * dy);
    if (d <= kNearDistance) return 1.0f;
    if (d >= kFarDistance) return 0.0f;
    return 1.0f - (d - kNearDistance) / (kFarDistance - kNearDistance);
}

constexpr int kFadeMs = 500;   // al cortarse el emote el audio se apaga en medio segundo

void Remove(size_t i)
{
    gSound.FadeOutEmoteSound(s_active[i].voice, kFadeMs);
    s_active.erase(s_active.begin() + i);
}
}

void EmoteSound_Start(unsigned char* character, int emote)
{
    if (!character) return;
    for (size_t i = 0; i < s_active.size(); ++i)
        if (s_active[i].character == character) { Remove(i); break; }

    const std::string candidates = CustomAnim_Sound(emote);
    const float gain = kBaseGain * DistanceGain(character);
    if (candidates.empty()) return;
    if (gain <= 0.0f) return;

    // Candidatos separados por '|': se prueba uno al azar y, si no esta, los demas.
    std::vector<std::string> names;
    for (size_t p = 0; p < candidates.size();) {
        size_t q = candidates.find('|', p);
        if (q == std::string::npos) q = candidates.size();
        if (q > p) names.push_back(candidates.substr(p, q - p));
        p = q + 1;
    }
    if (names.empty()) return;
    const size_t first = (size_t)rand() % names.size();
    for (size_t k = 0; k < names.size(); ++k) {
        char path[300];
        snprintf(path, sizeof(path), "Data\\Sound\\Dances\\%s.mp3", names[(first + k) % names.size()].c_str());
        const int voice = gSound.PlayEmoteSound(path, gain, CustomAnim_IsLoop(emote));
        if (voice >= 0) { s_active.push_back({character, emote, voice, std::chrono::steady_clock::now(), CustomAnim_Duration(emote)}); return; }
    }
}

void EmoteSound_Update()
{
    gSound.UpdateEmoteSounds();
    for (size_t i = 0; i < s_active.size();) {
        Active& a = s_active[i];
        unsigned char* c = a.character;
        const bool alive = c && c[0] &&
            CustomAnim_EmoteOfAction(AsObject(c)->CurrentAction) == a.emote;
        if (!alive || !gSound.IsEmoteSoundPlaying(a.voice)) {
            // Un emote sin loop que termino solo deja sonar el audio hasta el final; si se
            // cancelo antes (caminando, otro emote) el audio se apaga.
            const float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - a.start).count();
            if (!alive && !CustomAnim_IsLoop(a.emote) && elapsed >= a.duration * 0.9f) s_active.erase(s_active.begin() + i);
            else Remove(i);
            continue;
        }
        gSound.SetEmoteSoundGain(a.voice, kBaseGain * DistanceGain(c));
        ++i;
    }
}
