#!/usr/bin/env bash
set -euo pipefail

echo "MU Linux server 0.97.43 | Cliente compatible: https://github.com/EmanuelCatania/Mu-Linux/releases/tag/0.97.43"

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

# Traza de paquetes: MU_TRACE_ENABLED=1 deja el anillo disponible (se mira con
# `mu-trace tail`); MU_TRACE_RECORD=1 además lo graba a archivos rotativos.
# El grabador no entra en el `wait -n`: si se cae, el server sigue.
if [ "${MU_TRACE_ENABLED:-0}" = "1" ] && [ "${MU_TRACE_RECORD:-0}" = "1" ]; then
  (
    for _ in $(seq 1 60); do
      ls /dev/shm/mu-trace-* >/dev/null 2>&1 && break
      sleep 1
    done
    exec /opt/mu/mu-trace record --out "${MU_TRACE_DIR:-/opt/mu/GameServer/LOG/trace}" --keep-mb "${MU_TRACE_KEEP_MB:-1024}"
  ) &
  disown $!
fi

wait -n
