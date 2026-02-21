FROM mysql:5.7

COPY MuServer/MySQL/DB/CreateDatabase.sql /docker-entrypoint-initdb.d/01-create.sql
COPY docker/mysql/02-populate.sh /docker-entrypoint-initdb.d/02-populate.sh
COPY MuServer/MySQL/DB/PoblateDatabase.sql /docker-entrypoint-initdb.d/seed/PoblateDatabase.sql

RUN chmod +x /docker-entrypoint-initdb.d/02-populate.sh
