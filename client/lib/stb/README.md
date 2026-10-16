# stb: imagen y captura JPEG

Fuentes vendorizados sin modificaciones desde https://github.com/nothings/stb
Revisión fija: `2c980bb59875b0d32144a71867fbdebb2f77cd20`.

| Archivo | Versión | SHA256 |
|---|---|---|
| stb_image.h | 2.30 | 594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3 |
| stb_image_write.h | 1.16 | cbd5f0ad7a9cf4468affb36354a1d2338034f2c12473cf1a8e32053cb6914a05 |

Licencia dual MIT / dominio público; se adopta la opción MIT, cuyo texto completo y copyright se conservan al final de cada header. No se retiran avisos. No hay descarga durante el build.

La implementación se compila una sola vez en src/Render/Texture/ImageCodec.cpp,
sin PCH del juego. El decoder habilita sólo JPEG/TGA. El adaptador conserva prefijos,
rechazo de TGA de 24 bits, orden de filas y límite de textura. No activa un flip global.
stb_image_write conserva las capturas JPEG (RGB del framebuffer invertido).

JPEG no es idéntico a IJG 6b: comparación de todos los assets del cliente, máximo
3 por canal, aprobado por Emanuel el 2026-10-16. No se aplica umbral de negro.
El informe y banco reproducible están en doc-port/stb-comparison, fuera del repo.
