#!/bin/bash
set -euo pipefail

# Run from the repository root after publishing the server image for both architectures.
RELEASE_DIR=${RELEASE_DIR:-dist/pterodactyl-release}
SERVER_IMAGE=${SERVER_IMAGE:-emapupi/mu-linux-97k:0.97.20}

if [ "$RELEASE_DIR" != "dist/pterodactyl-release" ]; then
  echo "For safety, RELEASE_DIR must be dist/pterodactyl-release" >&2
  exit 1
fi

rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

package_server() {
  local platform="$1" arch="$2" stage container
  stage=$(mktemp -d)
  container=$(docker create --platform "$platform" --entrypoint /bin/true "$SERVER_IMAGE")
  docker cp "$container:/opt/mu/." "$stage"
  docker rm "$container" >/dev/null
  cp pterodactyl/server/start.sh "$stage/start.sh"
  chmod +x "$stage/start.sh" \
    "$stage/ConnectServer/ConnectServer" \
    "$stage/JoinServer/JoinServer" \
    "$stage/DataServer/DataServer" \
    "$stage/GameServer/GameServer"
  tar -C "$stage" -czf "$RELEASE_DIR/mu-linux-97k-server-${arch}.tar.gz" .
  rm -rf "$stage"
}

package_server linux/amd64 amd64
package_server linux/arm64 arm64
tar --exclude='node_modules' --exclude='.env' -C web -czf "$RELEASE_DIR/mu-linux-97k-web.tar.gz" .
python3 - "$RELEASE_DIR/Pterodactyl-eggs-v0.2.0.zip" 'Pterodactyl eggs' <<'PY'
import pathlib
import sys
import zipfile

destination = pathlib.Path(sys.argv[1])
source = pathlib.Path(sys.argv[2])
with zipfile.ZipFile(destination, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for file in sorted(source.rglob("*")):
        if file.is_file():
            archive.write(file, file.relative_to(source))
PY
(
  cd "$RELEASE_DIR"
  sha256sum ./* > SHA256SUMS.txt
)
