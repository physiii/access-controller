FROM node:20-slim AS base

WORKDIR /app

ENV NODE_ENV=production \
    npm_config_update_notifier=false \
    npm_config_fund=false

COPY package*.json ./

RUN npm ci --omit=dev && npm cache clean --force

COPY src ./src

USER node

EXPOSE 9000 9001

CMD ["node", "src/server.js"]


