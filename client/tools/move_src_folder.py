#!/usr/bin/env python3
"""Mueve una carpeta de client/src a su lugar nuevo (Fase 5, E3) sin cambiar código.

    python tools/move_src_folder.py Sound audio
    python tools/move_src_folder.py Party game/party
    python tools/move_src_folder.py Item/Item_Data.cpp assets/data/Item_Data.cpp

Hace `git mv` de la carpeta y actualiza todo lo que la nombra:
  - los #include "Vieja/..." de src/ (sólo líneas de include: no toca rutas
    de datos como "Data/Sound/...") y las menciones src/Vieja/ de los comentarios;
  - las rutas src/Vieja/ y src\\Vieja\\ de CMakeLists.txt, mu97k.vcxproj,
    src/platform/SDL3/CMakeLists.txt, .github/scripts/ y tests/;
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


def fix_disk_case(old, new):
    """En Windows `git mv` de Net/x a net/x deja la carpeta del disco como
    "Net"; los scripts que recorren el disco ven la ruta vieja.  Renombrar la
    carpeta y borrar la vieja si quedó vacía."""
    parent = ROOT / 'src' / Path(old).parent
    leaf = Path(old).name
    on_disk = [d for d in parent.iterdir() if d.is_dir() and d.name.lower() == leaf.lower()]
    for d in on_disk:
        if old.lower() == new.lower():
            if d.name != Path(new).name:
                tmp = d.with_name(d.name + '__case')
                d.rename(tmp)
                tmp.rename(ROOT / 'src' / new)
        else:
            for sub in sorted(d.rglob('*'), key=lambda p: len(p.parts), reverse=True):
                if sub.is_dir() and not any(sub.iterdir()):
                    sub.rmdir()
            if not any(d.iterdir()):
                d.rmdir()


RELATIVE_INCLUDE = re.compile(r'^(\s*#\s*include\s*")((?:\.\./)+[^"]+)(")', re.M)


def fix_relative_includes():
    """Un archivo que cambió de profundidad deja sus `#include "../../lib/..."`
    apuntando a otra carpeta: sumar o quitar un `../` si así resuelve."""
    # Contra la lista de git y no contra el disco: en Windows "../Combat/x.h"
    # existe aunque la carpeta sea "combat", y en Linux no compila.
    import posixpath
    tracked = set(subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode('utf-8').split('\0'))

    def exists(f, path):
        rel = f.relative_to(ROOT).as_posix()
        full = posixpath.normpath(posixpath.join(posixpath.dirname(rel), path))
        return full in tracked or (full.startswith('..') and (f.parent / path).exists())

    fixed = 0
    for f in files(['src/**/*.cpp', 'src/**/*.c', 'src/**/*.h']):
        def fix(m):
            path = m.group(2)
            if exists(f, path):
                return m.group(0)
            for other in ('../' + path, path[3:]):
                if exists(f, other):
                    return m.group(1) + other + m.group(3)
            print(f'sin resolver: {f.relative_to(ROOT)}: {path}')
            return m.group(0)
        fixed += rewrite(f, lambda t: RELATIVE_INCLUDE.sub(fix, t))
    return fixed


def check_cmake_relative():
    """Las fuentes relativas de los CMake de subcarpetas (../../render/...)."""
    for cmake in files(['src/**/CMakeLists.txt']):
        for path in re.findall(r'((?:\.\./)+[\w./-]+\.(?:cpp|c|h))', cmake.read_text(encoding='utf-8')):
            if not (cmake.parent / path).exists():
                print(f'CMake con ruta rota: {cmake.relative_to(ROOT)}: {path}')


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    old, new = sys.argv[1].strip('/'), sys.argv[2].strip('/')
    # Archivo por archivo, con la lista de git y las mayúsculas exactas: en
    # Windows src/Game y src/game son la misma carpeta, así que recorrer el
    # disco mezclaría carpetas distintas para git y "Game" -> "game/core"
    # sería mover una carpeta adentro de sí misma.
    # Una carpeta entera o un archivo suelto (Item/Item_Data.cpp).
    is_file = (not (ROOT / 'src' / old).is_dir()) and subprocess.run(
        ['git', 'ls-files', '--error-unmatch', f'src/{old}'], cwd=ROOT, capture_output=True).returncode == 0
    if is_file:
        tracked = [f'src/{old}']
        sep = ''
        end = r'(?![\w./\\-])'     # el nombre termina ahí: no matchear Item_Data.cpp.bak
        bend = end
    else:
        listed = subprocess.check_output(['git', 'ls-files', '-z', '--', f'src/{old}/'], cwd=ROOT).decode('utf-8')
        tracked = [p for p in listed.split('\0') if p.startswith(f'src/{old}/')]
        sep = '/'
        end = '/'
        bend = r'\\'
    if not tracked:
        sys.exit(f'git no tiene archivos en src/{old}/')
    for path in tracked:
        target = f'src/{new}' + path[len(f'src/{old}'):]
        if subprocess.run(['git', 'ls-files', '--error-unmatch', target], cwd=ROOT,
                          capture_output=True).returncode == 0:
            sys.exit(f'ya existe {target}')
        (ROOT / target).parent.mkdir(parents=True, exist_ok=True)
        subprocess.check_call(['git', 'mv', path, target], cwd=ROOT)
    if not is_file:
        fix_disk_case(old, new)

    include = re.compile(r'^(\s*#\s*include\s*["<])' + re.escape(old) + end, re.M)
    changed = 0
    slash = re.compile(r'src/' + re.escape(old) + end)
    for f in files(['src/**/*.cpp', 'src/**/*.h', 'src/**/*.c', 'src/**/*.hpp', 'src/**/*.rc',
                    'tests/**/*.cpp', 'tests/**/*.h']):
        # includes y las menciones "src/Vieja/..." de los comentarios
        changed += rewrite(f, lambda t: slash.sub(f'src/{new}{sep}', include.sub(r'\g<1>' + new + sep, t)))
    quoted = re.compile(r'''(["'])src/''' + re.escape(old) + r'''(["'])''')
    back = re.compile(r'src\\' + re.escape(old.replace('/', '\\')) + bend)
    back_new = 'src\\' + new.replace('/', '\\') + ('\\' if not is_file else '')
    for f in files(['CMakeLists.txt', 'src/platform/SDL3/CMakeLists.txt', 'mu97k.vcxproj', 'mu97k.vcxproj.filters',
                    '.github/scripts/*.py', 'tests/*.py']):
        if f.name == 'source_paths.py':
            continue
        changed += rewrite(f, lambda t: back.sub(lambda m: back_new, slash.sub(f'src/{new}{sep}', t)))
        # La carpeta sola entre comillas: root / "src/Vieja"
        changed += rewrite(f, lambda t: quoted.sub(r'\g<1>src/' + new + r'\g<2>', t))
    # Los tests arman código C++ con #include "Vieja/..." dentro de strings.
    embedded = re.compile(r'(#include \\?")' + re.escape(old) + end)
    for f in files(['tests/*.py']):
        changed += rewrite(f, lambda t: embedded.sub(r'\g<1>' + new + sep, t))

    def add_moved(t):
        line = f'    "src/{new}{sep}": "src/{old}{sep}",\n'
        return t if line in t else t.replace('MOVED = {\n', 'MOVED = {\n' + line, 1)
    rewrite(ROOT / 'tests' / 'source_paths.py', add_moved)
    changed += fix_relative_includes()
    check_cmake_relative()
    print(f'src/{old} -> src/{new}: {changed} archivos actualizados')


if __name__ == '__main__':
    main()
