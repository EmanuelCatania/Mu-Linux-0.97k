# MU 0.97k

[Short English overview](README.en.md)

Fork independente do MU Online 0.97k voltado à evolução do jogo e de sua base
técnica. O projeto busca manter a identidade clássica, adicionar conteúdo e
qualidade de vida e tornar o desenvolvimento mais previsível no Windows e no Linux.

> [!IMPORTANT]
> O cliente original, as marcas e os assets de MU Online pertencem à Webzen. O
> código comunitário deste repositório possui origens e condições de licenciamento
> distintas. Leia o [aviso de proveniência e licenciamento](NOTICE.md) antes de
> reutilizar ou redistribuir qualquer material.

## Visão do projeto

O trabalho segue três pilares com a mesma importância:

- **Gameplay e conteúdo:** correções, qualidade de vida, eventos, itens, mapas e
  novos sistemas.
- **Identidade clássica:** a experiência 0.97k permanece reconhecível; mudanças que
  alterem essa experiência devem ser configuráveis sempre que viável.
- **Plataforma de desenvolvimento:** builds reproduzíveis, automação, testes,
  ferramentas e manutenção mais simples.

Esse modelo é chamado de **clássico extensível**: a base histórica é preservada como
referência, mas não impede a evolução independente do jogo.

## Como o projeto funciona

O `main.exe` é o cliente fechado original e seu código-fonte não está disponível. A
`Main.dll` é um plugin comunitário carregado pelo cliente que aplica hooks, correções
e extensões. O `InfoEncoder.exe` transforma `MainInfo.ini` e os arquivos auxiliares
em `ClientInfo.bmd`, consumido pela DLL.

O servidor é uma emulação comunitária em C++, derivada do trabalho de Kayito e
posteriormente adaptada por Emanuel Catania para execução nativa no Linux, MySQL e
Docker. Cliente, DLL, encoder e servidor precisam evoluir de forma coordenada quando
uma funcionalidade altera protocolo ou configuração.

## Estado atual

| Componente | Ambiente | Local |
| --- | --- | --- |
| Cliente, `Main.dll` e encoder | Windows Win32 | `src/client` |
| Servidor C++ | Linux/WSL2 | `src/server` |
| Templates e dados de runtime | Windows/Linux | `runtime` |
| Painel web | Node.js/Docker | `services/web` |
| Editor opcional | Node.js/Docker | `services/editor` |

O ambiente suportado atualmente é Windows 11 com VS Code e um clone separado no
WSL2 Ubuntu 24.04. A publicação de binários e imagens ainda não faz parte do fluxo
oficial.

## Requisitos principais

- PowerShell 7, Git e VS Code no Windows;
- Visual Studio Build Tools com C++ e MSVC compatível com o projeto;
- Ubuntu 24.04, Docker Engine e Docker Compose no WSL2;
- clones Windows e WSL separados e sincronizados pelo Git.

## Início rápido do servidor

No clone do WSL2:

```bash
cp .env.example .env
# Revise as credenciais e mantenha PUBLIC_IP=127.0.0.1 no ambiente local.
docker compose config --quiet
docker compose up --build -d
```

O servidor expõe `44405/tcp` e `55901/tcp`; o painel fica disponível em
<http://127.0.0.1:8085>. Para desligar sem remover o banco, use
`docker compose down`.

## Início rápido do cliente

No clone Windows:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

Configure `C:\Dev\runtime\mu-097k\encoder\MainInfo.ini` e execute:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action Encode
```

O cliente executável fica em `C:\Dev\runtime\mu-097k\client`. O script também
oferece `Build`, `Deploy`, `Clean` e builds Release. No VS Code, a configuração
`Client: Debug Main.dll (x86)` prepara o runtime e inicia o debugger.

## Estrutura

```text
src/          Código C++ do cliente, servidor e ferramentas
runtime/      Templates do cliente/encoder e dados do servidor
services/     Painel web e editor opcional
deploy/       Dockerfiles e integrações legadas não suportadas
scripts/      Automação do ambiente local
docs/         Guia de desenvolvimento e histórico upstream
```

Consulte o [guia de desenvolvimento](docs/development.md) para arquitetura, runtime,
build, debug e operação local. Veja também o [changelog](CHANGELOG.md), as
[diretrizes de contribuição](CONTRIBUTING.md) e a [política de segurança](SECURITY.md).

## Direção

A trilha **Gameplay e Conteúdo** cobre correções funcionais, qualidade de vida,
opções configuráveis, itens, mapas, eventos e sistemas. A trilha **Plataforma e
Desenvolvimento** cobre CI, testes, dependências, segurança, CMake/Ninja, vcpkg e
ferramentas. O backlog detalhado será mantido nos Issues e Milestones do GitHub.

## Créditos

- [Nico Muratona (Kayito)](https://github.com/nicomuratona/MuEmu-0.97k-kayito):
  sources e ferramentas-base do MuEmu 0.97k.
- [Emanuel Catania](https://github.com/EmanuelCatania/Mu-Linux-0.97k): linha Linux,
  Docker, MySQL e repositório que originou este fork.
- **Trifon Dinev:** template web Simple MU Online Templates.
- **Kapocha33, SetecSoft, Zeus e ogocx:** contribuições específicas registradas no
  [histórico upstream](docs/history/upstream-readme.es.md).
- [Aldo Migge](https://github.com/aldomigge): manutenção da linha independente atual.

## Propriedade e licenciamento

MU Online, seu cliente original, nomes, marcas, músicas, imagens, sons e demais
assets pertencem à Webzen e/ou aos respectivos titulares. Esses materiais não devem
ser confundidos com o código do servidor emulado e com as extensões comunitárias.

Não foi identificada uma licença que abranja todo o conteúdo legado. Este projeto não
é afiliado nem endossado pela Webzen e não concede direitos sobre material de
terceiros. Consulte [NOTICE.md](NOTICE.md) para detalhes e atribuições.
