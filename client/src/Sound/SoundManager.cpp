// SoundManager.cpp — CSound. Ver SoundManager.h.

#include "stdafx.h"
#include "Platform/Text.h"
#include "Platform/IAudio.h"
#include "Platform/IFileSystem.h"
#include "Sound/SoundManager.h"

#ifndef MU_MINIAUDIO_LINKED
#define MINIAUDIO_IMPLEMENTATION
#endif
#include "miniaudio.h"

#include <math.h>

extern "C" void DbgLogPublic(const char* msg);

CSound gSound;

namespace {
constexpr int kMaxLevel = 9;

// La música se decodifica desde un FILE* abierto con IFileSystem. Pasarle el
// nombre directo a miniaudio no servía en Linux: los nombres del binario traen
// barras invertidas y no respetan mayúsculas (Data\Music\Tarkan.mp3 vs tarkan.mp3), y
// sólo IFileSystem::Open resuelve las dos cosas.
struct MusicStream {
    FILE* file = nullptr;
    ma_decoder decoder;
};
ma_result ReadMusic(ma_decoder* decoder, void* out, size_t bytes, size_t* read)
{
    auto* stream = static_cast<MusicStream*>(decoder->pUserData);
    const size_t count = fread(out, 1, bytes, stream->file);
    if (read) *read = count;
    return count == 0 && bytes > 0 ? MA_AT_END : MA_SUCCESS;
}
ma_result SeekMusic(ma_decoder* decoder, ma_int64 offset, ma_seek_origin origin)
{
    auto* stream = static_cast<MusicStream*>(decoder->pUserData);
    const int whence = origin == ma_seek_origin_current ? SEEK_CUR : origin == ma_seek_origin_end ? SEEK_END : SEEK_SET;
    return fseek(stream->file, long(offset), whence) == 0 ? MA_SUCCESS : MA_ERROR;
}
}

long CSound::LevelToDirectSound(int level)
{
    // Fórmula del DLL (CSound::UpdateSoundVolumeLevel): 0 es mudo y cada nivel
    // suma 6,25 dB hasta llegar a 0 dB (sin atenuación) en el 9.
    if (level <= 0) return -10000;
    long vol = 625 * (level - 1) - 5000;
    return vol > 0 ? 0 : vol;
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
        Platform::GetAudio().SetVolume(buffer, channel, LevelToDirectSound(m_SoundLevel));
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
    auto* stream = new MusicStream;
    stream->file = Platform::GetFileSystem().Open(fileName, "rb");
    const ma_decoder_config config = ma_decoder_config_init_default();
    if (!stream->file || ma_decoder_init(ReadMusic, SeekMusic, stream, &config, &stream->decoder) != MA_SUCCESS) {
        if (stream->file) fclose(stream->file);
        delete stream;
        return false;
    }
    ma_sound* sound = new ma_sound;
    if (ma_sound_init_from_data_source(static_cast<ma_engine*>(m_Engine), &stream->decoder,
                                       MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, sound) != MA_SUCCESS) {
        delete sound;
        ma_decoder_uninit(&stream->decoder);
        fclose(stream->file);
        delete stream;
        return false;
    }
    m_MusicStream = stream;
    ma_sound_set_volume(sound, LevelToGain(m_MusicLevel));
    ma_sound_start(sound);
    m_Music = sound;

    char line[300];
    Platform::TextFormat(line, "Music: %s", fileName);
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
    if (auto* stream = static_cast<MusicStream*>(m_MusicStream)) {
        ma_decoder_uninit(&stream->decoder);
        fclose(stream->file);
        delete stream;
        m_MusicStream = nullptr;
    }
}

void CSound::SetMusicStoppedByUser(bool stopped)
{
    m_UserStopped = stopped;
    if (stopped) StopMusic();
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
