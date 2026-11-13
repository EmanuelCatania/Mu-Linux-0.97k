#pragma once

// Lo propio de este server para Common/ServerManager: cuantos GameServer
// admite y que se limpia cuando uno se desconecta.

#define MAX_SERVER 20

void ServerManagerOnDelServer(WORD ServerCode);
