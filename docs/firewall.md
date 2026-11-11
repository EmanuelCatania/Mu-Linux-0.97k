# Firewall del host (nftables)

El server ya se defiende solo de lo más común:

- `MaxIpConnection` (10): conexiones abiertas por IP, en el ConnectServer y en el GameServer.
- `PacketRateCheck` / `PacketRateMax` (60 por segundo): paquetes por conexión, en el GameServer.
- Una conexión que no se loguea en 30 s, o que deja de responder 60 s, se corta.

Todo eso pasa cuando la conexión ya llegó al proceso. El firewall del host frena
antes, sin costo para el server:

- **Conexiones nuevas por segundo por IP.** Abrir y cerrar conexiones en loop no
  se nota en `MaxIpConnection`, que sólo cuenta las abiertas.
- **Conexiones abiertas por IP.** El mismo tope que `MaxIpConnection`, pero antes
  de que la conexión ocupe un lugar en el server.

Lo que sigue no frena un ataque que sature el ancho de banda: eso se resuelve en
el proveedor (protección DDoS del hosting).

## Reglas

Guardalas en `/etc/nftables.d/mu-linux.nft` (o donde las cargue tu sistema) y
aplicalas con `sudo nft -f /etc/nftables.d/mu-linux.nft`.

```nft
#!/usr/sbin/nft -f
# Límites de conexión para el ConnectServer (44405) y el GameServer (55901).

table inet mu_linux
delete table inet mu_linux

table inet mu_linux {
	set ports {
		type inet_service
		elements = { 44405, 55901 }
	}

	# Conexiones abiertas por IP (ct count necesita un set sin timeout).
	set open4 { type ipv4_addr; flags dynamic; }
	set open6 { type ipv6_addr; flags dynamic; }

	# Conexiones nuevas por segundo por IP.
	set rate4 { type ipv4_addr; flags dynamic; timeout 1m; }
	set rate6 { type ipv6_addr; flags dynamic; timeout 1m; }

	chain limits {
		tcp dport != @ports return
		ct state != new return

		add @open4 { ip saddr ct count over 10 } counter reject with tcp reset
		add @open6 { ip6 saddr ct count over 10 } counter reject with tcp reset

		update @rate4 { ip saddr limit rate over 5/second burst 10 packets } counter drop
		update @rate6 { ip6 saddr limit rate over 5/second burst 10 packets } counter drop
	}

	# Server instalado directo en el host.
	chain input {
		type filter hook input priority filter - 10; policy accept;
		jump limits
	}

	# Server en Docker: los puertos publicados pasan por forward (Docker hace
	# DNAT antes), no por input.
	chain forward {
		type filter hook forward priority filter - 10; policy accept;
		jump limits
	}
}
```

El `table` + `delete table` del principio hace que el archivo se pueda volver a
cargar sin duplicar las reglas.

Cuidado con los valores:

- **10 conexiones abiertas por IP.** Varios jugadores detrás de la misma IP (un
  cibercafé, una casa, una red con CGNAT) comparten el tope. Si es tu caso,
  subilo junto con `MaxIpConnection`.
- **5 conexiones nuevas por segundo, con ráfagas de 10.** Entrar al juego abre una
  conexión al ConnectServer y otra al GameServer, y cambiar de personaje o de
  server abre más. Eso entra con margen.
- **Puertos.** Si en el compose publicaste el server en otros puertos del host
  (por ejemplo `"7000:55901"`), en la cadena `forward` el puerto ya es el del
  contenedor (55901), pero en `input` es el del host. Poné los dos en `ports`.

## Ver qué está frenando

```bash
sudo nft list table inet mu_linux        # contadores de cada regla
sudo nft list set inet mu_linux rate4    # IPs que conectaron en el último minuto
```

Para sacar las reglas: `sudo nft delete table inet mu_linux`.

## Windows

Sin Docker y en Windows no hay equivalente directo a `ct count` en el firewall
del sistema. Quedan los límites del server (`MaxIpConnection` y
`PacketRateCheck`). El viejo `script anti ddos.txt` (reglas `netsh` con IPs
fijas) no sirve: una lista fija no frena un ataque nuevo. Para bloquear IPs
puntuales está `Data/Util/Blacklist.dat`.
