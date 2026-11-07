FROM mysql:8.4.11

COPY MuServer/MySQL/DB/CreateDatabase.sql /docker-entrypoint-initdb.d/01-create.sql
COPY MuServer/MySQL/DB/Procedures.sql /docker-entrypoint-initdb.d/01-procedures.sql
COPY docker/mysql/02-populate.sh /docker-entrypoint-initdb.d/02-populate.sh
COPY MuServer/MySQL/DB/PoblateDatabase.sql /docker-entrypoint-initdb.d/seed/PoblateDatabase.sql

RUN chmod +x /docker-entrypoint-initdb.d/02-populate.sh
