#!/bin/sh
# Lanzador local del cliente Linux (no se sube al repo).
# Build x64 (OpenGL + WebGPU/Vulkan), compilado en el SSD:
#   cmake --build ~/mu97k-build/linux-x64 --parallel "$(nproc)"
# y usa los assets, server.cfg y Config.ini de esta carpeta.
cd "$(dirname "$0")" || exit 1
export MU_CLIENT_ROOT="$PWD" MU_ASSET_ROOT="$PWD"
exec "$HOME/mu97k-build/linux-x64/bin/main" "$@"
