# Mu Linux 97K

[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k?label=server&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k)
[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-web?label=web&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-web)
[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mu-editor?label=editor&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mu-editor)

Proyecto para portar y operar MuEmu 0.97k en Linux de forma nativa, con foco en Docker y despliegues on-demand. Incluye fuentes del servidor, cliente y encoder necesarios para operar, y automatiza el build dentro del contenedor.

## Base y referencia
Este repo esta basado en las sources de Kayito. Referencia upstream:
https://github.com/nicomuratona/MuEmu-0.97k-kayito

## Creditos
- Kayito: sources base MuEmu 0.97k.
- Trifon Dinev: template web (Simple MU Online Templates) usado en `mu-web`.

## Objetivo
- Servidor nativo en Linux (epoll + ajustes de compatibilidad)
- Base de datos MySQL (sin dependencias MSSQL)
- Docker listo para levantar en un VPS y conectar con el cliente

## Imagenes oficiales (Docker Hub)
- `emapupi/mu-linux-97k` (server)
- `emapupi/mu-linux-97k-web` (web)
- `emapupi/mu-linux-97k-mu-editor` (editor API)

Tags recomendados:
- Server/Web: `latest`, `0.1.1`
- Editor API: `latest`, `0.1.0`

## Update 1 (2026-02-20)
### Configuracion basica
1. Copiar `.env.example` a `.env` y ajustar:
   - `PUBLIC_IP`, `WEB_PORT`, `SESSION_SECRET`
   - `MYSQL_*`, `DB_*`
   - `ADMIN_USER`, `ADMIN_PASS`
2. Levantar el stack base:
   ```bash
   docker compose up --build -d
   ```

### Editor del servidor (opcional)
1. Habilitar en `.env`:
   - `EDITOR_ENABLED=1`
   - `EDITOR_API_URL=http://mu-editor:8090`
2. Levantar con compose extendido:
   ```bash
   docker compose -f docker-compose.yml -f docker-compose.editor.yml up -d --build
   ```
3. Incluye backups por archivo y snapshots completos (ver seccion "Editor del servidor").

### Funcionalidades incluidas
- Editor web completo para data del servidor con recarga automatica (EditorReload.flag).
- Servicio opcional `mu-editor` (API interna) con backups por archivo y snapshots completos.
- Editor de cuentas/personajes con inventario y baul grafico.
- Editor de shops grafico con import/export de .txt.
- Editor de spawns (MonsterSetBase) con mapa, areas y zonas bloqueadas.
- Editor de gates con mapa y notas sobre Gate.bmd del cliente.
- Editor de Move.txt con link a gates.
- Editor de GM, Notices y Blacklist.
- Editor de CFG general (Common/Skill/Event/Command/StartUp) con restaurar valores.
- Editor de MapManager.
- Editor de items base (Item.txt / ItemValue.txt / ItemStack.txt).
- Editor de monsters (Monster.txt).
- Editor de ResetTable y ExperienceTable.
- Editor de ChaosMix.
- Editor de EventItemBag en modo normal y avanzado (incluye Manager).
- Editor de Drop con filtros y validaciones.
- Soporte a customs (CustomSafeZone, CustomPkFree, CustomNpcMove).
- Bloqueo de guardado cuando hay campos invalidos en editores sensibles.
- Fix del cliente: boton de quest proceed (ver Sources del cliente).

## Uso rapido (Docker)
1. Copia `.env.example` a `.env` y ajusta credenciales, IP publica y secrets.
2. En el servidor:
   ```bash
   docker-compose up --build -d
   ```
3. Puertos requeridos:
   - `44405/tcp` (ConnectServer)
   - `55601/udp` (ConnectServer UDP)
   - `55901/tcp` (GameServer)

## Configuracion por variables de entorno
El stack usa un archivo `.env` local (ignorado por git) basado en `.env.example`.
Cambia los valores antes de produccion.
Variables principales:
- `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`
- `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`
- `PUBLIC_IP`
- `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`
- `ADMIN_USER`, `ADMIN_PASS`
- `TRUST_PROXY` (1 si usas proxy inverso como Nginx Proxy Manager)
- `SEED_TEST_DATA` (1 para cargar cuentas de prueba, 0 para DB limpia)
- `EDITOR_ENABLED` (1 habilita editor, 0 lo apaga)
- `EDITOR_PORT` (puerto interno del editor, default 8090)
- `EDITOR_API_URL` (url interna para mu-web, ej `http://mu-editor:8090`)
- `EDITOR_MAX_BACKUPS` (cantidad de backups por archivo, default 5)
- `EDITOR_MAX_SNAPSHOTS` (cantidad de snapshots completos, default 5)

Nota: si cambias credenciales de MySQL despues del primer arranque, recrea el volumen:
```bash
docker compose down -v
docker compose up -d --build
```

## Editor del servidor (opcional)
El editor corre como servicio separado (`mu-editor`) y se monta sobre los volumenes internos de `mu-server`.
No es necesario exponer puertos; `mu-web` accede por red interna de Docker.

Para levantarlo:
```bash
docker compose -f docker-compose.yml -f docker-compose.editor.yml up -d --build
```

Backups:
- Por archivo: se guarda hasta `EDITOR_MAX_BACKUPS`.
- Snapshots completos de `MuServer/Data` y `MuServer/GameServer/DATA` (hasta `EDITOR_MAX_SNAPSHOTS`).
- Se administran desde el panel admin (crear/restaurar).

## Panel web (mu-web)
Se incluye un panel web simple (registro, login, rankings y noticias) corriendo en un contenedor aparte.
- Puerto: `8085` (mapea al `8080` interno del contenedor).
- Admin inicial: `admin / 123456` con cambio obligatorio al primer login.
- Captcha: Cloudflare Turnstile (configurable).

### Panel admin
Incluye editores graficos para:
- Cuentas, personajes, inventario y baul.
- Shops (import/export).
- Spawns (MonsterSetBase), gates y Move.txt.
- CFG general (Common/Skill/Event/Command/StartUp) + MapManager.
- GM, Notices, Blacklist.
- Items/Monsters base, Reset/Experience.
- ChaosMix, EventItemBag (normal/avanzado) y Drop.
- Customs (SafeZone, PkFree, NpcMove).
- Backups y snapshots.

Variables principales (desde `.env`):
- `TURNSTILE_SITE_KEY`
- `TURNSTILE_SECRET_KEY`
- `SESSION_SECRET`

## Seguridad de cuentas (MD5)
El JoinServer esta configurado con `MD5Encryption=2` (MD5 binario).
Esto requiere que la base de datos sea recreada si vienes de texto plano.
Pasos:
1. `docker compose down -v`
2. `docker compose up -d --build`

## Edici�n de data (shops, mensajes, etc.)
- Shops: editar `MuServer/Data/Shop/*.txt`.  
  Formato por l�nea: `Index Level Dur Skill Luck Option ExcOp SlotX SlotY`  
  Usa `SlotX`/`SlotY` = `-1 -1` para autoubicar en la grilla.
- Mensajes del server: `MuServer/Data/Message_Eng.txt`, `Message_Spn.txt`, `Message_Por.txt` y avisos globales en `MuServer/Data/Util/Notice.txt`.
- **Encoding**: guardar estos `.txt` en **ANSI / Windows-1252** y sin BOM (UTF-8 rompe acentos en el cliente).

## Prueba r�pida (solo testing)
Creaci�n manual de cuenta para verificar login (temporal hasta definir flujo definitivo):
```sql
INSERT INTO MEMB_INFO
  (memb___id, memb__pwd, memb_name, mail_addr, sno__numb, AccountLevel, bloc_code)
VALUES
  ('test', UNHEX(MD5('301993')), 'test', 'test@test.com', '111111111111111111', 0, '0');

INSERT IGNORE INTO MEMB_STAT (memb___id, ConnectStat)
VALUES ('test', 0);
```
Nota: en esta base `MD5Encryption=2`, la password se guarda en MD5 binario. No usar esto en producci�n.

## Logs y control
El control b�sico es desde Docker/Portainer (start/stop/restart). Los logs se pueden ver en la consola del contenedor (Portainer ? Logs o `docker logs -f`).

## Estructura del repo
- `Source/`: fuentes del servidor (nuestras modificaciones)
- `MuServer/`: data/config para correr
- `Client/` y `Encoder/`: esenciales para operar
- `kayito_ref/`: referencia de archivos Windows (ignorado por git)

---

## Upstream README (referencia original)

# kayito MuEmu 0.97k Full Package

## - Emulator: MuEmu

## - Main Base: 0.97k (0.97.11) KOR

## - Content:
- Client Original Version 0.97
- Encoder to generate the Main Info
- Full MuServer MuEmu 0.97
  - Connect Server
  - Join Server
  - Data Server
  - Game Server
- Tool to edit BMD/TXT files
- Tool to edit Accounts, Characters and Items
- Source Code of Everything above mentioned

## Updates

### Update 1
- Corregido problema con baul (era un error leve) -> [DataServer]
- Corregido problema con los stats y requisitos (ahora al repartir puntos por comando, actualiza en tiempo real) -> [GameServer y Main.dll]
- Corregido problema al minimizar (se bypasseó el pedazo de código que ocasionaba la desconexión al minimizar) -> [Main.dll]
### Update 2
- Corregido problema con baul al guardar items (era otro error leve de sincronización con el dataserver) -> [GameServer]

### Update 3
- Corregido problema al repartir todos los puntos, Points se mostraba en 0 y no quitaba los botones para sumar stats -> [Main.dll]
- Corregido problema al abrir Golden Archer no quitaba el minimapa -> [Main.dll]
- Corregido problema de items que se dropeaban con opciones que no correspondían (MuServer/Data/Item/ItemDrop.txt) -> [MuServer]

### Update 4
- Se redujo el tamaño del inventario al tamaño de la 97 original -> [Base de Datos][DataServer][GameServer]
- Se redujo el tamaño del baul al tamaño de la 97 original -> [Base de Datos][DataServer][GameServer]

### Update 5
- Se corrigió un error que no permitía juntar los kriss +0 -> [MuServer/Data/Quest/QuestObjective.txt]
- Se corrigió un error que bloqueaba el shop del NPC luego de hacer Reload Shop teniendo un Shop abierto -> [GameServer]

### Update 6
- Se añadió el sistema de Right Click Move hacia Warehouse, Trade y Chaos Mix -> [main.dll]
- Se corrigió un problema con el warehouse que te permitía extraer y superar el máximo de zen -> [GameServer.exe]
- Se añadió un switch para evitar que se puedan vender los items en los NPC al superar el máximo de zen -> [GameServer.exe][MuServer/GameServer/DATA/GameServerInfo-Common.dat]
- Se añadieron configuraciones para el costo de zen de cada mix en el chaos mix -> [GameServer.exe][MuServer/GameServer/DATA/GameServerInfo-ChaosMix.dat]
- Se añadió un sistema de sincronización para coordinar los rates del Chaos Mix con los del servidor -> [Main.dll][GameServer.exe][MuServer/GameServer/DATA/GameServerInfo-ChaosMix.dat]

### Update 7
- Se añadió el soporte al hardwareID para poder utilizarlo en MuServer/Data/Util/BlackList.dat -> [GameServer.exe]

### Update 8
- Se corrigió un error que al equipar o desequipar items, se visualizaban mal los stats -> [GameServer.exe]
- Se modificaron los mapas del cliente por los mapas originales junto con sus respectivos minimapas sincronizados al 100% -> [Cliente]
- Se añadió el Movelist funcional sincronizado con el move.txt del muserver -> [Main.dll][GameServer.exe]
- Se añadieron switchs para Minimap, Sky, Movelist y HealthBar -> [GetMainInfo.exe][Main.dll]
- Se corrigió un error que no previsualizaba correctamente al equiparse un Dinorant -> [GameServer.exe]
- Se corrigió un error que se veían unos cuadros negros del recorte del terreno al usar resoluciones superiores a 1280x1024 -> [Main.dll]
- Se corrigió y sincronizó todos los precios de compra de items por NPC en un 99% (Falta verificar las opciones excellent) -> [Main.dll][GameServer.exe]
- Se corrigió y sincronizó todos los precios de venta de items hacia un NPC en un 99% (Falta verificar las opciones excellent) -> [Main.dll][GameServer.exe]
- Se corrigió y sincronizó los costos de reparación de los items desde el inventario y desde el NPC -> [Main.dll][GameServer.exe]
- Se sincronizaron los stacks de items del main con el archivo ItemStack.txt del muserver -> [Main.dll][GameServer.exe]
- Se sincronizaron los precios de los items que se encuentren en ItemValue.txt en el muserver -> [Main.dll][GameServer.exe]
- Se corrigió el botón del zoom al clickear en el minimapa y ahora el personaje no camina al clickear ahí -> [Main.dll]

### Update 9
- Se corrigió un error en el ChaosMix que crasheaba el cliente -> [Cliente][GameServer.exe][main.dll]

### Update 10
- Se añadió un sistema de MapManager el cual permite manejar los mapas desde el GetMainInfo (Nombre, Movimiento, Acuático y Música) -> [Main.dll][GetMainInfo.exe]
- Se añadió un sistema de sincronización de los nombres de los subservers -> [Main.dll][ConnectServer.exe]

### Update 11:
- Corregido problema al resetear y quitar stats, no actualizaba los requisitos de los items -> [Main.dll]
- Corregido problema al dropear zen 0 -> [Main.dll][GameServer.exe]
- Corregido el problema del drop de zen 1 -> [GameServer.exe]
- Agregado caída de zen variable (antes dropeaba siempre una cantidad fija) -> [GameServer.exe]
- Agregado carpeta de sonido compatible (créditos: Kapocha33) -> [Cliente]
- Corregido problema de reconnect con sistema de desconexión de cuenta online -> [GameServer.exe]
- Corregido el problema del chat de guild y el guild master -> [GameServer.exe]
- Debido a la falta de auto attack, se optó por añadir un autoclick derecho con la tecla F9 -> [Main.dll]
- Añadido centrado de Server List en el Select Server (créditos: Kapocha33) -> [Main.dll]

### Update 12:
- Se corrigió el sistema de fruits [GameServer.exe][Main.dll]

### Update 13:
- Se corrigió un crasheo random al apuntar a un monster [Main.dll]
- Se corrigieron todos los sonidos del juego [Main.dll][Cliente/Data/Sound]

### Update 14:
- Se corrigió un crasheo que sucedía en un área específica de Stadium [Cliente][MuServer]
- Se corrigió los precios de compra, venta y reparación de items [Main.dll][GameServer.exe]
- Se añadió el sistema de juntar items con barra espaciadora [Main.dll]

### Update 15:
- Se añadió el sistema de Texturas para los items custom. Créditos: SetecSoft [Main.dll]
- Se añadió el sistema de Click Derecho para Equipar/Desequipar items. [Main.dll]
- Se añadió un sistema de empaquetamiento sincronizado con el ItemStack.txt del servidor que permite armar packs de cualquier item (ideal para jewels) [Main.dll][GameServer.exe][MuServer/Data/Item/ItemStack.txt]
- Se añadió un sistema de click derecho para desempaquetar de a uno los items empaquetados (ideal para jewels) [Main.dll]
- Se optimizó el sistema de protocolos del lado cliente para poder procesar y recibir todos los tipos de protocolos desde el servidor [Main.dll]
- Se optimizó el viewport de buffs y efectos del lado servidor, haciéndolo más rápido y eficiente [GameServer.exe]
- Se modificó la ubicación de los items custom, ahora permitiendo organizarlos y separarlos por carpetas individuales [Main.dll]
- Se corrigió un error que hacía que ciertos buffs y efectos desaparezcan al moverse de mapa o al desaparecer de la vista y reaparecer [Main.dll]
- Se corrigió un error que al intentar hacer Grand Reset, no se podía utilizar ningún otro comando y no se aplicaba el Grand Reset [DataServer.exe]
- Se corrigió un error que no se verificaba correctamente el nivel requerido para crear un MG [Main.dll][GameServer.exe]
- Se corrigió un error que calculaba mal el daño de la elfa cuando utilizaba Bows/Crossbows sin durabilidad [GameServer.exe]

### Update 16:
- Se separó por completo el sistema de Glow de los Custom Item. Créditos por la idea: Zeus [Main.dll][GetMainInfo.exe][MainInfo/CustomItem.txt][MainInfo/CustomGlow.txt]
- Se añadió al Custom Item la columna Skill para poder definir qué tipo de skill tendrán las armas [Main.dll][GetMainInfo.exe][MainInfo/CustomItem.txt]
- Se añadió el sistema de volumen para los sonidos del juego en el menu de opciones. Dicho sistema es compatible con el registro de Windows que todos los mains manejan [Main.dll]
- Se agregó el sonido al subir de nivel [Main.dll][Cliente/Data/Sound/pLevelUp.wav]
- Se corrigió que el minimapa no procese los clicks en su área correspondiente si la imagen del minimapa no existe en su correspondiente World en el cliente [Main.dll]
- Se corrigió un valor en el ItemValue que calculaba mal los precios de los items con muchas opciones [Main.dll][GameServer.exe]
- Se añadieron items custom a modo de ejemplo para que puedan continuar agregando por su cuenta sin problemas siguiendo dichas configuraciones

### Update 17:
- Se añadió el sistema de ItemOption para manipular las opciones de los items [Main.dll][GetMainInfo.exe][MainInfo/ItemOption.txt]
- Se sincronizó el sistema de manejo de opciones [Main.dll][GameServer.exe][MuServer/Data/Item/ItemOption.txt]
- Se sincronizó el sistema de ItemConvert (para requisitos, daños, durabilidad y opciones de los items) [Main.dll][GameServer.exe]
- Se sincronizó el sistema de ItemValue (para los precios de los items) [Main.dll][GameServer.exe]
- Se corrigieron los colores de los daños de todo tipo [GameServer.exe]
- Se corrigió poder ingresar a la misma cuenta usando mayúsculas y/o minúsculas [JoinServer.exe]
- Se corrigió utilizar el /move y soltar el item seleccionado para dupearlo visualmente [GameServer.exe]
- Se corrigió la animación del Power Slash del MG [Main.dll][Cliente/Data/Player/Player.bmd]
- Se corrigió que el Power Slash a veces no atacaba correctamente a los objetivos [GameServer.exe]
- Se modificó el campo ItemIndex en el BonusManager (ya no hace falta poner *,* y con un único * es suficiente) [GameServer.exe][MuServer/Data/Event/BonusManager.dat]

### Update 18:
- Se corrigió el drop de items con skill que no corresponden [GameServer.exe]
- Se corrigió el drop de items con nivel que no corresponden [MuServer/Data/Item/ItemDrop.txt]
- Se corrigió las opciones excellent de las alas cuando son full y cuando no (Damage y HP) [Main.dll][GameServer.exe]
- Se corrigió un crasheo inesperado que ocurría cuando se respawneaba luego de morir o de cambiar de mapa [Main.dll]
- Se corrigió un error visual por el cual el glow de los items no se mostraba correctamente acorde a su nivel [GameServer.exe]
- Se corrigió los requisitos del move respecto del MG (La ecuación para el MG es Requisito = ((MinLevel * 2) / 3)) [Main.dll]
- Se corrigió el respawn fuera del mapa de origen siendo menor a nivel 6 y estando fuera de safe zone [GameServer.exe]
- Se corrigieron errores en las tools que hacían que algunos txt sean mal interpretados [Tools/kayitoTools][Tools/kayitoEditor]
- Se sincronizó el editor de items con el ItemOption.txt para poder visualizar correctamente qué opciones puede llevar cada item [Tools/kayitoEditor]

### Update 19:
- Se migró el proyecto a GitHub para llevar mejor control de los cambios por update
- Se corrigió el problema que no permitía equipar los maces en la 2da mano [Main.dll]
- Se corrigió un error que generaba un crasheo en el main al intentar reparar un item muy caro [Main.dll]
- Se añadió un sistema de FONT en el que permite cambiar el tipo de fuente del cliente y el tamaño de letra [Main.dll]
- Se mejoró la interacción con el minimapa y el movelist respecto a los clicks [Main.dll]
- Se mejoró el dibujado de la barra de experiencia y el número que se muestra [Main.dll]
- Se reacomodaron algunos skills que funcionaban mal o no permitían atacarse entre usuarios (Por ej. Rageful Blow) [GameServer.exe]
- Se mejoró el sistema de cola de paquetes, reduciendo el consumo y aumentando la eficiencia (Créditos SetecSoft) [ConnectServer.exe][JoinServer.exe][DataServer.exe][GameServer.exe]
- Se corrigió el Weapon View en la zona safe. Ahora ambas armas se muestran como corresponde [Main.dll]
- Se implementó un nuevo MiniMapa llamado FullMap, que es generado por código automáticamente evitando así utilizar texturas (funcional para todos los mapas) [Main.dll]

### Update 20:
- Se corrigió un error en el MoveList que hacía que los colores de los nombres de los items dropeados se vean rojos [Main.dll]
- Se corrigió la posición del Skull Shield en la espalda [Main.dll]
- Se corrigió la interacción con click derecho en las entradas al Devil Square y Blood Castle en todos sus niveles [GameServer.exe]
- Se corrigió que al estar PK e intentar ingresar al Devil Square o al Blood Castle, no mostraba ningún mensaje [GameServer.exe]
- Se reconstruyeron los chequeos para equiparse items con click derecho [Main.dll][GameServer.exe]
- Se corrigió el MoveList que no se bloqueaba cuando el personaje es PK [Main.dll]
- Se optimizó el dibujado de la interface de los ejecutables. Ahora se recargan sólo cuando hay un log nuevo, reduciendo el consumo [ConnectServer][JoinServer][DataServer][GameServer]
- Se movieron las configuraciones de inicio que estaban en
"MuServer/GameServer/DATA/GameServerInfo - Common.dat"  
hacia otro archivo separado en  
"MuServer/GameServer/DATA/GameServerInfo - StartUp.dat"

### Update 21:
- Se corrigió el sistema de texturas que se continuaban perjudicando a medida que se agregaban más y más items (créditos: Zeus) [Main.dll]
- Se corrigieron los nombres en el HealthBar que se recortaban cuando el texto superaba el tamaño de la barra [Main.dll]
- Se añadió un sistema de reproductor musical para quitar definitivamente el MuPlayer.exe y que no haga falta integrar wzAudio.dll ni ogg.dll ni vorbisfile.dll [Main.dll]
- Se expandió el máximo de caracteres de los mensajes globales antes de que realice un salto de línea [Main.dll]
- Se migraron todas las lecturas de configuraciones al archivo Config.ini dentro del cliente (ya no se utiliza el registro de windows) [Main.dll]
- Se implementó un menu de opciones avanzadas que permite  
    - Cambiar el lenguaje (Eng, Spn, Por) sin salir del juego
    - Regular el volumen de los sonidos y la música por separado, y pausar/reproducir la música
    - Cambiar entre modo ventana y fullscreen y también cambiar la resolución del juego
    - Cambiar el tipo de fuente, el tamaño, la negrita y la cursiva
- Se corrigió que luego de reconectar, la barra de experiencia y el número mostraban datos erróneos
- Se corrigieron los textos de Bolts/Arrows que se muestran en la esquina superior derecha
- Se corrigieron los textos y las barras de HP de los pets que se muestran en la esquina superior derecha
- Se corrigieron los textos de los items que hacen falta reparar que se muestran en la esquina superior derecha

### Update 22:
- Se añadió el main.ida al repositorio de github
- Se corrigió el renderizado de los ejecutables del servidor [ConnectServer][JoinServer][DataServer][GameServer]
- Se corrigió que se mostaba el botón de subir puntos aún teniendo 0 puntos disponibles [Main.dll]
- Se agregó que se pueda guardar el ID desde el Config.ini [Main.dll]
- Se eliminó el límite de tamaño de texturas tanto JPG como TGA (usar a discresión) [Main.dll]
- Se unificó el sistema de TrayMode con el sistema de Window para corregir el autoclick F9 que no funcionaba ni en TrayMode ni al sacar de foco al juego [Main.dll]
- Se verificó nuevamente el ItemOption porque algunos items caían con skill cuando no correspondía [Main.dll][GameServer][kayito Editor]
- Se corrigió el contador de monstruos en negativo del GameServer (El problema estaba al invocar un monstruo con la Elf) [GameServer]
- Se corrigió el StoredProcedure WZ_DISCONNECT_MEMB que se encargaba de contar las horas online (Pueden revisar el archivo CreateDatabase y revisar la diferencia del procedure con el que está en su base de datos, modifican y le dan a Ejecutar) [Base de Datos]
- Se corrigió el error al morir el Dinorant estando en Icarus y sin alas, no retornaba a Devias [GameServer]
- Se añadió una opción para permitir crear personajes y guilds con caracteres especiales [Encoder][Main.dll][DataServer]
- Se corrigió la velocidad de ataque de la animación Power Slash del MG [Player.bmd]

### Update 23:
- Se añadió soporte a MySQL en una rama paralela en el mismo Github para poner a prueba el nuevo motor [JoinServer][DataServer]
- Se corrigió el error que algunos skills dejaban de atacar a algunos monstruos luego de un tiempo [Main.dll][GameServer]
- Se optimizó el uso del Fullmap para que en lugar de dibujar cada cuadrito del suelo, genere una textura previamente y renderice la textura [Main.dll]
- También se hizo que el Fullmap se rote a 45º para que coincida con la orientación real de la cámara respecto del mapa [Main.dll]
- Se realizó una función para retornar correctamente los valores y rates de cada opción excellent obtenida desde el ItemOption.txt y también se acomodaron los textos correspondientes en el Text.bmd [Main.dll][Text.bmd]
- Se añadió un log para mostrar la IP en la que se genere algún tipo de error desde el ConnectServer [ConnectServer]
- Se corrigió el valor real del Attack Speed de todos los personajes. Ahora utiliza siempre el recibido por el GameServer [Main.dll]
- Se añadió la interacción al juntar un item, diciendo el nombre y el nivel del mismo [Main.dll]
- Se modificó la ecuación de la experiencia para poder expandir el nivel máximo hasta 1000 y se añadió una configuración en el GameServer Common.dat para poder manipularlo a gusto [Main.dll][GameServer]
- Se añadió la vista del PING y de los FPS en el texto del borde de la ventana del juego [Main.dll][GameServer]
- Se añadió un panel para visualizar los temporizadores de los eventos e invasiones [Main.dll][GameServer]
- Se optimizó el MemScript que se encargaba de leer los TXT de configuración para que ahora informe la línea donde ocurra un error [Encoder][GameServer]
- Se corrigió un error que al colocarse un item con skill, no lo asignaba correctamente a la primera vez [GameServer]
- Se implementó el Custom Monster con la posibilidad de agregar Monsters y NPCs tanto normales como Goldens [Encoder][Main.dll]
- Se corrigió un error que al atacar y moverse sin parar de atacar, podía bugearse visualmente de tal modo que las demás personas te veían saltando por todos lados [Main.dll]

## Fixes varios:
- Se corrigió el rango de ataque de los skills Triple Shot y Power Slash (ahora pueden ser manipulados desde skill.txt con range y radio) [GameServer]
- Se integró la función GetItemName para que al juntar los items, muestre correctamente el nombre de los items que según su nivel son otro item [Main.dll]
- Se corrigió un error en el dinorant mientras vuela que se reiniciaba la animación constantemente [Main.dll]
- Se corrigió un error que al desconectarse algunos miembros del guild, no se reflejaba correctamente su estado [DataServer]

- Se corrigió los items que no podían ser vendidos en los shops (Fairy, Satan, Uniria, Dinorant) [GameServer.exe]
- Se corrigió la visualización de todas las opciones en los items full option [GameServer.exe][Main.dll]
- Se corrigió el orden de las opciones Luck y Additional en las Alas [Main.dll]

- Se corrigió poder usar los skills de los items equipados [GameServer.exe][Main.dll]
- Se mejoró el manejo del modo ventana y modo full screen al cambiar las "Display Settings" [Main.dll]
- Se corrigió que los custom monsters no sean detectados como NPC (ahora deberán utilizar la segunda columna del Monster.txt donde 0 = NPC y 1 = Monster) [GameServer.exe]
- Se añadió al Kayito Tools la columna Type al leer y guardar Monster.txt y NpcName.txt [KayitoTools.exe]
- Se corrigió un error que hacía que se lean mal los nombres rellenándolos con espacios [KayitoTools.exe][KayitoEditor.exe]

- Se tradujeron los textos del servidor [Message_por.txt][Message_spn.txt]
- Se tradujeron los textos del cliente [Dialog_por.bmd][Dialog_spn.bmd][Text_por.bmd][Text_Spn.bmd]
- Se corrigieron los lectores de texto que no permitían caracteres especiales [GameServer.exe][Main.dll][Encoder.exe]
- Se añadió la música de login. El archivo deberá ser el siguiente: "Cliente\\Data\\Music\\MuTheme.mp3" [Main.dll]
- Se corrigieron las animaciones del MG con Rune Blade [Player.bmd][Main.dll]
- Se corrigió la barra de experiencia que no mostraba correctamente el progreso de las 10 partes [Main.dll]
- Se añadió la encriptación default MD5 sin necesidad de usar el usuario como key [JoinServer.exe]
- Se corrigió que el editor al guardar los cambios en un personaje, le actualiza la experiencia correctamente [KayitoEditor.exe]
- Se corrigió un error que generaba un lenguaje al azar y enviaba mensajes incorrectos [GameServer.exe][Main.dll]

### Update 24:
- Se upgradeó la versión de OpenGL a 3.3 [Main.dll]
- Se añadió nuevamente la opción EnableTrusted al editor para SQL para utilizar las credenciales de Windows [kayitoTools.exe]
- Se corrigió un error en el envío de datos del item 31 de cada sección [GameServer.exe]
- Se mejoró el sistema de ventana sin bordes y la forma de cambiarlo en el menu de opciones [Main.dll]
- Se mejoró el reproductor de música integrado [Main.dll]
- Se optimizó la carga y manejo del SkyDome [Main.dll]
- Se añadió un fix al glow de todas las armas de la primer mano [Main.dll]
- Se modificó la forma de leer la carpeta de screenshots y se añadió un sistema que crea automáticamente la carpeta al tomar una captura [Main.dll]
- Se aplicó una mejora a la carga de fuentes para poder elegir [Main.dll]
- Se agregó nuevamente el sistema de minimap junto con el sistema de Fullmap [Main.dll]
- Se modificó el MapManager para poder elegir por cada mapa FullMap/MiniMap o ninguno y mostrar o no el SkyDome [Encoder.exe][Main.dll]
- Se modificó el sistema de carga de modelos y texturas para intentar solucionar el problema de texturas del main [Main.dll]

### Mini-Update 25:
- Se añadió CustomBow [Main.dll][Encoder.exe]
- Se corrigió el skill Fire Slash con el skill Twisting Slash [Main.dll][GameServer.exe]
- Se corrigió la apertura del minimapa con el movelist y el event timer [Main.dll]
- Se corrigió la carga de modelos y texturas de items [Main.dll]
- Se añadió un menu antilag [Main.dll]

### Update 26:
- Se corrigió el sonido al juntar Jewel of Chaos [Main.dll]
- Se corrigió el error de fechas de baneo y vip en el editor al guardar una cuenta [KayitoEditor.exe]
- Se corrigió el Text.bmd (Eng, Por y Spn) respecto a los textos del menu de opciones [Client/Data/Local/Text_XXX.bmd]
- Se corrigieron las armas de 2 manos originales en Item.bmd e Item.txt (antes estaban de 1 mano) [MuServer/Data/Item/Item.txt][Client/Data/Local/Item.bmd]
- Se corrigió que al apuntar a los monsters, cambiaba el color de los textos de los items dropeados [Main.dll]
- Se añadió un bloqueo de máximo tamaño de letra en menu de opciones y config.ini [Main.dll]
- Se corrigió un error al cambiar el nombre de la ventana del juego [Main.dll]
- Se mejoró el sistema de ping (créditos ogocx) [Main.dll]
- Se corrigió el daño visual de los staffs [Main.dll]
- Se mejoró el rango visual del minimapa respecto al zoom (Minimapa tiene mayor zoom por defecto) [Main.dll]
- Se mejoró la función del protocolo, permitiendo ignorar paquetes en el protocolo original [Main.dll]
- Se añadió sistema de niveles de ingreso para Devil Square y Blood Castle [GameServer.exe][Main.dll][MuServer/Data/Event/DevilSquare.dat][MuServer/Data/Event/BloodCastle.dat]
- Se añadió el sistema de Custom wing [GameServer.exe][Main.dll][Encoder.exe]
- Se añadió una configuración de rate para que el mix de Alas S2 permita generar Custom Wings [GameServer.exe]
- Revisar puntos por reset cuando grand reset otorga puntos también [GameServer.exe]
- Se añadió el sistema de Custom Item Position [Main.dll][Encoder.exe]
- Se corrigió el problema de colisión de modelos entre monsters e items [Main.dll]
- Se creó un cálculo de experiencia automático a partir del nivel máximo definido en `GameServerInfo - Common.dat` [GameServer.exe][Main.dll]
- Se añadió un ItemBag para las Box of Kundun +5 y para el puesto 1º del evento DevilSquare [GameServer.exe]

### Update 27:
- Se añadieron opciones al encoder para deshabilitar visualmente resets y grand resets [Encoder/MainInfo.ini]
    - Estos cambios influyen en el texto de la ventana y en la interface C
- Se reconstruyó la interface C y se sincronizaron los valores con el gameserver
- Se corrigió el error que al equipar Angel, no incrementaba el HP +50
- Se corrigió que el mg no podía equiparse 2 staffs con click derecho
- Se cambió que el BonusTime del BonusManager ahora es en minutos (antes en segundos) [MuServer/Data/Event/BonusManager.dat]
- Se cambió que el MaxRegenTime del EventSpawnMonster ahora es en segundos (antes en milisegundos) [MuServer/Data/Event/EventSpawnMonster.dat]
- Se cambió que el InvasionTime del InvasionManager ahora es en minutos (antes en segundos) [MuServer/Data/Event/InvasionManager.dat]
- Se cambió que el RegenTime del InvasionManager ahora es en segundos (antes en milisegundos) [MuServer/Data/Event/InvasionManager.dat]
- Se agregó poder inicializar las invasiones y los bonus desde el menu del GameServer
- Se corrigió la cantidad de dragones que aparecían en las invasiones en los mapas Atlans e Icarus
- Se añadió al menu de opciones antilag la opción de quitar el HPBar de los monsters [Client/Data/Local/Text_xxx.bmd]
- Se corrigió poder hablar con npc y moverse de mapa si PkLimitFree está activo
- Se añadió nivel mínimo y máximo, reset mínimo y máximo, y vip al MoveList
- Se bloqueó el buff de elfa para que sólo pueda darse a los miembros del party
- Se añadió una opción al menu de opciones para activar pvp sin presionar Ctrl [Client/Data/Local/Text_xxx.bmd]
- Se corrigió la resolución 1366x768 que no sacaba screenshots correctamente
- Se quitó la actualización de OpenGL hacia la 3.3 debido a reclamos por incompatibilidad con ciertas pc
- Se corrigió la visualización de los ataques de la elfa visto desde otro personaje
- Se eliminó el SkyDome del juego completamente
- Se reconstruyó el sistema de Cámara 3D y se añadió un sistema de FOG
- Se añadió el SellValue al archivo ItemValue para colocar valor de venta [MuServer/Data/Item/ItemValue.txt]
- Se reconstruyó por completo la funcionalidad del Golden Archer, incluyendo un evento propio
    - Reemplazar:
        - GameServer.exe
        - DataServer.exe
        - Main.dll
    - Afecta los siguientes archivos:
        - MuServer/GameServer/DATA/GameServerInfo - Event.dat
        - MuServer/Data/Message_xxx.txt
        - MuServer/Data/EventItemBagManager.txt
        - MuServer/Data/EventItemBag/
            - 032 - Golden Archer Rena 1.txt
            - 033 - Golden Archer Rena 2.txt
            - 034 - Golden Archer Rena 3.txt
            - 035 - Golden Archer Rena 4.txt
            - 036 - Golden Archer Bingo.txt
        - MuServer/Data/Event/GoldenArcherBingo.dat
    - Hace falta añadir una tabla y modificar otra. De ser posible, volver a crear la base de datos con los scripts. Sino se hará un script sólo para updatear para aquellos con base de datos en uso.
- Se corrigió un error que al atacar con Twisting Slash y luego tirar Death Stab, el arma rotando era cambiada por otras

## Fixes post-update
- Se corrigió el cierre de la ventana al moverse con el Golden Archer abierto.
- Se añadieron los textos "Este item no te pertenece" al intentar juntar un item de otra persona
- Se corrigió el error al intentar colocar zen superior al propio dentro del trade
- Se corrigió el error al clickear en el NPC de Quest cuando no se cumplen los requisitos de nivel
- Se corrigió un error que al estar utilizando el Twisting, no se podía quitar el arma
- Se expandió el máximo de zen para insertar al trade/warehouse
- Se añadió el mensaje de zen al máximo
- Se añadió el mensaje de zen insuficiente
- Se eliminó el log de chaos mix al subir a +11
- Se añadió la opción Infinity Arrows al Common.dat
- Se corrigió un error en el Kayito Editor que fallaba al cambiar la fecha de Ban y de VIP











