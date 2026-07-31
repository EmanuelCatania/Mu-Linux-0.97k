# MU 0.97k

[English overview](README.en.md)

Fork independente de MU Online 0.97k. O objetivo é evoluir o jogo sem perder a
identidade clássica e, ao mesmo tempo, deixar o desenvolvimento mais simples de
compilar, testar e depurar.

## O que este projeto busca

- **Gameplay e conteúdo:** correções, qualidade de vida, eventos, itens, mapas e
  sistemas novos.
- **Identidade clássica:** mudanças que alterem a experiência original devem ser
  configuráveis quando isso for viável.
- **Desenvolvimento:** builds reproduzíveis, ferramentas e documentação que não
  dependam de conhecimento escondido.

## Componentes

O `main.exe` é o cliente original fechado. Como seu código-fonte não está disponível,
o projeto usa `Main.dll`, um plugin comunitário que aplica hooks e extensões. O
`InfoEncoder.exe` gera o `ClientInfo.bmd` usado pela DLL.

O servidor é uma emulação comunitária em C++ executada no Linux. A linha atual usa
WSL2, Docker, MySQL e um painel web. Cliente, plugin, encoder e servidor precisam ser
alterados em conjunto quando uma funcionalidade muda protocolo ou configuração.

## Estado e ambientes

- Cliente, `Main.dll` e encoder: Windows 11, Win32, MSVC e VS Code.
- Servidor: Ubuntu 24.04 no WSL2, CMake/Ninja e Docker Compose.
- Painel web: Node.js e Docker.
- O runtime de execução fica fora do repositório, em `C:\Dev\runtime\mu-097k`.
- Não há distribuição oficial de binários ou imagens neste momento.

## Começo rápido

No clone do WSL2:

```bash
cp .env.example .env
# Revise as credenciais e mantenha PUBLIC_IP=127.0.0.1 para uso local.
docker compose config --quiet
docker compose up --build -d
```

O painel fica em <http://127.0.0.1:8085>. Para parar os serviços sem remover o banco,
use `docker compose down`.

No clone do Windows:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
```

Edite `C:\Dev\runtime\mu-097k\encoder\MainInfo.ini` e depois execute:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

`BuildDeploy` compila, copia os artefatos e executa o encoder. O guia de
[desenvolvimento](docs/development.md) explica Release, F5, MSBuild, CMake e os
testes do servidor.

## Estrutura

```text
src/       código do cliente, servidor e ferramentas
runtime/   templates rastreados e dados do servidor
services/  painel web e editor opcional
deploy/    Dockerfiles e integrações legadas
scripts/   automação local
docs/      desenvolvimento e histórico upstream
```

Veja também as [regras de contribuição](CONTRIBUTING.md), a [política de
segurança](SECURITY.md) e o [aviso legal e de proveniência](NOTICE.md).

## Créditos

- [Nico Muratona (Kayito)](https://github.com/nicomuratona/MuEmu-0.97k-kayito):
  sources e ferramentas-base do MuEmu 0.97k.
- [Emanuel Catania](https://github.com/EmanuelCatania/Mu-Linux-0.97k): linha Linux,
  Docker, MySQL e o repositório que originou este fork.
- **Trifon Dinev:** template web Simple MU Online Templates.
- **Kapocha33, SetecSoft, Zeus e ogocx:** contribuições registradas no
  [README histórico](docs/history/upstream-readme.es.md).
- [Aldo Migge](https://github.com/aldomigge): mantenedor da linha independente atual.

MU Online, o cliente original, marcas, músicas, imagens e demais assets pertencem à
Webzen e/ou aos respectivos titulares. Este projeto não é afiliado nem endossado pela
Webzen. A situação de licenciamento do material legado está descrita em
[`NOTICE.md`](NOTICE.md); nenhum direito de terceiros é concedido aqui.
