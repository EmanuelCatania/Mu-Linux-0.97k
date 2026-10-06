// SoundManager.cpp — CSound. Ver SoundManager.h.

#include "stdafx.h"
#include "Sound/SoundManager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <math.h>

extern "C" void DbgLogPublic(const char* msg);

CSound gSound;

namespace {
constexpr int kMaxLevel = 9;
}

long CSound::LevelToDirectSound(int level)
{
    // Fórmula del DLL (CSound::UpdateSoundVolumeLevel): 0 es mudo y cada nivel
    // suma 6,25 dB hasta llegar a 0 dB (sin atenuación) en el 9.
    if (level <= 0) return DSBVOLUME_MIN;
    long vol = 625 * (level - 1) - 5000;
    return vol > DSBVOLUME_MAX ? DSBVOLUME_MAX : vol;
}

float CSound::LevelToGain(int level)
{
    // La misma curva que LevelToDirectSound, en ganancia lineal.
    if (level <= 0) return 0.0f;
    const double db = (625.0 * (level - 1) - 5000.0) / 100.0;
    return (float)pow(10.0, db / 20.0);
}

void CSound::SetLevels(int soundLevel, int musicLevel)
{
    auto clamp = [](int level) {
        if (level < 0) return kMaxLevel;
        return level > kMaxLevel ? kMaxLevel : level;
    };
    m_SoundLevel = clamp(soundLevel);
    m_MusicLevel = clamp(musicLevel);

    for (int buffer = 0; buffer < 420; ++buffer)
        ApplySoundVolume(buffer);
    if (m_Music)
        ma_sound_set_volume(static_cast<ma_sound*>(m_Music), LevelToGain(m_MusicLevel));
}

void CSound::ApplySoundVolume(int buffer)
{
    if (!g_EnableSound || buffer < 0 || buffer >= 420) return;
    for (int channel = 0; channel < 4; ++channel) {
        if (g_lpDSBuffer[buffer][channel])
            g_lpDSBuffer[buffer][channel]->SetVolume(LevelToDirectSound(m_SoundLevel));
    }
}

bool CSound::EnsureEngine()
{
    if (m_Engine) return true;

    ma_engine* engine = new ma_engine;
    if (ma_engine_init(nullptr, engine) != MA_SUCCESS) {
        delete engine;
        DbgLogPublic("Music: no se pudo iniciar miniaudio");
        return false;
    }
    m_Engine = engine;
    return true;
}

bool CSound::PlayMusic(const char* fileName)
{
    StopMusic();
    if (!fileName || !EnsureEngine()) return false;

    // Streaming: el archivo se decodifica de a poco, sin cargarlo entero.
    ma_sound* sound = new ma_sound;
    const ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;
    if (ma_sound_init_from_file(static_cast<ma_engine*>(m_Engine), fileName, flags,
                                nullptr, nullptr, sound) != MA_SUCCESS) {
        delete sound;
        return false;
    }
    ma_sound_set_volume(sound, LevelToGain(m_MusicLevel));
    ma_sound_start(sound);
    m_Music = sound;

    char line[300];
    wsprintfA(line, "Music: %s", fileName);
    DbgLogPublic(line);
    return true;
}

void CSound::StopMusic()
{
    if (!m_Music) return;
    ma_sound* sound = static_cast<ma_sound*>(m_Music);
    ma_sound_stop(sound);
    ma_sound_uninit(sound);
    delete sound;
    m_Music = nullptr;
}

void CSound::Shutdown()
{
    StopMusic();
    if (m_Engine) {
        ma_engine* engine = static_cast<ma_engine*>(m_Engine);
        ma_engine_uninit(engine);
        delete engine;
        m_Engine = nullptr;
    }
}
