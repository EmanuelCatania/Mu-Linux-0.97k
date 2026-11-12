# Mu Linux 97K

[![Sitio web](https://img.shields.io/badge/sitio%20web-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/es/) [![XERJ Corpus Hub contributor](https://hub.xerj.org/contributors/EmanuelCatania/badge.svg)](https://hub.xerj.org/contributors/EmanuelCatania)

[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k?label=server&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-web?label=web&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-web) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mu-editor?label=editor&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mu-editor) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mysql?label=mysql&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mysql) 

🇦🇷 Español | [🇺🇸 English](README.en.md) | [🇧🇷 Português](README.pt-BR.md)

Servidor y cliente de Mu Online 0.97k en un solo repositorio. El servidor (Mu-Linux, basado en MuEmu 0.97k) corre de forma nativa en Linux, con Docker, panel web y editor opcional; el cliente está reconstruido desde el binario original y corre en Windows y Linux.

| Carpeta | Qué tiene |
|---|---|
| `server/` | Servidor: fuentes (`Source/`), datos (`server/MuServer/`), CMake y Docker |
| `client/` | Cliente: ver su [README](client/README.md) |
| `web/`, `editor/` | Panel web y editor de datos |
| `pterodactyl/`, `Pterodactyl eggs/` | Runtime y eggs de Pterodactyl |
| `docs/` | Guías de migración |

Cliente y servidor comparten la numeración y salen juntos en cada Release. Hasta 0.97.40 el cliente estaba en [Mu-97k-Client-Source](https://github.com/EmanuelCatania/Mu-97k-Client-Source), donde siguen sus Releases anteriores.

## Novedades de 0.97.40 — cierre de la Fase 4

- **Texto en UTF-8 de punta a punta.** El server deja de convertir a
  Windows-1252: lo que el jugador escribe es lo que se guarda y lo que ven los
  demás. Nombres de personaje y guild en cualquier escritura (10 y 8
  caracteres, validados y normalizados a NFC), y chat, susurros, avisos de guild
  y `/post` de hasta 200 bytes. La base pasa a `utf8mb4`.
- **Protocolo 0.97.40.** Nombres de personaje de 40 bytes y de guild de 32, y
  chat de 200. Los paquetes que con esos largos pasan de 255 bytes (lista de
  personajes, party, ranking de Devil Square) van con header C2.
- **Textos traducidos que manda el server.** Al entrar, cada cliente recibe en
  su idioma los nombres de items, monstruos y skills, las líneas de tooltip de
  los items custom y los textos propios del cliente. Ver
  [Textos e idiomas](#textos-e-idiomas).
- **Panel web:** editor de traducciones (`Data/Text`) y editor de items custom
  (efectos, pose, tooltip y pets de `Data/Custom/Items` y `Data/Custom/Pets`),
  los dos con la misma validación que el server; cambio de contraseña, Turnstile
  y orden de cuentas en el panel de administración.
- **Barra de buffs del cliente:** el server le manda a cada jugador el tiempo
  que le queda a cada efecto, sus valores y quién lo lanzó (F3:F1).
- **Niebla por mapa:** `MapFog.txt` acepta, además de los colores, la distancia
  de inicio y fin de la niebla de cada mapa (F3:E9).
- **MySQL con la API en C.** JoinServer y DataServer usan `mysql.h`: en Linux,
  `libmysqlclient`; en Windows, MariaDB Connector/C compilado estático. Ya no
  queda ninguna DLL de MySQL junto a los servers.
- Correcciones: el puntaje del ranking de Blood Castle llegaba corrido dos
  bytes; el paquete de recall (C3:B0) tenía el layout equivocado con GCC; Rena y
  Stone se apilan (`ItemStack.txt`).

**Compatibilidad: server 0.97.40 ↔ [cliente 0.97.40](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.40).**
Actualizá los dos juntos: el cliente 0.97.30 lee los paquetes con los largos
viejos y no entra a un server 0.97.40, y al revés.

### Cómo actualizar desde 0.97.30

Esta versión **cambia la base de datos**. Hacé un backup antes de empezar.

1. **Base de datos.** Seguí la [guía de migración a utf8mb4](docs/utf8mb4-migracion.md):
   backup, detener `mu-server` y `mu-web`, y correr `Migrate-utf8mb4.sql`,
   `Migrate-0.97.40.sql` y `Procedures.sql`. Si tu base ya estaba en `utf8mb4`,
   alcanza con los dos últimos. El script se detiene sin tocar nada si dos
   nombres pasarían a ser iguales con la collation nueva.
2. **Imágenes.** Con el compose de imágenes publicadas:

   ```bash
   docker compose -f docker-compose.images.yml pull mu-server mu-web mu-editor
   docker compose -f docker-compose.images.yml up -d
   ```

   Para fijar la versión, usá las imágenes con tag `0.97.40`.
3. **Volúmenes.** Un volumen persistente no se actualiza con el pull. Integrá
   en el tuyo, desde los archivos de esta versión:
   - `GameServer/Data/GameServerInfo - StartUp.dat`: cambiá `ServerVersion` a
     `0.97.40`. **Sin esto, el cliente nuevo no entra** (*"versión incorrecta"*).
   - `Data/Text/`: la carpeta completa (`Eng`, `Spn`, `Por`). Sin ella todo
     funciona, pero sin traducciones.
   - `Data/Message_Spn.txt` y `Data/Message_Por.txt`: ahora están en UTF-8. Si
     los modificaste, convertí tu copia de Windows-1252 a UTF-8.
   - `Data/Item/ItemStack.txt`: la fila `14,021` (Rena y Stone).
   - `Data/Custom/Encoder/MapFog.txt`: opcional, el bloque nuevo de
     distancias. Sin él, cada mapa usa 2000 a 2700 como antes.

**Pterodactyl:** seleccioná `0.97.40` en `MU_RELEASE_TAG` y actualizá también la
imagen de runtime, que ahora instala `libmysqlclient21`. Con el servidor
detenido, extraé el archivo de tu arquitectura en una carpeta temporal,
reemplazá los cuatro binarios y `start.sh`, e integrá los archivos de Data de
arriba conservando tus INI. La migración de la base es la misma.

### Textos e idiomas

El cliente le dice al server su idioma al conectarse (`[Language]
LangSelection` del cliente: `Eng`, `Spn` o `Por`), y el server le manda las
traducciones de ese idioma junto con el catálogo de contenido. Están en
`server/MuServer/Data/Text/<idioma>/`:

| Archivo | Qué traduce | Índice |
|---|---|---|
| `Item.txt` | nombres de items | sección e índice de `Item/Item.txt` |
| `Monster.txt` | nombres de monstruos y NPCs | índice de `Monster/Monster.txt` |
| `Skill.txt` | nombres de skills | índice de `Skill/Skill.txt` |
| `Tooltip.txt` | líneas del tooltip de los items custom | item y número de línea del JSON de `Custom/Items` |
| `Client.txt` | textos propios del cliente (opciones, panel de eventos, barra de buffs, título de la ventana) | el número de cada texto; el comentario de arriba dice cuál es |

Lo que no está traducido lo resuelve el cliente con sus propios archivos, así
que un archivo vacío no rompe nada. Reglas, que el server y el editor
verifican:

- UTF-8 sin BOM, una línea por texto, sin comillas ni saltos de línea adentro.
- Hasta 63 bytes por nombre (el cliente corta los de items a 29 y los de skills
  a 31) y 59 por línea de tooltip.
- En `Client.txt`, los `%d`/`%s`/`%u` de cada línea tienen que ser los mismos que
  los del inglés y en el mismo orden: si no, el cliente descarta la línea.

Se pueden editar a mano o desde el **editor de traducciones** del panel web,
que muestra el nombre actual al lado de la traducción y recarga el server al
guardar.

## Novedades de 0.97.30 — cierre de la Fase 3

- El server compila con CMake en Windows x86 y x64, además de Linux x64 y ARM64. El CI cubre las cuatro combinaciones en Debug y Release.
- MySQL Connector/C++ se descarga en el configure con versión y hash fijos; sus binarios ya no se versionan.
- Correcciones del ancho de puntero en callbacks de sockets, IOCP y timers, necesarias para x64.
- Cola de envío ampliada para el catálogo de items que el server manda al iniciar sesión.
- Comandos GM para adelantar Blood Castle, Devil Square, Golden Archer Bingo e invasiones sin reiniciar el server.

**Compatibilidad: server 0.97.30 ↔ [cliente 0.97.30](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.30).**

### Cómo actualizar desde 0.97.20

Con Docker, actualizá únicamente el server:

```bash
docker compose -f docker-compose.images.yml pull mu-server
docker compose -f docker-compose.images.yml up -d --no-deps mu-server
```

Para fijar la versión, usá la imagen `emapupi/mu-linux-97k:0.97.30` (amd64 y arm64). Un volumen persistente no se actualiza con el pull: agregá en tu `Data/Command.txt` las cuatro filas de los comandos GM nuevos, como se explica en [Administración](#administración-eventos-gm-y-recargas). MySQL, la web y el editor no cambian.

**Compilando desde el código (`docker-compose.yml`):** su servicio MySQL pasa de la imagen `mysql:5.7` a `mysql:8.4.11`, la misma versión que la imagen publicada. Un volumen creado con 5.7 no arranca con 8.4: antes de actualizar, seguí la [guía de migración a 8.4](docs/mysql-8.4-upgrade.md). El compose de imágenes publicadas ya usaba 8.4 y no cambia.

**Pterodactyl:** seleccioná `0.97.30` en `MU_RELEASE_TAG`. En instalaciones existentes, con el servidor detenido, extraé el archivo de tu arquitectura de este Release en una carpeta temporal, reemplazá los cuatro binarios y `start.sh`, y agregá las filas de `Command.txt`, conservando tus INI y Data.

### Compilar y correr en Windows (sin Docker)

1. Requisitos: Visual Studio 2022 o 2026 con "Desarrollo para el escritorio con C++" (toolset v143, incluye CMake) y MySQL 8.
2. Compilar desde la carpeta `server`:

   ```
   cmake --preset windows-x64-release
   cmake --build --preset windows-x64-release --parallel 4
   ```

   Los ejecutables quedan en `server/Source/MuServer/bin/Release/`.
3. Copiar cada ejecutable a su carpeta de `server/MuServer/`, junto a su `.ini`:
   - `ConnectServer/ConnectServer.exe` → `server/MuServer/ConnectServer/`
   - `GameServer/GameServer.exe` → `server/MuServer/GameServer/` (lee los datos de `server/MuServer/Data/`)
   - `MYSQL/JoinServer/JoinServer.exe` → `server/MuServer/MySQL/JoinServer/`
   - `MYSQL/DataServer/DataServer.exe` → `server/MuServer/MySQL/DataServer/`

   No hace falta copiar ninguna DLL de MySQL: el conector queda compilado dentro de JoinServer y DataServer.
4. Base de datos: ejecutar `server/MuServer/MySQL/DB/CreateDatabase.sql` y después `PoblateDatabase.sql`. El usuario por defecto es `admin` / `asd123`; si usás otro, cambialo en `DataServer.ini` y `JoinServer.ini`.
5. En `server/MuServer/ConnectServer/ServerList.dat`, poner la IP del server (`127.0.0.1` para probar en la misma PC).
6. Arrancar en este orden: DataServer, JoinServer, ConnectServer y GameServer.

El cliente `0.97.40` trae compilada la dirección `mu.server-pups.space`. Para apuntarlo a un server propio sin recompilar, agregar en `C:\Windows\System32\drivers\etc\hosts` (como administrador) la línea `127.0.0.1 mu.server-pups.space`, y sacarla al terminar.

#### Windows x86 y dependencias

x64 es el build recomendado. x86 (Win32) queda como base histórica: es la arquitectura del MuEmu original y sigue compilando en el CI, pero ya no es el camino de esta guía. Para compilarlo, también desde `server`:

```powershell
cmake --preset windows-x86-release
cmake --build --preset windows-x86-release --parallel 4
```

También están disponibles `windows-x86-debug` y `windows-x64-debug`; los ejecutables quedan en la misma carpeta que los de x64. JoinServer y DataServer hablan con MySQL por la API en C (`mysql.h`): CMake descarga el código fuente de **MariaDB Connector/C 3.4.11** (SHA256 fijado en `server/cmake/MuMySQLPackages.cmake`) y lo compila como biblioteca estática, con Schannel para TLS. Al ser API en C sirve igual con MSVC y con MinGW. MySQL 8.4 autentica con `caching_sha2_password`, que necesita TLS; el conector lo pide sin verificar el certificado autofirmado del server. En Linux se usa `libmysqlclient-dev` (o `libmariadb-dev`) del sistema. El primer configure requiere acceso a GitHub. Los proyectos MSBuild históricos compilan el mismo conector en `Debug_MySQL` y `Release_MySQL`, con el toolset del proyecto; necesitan CMake en PATH.

Los cuatro servicios compilan en el CI para Windows x86/x64 y Linux x64/ARM64, Debug y Release. ARM64 sigue como compilado, no probado en partida. Fase 3 incluye correcciones del ancho de puntero en callbacks, IOCP y timers, y una cola de envío de 256 KiB para el catálogo de login; el límite individual sigue en 8 KiB. La prueba local de Windows x64 cubrió login, partida, reset, compra/venta y Chaos; no equivale a una prueba de carga.

#### Compilar con MinGW

Los cuatro servicios y `mu-data` también compilan con MinGW-w64 (GCC), en x64 y x86. Los presets son para compilación cruzada desde Linux; en Debian/Ubuntu hacen falta `g++-mingw-w64-x86-64-posix`, `g++-mingw-w64-i686-posix` y `ninja-build`:

```sh
cd server
cmake --preset mingw-x64-release
cmake --build --preset mingw-x64-release --parallel 4
```

Los `.exe` quedan en `server/out/build/mingw-x64-release/bin/` (`mingw-x86-release` para 32 bits) y salen estáticos: no piden las DLL del toolchain. Se copian a `server/MuServer/` igual que los de MSVC. Compilar con MSYS2 desde Windows todavía no está probado.

### Administración: eventos GM y recargas

Sólo GM nivel 1 o superior, desde el chat del juego:

| Comando | Evento |
| --- | --- |
| `/tamastart` | Tamachan |
| `/bcstart` | Blood Castle, todos los niveles |
| `/dsstart` | Devil Square, todos los niveles |
| `/bingostart` | Golden Archer Bingo |
| `/invasionstart <indice>` | `index` de la invasión en `server/MuServer/Data/Events/Invasions/Invasions.toml` (o, en un server sin ese archivo, el índice de `Event/InvasionManager.dat`) |

Programan una sola ejecución aproximadamente dos minutos después, sin modificar los horarios del archivo ni reiniciar eventos activos. BC/DS también rechazan el pedido con la entrada abierta. Iniciar una invasión no reinicia las demás. El evento debe estar habilitado. Los nuevos comandos registran el pedido y resultado en `COMMAND_LOG`.

En instalaciones existentes, agregar las cuatro filas nuevas de `server/MuServer/Data/Command.txt` después de `/tamastart` y antes de `end`, en este orden: bcstart, dsstart, bingostart, invasionstart. El orden determina el ID; conservar el resto de la configuración.

#### Ejemplo: recargar tiendas desde Docker

Escribir sólo `shop` o `/reload shops` en la consola **no funciona**: el GameServer Linux no lee órdenes desde stdin. Desde la terminal del host, en la carpeta del Compose:

```sh
docker compose exec -T mu-server sh -c 'printf "shop\n" > "${MU_DATA_PATH:-/opt/mu/Data}/EditorReload.flag"'
```

Si ya estás dentro de la shell del contenedor del servidor:

```sh
printf 'shop\n' > "${MU_DATA_PATH:-/opt/mu/Data}/EditorReload.flag"
```

El servidor consume `EditorReload.flag` en unos 10 segundos. Confirmar `Shop reloaded by editor flag` en el log. En despliegues con `docker-compose.images.yml`, agregar `-f docker-compose.images.yml` a `docker compose`. Sin Compose, usar `docker exec <contenedor>` en lugar de `docker compose exec -T mu-server`.

Tokens disponibles: `all`, `common`, `shop`, `monster`, `move`/`gate`, `quest`, `util`, `item`, `skill`, `event`, `eventitembag`, `chaosmix`, `command`, `custom`, `character`, `map`, `hack`. Se pueden separar con espacios o comas. Esperar a que se consuma un pedido antes de enviar otro. `event` y `all` reinicializan eventos: no usarlos para adelantar uno ni durante un evento activo. Windows conserva las recargas del menú.

## Novedades de 0.97.20 — cierre de la Fase 2

**Compatibilidad: server 0.97.20 ↔ [cliente 0.97.20](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.20).** Actualizá ambos juntos: el formato extendido de items no es compatible con clientes anteriores ni con el Main.dll histórico.

- El servidor envía al iniciar sesión el catálogo de items, monstruos, efectos, pets y tooltips. El cliente recibe sus definiciones y conserva los modelos y texturas en sus archivos.
- Cada sección admite items nuevos con índices 32..511 y formato de 7 bytes. La columna **Comportamiento** permite reutilizar la lógica de un item clásico en ataques, skills, mezclas y eventos.
- Los JSON de items permiten definir efectos, pose en el inventario y textos del tooltip. El equipo custom se muestra también en otros jugadores y en la selección de personajes.
- Los pets se configuran en JSON. Se incluye **Pet Rudolph (13,400)**, que vuela hacia el zen cercano y lo recoge bajo control del servidor.
- Se conservan los archivos de customs del Encoder del DLL como entrada del catálogo.
- Se corrige la lectura de sección e índice en `ItemOption.txt` con GCC, que impedía que apareciera Triple Shot.

### Cómo actualizar desde 0.97.00

Hacé un backup de tus archivos de Data antes de integrar los ejemplos. No borres los volúmenes ni reemplaces tus configuraciones personalizadas completas.

Con Docker y el compose de imágenes, actualizá únicamente el server:

```bash
docker compose -f docker-compose.images.yml pull mu-server
# Integrá los archivos de Data indicados abajo antes de iniciar el server nuevo.
docker compose -f docker-compose.images.yml up -d --no-deps mu-server
```

El compose usa `latest`; para fijar la versión, cambiá sólo la imagen de `mu-server` a `emapupi/mu-linux-97k:0.97.20`. Hay imágenes para `linux/amd64` y `linux/arm64`.

**Un volumen persistente no se actualiza con el pull.** Desde los archivos de esta versión en `server/MuServer/Data`, integrá en tu volumen `/opt/mu/Data`:

- `Custom/Items/`: `03_000.json` (pose de Light Spear) y `13_400.json` (pose y tooltip de Rudolph).
- `Custom/Pets/13_400.json`: movimiento y recogida de zen de Rudolph.
- `Custom/Encoder/`: `CustomBow.txt`, `CustomGlow.txt`, `CustomItem.txt`, `CustomMonster.txt`, `ItemPosition.txt` y `MapFog.txt`. Si ya tenés customs, fusioná sus entradas; no los sobrescribas.
- `Item/Item.txt`: agregá la fila **400 dentro de la sección 13** de esta versión, si no existe. Conservá el resto de tus filas. Si ya usás 13,400, resolvé ese conflicto antes de incorporar el ejemplo.

Instalá también los assets correspondientes en el cliente 0.97.20. Reiniciá el server y comprobá los mensajes `[ContentCatalog]`, incluyendo `13_400.json`, su tooltip y su pose. El inicio debe mostrar `MU Linux server 0.97.20` y el enlace al cliente compatible.

**MySQL, la web y el editor no cambian en esta entrega:** no hay migración de esquema ni se requiere reconstruir sus imágenes. Si tu instalación aún usa MySQL 8.0, la [migración previa a 8.4](docs/mysql-8.4-upgrade.md) es un trabajo separado.

**Pterodactyl:** importá el egg de server actualizado, que admite el tag `0.97.20` sin `v`, y seleccioná ese valor en `MU_RELEASE_TAG`. La imagen de runtime de Pterodactyl sólo contiene dependencias: actualizarla no cambia los binarios del volumen. En instalaciones existentes, con el servidor detenido, extraé el archivo `mu-linux-97k-server-amd64.tar.gz` o `mu-linux-97k-server-arm64.tar.gz` de este Release en una carpeta temporal; actualizá los cuatro binarios y `start.sh`, e integrá Data como arriba conservando tus INI. No reinstales a ciegas: el instalador extrae también configuraciones y datos. Los eggs de web y MySQL no necesitan cambios. Los archivos por arquitectura deben estar adjuntos al Release antes de instalar esta versión.

### Cómo agregar items custom

El servidor es la fuente de las definiciones; el cliente sólo necesita los modelos `.bmd` y sus texturas. No se regenera `item.bmd` ni `ClientInfo.bmd` para el cliente basado en sources.

1. Agregá la fila del item en su sección de `Data/Item/Item.txt`. Para índices 32..511, indicá el item clásico que imita en **Comportamiento** (por ejemplo `00,020`). Copiá una fila completa de la sección y ajustá sus estadísticas; no uses una fila abreviada con `...`.
2. Lo demás del item (modelo, brillo, arco, ala, pose, efectos, tooltip, pet, apilado y precio) va en `Data/Items/NN-sección.toml`, en una tabla con su índice: `[020]` es el item `0,20`. El encabezado de cada archivo lista las claves; `13-helpers.toml` tiene el ejemplo completo de Rudolph (`13,400`).
3. El modelo va en el cliente, en la carpeta del item: `Data/Item/NN-sección/III/` (el `.bmd` y sus texturas). Con `model = { name = "..." }` el server lo busca ahí.
4. Revisá con `mu-data validate /opt/mu/Data`, reiniciá el servidor y volvé a iniciar sesión para recibir el catálogo. Con permisos GM, probá `/make 13 400` para Rudolph.

Un server con los archivos de antes (`Custom/Encoder`, `CustomWing.txt`, `ItemStack.txt`, `ItemValue.txt`, `Custom/Items` y `Custom/Pets`) los sigue leyendo mientras `Data/Items` no tenga ningún archivo de sección. Para pasarlos al formato nuevo: `mu-data import-items /opt/mu/Data` y `mu-data check-items /opt/mu/Data`.

Consultá los [ejemplos completos y las rutas de assets del cliente](client/README.md#cómo-agregar-items-custom).

## Base y referencia

Basado en las sources de Kayito.

[![Ver README original de Kayito](https://img.shields.io/badge/README%20original-Kayito-181717?logo=github)](https://github.com/nicomuratona/MuEmu-0.97k-kayito#readme)

## Créditos

- Kayito: sources base MuEmu 0.97k.
- Trifon Dinev: Simple MU Online Templates usado en `mu-web`.

## Objetivo

- Servidor Linux nativo con epoll y ajustes de compatibilidad.
- MySQL sin dependencias MSSQL.
- Docker listo para VPS y conexión desde el cliente.

## Imágenes oficiales

- `emapupi/mu-linux-97k` (server), `emapupi/mu-linux-97k-web` (web).
- `emapupi/mu-linux-97k-mu-editor` (editor API), `emapupi/mu-linux-97k-mysql` (MySQL e init).

## Update 1 (2026-02-20)

- Editor web de data, cuentas, personajes, shops, spawns, gates, CFG, items, monsters, drops, backups y snapshots.
- Servicio opcional `mu-editor` con recarga automática mediante `EditorReload.flag`.

## Update 2 (2026-03-26)

- Fixes de cliente para staffs, Character Info, guild, macros y quests; drop Divine y Crystal Sword/Morning Star.

## Update 3 (2026-09-20)

- Server: corrección de Blood Castle y evento de lluvia de joyas con Tamachan (requiere cliente basado en sources para verse completo).
- `MonsterSetBase.txt`: restaurados spawn points normales faltantes y NPCs necesarios en los mapas reportados.
- Docker: CMake limitado a un trabajo (`-j1`) para evitar OOM en hosts con poca memoria.
- Arquitecturas: soporte nativo `linux/amd64` y `linux/arm64`, con MySQL 8.4 LTS para ambas.
- Base de datos: actualización a MySQL 8.4 LTS. Consultá la [guía de migración](docs/mysql-8.4-upgrade.md) antes de reutilizar un volumen existente.
- ARM64: corrección de lectura de scripts y semántica heredada de `char`, evitando errores de parseo y desconexiones incorrectas.
- CI: cada push, PR y ejecución manual valida una imagen `linux/arm64` y un smoke test de inicio.

## Estado del cliente

El cliente basado en inyección de DLL está completamente deprecado y fue retirado de este repositorio junto con sus fuentes y el Encoder ejecutable. Usá el [cliente basado en sources](client/), que está en este repositorio. Los archivos antiguos siguen disponibles en la historia de Git. Las tablas de `server/MuServer/Data/Custom/Encoder/` se conservan porque las consume el catálogo del servidor.

## Inicio rápido

1. Copiá `.env.example` a `.env` y ajustá credenciales, `PUBLIC_IP` y secretos.
2. Levantá el stack:

   ```bash
   docker compose up -d --build
   ```

3. Abrí `44405/tcp` y `55901/tcp`. Para limitar conexiones por IP desde el firewall del host, ver [docs/firewall.md](docs/firewall.md).

Para imágenes publicadas sin build:

```bash
docker compose -f docker-compose.images.yml up -d
```

En ARM con 1 GB de RAM o menos, habilitá 2 GB de swap antes del primer build. `uname -m`: `x86_64` = amd64, `aarch64` = arm64.

## Variables de entorno

`.env` se basa en `.env.example`. Cambiá los valores antes de producción.

- MySQL: `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`.
- Servidor: `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`, `PUBLIC_IP`.
- Web: `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`, `ADMIN_USER`, `ADMIN_PASS`, `TRUST_PROXY`. `GEOIP_ENABLED` y `ACQUISITION_TRACKING` quedan en `0`: las banderas de país y el origen del registro no se activan hasta configurarlos. `GEOIP_ENABLED` necesita el paquete opcional `geoip-lite` (`INSTALL_GEOIP=1` al construir la imagen). `GEOIP_HIDDEN_COUNTRIES` es una lista vacía de códigos ISO. `RANKINGS_RESETS_FIRST=1` ordena las listas de nivel y clase por resets antes que por nivel. `WEB_RATE_LIMIT_PER_MINUTE` (600 por defecto) es el tope de peticiones por minuto y por IP; detrás de un proxy necesita `TRUST_PROXY=1`, o todos los visitantes comparten la IP del proxy.
- Datos de prueba: `SEED_TEST_DATA` (`1` carga cuentas, `0` deja DB limpia).
- Editor: `EDITOR_ENABLED`, `EDITOR_PORT`, `EDITOR_API_URL`, `EDITOR_MAX_BACKUPS`, `EDITOR_MAX_SNAPSHOTS`.

Al cambiar credenciales MySQL tras el primer inicio, recreá el volumen:

```bash
docker compose down -v
docker compose up -d --build
```

## Editor y panel web

El editor es opcional y usa los volúmenes internos de `mu-server`:

```bash
docker compose -f docker-compose.yml -f docker-compose.editor.yml up -d --build
```

Mantiene backups por archivo y snapshots de `server/MuServer/Data` y `server/MuServer/GameServer/DATA`. `mu-web` ofrece registro, login, rankings y noticias en el puerto externo `8085` (interno `8080`). Admin inicial: `admin / 123456`, con cambio obligatorio al primer login. Incluye editores gráficos para cuentas, personajes, inventario, shops, spawns, gates, configuración, GM, eventos, drops y customs.

## Datos, seguridad y logs

- Contraseñas: el JoinServer y el panel las guardan con bcrypt (`PasswordHash=1` en `JoinServer.ini`). Las viejas (texto o MD5, según `MD5Encryption`) siguen entrando y se reemplazan por el hash la primera vez que cada cuenta entra. Hace falta `memb__pwd` de 60 bytes: el panel lo ajusta al arrancar, o corré `Migrate-password-hash.sql`.
- `GlobalPassword` (entra a cualquier cuenta) viene vacía, o sea apagada. Si tu `JoinServer.ini` todavía tiene `XwefDastoD`, borrala: es un valor público.
- Guardá los TXT del servidor en UTF-8 sin BOM. Desde 0.97.40 el server ya no convierte a Windows-1252: un archivo en ANSI con acentos se ve mal en el cliente.
- Shops: `server/MuServer/Data/Shop/*.txt`; `SlotX SlotY` en `-1 -1` autoubica el ítem.
- Mensajes: `server/MuServer/Data/Message_Eng.txt`, `Message_Spn.txt`, `Message_Por.txt` y `server/MuServer/Data/Util/Notice.txt`.
- Logs: `docker compose logs -f`.

Estructura: `server/Source/` (fuentes), `server/MuServer/` (data), `server/docker/` (imágenes y scripts) y `client/` (cliente).

## Web y editor 0.97.20

Primer inicio en /admin: cambio obligatorio de contraseña y asistente de idioma predeterminado (ES/EN/PT/UK, el valor inicial sigue siendo ES), diseño clásico, Elf, Dark Knight o Dark Wizard y estado de conexión del editor. Después se modifica desde /admin/settings. Cada visitante puede elegir su idioma con las banderas; no cambia el idioma del admin/editor ni traduce noticias o datos del juego.

La configuración se guarda automáticamente en MySQL (web_site_settings); conserve su volumen. Actualice únicamente las imágenes emapupi/mu-linux-97k-web:0.97.20 y emapupi/mu-linux-97k-mu-editor:0.97.20 y recree esos servicios. No hace falta reconstruir el servidor ni MySQL. Las pantallas del editor se sirven desde la web; su API conserva el comportamiento anterior.
