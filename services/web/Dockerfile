FROM node:18-alpine

WORKDIR /app

COPY package.json package-lock.json* ./
RUN npm install --omit=dev

COPY . .

ENV WEB_PORT=8080
EXPOSE 8080

CMD ["npm", "start"]
