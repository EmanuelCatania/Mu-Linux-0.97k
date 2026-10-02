#!/usr/bin/env python3
"""Compara los símbolos de dos main.map generados por link.exe (/MAP).

Toma los nombres de las secciones "Publics by Value" y "Static symbols" (las
direcciones se ignoran: cambian con cualquier edición) y lista los símbolos
que desaparecen o aparecen entre <base.map> y <head.map>.

Uso: python tools/compare_map_symbols.py <base.map> <head.map>
Sale con código 1 si desaparece algún símbolo.
"""
import re
import sys

# Estáticos locales de función: el decorado lleva el número de scope del bloque
# (`?s_x@?DF@??Func@@...`), que cambia si se agrega o quita un bloque antes en la
# misma función aunque la variable sea la misma. Se normaliza ese número.
STATIC_SCOPE = re.compile(r"@\?[0-9A-Z]{1,4}@\?\?")
# Etiquetas internas del compilador ($LN123): no son símbolos del programa.
INTERNAL = re.compile(r"^\$LN\d+$")

LINE = re.compile(r"^\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+[0-9a-fA-F]{8}\s+(.*)$")


def symbols(path):
    syms = {}
    section = None
    with open(path, encoding="latin-1") as f:
        for line in f:
            s = line.strip()
            if s.startswith("Address") and "Publics by Value" in s:
                section = "public"
                continue
            if s.startswith("Static symbols"):
                section = "static"
                continue
            if s.startswith("entry point at") or s.startswith("Exports"):
                section = None if not s.startswith("Exports") else None
                continue
            if section is None:
                continue
            m = LINE.match(line)
            if not m:
                continue
            name = m.group(1)
            if INTERNAL.match(name):
                continue
            name = STATIC_SCOPE.sub("@?#@??", name)
            rest = m.group(2).split()
            obj = rest[-1] if rest else ""
            syms.setdefault(name, set()).add(obj)
    return syms


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    a = symbols(sys.argv[1])
    b = symbols(sys.argv[2])
    lost = sorted(set(a) - set(b))
    new = sorted(set(b) - set(a))
    print("base: %d símbolos, head: %d símbolos" % (len(a), len(b)))
    print("desaparecen: %d" % len(lost))
    for s in lost:
        print("  - %s  (%s)" % (s, ", ".join(sorted(a[s]))))
    print("aparecen: %d" % len(new))
    for s in new:
        print("  + %s  (%s)" % (s, ", ".join(sorted(b[s]))))
    return 1 if lost else 0


if __name__ == "__main__":
    sys.exit(main())
