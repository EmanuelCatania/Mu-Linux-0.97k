#!/usr/bin/env python3
"""Verifica que las listas de fuentes de CMake coincidan con los .vcxproj.

Compara, en orden y sin duplicados:
  - CMakeLists.txt (MU_SOURCES / MU_HEADERS)  vs  mu97k.vcxproj (ClCompile / ClInclude)
  - lib/libjpeg/CMakeLists.txt (JPEG_SOURCES / JPEG_HEADERS)
                                              vs  lib/libjpeg/libjpeg.vcxproj

Sale con código 1 si hay cualquier diferencia. Lo corre el CI.

Uso: python tools/check_cmake_sources.py
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def vcxproj_items(path, kind):
    with open(path, encoding="utf-8-sig") as f:
        text = f.read()
    return [s.replace("\\", "/") for s in re.findall(r'<%s Include="([^"]+)"' % kind, text)]


def cmake_list(path, name):
    with open(path, encoding="utf-8") as f:
        text = f.read()
    m = re.search(r"set\(%s\s*\n(.*?)\)" % re.escape(name), text, re.S)
    if not m:
        raise SystemExit("%s: no se encontró set(%s ...)" % (path, name))
    items = []
    for line in m.group(1).splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            items.append(line)
    return items


def compare(label, expected, actual):
    ok = True
    if len(set(actual)) != len(actual):
        dups = sorted({x for x in actual if actual.count(x) > 1})
        print("[%s] duplicados en CMake: %s" % (label, dups))
        ok = False
    missing = [x for x in expected if x not in actual]
    extra = [x for x in actual if x not in expected]
    for x in missing:
        print("[%s] falta en CMake: %s" % (label, x))
    for x in extra:
        print("[%s] sobra en CMake (no está en el .vcxproj): %s" % (label, x))
    if missing or extra:
        ok = False
    elif expected != actual:
        print("[%s] mismo contenido pero distinto orden" % label)
        ok = False
    for x in actual:
        if not os.path.isfile(os.path.join(ROOT, x if label.startswith("mu97k") else os.path.join("lib/libjpeg", x))):
            print("[%s] el archivo no existe: %s" % (label, x))
            ok = False
    print("[%s] %d entradas: %s" % (label, len(actual), "OK" if ok else "ERROR"))
    return ok


def main():
    results = [
        compare("mu97k ClCompile",
                vcxproj_items(os.path.join(ROOT, "mu97k.vcxproj"), "ClCompile"),
                cmake_list(os.path.join(ROOT, "CMakeLists.txt"), "MU_SOURCES")),
        compare("mu97k ClInclude",
                vcxproj_items(os.path.join(ROOT, "mu97k.vcxproj"), "ClInclude"),
                cmake_list(os.path.join(ROOT, "CMakeLists.txt"), "MU_HEADERS")),
        compare("libjpeg ClCompile",
                vcxproj_items(os.path.join(ROOT, "lib/libjpeg/libjpeg.vcxproj"), "ClCompile"),
                cmake_list(os.path.join(ROOT, "lib/libjpeg/CMakeLists.txt"), "JPEG_SOURCES")),
        compare("libjpeg ClInclude",
                vcxproj_items(os.path.join(ROOT, "lib/libjpeg/libjpeg.vcxproj"), "ClInclude"),
                cmake_list(os.path.join(ROOT, "lib/libjpeg/CMakeLists.txt"), "JPEG_HEADERS")),
    ]
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())
