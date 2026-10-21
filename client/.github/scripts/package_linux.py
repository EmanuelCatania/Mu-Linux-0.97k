#!/usr/bin/env python3
"""Empaqueta el ELF32 y únicamente los assets versionados, sin preferencias locales."""
import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--version", default="0.97.30")
    args = parser.parse_args()
    if not re.fullmatch(r"[A-Za-z0-9._-]+", args.version):
        parser.error("Versión inválida")
    repo = Path(__file__).resolve().parents[2]
    executable = args.build.resolve() / "bin/main"
    header = executable.read_bytes()[:20]
    if header[:5] != b"\x7fELF\x01" or header[18:20] != b"\x03\x00":
        parser.error("Se requiere un ejecutable ELF32 i386")
    symbols = subprocess.check_output(["objdump", "-T", str(executable)], text=True)
    versions = {tuple(map(int, v.split('.'))) for v in re.findall(r"GLIBC_([0-9.]+)", symbols)}
    if not versions or max(versions) > (2, 35):
        parser.error("El binario requiere una glibc posterior a Ubuntu 22.04 (2.35)")
    dependencies = subprocess.check_output(["ldd", str(executable)], text=True)
    if "not found" in dependencies:
        parser.error("Faltan bibliotecas de sistema: " + dependencies)
    if re.search(r"lib(?:SDL3|freetype|miniaudio)[.-]", dependencies, re.I):
        parser.error("SDL3, FreeType y miniaudio deben estar enlazados estáticamente")
    cache = (args.build / "CMakeCache.txt").read_text(encoding="utf-8")
    def source(name):
        override = re.search(rf"^FETCHCONTENT_SOURCE_DIR_{name.upper()}:PATH=(.+)$", cache, re.M)
        return Path(override[1]) if override else args.build / "_deps" / (name + "-src")
    args.output.mkdir(parents=True, exist_ok=True)
    name = "mu97k-" + args.version + "-linux-x86"
    with tempfile.TemporaryDirectory() as temporary:
        package = Path(temporary) / name
        package.mkdir()
        shutil.copy2(executable, package / "main")
        shutil.copy2(repo / ".github/scripts/launch-linux.sh", package / "launch.sh")
        (package / "main").chmod(0o755)
        (package / "launch.sh").chmod(0o755)
        assets = subprocess.check_output(["git", "-C", str(repo), "ls-files", "-z", "bin/Client/Data"])
        for raw in assets.split(b"\0"):
            if not raw:
                continue
            relative = Path(raw.decode("utf-8"))
            destination = package / relative.relative_to("bin/Client")
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(repo / relative, destination)
        licenses = package / "licenses"
        licenses.mkdir()
        for origin, destination in [
            (source("mu_sdl") / "LICENSE.txt", "SDL3.txt"),
            (source("mu_miniaudio") / "LICENSE", "miniaudio.txt"),
            (source("mu_freetype") / "LICENSE.TXT", "FreeType.txt"),
            (source("mu_freetype") / "docs/FTL.TXT", "FreeType-FTL.txt"),
            (source("mu_freetype") / "docs/GPLv2.TXT", "FreeType-GPLv2.txt"),
            (repo / "lib/stb/stb_image.h", "stb_image.h"),
            (repo / "lib/stb/stb_image_write.h", "stb_image_write.h"),
            (repo / "src/Platform/GL/LICENSE.glad", "glad.txt"),
            (repo / "src/Platform/GL/glad.h", "glad.h"),
        ]:
            shutil.copy2(origin, licenses / destination)
        (package / "runtime-libraries.txt").write_text(dependencies, encoding="utf-8")
        (package / "main.sha256").write_text(hashlib.sha256(executable.read_bytes()).hexdigest() + "  main\n", encoding="ascii")
        for readme in ("README.md", "README.en.md", "README.pt-BR.md"):
            shutil.copy2(repo / readme, package / readme)
        (package / "README.txt").write_text(
            "Ejecutar ./launch.sh. Requiere bibliotecas del sistema de 32 bits y OpenGL.\n"
            "Preferencias, capturas y debug.log: ${XDG_DATA_HOME:-$HOME/.local/share}/mu97k.\n"
            "MU_ASSET_ROOT apunta a la carpeta que contiene Data (puede estar en NTFS).\n"
            "SDL_VIDEODRIVER=wayland o x11 fuerza el backend; ausente: selección por sesión.\n"
            "FreeType se distribuye bajo FreeType License; ver licenses/FreeType-FTL.txt.\n"
            "Portions of this software are copyright (c) The FreeType Project (www.freetype.org). All rights reserved.\n",
            encoding="utf-8")
        archive = args.output / (name + ".tar.gz")
        with tarfile.open(archive, "w:gz") as output:
            output.add(package, arcname=name)
        digest = hashlib.sha256(archive.read_bytes()).hexdigest()
        archive.with_suffix(archive.suffix + ".sha256").write_text(digest + "  " + archive.name + "\n", encoding="ascii")
        print(archive)
        print("GLIBC máxima:", ".".join(map(str, max(versions))))
        print(dependencies)


if __name__ == "__main__":
    main()
