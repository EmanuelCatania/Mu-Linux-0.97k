# Migración de la base a utf8mb4 (0.97.40)

Desde 0.97.40 la base `MuOnline97` usa `utf8mb4` con la collation `utf8mb4_0900_as_ci`:

- guarda cualquier escritura (acentos, cirílico, chino, japonés, árabe…);
- distingue acentos y no mayúsculas: `Añañe` = `añañe`, pero `añañe` ≠ `anane`.

Una instalación nueva ya nace así (`CreateDatabase.sql` + `Procedures.sql`). Una base existente hay que migrarla una vez. Si se creó con MySQL 5.7 (o se restauró desde un dump de 5.7), sus tablas están en `latin1`. En MySQL `latin1` es Windows-1252, así que la conversión conserva los acentos.

Mientras el protocolo con el cliente siga en 1252, el server convierte en el borde con la base (`QueryManager`): lo que no existe en 1252 se guarda y se manda como `?`.

## 1. Backup

Con el stack levantado:

```bash
mkdir -p backups
docker compose exec -T mysql sh -c 'mysqldump -uroot -p"$MYSQL_ROOT_PASSWORD" --single-transaction --routines --triggers --events "$MYSQL_DATABASE"' > backups/muonline97-before-utf8mb4.sql
ls -lh backups/muonline97-before-utf8mb4.sql
```

## 2. Lista de nombres antes

```bash
docker compose exec -T mysql sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD" -N --default-character-set=utf8mb4 "$MYSQL_DATABASE" -e "SELECT Name FROM \`Character\` UNION ALL SELECT G_Name FROM Guild UNION ALL SELECT memb___id FROM MEMB_INFO ORDER BY 1"' > backups/nombres-antes.txt
```

## 3. Detener los servers

Con los servers del juego detenidos nadie escribe durante la conversión:

```bash
docker compose stop mu-server mu-web
```

## 4. Migrar

```bash
docker compose exec -T mysql sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD" "$MYSQL_DATABASE"' < MuServer/MySQL/DB/Migrate-utf8mb4.sql
docker compose exec -T mysql sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD" "$MYSQL_DATABASE"' < MuServer/MySQL/DB/Procedures.sql
```

El script verifica primero que ningún nombre de personaje, cuenta o guild se vuelva igual a otro con la collation nueva. Si encuentra alguno, se detiene sin convertir nada y lista los nombres; hay que renombrarlos y volver a ejecutarlo. Al terminar muestra la collation de cada tabla: todas tienen que decir `utf8mb4_0900_as_ci`.

`Procedures.sql` recrea los procedimientos para que sus parámetros también sean `utf8mb4`.

## 5. Verificar

```bash
docker compose exec -T mysql sh -c 'mysql -uroot -p"$MYSQL_ROOT_PASSWORD" -N --default-character-set=utf8mb4 "$MYSQL_DATABASE" -e "SELECT Name FROM \`Character\` UNION ALL SELECT G_Name FROM Guild UNION ALL SELECT memb___id FROM MEMB_INFO ORDER BY 1"' > backups/nombres-despues.txt
diff backups/nombres-antes.txt backups/nombres-despues.txt && echo "Nombres iguales"
```

## 6. Levantar con la versión nueva

```bash
docker compose up -d --build
```

## Volver atrás

Restaurar el backup del paso 1 sobre una base vacía, como en `docs/mysql-8.4-upgrade.md`, y volver a la imagen anterior del server.
