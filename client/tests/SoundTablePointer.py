#!/usr/bin/env python3
"""Tabla de sonidos (Content/audio/sound/sounds.toml) leída por el SoundTable real.

Compila src/audio/SoundTable.cpp con un LoadWaveFile que registra cada llamada,
carga [sound] y [monster.0..255] desde el sounds.toml del repo y comprueba:

  * que no haya líneas que el lector rechace;
  * que ningún id se repita dentro de una sección;
  * que los sonidos de [sound] no caigan en 120..419, el rango que
    OpenWorldModels libera en cada cambio de mapa;
  * que cada .wav exista en bin/Client.

Con --reference-ref REV compila además el código anterior a la tabla
(Sound_LoadGameSamples y el último switch de OpenMonsterModel, tal como estaban
en REV) y exige la misma secuencia de llamadas. La tabla se generó así desde
el commit de T3 del refactor; ese es el valor útil para REV.
"""
import argparse
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOML = ROOT / "bin/Client/Content/audio/sound/sounds.toml"


def build(directory, source, args, cwd=None):
    directory.mkdir(parents=True)
    (directory / "main.cpp").write_text(source, encoding="utf-8")
    (directory / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.25)\nproject(SoundTablePointer LANGUAGES CXX)\n"
        "add_executable(pointer_test main.cpp)\ntarget_compile_features(pointer_test PRIVATE cxx_std_17)\n"
        "if(MSVC)\n target_compile_options(pointer_test PRIVATE /utf-8)\nendif()\n", encoding="utf-8")
    command = ["cmake", "-S", str(directory), "-B", str(directory / "build")]
    if args.generator:
        command += ["-G", args.generator]
    if os.name == "nt":
        command += ["-A", args.architecture, "-T", "v143"]
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
    subprocess.run(["cmake", "--build", str(directory / "build"), "--config", "Release"],
                   check=True, stdout=subprocess.DEVNULL)
    binary = directory / "build" / "pointer_test"
    if os.name == "nt":
        binary = directory / "build" / "Release" / "pointer_test.exe"
    return subprocess.check_output([str(binary)], cwd=cwd or directory, text=True,
                                   encoding="utf-8").splitlines()


RECORDER = r'''
#include <cstdio>
static int g_section = -1;   // -1 = [sound]; N = monstruo N
void LoadWaveFile(int id, const char* path, int channels, char spatial)
{
    std::printf("%d|%d|%s|%d|%d\n", g_section, id, path, channels, spatial ? 1 : 0);
}
'''


def current_harness():
    source = (ROOT / "src/audio/SoundTable.cpp").read_text(encoding="utf-8")
    for include in ('#include "stdafx.h"', '#include "audio/SoundTable.h"', '#include "functions.h"'):
        source = source.replace(include, "")
    source = source.replace('extern "C" void DbgLogPublic(const char* msg);',
                            'void DbgLogPublic(const char* msg) { std::printf("WARN|%s\\n", msg); }')
    return RECORDER + source + r'''
int main()
{
    SoundTable_LoadStartup();
    for (g_section = 0; g_section < 256; ++g_section) SoundTable_LoadMonster(g_section);
}
'''


def body(text, signature):
    start = text.index("{", text.index(signature))
    depth, end = 1, start + 1
    while depth:
        depth += {"{": 1, "}": -1}.get(text[end], 0)
        end += 1
    return text[start + 1:end - 1]


def reference_harness(revision):
    def show(path):
        return subprocess.check_output(["git", "show", f"{revision}:client/{path}"], cwd=ROOT).decode("utf-8")
    startup = body(show("src/audio/GameSounds.cpp"), "void __cdecl Sound_LoadGameSamples(void)")
    monster = body(show("src/assets/data/Monster_Data.cpp"), "void __cdecl OpenMonsterModel(int param_1)")
    switch = monster[[m.start() for m in re.finditer(r"switch\s*\(\s*param_1\s*\)\s*\{", monster)][-1]:]
    switch = switch[:switch.index("{")] + "{" + body(switch, "switch") + "}"
    kept = []
    for line in switch.splitlines():
        text = line.strip()
        if text.startswith(("switch", "case ", "LoadWaveFile", "return;", "break;", "if (param_1", "{", "}")):
            kept.append(line)
    strings = show("src/globals.cpp")
    symbols = []
    for name in sorted(set(re.findall(r"\bs_Data_Sound_\w+", switch))):
        value = re.search(r"char\s+" + name + r"\[\]\s*=\s*(\"[^\"]*\")\s*;", strings)[1]
        symbols.append(f"const char {name}[] = {value};")
    return RECORDER + "\n".join(symbols) + "\nstatic void Startup() {\n" + startup + "\n}\n" \
        "static void Monster(int param_1) {\n" + "\n".join(kept) + "\n}\n" \
        "int main() { Startup(); for (g_section = 0; g_section < 256; ++g_section) Monster(g_section); }\n"


def normalize(lines):
    calls = []
    for line in lines:
        section, sound, path, channels, spatial = line.split("|")
        path = path.replace("\\", "/")
        if not path.lower().startswith("data/sound/"):
            raise RuntimeError(f"ruta fuera de Data/Sound: {path}")
        calls.append((int(section), int(sound), path[len("data/sound/"):], int(channels), int(spatial)))
    return calls


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture", choices=("Win32", "x64"), default="x64")
    parser.add_argument("--generator")
    parser.add_argument("--reference-ref")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-sound-table-") as temp:
        # El lector abre Content/audio/sound/sounds.toml relativo al directorio de trabajo.
        data = Path(temp) / "data"
        (data / "Content/audio/sound").mkdir(parents=True)
        shutil.copyfile(TOML, data / "Content/audio/sound/sounds.toml")
        output = build(Path(temp) / "current", current_harness(), args, cwd=data)
        warnings = [line for line in output if line.startswith("WARN|")]
        if warnings:
            raise RuntimeError("el lector rechazó líneas:\n" + "\n".join(warnings))
        calls = normalize(output)

        seen = {}
        for section, sound, *_ in calls:
            if sound in seen.setdefault(section, set()):
                raise RuntimeError(f"id {sound} repetido en la sección {section}")
            seen[section].add(sound)
        startup = [c for c in calls if c[0] == -1]
        # 286 (mShadowAttack1) ya está así en OpenSounds del binario: el primer cambio
        # de mapa lo libera y el monstruo sombra lo vuelve a cargar con mShadowAttack2.
        reserved = [c for c in startup if 120 <= c[1] <= 419 and c[1] != 286]
        if reserved:
            raise RuntimeError(f"sonidos globales en el rango que se libera por mapa: {reserved}")
        names = {p.name.lower() for p in (ROOT / "bin/Client").rglob("*") if p.suffix.lower() == ".wav"}
        missing = sorted({c[2] for c in calls if c[2].lower() not in names})
        if missing:
            raise RuntimeError(f"archivos que no están: {missing}")
        monsters = len({c[0] for c in calls if c[0] >= 0})
        print(f"SoundTable: {len(startup)} globales, {monsters} tipos de monstruo, "
              f"{len(calls) - len(startup)} sonidos de monstruo; archivos presentes")

        if args.reference_ref:
            reference = normalize(build(Path(temp) / "reference", reference_harness(args.reference_ref), args))
            if reference != calls:
                for index, (a, b) in enumerate(zip(reference, calls)):
                    if a != b:
                        raise RuntimeError(f"difiere en la llamada {index}: código {a} / tabla {b}")
                raise RuntimeError(f"cantidad distinta: código {len(reference)} / tabla {len(calls)}")
            print(f"Equivalencia con {args.reference_ref}: OK ({len(calls)} llamadas)")


if __name__ == "__main__":
    main()
