FROM node:18-alpine

LABEL org.opencontainers.image.version="0.97.20" org.opencontainers.image.source="https://github.com/EmanuelCatania/Mu-Linux-0.97k"

WORKDIR /app

ARG INSTALL_GEOIP=0
COPY package.json package-lock.json* ./
RUN if [ "$INSTALL_GEOIP" = "1" ]; then npm install --omit=dev; else npm install --omit=dev --omit=optional; fi

COPY . .

ENV WEB_PORT=8080
EXPOSE 8080

CMD ["npm", "start"]
