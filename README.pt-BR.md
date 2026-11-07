# Mu Linux 97K

[![Site](https://img.shields.io/badge/site-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/pt-br/) [![XERJ Corpus Hub contributor](https://hub.xerj.org/contributors/EmanuelCatania/badge.svg)](https://hub.xerj.org/contributors/EmanuelCatania)

[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k?label=server&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-web?label=web&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-web) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mu-editor?label=editor&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mu-editor) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mysql?label=mysql&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mysql) 

[🇦🇷 Español](README.md) | [🇺🇸 English](README.en.md) | 🇧🇷 Português

Port nativo para Linux e stack operacional do MuEmu 0.97k com Docker, painel web e editor opcional.

## Novidades da 0.97.40 — conclusão da Fase 4

- **Texto em UTF-8 de ponta a ponta.** O servidor deixa de converter para
  Windows-1252: o que o jogador digita é o que fica salvo e o que os outros
  veem. Nomes de personagem e guild em qualquer escrita (10 e 8 caracteres,
  validados e normalizados em NFC), e chat, sussurros, avisos de guild e `/post`
  de até 200 bytes. O banco passa a `utf8mb4`.
- **Protocolo 0.97.40.** Nomes de personagem de 40 bytes e de guild de 32, e
  chat de 200. Os pacotes que com esses tamanhos passam de 255 bytes (lista de
  personagens, party, ranking do Devil Square) usam header C2.
- **Textos traduzidos enviados pelo servidor.** Ao entrar, cada cliente recebe
  no seu idioma os nomes de itens, monstros e skills, as linhas de tooltip dos
  itens custom e os textos próprios do cliente. Veja
  [Textos e idiomas](#textos-e-idiomas).
- **Painel web:** editor de traduções (`Data/Text`) e editor de itens custom
  (efeitos, pose, tooltip e pets de `Data/Custom/Items` e `Data/Custom/Pets`),
  os dois com a mesma validação do servidor; troca de senha, Turnstile e
  ordenação de contas no painel de administração.
- **Barra de buffs do cliente:** o servidor manda a cada jogador o tempo
  restante de cada efeito, seus valores e quem o lançou (F3:F1).
- **Névoa por mapa:** além das cores, o `MapFog.txt` aceita a distância de
  início e fim da névoa de cada mapa (F3:E9).
- **MySQL pela API em C.** JoinServer e DataServer usam `mysql.h`: no Linux,
  `libmysqlclient`; no Windows, o MariaDB Connector/C compilado estático. Não
  fica nenhuma DLL do MySQL junto aos servidores.
- Correções: a pontuação do ranking do Blood Castle chegava deslocada dois
  bytes; o pacote de recall (C3:B0) tinha o layout errado com GCC; Rena e Stone
  empilham (`ItemStack.txt`).

**Compatibilidade: server 0.97.40 ↔ [cliente 0.97.40](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.40).**
Atualize os dois juntos: o cliente 0.97.30 lê os pacotes com os tamanhos antigos
e não entra num servidor 0.97.40, e vice-versa.

### Como atualizar a partir da 0.97.30

Esta versão **muda o banco de dados**. Faça um backup antes de começar.

1. **Banco de dados.** Siga o [guia de migração para utf8mb4](docs/utf8mb4-migracion.md)
   (em espanhol): backup, parar `mu-server` e `mu-web`, e rodar
   `Migrate-utf8mb4.sql`, `Migrate-0.97.40.sql` e `Procedures.sql`. Se o seu
   banco já estava em `utf8mb4`, bastam os dois últimos. O script para sem mexer
   em nada se dois nomes ficariam iguais com a collation nova.
2. **Imagens.** Com o compose de imagens publicadas:

   ```bash
   docker compose -f docker-compose.images.yml pull mu-server mu-web mu-editor
   docker compose -f docker-compose.images.yml up -d
   ```

   Para fixar a versão, use as imagens com tag `0.97.40`.
3. **Volumes.** O pull não atualiza um volume persistente. Integre no seu, a
   partir dos arquivos desta versão:
   - `GameServer/Data/GameServerInfo - StartUp.dat`: mude `ServerVersion` para
     `0.97.40`. **Sem isso, o cliente novo não entra** (*"versão incorreta"*).
   - `Data/Text/`: a pasta inteira (`Eng`, `Spn`, `Por`). Sem ela tudo funciona,
     só que sem traduções.
   - `Data/Message_Spn.txt` e `Data/Message_Por.txt`: agora estão em UTF-8. Se
     você os modificou, converta sua cópia de Windows-1252 para UTF-8.
   - `Data/Item/ItemStack.txt`: a linha `14,021` (Rena e Stone).
   - `Data/Custom/Encoder/MapFog.txt`: opcional, o bloco novo de distâncias.
     Sem ele, cada mapa usa 2000 a 2700 como antes.

**Pterodactyl:** selecione `0.97.40` em `MU_RELEASE_TAG` e atualize também a
imagem de runtime, que agora instala `libmysqlclient21`. Com o servidor parado,
extraia o arquivo da sua arquitetura numa pasta temporária, substitua os quatro
binários e o `start.sh`, e integre os arquivos de Data acima mantendo seus INI.
A migração do banco é a mesma.

### Textos e idiomas

O cliente informa seu idioma ao servidor ao conectar (`[Language]
LangSelection` do cliente: `Eng`, `Spn` ou `Por`), e o servidor manda as
traduções desse idioma junto com o catálogo de conteúdo. Ficam em
`MuServer/Data/Text/<idioma>/`:

| Arquivo | O que traduz | Índice |
|---|---|---|
| `Item.txt` | nomes de itens | seção e índice de `Item/Item.txt` |
| `Monster.txt` | nomes de monstros e NPCs | índice de `Monster/Monster.txt` |
| `Skill.txt` | nomes de skills | índice de `Skill/Skill.txt` |
| `Tooltip.txt` | linhas do tooltip dos itens custom | item e número da linha do JSON de `Custom/Items` |
| `Client.txt` | textos próprios do cliente (opções, painel de eventos, barra de buffs, título da janela) | o número de cada texto; o comentário de cima diz qual é |

O que não estiver traduzido o cliente resolve com seus próprios arquivos, então
um arquivo vazio não quebra nada. Regras, que o servidor e o editor verificam:

- UTF-8 sem BOM, uma linha por texto, sem aspas nem quebras de linha dentro.
- Até 63 bytes por nome (o cliente corta os de itens em 29 e os de skills em 31)
  e 59 por linha de tooltip.
- No `Client.txt`, os `%d`/`%s`/`%u` de cada linha precisam ser os mesmos do
  inglês e na mesma ordem: senão o cliente descarta a linha.

Dá para editar à mão ou pelo **editor de traduções** do painel web, que mostra
o nome atual ao lado da tradução e recarrega o servidor ao salvar.

## Novidades da 0.97.30 — conclusão da Fase 3

- O servidor compila com CMake no Windows x86 e x64, além de Linux x64 e ARM64. O CI cobre as quatro combinações em Debug e Release.
- O MySQL Connector/C++ é baixado no configure com versão e hash fixos; seus binários não são mais versionados.
- Correções da largura de ponteiro em callbacks de sockets, IOCP e timers, necessárias para x64.
- Fila de envio ampliada para o catálogo de itens que o servidor envia no login.
- Comandos GM para antecipar Blood Castle, Devil Square, Golden Archer Bingo e invasões sem reiniciar o servidor.

**Compatibilidade: server 0.97.30 ↔ [cliente 0.97.30](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.30).**

### Como atualizar da 0.97.20

Com Docker, atualize apenas o servidor:

```bash
docker compose -f docker-compose.images.yml pull mu-server
docker compose -f docker-compose.images.yml up -d --no-deps mu-server
```

Para fixar a versão, use a imagem `emapupi/mu-linux-97k:0.97.30` (amd64 e arm64). O pull não atualiza um volume persistente: adicione ao seu `Data/Command.txt` as quatro linhas dos novos comandos GM, como explicado em [Administração](#administração-eventos-gm-e-recargas). MySQL, o site e o editor não mudam.

**Compilando a partir do código (`docker-compose.yml`):** o serviço MySQL passa da imagem `mysql:5.7` para `mysql:8.4.11`, a mesma versão da imagem publicada. Um volume criado com 5.7 não inicia com 8.4: antes de atualizar, siga o [guia de migração para 8.4](docs/mysql-8.4-upgrade.md). O compose de imagens publicadas já usava 8.4 e não muda.

**Pterodactyl:** selecione `0.97.30` em `MU_RELEASE_TAG`. Em instalações existentes, com o servidor parado, extraia o arquivo da sua arquitetura deste Release em uma pasta temporária, substitua os quatro binários e o `start.sh`, e adicione as linhas do `Command.txt`, mantendo seus INI e Data.

### Compilar e executar no Windows (sem Docker)

1. Requisitos: Visual Studio 2022 ou 2026 com "Desenvolvimento para desktop com C++" (toolset v143, inclui CMake) e MySQL 8.
2. Compilar a partir da raiz do repositório:

   ```
   cmake --preset windows-x64-release
   cmake --build --preset windows-x64-release --parallel 4
   ```

   Os executáveis ficam em `Source/MuServer/bin/Release/`.
3. Copiar cada executável para a sua pasta em `MuServer/`, junto ao seu `.ini`:
   - `ConnectServer/ConnectServer.exe` → `MuServer/ConnectServer/`
   - `GameServer/GameServer.exe` → `MuServer/GameServer/` (lê os dados de `MuServer/Data/`)
   - `MYSQL/JoinServer/JoinServer.exe` → `MuServer/MySQL/JoinServer/`
   - `MYSQL/DataServer/DataServer.exe` → `MuServer/MySQL/DataServer/`

   Não é preciso copiar nenhuma DLL do MySQL: o conector fica compilado dentro do JoinServer e do DataServer.
4. Banco de dados: executar `MuServer/MySQL/DB/CreateDatabase.sql` e depois `PoblateDatabase.sql`. O usuário padrão é `admin` / `asd123`; se usar outro, altere em `DataServer.ini` e `JoinServer.ini`.
5. Em `MuServer/ConnectServer/ServerList.dat`, colocar o IP do servidor (`127.0.0.1` para testar no mesmo PC).
6. Iniciar nesta ordem: DataServer, JoinServer, ConnectServer e GameServer.

O cliente `0.97.40` tem o endereço `mu.server-pups.space` compilado. Para apontá-lo para um servidor próprio sem recompilar, adicione a linha `127.0.0.1 mu.server-pups.space` em `C:\Windows\System32\drivers\etc\hosts` (como administrador) e remova-a ao terminar.

#### Windows x86 e dependências

x64 é o build recomendado. x86 (Win32) fica como base histórica: é a arquitetura do MuEmu original e o CI continua compilando, mas deixou de ser o caminho deste guia. Para compilá-lo:

```powershell
cmake --preset windows-x86-release
cmake --build --preset windows-x86-release --parallel 4
```

Também estão disponíveis `windows-x86-debug` e `windows-x64-debug`; os executáveis ficam na mesma pasta que os de x64. O JoinServer e o DataServer falam com o MySQL pela API em C (`mysql.h`): o CMake baixa o código-fonte do **MariaDB Connector/C 3.4.11** (SHA256 fixado em `cmake/MuMySQLPackages.cmake`) e o compila como biblioteca estática, com Schannel para TLS. Por ser API em C, funciona igual com MSVC e MinGW. O MySQL 8.4 autentica com `caching_sha2_password`, que exige TLS; o conector o solicita sem verificar o certificado autoassinado do server. No Linux usa-se o `libmysqlclient-dev` (ou `libmariadb-dev`) do sistema. A primeira configuração precisa acessar o GitHub. Os projetos MSBuild antigos compilam o mesmo conector em `Debug_MySQL` e `Release_MySQL`, com o toolset do projeto, e precisam do CMake no PATH.

O CI compila os quatro serviços para Windows x86/x64 e Linux x64/ARM64, em Debug e Release. ARM64 continua apenas compilado, sem validação em jogo. A Fase 3 corrige larguras de ponteiros em callbacks, IOCP e timers, e usa uma fila de envio de 256 KiB para o catálogo de login; cada pacote continua limitado a 8 KiB. O teste local de Windows x64 incluiu login, jogo, reset, compra/venda e máquina de Chaos; não foi um teste de carga.

### Administração: eventos GM e recargas

Somente GM de nível 1 ou superior, pelo chat do jogo:

| Comando | Evento |
| --- | --- |
| `/tamastart` | Tamachan |
| `/bcstart` | Blood Castle, todos os níveis |
| `/dsstart` | Devil Square, todos os níveis |
| `/bingostart` | Golden Archer Bingo |
| `/invasionstart <indice>` | Índice em `MuServer/Data/Event/InvasionManager.dat` |

Agendam uma única execução aproximadamente dois minutos depois, sem alterar os horários do arquivo nem reiniciar eventos ativos. BC/DS também rejeitam o pedido quando a entrada já está aberta. Iniciar uma invasão não reinicia as outras. O evento deve estar habilitado. Os novos comandos registram o pedido e o resultado em `COMMAND_LOG`.

Em instalações existentes, adicione as quatro novas linhas de `MuServer/Data/Command.txt` depois de `/tamastart` e antes de `end`, nesta ordem: bcstart, dsstart, bingostart, invasionstart. A ordem determina o ID; preserve as outras configurações.

#### Exemplo: recarregar lojas pelo Docker

Digitar apenas `shop` ou `/reload shops` no console **não funciona**: o GameServer Linux não lê comandos de stdin. No terminal do host, na pasta do Compose:

```sh
docker compose exec -T mu-server sh -c 'printf "shop\n" > "${MU_DATA_PATH:-/opt/mu/Data}/EditorReload.flag"'
```

Se você já estiver no shell do contêiner do servidor:

```sh
printf 'shop\n' > "${MU_DATA_PATH:-/opt/mu/Data}/EditorReload.flag"
```

O servidor consome `EditorReload.flag` em cerca de 10 segundos. Confira `Shop reloaded by editor flag` no log. Para instalações com imagens, acrescente `-f docker-compose.images.yml` ao `docker compose`. Sem Compose, substitua `docker compose exec -T mu-server` por `docker exec <conteiner>`.

Tokens aceitos: `all`, `common`, `shop`, `monster`, `move`/`gate`, `quest`, `util`, `item`, `skill`, `event`, `eventitembag`, `chaosmix`, `command`, `custom`, `character`, `map`, `hack`. Separe vários tokens com espaços ou vírgulas. Espere o consumo de um pedido antes de enviar outro. `event` e `all` reinicializam eventos: não os use para antecipar um evento nem durante um evento ativo. O Windows mantém as recargas pelo menu.

## Novidades da 0.97.20 — conclusão da Fase 2

**Compatibilidade: server 0.97.20 ↔ [cliente 0.97.20](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.20).** Atualize os dois juntos: o formato estendido de itens não é compatível com clientes anteriores nem com o Main.dll histórico.

- Ao entrar, o servidor envia o catálogo de itens, monstros, efeitos, pets e tooltips. O cliente recebe as definições e mantém os modelos e texturas em seus arquivos locais.
- Cada seção aceita novos índices 32..511 e formato de 7 bytes. A coluna **Comportamento** reutiliza a lógica de um item clássico em ataques, skills, combinações e eventos.
- Os JSON de itens definem efeitos, pose no inventário e textos do tooltip. O equipamento custom também aparece nos outros jogadores e na seleção de personagens.
- Os pets são configurados em JSON. Inclui **Pet Rudolph (13,400)**, que voa até o zen próximo e o recolhe sob controle do servidor.
- Os arquivos de customs do Encoder da DLL continuam sendo aceitos como entrada do catálogo.
- Corrigida a ordem de leitura da seção e do índice em `ItemOption.txt` no GCC, que impedia o aparecimento de Triple Shot.

### Como atualizar da 0.97.00

Faça backup dos arquivos de Data antes de integrar os exemplos. Não apague os volumes nem substitua suas configurações personalizadas completas.

Com Docker e o compose de imagens, atualize apenas o servidor:

```bash
docker compose -f docker-compose.images.yml pull mu-server
# Integre os arquivos de Data abaixo antes de iniciar o novo servidor.
docker compose -f docker-compose.images.yml up -d --no-deps mu-server
```

O compose usa `latest`; para fixar a versão, altere apenas a imagem de `mu-server` para `emapupi/mu-linux-97k:0.97.20`. As imagens suportam `linux/amd64` e `linux/arm64`.

**O pull da imagem não atualiza um volume persistente.** A partir de `MuServer/Data` desta versão, integre os arquivos ao volume `/opt/mu/Data`:

- `Custom/Items/`: `03_000.json` (pose da Light Spear) e `13_400.json` (pose e tooltip do Rudolph).
- `Custom/Pets/13_400.json`: movimento e coleta de zen do Rudolph.
- `Custom/Encoder/`: `CustomBow.txt`, `CustomGlow.txt`, `CustomItem.txt`, `CustomMonster.txt`, `ItemPosition.txt` e `MapFog.txt`. Mescle as entradas custom existentes em vez de sobrescrevê-las.
- `Item/Item.txt`: adicione a linha **400 dentro da seção 13** desta versão, se ainda não existir. Preserve as demais linhas. Se já utiliza 13,400, resolva o conflito antes de incorporar o exemplo.

Instale também os assets correspondentes no cliente 0.97.20. Reinicie o servidor e confira as mensagens `[ContentCatalog]`, incluindo `13_400.json`, seu tooltip e sua pose. O início deve mostrar `MU Linux server 0.97.20` e o link do cliente compatível.

**MySQL, web e editor não mudam nesta versão:** não há migração de esquema nem necessidade de reconstruir suas imagens. Se sua instalação ainda usa MySQL 8.0, a [migração anterior para 8.4](docs/mysql-8.4-upgrade.md) é uma tarefa separada.

**Pterodactyl:** importe o egg de servidor atualizado, que aceita `0.97.20` sem `v`, e configure esse valor em `MU_RELEASE_TAG`. A imagem de runtime do Pterodactyl contém apenas dependências: atualizá-la não altera os binários do volume. Em instalações existentes, pare o servidor e extraia `mu-linux-97k-server-amd64.tar.gz` ou `mu-linux-97k-server-arm64.tar.gz` deste Release em uma pasta temporária; atualize os quatro binários e `start.sh`, e integre Data como acima, preservando seus INI. Não reinstale sem revisar: o instalador também extrai configurações e dados. Os eggs de web e MySQL não precisam mudar. Os arquivos por arquitetura devem estar anexados ao Release antes da instalação desta versão.

### Como adicionar itens custom

O servidor é a fonte das definições; o cliente só precisa dos modelos `.bmd` e das texturas. O cliente baseado em sources não exige regenerar `item.bmd` ou `ClientInfo.bmd`.

1. Adicione a linha do item à sua seção em `Data/Item/Item.txt`. Para índices 32..511, informe o item clássico imitado em **Comportamento** (por exemplo `00,020`), seguido da pasta, modelo e brilho RGB. Copie uma linha completa da seção e ajuste os atributos; não use uma linha abreviada com `...`.
2. Para customs clássicos, mantenha as definições de modelo/brilho em `Data/Custom/Encoder/CustomItem.txt` e `CustomGlow.txt`, além das demais tabelas do Encoder incluídas.
3. Opcionalmente, crie `Data/Custom/Items/<seção>_<índice>.json` com `item`, `effects`, `inventory` e/ou `tooltip`. Os exemplos `03_000.json` e `13_400.json` mostram o formato real.
4. Para um pet custom, adicione `Data/Custom/Pets/<seção>_<índice>.json`. Rudolph usa `13,400`, slot 8, comportamento `*`, e define movimento e coleta de zen. O cliente anima o pet; o servidor autoriza e executa a coleta.
5. Distribua os assets do modelo ao cliente, reinicie o servidor e entre novamente para receber o catálogo. Com permissão de GM, teste Rudolph usando `/make 13 400`.

Consulte os [exemplos completos e caminhos dos assets do cliente](https://github.com/EmanuelCatania/Mu-97k-Client-Source/blob/0.97.20/README.md#cómo-agregar-items-custom).

## Base e referência

Baseado nas sources do Kayito.

[![Ver README original do Kayito](https://img.shields.io/badge/README%20original-Kayito-181717?logo=github)](https://github.com/nicomuratona/MuEmu-0.97k-kayito#readme)

## Créditos e objetivos

- Kayito: sources base MuEmu 0.97k; Trifon Dinev: Simple MU Online Templates usado no `mu-web`.
- Servidor Linux nativo com epoll, MySQL sem MSSQL e Docker pronto para VPS.

## Imagens oficiais

- `emapupi/mu-linux-97k` (server), `emapupi/mu-linux-97k-web` (web).
- `emapupi/mu-linux-97k-mu-editor` (API do editor), `emapupi/mu-linux-97k-mysql` (MySQL/init).

## Update 1 (2026-02-20)

- Editor web para dados, contas, personagens, shops, spawns, gates, configurações, itens, monstros, drops, backups e snapshots.
- Serviço opcional `mu-editor` com recarga automática por `EditorReload.flag`.

## Update 2 (2026-03-26)

- Correções do cliente para staffs, Character Info, guild, macros e quests; drop Divine e Crystal Sword/Morning Star.

## Update 3 (2026-09-20)

- Server: correção do Blood Castle e evento de chuva de joias Tamachan (o cliente baseado nas sources é necessário para vê-lo completamente).
- `MonsterSetBase.txt`: restauração de spawn points normais ausentes e NPCs necessários nos mapas reportados.
- Docker: CMake limitado a um trabalho (`-j1`) para evitar OOM em hosts com pouca memória.
- Arquiteturas: suporte nativo `linux/amd64` e `linux/arm64`, com MySQL 8.4 LTS em ambas.
- Banco de dados: atualização para MySQL 8.4 LTS. Leia o [guia de migração](docs/mysql-8.4-upgrade.md) antes de reutilizar um volume existente.
- ARM64: correção da leitura de scripts e semântica herdada de `char`, evitando parse incorreto e desconexões erradas.
- CI: cada push, pull request e execução manual valida imagem `linux/arm64` e smoke test de início.

## Estado do cliente

O cliente baseado em injeção de DLL está totalmente descontinuado e foi removido deste repositório junto com suas fontes e o executável do Encoder. Use o [cliente baseado em sources 0.97.40](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.40). Os arquivos antigos continuam disponíveis no histórico do Git. As tabelas de `MuServer/Data/Custom/Encoder/` são mantidas porque o catálogo do servidor as utiliza.

## Início rápido

1. Copie `.env.example` para `.env` e configure credenciais, `PUBLIC_IP` e secrets.
2. Inicie a stack:

   ```bash
   docker compose up -d --build
   ```

3. Abra `44405/tcp` e `55901/tcp`.

Para imagens publicadas sem build:

```bash
docker compose -f docker-compose.images.yml up -d
```

Em ARM com 1 GB de RAM ou menos, ative 2 GB de swap antes do primeiro build. `uname -m`: `x86_64` = amd64, `aarch64` = arm64.

## Ambiente

`.env` usa `.env.example` como base; altere os valores antes de produção.

- MySQL: `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`.
- Servidor: `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`, `PUBLIC_IP`.
- Web: `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`, `ADMIN_USER`, `ADMIN_PASS`, `TRUST_PROXY`. `GEOIP_ENABLED` e `ACQUISITION_TRACKING` ficam em `0`: bandeiras de país e origem do cadastro só ligam quando configurados. `GEOIP_ENABLED` precisa do pacote opcional `geoip-lite` (`INSTALL_GEOIP=1` ao construir a imagem). `GEOIP_HIDDEN_COUNTRIES` é uma lista vazia de códigos ISO. `RANKINGS_RESETS_FIRST=1` ordena as listas de nível e classe por resets antes do nível.
- Dados de teste: `SEED_TEST_DATA`; editor: `EDITOR_ENABLED`, `EDITOR_PORT`, `EDITOR_API_URL`, `EDITOR_MAX_BACKUPS`, `EDITOR_MAX_SNAPSHOTS`.

Depois de mudar credenciais MySQL, recrie o volume:

```bash
docker compose down -v
docker compose up -d --build
```

## Editor, painel web e notas

O editor opcional usa os volumes internos do `mu-server`:

```bash
docker compose -f docker-compose.yml -f docker-compose.editor.yml up -d --build
```

Ele mantém backups por arquivo e snapshots de `MuServer/Data` e `MuServer/GameServer/DATA`. `mu-web` oferece registro, login, rankings e notícias na porta externa `8085` (interna `8080`). Admin inicial: `admin / 123456`, com troca obrigatória no primeiro login.

- `MD5Encryption=2` usa MD5 binário; recrie a DB com `docker compose down -v` ao migrar de texto simples.
- Salve os TXT do servidor em UTF-8 sem BOM. Desde a 0.97.40 o servidor não converte mais para Windows-1252: um arquivo em ANSI com acentos aparece errado no cliente. Shops ficam em `MuServer/Data/Shop/*.txt`; `-1 -1` posiciona `SlotX SlotY` automaticamente.
- Logs: `docker compose logs -f`. Pastas principais: `Source/`, `MuServer/`, `docker/`.

## Site e editor 0.97.20

Primeiro acesso a /admin: troca obrigatória de senha e configuração do idioma padrão (ES/EN/PT/UK; o valor inicial continua ES), tema clássico, Elf, Dark Knight ou Dark Wizard e estado da conexão do editor. Alterações posteriores ficam em /admin/settings. Visitantes podem escolher seu idioma pelas bandeiras; isso não altera o idioma do admin/editor nem traduz notícias ou dados do jogo.

As configurações são salvas automaticamente no MySQL (web_site_settings); preserve seu volume. Atualize apenas emapupi/mu-linux-97k-web:0.97.20 e emapupi/mu-linux-97k-mu-editor:0.97.20 e recrie esses serviços. Não é necessário reconstruir o servidor ou MySQL. As telas do editor são servidas pela web; a API do editor mantém o comportamento anterior.
