FROM node:18-alpine

LABEL org.opencontainers.image.version="0.97.20" org.opencontainers.image.source="https://github.com/EmanuelCatania/Mu-Linux-0.97k"

WORKDIR /app

COPY package.json package-lock.json* ./
RUN npm install --omit=dev

COPY . .

ENV WEB_PORT=8080
EXPOSE 8080

CMD ["npm", "start"]
