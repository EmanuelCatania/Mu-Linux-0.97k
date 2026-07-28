[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [ValidateSet("InitializeRuntime", "Build", "Deploy", "BuildDeploy", "Encode", "Clean")]
    [string]$Action,

    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidatePattern("^[A-Za-z0-9.-]+$")]
    [string]$ServerAddress = "127.0.0.1",

    [ValidateRange(1, 65535)]
    [int]$ServerPort = 44405,

    [string]$RuntimeRoot,

    [switch]$ForceRuntime
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$devRoot = Split-Path (Split-Path $repoRoot -Parent) -Parent
$externalRuntimeBase = [System.IO.Path]::GetFullPath((Join-Path $devRoot "runtime"))
$clientTemplate = Join-Path $repoRoot "runtime\client"
$encoderTemplate = Join-Path $repoRoot "runtime\encoder"

if ([string]::IsNullOrWhiteSpace($RuntimeRoot)) {
    $RuntimeRoot = Join-Path $externalRuntimeBase "mu-097k"
}

$RuntimeRoot = [System.IO.Path]::GetFullPath($RuntimeRoot)
$runtimePrefix = $externalRuntimeBase.TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar

if (-not $RuntimeRoot.StartsWith($runtimePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "RuntimeRoot must be a child of '$externalRuntimeBase'. Resolved value: '$RuntimeRoot'."
}

$runtimeClient = Join-Path $RuntimeRoot "client"
$runtimeEncoder = Join-Path $RuntimeRoot "encoder"
$solutionPath = Join-Path $repoRoot "src\client\Client.sln"

function Get-BuildEnvironment {
    $vsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"

    if (-not (Test-Path -LiteralPath $vsWhere -PathType Leaf)) {
        throw "vswhere.exe was not found. Install Visual Studio Build Tools 2026 with Desktop development with C++."
    }

    $vsRoot = & $vsWhere -latest -products * -version "[18.0,19.0)" `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath | Select-Object -First 1

    if ([string]::IsNullOrWhiteSpace($vsRoot)) {
        throw "Visual Studio Build Tools 2026 with MSVC x86/x64 was not found."
    }

    $vsRoot = $vsRoot.Trim()

    $versionFile = Join-Path $vsRoot "VC\Auxiliary\Build\Microsoft.VCToolsVersion.v143.default.txt"
    if (-not (Test-Path -LiteralPath $versionFile -PathType Leaf)) {
        throw "The installed Build Tools do not expose a v143 compiler version file: '$versionFile'."
    }

    $vcToolsVersion = (Get-Content -Raw -LiteralPath $versionFile).Trim()
    $vcToolsDirectory = Join-Path $vsRoot "VC\Tools\MSVC\$vcToolsVersion"
    $msBuild = Join-Path $vsRoot "MSBuild\Current\Bin\MSBuild.exe"

    if (-not (Test-Path -LiteralPath $vcToolsDirectory -PathType Container)) {
        throw "MSVC $vcToolsVersion is declared but its tool directory was not found: '$vcToolsDirectory'."
    }

    if (-not (Test-Path -LiteralPath $msBuild -PathType Leaf)) {
        throw "MSBuild was not found: '$msBuild'."
    }

    return [pscustomobject]@{
        VsRoot         = $vsRoot
        MsBuild        = $msBuild
        VCToolsVersion = $vcToolsVersion
    }
}

function Invoke-RobocopyMirror {
    param(
        [Parameter(Mandatory)]
        [string]$Source,

        [Parameter(Mandatory)]
        [string]$Destination
    )

    & robocopy.exe $Source $Destination /MIR /R:2 /W:1 /NFL /NDL /NJH /NJS /NP
    $robocopyExitCode = $LASTEXITCODE

    if ($robocopyExitCode -ge 8) {
        throw "robocopy failed with exit code $robocopyExitCode while mirroring '$Source' to '$Destination'."
    }
}

function Set-IniValue {
    param(
        [Parameter(Mandatory)]
        [string]$Path,

        [Parameter(Mandatory)]
        [string]$Key,

        [Parameter(Mandatory)]
        [string]$Value
    )

    $content = [System.IO.File]::ReadAllText($Path)
    $pattern = "(?m)^$([regex]::Escape($Key))=.*$"

    if (-not [regex]::IsMatch($content, $pattern)) {
        throw "Key '$Key' was not found in '$Path'."
    }

    $updated = [regex]::Replace($content, $pattern, "$Key=$Value", 1)
    [System.IO.File]::WriteAllText($Path, $updated, [System.Text.UTF8Encoding]::new($false))
}

function Initialize-Runtime {
    if ((Test-Path -LiteralPath $RuntimeRoot) -and -not $ForceRuntime) {
        throw "Runtime '$RuntimeRoot' already exists. Use -ForceRuntime to recreate it from tracked Client and Encoder files."
    }

    New-Item -ItemType Directory -Path $RuntimeRoot -Force | Out-Null
    Invoke-RobocopyMirror -Source $clientTemplate -Destination $runtimeClient
    Invoke-RobocopyMirror -Source $encoderTemplate -Destination $runtimeEncoder

    $mainInfo = Join-Path $runtimeEncoder "MainInfo.ini"
    Set-IniValue -Path $mainInfo -Key "IpAddress" -Value $ServerAddress
    Set-IniValue -Path $mainInfo -Key "IpAddressPort" -Value $ServerPort.ToString()

    Write-Host "Runtime initialized at '$RuntimeRoot' for ${ServerAddress}:$ServerPort."
}

function Invoke-ClientBuild {
    param(
        [Parameter(Mandatory)]
        [ValidateSet("Build", "Clean")]
        [string]$Target
    )

    $buildEnvironment = Get-BuildEnvironment
    $arguments = @(
        $solutionPath
        "/m"
        "/t:$Target"
        "/p:Configuration=$Configuration"
        "/p:Platform=Win32"
        "/p:PlatformToolset=v145"
        "/p:VCToolsVersion=$($buildEnvironment.VCToolsVersion)"
        "/verbosity:minimal"
    )

    Write-Host "MSBuild $Target ($Configuration|Win32) with MSVC $($buildEnvironment.VCToolsVersion)."
    & $buildEnvironment.MsBuild @arguments

    if ($LASTEXITCODE -ne 0) {
        throw "MSBuild $Target failed with exit code $LASTEXITCODE."
    }
}

function Assert-RuntimeInitialized {
    $requiredPaths = @(
        (Join-Path $runtimeClient "main.exe")
        (Join-Path $runtimeEncoder "MainInfo.ini")
        (Join-Path $runtimeEncoder "Client\main.exe")
    )

    foreach ($path in $requiredPaths) {
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "Runtime is not initialized or is incomplete. Missing '$path'. Run -Action InitializeRuntime first."
        }
    }
}

function Invoke-Encoder {
    Assert-RuntimeInitialized

    $encoderExecutable = Join-Path $runtimeEncoder "InfoEncoder.exe"
    if (-not (Test-Path -LiteralPath $encoderExecutable -PathType Leaf)) {
        throw "InfoEncoder.exe was not found in '$runtimeEncoder'. Build and deploy it first."
    }

    Push-Location $runtimeEncoder
    try {
        & $encoderExecutable
        if ($LASTEXITCODE -ne 0) {
            throw "InfoEncoder.exe failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Pop-Location
    }

    $generatedClientInfo = Join-Path $runtimeEncoder "Client\Data\Local\ClientInfo.bmd"
    if (-not (Test-Path -LiteralPath $generatedClientInfo -PathType Leaf)) {
        throw "InfoEncoder.exe completed but did not create '$generatedClientInfo'."
    }

    $clientInfoDestination = Join-Path $runtimeClient "Data\Local\ClientInfo.bmd"
    New-Item -ItemType Directory -Path (Split-Path $clientInfoDestination -Parent) -Force | Out-Null
    Copy-Item -LiteralPath $generatedClientInfo -Destination $clientInfoDestination -Force
    Write-Host "ClientInfo.bmd generated and copied to the client runtime."
}

function Deploy-Client {
    Assert-RuntimeInitialized

    $outputRoot = Join-Path $repoRoot "src\client\bin\$Configuration"
    $mainDll = Join-Path $outputRoot "Main\Main.dll"
    $mainPdb = Join-Path $outputRoot "Main\Main.pdb"
    $infoEncoder = Join-Path $outputRoot "InfoEncoder\InfoEncoder.exe"

    foreach ($artifact in @($mainDll, $infoEncoder)) {
        if (-not (Test-Path -LiteralPath $artifact -PathType Leaf)) {
            throw "Build artifact '$artifact' was not found. Run -Action Build first."
        }
    }

    Copy-Item -LiteralPath $mainDll -Destination (Join-Path $runtimeClient "Main.dll") -Force
    Copy-Item -LiteralPath $mainDll -Destination (Join-Path $runtimeEncoder "Client\Main.dll") -Force
    Copy-Item -LiteralPath $infoEncoder -Destination (Join-Path $runtimeEncoder "InfoEncoder.exe") -Force

    $runtimePdb = Join-Path $runtimeClient "Main.pdb"
    if ($Configuration -eq "Debug") {
        if (-not (Test-Path -LiteralPath $mainPdb -PathType Leaf)) {
            throw "Debug symbols were not found: '$mainPdb'."
        }

        Copy-Item -LiteralPath $mainPdb -Destination $runtimePdb -Force
    }
    elseif (Test-Path -LiteralPath $runtimePdb -PathType Leaf) {
        Remove-Item -LiteralPath $runtimePdb -Force
    }

    Invoke-Encoder
    Write-Host "$Configuration client artifacts deployed to '$runtimeClient'."
}

switch ($Action) {
    "InitializeRuntime" { Initialize-Runtime }
    "Build" { Invoke-ClientBuild -Target Build }
    "Deploy" { Deploy-Client }
    "BuildDeploy" {
        Invoke-ClientBuild -Target Build
        Deploy-Client
    }
    "Encode" { Invoke-Encoder }
    "Clean" { Invoke-ClientBuild -Target Clean }
}
