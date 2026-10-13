FROM node:20-alpine

LABEL org.opencontainers.image.version="0.97.20" org.opencontainers.image.source="https://github.com/EmanuelCatania/Mu-Linux-0.97k"

WORKDIR /app

COPY package.json package-lock.json* ./
RUN npm install --omit=dev

COPY server.js ./

ENV PORT=8090
EXPOSE 8090

CMD ["npm", "start"]
