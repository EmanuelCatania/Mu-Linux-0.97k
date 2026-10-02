#!/usr/bin/env python3
"""Compara las líneas de comando de cl/link/lib de dos builds de MSBuild.

Lee los *.command.1.tlog que deja MSBuild en los directorios intermedios
(obj/<Config>/ para el .vcxproj, build/<target>.dir/<Config>/ para CMake) y,
para cada archivo compilado y para el link, compara el conjunto de flags.
Se descartan los argumentos que dependen de la ruta del build (/Fo, /Fd,
/Fp, /I, /OUT, /PDB, /MAP:..., el .obj/.lib de entrada, etc.).

Uso: python tools/compare_build_flags.py <dir_vcxproj> <dir_cmake>
Sale con código 1 si hay diferencias.
"""
import os
import re
import shlex
import sys

# Prefijos de argumentos cuyo valor es una ruta propia de cada build.
PATH_ARGS = ("/FO", "/FD", "/FP", "/FA", "/FR", "/FS", "/I", "/OUT:", "/PDB:",
             "/ILK:", "/MAP:", "/MANIFESTFILE:", "/IMPLIB:", "/PGD:",
             "/LTCGOUT:", "/LIBPATH:", "/TLBOUT:", "/MANIFESTINPUT:",
             "/EXTERNAL:I", "/DEF:", "/PROFILE")
# Defines que CMake agrega por su cuenta y no cambian el código.
IGNORED = {'/D CMAKE_INTDIR="DEBUG"', '/D CMAKE_INTDIR="RELEASE"'}


def read_tlog(path):
    with open(path, "rb") as f:
        data = f.read()
    for enc in ("utf-16", "utf-8-sig", "mbcs" if os.name == "nt" else "latin-1"):
        try:
            return data.decode(enc)
        except (UnicodeDecodeError, LookupError):
            continue
    return data.decode("latin-1")


def parse_tlog(path, per_file):
    """Devuelve {clave: comando} a partir de un .command.1.tlog.

    Con per_file=False (link/lib) hay un único comando por tlog y la clave es
    el nombre del tlog.
    """
    entries = {}
    key = None
    for line in read_tlog(path).splitlines():
        if line.startswith("^") and not per_file:
            key = os.path.basename(path).upper()
        elif line.startswith("^"):
            # La clave son los fuentes (separados por |); se usa el nombre base.
            names = sorted(os.path.basename(p).upper() for p in line[1:].split("|"))
            key = "|".join(names)
        elif key is not None and line.strip():
            entries[key] = line.strip()
    return entries


def normalize(cmd):
    try:
        toks = shlex.split(cmd, posix=False)
    except ValueError:
        toks = cmd.split()
    out = []
    i = 0
    while i < len(toks):
        t = toks[i].strip('"')
        up = t.upper().replace("-", "/", 1) if t.startswith("-") else t.upper()
        # "/D X" y "/I X" pueden venir separados.
        if up in ("/D", "/I", "/U", "/FI") and i + 1 < len(toks):
            nxt = toks[i + 1].strip('"')
            i += 2
            if up == "/I":
                continue
            out.append(up + " " + nxt.upper())
            continue
        i += 1
        if up.startswith(PATH_ARGS):
            continue
        if re.search(r"\.(CPP|C|OBJ|LIB|RES|RC|PCH)$", up) and ("\\" in up or "/" in up[1:] or not up.startswith("/")):
            # Fuentes y entradas con ruta (los .lib del sistema van sin ruta).
            if "\\" in up or "/" in up[1:]:
                continue
        if up.startswith("/D") and len(up) > 2:
            up = "/D " + up[2:]
        out.append(up)
    return sorted(set(o for o in out if o not in IGNORED))


def find_tlogs(root, kind):
    found = {}
    for d, _, files in os.walk(root):
        if "CMakeFiles" in d:
            continue  # builds de prueba de CMake (detección del compilador)
        for f in files:
            if f.lower() == kind.lower():
                found[os.path.join(d, f)] = parse_tlog(os.path.join(d, f),
                                                       kind.lower().startswith(("cl.", "rc.")))
    return found


def merged(root, kind):
    res = {}
    for _, ent in sorted(find_tlogs(root, kind).items()):
        res.update(ent)
    return res


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    a_root, b_root = sys.argv[1], sys.argv[2]
    diffs = 0
    for kind in ("CL.command.1.tlog", "link.command.1.tlog", "Lib-link.command.1.tlog",
                 "lib.command.1.tlog", "rc.command.1.tlog"):
        a = merged(a_root, kind)
        b = merged(b_root, kind)
        if not a and not b:
            continue
        print("== %s: %d entradas (vcxproj) / %d (cmake)" % (kind, len(a), len(b)))
        for k in sorted(set(a) - set(b)):
            print("  sólo en vcxproj: %s" % k)
            diffs += 1
        for k in sorted(set(b) - set(a)):
            print("  sólo en cmake:   %s" % k)
            diffs += 1
        # Agrupar por diferencia para no repetir 248 veces la misma línea.
        groups = {}
        for k in sorted(set(a) & set(b)):
            na, nb = normalize(a[k]), normalize(b[k])
            if na != nb:
                only_a = tuple(x for x in na if x not in nb)
                only_b = tuple(x for x in nb if x not in na)
                groups.setdefault((only_a, only_b), []).append(k)
        for (only_a, only_b), keys in groups.items():
            diffs += len(keys)
            sample = ", ".join(keys[:3]) + (" ..." if len(keys) > 3 else "")
            print("  %d archivo(s) difieren (%s)" % (len(keys), sample))
            print("    sólo vcxproj: %s" % " ".join(only_a))
            print("    sólo cmake:   %s" % " ".join(only_b))
        if not groups:
            print("  flags idénticos")
    print("TOTAL diferencias: %d" % diffs)
    return 1 if diffs else 0


if __name__ == "__main__":
    sys.exit(main())
