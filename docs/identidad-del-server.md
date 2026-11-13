# Identidad del server

Cada instalación del server tiene una clave de firma propia. Con ella el
GameServer firma la lista de datos que le manda al cliente (el catálogo), y el
cliente la usa para saber que sigue hablando con el mismo server de siempre.

## Dónde está

`Data/Keys/ServerIdentity.key`, dentro del volumen de datos. Se genera sola la
primera vez que arranca el GameServer y sobrevive a las actualizaciones de la
imagen.

El archivo tiene la clave **privada**: no lo compartas ni lo subas a un
repositorio. Quien lo tenga puede hacerse pasar por tu server. Guardá una copia
junto con el resto del respaldo de `Data/`.

La clave **pública** sí se comparte. Sale en el log del GameServer al arrancar:

```
[ServerIdentity] Clave publica: 3f9c...
```

## Qué hace el cliente con ella

- La primera vez que un cliente entra a tu server guarda la clave pública en
  `User/Cache/<server>/server.pub`.
- En cada login verifica que el catálogo venga firmado con esa clave. Si no,
  muestra un aviso y se cierra.
- Si compilás tu propio cliente podés fijarla de antemano en
  `client/src/app/config/ServerConfig.h` (`ServerPublicKey`). Así ni siquiera el
  primer contacto queda sin verificar.

## Cambiar la clave

Hay dos formas, según el caso.

**Rotación (lo normal).** Creá el archivo vacío
`Data/Keys/ServerIdentity.regenerate` y reiniciá el GameServer. Se genera una
clave nueva, la anterior firma el reemplazo y el archivo marcador se borra solo.
Los clientes que ya conocían tu server aceptan la clave nueva sin hacer nada.

```
touch Data/Keys/ServerIdentity.regenerate
docker compose restart mu-server
```

**Empezar de cero.** Si borrás `ServerIdentity.key`, el server genera otra sin
relación con la anterior. Los clientes que tenían la vieja la van a rechazar
hasta que borren su `User/Cache/<server>/server.pub`. Usalo sólo si perdiste la
clave o creés que alguien más la tiene: en ese caso una rotación no sirve,
porque quien tenga la clave vieja también podría firmar un reemplazo.

## Si se pierde `Data/`

Restaurá `Data/Keys/ServerIdentity.key` del respaldo antes de arrancar. Si no lo
tenés, el server va a generar una clave nueva y vale lo de "empezar de cero".
