# Mu Linux 97K

[![Site do projeto](https://img.shields.io/badge/site%20do%20projeto-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/es/)
[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k?label=server&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-web?label=web&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-web) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mu-editor?label=editor&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mu-editor) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mysql?label=mysql&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mysql)

[🇪🇸 Español](README.md) | [🇺🇸 English](README.en.md) | 🇧🇷 Português

Port nativo para Linux e stack operacional do MuEmu 0.97k com Docker, painel web e editor opcional.

## Novidades da 0.97.30 — conclusão da Fase 3

- O servidor compila com CMake no Windows x86 e x64, além de Linux x64 e ARM64. O CI cobre as quatro combinações em Debug e Release.
- O MySQL Connector/C++ é baixado no configure com versão e hash fixos; seus binários não são mais versionados.
- Correções da largura de ponteiro em callbacks de sockets, IOCP e timers, necessárias para x64.
- Fila de envio ampliada para o catálogo de itens que o servidor envia no login.
- Comandos GM para antecipar Blood Castle, Devil Square, Golden Archer Bingo e invasões sem reiniciar o servidor.

### Compilar e executar no Windows (sem Docker)

1. Requisitos: Visual Studio 2022 ou 2026 com "Desenvolvimento para desktop com C++" (toolset v143, inclui CMake) e MySQL 8.
2. Compilar a partir da raiz do repositório:

   ```
   cmake -S . -B out/win-x86 -A Win32 -T v143
   cmake --build out/win-x86 --config Release
   ```

   Os executáveis ficam em `Source/MuServer/bin/Release/`.
3. Copiar cada executável para a sua pasta em `MuServer/`, junto ao seu `.ini`:
   - `ConnectServer/ConnectServer.exe` → `MuServer/ConnectServer/`
   - `GameServer/GameServer.exe` → `MuServer/GameServer/` (lê os dados de `MuServer/Data/`)
   - `MYSQL/JoinServer/JoinServer.exe` → `MuServer/MySQL/JoinServer/`
   - `MYSQL/DataServer/DataServer.exe` → `MuServer/MySQL/DataServer/`

   Copie também as DLLs, a pasta `plugin` e `LICENSE-MySQL.txt` que o CMake coloca junto ao JoinServer e DataServer. A arquitetura e a configuração devem corresponder às dos executáveis.
4. Banco de dados: executar `MuServer/MySQL/DB/CreateDatabase.sql` e depois `PoblateDatabase.sql`. O usuário padrão é `admin` / `asd123`; se usar outro, altere em `DataServer.ini` e `JoinServer.ini`.
5. Em `MuServer/ConnectServer/ServerList.dat`, colocar o IP do servidor (`127.0.0.1` para testar no mesmo PC).
6. Iniciar nesta ordem: DataServer, JoinServer, ConnectServer e GameServer.

O cliente `0.97.20` tem o endereço `mu.server-pups.space` compilado. Para apontá-lo para um servidor próprio sem recompilar, adicione a linha `127.0.0.1 mu.server-pups.space` em `C:\Windows\System32\drivers\etc\hosts` (como administrador) e remova-a ao terminar.

#### Windows x64 e dependências da Fase 3

Para x64, na raiz do repositório:

```powershell
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release --parallel 4
```

Também estão disponíveis `windows-x86-release`, `windows-x86-debug` e `windows-x64-debug`. O CMake baixa o MySQL Connector/C++ **8.0.33**, com SHA256 fixo para x86/x64 e Debug/Release; os binários não ficam mais versionados. A primeira configuração precisa acessar os arquivos oficiais do MySQL. Os projetos MSBuild antigos usam o mesmo mecanismo em `Debug_MySQL` e `Release_MySQL` e precisam do CMake no PATH.

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

O cliente baseado em injeção de DLL está totalmente descontinuado e foi removido deste repositório junto com suas fontes e o executável do Encoder. Use o [cliente baseado em sources 0.97.20](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.20). Os arquivos antigos continuam disponíveis no histórico do Git. As tabelas de `MuServer/Data/Custom/Encoder/` são mantidas porque o catálogo do servidor as utiliza.

## Início rápido

1. Copie `.env.example` para `.env` e configure credenciais, `PUBLIC_IP` e secrets.
2. Inicie a stack:

   ```bash
   docker compose up -d --build
   ```

3. Abra `44405/tcp`, `55601/udp` e `55901/tcp`.

Para imagens publicadas sem build:

```bash
docker compose -f docker-compose.images.yml up -d
```

Em ARM com 1 GB de RAM ou menos, ative 2 GB de swap antes do primeiro build. `uname -m`: `x86_64` = amd64, `aarch64` = arm64.

## Ambiente

`.env` usa `.env.example` como base; altere os valores antes de produção.

- MySQL: `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`.
- Servidor: `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`, `PUBLIC_IP`.
- Web: `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`, `ADMIN_USER`, `ADMIN_PASS`, `TRUST_PROXY`.
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
- Salve TXT do servidor em ANSI / Windows-1252 sem BOM. Shops ficam em `MuServer/Data/Shop/*.txt`; `-1 -1` posiciona `SlotX SlotY` automaticamente.
- Logs: `docker compose logs -f`. Pastas principais: `Source/`, `MuServer/`, `docker/`.

## Site e editor 0.97.20

Primeiro acesso a /admin: troca obrigatória de senha e configuração do idioma padrão (ES/EN/PT), tema clássico, Elf, Dark Knight ou Dark Wizard e estado da conexão do editor. Alterações posteriores ficam em /admin/settings. Visitantes podem escolher seu idioma pelas bandeiras; isso não altera o idioma do admin/editor nem traduz notícias ou dados do jogo.

As configurações são salvas automaticamente no MySQL (web_site_settings); preserve seu volume. Atualize apenas emapupi/mu-linux-97k-web:0.97.20 e emapupi/mu-linux-97k-mu-editor:0.97.20 e recrie esses serviços. Não é necessário reconstruir o servidor ou MySQL. As telas do editor são servidas pela web; a API do editor mantém o comportamento anterior.
