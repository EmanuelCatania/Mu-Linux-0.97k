#!/bin/sh
set -e

seed_flag="${SEED_TEST_DATA:-0}"

case "$seed_flag" in
  1|true|TRUE|yes|YES|on|ON)
    echo "Seeding test data (SEED_TEST_DATA=$seed_flag)"
    ;;
  *)
    echo "Skipping test data (SEED_TEST_DATA=$seed_flag)"
    exit 0
    ;;
esac

if [ -z "${MYSQL_ROOT_PASSWORD}" ]; then
  mysql -uroot "${MYSQL_DATABASE}" < /docker-entrypoint-initdb.d/seed/PoblateDatabase.sql
else
  mysql -uroot -p"${MYSQL_ROOT_PASSWORD}" "${MYSQL_DATABASE}" < /docker-entrypoint-initdb.d/seed/PoblateDatabase.sql
fi
