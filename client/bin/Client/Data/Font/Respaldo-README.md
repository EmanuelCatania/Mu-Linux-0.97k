# Fuentes de respaldo

El cliente dibuja con LiberationSans (o la fuente de `[Font]`). Lo que esa
fuente no tiene se busca en estos archivos, en orden alfabético y por
carácter, y después en las fuentes del sistema. Se pueden agregar más
fuentes `.ttf`/`.otf` a esta carpeta: el nombre antes del primer `-` es la
familia y el resto el estilo (`Regular`, `Bold`).

| Archivo | Cubre | Origen | Licencia | SHA256 |
|---|---|---|---|---|
| DroidSansFallbackFull.ttf | chino y japonés | AOSP `frameworks/base` (lollipop-release) `data/fonts` | Apache 2.0, `DroidSansFallback-LICENSE.txt`. Copyright (c) 2005-2008 The Android Open Source Project | acb6440a713d880a13a21b468ba7cd43f5a2b2934972e51be791c880730777b8 |
| NanumGothic-Regular.ttf | coreano (hangul) | Google Fonts `ofl/nanumgothic` | OFL 1.1, `NanumGothic-LICENSE.txt`. Copyright (c) 2010 NHN Corporation | 76f45ef4a6bcff344c837c95a7dcc26e017e38b5846d5ae0cdcb5b86be2e2d31 |
| NotoEmoji-Regular.ttf | emojis (blanco y negro) | Google Fonts `ofl/notoemoji/NotoEmoji[wght].ttf` | OFL 1.1, `Noto-LICENSE.txt`. Copyright 2013 Google LLC | de6c18832938afc99caf132b39d6a30a19bac7f2e812e28db2535b4608d27551 |
| NotoSansArabic-Regular.ttf | árabe | Google Fonts `ofl/notosansarabic` | OFL 1.1. Copyright 2022 The Noto Project Authors | bdff3e5659d67e67def05b33f749683b9376ae819d65d3dd62ac4640b3aaef48 |
| NotoSansDevanagari-Regular.ttf | devanagari (hindi) | Google Fonts `ofl/notosansdevanagari` | OFL 1.1. Copyright 2022 The Noto Project Authors | 4e3c66638958c3e2ab5d37f47a8deb89fffeb7be9985c665a519bbc7ba762313 |
| NotoSansHebrew-Regular.ttf | hebreo | Google Fonts `ofl/notosanshebrew` | OFL 1.1. Copyright 2022 The Noto Project Authors | cdefaf8efd47045f6820928eba84db5bed7557539328952b5f828315485e02ee |
| NotoSansThai-Regular.ttf | tailandés | Google Fonts `ofl/notosansthai` | OFL 1.1. Copyright 2022 The Noto Project Authors | 61cf814eec46b294d6ea4401ac295d0cecd5207bd2331dcc5a15e7301d30ee44 |

`Noto-LICENSE.txt` es el texto de la SIL Open Font License 1.1, que cubre
todas las fuentes Noto de esta carpeta.

Los emojis salen en blanco y negro porque el texto se rasteriza a 1 bit,
como lo hacía GDI; los emojis a color quedan para cuando el atlas de texto
tenga color.
