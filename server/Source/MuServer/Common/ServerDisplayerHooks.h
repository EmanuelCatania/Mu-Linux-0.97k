#pragma once

// Lo propio de cada server para Common/ServerDisplayer. Cada uno las define
// en su ServerDisplayerServer.cpp.

// Registra los logs del server, en el orden de su eLogType.
void ServerDisplayerAddLogs();

#ifdef _WIN32

// Titulo de la ventana (buff de 256).
void ServerDisplayerWindowName(char* buff);

// true = cartel "ACTIVE MODE"; false = "STANDBY MODE".
bool ServerDisplayerIsActive();

#endif
