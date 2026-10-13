# Mu Linux 97K

[![Sitio web](https://img.shields.io/badge/sitio%20web-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/es/)
[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k?label=server&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-web?label=web&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-web) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mu-editor?label=editor&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mu-editor) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mysql?label=mysql&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mysql)

🇪🇸 Español | [🇺🇸 English](README.en.md) | [🇧🇷 Português](README.pt-BR.md)

Proyecto para operar MuEmu 0.97k en Linux de forma nativa, con Docker, web y editor opcional.

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

**Un volumen persistente no se actualiza con el pull.** Desde los archivos de esta versión en `MuServer/Data`, integrá en tu volumen `/opt/mu/Data`:

- `Custom/Items/`: `03_000.json` (pose de Light Spear) y `13_400.json` (pose y tooltip de Rudolph).
- `Custom/Pets/13_400.json`: movimiento y recogida de zen de Rudolph.
- `Custom/Encoder/`: `CustomBow.txt`, `CustomGlow.txt`, `CustomItem.txt`, `CustomMonster.txt`, `ItemPosition.txt` y `MapFog.txt`. Si ya tenés customs, fusioná sus entradas; no los sobrescribas.
- `Item/Item.txt`: agregá la fila **400 dentro de la sección 13** de esta versión, si no existe. Conservá el resto de tus filas. Si ya usás 13,400, resolvé ese conflicto antes de incorporar el ejemplo.

Instalá también los assets correspondientes en el cliente 0.97.20. Reiniciá el server y comprobá los mensajes `[ContentCatalog]`, incluyendo `13_400.json`, su tooltip y su pose. El inicio debe mostrar `MU Linux server 0.97.20` y el enlace al cliente compatible.

**MySQL, la web y el editor no cambian en esta entrega:** no hay migración de esquema ni se requiere reconstruir sus imágenes. Si tu instalación aún usa MySQL 8.0, la [migración previa a 8.4](docs/mysql-8.4-upgrade.md) es un trabajo separado.

**Pterodactyl:** importá el egg de server actualizado, que admite el tag `0.97.20` sin `v`, y seleccioná ese valor en `MU_RELEASE_TAG`. La imagen de runtime de Pterodactyl sólo contiene dependencias: actualizarla no cambia los binarios del volumen. En instalaciones existentes, con el servidor detenido, extraé el archivo `mu-linux-97k-server-amd64.tar.gz` o `mu-linux-97k-server-arm64.tar.gz` de este Release en una carpeta temporal; actualizá los cuatro binarios y `start.sh`, e integrá Data como arriba conservando tus INI. No reinstales a ciegas: el instalador extrae también configuraciones y datos. Los eggs de web y MySQL no necesitan cambios. Los archivos por arquitectura deben estar adjuntos al Release antes de instalar esta versión.

### Cómo agregar items custom

El servidor es la fuente de las definiciones; el cliente sólo necesita los modelos `.bmd` y sus texturas. No se regenera `item.bmd` ni `ClientInfo.bmd` para el cliente basado en sources.

1. Agregá la fila del item en su sección de `Data/Item/Item.txt`. Para índices 32..511, indicá el item clásico que imita en **Comportamiento** (por ejemplo `00,020`), seguido de carpeta, modelo y brillo RGB. Copiá una fila completa de la sección y ajustá sus estadísticas; no uses una fila abreviada con `...`.
2. Para customs clásicos, podés conservar las definiciones de modelo/brillo en `Data/Custom/Encoder/CustomItem.txt` y `CustomGlow.txt`, y las demás tablas del Encoder incluidas.
3. Opcionalmente, creá `Data/Custom/Items/<sección>_<índice>.json` con `item`, `effects`, `inventory` y/o `tooltip`. Los ejemplos `03_000.json` y `13_400.json` muestran el formato real.
4. Para un pet propio, agregá `Data/Custom/Pets/<sección>_<índice>.json`. Rudolph usa `13,400`, slot 8, comportamiento `*`, y define movimiento y recogida de zen. El cliente anima el pet; el servidor autoriza y ejecuta la recogida.
5. Distribuí los assets del modelo al cliente, reiniciá el servidor y volvé a iniciar sesión para recibir el catálogo. Probá, con permisos GM, `/make 13 400` para Rudolph.

Consultá los [ejemplos completos y las rutas de assets del cliente](https://github.com/EmanuelCatania/Mu-97k-Client-Source/blob/0.97.20/README.md#cómo-agregar-items-custom).

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

El cliente basado en inyección de DLL está completamente deprecado y fue retirado de este repositorio junto con sus fuentes y el Encoder ejecutable. Usá el [cliente basado en sources 0.97.20](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.20). Los archivos antiguos siguen disponibles en la historia de Git. Las tablas de `MuServer/Data/Custom/Encoder/` se conservan porque las consume el catálogo del servidor.

## Inicio rápido

1. Copiá `.env.example` a `.env` y ajustá credenciales, `PUBLIC_IP` y secretos.
2. Levantá el stack:

   ```bash
   docker compose up -d --build
   ```

3. Abrí `44405/tcp`, `55601/udp` y `55901/tcp`.

Para imágenes publicadas sin build:

```bash
docker compose -f docker-compose.images.yml up -d
```

En ARM con 1 GB de RAM o menos, habilitá 2 GB de swap antes del primer build. `uname -m`: `x86_64` = amd64, `aarch64` = arm64.

## Variables de entorno

`.env` se basa en `.env.example`. Cambiá los valores antes de producción.

- MySQL: `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`.
- Servidor: `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`, `PUBLIC_IP`.
- Web: `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`, `ADMIN_USER`, `ADMIN_PASS`, `TRUST_PROXY`.
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

Mantiene backups por archivo y snapshots de `MuServer/Data` y `MuServer/GameServer/DATA`. `mu-web` ofrece registro, login, rankings y noticias en el puerto externo `8085` (interno `8080`). Admin inicial: `admin / 123456`, con cambio obligatorio al primer login. Incluye editores gráficos para cuentas, personajes, inventario, shops, spawns, gates, configuración, GM, eventos, drops y customs.

## Datos, seguridad y logs

- `MD5Encryption=2` usa MD5 binario. Al migrar desde texto plano, recreá la DB con `docker compose down -v`.
- Guardá TXT del servidor en ANSI / Windows-1252 sin BOM; UTF-8 puede romper acentos del cliente.
- Shops: `MuServer/Data/Shop/*.txt`; `SlotX SlotY` en `-1 -1` autoubica el ítem.
- Mensajes: `MuServer/Data/Message_Eng.txt`, `Message_Spn.txt`, `Message_Por.txt` y `MuServer/Data/Util/Notice.txt`.
- Logs: `docker compose logs -f`.

Estructura: `Source/` (fuentes), `MuServer/` (data), `docker/` (imágenes y scripts).

## Web y editor 0.97.20

Primer inicio en /admin: cambio obligatorio de contraseña y asistente de idioma predeterminado (ES/EN/PT), diseño clásico, Elf, Dark Knight o Dark Wizard y estado de conexión del editor. Después se modifica desde /admin/settings. Cada visitante puede elegir su idioma con las banderas; no cambia el idioma del admin/editor ni traduce noticias o datos del juego.

La configuración se guarda automáticamente en MySQL (web_site_settings); conserve su volumen. Actualice únicamente las imágenes emapupi/mu-linux-97k-web:0.97.20 y emapupi/mu-linux-97k-mu-editor:0.97.20 y recree esos servicios. No hace falta reconstruir el servidor ni MySQL. Las pantallas del editor se sirven desde la web; su API conserva el comportamiento anterior.
