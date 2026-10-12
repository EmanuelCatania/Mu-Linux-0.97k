#!/bin/bash
set -euo pipefail

echo "MU Linux server 0.97.20 | Cliente compatible: https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.20"

DB_HOST=${DB_HOST:-mysql}
DB_PORT=${DB_PORT:-3306}
DB_USER=${DB_USER:-admin}
DB_PASS=${DB_PASS:-asd123}
DB_NAME=${DB_NAME:-MuOnline97}
PUBLIC_IP=${PUBLIC_IP:-}
MU_ROOT=${MU_ROOT:-/home/container}
children=()

apply_db_settings() {
  local file="$1"
  [ -f "$file" ] || return 0
  sed -i -E "s#^DataBaseHost=.*#DataBaseHost=tcp://${DB_HOST}#" "$file"
  sed -i -E "s#^DataBasePort=.*#DataBasePort=${DB_PORT}#" "$file"
  sed -i -E "s#^DataBaseUser=.*#DataBaseUser=${DB_USER}#" "$file"
  sed -i -E "s#^DataBasePass=.*#DataBasePass=${DB_PASS}#" "$file"
  sed -i -E "s#^DataBaseName=.*#DataBaseName=${DB_NAME}#" "$file"
}

shutdown() {
  local status=${1:-0}
  trap - INT TERM
  echo "Stopping MU services..."
  ((${#children[@]})) && kill -TERM "${children[@]}" 2>/dev/null || true
  wait "${children[@]}" 2>/dev/null || true
  exit "$status"
}

trap 'shutdown 0' INT TERM

apply_db_settings "$MU_ROOT/JoinServer/JoinServer.ini"
apply_db_settings "$MU_ROOT/DataServer/DataServer.ini"

if [ -n "$PUBLIC_IP" ] && [ -f "$MU_ROOT/ConnectServer/ServerList.dat" ]; then
  sed -i -E "s/\"([0-9]{1,3}\.){3}[0-9]{1,3}\"/\"${PUBLIC_IP}\"/g" "$MU_ROOT/ConnectServer/ServerList.dat"
fi

echo "Waiting for MySQL at ${DB_HOST}:${DB_PORT}..."
until (echo >"/dev/tcp/${DB_HOST}/${DB_PORT}") >/dev/null 2>&1; do
  sleep 1
done

export MU_DATA_PATH="$MU_ROOT/Data/"

start_service() {
  local directory="$1" binary="$2"
  (
    cd "$MU_ROOT/$directory"
    exec "./$binary"
  ) &
  children+=("$!")
}

start_service ConnectServer ConnectServer
start_service JoinServer JoinServer
start_service DataServer DataServer
start_service GameServer GameServer

echo "GameServer started"
status=0
wait -n "${children[@]}" || status=$?
shutdown "$status"
