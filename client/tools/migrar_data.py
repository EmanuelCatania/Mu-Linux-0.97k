#!/usr/bin/env python3
"""Pasa una instalación vieja (con Data/) al orden nuevo de carpetas.

Desde 0.97.50 el cliente ya no trae Data/: el contenido está en Content/ y lo
mínimo para arrancar en System/. Un Data/ viejo se sigue leyendo, pero como la
última capa y sólo por compatibilidad. Esta herramienta lo elimina sin perder
nada:

- Cada archivo de Data/ se busca como lo buscaría el cliente (Content/, con su
  registro y alias.toml, y System/).
- Si el cliente nuevo ya trae ese archivo idéntico, se borra de Data/.
- Si es un archivo oficial de una versión anterior (está en
  tools/datos_oficiales.txt, la lista de todos los archivos publicados), se
  borra: el cliente nuevo trae su reemplazo y la versión vieja lo taparía.
- Si no es ninguna de las dos cosas (un cambio o un custom de esa
  instalación), se mueve a Packs/<pack>/ con la misma ruta. Los packs ganan
  sobre Content/, así que el cliente lo sigue usando igual que antes.
- Al final se borran las carpetas vacías de Data/.

Mantenimiento (en el repo, antes de publicar una versión):

    python tools/migrar_data.py --generar-oficiales

regenera tools/datos_oficiales.txt desde los tags de git.

Uso, desde la carpeta del cliente nuevo (la que tiene main.exe, Content/ y
System/) con el Data/ viejo al lado:

    python tools/migrar_data.py            # muestra qué haría, no toca nada
    python tools/migrar_data.py --aplicar  # lo hace

Opciones: --cliente RUTA (otra carpeta), --pack NOMBRE (por defecto
"datos-anteriores"). Sólo usa la biblioteca estándar de Python 3.8+.
"""
import argparse
import hashlib
import os
import re
import shutil
import sys
from pathlib import Path


def lower(text):
    return text.lower()


def normalize(path):
    out = []
    for part in path.replace("\\", "/").split("/"):
        if part:
            out.append(part)
    return "/".join(out)


def read_aliases(content):
    """alias.toml, sección [alias]: "vieja" = "nueva", del más largo al más corto."""
    aliases = []
    path = content / "alias.toml"
    if not path.is_file():
        return aliases
    section = ""
    for line in path.read_text(encoding="utf-8").splitlines():
        hash_at = line.find("#")
        if hash_at >= 0 and (line.find('"') < 0 or line.find('"') > hash_at):
            line = line[:hash_at]
        line = line.strip()
        if not line:
            continue
        if line.startswith("["):
            section = line.strip("[]").strip().lower()
            continue
        match = re.match(r'"([^"]+)"\s*=\s*"([^"]+)"', line)
        if section == "alias" and match:
            aliases.append((lower(normalize(match.group(1))), normalize(match.group(2))))
    aliases.sort(key=lambda item: len(item[0]), reverse=True)
    return aliases


def read_source(toml):
    for line in toml.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("["):
            break
        match = re.match(r'source\s*=\s*"([^"]+)"', line)
        if match:
            return normalize(match.group(1))
    return ""


def read_entities(content):
    """Registro: Content/<tipo>/.../<tipo>.toml con source; gana el primero alfabético."""
    folders = []
    for kind in sorted(p for p in content.iterdir() if p.is_dir()):
        toml_name = kind.name.lower() + ".toml"
        for root, _dirs, files in os.walk(kind):
            for name in files:
                if name.lower() == toml_name:
                    folders.append((Path(root), name))
    folders.sort(key=lambda item: item[0].relative_to(content).as_posix().lower())
    files = {}
    for folder, toml in folders:
        source = read_source(folder / toml)
        if not source:
            continue
        for item in folder.iterdir():
            if item.is_file() and item.suffix.lower() != ".toml":
                files.setdefault(lower(source + "/" + item.name), item)
    return files


class Resolver:
    """Las capas Content y System del cliente (Platform::ContentLayers)."""

    def __init__(self, client):
        self.content = client / "Content"
        self.system = client / "System"
        self.entities = read_entities(self.content) if self.content.is_dir() else {}
        self.aliases = read_aliases(self.content) if self.content.is_dir() else []
        self.index = {}
        for base in (self.content, self.system):
            if base.is_dir():
                for root, _dirs, names in os.walk(base):
                    for name in names:
                        full = Path(root) / name
                        self.index[lower(full.relative_to(client).as_posix())] = full

    def find(self, inner):
        low = lower(inner)
        if low in self.entities:
            return self.entities[low]
        candidates = []
        for old, new in self.aliases:
            if low == old or low.startswith(old + "/"):
                candidates.append("Content/" + new + inner[len(old):])
                break
        candidates += ["Content/" + inner, "System/" + inner]
        for candidate in candidates:
            hit = self.index.get(lower(candidate))
            if hit is not None:
                return hit
        return None


def same_file(a, b):
    if a.stat().st_size != b.stat().st_size:
        return False
    return hashlib.sha1(a.read_bytes()).digest() == hashlib.sha1(b.read_bytes()).digest()


OFFICIAL_LIST = Path(__file__).resolve().parent / "datos_oficiales.txt"
OFFICIAL_PREFIX = 16   # caracteres del hash que se guardan


def blob_ids(data):
    """Hash de blob de git del archivo, tal cual y con los finales de línea en LF
    (git normaliza los archivos de texto al guardarlos)."""
    ids = set()
    for variant in {data, data.replace(b"\r\n", b"\n")}:
        ids.add(hashlib.sha1(b"blob %d\0" % len(variant) + variant).hexdigest()[:OFFICIAL_PREFIX])
    return ids


def load_official():
    if not OFFICIAL_LIST.is_file():
        return set()
    return {line.strip() for line in OFFICIAL_LIST.read_text(encoding="ascii").splitlines()
            if line.strip() and not line.startswith("#")}


def generate_official():
    """Todos los archivos de datos del cliente publicados en un tag de git."""
    import subprocess
    repo = Path(__file__).resolve().parent.parent
    run = lambda *cmd: subprocess.run(["git", "-C", str(repo), *cmd], check=True,
                                      capture_output=True).stdout.decode("utf-8")
    refs = run("tag").split() + ["HEAD"]
    ids = set()
    for ref in refs:
        for line in run("ls-tree", "-r", ref, "--", "bin/Client/Data", "bin/Client/Content",
                        "bin/Client/System").splitlines():
            ids.add(line.split()[2][:OFFICIAL_PREFIX])
    OFFICIAL_LIST.write_text(
        "# Hashes de blob de git (primeros %d caracteres) de los archivos de datos\n"
        "# publicados en los tags. Lo genera: python tools/migrar_data.py --generar-oficiales\n"
        % OFFICIAL_PREFIX + "\n".join(sorted(ids)) + "\n", encoding="ascii")
    print(f"{OFFICIAL_LIST}: {len(ids)} archivos de {len(refs)} versiones")


def main():
    parser = argparse.ArgumentParser(description="Pasa el Data/ de una instalación vieja al orden nuevo.")
    parser.add_argument("--cliente", type=Path, default=Path("."), help="carpeta del cliente (por defecto, la actual)")
    parser.add_argument("--pack", default="datos-anteriores", help="pack donde quedan los archivos propios de la instalación vieja")
    parser.add_argument("--aplicar", action="store_true", help="hacer los cambios (sin esto sólo se muestran)")
    parser.add_argument("--generar-oficiales", action="store_true", help="(repo) regenerar tools/datos_oficiales.txt desde los tags")
    args = parser.parse_args()
    if args.generar_oficiales:
        generate_official()
        return 0

    client = args.cliente.resolve()
    data = client / "Data"
    if not data.is_dir():
        print(f"No hay Data/ en {client}: nada que migrar.")
        return 0
    if not (client / "Content").is_dir() or not (client / "System").is_dir():
        print(f"{client} no tiene Content/ y System/: copiá primero el cliente nuevo y después corré esto.", file=sys.stderr)
        return 1

    resolver = Resolver(client)
    official = load_official()
    if not official:
        print(f"Aviso: falta {OFFICIAL_LIST.name}; los archivos oficiales de versiones viejas"
              " se van a tratar como propios.", file=sys.stderr)
    pack = client / "Packs" / args.pack
    duplicated, old_official, moved = [], [], []
    for root, _dirs, names in os.walk(data):
        for name in sorted(names):
            source = Path(root) / name
            inner = source.relative_to(data).as_posix()
            target = resolver.find(inner)
            if target is not None and same_file(source, target):
                duplicated.append(source)
            elif blob_ids(source.read_bytes()) & official:
                old_official.append(source)
            else:
                moved.append((source, pack / inner, target))

    print(f"Data/ en {client}:")
    print(f"  {len(duplicated)} archivos iguales a los del cliente nuevo: se borran")
    print(f"  {len(old_official)} archivos oficiales de versiones anteriores: se borran")
    print(f"  {len(moved)} archivos propios de esta instalación: van a Packs/{args.pack}/")
    for source, destination, target in moved[:40]:
        why = "distinto del de " + target.relative_to(client).as_posix() if target else "no está en el cliente nuevo"
        print(f"    {source.relative_to(client).as_posix()}  ({why})")
    if len(moved) > 40:
        print(f"    ... y {len(moved) - 40} más")

    if not args.aplicar:
        print("\nNo se cambió nada. Para hacerlo: agregá --aplicar.")
        return 0

    for source, destination, _target in moved:
        if destination.exists():
            print(f"Ya existe {destination}: no se pisa, queda en Data/.", file=sys.stderr)
            continue
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(str(source), str(destination))
    for source in duplicated + old_official:
        source.unlink()
    for root, _dirs, _names in sorted(os.walk(data), key=lambda item: -len(item[0])):
        if not os.listdir(root):
            os.rmdir(root)
    if data.exists():
        print(f"\nQuedaron archivos en {data} (ver los avisos de arriba).")
        return 1
    print("\nListo: ya no hay Data/.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
