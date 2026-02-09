# Mu Linux 97K

![Docker Hub (Server)](https://img.shields.io/docker/v/emapupi/mu-linux-97k?label=server&logo=docker)
![Docker Hub (Web)](https://img.shields.io/docker/v/emapupi/mu-linux-97k-web?label=web&logo=docker)

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

## Docker Hub
Imagen publica (solo mu-server):

```
docker pull emapupi/mu-linux-97k:latest
```

La base de datos y el panel web se levantan con `docker-compose` desde este repo.

## Uso rapido (Docker)
1. Edita `.env` y ajusta credenciales, IP publica y secrets.
2. En el servidor:
   ```bash
   docker-compose up --build -d
   ```
3. Puertos requeridos:
   - `44405/tcp` (ConnectServer)
   - `55601/udp` (ConnectServer UDP)
   - `55901/tcp` (GameServer)

## Configuracion por variables de entorno
El stack usa un archivo `.env` incluido como ejemplo. Cambia los valores antes de produccion.
Variables principales:
- `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`
- `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`
- `PUBLIC_IP`
- `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`
- `ADMIN_USER`, `ADMIN_PASS`
- `TRUST_PROXY` (1 si usas proxy inverso como Nginx Proxy Manager)
- `SEED_TEST_DATA` (1 para cargar cuentas de prueba, 0 para DB limpia)

Nota: si cambias credenciales de MySQL despues del primer arranque, recrea el volumen:
```bash
docker compose down -v
docker compose up -d --build
```

## Panel web (mu-web)
Se incluye un panel web simple (registro, login, rankings y noticias) corriendo en un contenedor aparte.
- Puerto: `8085` (mapea al `8080` interno del contenedor).
- Admin inicial: `admin / 123456` con cambio obligatorio al primer login.
- Captcha: Cloudflare Turnstile (configurable).

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

## Edicion de data (shops, mensajes, etc.)
- Shops: editar `MuServer/Data/Shop/*.txt`.  
  Formato por linea: `Index Level Dur Skill Luck Option ExcOp SlotX SlotY`  
  Usa `SlotX`/`SlotY` = `-1 -1` para autoubicar en la grilla.
- Mensajes del server: `MuServer/Data/Message_Eng.txt`, `Message_Spn.txt`, `Message_Por.txt` y avisos globales en `MuServer/Data/Util/Notice.txt`.
- **Encoding**: guardar estos `.txt` en **ANSI / Windows-1252** y sin BOM (UTF-8 rompe acentos en el cliente).

## Prueba rapida (solo testing)
Creacion manual de cuenta para verificar login (temporal hasta definir flujo definitivo):
```sql
INSERT INTO MEMB_INFO
  (memb___id, memb__pwd, memb_name, mail_addr, sno__numb, AccountLevel, bloc_code)
VALUES
  ('test', UNHEX(MD5('123456')), 'test', 'test@test.com', '111111111111111111', 0, '0');

INSERT IGNORE INTO MEMB_STAT (memb___id, ConnectStat)
VALUES ('test', 0);
```
Nota: en esta base `MD5Encryption=2`, la password se guarda en MD5 binario. No usar esto en produccion.

## Logs y control
El control basico es desde Docker/Portainer (start/stop/restart). Los logs se pueden ver en la consola del contenedor (Portainer ? Logs o `docker logs -f`).

## Estructura del repo
- `Source/`: fuentes del servidor
- `MuServer/`: data/config para correr
- `Client/` y `Encoder/`: esenciales para operar

---

**Pterodactyl**
Eggs disponibles en `Pterodactyl eggs/` y también en la Release v0.1.0 (asset `Pterodactyl eggs.zip`).

Imágenes Docker:
- emapupi/mu-linux-97k:latest
- emapupi/mu-linux-97k-web:latest

Notas:
- Requiere UDP abierto (puerto 55601) para jugar.
- La DB debe crearse desde el panel y asignarse al servidor.
- La web usa el mismo MySQL (variables DB_*).

Release:
https://github.com/EmanuelCatania/Mu-Linux-0.97k/releases/tag/v0.1.0

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
- Corregido problema al minimizar (se bypasseo el pedazo de codigo que ocasionaba la desconexion al minimizar) -> [Main.dll]
### Update 2
- Corregido problema con baul al guardar items (era otro error leve de sincronizacion con el dataserver) -> [GameServer]

### Update 3
- Corregido problema al repartir todos los puntos, Points se mostraba en 0 y no quitaba los botones para sumar stats -> [Main.dll]
- Corregido problema al abrir Golden Archer no quitaba el minimapa -> [Main.dll]
- Corregido problema de items que se dropeaban con opciones que no correspondian (MuServer/Data/Item/ItemDrop.txt) -> [MuServer]

### Update 4
- Se redujo el tamano del inventario al tamano de la 97 original -> [Base de Datos][DataServer][GameServer]
- Se redujo el tamano del baul al tamano de la 97 original -> [Base de Datos][DataServer][GameServer]

### Update 5
- Se corrigio un error que no permitia juntar los kriss +0 -> [MuServer/Data/Quest/QuestObjective.txt]
- Se corrigio un error que bloqueaba el shop del NPC luego de hacer Reload Shop teniendo un Shop abierto -> [GameServer]

### Update 6
- Se anadio el sistema de Right Click Move hacia Warehouse, Trade y Chaos Mix -> [main.dll]
- Se corrigio un problema con el warehouse que te permitia extraer y superar el maximo de zen -> [GameServer.exe]
- Se anadio un switch para evitar que se puedan vender los items en los NPC al superar el maximo de zen -> [GameServer.exe][MuServer/GameServer/DATA/GameServerInfo-Common.dat]
- Se anadieron configuraciones para el costo de zen de cada mix en el chaos mix -> [GameServer.exe][MuServer/GameServer/DATA/GameServerInfo-ChaosMix.dat]
- Se anadio un sistema de sincronizacion para coordinar los rates del Chaos Mix con los del servidor -> [Main.dll][GameServer.exe][MuServer/GameServer/DATA/GameServerInfo-ChaosMix.dat]

### Update 7
- Se anadio el soporte al hardwareID para poder utilizarlo en MuServer/Data/Util/BlackList.dat -> [GameServer.exe]

### Update 8
- Se corrigio un error que al equipar o desequipar items, se visualizaban mal los stats -> [GameServer.exe]
- Se modificaron los mapas del cliente por los mapas originales junto con sus respectivos minimapas sincronizados al 100% -> [Cliente]
- Se anadio el Movelist funcional sincronizado con el move.txt del muserver -> [Main.dll][GameServer.exe]
- Se anadieron switchs para Minimap, Sky, Movelist y HealthBar -> [GetMainInfo.exe][Main.dll]
- Se corrigio un error que no previsualizaba correctamente al equiparse un Dinorant -> [GameServer.exe]
- Se corrigio un error que se veian unos cuadros negros del recorte del terreno al usar resoluciones superiores a 1280x1024 -> [Main.dll]
- Se corrigio y sincronizo todos los precios de compra de items por NPC en un 99% (Falta verificar las opciones excellent) -> [Main.dll][GameServer.exe]
- Se corrigio y sincronizo todos los precios de venta de items hacia un NPC en un 99% (Falta verificar las opciones excellent) -> [Main.dll][GameServer.exe]
- Se corrigio y sincronizo los costos de reparacion de los items desde el inventario y desde el NPC -> [Main.dll][GameServer.exe]
- Se sincronizaron los stacks de items del main con el archivo ItemStack.txt del muserver -> [Main.dll][GameServer.exe]
- Se sincronizaron los precios de los items que se encuentren en ItemValue.txt en el muserver -> [Main.dll][GameServer.exe]
- Se corrigio el boton del zoom al clickear en el minimapa y ahora el personaje no camina al clickear ahi -> [Main.dll]

### Update 9
- Se corrigio un error en el ChaosMix que crasheaba el cliente -> [Cliente][GameServer.exe][main.dll]

### Update 10
- Se anadio un sistema de MapManager el cual permite manejar los mapas desde el GetMainInfo (Nombre, Movimiento, Acuatico y Musica) -> [Main.dll][GetMainInfo.exe]
- Se anadio un sistema de sincronizacion de los nombres de los subservers -> [Main.dll][ConnectServer.exe]

### Update 11:
- Corregido problema al resetear y quitar stats, no actualizaba los requisitos de los items -> [Main.dll]
- Corregido problema al dropear zen 0 -> [Main.dll][GameServer.exe]
- Corregido el problema del drop de zen 1 -> [GameServer.exe]
- Agregado caida de zen variable (antes dropeaba siempre una cantidad fija) -> [GameServer.exe]
- Agregado carpeta de sonido compatible (creditos: Kapocha33) -> [Cliente]
- Corregido problema de reconnect con sistema de desconexion de cuenta online -> [GameServer.exe]
- Corregido el problema del chat de guild y el guild master -> [GameServer.exe]
- Debido a la falta de auto attack, se opto por anadir un autoclick derecho con la tecla F9 -> [Main.dll]
- Anadido centrado de Server List en el Select Server (creditos: Kapocha33) -> [Main.dll]

### Update 12:
- Se corrigio el sistema de fruits [GameServer.exe][Main.dll]

### Update 13:
- Se corrigio un crasheo random al apuntar a un monster [Main.dll]
- Se corrigieron todos los sonidos del juego [Main.dll][Cliente/Data/Sound]

### Update 14:
- Se corrigio un crasheo que sucedia en un area especifica de Stadium [Cliente][MuServer]
- Se corrigio los precios de compra, venta y reparacion de items [Main.dll][GameServer.exe]
- Se anadio el sistema de juntar items con barra espaciadora [Main.dll]

### Update 15:
- Se anadio el sistema de Texturas para los items custom. Creditos: SetecSoft [Main.dll]
- Se anadio el sistema de Click Derecho para Equipar/Desequipar items. [Main.dll]
- Se anadio un sistema de empaquetamiento sincronizado con el ItemStack.txt del servidor que permite armar packs de cualquier item (ideal para jewels) [Main.dll][GameServer.exe][MuServer/Data/Item/ItemStack.txt]
- Se anadio un sistema de click derecho para desempaquetar de a uno los items empaquetados (ideal para jewels) [Main.dll]
- Se optimizo el sistema de protocolos del lado cliente para poder procesar y recibir todos los tipos de protocolos desde el servidor [Main.dll]
- Se optimizo el viewport de buffs y efectos del lado servidor, haciendolo mas rapido y eficiente [GameServer.exe]
- Se modifico la ubicacion de los items custom, ahora permitiendo organizarlos y separarlos por carpetas individuales [Main.dll]
- Se corrigio un error que hacia que ciertos buffs y efectos desaparezcan al moverse de mapa o al desaparecer de la vista y reaparecer [Main.dll]
- Se corrigio un error que al intentar hacer Grand Reset, no se podia utilizar ningun otro comando y no se aplicaba el Grand Reset [DataServer.exe]
- Se corrigio un error que no se verificaba correctamente el nivel requerido para crear un MG [Main.dll][GameServer.exe]
- Se corrigio un error que calculaba mal el dano de la elfa cuando utilizaba Bows/Crossbows sin durabilidad [GameServer.exe]

### Update 16:
- Se separo por completo el sistema de Glow de los Custom Item. Creditos por la idea: Zeus [Main.dll][GetMainInfo.exe][MainInfo/CustomItem.txt][MainInfo/CustomGlow.txt]
- Se anadio al Custom Item la columna Skill para poder definir que tipo de skill tendran las armas [Main.dll][GetMainInfo.exe][MainInfo/CustomItem.txt]
- Se anadio el sistema de volumen para los sonidos del juego en el menu de opciones. Dicho sistema es compatible con el registro de Windows que todos los mains manejan [Main.dll]
- Se agrego el sonido al subir de nivel [Main.dll][Cliente/Data/Sound/pLevelUp.wav]
- Se corrigio que el minimapa no procese los clicks en su area correspondiente si la imagen del minimapa no existe en su correspondiente World en el cliente [Main.dll]
- Se corrigio un valor en el ItemValue que calculaba mal los precios de los items con muchas opciones [Main.dll][GameServer.exe]
- Se anadieron items custom a modo de ejemplo para que puedan continuar agregando por su cuenta sin problemas siguiendo dichas configuraciones

### Update 17:
- Se anadio el sistema de ItemOption para manipular las opciones de los items [Main.dll][GetMainInfo.exe][MainInfo/ItemOption.txt]
- Se sincronizo el sistema de manejo de opciones [Main.dll][GameServer.exe][MuServer/Data/Item/ItemOption.txt]
- Se sincronizo el sistema de ItemConvert (para requisitos, danos, durabilidad y opciones de los items) [Main.dll][GameServer.exe]
- Se sincronizo el sistema de ItemValue (para los precios de los items) [Main.dll][GameServer.exe]
- Se corrigieron los colores de los danos de todo tipo [GameServer.exe]
- Se corrigio poder ingresar a la misma cuenta usando mayusculas y/o minusculas [JoinServer.exe]
- Se corrigio utilizar el /move y soltar el item seleccionado para dupearlo visualmente [GameServer.exe]
- Se corrigio la animacion del Power Slash del MG [Main.dll][Cliente/Data/Player/Player.bmd]
- Se corrigio que el Power Slash a veces no atacaba correctamente a los objetivos [GameServer.exe]
- Se modifico el campo ItemIndex en el BonusManager (ya no hace falta poner *,* y con un unico * es suficiente) [GameServer.exe][MuServer/Data/Event/BonusManager.dat]

### Update 18:
- Se corrigio el drop de items con skill que no corresponden [GameServer.exe]
- Se corrigio el drop de items con nivel que no corresponden [MuServer/Data/Item/ItemDrop.txt]
- Se corrigio las opciones excellent de las alas cuando son full y cuando no (Damage y HP) [Main.dll][GameServer.exe]
- Se corrigio un crasheo inesperado que ocurria cuando se respawneaba luego de morir o de cambiar de mapa [Main.dll]
- Se corrigio un error visual por el cual el glow de los items no se mostraba correctamente acorde a su nivel [GameServer.exe]
- Se corrigio los requisitos del move respecto del MG (La ecuacion para el MG es Requisito = ((MinLevel * 2) / 3)) [Main.dll]
- Se corrigio el respawn fuera del mapa de origen siendo menor a nivel 6 y estando fuera de safe zone [GameServer.exe]
- Se corrigieron errores en las tools que hacian que algunos txt sean mal interpretados [Tools/kayitoTools][Tools/kayitoEditor]
- Se sincronizo el editor de items con el ItemOption.txt para poder visualizar correctamente que opciones puede llevar cada item [Tools/kayitoEditor]

### Update 19:
- Se migro el proyecto a GitHub para llevar mejor control de los cambios por update
- Se corrigio el problema que no permitia equipar los maces en la 2da mano [Main.dll]
- Se corrigio un error que generaba un crasheo en el main al intentar reparar un item muy caro [Main.dll]
- Se anadio un sistema de FONT en el que permite cambiar el tipo de fuente del cliente y el tamano de letra [Main.dll]
- Se mejoro la interaccion con el minimapa y el movelist respecto a los clicks [Main.dll]
- Se mejoro el dibujado de la barra de experiencia y el numero que se muestra [Main.dll]
- Se reacomodaron algunos skills que funcionaban mal o no permitian atacarse entre usuarios (Por ej. Rageful Blow) [GameServer.exe]
- Se mejoro el sistema de cola de paquetes, reduciendo el consumo y aumentando la eficiencia (Creditos SetecSoft) [ConnectServer.exe][JoinServer.exe][DataServer.exe][GameServer.exe]
- Se corrigio el Weapon View en la zona safe. Ahora ambas armas se muestran como corresponde [Main.dll]
- Se implemento un nuevo MiniMapa llamado FullMap, que es generado por codigo automaticamente evitando asi utilizar texturas (funcional para todos los mapas) [Main.dll]

### Update 20:
- Se corrigio un error en el MoveList que hacia que los colores de los nombres de los items dropeados se vean rojos [Main.dll]
- Se corrigio la posicion del Skull Shield en la espalda [Main.dll]
- Se corrigio la interaccion con click derecho en las entradas al Devil Square y Blood Castle en todos sus niveles [GameServer.exe]
- Se corrigio que al estar PK e intentar ingresar al Devil Square o al Blood Castle, no mostraba ningun mensaje [GameServer.exe]
- Se reconstruyeron los chequeos para equiparse items con click derecho [Main.dll][GameServer.exe]
- Se corrigio el MoveList que no se bloqueaba cuando el personaje es PK [Main.dll]
- Se optimizo el dibujado de la interface de los ejecutables. Ahora se recargan solo cuando hay un log nuevo, reduciendo el consumo [ConnectServer][JoinServer][DataServer][GameServer]
- Se movieron las configuraciones de inicio que estaban en
"MuServer/GameServer/DATA/GameServerInfo - Common.dat"  
hacia otro archivo separado en  
"MuServer/GameServer/DATA/GameServerInfo - StartUp.dat"

### Update 21:
- Se corrigio el sistema de texturas que se continuaban perjudicando a medida que se agregaban mas y mas items (creditos: Zeus) [Main.dll]
- Se corrigieron los nombres en el HealthBar que se recortaban cuando el texto superaba el tamano de la barra [Main.dll]
- Se anadio un sistema de reproductor musical para quitar definitivamente el MuPlayer.exe y que no haga falta integrar wzAudio.dll ni ogg.dll ni vorbisfile.dll [Main.dll]
- Se expandio el maximo de caracteres de los mensajes globales antes de que realice un salto de linea [Main.dll]
- Se migraron todas las lecturas de configuraciones al archivo Config.ini dentro del cliente (ya no se utiliza el registro de windows) [Main.dll]
- Se implemento un menu de opciones avanzadas que permite  
    - Cambiar el lenguaje (Eng, Spn, Por) sin salir del juego
    - Regular el volumen de los sonidos y la musica por separado, y pausar/reproducir la musica
    - Cambiar entre modo ventana y fullscreen y tambien cambiar la resolucion del juego
    - Cambiar el tipo de fuente, el tamano, la negrita y la cursiva
- Se corrigio que luego de reconectar, la barra de experiencia y el numero mostraban datos erroneos
- Se corrigieron los textos de Bolts/Arrows que se muestran en la esquina superior derecha
- Se corrigieron los textos y las barras de HP de los pets que se muestran en la esquina superior derecha
- Se corrigieron los textos de los items que hacen falta reparar que se muestran en la esquina superior derecha

### Update 22:
- Se anadio el main.ida al repositorio de github
- Se corrigio el renderizado de los ejecutables del servidor [ConnectServer][JoinServer][DataServer][GameServer]
- Se corrigio que se mostaba el boton de subir puntos aun teniendo 0 puntos disponibles [Main.dll]
- Se agrego que se pueda guardar el ID desde el Config.ini [Main.dll]
- Se elimino el limite de tamano de texturas tanto JPG como TGA (usar a discresion) [Main.dll]
- Se unifico el sistema de TrayMode con el sistema de Window para corregir el autoclick F9 que no funcionaba ni en TrayMode ni al sacar de foco al juego [Main.dll]
- Se verifico nuevamente el ItemOption porque algunos items caian con skill cuando no correspondia [Main.dll][GameServer][kayito Editor]
- Se corrigio el contador de monstruos en negativo del GameServer (El problema estaba al invocar un monstruo con la Elf) [GameServer]
- Se corrigio el StoredProcedure WZ_DISCONNECT_MEMB que se encargaba de contar las horas online (Pueden revisar el archivo CreateDatabase y revisar la diferencia del procedure con el que esta en su base de datos, modifican y le dan a Ejecutar) [Base de Datos]
- Se corrigio el error al morir el Dinorant estando en Icarus y sin alas, no retornaba a Devias [GameServer]
- Se anadio una opcion para permitir crear personajes y guilds con caracteres especiales [Encoder][Main.dll][DataServer]
- Se corrigio la velocidad de ataque de la animacion Power Slash del MG [Player.bmd]

### Update 23:
- Se anadio soporte a MySQL en una rama paralela en el mismo Github para poner a prueba el nuevo motor [JoinServer][DataServer]
- Se corrigio el error que algunos skills dejaban de atacar a algunos monstruos luego de un tiempo [Main.dll][GameServer]
- Se optimizo el uso del Fullmap para que en lugar de dibujar cada cuadrito del suelo, genere una textura previamente y renderice la textura [Main.dll]
- Tambien se hizo que el Fullmap se rote a 45o para que coincida con la orientacion real de la camara respecto del mapa [Main.dll]
- Se realizo una funcion para retornar correctamente los valores y rates de cada opcion excellent obtenida desde el ItemOption.txt y tambien se acomodaron los textos correspondientes en el Text.bmd [Main.dll][Text.bmd]
- Se anadio un log para mostrar la IP en la que se genere algun tipo de error desde el ConnectServer [ConnectServer]
- Se corrigio el valor real del Attack Speed de todos los personajes. Ahora utiliza siempre el recibido por el GameServer [Main.dll]
- Se anadio la interaccion al juntar un item, diciendo el nombre y el nivel del mismo [Main.dll]
- Se modifico la ecuacion de la experiencia para poder expandir el nivel maximo hasta 1000 y se anadio una configuracion en el GameServer Common.dat para poder manipularlo a gusto [Main.dll][GameServer]
- Se anadio la vista del PING y de los FPS en el texto del borde de la ventana del juego [Main.dll][GameServer]
- Se anadio un panel para visualizar los temporizadores de los eventos e invasiones [Main.dll][GameServer]
- Se optimizo el MemScript que se encargaba de leer los TXT de configuracion para que ahora informe la linea donde ocurra un error [Encoder][GameServer]
- Se corrigio un error que al colocarse un item con skill, no lo asignaba correctamente a la primera vez [GameServer]
- Se implemento el Custom Monster con la posibilidad de agregar Monsters y NPCs tanto normales como Goldens [Encoder][Main.dll]
- Se corrigio un error que al atacar y moverse sin parar de atacar, podia bugearse visualmente de tal modo que las demas personas te veian saltando por todos lados [Main.dll]

## Fixes varios:
- Se corrigio el rango de ataque de los skills Triple Shot y Power Slash (ahora pueden ser manipulados desde skill.txt con range y radio) [GameServer]
- Se integro la funcion GetItemName para que al juntar los items, muestre correctamente el nombre de los items que segun su nivel son otro item [Main.dll]
- Se corrigio un error en el dinorant mientras vuela que se reiniciaba la animacion constantemente [Main.dll]
- Se corrigio un error que al desconectarse algunos miembros del guild, no se reflejaba correctamente su estado [DataServer]

- Se corrigio los items que no podian ser vendidos en los shops (Fairy, Satan, Uniria, Dinorant) [GameServer.exe]
- Se corrigio la visualizacion de todas las opciones en los items full option [GameServer.exe][Main.dll]
- Se corrigio el orden de las opciones Luck y Additional en las Alas [Main.dll]

- Se corrigio poder usar los skills de los items equipados [GameServer.exe][Main.dll]
- Se mejoro el manejo del modo ventana y modo full screen al cambiar las "Display Settings" [Main.dll]
- Se corrigio que los custom monsters no sean detectados como NPC (ahora deberan utilizar la segunda columna del Monster.txt donde 0 = NPC y 1 = Monster) [GameServer.exe]
- Se anadio al Kayito Tools la columna Type al leer y guardar Monster.txt y NpcName.txt [KayitoTools.exe]
- Se corrigio un error que hacia que se lean mal los nombres rellenandolos con espacios [KayitoTools.exe][KayitoEditor.exe]

- Se tradujeron los textos del servidor [Message_por.txt][Message_spn.txt]
- Se tradujeron los textos del cliente [Dialog_por.bmd][Dialog_spn.bmd][Text_por.bmd][Text_Spn.bmd]
- Se corrigieron los lectores de texto que no permitian caracteres especiales [GameServer.exe][Main.dll][Encoder.exe]
- Se anadio la musica de login. El archivo debera ser el siguiente: "Cliente\\Data\\Music\\MuTheme.mp3" [Main.dll]
- Se corrigieron las animaciones del MG con Rune Blade [Player.bmd][Main.dll]
- Se corrigio la barra de experiencia que no mostraba correctamente el progreso de las 10 partes [Main.dll]
- Se anadio la encriptacion default MD5 sin necesidad de usar el usuario como key [JoinServer.exe]
- Se corrigio que el editor al guardar los cambios en un personaje, le actualiza la experiencia correctamente [KayitoEditor.exe]
- Se corrigio un error que generaba un lenguaje al azar y enviaba mensajes incorrectos [GameServer.exe][Main.dll]

### Update 24:
- Se upgradeo la version de OpenGL a 3.3 [Main.dll]
- Se anadio nuevamente la opcion EnableTrusted al editor para SQL para utilizar las credenciales de Windows [kayitoTools.exe]
- Se corrigio un error en el envio de datos del item 31 de cada seccion [GameServer.exe]
- Se mejoro el sistema de ventana sin bordes y la forma de cambiarlo en el menu de opciones [Main.dll]
- Se mejoro el reproductor de musica integrado [Main.dll]
- Se optimizo la carga y manejo del SkyDome [Main.dll]
- Se anadio un fix al glow de todas las armas de la primer mano [Main.dll]
- Se modifico la forma de leer la carpeta de screenshots y se anadio un sistema que crea automaticamente la carpeta al tomar una captura [Main.dll]
- Se aplico una mejora a la carga de fuentes para poder elegir [Main.dll]
- Se agrego nuevamente el sistema de minimap junto con el sistema de Fullmap [Main.dll]
- Se modifico el MapManager para poder elegir por cada mapa FullMap/MiniMap o ninguno y mostrar o no el SkyDome [Encoder.exe][Main.dll]
- Se modifico el sistema de carga de modelos y texturas para intentar solucionar el problema de texturas del main [Main.dll]

### Mini-Update 25:
- Se anadio CustomBow [Main.dll][Encoder.exe]
- Se corrigio el skill Fire Slash con el skill Twisting Slash [Main.dll][GameServer.exe]
- Se corrigio la apertura del minimapa con el movelist y el event timer [Main.dll]
- Se corrigio la carga de modelos y texturas de items [Main.dll]
- Se anadio un menu antilag [Main.dll]

### Update 26:
- Se corrigio el sonido al juntar Jewel of Chaos [Main.dll]
- Se corrigio el error de fechas de baneo y vip en el editor al guardar una cuenta [KayitoEditor.exe]
- Se corrigio el Text.bmd (Eng, Por y Spn) respecto a los textos del menu de opciones [Client/Data/Local/Text_XXX.bmd]
- Se corrigieron las armas de 2 manos originales en Item.bmd e Item.txt (antes estaban de 1 mano) [MuServer/Data/Item/Item.txt][Client/Data/Local/Item.bmd]
- Se corrigio que al apuntar a los monsters, cambiaba el color de los textos de los items dropeados [Main.dll]
- Se anadio un bloqueo de maximo tamano de letra en menu de opciones y config.ini [Main.dll]
- Se corrigio un error al cambiar el nombre de la ventana del juego [Main.dll]
- Se mejoro el sistema de ping (creditos ogocx) [Main.dll]
- Se corrigio el dano visual de los staffs [Main.dll]
- Se mejoro el rango visual del minimapa respecto al zoom (Minimapa tiene mayor zoom por defecto) [Main.dll]
- Se mejoro la funcion del protocolo, permitiendo ignorar paquetes en el protocolo original [Main.dll]
- Se anadio sistema de niveles de ingreso para Devil Square y Blood Castle [GameServer.exe][Main.dll][MuServer/Data/Event/DevilSquare.dat][MuServer/Data/Event/BloodCastle.dat]
- Se anadio el sistema de Custom wing [GameServer.exe][Main.dll][Encoder.exe]
- Se anadio una configuracion de rate para que el mix de Alas S2 permita generar Custom Wings [GameServer.exe]
- Revisar puntos por reset cuando grand reset otorga puntos tambien [GameServer.exe]
- Se anadio el sistema de Custom Item Position [Main.dll][Encoder.exe]
- Se corrigio el problema de colision de modelos entre monsters e items [Main.dll]
- Se creo un calculo de experiencia automatico a partir del nivel maximo definido en `GameServerInfo - Common.dat` [GameServer.exe][Main.dll]
- Se anadio un ItemBag para las Box of Kundun +5 y para el puesto 1o del evento DevilSquare [GameServer.exe]

### Update 27:
- Se anadieron opciones al encoder para deshabilitar visualmente resets y grand resets [Encoder/MainInfo.ini]
    - Estos cambios influyen en el texto de la ventana y en la interface C
- Se reconstruyo la interface C y se sincronizaron los valores con el gameserver
- Se corrigio el error que al equipar Angel, no incrementaba el HP +50
- Se corrigio que el mg no podia equiparse 2 staffs con click derecho
- Se cambio que el BonusTime del BonusManager ahora es en minutos (antes en segundos) [MuServer/Data/Event/BonusManager.dat]
- Se cambio que el MaxRegenTime del EventSpawnMonster ahora es en segundos (antes en milisegundos) [MuServer/Data/Event/EventSpawnMonster.dat]
- Se cambio que el InvasionTime del InvasionManager ahora es en minutos (antes en segundos) [MuServer/Data/Event/InvasionManager.dat]
- Se cambio que el RegenTime del InvasionManager ahora es en segundos (antes en milisegundos) [MuServer/Data/Event/InvasionManager.dat]
- Se agrego poder inicializar las invasiones y los bonus desde el menu del GameServer
- Se corrigio la cantidad de dragones que aparecian en las invasiones en los mapas Atlans e Icarus
- Se anadio al menu de opciones antilag la opcion de quitar el HPBar de los monsters [Client/Data/Local/Text_xxx.bmd]
- Se corrigio poder hablar con npc y moverse de mapa si PkLimitFree esta activo
- Se anadio nivel minimo y maximo, reset minimo y maximo, y vip al MoveList
- Se bloqueo el buff de elfa para que solo pueda darse a los miembros del party
- Se anadio una opcion al menu de opciones para activar pvp sin presionar Ctrl [Client/Data/Local/Text_xxx.bmd]
- Se corrigio la resolucion 1366x768 que no sacaba screenshots correctamente
- Se quito la actualizacion de OpenGL hacia la 3.3 debido a reclamos por incompatibilidad con ciertas pc
- Se corrigio la visualizacion de los ataques de la elfa visto desde otro personaje
- Se elimino el SkyDome del juego completamente
- Se reconstruyo el sistema de Camara 3D y se anadio un sistema de FOG
- Se anadio el SellValue al archivo ItemValue para colocar valor de venta [MuServer/Data/Item/ItemValue.txt]
- Se reconstruyo por completo la funcionalidad del Golden Archer, incluyendo un evento propio
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
    - Hace falta anadir una tabla y modificar otra. De ser posible, volver a crear la base de datos con los scripts. Sino se hara un script solo para updatear para aquellos con base de datos en uso.
- Se corrigio un error que al atacar con Twisting Slash y luego tirar Death Stab, el arma rotando era cambiada por otras

## Fixes post-update
- Se corrigio el cierre de la ventana al moverse con el Golden Archer abierto.
- Se anadieron los textos "Este item no te pertenece" al intentar juntar un item de otra persona
- Se corrigio el error al intentar colocar zen superior al propio dentro del trade
- Se corrigio el error al clickear en el NPC de Quest cuando no se cumplen los requisitos de nivel
- Se corrigio un error que al estar utilizando el Twisting, no se podia quitar el arma
- Se expandio el maximo de zen para insertar al trade/warehouse
- Se anadio el mensaje de zen al maximo
- Se anadio el mensaje de zen insuficiente
- Se elimino el log de chaos mix al subir a +11
- Se anadio la opcion Infinity Arrows al Common.dat
- Se corrigio un error en el Kayito Editor que fallaba al cambiar la fecha de Ban y de VIP










