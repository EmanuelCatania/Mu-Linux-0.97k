"""Verifica que el código del juego no use OpenGL directamente.

Sólo src/Render/Backend/** y src/Platform/** pueden llamar a gl*/wgl*/glu*,
usar constantes GL_* o tipos GL*. El resto pasa por Render/Gfx.h y
Render/BufferedDraw.h. Se ignoran comentarios y literales de texto.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
ALLOWED = ('src/Render/Backend/', 'src/Platform/')
ALLOWED_FILES = {'src/Render/GLCallBarrier.h'}

PATTERN = re.compile(
    r'\b(?:w?glu?|wgl)[A-Z][A-Za-z0-9]*\s*\('
    r'|\bGL_[A-Z_0-9]+\b'
    r'|\bGL(?:float|uint|int|enum|ubyte|sizei|boolean|bitfield|double|void)\b'
    r'|#\s*include\s*[<"]GL/')


def strip_comments_and_strings(text):
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith('//', i):
            j = text.find('\n', i)
            i = n if j < 0 else j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            seg = text[i:n if j < 0 else j + 2]
            out.append('\n' * seg.count('\n'))
            i = n if j < 0 else j + 2
        elif c in '"\'':
            j = i + 1
            while j < n and text[j] != c and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            out.append(c + c)
            i = j + 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def main():
    hits = 0
    for path in sorted((ROOT / 'src').rglob('*')):
        if path.suffix not in ('.cpp', '.h', '.hpp', '.inl'):
            continue
        rel = path.relative_to(ROOT).as_posix()
        if rel.startswith(ALLOWED) or rel in ALLOWED_FILES:
            continue
        text = strip_comments_and_strings(path.read_text(encoding='utf-8', errors='replace'))
        for lineno, line in enumerate(text.split('\n'), 1):
            for m in PATTERN.finditer(line):
                print(f'{rel}:{lineno}: {m.group(0)}')
                hits += 1
    if hits:
        print(f'\n{hits} usos de OpenGL fuera del backend.')
        return 1
    print('OK: sin OpenGL fuera de src/Render/Backend y src/Platform.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
