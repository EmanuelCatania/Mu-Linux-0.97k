# Desenvolvimento local

Este é o fluxo suportado para desenvolver o cliente no Windows e o servidor no
WSL2. Os clones são separados: altere e compile um por vez e sincronize o outro pelo
Git.

## Visão rápida

```text
Windows: src/client -> CMake/Ninja -> C:\Dev\runtime\mu-097k -> main.exe
                         InfoEncoder -> ClientInfo.bmd

WSL2: src/server -> CMake/Ninja/Docker -> mu-server -> MySQL
                                      \-> mu-web
```

O `main.exe` original é fechado. `Main.dll` é carregada por ele e aplica hooks e
extensões. O encoder lê `MainInfo.ini` e gera a configuração que a DLL consome.

## Requisitos

### Windows

- PowerShell 7, Git, CMake, Ninja e VS Code;
- Visual Studio Build Tools com C++ e o MSVC 14.44 usado pelo projeto;
- extensão C/C++ e as tarefas recomendadas pelo workspace.

### WSL2

- Ubuntu 24.04;
- Docker Engine e Docker Compose;
- para build nativo: `build-essential`, CMake, Ninja e MySQL Connector/C++.

## Cliente Windows

Use `C:\Dev\projects\mu-097k` no Windows. O runtime de execução fica fora do Git,
em `C:\Dev\runtime\mu-097k`.

### 1. Criar o runtime

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
```

O script copia os templates rastreados de `runtime/client` e `runtime/encoder`.
Para recriar uma cópia existente, use `-ForceRuntime` somente se puder perder as
alterações feitas nela.

### 2. Configurar o encoder

Edite o arquivo externo:

```text
C:\Dev\runtime\mu-097k\encoder\MainInfo.ini
```

Para um servidor local, use `IpAddress=127.0.0.1` e a porta TCP do ConnectServer,
normalmente `IpAddressPort=44405`. `ClientSerial` e `ClientVersion` precisam coincidir
com `ServerSerial` e `ServerVersion` em `runtime/server/GameServer/DATA/GameServerInfo - StartUp.dat`.

### 3. Compilar, implantar e gerar a configuração

O sistema padrão é CMake/Ninja. `BuildDeploy` compila `Main.dll` e `InfoEncoder.exe`,
os copia para o runtime e executa o encoder automaticamente:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

O cliente executável fica em `C:\Dev\runtime\mu-097k\client`. Para gerar novamente
somente o `ClientInfo.bmd` depois de alterar o INI, use:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action Encode
```

O encoder não é interativo quando chamado pelo script. Executado sem argumentos,
`InfoEncoder.exe` mantém o comportamento manual original.

### 4. Debug e Release

No VS Code, `F5` usa a configuração `Client: Debug Main.dll (x86)`. Ela executa a
tarefa `Client: Build + Deploy Debug`, inicia `main.exe` no runtime externo e carrega
os símbolos `Main.pdb`.

Release não é uma segunda configuração de F5. Para preparar uma versão otimizada:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

Também é possível usar a tarefa `Client: Build + Deploy Release`. O PDB não é mantido
no runtime Release.

MSBuild continua disponível como fallback:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy `
  -Configuration Debug -BuildSystem MSBuild
```

As ações `Build`, `Deploy`, `Encode` e `Clean` aceitam o mesmo `-BuildSystem`
usado para compilar. O CMake usa os presets `client-windows-debug` e
`client-windows-release`; os artefatos ficam em `out/build/<preset>/bin`.

## Servidor no WSL2

Use `~/Dev/projects/mu-097k` no Ubuntu. Não compile o mesmo working tree ao mesmo
tempo no Windows e no WSL2.

### 1. Ambiente local

```bash
cd ~/Dev/projects/mu-097k
cp .env.example .env
```

Revise as credenciais e mantenha `PUBLIC_IP=127.0.0.1` para o cliente Windows local.
O arquivo `.env` não deve ser commitado.

### 2. Build nativo opcional

Instale as dependências uma vez:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build libmysqlcppconn-dev
```

Compile as variantes do servidor:

```bash
cmake --preset server-linux-debug
cmake --build --preset server-linux-debug

cmake --preset server-linux-release
cmake --build --preset server-linux-release
```

Os quatro executáveis ficam em `out/build/<preset>/bin`. A ausência do MySQL
Connector/C++ interrompe a configuração.

### 3. Subir e parar o Compose

```bash
docker compose config --quiet
docker compose up --build -d
docker compose ps
```

O fluxo base usa `mysql`, `mu-server` e `mu-web`. O painel responde em
<http://127.0.0.1:8085>; as portas do servidor são `44405/tcp` e `55901/tcp`.
Verifique logs com `docker compose logs --follow --tail=200` e desligue com:

```bash
docker compose down
```

Não use `docker compose down -v` sem confirmar que o volume MySQL pode ser removido.
O editor é opcional e usa `compose.editor.yaml` quando necessário.

## Antes de abrir um PR

Valide a documentação e scripts:

```powershell
pwsh -File .\scripts\validate-repository.ps1
git diff --check
```

No cliente, execute pelo menos um `BuildDeploy` Debug. No servidor, execute os dois
presets quando houver alteração C++ e valide o Compose. Confirme que `git status` está
limpo depois de build, deploy e containers.

Problemas de MSVC normalmente indicam que o Build Tools ou o MSVC 14.44 não está
instalado. Se o runtime já existir, `InitializeRuntime` exige `-ForceRuntime`; isso
evita apagar configurações locais por engano.
