#!/usr/bin/env python3
"""Mueve una carpeta de client/src a su lugar nuevo (Fase 5, E3) sin cambiar código.

    python tools/move_src_folder.py Sound audio
    python tools/move_src_folder.py Party game/party

Hace `git mv` de la carpeta y actualiza todo lo que la nombra:
  - los #include "Vieja/..." de src/ (sólo líneas de include: no toca rutas
    de datos como "Data/Sound/...") y las menciones src/Vieja/ de los comentarios;
  - las rutas src/Vieja/ y src\\Vieja\\ de CMakeLists.txt, mu97k.vcxproj,
    src/Platform/SDL3/CMakeLists.txt, .github/scripts/ y tests/;
  - tests/source_paths.py, para que los tests que leen revisiones viejas
    encuentren el archivo en su ruta anterior.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def files(patterns):
    for pattern in patterns:
        yield from sorted(ROOT.glob(pattern))


def rewrite(path, fn):
    raw = path.read_bytes()
    crlf = b'\r\n' in raw
    text = raw.decode('utf-8').replace('\r\n', '\n')
    new = fn(text)
    if new != text:
        path.write_bytes((new.replace('\n', '\r\n') if crlf else new).encode('utf-8'))
        return 1
    return 0


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    old, new = sys.argv[1].strip('/'), sys.argv[2].strip('/')
    src_old, src_new = ROOT / 'src' / old, ROOT / 'src' / new
    if not src_old.is_dir():
        sys.exit(f'no existe src/{old}')
    src_new.parent.mkdir(parents=True, exist_ok=True)
    if src_new.exists():
        # Destino ya creado por otra tanda: mover archivo por archivo.
        for f in sorted(src_old.rglob('*')):
            if f.is_file():
                target = src_new / f.relative_to(src_old)
                if target.exists():
                    sys.exit(f'ya existe {target}')
                target.parent.mkdir(parents=True, exist_ok=True)
                subprocess.check_call(['git', 'mv', str(f), str(target)], cwd=ROOT)
    else:
        subprocess.check_call(['git', 'mv', f'src/{old}', f'src/{new}'], cwd=ROOT)

    include = re.compile(r'^(\s*#\s*include\s*["<])' + re.escape(old) + r'/', re.M)
    changed = 0
    slash = re.compile(r'src/' + re.escape(old) + r'/')
    for f in files(['src/**/*.cpp', 'src/**/*.h', 'src/**/*.c', 'src/**/*.hpp', 'src/**/*.rc']):
        # includes y las menciones "src/Vieja/..." de los comentarios
        changed += rewrite(f, lambda t: slash.sub(f'src/{new}/', include.sub(r'\g<1>' + new + '/', t)))
    back = re.compile(r'src\\' + re.escape(old.replace('/', '\\')) + r'\\')
    for f in files(['CMakeLists.txt', 'src/Platform/SDL3/CMakeLists.txt', 'mu97k.vcxproj', 'mu97k.vcxproj.filters',
                    '.github/scripts/*.py', 'tests/*.py']):
        if f.name == 'source_paths.py':
            continue
        changed += rewrite(f, lambda t: back.sub('src\\\\' + new.replace('/', '\\\\') + '\\\\', slash.sub(f'src/{new}/', t)))
    # Los tests arman código C++ con #include "Vieja/..." dentro de strings.
    embedded = re.compile(r'(#include \\?")' + re.escape(old) + r'/')
    for f in files(['tests/*.py']):
        changed += rewrite(f, lambda t: embedded.sub(r'\g<1>' + new + '/', t))

    def add_moved(t):
        line = f'    "src/{new}/": "src/{old}/",\n'
        return t if line in t else t.replace('MOVED = {\n', 'MOVED = {\n' + line, 1)
    rewrite(ROOT / 'tests' / 'source_paths.py', add_moved)
    print(f'src/{old} -> src/{new}: {changed} archivos actualizados')


if __name__ == '__main__':
    main()
