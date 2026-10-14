#include "Platform/IAudio.h"
#include "Platform/IFileSystem.h"
#include <miniaudio.h>
#include <array>
#include <vector>
#include <cmath>
#include <cstring>
#include <limits>

namespace Platform {
namespace {
constexpr int kSlots = 420;
constexpr int32_t kFailure = static_cast<int32_t>(0x80004005u);
struct Buffer {
    std::vector<unsigned char> bytes;
    ma_decoder decoder{};
    ma_sound sound{};
    bool decoded = false;
    bool ready = false;
};
// Sólo PCM WAV, como CWaveFile_Open (0x00404E90); DS3D requiere mono.
bool IsPCM(const std::vector<unsigned char>& bytes, bool spatial) {
    if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) || std::memcmp(bytes.data()+8, "WAVE", 4)) return false;
    for (size_t offset = 12; offset + 8 <= bytes.size();) {
        const auto* p = bytes.data()+offset;
        const uint32_t length = uint32_t(p[4]) | (uint32_t(p[5])<<8) | (uint32_t(p[6])<<16) | (uint32_t(p[7])<<24);
        if (length > bytes.size()-offset-8) return false;
        if (!std::memcmp(p,"fmt ",4)) {
            if (length < 16) return false;
            const unsigned format = p[8] | unsigned(p[9])<<8;
            const unsigned channels = p[10] | unsigned(p[11])<<8;
            return format == 1 && channels > 0 && (!spatial || channels == 1);
        }
        offset += 8 + size_t(length);
        if (length & 1) ++offset;
    }
    return false;
}
class SDLAudio final : public IAudio {
    ma_engine engine{};
    bool initialized = false;
    std::array<Buffer, kSlots> buffers{};
public:
    int32_t Initialize(void*) override {
        if (initialized) return 0;
        auto config = ma_engine_config_init();
        // IDA: InitDirectSound (0x004044A0), mezcla estéreo a 22050 Hz.
        config.channels = 2;
        config.sampleRate = 22050;
        config.gainSmoothTimeInFrames = 1;
        if (ma_engine_init(&config, &engine) != MA_SUCCESS) return kFailure;
        initialized = true;
        return 0;
    }
    int32_t CreateBuffer(int slot, const char* path, int, bool spatial) override {
        if (!initialized || slot < 0 || slot >= kSlots || buffers[slot].ready) return kFailure;
        auto& buffer = buffers[slot];
        FILE* file = GetFileSystem().Open(path, "rb");
        if (!file) return kFailure;
        if (std::fseek(file, 0, SEEK_END) != 0) { std::fclose(file); return kFailure; }
        const long size = std::ftell(file);
        if (size <= 0 || std::fseek(file, 0, SEEK_SET) != 0) { std::fclose(file); return kFailure; }
        try { buffer.bytes.resize(static_cast<size_t>(size)); }
        catch (...) { std::fclose(file); return kFailure; }
        const bool read = std::fread(buffer.bytes.data(), 1, buffer.bytes.size(), file) == buffer.bytes.size();
        std::fclose(file);
        if (!read || !IsPCM(buffer.bytes, spatial)) { ReleaseBuffer(slot); return kFailure; }
        auto config = ma_decoder_config_init(ma_format_f32, 0, 0);
        if (ma_decoder_init_memory(buffer.bytes.data(), buffer.bytes.size(), &config, &buffer.decoder) != MA_SUCCESS) {
            ReleaseBuffer(slot); return kFailure;
        }
        buffer.decoded = true;
        const ma_uint32 flags = spatial ? 0 : MA_SOUND_FLAG_NO_SPATIALIZATION;
        if (ma_sound_init_from_data_source(&engine, &buffer.decoder, flags, nullptr, &buffer.sound) != MA_SUCCESS) {
            ReleaseBuffer(slot); return kFailure;
        }
        buffer.ready = true;
        if (spatial) {
            ma_sound_set_positioning(&buffer.sound, ma_positioning_relative);
            ma_sound_set_attenuation_model(&buffer.sound, ma_attenuation_model_inverse);
            ma_sound_set_min_distance(&buffer.sound, 1.0f);
            ma_sound_set_max_distance(&buffer.sound, 1000000000.0f);
            ma_sound_set_rolloff(&buffer.sound, 1.0f);
            ma_sound_set_doppler_factor(&buffer.sound, 0.0f); // El original no asigna velocidades.
            // IDA: CreateStaticBuffer (0x00404650), posición inicial inmediata.
            SetPosition(slot, 0, -2.0f, 0.0f, 2.0f);
        }
        return 0;
    }
    int32_t Play(int slot, uint32_t channel, bool loop) override {
        // IDA: FillBuffer (0x00404A00) es vacío; sólo existe el canal 0.
        if (slot < 0 || slot >= kSlots || channel != 0 || !buffers[slot].ready) return kFailure;
        auto& sound = buffers[slot].sound;
        ma_sound_set_looping(&sound, loop);
        return ma_sound_start(&sound) == MA_SUCCESS ? 0 : kFailure;
    }
    void Stop(int slot) override {
        if (slot < 0 || slot >= kSlots || !buffers[slot].ready) return;
        auto& sound = buffers[slot].sound;
        ma_sound_stop(&sound);
        // IDA: Sound_StopBuffer (0x00404C60), el slot 0 conserva el cursor.
        if (slot != 0) ma_sound_seek_to_pcm_frame(&sound, 0);
    }
    int32_t ReleaseBuffer(int slot) override {
        if (slot < 0 || slot >= kSlots) return 0;
        auto& buffer = buffers[slot];
        if (buffer.ready) { ma_sound_uninit(&buffer.sound); buffer.ready = false; }
        if (buffer.decoded) { ma_decoder_uninit(&buffer.decoder); buffer.decoded = false; }
        std::vector<unsigned char>().swap(buffer.bytes);
        return 0;
    }
    void SetVolume(int slot, int channel, int32_t hundredthsDb) override {
        if (slot < 0 || slot >= kSlots || channel != 0 || !buffers[slot].ready) return;
        if (hundredthsDb < -10000 || hundredthsDb > 0) return;
        const float gain = hundredthsDb == -10000 ? 0.0f : std::pow(10.0f, hundredthsDb / 2000.0f);
        ma_sound_set_volume(&buffers[slot].sound, gain);
    }
    void SetPosition(int slot, int channel, float x, float y, float z) override {
        if (slot < 0 || slot >= kSlots || channel != 0 || !buffers[slot].ready) return;
        // IAudio usa +Z hacia delante (DirectSound); miniaudio usa -Z. Sin diferir.
        ma_sound_set_position(&buffers[slot].sound, x, y, -z);
    }
    void Shutdown() override {
        if (!initialized) return;
        for (int slot = 0; slot < kSlots; ++slot) ReleaseBuffer(slot);
        ma_engine_uninit(&engine);
        initialized = false;
    }
};
}
IAudio& GetAudio() { static auto* audio = new SDLAudio; return *audio; }
}
