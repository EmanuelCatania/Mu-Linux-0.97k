# Cliente Windows

## Ambiente

Use o clone em `C:\Dev\projects\mu-097k` com PowerShell 7, VS Code e Visual Studio
Build Tools. O script localiza o Build Tools 2026 com `vswhere` e seleciona MSVC 14.44
para compilar os projetos Win32.

## Runtime externo

Inicialize uma única vez:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
```

O destino padrão é `C:\Dev\runtime\mu-097k`. Use `-ForceRuntime` somente para recriar
completamente essa cópia a partir dos templates rastreados.

## Configuração do cliente

Edite apenas:

```text
C:\Dev\runtime\mu-097k\encoder\MainInfo.ini
```

Depois gere e copie a configuração:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action Encode
```

`ClientSerial` e `ClientVersion` devem corresponder a `ServerSerial` e
`ServerVersion` no GameServer.

## Build e debug

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

No VS Code, selecione `Client: Debug Main.dll (x86)` para compilar, implantar e iniciar
o cliente com `cppvsdbg` e o PDB ao lado de `Main.dll`.
