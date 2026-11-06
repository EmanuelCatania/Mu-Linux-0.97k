# Pack de texturas HD

Pack opcional que el cliente usa con **Opciones → Pantalla → Avanzado →
Gráficos → Texturas HD** (se aplica al reiniciar). No reemplaza ningún archivo:
cada textura `Data/X/nombre.OZJ` (u `.OZT`) se toma de `Data/HD/X/nombre.ozj.png`
si existe y mide un múltiplo entero del original; si no, se usa la original.

## Generarlo

Hace falta Python 3 con Pillow (`pip install Pillow`) y un reescalador
compatible con Real-ESRGAN ncnn-vulkan, por ejemplo el `upscayl-bin` de
[Upscayl](https://github.com/upscayl/upscayl), que ya trae modelos:

```
python tools/hd/generar_pack_hd.py --data bin/Client/Data --reescalador realesrgan \
    --exe "C:/Program Files/Upscayl/resources/bin/upscayl-bin.exe" \
    --modelos "C:/Program Files/Upscayl/resources/models" \
    --modelo ultrasharp-4x --escala 2
```

- `--escala 2` deja las texturas al doble (4 = cuádruple, 16 veces la memoria).
- `--modelo`: `ultrasharp-4x` da más detalle; `remacri-4x` es más natural.
- `--carpetas World1 Object1` limita a algunas carpetas (por defecto, todo lo 3D;
  la interfaz no se procesa).
- `--reescalador lanczos` amplía sin IA, sólo para probar el pack.
- Las que ya existen se saltean; `--forzar` las rehace.

La herramienta reescala de a una imagen: procesando una carpeta entera, el
reescalador a veces devuelve negro o cruza resultados entre texturas. Cada
resultado se reduce y se compara con su original; si no coincide se reintenta
y, si sigue fallando, se usa Lanczos.
