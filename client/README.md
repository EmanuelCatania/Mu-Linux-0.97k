# Mu Online 0.97k — reconstrucción del código fuente

Port a C++ del cliente **Mu Online 0.97k** (`main.exe`, MD5
`eb95ac0785e40a7ad60c9ddb5d8bef34`), reconstruido por ingeniería inversa a
partir del binario original.

El objetivo es que el cliente compilado **se comporte igual que el binario
original**: mismo flujo de login, mismo render, mismos paquetes. No es un
reescritura ni un "cliente inspirado en" — cada función es un port de su
contraparte en el binario, y las desviaciones deliberadas están documentadas
en un comentario junto al código.

El código está en español. Los nombres de funciones, campos y direcciones se
mantienen como en el binario / IDA.

---

## Estado

**Funciona end-to-end**: arranca, conecta, hace login, elige personaje, entra
al mundo y se juega. Terreno, personajes, inventario, equipo, chat, party,
guild, tienda, baúl, combate básico y efectos están operativos.

No es un cliente terminado. Quedan funciones portadas a medias (las tres
grandes: `Attack`, `Action`, `MoveCharacterVisual`), subsistemas con huecos
conocidos y bugs de port apareciendo a medida que se ejercitan caminos nuevos.

### Por subsistema

| Subsistema | Estado |
|---|---|
| Arranque, ventana, OpenGL | Completo |
| Login + select-server (incluye flujo ConnectServer) | Completo, verificado contra server real |
| Select-character | Funcional |
| Red / protocolo (85+ opcodes) | Completo en lo ejercitado; opcodes nuevos aparecen al usar features nuevas |
| Terreno, iluminación, agua | Completo |
| Render de personajes y equipo | Funcional |
| Efectos, joints, partículas, clima | Funcional |
| Inventario, equipo, baúl, tienda, trade | Funcional |
| Chat, party, guild | Funcional |
| Sonido (DirectSound) | Funcional |
| Música (BGM) | **No suena** — el original lanza `MuPlayer.exe`, que no se distribuye |
| Combate | Parcial — `Attack` (0x49CBF0) está portada al ~4% |
| Movimiento de NPCs / monstruos | Parcial |

### Deuda técnica principal

Los archivos `stubs_*.cpp` (~55.000 líneas, ~39% del proyecto) son un depósito
histórico: funciones portadas que nunca se movieron a su módulo de dominio.
Hay un refactor en curso para repartirlas.

`stubs_IDA_ports.cpp` es distinto: es el almacén de volcados crudos de IDA,
cada uno detrás de una macro `IDA_PORT_*`. **No mover ni reorganizar ese
archivo** — se rompe el mecanismo de activación.

---

## Qué necesitás además de este repo

Casi nada: los **assets del juego ya están incluidos** en `bin/Client/Data/`
(~209 MB — modelos `.bmd`, texturas `.ozj`/`.ozt`, mapas, sonidos y música),
junto con `MuPlayer.exe`, que es lo que el cliente lanza para reproducir el BGM.
Clonás, compilás y arranca.

> ⚠️ **Este repositorio es privado y no puede hacerse público como está.**
> Los assets son copyright de WebZen. Si alguna vez se abre, hay que sacarlos
> primero — y como quedan en la historia de git, eso implica reescribirla, no
> basta con borrarlos en un commit nuevo.

Lo único que **no** está es el **`main.exe` original**, que sólo hace falta si
querés decompilarlo vos mismo para verificar un port contra el binario. Sale de
cualquier distribución del cliente 0.97k; verificá el MD5
(`eb95ac0785e40a7ad60c9ddb5d8bef34`) antes de comparar direcciones, porque hay
muchas variantes parcheadas dando vueltas y no coinciden.

También necesitás un **servidor**. El port está validado contra
[MuEmu - Linux](https://github.com/EmanuelCatania/Mu-Linux-0.97k) (season 0.97k), que es la fuente autoritativa
para el formato de los paquetes. Tambien se puede utilizar la version de windows [MuEmu - Kayito](https://github.com/nicomuratona/MuEmu-0.97k-kayito) (season 0.97k)

---

## Compilar

Requiere **Visual Studio 2022** con el toolset de C++ para escritorio.

1. Abrir `mu97k.sln`.
2. Seleccionar configuración **Debug** / plataforma **Win32**.
3. Compilar.

**La plataforma tiene que ser Win32 (x86).** Todo el port asume punteros de 32
bits: las direcciones del binario original, los layouts de struct y los pools
de memoria. En x64 no compila, y si compilara no serviría.

Salida: `bin/Client/main.exe`. El proyecto enlaza directo sobre `bin/Client/`,
que es donde viven los assets y `server.cfg`, así que no hay copia intermedia ni
riesgo de terminar ejecutando un binario viejo.

Librerías enlazadas (todas del SDK de Windows, salvo libjpeg que va incluida):
`opengl32.lib`, `glu32.lib`, `winmm.lib`, `ws2_32.lib`.

### Ejecutar

1. Copiar `server.cfg.example` a `bin/Client/server.cfg` y editar la dirección
   del server (es el único archivo que no viene en el repo).
2. Ejecutar `bin/Client/main.exe`.

`server.cfg` acepta una o dos líneas `<IP> <puerto>`:

```
127.0.0.1 44405    ← ConnectServer (lista de servers + barra de carga)
127.0.0.1 55901    ← GameServer (fallback)
```

Con una sola línea se conecta directo al GameServer, que es el comportamiento
clásico.

---

## Estructura

```
mu97k-src/
├── mu97k.sln            solución de VS2022
├── mu97k.vcxproj        proyecto (Win32)
├── lib/libjpeg/         libjpeg 6b (decodifica las texturas .ozj)
└── src/
    ├── WinMain.cpp      punto de entrada + WndProc + loop de mensajes
    ├── globals.{h,cpp}  todos los DAT_ globales del binario original
    ├── functions.h      declaraciones de las funciones portadas (FUN_xxxxxxxx)
    ├── structs.h        layouts de struct verificados contra IDA
    ├── ghidra_compat.h  macros que el decompile de Ghidra da por existentes
    │                    (qmemcpy, LODWORD, SLOBYTE, ...)
    │
    ├── Combat/  Config/  Entity/  Game/  GameGuard/  Input/  Item/
    ├── Local/   Math/    Model/   Monster/  Net/     Party/
    ├── Render/  Scene/   Sound/   Terrain/  Trade/   UI/  Util/
    │
    └── stubs_*.cpp      depósito histórico en proceso de refactor
                         (stubs_IDA_ports.cpp: volcados crudos, no tocar)
```

Los módulos agrupan por **dirección en el binario**, no por nombre: en el
binario las funciones vecinas pertenecían a la misma unidad de compilación, así
que la vecindad de direcciones es la mejor pista sobre a qué módulo pertenece
una función.

---

## Cómo trabajar en esto

### La regla principal: fiel al binario

El orden de autoridad para resolver cualquier duda:

1. **IDA / el binario original.** Es la verdad. Si el decompile dice algo raro,
   probablemente el decompile tenga razón y nuestra intuición no.
2. **El servidor MuEmu**, para todo lo que sea formato de paquetes.
3. **El DLL de inyección**, como segunda referencia de comportamiento.
4. **El source de Mu Online 5.2**, sólo como ayuda para identificar nombres y
   tipos de retorno. Las structs core son estables entre versiones, pero las
   features posteriores no existen en 0.97k.

Lo que no está en ninguna de esas fuentes no se inventa. Si hace falta una
desviación (porque un camino del original es inalcanzable, o depende de algo
que todavía no está portado), se implementa **y se documenta en un comentario
ahí mismo**, explicando qué hace el original y por qué nos apartamos.

Lo único que se saltea deliberadamente es el ruido anti-tamper: las
operaciones de hash-table intercaladas, los bloques inalcanzables y el
scrambling XOR de la versión protegida. No son lógica de juego.

### Trampas conocidas

Estas costaron sesiones enteras de depuración. Todas volvieron a aparecer
más de una vez.

**1. Símbolos duplicados.** El mismo nombre definido dos veces: un stub viejo y
el port real. C++ puede aceptarlo como sobrecarga si las firmas difieren, y
entonces cada llamador resuelve a una copia distinta. Síntoma típico: un valor
se corrompe y no aparece ningún escritor que lo explique. Antes de auditar
cualquier función, confirmá que estás leyendo **la copia que se compila** —
no una dentro de `#if 0` ni una detrás de una macro `IDA_PORT_*` sin definir.
Corolario: una sonda de diagnóstico puesta en código muerto da cero resultados,
y ese silencio parece evidencia de que no hay bug.

**2. Locales que Ghidra separó.** El decompile emite como variables sueltas lo
que en el frame original era un bloque contiguo, y el código las recorre como
si lo fueran (`&local_XX` de un escalar pasado como `vec3`). El compilador no
garantiza ese layout. Síntoma: la primera componente sale bien y el resto es
basura (valores de ~1e9). Se arregla reconstruyendo el frame como un array
contiguo y mapeando los nombres por offset.

**3. Campos enteros leídos como float.** Ghidra tipa el slot como `float*` y
entonces *todo* acceso sale como float, incluidos los campos que son enteros o
punteros. `(float)(uintptr_t)ptr` convierte numéricamente lo que había que
reinterpretar por bits. Síntoma: no es un crash, es funcionalidad que
simplemente no ocurre — comparaciones que nunca dan verdadero, punteros en
cero, contadores clavados. Pista: valores de ~1e9 que leídos como bits dan
floats chicos y razonables. Dentro de un mismo archivo suelen convivir accesos
correctos y rotos; esa mezcla es la señal.

**4. Padding de structs en los paquetes.** El servidor manda structs de C con
su padding de alineación. Leer los campos por offset "lógico" en vez del real
devuelve basura convincente. Ya mordió en los stats, en la lista de guild y en
los números de daño.

**5. Las etiquetas de los offsets mienten.** Varios campos de la struct de
entidad estuvieron mal etiquetados durante meses (`+0x1BC` no son flags de
movimiento: es la clase del personaje; `+0x34E` no es "muerto": es SafeZone).
Antes de confiar en el nombre de un offset, buscá quién lo **escribe** en el
binario.

**6. Nombres de funciones parecidos con efectos opuestos.** El caso recurrente
es la familia de estado de OpenGL: `EnableAlphaTest` (0x511680),
`EnableAlphaBlend` (0x511710, aditivo) y `DisableTexture` (0x511590, que apaga
el texturizado). Confundirlas pinta cuadrados blancos sobre medio frame,
porque el estado de GL queda pegado y contamina todo lo que se dibuje después.

---

## Licencia

MIT — ver [LICENSE](LICENSE).

La licencia cubre **el código**: todo lo que está bajo `src/`.

**No** cubre los assets de `bin/Client/Data/` ni `MuPlayer.exe`, que son
copyright de WebZen Inc. y están en el repo sólo porque es privado y de uso
interno entre colaboradores.

`lib/libjpeg/jpeg-6b` es del Independent JPEG Group, bajo su propia licencia
permisiva (`lib/libjpeg/jpeg-6b/README`, sección "LEGAL ISSUES").
