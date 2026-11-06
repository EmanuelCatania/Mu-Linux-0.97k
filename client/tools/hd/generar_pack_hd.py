#!/usr/bin/env python3
"""Genera el pack opcional de texturas HD (Data/HD) a partir de los OZJ/OZT del cliente.

Cada textura se decodifica igual que en el cliente (OZJ: JPEG tras 24 bytes;
OZT: TGA de 32 bits tras 4 bytes, con las filas en el mismo orden), se amplía
con un reescalador y se guarda como Data/HD/<misma ruta>.ozj.png (u .ozt.png). El cliente la
usa con la opción "Texturas HD" si el tamaño es un múltiplo entero del original.

Reescaladores:
  --reescalador realesrgan  Real-ESRGAN (realesrgan-ncnn-vulkan o el upscayl-bin de
                            Upscayl, mismos parámetros), por IA. Se descarga aparte:
                            --exe es el ejecutable y --modelos su carpeta de modelos.
  --reescalador lanczos     Ampliación clásica con Pillow; sirve para probar el
                            pack sin descargar nada (no agrega detalle).

Ejemplo:
  python tools/hd/generar_pack_hd.py --data bin/Client/Data --reescalador realesrgan \\
      --exe "C:/Program Files/Upscayl/resources/bin/upscayl-bin.exe" --escala 2 \\
      --modelos "C:/Program Files/Upscayl/resources/models" --modelo ultrasharp-4x
"""
import argparse
import io
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit('Falta Pillow: pip install Pillow')

# Carpetas de lo 3D. La interfaz (Interface, Local, Logo) se deja afuera: se
# dibuja a escala fija y no gana con el reescalado.
CARPETAS = ('World', 'Object', 'Monster', 'NPC', 'Player', 'Item', 'Skill', 'Effect')


def decodificar(ruta: Path) -> Image.Image:
    datos = bytearray(ruta.read_bytes())
    if ruta.suffix.lower() == '.ozj':
        return Image.open(io.BytesIO(bytes(datos[24:]))).convert('RGB')
    tga = datos[4:]
    tga[17] &= ~0x30   # como el cliente: ignora los bits de origen
    return Image.open(io.BytesIO(bytes(tga))).convert('RGBA')


def negra(imagen: Image.Image) -> bool:
    return imagen.convert('RGB').getextrema() == ((0, 0), (0, 0), (0, 0))


def texturas(data: Path, carpetas):
    for raiz in sorted(data.iterdir()):
        if not raiz.is_dir() or raiz.name.upper() == 'HD':
            continue
        if not any(raiz.name.upper().startswith(c.upper()) for c in carpetas):
            continue
        for ruta in sorted(raiz.rglob('*')):
            if ruta.suffix.lower() in ('.ozj', '.ozt'):
                yield ruta


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--data', type=Path, required=True, help='carpeta Data del cliente')
    p.add_argument('--reescalador', choices=('realesrgan', 'lanczos'), default='lanczos')
    p.add_argument('--exe', type=Path, help='realesrgan-ncnn-vulkan(.exe)')
    p.add_argument('--modelo', default='realesrgan-x4plus', help='modelo (p. ej. ultrasharp-4x, remacri-4x de Upscayl)')
    p.add_argument('--modelos', type=Path, help='carpeta de modelos (.param/.bin), si no es la "models" junto al ejecutable')
    p.add_argument('--escala', type=int, choices=(2, 3, 4), default=2, help='factor final (2 = el doble)')
    p.add_argument('--carpetas', nargs='*', default=list(CARPETAS), help='prefijos de carpetas a procesar')
    p.add_argument('--forzar', action='store_true', help='rehacer las que ya existen')
    a = p.parse_args()
    if a.reescalador == 'realesrgan' and (not a.exe or not a.exe.is_file()):
        p.error('--exe tiene que apuntar a realesrgan-ncnn-vulkan')

    salida = a.data / 'HD'
    pendientes = []
    for ruta in texturas(a.data, a.carpetas):
        rel = ruta.relative_to(a.data)
        destino = salida / rel.with_name(rel.stem + rel.suffix.lower() + '.png')   # casa.ozj.png
        if destino.exists() and not a.forzar:
            continue
        try:
            pendientes.append((ruta, destino, decodificar(ruta)))
        except Exception as error:   # textura dañada o en un formato que el cliente tampoco lee
            print(f'  salteada {ruta}: {error}')
    print(f'{len(pendientes)} texturas para ampliar')
    if not pendientes:
        return

    with tempfile.TemporaryDirectory(prefix='mu-hd-') as temporal:
        entrada, ampliadas = Path(temporal) / 'in', Path(temporal) / 'out'
        entrada.mkdir(); ampliadas.mkdir()
        # Nombres planos: el reescalador procesa una carpeta sin subcarpetas.
        for i, (_, _, imagen) in enumerate(pendientes):
            imagen.save(entrada / f'{i:05d}.png')
        def reescalar(origen: Path, destino: Path):
            comando = [str(a.exe), '-i', str(origen), '-o', str(destino), '-n', a.modelo, '-s', '4', '-f', 'png']
            if a.modelos:
                comando += ['-m', str(a.modelos)]
            subprocess.run(comando, check=True, cwd=a.exe.parent, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        if a.reescalador == 'realesrgan':
            print('reescalando con', a.modelo)
            reescalar(entrada, ampliadas)
        for i, (ruta, destino, imagen) in enumerate(pendientes):
            tamano = (imagen.width * a.escala, imagen.height * a.escala)
            ampliada = None
            if a.reescalador == 'realesrgan':
                archivo = ampliadas / f'{i:05d}.png'
                # Por carpeta, ncnn devuelve algunas imágenes en negro de forma
                # intermitente; de a una salen bien. Se reintentan así.
                for intento in range(4):
                    if archivo.exists():
                        ampliada = Image.open(archivo).convert(imagen.mode)
                        if negra(ampliada) == negra(imagen):
                            break
                        ampliada = None
                    reescalar(entrada / f'{i:05d}.png', archivo)
                if ampliada is None:
                    print(f'  {ruta}: el reescalador falló, se usa Lanczos')
                elif ampliada.size != tamano:
                    ampliada = ampliada.resize(tamano, Image.LANCZOS)
            if ampliada is None:
                ampliada = imagen.resize(tamano, Image.LANCZOS)
            destino.parent.mkdir(parents=True, exist_ok=True)
            ampliada.save(destino)
    print(f'pack en {salida}')


if __name__ == '__main__':
    main()
