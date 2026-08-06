[CmdletBinding()]
param(
    [string]$RepositoryRoot = (Join-Path $PSScriptRoot "..")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RepositoryRoot = [System.IO.Path]::GetFullPath($RepositoryRoot)

function Get-TrackedFiles {
    param([Parameter(Mandatory)][string]$Pathspec)

    $files = @(& git -C $RepositoryRoot ls-files -- $Pathspec)
    if ($LASTEXITCODE -ne 0) {
        throw "git ls-files failed for '$Pathspec'."
    }

    return $files
}

function Resolve-RepositoryPath {
    param([Parameter(Mandatory)][string]$RelativePath)

    return [System.IO.Path]::GetFullPath((Join-Path $RepositoryRoot $RelativePath))
}

function Read-KeyValueFile {
    param([Parameter(Mandatory)][string]$RelativePath)

    $values = @{}
    foreach ($line in [System.IO.File]::ReadAllLines((Resolve-RepositoryPath $RelativePath))) {
        $match = [regex]::Match($line, '^\s*([^;#\[\]][^=]*?)\s*=\s*(.*?)\s*$')
        if ($match.Success) {
            $values[$match.Groups[1].Value.Trim()] = $match.Groups[2].Value.Trim()
        }
    }

    return $values
}

function Assert-Equal {
    param(
        [Parameter(Mandatory)]$Actual,
        [Parameter(Mandatory)]$Expected,
        [Parameter(Mandatory)][string]$Message
    )

    if ($Actual -ne $Expected) {
        throw "$Message Actual: '$Actual'. Expected: '$Expected'."
    }
}

Write-Host "Validating tracked JSON files."
foreach ($file in Get-TrackedFiles "*.json") {
    $document = [System.Text.Json.JsonDocument]::Parse(
        [System.IO.File]::ReadAllText((Resolve-RepositoryPath $file))
    )
    $document.Dispose()
}

Write-Host "Validating tracked PowerShell files."
foreach ($file in Get-TrackedFiles "*.ps1") {
    $tokens = $null
    $errors = $null
    [System.Management.Automation.Language.Parser]::ParseFile(
        (Resolve-RepositoryPath $file),
        [ref]$tokens,
        [ref]$errors
    ) | Out-Null

    if ($errors.Count -gt 0) {
        $messages = $errors | ForEach-Object { $_.Message }
        throw "PowerShell syntax errors in '$file': $($messages -join '; ')"
    }
}

if (-not $IsWindows) {
    Write-Host "Validating tracked shell scripts."
    foreach ($file in Get-TrackedFiles "*.sh") {
        & bash -n -- (Resolve-RepositoryPath $file)
        if ($LASTEXITCODE -ne 0) {
            throw "Shell syntax validation failed for '$file'."
        }
    }
}
else {
    Write-Host "Shell syntax validation is executed by the Linux CI job."
}

Write-Host "Validating local Markdown links."
foreach ($file in Get-TrackedFiles "*.md") {
    $absoluteFile = Resolve-RepositoryPath $file
    $content = [System.IO.File]::ReadAllText($absoluteFile)

    foreach ($match in [regex]::Matches($content, '\[[^\]]+\]\(([^)]+)\)')) {
        $target = $match.Groups[1].Value.Trim()
        if ($target -match '^(https?://|mailto:|#)') {
            continue
        }

        $pathPart = ($target -split '#', 2)[0].Trim('<', '>')
        if ([string]::IsNullOrWhiteSpace($pathPart)) {
            continue
        }

        $decodedPath = [System.Uri]::UnescapeDataString($pathPart)
        $resolvedTarget = [System.IO.Path]::GetFullPath(
            (Join-Path ([System.IO.Path]::GetDirectoryName($absoluteFile)) $decodedPath)
        )

        if (-not (Test-Path -LiteralPath $resolvedTarget)) {
            throw "Broken local Markdown link in '$file': '$target'."
        }
    }
}

Write-Host "Validating repository-local agent skills."
$skillEntries = @(Get-TrackedFiles ".agents/skills")
$skillDirectories = @(
    $skillEntries |
        ForEach-Object {
            if ($_ -match '^\.agents/skills/([^/]+)/') {
                $Matches[1]
            }
        } |
        Sort-Object -Unique
)

foreach ($skillDirectory in $skillDirectories) {
    $skillFile = ".agents/skills/$skillDirectory/SKILL.md"
    if ($skillEntries -notcontains $skillFile) {
        throw "Agent skill '$skillDirectory' does not contain a tracked SKILL.md."
    }

    $lines = [System.IO.File]::ReadAllLines((Resolve-RepositoryPath $skillFile))
    if ($lines.Count -lt 4 -or $lines[0].Trim() -ne "---") {
        throw "Agent skill '$skillDirectory' has invalid front matter."
    }

    $closingDelimiter = -1
    for ($lineIndex = 1; $lineIndex -lt $lines.Count; $lineIndex++) {
        if ($lines[$lineIndex].Trim() -eq "---") {
            $closingDelimiter = $lineIndex
            break
        }
    }

    if ($closingDelimiter -lt 2) {
        throw "Agent skill '$skillDirectory' has no closing front-matter delimiter."
    }

    $metadata = @{}
    for ($lineIndex = 1; $lineIndex -lt $closingDelimiter; $lineIndex++) {
        $match = [regex]::Match(
            $lines[$lineIndex],
            '^(?<key>[A-Za-z0-9_-]+):\s*(?<value>.+?)\s*$'
        )

        if ($match.Success) {
            $metadata[$match.Groups['key'].Value] = $match.Groups['value'].Value
        }
    }

    if (-not $metadata.ContainsKey("name")) {
        throw "Agent skill '$skillDirectory' is missing front-matter field 'name'."
    }

    if (-not $metadata.ContainsKey("description") -or
        [string]::IsNullOrWhiteSpace($metadata["description"])) {
        throw "Agent skill '$skillDirectory' is missing front-matter field 'description'."
    }

    Assert-Equal $metadata["name"] $skillDirectory `
        "Agent skill name does not match its directory."
}

Write-Host "Validating client and server configuration invariants."
$clientInfo = Read-KeyValueFile "runtime/encoder/MainInfo.ini"
$serverInfo = Read-KeyValueFile "runtime/server/GameServer/DATA/GameServerInfo - StartUp.dat"
$connectServerInfo = Read-KeyValueFile "runtime/server/ConnectServer/ConnectServer.ini"
$dataServerInfo = Read-KeyValueFile "runtime/server/MySQL/DataServer/DataServer.ini"

Assert-Equal $clientInfo.ClientSerial $serverInfo.ServerSerial "ClientSerial and ServerSerial differ."
Assert-Equal $clientInfo.ClientVersion $serverInfo.ServerVersion "ClientVersion and ServerVersion differ."
Assert-Equal $clientInfo.IpAddressPort $connectServerInfo.ConnectServerPortTCP "Client and ConnectServer TCP ports differ."
Assert-Equal $clientInfo.EnableSpecialCharacters $dataServerInfo.EnableSpecialCharacters "Special-character flags differ."

Write-Host "Validating the duplicated ClientInfo binary layout."
$encoderSource = [System.IO.File]::ReadAllText(
    (Resolve-RepositoryPath "src/client/InfoEncoder/InfoEncoder.cpp")
)
$clientHeader = [System.IO.File]::ReadAllText(
    (Resolve-RepositoryPath "src/client/Main/Protect.h")
)
$structPattern = '(?s)struct\s+MAIN_FILE_INFO\s*\{(?<body>.*?)\};'
$encoderStruct = [regex]::Match($encoderSource, $structPattern)
$clientStruct = [regex]::Match($clientHeader, $structPattern)

if (-not $encoderStruct.Success -or -not $clientStruct.Success) {
    throw "MAIN_FILE_INFO was not found in both encoder and client sources."
}

$normalize = {
    param([string]$Value)
    return [regex]::Replace($Value, '\s+', '')
}

Assert-Equal (& $normalize $encoderStruct.Groups['body'].Value) `
    (& $normalize $clientStruct.Groups['body'].Value) `
    "MAIN_FILE_INFO layouts differ between encoder and client."

$trackedEnvironmentFiles = @(Get-TrackedFiles ".env") + @(Get-TrackedFiles ".env.*")
$unexpectedEnvironmentFiles = @($trackedEnvironmentFiles | Where-Object { $_ -ne ".env.example" })
if ($unexpectedEnvironmentFiles.Count -gt 0) {
    throw "Environment files with possible secrets are tracked: $($unexpectedEnvironmentFiles -join ', ')."
}

Write-Host "Repository validation completed successfully."
