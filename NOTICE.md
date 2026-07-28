# Proveniência, propriedade e licenciamento

O MU 0.97k é uma linha independente construída sobre software comunitário legado e
materiais relacionados a MU Online. As categorias abaixo possuem origens e direitos
distintos e não devem ser tratadas como uma única obra sob uma licença comum.

## MU Online e o cliente original

MU Online, o executável fechado `main.exe`, nomes, marcas, músicas, imagens, sons e
demais assets originais pertencem à Webzen e/ou aos respectivos titulares. O projeto
não possui o código-fonte do cliente original, não representa a Webzen e não declara
afiliação ou endosso oficial.

## Código comunitário

Esta linha deriva dos seguintes trabalhos declarados pelo upstream:

- [MuEmu 0.97k de Nico Muratona/Kayito](https://github.com/nicomuratona/MuEmu-0.97k-kayito),
  base das sources e ferramentas;
- [Mu-Linux-0.97k de Emanuel Catania](https://github.com/EmanuelCatania/Mu-Linux-0.97k),
  responsável pela linha Linux, Docker e MySQL usada como base deste fork;
- Simple MU Online Templates de Trifon Dinev, utilizado no painel web;
- contribuições atribuídas a Kapocha33, SetecSoft, Zeus e ogocx no
  [README histórico](docs/history/upstream-readme.es.md).

O ponto de divergência do fork está registrado pela tag assinada
`upstream-baseline-a735600`, referente ao commit
`a73560053a97c71d4d1a1eecf7a0f797a308f402`.

## Situação de licenciamento

Não foi identificado um arquivo de licença que abranja todo o código, binários e
assets recebidos do upstream. Portanto, este repositório não declara MIT, GPL ou outra
licença comum para o conjunto legado e não concede direitos sobre material de
terceiros.

Na ausência de autorização aplicável, não presuma permissão para copiar,
redistribuir, sublicenciar ou comercializar o conteúdo. Contribuições novas também
não alteram automaticamente os direitos incidentes sobre o material preexistente.

O histórico e as atribuições originais são preservados em
`docs/history/upstream-readme.es.md`. Uma auditoria futura deverá inventariar e separar
código próprio, código comunitário, dependências e assets antes de qualquer decisão
de relicenciamento ou distribuição pública.
