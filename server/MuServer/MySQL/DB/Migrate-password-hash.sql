-- Contraseñas con bcrypt: memb__pwd pasa de 16 a 60 bytes.
--
-- Agrandar la columna no toca lo guardado: las contraseñas viejas (texto o
-- MD5) siguen entrando, y el JoinServer las reemplaza por el hash bcrypt la
-- primera vez que cada cuenta entra (PasswordHash=1 en JoinServer.ini).
--
-- Sin esta migración el JoinServer no reemplaza nada: un hash de 60 caracteres
-- en una columna de 16 quedaría cortado y la cuenta no podría entrar.
-- El panel web la aplica solo al arrancar.

USE MuOnline97;

ALTER TABLE `MEMB_INFO` MODIFY `memb__pwd` VARBINARY(60) NOT NULL;
