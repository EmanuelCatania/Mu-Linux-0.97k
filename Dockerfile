FROM ubuntu:22.04 AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libmysqlcppconn-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY Source/MuServer /src/Source/MuServer
WORKDIR /src/Source/MuServer

RUN cmake -S . -B /build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build -j

FROM ubuntu:22.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        netcat-openbsd \
    && (apt-get install -y --no-install-recommends libmysqlcppconn7v5 \
        || apt-get install -y --no-install-recommends libmysqlcppconn8-2) \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/mu

COPY --from=build /build/bin/ConnectServer /opt/mu/ConnectServer/ConnectServer
COPY --from=build /build/bin/JoinServer /opt/mu/JoinServer/JoinServer
COPY --from=build /build/bin/DataServer /opt/mu/DataServer/DataServer
COPY --from=build /build/bin/GameServer /opt/mu/GameServer/GameServer

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
    && mv /opt/mu/GameServer/DATA /opt/mu/GameServer/Data

EXPOSE 44405/tcp 55601/udp 55901/tcp

ENTRYPOINT ["/opt/mu/entrypoint.sh"]
