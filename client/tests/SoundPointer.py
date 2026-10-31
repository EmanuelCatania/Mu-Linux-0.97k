#!/usr/bin/env python3
"""Prueba PlayBuffer y posicionamiento 3D reales sin dispositivo de audio.

Usa tabla y firmas de producción, matemáticas reales y un servicio de audio
que registra llamadas. OBJECT conserva sólo Position en el offset x86.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile

from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

TYPES = r'''
#include <cmath>
#include <type_traits>
#ifndef _WIN32
using BOOL = int;
using HRESULT = std::int32_t;
constexpr BOOL FALSE = 0, TRUE = 1;
constexpr HRESULT S_OK = 0;
#define FAILED(value) ((value) < 0)
#endif
using float10 = long double;
float10 fsin(float10 value) { return std::sin(value); }
float10 fcos(float10 value) { return std::cos(value); }
using vec3_t = float[3];
struct OBJECT {
    BYTE prefix[16];
    POSITION_FIELD
    BYTE suffix[444 - 28];
};
static_assert(sizeof(void*) != 4 || (sizeof(OBJECT) == 444 && offsetof(OBJECT, Position) == 16));
char* Hero;
#define DAT_07abf5d8 Hero
DWORD DAT_083a42c0;
#define Ff(sym) (*(float*)&(sym))
'''

SERVICES = r'''
std::uint64_t digest;
void Hash(const void* data, std::size_t size) {
    const auto* bytes = static_cast<const BYTE*>(data);
    while(size--) { digest ^= *bytes++; digest *= 1099511628211ull; }
}
struct PositionEvent { int slot, channel; float x, y, z; };
struct Audio {
    HRESULT result;
    int calls, slot, channel;
    bool loop;
    std::vector<PositionEvent> positions;
    HRESULT Play(int s, int ch, bool l) { ++calls; slot = s; channel = ch; loop = l; return result; }
    void SetPosition(int s, int ch, float x, float y, float z) { positions.push_back({s, ch, x, y, z}); }
} audio;
namespace Platform { Audio& GetAudio() { return audio; } }
template<class T> T Binding(const OBJECT* owner) {
    if constexpr(std::is_pointer_v<T>) return owner;
    else return static_cast<T>(reinterpret_cast<std::uintptr_t>(owner));
}
template<class T> std::uintptr_t Address(T value) {
    if constexpr(std::is_pointer_v<T>) return reinterpret_cast<std::uintptr_t>(value);
    else return value;
}
void Bind(int slot, int channel, const OBJECT* owner) {
    Object3DSound[slot][channel] = Binding<std::remove_reference_t<decltype(Object3DSound[slot][channel])>>(owner);
}
'''

MAIN = r'''
HRESULT CallPlay(int slot, const OBJECT* owner, BOOL loop) { return CALL_PLAY; }
std::uint64_t Playback(OBJECT* owner) {
    digest = 14695981039346656037ull;
    for(int slot : {-1, 0, 1, 419, 420})
    for(BOOL enabled : {FALSE, TRUE})
    for(bool spatial : {false, true})
    for(int channel : {-1, 0, 1, 3, 4})
    for(int maximum : {0, 1, 3, 4})
    for(BOOL loop : {-1, 0, 1})
    for(HRESULT result : {HRESULT(0), HRESULT(1), HRESULT(-2147467259)})
    for(bool attached : {false, true}) {
        g_EnableSound = enabled;
        std::memset(Object3DSound, 0, sizeof(Object3DSound));
        std::memset(BufferChannel, 0, sizeof(BufferChannel));
        std::memset(MaxBufferChannel, 0, sizeof(MaxBufferChannel));
        std::memset(Enable3DSound, 0, sizeof(Enable3DSound));
        const bool valid = enabled && slot >= 0 && slot < 420;
        const int normalized = channel < 0 || channel >= 4 ? 0 : channel;
        if(slot >= 0 && slot < 420) {
            BufferChannel[slot] = channel; MaxBufferChannel[slot] = maximum; Enable3DSound[slot] = spatial;
        }
        audio.calls = 0; audio.result = result;
        const auto* anchor = attached ? owner : nullptr;
        const HRESULT returned = CallPlay(slot, anchor, loop);
        Require(returned == (valid && result < 0 ? result : S_OK));
        Require(audio.calls == int(valid));
        if(valid) Require(audio.slot == slot && audio.channel == normalized && audio.loop == (loop != 0));
        const bool success = valid && result >= 0;
        for(int s = 0; s < 420; ++s) {
            int expected = s == slot ? channel : 0;
            if(valid && s == slot) expected = success && normalized >= maximum ? 0 : normalized;
            Require(BufferChannel[s] == expected);
            for(int ch = 0; ch < 4; ++ch) {
                const bool stored = success && spatial && s == slot && ch == normalized;
                Require(Address(Object3DSound[s][ch]) == (stored ? reinterpret_cast<std::uintptr_t>(anchor) : 0));
            }
        }
        Hash(&returned, sizeof(returned)); Hash(&audio.calls, sizeof(audio.calls));
        Hash(BufferChannel, sizeof(BufferChannel));
    }
    return digest;
}
std::uint64_t Positions(OBJECT* owners, OBJECT* listener) {
    Hero = reinterpret_cast<char*>(listener);
    for(int i = 0; i < 4; ++i) {
        owners[i].Position[0] = 111.25f * i - 200.0f;
        owners[i].Position[1] = 33.5f * i + 20.0f;
        owners[i].Position[2] = -75.0f + i;
    }
    listener->Position[0] = 710.0f; listener->Position[1] = -120.0f; listener->Position[2] = 345.0f;
    for(int slot = 0; slot < 420; ++slot) {
        Enable3DSound[slot] = slot % 3 != 0; MaxBufferChannel[slot] = slot % 5;
        for(int ch = 0; ch < 4; ++ch) Bind(slot, ch, (slot + ch) % 5 ? &owners[(slot + ch) % 4] : nullptr);
    }
    digest = 14695981039346656037ull;
    for(BOOL enabled : {FALSE, TRUE})
    for(bool spatial : {false, true})
    for(float yaw : {0.0f, 45.0f, 90.0f, 180.0f}) {
        g_EnableSound = enabled; g_Enable3DSound = spatial;
        std::memcpy(&DAT_083a42c0, &yaw, 4);
        audio.positions.clear(); Sound_Update3DPositions();
        std::size_t next = 0;
        for(int slot = 0; slot < 420; ++slot) for(int ch = 0; ch < slot % 5; ++ch) {
            if(!enabled || !spatial || slot % 3 == 0 || (slot + ch) % 5 == 0) continue;
            Require(next < audio.positions.size());
            const auto& event = audio.positions[next++];
            Require(event.slot == slot && event.channel == ch && event.y == 0.0f);
            Require(std::isfinite(event.x) && std::isfinite(event.z));
            if(yaw == 0.0f) {
                const auto& p = owners[(slot + ch) % 4].Position;
                Require(event.x == -(listener->Position[0] - p[0]) * _DAT_005524bc);
                Require(event.z == -(listener->Position[1] - p[1]) * _DAT_005524bc);
            }
            Hash(&event.slot, sizeof(event.slot)); Hash(&event.channel, sizeof(event.channel));
            Hash(&event.x, 4); Hash(&event.y, 4); Hash(&event.z, 4);
        }
        Require(next == audio.positions.size());
    }
    return digest;
}
int main() {
    auto* owners = static_cast<OBJECT*>(Reserve(0x77000000u));
    auto* listener = static_cast<OBJECT*>(Reserve(0x78000000u)); Require(owners && listener);
    const auto playback = Playback(owners);
    const auto positions = Positions(owners, listener);
    if(sizeof(void*) > 4) {
        auto* highOwners = static_cast<OBJECT*>(Reserve(std::uint64_t(0x177000000ull)));
        auto* highListener = static_cast<OBJECT*>(Reserve(std::uint64_t(0x178000000ull))); Require(highOwners && highListener);
        Require(Playback(highOwners) == playback);
        std::memset(owners, 0xcc, sizeof(OBJECT) * 4);
        Require(Positions(highOwners, listener) == positions);
        Require(Positions(owners, highListener) == positions);
        Require(Positions(highOwners, highListener) == positions);
        std::puts("audio sobre 4 GiB: anclas completas y cuatro combinaciones de posición equivalentes");
    }
    std::printf("PlayBuffer: 7200 casos %016llx; posiciones %016llx\n",
                static_cast<unsigned long long>(playback), static_cast<unsigned long long>(positions));
}
'''


def harness(revision=None):
    header, globals_source = (read_source(path, revision) for path in ("src/globals.h", "src/globals.cpp"))
    declarations = ""
    for name in ("g_EnableSound", "g_Enable3DSound", "MaxBufferChannel", "BufferChannel", "Enable3DSound", "Object3DSound"):
        declarations += re.search(rf"extern [^;\n]*\b{name}\b[^;\n]*;", header).group()
        declarations += re.search(rf"^[^/\n]*\b{name}\b\s*(?:\[[^;\n]*\])?\s*=\s*[^;]+;", globals_source, re.M).group()
    for name in ("_DAT_005524bc", "Math_DegreesToRadians"):
        declarations += re.search(rf"^float\s+{name}\s*=\s*[^;]+;", globals_source, re.M).group()
    field = re.search(r"vec3_t\s+Position\s*;", read_source("src/Entity/EntityTypes.h", revision)).group()
    source = read_source("src/Sound/Sound_DS3D.cpp", revision)
    play = function(source, "PlayBuffer")
    update = function(source, "Sound_Update3DPositions")
    math_source = read_source("src/Math/Math_3D.cpp", revision)
    math = function(math_source, "Matrix_BuildFromEuler") + function(math_source, "Vector_Rotate")
    arg = "owner" if "const OBJECT* Object" in play else "static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(owner))"
    return (PREAMBLE.split("BYTE* reservation;")[0] + TYPES.replace("POSITION_FIELD", field)
            + declarations + SERVICES + math + play + update
            + MAIN.replace("CALL_PLAY", f"PlayBuffer(slot, {arg}, loop)"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    if args.reference_ref and (os.name != "nt" or args.architecture != "Win32"):
        parser.error("La referencia DWORD sólo se compara en Windows x86")
    with tempfile.TemporaryDirectory(prefix="mu-sound-pointer-") as temp:
        directory = Path(temp)
        current = build_and_run(directory / "current", harness(), args)
        print(current, end="")
        if args.reference_ref:
            reference = build_and_run(directory / "reference", harness(args.reference_ref), args)
            if current != reference:
                raise RuntimeError("Diferencia con la referencia x86")
            print(f"Equivalencia de bytes/eventos con {args.reference_ref}: OK")


if __name__ == "__main__":
    main()
