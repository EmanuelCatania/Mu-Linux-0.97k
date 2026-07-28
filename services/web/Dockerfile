FROM node:24.18.0-alpine3.23

WORKDIR /app

COPY package.json package-lock.json ./
RUN npm ci --omit=dev

COPY . .

ENV WEB_PORT=8080
EXPOSE 8080

CMD ["npm", "start"]
