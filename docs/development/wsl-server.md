# Servidor no WSL2

Mantenha um clone Linux em `~/Dev/projects/mu-097k`. Não compile ou edite o mesmo
working tree simultaneamente pelo Windows e pelo WSL.

## Preparação

```bash
cd ~/Dev/projects/mu-097k
cp .env.example .env
```

Revise todas as credenciais locais. Para o cliente Windows conectado ao WSL2:

```dotenv
PUBLIC_IP=127.0.0.1
```

## Execução

```bash
docker compose config --quiet
docker compose up --build -d
docker compose ps
```

Use `docker compose down` para desligar sem excluir o volume MySQL. Nunca use `-v`
sem confirmar que os dados persistentes podem ser descartados.

Sincronize alterações entre os clones exclusivamente pelo Git e pelo remoto `origin`.
