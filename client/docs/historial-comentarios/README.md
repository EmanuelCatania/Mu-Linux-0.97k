# Historial de comentarios

Esta carpeta guarda los comentarios de desarrollo que se sacaron del código en
la Fase 1. El código fuente completo, con todos los comentarios originales,
sigue en el tag `0.97.00`; acá queda el texto movido agrupado por carpeta de
`src/`, para poder consultarlo sin hacer checkout del tag.

## Criterio

En el código sólo queda lo que explica el código **actual**.

**Se conserva:**

- la referencia a IDA: `// IDA: <nombre> (<dirección>)`, `IDA: FUN_xxx`,
  `@ 0x00xxxxxx` y equivalentes;
- una línea de referencia cuando aporta ("usa X como …", el layout de un
  struct o de un paquete, offsets de campos);
- las **desviaciones vigentes** respecto del binario, resumidas a 1-2 líneas;
- las advertencias que evitan romper algo ("no mover esto antes de X
  porque …", "no declarar una variable propia acá").

**Se mueve a esta carpeta:**

- la narrativa de investigación: fechas, "esto trajo problemas", "se cazó con
  la sonda X", "el port viejo hacía …", intentos descartados, logs pegados,
  explicaciones largas de cómo se llegó al fix;
- referencias a archivos o líneas que ya no existen (`stubs.cpp:275`,
  "movida desde stubs_IDA_ports.cpp", "~line 13553").

Cuando un comentario mezclaba las dos cosas, en el código queda la versión
resumida y acá el bloque original completo. Los comentarios en inglés que se
conservan no se tradujeron (para no tocar líneas sin necesidad); los que se
reescribieron quedan en español.

## Formato

Un archivo por carpeta de `src/` (`Core.md`, `Config.md`, …), agrupado por
archivo fuente. Cada entrada indica la función o el bloque de origen y la
línea donde estaba en el tag `0.97.00`, y copia el texto **tal cual**.

## Verificación

Cada PR de limpieza se verifica con `tools/strip_comments_diff.py`, que quita
los comentarios de las dos versiones de cada archivo, normaliza whitespace y
compara: tiene que dar 0 diferencias de código.

```
python tools/strip_comments_diff.py --base fase/1 --head HEAD
```
