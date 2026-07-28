# Operação local com Docker

## Stack base

O arquivo `compose.yaml` executa:

- `mysql`;
- `mu-server`;
- `mu-web`.

```bash
docker compose config --quiet
docker compose up --build -d
docker compose ps
docker compose logs --follow --tail=200
```

## Editor opcional

```bash
docker compose -f compose.yaml -f compose.editor.yaml up --build -d
```

O overlay cria volumes de dados e backup compartilhados com o servidor. Ele deve ser
usado somente quando o editor for necessário.

## Persistência

O banco utiliza o volume nomeado `mu-097k_mu_mysql`. `docker compose down` preserva o
volume; `docker compose down -v` o remove e não faz parte do fluxo normal.

## Integrações legadas

Arquivos em `deploy/legacy` referenciam imagens upstream e não fazem parte do stack
local suportado. Eles são mantidos apenas para consulta histórica.
