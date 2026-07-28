# Integrações legadas

Estes arquivos foram recebidos do upstream e são preservados somente como referência.

- `compose.upstream-images.yaml` usa imagens `emapupi/*` que não são publicadas nem
  mantidas pelo projeto MU 0.97k.
- `pterodactyl/` contém eggs que apontam para as mesmas imagens upstream.

Não use essas integrações como fluxo oficial desta linha independente. O ambiente local
suportado utiliza `compose.yaml` e builds feitos diretamente do código-fonte.
