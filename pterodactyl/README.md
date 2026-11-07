# Pterodactyl eggs

The eggs in [`../Pterodactyl eggs`](../Pterodactyl%20eggs) use Pterodactyl's expected layout: the installer downloads server files into `/mnt/server`, which Wings mounts as `/home/container` at runtime. The custom server runtime image contains only OS dependencies and the Pterodactyl entrypoint.

## Release assets

Every release using these eggs includes:

- `Pterodactyl-eggs-0.97.40.zip`
- `mu-linux-97k-server-amd64.tar.gz`
- `mu-linux-97k-server-arm64.tar.gz`
- `mu-linux-97k-web.tar.gz`
- `SHA256SUMS.txt`

Run `./pterodactyl/package-release-assets.sh` on a Linux Docker host after publishing the server image. It creates the architecture-specific runtime archives, web archive and egg ZIP.

`start.sh` handles `SIGINT` and `SIGTERM`, forwarding them to ConnectServer, JoinServer, DataServer and GameServer. The egg's `^C` stop command therefore shuts services down cleanly.
