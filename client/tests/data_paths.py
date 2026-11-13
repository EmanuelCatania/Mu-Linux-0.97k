"""Dónde está hoy un archivo de datos que el código pide como Data/<ruta> (Fase 5).

Misma resolución que Platform/ContentLayers del cliente, sin packs ni server:
primero el registro de entidades (Content/<tipo>/.../<tipo>.toml con
`source`), después los alias de Content/alias.toml, la ruta tal cual dentro de
Content/ y al final Data/.  Así un test que pide "Player/Angel.bmd" sigue
encontrando el archivo aunque se haya movido.
"""
import re
from pathlib import Path


def _read_aliases(content):
    aliases = []
    alias_file = content / "alias.toml"
    if not alias_file.is_file():
        return aliases
    section = ""
    for line in alias_file.read_text(encoding="utf-8").splitlines():
        line = line.split("#", 1)[0].strip() if not line.lstrip().startswith('"') else line.strip()
        if not line:
            continue
        if line.startswith("["):
            section = line.strip("[]").strip().lower()
            continue
        m = re.match(r'"([^"]+)"\s*=\s*"([^"]+)"', line)
        if section == "alias" and m:
            aliases.append((m.group(1).replace("\\", "/").strip("/").lower(), m.group(2).strip("/")))
    aliases.sort(key=lambda a: len(a[0]), reverse=True)
    return aliases


def _read_entities(content):
    files = {}
    if not content.is_dir():
        return files
    for type_dir in sorted(p for p in content.iterdir() if p.is_dir()):
        for toml in sorted(type_dir.rglob("*.toml"), key=lambda p: str(p).lower()):
            if toml.name.lower() != type_dir.name.lower() + ".toml":
                continue
            m = re.search(r'^source\s*=\s*"([^"]+)"', toml.read_text(encoding="utf-8"), re.M)
            if not m:
                continue
            source = m.group(1).replace("\\", "/").strip("/")
            for f in toml.parent.iterdir():
                if f.is_file() and f.suffix.lower() != ".toml":
                    files.setdefault(f"{source}/{f.name}".lower(), f)
    return files


_cache = {}


def asset(root, path):
    """Path real de Data/<path> (relativa al interior de Data) bajo root/bin/Client."""
    client = Path(root) / "bin" / "Client"
    key = str(client)
    if key not in _cache:
        content = client / "Content"
        _cache[key] = (_read_entities(content), _read_aliases(content))
    entities, aliases = _cache[key]
    inner = path.replace("\\", "/").strip("/")
    if inner.lower().startswith("data/"):
        inner = inner[5:]
    lower = inner.lower()
    candidates = []
    if lower in entities:
        candidates.append(entities[lower])
    for old, new in aliases:
        if lower == old or lower.startswith(old + "/"):
            candidates.append(client / "Content" / (new + inner[len(old):]))
            break
    candidates += [client / "Content" / inner, client / "Data" / inner]
    for c in candidates:
        if c.is_file():
            return c
    return candidates[-1]
