# libunibreak: cortes de línea (UAX #14)

Fuentes vendorizados sin modificaciones desde https://github.com/adah1972/libunibreak
Versión fija: `8.0` (tarball `libunibreak-8.0.tar.gz`, SHA256
`9c4fad6e517338a098373acc9f35579ae2c325e6446666fb9ac2666ba15ceba4`).

Sólo están los archivos del corte de línea; los de palabras y grapheme
clusters no hacen falta (los clusters salen de utf8proc).

| Archivo | SHA256 |
|---|---|
| eastasianwidthdata.c | cfdc4c2abb651092cfa92fa8004a8dc7ae1e82841912720cb2b627b02534fab7 |
| eastasianwidthdef.c | f609d5915e4cf7572765e66aaa8bda6d315cc8ec89e01085c92683a2b1edd42d |
| emojidata.c | b33ed203eb6bacfc7c9598a726a41c64ab67032b767bc676aecc430a207faeb6 |
| emojidef.c | 4baf8f003201d49cbf78d58c0938eb28e19121095314724e8dedbb610bae327f |
| linebreak.c | 35fa884d87b37105cc541b4cfd085935f0262b8c472532edfb659fe37b9cc51e |
| linebreakauxdata.c | 09b9327653433d22cc1c3ff4cbdd176abe577286365275cc70617a79adea29db |
| linebreakdata.c | d2a1008ad93a15c15389e5cdd3c2d21b5e03820ed9daeee5a5a902ef56649719 |
| linebreakdef.c | 105b926c59ed3b1a87e2c7a19ba68adaec69032b501d0c61d9a7043f534bd62b |
| unibreakbase.c | aa44089bcde4bf6cedb387af7bed3ba58d02d3a2c1202b306812a0be74bc0834 |
| unibreakdef.c | 9d0f2a973ee07e748091ec18e09131ab7109899ae15b17fa769c93794a7fd58e |
| eastasianwidthdef.h | 181068f45ca8a6ada6acf3567caee7d86ce12a30c077949b1e7dc3738b4d2707 |
| emojidef.h | d14675c270334b046288f647a060186066b4d6d1905a457459d58388879efabe |
| linebreak.h | 394d9879d87d6a9930a42a9256c0b732417bbac30d051138b1a0da7c18347538 |
| linebreakdef.h | 5b5c1470331752a11b340eea2c0f48b7f04bd908c20864d475f05aec498a9906 |
| unibreakbase.h | 3880a7c664fe6eec033950935ec99c297d9c44fe154fbd6972b4d8a55fd20976 |
| unibreakdef.h | 2cffd22ab7ed6a69f819905e30b2214891518c2b601d566e75186a0047095b40 |

Licencia zlib, texto completo en `LICENCE`. No hay descarga durante el build.

Se compila como una sola unidad, `src/Text/UniBreak.c`, sin PCH del juego.
Lo usa únicamente `src/Text/TextCore.cpp` (`TextEngine::LineBreaks`).
