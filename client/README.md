# Mu Online 0.97k — reconstrucción del código fuente

Port a C++ del cliente **Mu Online 0.97k** (`main.exe`, MD5
`eb95ac0785e40a7ad60c9ddb5d8bef34`), reconstruido por ingeniería inversa a
partir del binario original.

El objetivo es que el cliente compilado **se comporte igual que el binario
original**: mismo flujo de login, mismo render, mismos paquetes. No es un
reescritura ni un "cliente inspirado en" — cada función es un port de su
contraparte en el binario, y las desviaciones deliberadas están documentadas
en un comentario junto al código.

El código está en español. Los símbolos que ya pudieron identificarse se
nombran por responsabilidad; su definición canónica conserva un comentario
`IDA: FUN_xxxxxxxx` o `IDA: DAT_xxxxxxxx` para mantener la trazabilidad con el
binario.

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
| Música (BGM) | el original lanza `MuPlayer.exe` |
| Combate | Parcial — `Attack` (0x49CBF0) está portada al ~4% |
| Movimiento de NPCs / monstruos | Parcial |

### Arquitectura y deuda técnica

El código portado está distribuido por dominio (`Render/`, `Terrain/`, `UI/`,
`Item/`, `Entity/`, `Combat/`, `Net/`, `Scene/`, etc.); ya no existe un depósito
general de `stubs_*.cpp` pendiente de repartir. El árbol actual contiene 247
archivos `.cpp` y 53 headers bajo `src/`.

`stubs_IDA_ports.cpp` es la excepción intencional: contiene 28.722 líneas de
decompilados crudos de IDA, cada uno protegido por `IDA_PORT_*`. Se preserva
como referencia canónica del decompile y como mecanismo de compatibilidad o
activación selectiva mientras se valida un port. **No refactorizar, renombrar
ni distribuir este archivo entre módulos.** Sus aliases o bridges ABI tampoco
son deuda de nomenclatura: existen para que los puertos selectivos mantengan
su contrato original.

Los `FUN_*` y `DAT_*` que todavía aparecen fuera de ese archivo no son, por sí
solos, deuda de renombrado. Algunos describen infraestructura, CRT, GameGuard,
layouts binarios, pools o compatibilidad; otros requieren investigación o un
port futuro antes de poder recibir un nombre semántico seguro.

---

## Qué necesitás además de este repo

Casi nada: los **assets del juego ya están incluidos** en `bin/Client/Data/`
(~209 MB — modelos `.bmd`, texturas `.ozj`/`.ozt`, mapas, sonidos y música),
junto con `MuPlayer.exe`, que es lo que el cliente lanza para reproducir el BGM.
Clonás, compilás y arranca.

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

1. Copiar `server.cfg.example` a `bin/Client/server.cfg` y editarlo (es el único
   archivo que no viene en el repo).
2. Ejecutar `bin/Client/main.exe`.

#### `server.cfg`

Dos tipos de línea: las direcciones (`<IP> <puerto>`) y las de identidad del
server (`clave=valor`). Las que empiezan con `#` o `;` son comentarios.

```
127.0.0.1 44405        ← ConnectServer (lista de servers + barra de carga)
127.0.0.1 55901        ← GameServer (fallback)

CustomerName=MuLinux
ServerSerial=TbYehR2hFUPBKgZj
ClientVersion=0.97.11
```

**Direcciones.** Con dos líneas se usa el flujo ConnectServer: el cliente pide
la lista real (`F4/02`), el server contesta con nombres y ocupación, y al elegir
uno el `F4/03` redirige al GameServer. Con **una sola línea** se conecta directo
al GameServer — el comportamiento clásico, y en ese caso el select-server muestra
una entrada estática de relleno.

> El select-server aparece **siempre**, incluso apuntando directo al GameServer:
> es una pantalla del flujo original, no un indicio de que estés llegando al
> ConnectServer.

**Identidad del server.** Los tres valores tienen que coincidir con los del
GameServer (`MuServer/GameServer/DATA/GameServerInfo - StartUp.dat`). Si alguno
no coincide, el cliente **conecta pero no entra**, y sin ningún mensaje útil:

| Clave | De dónde sale | Qué pasa si no coincide |
|---|---|---|
| `CustomerName` | `CustomerName=` del `.dat` | El cliente conecta, desencripta basura y se queda en *"conectando al GameServer"* para siempre |
| `ServerSerial` | `ServerSerial=` del `.dat` | Igual que arriba, **y además** el login devuelve *"versión incorrecta"* |
| `ClientVersion` | `ServerVersion=` del `.dat` | Login rechazado con *"versión incorrecta"* |

`CustomerName` y `ServerSerial` alimentan la clave de encriptación, que el
GameServer deriva de los dos combinados (`GameServer/HackCheck.cpp::InitHackCheck`);
por eso cambiar el nombre del cliente rompe la conexión aunque todo lo demás esté
bien. `ServerSerial` cumple doble función: entra en esa derivación y además el
server lo compara byte a byte en el login.

Si se omiten, se usan los valores por defecto de este fork (los de la tabla de
arriba). `ClientVersion` acepta tanto `0.97.11` como `09711`.

**Para diagnosticar**, `bin/Client/debug.log` registra la clave derivada al
arrancar:

```
MuEmu: InitKeys CustomerName='MuLinux' Serial='TbYehR2hFUPBKgZj' -> EncDecKey1=0xC2 EncDecKey2=0x01 (xor=0xC2 add=0xC2)
server.cfg: ClientVersion='09711'
```

Si el cliente se queda colgado conectando, esa línea es lo primero que hay que
mirar: comparala con el `CustomerName` del server.

---

## Estructura

```
mu97k-src/
├── mu97k.sln            solución de VS2022
├── mu97k.vcxproj        proyecto (Win32)
├── lib/libjpeg/         libjpeg 6b (decodifica las texturas .ozj)
└── src/
    ├── WinMain.cpp      punto de entrada + WndProc + loop de mensajes
    ├── globals.{h,cpp}  estado global; los DAT identificados conservan trazabilidad IDA
    ├── functions.h      declaraciones compartidas y procedencia IDA de símbolos renombrados
    ├── structs.h        layouts de struct verificados contra IDA
    ├── ghidra_compat.h  macros que el decompile de Ghidra da por existentes
    │                    (qmemcpy, LODWORD, SLOBYTE, ...)
    │
    ├── Combat/  Config/  Core/    Entity/  Game/     GameGuard/ Input/ Item/
    ├── Local/   Math/    Model/   Monster/ Net/       Party/     Path/  Physics/
    ├── Render/  Scene/   Sound/   Terrain/ Trade/     UI/        Util/
    │
    └── stubs_IDA_ports.cpp
                         decompilados crudos IDA detrás de IDA_PORT_*;
                         infraestructura preservada, no refactorizar
```

Los módulos agrupan por responsabilidad. La dirección en el binario sigue
siendo una pista importante para verificar una función o resolver un símbolo,
pero no determina la ubicación del código portado.

---

## Cómo trabajar en esto

### La regla principal: fiel al binario

El orden de autoridad para resolver cualquier duda:

1. **IDA / el binario original.** Es la verdad. Si el decompile dice algo raro,
   probablemente el decompile tenga razón y nuestra intuición no.
2. **El servidor MuEmu**, para todo lo que sea formato de paquetes.
3. **El DLL de inyección**, como segunda referencia de comportamiento.
4. **El source de Mu Online 5.2**, sólo como apoyo semántico y de nomenclatura
   cuando el contexto actual lo permite. No se copia implementación ni se
   incorpora comportamiento de 5.2: la UI, las definiciones y las features
   pueden diferir de 0.97k.

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
