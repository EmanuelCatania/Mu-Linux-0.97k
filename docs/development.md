# Desenvolvimento local

Este guia reúne a arquitetura, a preparação do runtime, o build do cliente Windows e
a operação do servidor no WSL2. O fluxo mantém clones separados e sincroniza mudanças
exclusivamente pelo Git.

## Arquitetura

```text
src/client ──MSBuild/CMake──> C:\Dev\runtime\mu-097k\client
     │                              ▲
     └──InfoEncoder + MainInfo.ini──┘

CMake/Ninja ──> src/server ──build Docker──> mu-server ──> MySQL
runtime/server──────────────────────────────┘       └──> services/web
```

`runtime/client` e `runtime/encoder` são templates rastreados. A automação os copia
para um runtime externo, compila `Main.dll` e `InfoEncoder.exe`, executa o encoder e
instala `ClientInfo.bmd`. Os binários rastreados no repositório não são sobrescritos.

O ponto de entrada CMake fica na raiz do repositório. No Linux, ele seleciona os
quatro processos em `src/server`; no Windows, seleciona `Main.dll` e
`InfoEncoder.exe` em `src/client`. O Dockerfile usa o preset Ninja Release do
servidor e instala os binários junto aos dados de `runtime/server`. MySQL, servidor
e painel compartilham a rede privada do Compose. O editor é opcional e usa
`compose.editor.yaml`.

## Cliente Windows

Use o clone em `C:\Dev\projects\mu-097k` com PowerShell 7, VS Code e Visual Studio
Build Tools. O projeto atual é Win32 e usa MSVC 14.44 por meio do toolset compatível
instalado no Build Tools.

Inicialize o runtime uma vez:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
```

O destino padrão é `C:\Dev\runtime\mu-097k`. `-ForceRuntime` recria essa cópia a
partir dos templates e deve ser usado somente quando as alterações locais puderem ser
descartadas.

Edite a configuração externa:

```text
C:\Dev\runtime\mu-097k\encoder\MainInfo.ini
```

`ClientSerial` e `ClientVersion` devem corresponder a `ServerSerial` e
`ServerVersion` no GameServer. Gere `ClientInfo.bmd` com:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action Encode
```

O script aceita `-BuildSystem CMake|MSBuild`. CMake/Ninja é o padrão; MSBuild
permanece disponível como fallback explícito durante o período de transição:

```powershell
# Fluxo padrão com CMake/Ninja
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Release

# Fallback temporário com MSBuild
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug -BuildSystem MSBuild
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Release -BuildSystem MSBuild
```

O modo CMake localiza o Build Tools, prepara um ambiente MSVC x86 temporário com
`vcvarsall.bat -vcvars_ver=14.44` e usa os presets `client-windows-debug` e
`client-windows-release`. Os artefatos ficam em `out/build/<preset>/bin`; os do
MSBuild permanecem em `src/client/bin/<configuração>`. CMake e Ninja devem estar no
`PATH`.

As outras ações disponíveis são `Build`, `Deploy` e `Clean`, sempre usando o mesmo
`-BuildSystem` escolhido para o build. As tarefas de cliente do VS Code usam CMake
explicitamente. A configuração `Client: Debug Main.dll (x86)` executa BuildDeploy
Debug, implanta a DLL e o PDB e inicia `main.exe` com `cppvsdbg`. A configuração
automática do CMake ao abrir o workspace fica desativada; os presets podem ser
selecionados manualmente na extensão CMake Tools.

## Servidor no WSL2

Mantenha o clone Linux em `~/Dev/projects/mu-097k`. Não edite ou compile o mesmo
working tree simultaneamente pelo Windows e pelo WSL.

Prepare a configuração local:

```bash
cd ~/Dev/projects/mu-097k
cp .env.example .env
```

Revise as credenciais. Para o cliente Windows conectado ao WSL2, mantenha:

```dotenv
PUBLIC_IP=127.0.0.1
```

Para compilar o servidor diretamente no WSL2, instale as dependências uma vez:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build libmysqlcppconn-dev
```

Configure e compile a variante desejada a partir da raiz do repositório:

```bash
cmake --preset server-linux-debug
cmake --build --preset server-linux-debug

cmake --preset server-linux-release
cmake --build --preset server-linux-release
```

Os quatro executáveis são gravados em
`out/build/<preset>/bin`. `CMakeUserPresets.json` pode ser usado para ajustes locais
e não é versionado. A ausência do MySQL Connector/C++ interrompe a configuração em
vez de produzir um build incompleto.

Valide e inicie o stack base:

```bash
docker compose config --quiet
docker compose up --build -d
docker compose ps
```

O stack base contém `mysql`, `mu-server` e `mu-web`. Para acompanhar logs:

```bash
docker compose logs --follow --tail=200
```

Use `docker compose down` para desligar sem excluir o volume MySQL. Não use `-v` sem
confirmar que os dados persistentes podem ser descartados.

## Editor opcional

Inicie o editor somente quando necessário:

```bash
docker compose -f compose.yaml -f compose.editor.yaml up --build -d
```

O overlay adiciona volumes de dados e backup compartilhados com o servidor. Os
arquivos em `deploy/legacy` referenciam imagens upstream e Pterodactyl e permanecem
somente como referência histórica; eles não fazem parte do fluxo suportado.

## Sincronização e validação

Faça alterações em apenas um clone por vez, envie a branch ao `origin` e atualize o
outro clone pelo Git. Antes de abrir um pull request, execute as validações relevantes:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

```bash
cmake --preset server-linux-debug
cmake --build --preset server-linux-debug
docker compose config --quiet
docker compose up --build -d
```

Confirme que `git status` continua limpo depois de build, deploy e execução dos
containers. A CI compila Debug e Release primeiro pelo padrão CMake e depois pelo
fallback MSBuild, que permanecerá disponível pelo menos até a próxima release.
