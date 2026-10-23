#!/usr/bin/env python3
"""Impide reintroducir OpenGL en productores y contratos públicos BMD."""
import re
from pathlib import Path

root = Path(__file__).resolve().parents[2]
files = sorted((root / "src/Render").glob("BMD*.cpp"))
files += [root / "src/Render" / name for name in
          ("BMDDevice.h", "BufferedDraw.h", "MeshDraw.h", "Skinning.h")]
# Quitar comentarios y literales antes de buscar símbolos: las referencias
# históricas a IDA y los nombres dentro de mensajes no son dependencias GL.
ignored = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
forbidden = re.compile(r"\b(?:gl[A-Z]\w*|glu[A-Z]\w*|GL_\w+|GL[a-z]\w*)\b")
errors = []
for path in files:
    text = ignored.sub(lambda m: "\n" * m.group().count("\n"), path.read_text(encoding="utf-8"))
    for match in forbidden.finditer(text):
        line = text.count("\n", 0, match.start()) + 1
        errors.append(f"{path.relative_to(root)}:{line}: {match.group()}")
if errors:
    raise SystemExit("Dependencias GL fuera del backend BMD:\n" + "\n".join(errors))
print(f"Frontera BMD: {len(files)} productores/contratos sin dependencias GL")
