FROM ubuntu:22.04 AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY Source/MuServer /src/Source/MuServer
COPY cmake /src/cmake
WORKDIR /src/Source/MuServer

RUN cmake -S . -B /build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build -j1

FROM ubuntu:22.04

LABEL org.opencontainers.image.version="0.97.42" \
      org.opencontainers.image.source="https://github.com/EmanuelCatania/Mu-Linux" \
      org.opencontainers.image.description="Servidor MU Online 0.97k para Linux; texto UTF-8, traducciones por idioma y MySQL por la API en C (Fase 4)" \
      mu-linux.client-release="https://github.com/EmanuelCatania/Mu-Linux/releases/tag/0.97.42"

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        netcat-openbsd \
        libmysqlclient21 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/mu

COPY --from=build /build/bin/ConnectServer /opt/mu/ConnectServer/ConnectServer
COPY --from=build /build/bin/JoinServer /opt/mu/JoinServer/JoinServer
COPY --from=build /build/bin/DataServer /opt/mu/DataServer/DataServer
COPY --from=build /build/bin/GameServer /opt/mu/GameServer/GameServer
COPY --from=build /build/bin/mu-data /opt/mu/mu-data

COPY MuServer/ConnectServer/ConnectServer.ini /opt/mu/ConnectServer/
COPY MuServer/ConnectServer/ServerList.dat /opt/mu/ConnectServer/

COPY MuServer/MySQL/JoinServer/JoinServer.ini /opt/mu/JoinServer/
COPY MuServer/MySQL/JoinServer/AllowableIpList.txt /opt/mu/JoinServer/

COPY MuServer/MySQL/DataServer/DataServer.ini /opt/mu/DataServer/
COPY MuServer/MySQL/DataServer/AllowableIpList.txt /opt/mu/DataServer/
COPY MuServer/MySQL/DataServer/BadSyntax.txt /opt/mu/DataServer/

COPY MuServer/GameServer/DATA /opt/mu/GameServer/DATA
COPY MuServer/Data /opt/mu/Data

COPY docker/entrypoint.sh /opt/mu/entrypoint.sh

RUN chmod +x /opt/mu/entrypoint.sh \
    /opt/mu/ConnectServer/ConnectServer \
    /opt/mu/JoinServer/JoinServer \
    /opt/mu/DataServer/DataServer \
    /opt/mu/GameServer/GameServer \
    /opt/mu/mu-data \
    && ln -s /opt/mu/mu-data /usr/local/bin/mu-data \
    && mv /opt/mu/GameServer/DATA /opt/mu/GameServer/Data

EXPOSE 44405/tcp 55601/udp 55901/tcp

ENTRYPOINT ["/opt/mu/entrypoint.sh"]
