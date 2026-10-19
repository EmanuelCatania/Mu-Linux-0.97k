#!/bin/sh
set -eu
package_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
asset_root=${MU_ASSET_ROOT:-$package_dir}
asset_root=$(CDPATH= cd -- "$asset_root" && pwd)
if [ ! -d "$asset_root/Data/Font" ]; then
    echo "Falta Data/Font en MU_ASSET_ROOT=$asset_root" >&2
    exit 1
fi
export MU_ASSET_ROOT="$asset_root"
export MU_CLIENT_ROOT="${MU_CLIENT_ROOT:-${XDG_DATA_HOME:-$HOME/.local/share}/mu97k}"
mkdir -p "$MU_CLIENT_ROOT"
MU_CLIENT_ROOT=$(CDPATH= cd -- "$MU_CLIENT_ROOT" && pwd)
export MU_CLIENT_ROOT
if [ -L "$MU_CLIENT_ROOT/Data" ]; then
    if [ "$(readlink "$MU_CLIENT_ROOT/Data")" != "$asset_root/Data" ]; then
        ln -sfn "$asset_root/Data" "$MU_CLIENT_ROOT/Data"
    fi
elif [ ! -e "$MU_CLIENT_ROOT/Data" ]; then
    ln -s "$asset_root/Data" "$MU_CLIENT_ROOT/Data"
elif [ "$(readlink -f "$MU_CLIENT_ROOT/Data")" != "$asset_root/Data" ]; then
    echo "MU_CLIENT_ROOT/Data ya existe y no corresponde a MU_ASSET_ROOT; elegí otro MU_CLIENT_ROOT." >&2
    exit 1
fi
# Crear preferencias sólo en el primer arranque; nunca copiar las del build.
if [ ! -e "$MU_CLIENT_ROOT/Config.ini" ] && [ ! -e "$MU_CLIENT_ROOT/config.ini" ]; then
    printf '[Window]\nWindowMode=1\nBorderless=0\nResolution=2\n' > "$MU_CLIENT_ROOT/Config.ini"
fi
cd "$MU_CLIENT_ROOT"
exec "$package_dir/main" "$@"
