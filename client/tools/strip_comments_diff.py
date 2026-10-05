#!/usr/bin/env python3
"""Verifica que un cambio sólo toque comentarios.

Para cada .cpp/.h/.c/.hpp/.inl que cambió entre <base> y <head>:
  1. toma las dos versiones con `git show`,
  2. quita los comentarios (// y /* */), respetando string literals, char
     literals, raw strings (R"x(...)x") y separadores de dígitos (1'000),
  3. normaliza whitespace (cada línea: espacios colapsados y recortada; las
     líneas vacías se descartan),
  4. compara línea a línea.

Además controla que no cambien los finales de línea: si una línea de código
pasa de CRLF a LF (o al revés) lo reporta como diferencia.

El resultado tiene que ser "0 diferencias de código".

Uso:
    python tools/strip_comments_diff.py [--base fase/1] [--head HEAD] [rutas...]
    python tools/strip_comments_diff.py --head WORKTREE   (cambios sin commitear)

Sin rutas, compara todos los fuentes que difieren entre base y head.
Sale con código 1 si hay alguna diferencia.
"""
import argparse
import difflib
import subprocess
import sys

WORKTREE = "WORKTREE"  # --head WORKTREE: comparar contra el árbol de trabajo
SOURCE_EXT = (".cpp", ".h", ".c", ".hpp", ".inl", ".cc", ".cxx")


def git(*args):
    return subprocess.run(["git"] + list(args), check=True,
                          stdout=subprocess.PIPE).stdout


def git_show(rev, path):
    if rev == WORKTREE:
        # Pasar el archivo por los filtros de git (.gitattributes) para
        # compararlo en la misma forma en que quedaría commiteado.
        try:
            with open(path, "rb") as f:
                data = f.read()
        except OSError:
            return None
        h = subprocess.run(["git", "hash-object", "-w", "--stdin", "--path", path],
                           input=data, check=True, stdout=subprocess.PIPE).stdout
        return git("cat-file", "blob", h.decode().strip())
    try:
        return subprocess.run(["git", "show", "%s:%s" % (rev, path)], check=True,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.DEVNULL).stdout
    except subprocess.CalledProcessError:
        return None


def _ident_start(text, i):
    """Índice de comienzo del token identificador/número que termina en i-1."""
    j = i
    while j > 0 and (text[j - 1].isalnum() or text[j - 1] in "_."):
        j -= 1
    return j


def strip_comments(text):
    """Quita comentarios de C/C++. Cada comentario /* */ se reemplaza por un
    espacio (conservando sus saltos de línea) y cada // hasta el fin de línea
    se elimina (conservando el salto). `text` es str decodificado como latin-1
    para no depender del encoding: en UTF-8 ningún byte de un carácter
    multibyte coincide con / * " ' o \\."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if c == "/" and nxt == "/":
            # Comentario de línea; una barra invertida al final lo continúa.
            i += 2
            while i < n:
                if text[i] == "\\" and text[i + 1:i + 3] == "\r\n":
                    i += 3
                    continue
                if text[i] == "\\" and text[i + 1:i + 2] == "\n":
                    i += 2
                    continue
                if text[i] in "\r\n":
                    break
                i += 1
            continue
        if c == "/" and nxt == "*":
            end = text.find("*/", i + 2)
            if end < 0:
                end = n - 2
            body = text[i:end + 2]
            out.append(" ")
            out.append("".join(ch for ch in body if ch in "\r\n"))
            i = end + 2
            continue
        if c == '"':
            start = _ident_start(text, i)
            prefix = text[start:i]
            if prefix.endswith("R") and prefix in ("R", "LR", "uR", "UR", "u8R"):
                # Raw string: R"delim( ... )delim"
                paren = text.find("(", i)
                delim = text[i + 1:paren]
                close = text.find(")" + delim + '"', paren)
                if close < 0:
                    close = n
                end = close + len(delim) + 2
                out.append(text[i:end])
                i = end
                continue
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                elif text[j] == "\n":
                    break  # string sin cerrar: no seguir más allá de la línea
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
            continue
        if c == "'":
            start = _ident_start(text, i)
            if start < i and text[start].isdigit():
                # Separador de dígitos (C++14): 1'000'000
                out.append(c)
                i += 1
                continue
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == "\\":
                    j += 1
                elif text[j] == "\n":
                    break
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


def code_lines(raw):
    """Devuelve [(línea normalizada, eol)] de las líneas con código."""
    text = strip_comments(raw.decode("latin-1"))
    res = []
    for line in text.split("\n"):
        eol = "CRLF" if line.endswith("\r") else "LF"
        norm = " ".join(line.replace("\r", " ").split())
        if norm:
            res.append((norm, eol))
    return res


def eol_style(raw):
    crlf = raw.count(b"\r\n")
    lf = raw.count(b"\n") - crlf
    return crlf, lf


def compare(base, head, path, show):
    a = git_show(base, path)
    b = git_show(head, path)
    if a is None or b is None:
        state = "agregado" if a is None else "eliminado"
        lines = code_lines(b if a is None else a)
        if lines:
            print("  %s: %s con %d líneas de código" % (path, state, len(lines)))
            return len(lines)
        print("  %s: %s, sin código (sólo comentarios)" % (path, state))
        return 0
    la, lb = code_lines(a), code_lines(b)
    diffs = 0
    ta, tb = [x for x, _ in la], [x for x, _ in lb]
    if ta != tb:
        delta = [d for d in difflib.unified_diff(ta, tb, "base/" + path, "head/" + path,
                                                 lineterm="", n=1)]
        changed = sum(1 for d in delta if d[:1] in "+-" and d[:3] not in ("+++", "---"))
        diffs += changed
        print("  %s: %d líneas de código distintas" % (path, changed))
        if show:
            for d in delta[:200]:
                print("    " + d)
    else:
        # Mismo código: los finales de línea de cada línea de código tienen que
        # coincidir (no convertir CRLF<->LF).
        eol_changes = sum(1 for (x, ea), (_, eb) in zip(la, lb) if ea != eb)
        if eol_changes:
            print("  %s: %d líneas de código cambiaron de fin de línea" % (path, eol_changes))
            diffs += eol_changes
    ca, fa = eol_style(a)
    cb, fb = eol_style(b)
    if (fa == 0 and fb > 0) or (ca == 0 and cb > 0):
        print("  %s: cambió el estilo de fin de línea (CRLF %d->%d, LF %d->%d)"
              % (path, ca, cb, fa, fb))
        diffs += 1
    return diffs


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--base", default="fase/1")
    ap.add_argument("--head", default="HEAD")
    ap.add_argument("-q", "--quiet", action="store_true",
                    help="no mostrar el diff de las líneas distintas")
    ap.add_argument("paths", nargs="*")
    args = ap.parse_args()

    if args.paths:
        paths = args.paths
    else:
        rng = [args.base] if args.head == WORKTREE else [args.base, args.head]
        out = git("diff", "--name-only", "--no-renames", *rng).decode()
        paths = [p for p in out.splitlines() if p.lower().endswith(SOURCE_EXT)]

    print("strip_comments_diff: %s..%s, %d archivo(s) fuente" % (args.base, args.head, len(paths)))
    total = 0
    for p in sorted(paths):
        d = compare(args.base, args.head, p, not args.quiet)
        if d == 0:
            print("  %s: OK" % p)
        total += d
    print("RESULTADO: %d diferencias de código" % total)
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
