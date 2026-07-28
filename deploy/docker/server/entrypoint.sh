#!/usr/bin/env bash
set -euo pipefail

DB_HOST=${DB_HOST:-mysql}
DB_PORT=${DB_PORT:-3306}
DB_USER=${DB_USER:-admin}
DB_PASS=${DB_PASS:-asd123}
DB_NAME=${DB_NAME:-MuOnline97}
PUBLIC_IP=${PUBLIC_IP:-}
MU_DATA_PATH=${MU_DATA_PATH:-/opt/mu/Data/}

apply_db_settings() {
  local file="$1"
  if [ -f "$file" ]; then
    sed -i -E "s#^DataBaseHost=.*#DataBaseHost=tcp://${DB_HOST}#" "$file"
    sed -i -E "s#^DataBasePort=.*#DataBasePort=${DB_PORT}#" "$file"
    sed -i -E "s#^DataBaseUser=.*#DataBaseUser=${DB_USER}#" "$file"
    sed -i -E "s#^DataBasePass=.*#DataBasePass=${DB_PASS}#" "$file"
    sed -i -E "s#^DataBaseName=.*#DataBaseName=${DB_NAME}#" "$file"
  fi
}

apply_db_settings /opt/mu/JoinServer/JoinServer.ini
apply_db_settings /opt/mu/DataServer/DataServer.ini

if [ -n "$PUBLIC_IP" ] && [ -f /opt/mu/ConnectServer/ServerList.dat ]; then
  sed -i -E "s/\"([0-9]{1,3}\.){3}[0-9]{1,3}\"/\"${PUBLIC_IP}\"/g" /opt/mu/ConnectServer/ServerList.dat
fi

if command -v nc >/dev/null 2>&1; then
  echo "Waiting for MySQL at ${DB_HOST}:${DB_PORT}..."
  until nc -z "$DB_HOST" "$DB_PORT"; do
    sleep 1
  done
fi

export MU_DATA_PATH

cd /opt/mu/ConnectServer
./ConnectServer &

cd /opt/mu/JoinServer
./JoinServer &

cd /opt/mu/DataServer
./DataServer &

cd /opt/mu/GameServer
./GameServer &

wait -n
