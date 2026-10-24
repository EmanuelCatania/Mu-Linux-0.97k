# utf8proc: normalización, case folding y grapheme clusters

Fuentes vendorizados sin modificaciones desde https://github.com/JuliaStrings/utf8proc
Versión fija: `v2.12.0` (tarball SHA256 `f564011d38b2888d583d510b08e69ffa15aa117155db1b9b49ef1dfe1fa25111`).

| Archivo | SHA256 |
|---|---|
| utf8proc.c | f38438eb986ba7eedad0f2e820a70fae1aaf05197364ba0381a5e84775099ef8 |
| utf8proc.h | 08da85e820efaa3613500946c26b2dc427a07f76001ed7c16bc90f298b814c35 |
| utf8proc_data.c | b4911d4162d216afa612317a9b981851e15d672f4d93b25f3a1eb986f8c0aedb |

Licencia MIT (más la licencia de los datos de Unicode), texto completo en
`LICENSE.md`. No hay descarga durante el build.

Se compila sólo `utf8proc.c` (que incluye `utf8proc_data.c`), sin PCH del juego y
con `UTF8PROC_STATIC`. Lo usa únicamente `src/Text/TextCore.cpp`.
