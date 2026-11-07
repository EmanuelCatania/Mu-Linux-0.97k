FROM ubuntu:22.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        bash \
        ca-certificates \
        libmysqlclient21 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --home-dir /home/container --uid 988 --shell /bin/bash container

COPY pterodactyl/server/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

USER container
WORKDIR /home/container
ENTRYPOINT ["/entrypoint.sh"]
