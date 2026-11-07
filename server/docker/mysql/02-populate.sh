#!/bin/sh
set -e

seed_flag="${SEED_TEST_DATA:-0}"
should_seed=false

case "$seed_flag" in
  1|true|TRUE|yes|YES|on|ON)
    echo "Seeding test data (SEED_TEST_DATA=$seed_flag)"
    should_seed=true
    ;;
  *)
    echo "Skipping test data (SEED_TEST_DATA=$seed_flag)"
    ;;
esac

if [ "$should_seed" = true ]; then
  if [ -z "${MYSQL_ROOT_PASSWORD}" ]; then
    mysql -uroot "${MYSQL_DATABASE}" < /docker-entrypoint-initdb.d/seed/PoblateDatabase.sql
  else
    mysql -uroot -p"${MYSQL_ROOT_PASSWORD}" "${MYSQL_DATABASE}" < /docker-entrypoint-initdb.d/seed/PoblateDatabase.sql
  fi
fi
