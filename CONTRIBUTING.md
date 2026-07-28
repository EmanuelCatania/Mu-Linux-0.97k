# Contribuindo

O MU 0.97k segue um fluxo simples baseado em trunk:

1. crie uma branch curta a partir de `main`;
2. use os prefixos `feat/`, `fix/`, `refactor/`, `docs/` ou `chore/`;
3. faça commits assinados e objetivos;
4. execute `pwsh -File scripts/validate-repository.ps1` e as validações de build
   relevantes no Windows e/ou WSL2;
5. abra um pull request para `aldomigge/mu-097k:main`.

`main` não recebe pushes diretos. Pull requests usam squash merge e não devem incluir
alterações funcionais sem documentação ou validação correspondente.

## Upstream

Não abra pull requests automaticamente contra o projeto upstream. Mudanças externas
são avaliadas e importadas individualmente, preservando autoria e referência ao commit
de origem.

## Licenciamento

A licença do código e dos assets legados ainda não foi determinada. Contribuições
externas podem permanecer pendentes até que os direitos de distribuição e
relicenciamento sejam esclarecidos. Consulte `NOTICE.md`.
