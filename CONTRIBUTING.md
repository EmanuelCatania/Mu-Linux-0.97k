# Contribuindo

O fluxo normal é uma branch curta, um pull request para `main` e squash merge. Antes
de alterar código, leia o [guia de desenvolvimento](docs/development.md) e procure
Issues relacionadas.

## Fluxo

1. Atualize `main` com `git pull --ff-only`.
2. Crie uma branch com prefixo `feat/`, `fix/`, `refactor/`, `docs/` ou `chore/`.
3. Faça commits pequenos, assinados e com uma mensagem que explique a mudança.
4. Execute as validações correspondentes ao cliente, servidor ou documentação.
5. Abra um PR para `aldomigge/mu-097k:main` descrevendo o que mudou e como foi testado.

`main` não recebe push direto. PRs são integrados por squash e não devem misturar
mudanças sem relação.

## Validação rápida

- Documentação ou scripts: `pwsh -File scripts/validate-repository.ps1` e
  `git diff --check`.
- Cliente: `BuildDeploy` Debug e Release no Windows; use CMake como padrão e
  `-BuildSystem MSBuild` quando estiver testando o fallback.
- Servidor: `cmake --preset server-linux-debug`, `cmake --build --preset
  server-linux-debug`, `docker compose config --quiet` e o smoke test local.

Se uma mudança altera protocolo, configuração, encoder ou runtime, explique a
compatibilidade com os demais componentes no PR. Não inclua credenciais, binários
gerados ou material de terceiros sem origem e autorização verificáveis.

## Upstream e licenciamento

Este fork segue uma linha independente. Não abra PRs automáticos contra o upstream;
correções externas são avaliadas e importadas individualmente, preservando autoria e
referência.

O material legado não possui uma licença geral identificada. Consulte
[`NOTICE.md`](NOTICE.md) antes de copiar, redistribuir ou adicionar conteúdo de
terceiros.

## Releases

O histórico oficial fica nas GitHub Releases. Use títulos no formato
`MU 0.97k — fork vX.Y.Z`, liste poucas mudanças relevantes e mencione compatibilidade
ou limitações somente quando forem importantes para quem usa a versão.
