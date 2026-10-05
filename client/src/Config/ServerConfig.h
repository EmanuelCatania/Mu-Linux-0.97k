#pragma once
// ServerConfig.h — conexión e identidad del server, compiladas en el cliente.
//
// Como en MU 5.2 (ZzzScene.cpp / WSclient.cpp), la dirección y la identidad del
// server van en el source: no hay archivo de configuración que distribuir ni que
// pueda editarse por fuera del build. Para apuntar a otro server se editan los
// valores de abajo y se recompila.
//
// La dirección puede ser una IP o un nombre de host: Net_Connect prueba
// inet_addr y, si no es una IP, resuelve con gethostbyname, igual que el binario
// original (que trae de fábrica connect.muonline.co.kr y otros, en 0x558ED8).
// Usar un nombre evita dejar una IP pública fija en el código.
//
// Los tres valores de identidad tienen que coincidir con
// `MuServer/GameServer/DATA/GameServerInfo - StartUp.dat` del GameServer:
//   - CustomerName y ServerSerial derivan la clave de encriptación
//     (GameServer/HackCheck.cpp::InitHackCheck). Si no coinciden, el cliente
//     conecta pero se queda en "conectando al GameServer".
//   - ServerSerial y ClientVersion además se comparan en el login F1/01; si no
//     coinciden, el server responde "versión incorrecta".

namespace ServerConfig {

// ConnectServer: lista de servers y ocupación (F4/02), redirect al GameServer
// (F4/03). Con puerto 0 no se usa: el cliente conecta directo al GameServer y el
// select-server muestra una entrada fija.
constexpr char           ConnectServerIP[]   = "mu.server-pups.space";
constexpr unsigned short ConnectServerPort   = 44405;

// GameServer: destino directo cuando no hay ConnectServer, y fallback si el
// ConnectServer no responde. Con ConnectServer, el GameServer al que se salta lo
// manda el server en el redirect F4/03 (sale de su ServerList.dat).
constexpr char           GameServerIP[]      = "mu.server-pups.space";
constexpr unsigned short GameServerPort      = 55901;

// Identidad del server (ver arriba). ClientVersion acepta "0.97.11" o "09711".
constexpr char           CustomerName[]      = "MuLinux";
constexpr char           ServerSerial[]      = "TbYehR2hFUPBKgZj";
constexpr char           ClientVersion[]     = "0.97.11";

} // namespace ServerConfig
