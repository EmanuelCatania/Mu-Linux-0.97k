"""Verifica que el código del juego no use OpenGL directamente.

Sólo src/Render/Backend/** y src/Platform/** pueden llamar a gl*/wgl*/glu*,
usar constantes GL_* o tipos GL*. El resto pasa por Render/Gfx.h y
Render/BufferedDraw.h. Se ignoran comentarios y literales de texto.

En todo src/ (backend incluido) tampoco se admite el pipeline fijo de
OpenGL 1.1: el contexto es 3.3 core y glad.h se genera sin compatibility.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
ALLOWED = ('src/Render/Backend/', 'src/Platform/')
ALLOWED_FILES = set()

PATTERN = re.compile(
    r'\b(?:w?glu?|wgl)[A-Z][A-Za-z0-9]*\s*\('
    r'|\bGL_[A-Z_0-9]+\b'
    r'|\bGL(?:float|uint|int|enum|ubyte|sizei|boolean|bitfield|double|void)\b'
    r'|#\s*include\s*[<"]GL/')

# Entradas y constantes que no existen en OpenGL 3.3 core.
LEGACY = re.compile(
    r'\b(?:gl(?:Begin(?!\w)|End(?!\w)|Vertex[234]|Color[34]|TexCoord[1234]|MultiTexCoord|Normal3|MatrixMode|'
    r'PushMatrix|PopMatrix|LoadIdentity|LoadMatrix|MultMatrix|Ortho|Frustum|Rotate|Translate|Scale[fd]|'
    r'PushAttrib|PopAttrib|PushClientAttrib|PopClientAttrib|EnableClientState|DisableClientState|'
    r'VertexPointer|ColorPointer|TexCoordPointer|NormalPointer|NewList|EndList|CallLists?|GenLists|'
    r'DeleteLists|ListBase|Bitmap|RasterPos[234]|WindowPos[23]|AlphaFunc|TexEnv[fi]v?|Fog[fi]v?|'
    r'ShadeModel|Light[fi]v?|Material[fi]v?|PixelZoom|DrawPixels)\w*|wglUseFont\w*|glu[A-Z]\w*)\s*\('
    r'|\bGL_(?:CLAMP|LUMINANCE\w*|QUADS|QUAD_STRIP|POLYGON|ALPHA_TEST\w*|FOG\w*|LIGHTING|MODELVIEW\w*|'
    r'PROJECTION\w*|COMPILE|CURRENT_RASTER\w*|TEXTURE_ENV\w*|COMPATIBILITY_PROFILE\w*)\b')


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
        if rel == 'src/Platform/GL/glad.h':
            continue
        allowed = rel.startswith(ALLOWED) or rel in ALLOWED_FILES
        text = strip_comments_and_strings(path.read_text(encoding='utf-8', errors='replace'))
        for lineno, line in enumerate(text.split('\n'), 1):
            for m in LEGACY.finditer(line):
                print(f'{rel}:{lineno}: {m.group(0)} (OpenGL 1.1)')
                hits += 1
            if allowed:
                continue
            for m in PATTERN.finditer(line):
                print(f'{rel}:{lineno}: {m.group(0)}')
                hits += 1
    if hits:
        print(f'\n{hits} usos de OpenGL fuera del backend o del pipeline fijo.')
        return 1
    print('OK: sin OpenGL fuera de src/Render/Backend y src/Platform, ni pipeline fijo.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
