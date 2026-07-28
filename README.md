# MU 0.97k

[English version](README.en.md)

Preservação e modernização independente do MU Online 0.97k, com cliente Windows,
servidor C++ para Linux e ambiente local baseado em WSL2 e Docker Compose.

> [!IMPORTANT]
> Este repositório contém código e assets legados cuja licença original não foi
> identificada. Consulte [NOTICE.md](NOTICE.md) e
> [docs/provenance.md](docs/provenance.md) antes de redistribuir ou reutilizar o
> conteúdo.

## Estado do projeto

- Cliente: Windows, Win32, Visual Studio Build Tools e MSVC.
- Servidor: Linux, CMake e Docker.
- Desenvolvimento suportado: Windows 11 com WSL2 Ubuntu 24.04.
- Objetivo atual: preservar a compatibilidade 0.97k enquanto o build, a estrutura e
  a experiência de desenvolvimento são modernizados.
- Distribuição pública de binários e imagens: ainda não suportada.

## Componentes

| Componente | Local | Execução |
| --- | --- | --- |
| Cliente e encoder | `src/client` | Windows |
| Servidor | `src/server` | Linux/WSL2 |
| Runtime do cliente | `runtime/client` | Template rastreado |
| Dados do servidor | `runtime/server` | Docker |
| Painel web | `services/web` | Node.js/Docker |
| Editor opcional | `services/editor` | Node.js/Docker |

## Requisitos

No Windows:

- PowerShell 7;
- Git e VS Code;
- Visual Studio Build Tools 2026 com Desktop C++, toolset v145 e MSVC 14.44;
- extensões recomendadas pelo workspace.

No WSL2:

- Ubuntu 24.04;
- Docker Engine e Docker Compose;
- clone Linux separado do clone Windows.

## Servidor local

No clone do WSL2:

```bash
cp .env.example .env
# Revise senhas e mantenha PUBLIC_IP=127.0.0.1 para desenvolvimento local.
docker compose config --quiet
docker compose up --build -d
```

Serviços expostos:

- ConnectServer: `127.0.0.1:44405/tcp`;
- GameServer: `127.0.0.1:55901/tcp`;
- painel web: <http://127.0.0.1:8085>.

Para desligar sem apagar o banco:

```bash
docker compose down
```

## Cliente Windows

No primeiro uso, a partir do clone Windows:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

O runtime executável é criado fora do Git em:

```text
C:\Dev\runtime\mu-097k\client
```

Execute o cliente preservando o diretório de trabalho:

```powershell
Start-Process `
    -FilePath "C:\Dev\runtime\mu-097k\client\main.exe" `
    -WorkingDirectory "C:\Dev\runtime\mu-097k\client"
```

O script também oferece `Build`, `Deploy`, `Encode`, `Clean` e builds Release. No
VS Code, use as tarefas `Client:*` no clone Windows e `Server:*` no clone WSL.

## Estrutura

```text
src/          Código C++ do cliente, servidor e ferramentas
runtime/      Templates do cliente/encoder e dados do servidor
services/     Painel web e editor opcional
deploy/       Docker e integrações legadas
docs/         Arquitetura, desenvolvimento, operação e histórico
scripts/      Automação do ambiente local
```

## Documentação

- [Arquitetura](docs/architecture.md)
- [Cliente no Windows](docs/development/windows-client.md)
- [Servidor no WSL2](docs/development/wsl-server.md)
- [Operação com Docker](docs/operations/docker.md)
- [Proveniência e atribuições](docs/provenance.md)
- [Histórico upstream em espanhol](docs/history/upstream-readme.es.md)
- [Como contribuir](CONTRIBUTING.md)
- [Política de segurança](SECURITY.md)

## Manutenção

`main` é protegida. Mudanças entram por branches curtas e pull requests neste
repositório. O remoto upstream existe somente para consulta; correções externas são
avaliadas e importadas manualmente.

## Aviso

Este projeto não é afiliado nem endossado pelos proprietários de MU Online. Nomes,
marcas e assets permanecem pertencentes aos respectivos titulares.
