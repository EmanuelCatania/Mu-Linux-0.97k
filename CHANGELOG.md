# Changelog

Todas as mudanças relevantes da linha independente MU 0.97k serão registradas aqui.
O histórico anterior permanece em `docs/history/`.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/). As
versões deste fork são independentes da versão de compatibilidade MU 0.97k e usam
tags no formato `fork-vX.Y.Z` para não colidir com tags herdadas do upstream.

## [Não lançado]

## [0.3.0] - 2026-07-28

### Adicionado

- Entrada CMake/Ninja na raiz com presets Linux Debug e Release para os quatro
  processos do servidor.
- Descoberta obrigatória do MySQL Connector/C++ por target importado.
- Targets CMake/Ninja Win32 Debug e Release para `Main.dll` e `InfoEncoder.exe`,
  preservando recursos, PCH e runtimes MSVC dos projetos existentes.

### Alterado

- Build do servidor passou a usar listas explícitas de fontes, `Threads::Threads` e
  o mesmo preset Release no Docker e no desenvolvimento local.
- CI Linux passou a validar os builds nativos Debug e Release antes do smoke test do
  stack Compose.
- Workflow do cliente passou a aceitar `-BuildSystem CMake|MSBuild`, usando CMake
  como padrão e mantendo MSBuild como fallback validado pela CI.
- Tarefas e debug do VS Code passaram a construir e implantar o cliente com
  CMake/Ninja.

## [0.2.0] - 2026-07-28

### Adicionado

- Validação automatizada de configurações, links, JSON, PowerShell e scripts shell.
- Validação de ponta a ponta do runtime externo, deploy e encoder do cliente Win32.
- Health checks do servidor e do painel web com verificação real do MySQL.
- Lockfile das dependências de produção do painel web.

### Alterado

- CI Linux ampliada para validar schema, servidor, painel web, portas TCP e HTTP.
- Painel web atualizado para Node.js 24.18.0 e instalações reproduzíveis com
  `npm ci`.
- Branch `main` protegida pelos checks obrigatórios `Client Win32` e
  `Server Linux` em modo estrito.

### Corrigido

- Códigos de sucesso do `robocopy` deixam de causar falha residual no GitHub
  Actions.

## [0.1.0] - 2026-07-28

### Adicionado

- Fluxo reproduzível para build, deploy, encode e debug do cliente Windows.
- Tarefas e recomendações de extensões para VS Code.
- Documentação em português brasileiro e inglês.
- Guia consolidado de desenvolvimento, arquitetura e operação local.

### Alterado

- Estrutura do repositório organizada por código, runtime, serviços e deployment.
- Compose local independente de uma rede externa de proxy.
- Nome público do projeto alterado para MU 0.97k.
- Projeto reposicionado em torno de gameplay e conteúdo, identidade clássica e
  experiência de desenvolvimento.
- Créditos e proveniência consolidados no README e no aviso legal.

### Corrigido

- Inicialização do MySQL quando `SEED_TEST_DATA=0`.

[Não lançado]: https://github.com/aldomigge/mu-097k/compare/fork-v0.3.0...HEAD
[0.3.0]: https://github.com/aldomigge/mu-097k/compare/fork-v0.2.0...fork-v0.3.0
[0.2.0]: https://github.com/aldomigge/mu-097k/compare/fork-v0.1.0...fork-v0.2.0
[0.1.0]: https://github.com/aldomigge/mu-097k/releases/tag/fork-v0.1.0
