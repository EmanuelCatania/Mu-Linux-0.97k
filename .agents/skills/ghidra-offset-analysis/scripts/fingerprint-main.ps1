[CmdletBinding()]
param(
    [string]$Path = "runtime/client/main.exe",
    [string]$ComparePath,
    [switch]$AsJson
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RepositoryRoot = [System.IO.Path]::GetFullPath(
    (Join-Path $PSScriptRoot "../../../..")
)

function Resolve-InputPath {
    param([Parameter(Mandatory)][string]$InputPath)

    if ([System.IO.Path]::IsPathRooted($InputPath)) {
        return (Resolve-Path -LiteralPath $InputPath).Path
    }

    return (Resolve-Path -LiteralPath (
        Join-Path $RepositoryRoot $InputPath
    )).Path
}

function Get-Pe32Fingerprint {
    param([Parameter(Mandatory)][string]$FilePath)

    $resolvedPath = Resolve-InputPath -InputPath $FilePath
    $fileInfo = Get-Item -LiteralPath $resolvedPath
    $stream = [System.IO.File]::OpenRead($resolvedPath)
    $reader = [System.IO.BinaryReader]::new($stream)

    try {
        if ($reader.ReadUInt16() -ne 0x5A4D) {
            throw "'$resolvedPath' does not contain an MZ header."
        }

        $stream.Position = 0x3C
        $peOffset = $reader.ReadUInt32()
        if ($peOffset -gt ($stream.Length - 24)) {
            throw "'$resolvedPath' contains an invalid PE header offset."
        }

        $stream.Position = $peOffset
        if ($reader.ReadUInt32() -ne 0x00004550) {
            throw "'$resolvedPath' does not contain a valid PE signature."
        }

        $machine = $reader.ReadUInt16()
        $numberOfSections = $reader.ReadUInt16()
        $timeDateStamp = $reader.ReadUInt32()

        $stream.Position = $peOffset + 20
        $sizeOfOptionalHeader = $reader.ReadUInt16()
        $characteristics = $reader.ReadUInt16()

        $optionalHeaderOffset = $peOffset + 24
        if (($optionalHeaderOffset + $sizeOfOptionalHeader) -gt $stream.Length) {
            throw "'$resolvedPath' contains a truncated optional header."
        }

        $stream.Position = $optionalHeaderOffset
        $optionalMagic = $reader.ReadUInt16()
        if ($machine -ne 0x014C -or $optionalMagic -ne 0x010B) {
            throw "'$resolvedPath' is not the expected PE32/x86 executable."
        }

        $stream.Position = $optionalHeaderOffset + 16
        $entryPointRva = $reader.ReadUInt32()

        $stream.Position = $optionalHeaderOffset + 28
        $imageBase = $reader.ReadUInt32()
    }
    finally {
        $reader.Dispose()
        $stream.Dispose()
    }

    $gitBlob = $null
    & git -C $RepositoryRoot rev-parse --is-inside-work-tree 2>$null | Out-Null
    if ($LASTEXITCODE -eq 0) {
        $blobOutput = @(& git -C $RepositoryRoot hash-object -- $resolvedPath)
        if ($LASTEXITCODE -eq 0 -and $blobOutput.Count -gt 0) {
            $gitBlob = $blobOutput[0].Trim()
        }
    }

    $timestampUtc = [DateTimeOffset]::FromUnixTimeSeconds(
        [int64]$timeDateStamp
    ).UtcDateTime

    return [PSCustomObject][ordered]@{
        Path = $resolvedPath
        SHA256 = (Get-FileHash -LiteralPath $resolvedPath -Algorithm SHA256).Hash
        FileSize = $fileInfo.Length
        GitBlob = $gitBlob
        Architecture = "PE32 / x86"
        Machine = ("0x{0:X4}" -f $machine)
        NumberOfSections = $numberOfSections
        PETimestamp = ("0x{0:X8}" -f $timeDateStamp)
        PETimestampUtc = $timestampUtc.ToString("yyyy-MM-dd HH:mm:ss 'UTC'")
        PreferredImageBase = ("0x{0:X8}" -f $imageBase)
        EntryPointRva = ("0x{0:X8}" -f $entryPointRva)
        OptionalHeaderSize = $sizeOfOptionalHeader
        Characteristics = ("0x{0:X4}" -f $characteristics)
    }
}

$primary = Get-Pe32Fingerprint -FilePath $Path
$comparison = $null
$matchesPrimary = $null

if (-not [string]::IsNullOrWhiteSpace($ComparePath)) {
    $comparison = Get-Pe32Fingerprint -FilePath $ComparePath
    $matchesPrimary = (
        $primary.SHA256 -eq $comparison.SHA256 -and
        $primary.FileSize -eq $comparison.FileSize
    )
}

$result = [PSCustomObject][ordered]@{
    Path = $primary.Path
    SHA256 = $primary.SHA256
    FileSize = $primary.FileSize
    GitBlob = $primary.GitBlob
    Architecture = $primary.Architecture
    Machine = $primary.Machine
    NumberOfSections = $primary.NumberOfSections
    PETimestamp = $primary.PETimestamp
    PETimestampUtc = $primary.PETimestampUtc
    PreferredImageBase = $primary.PreferredImageBase
    EntryPointRva = $primary.EntryPointRva
    OptionalHeaderSize = $primary.OptionalHeaderSize
    Characteristics = $primary.Characteristics
    ComparisonPath = if ($null -ne $comparison) { $comparison.Path } else { $null }
    ComparisonSHA256 = if ($null -ne $comparison) { $comparison.SHA256 } else { $null }
    ComparisonFileSize = if ($null -ne $comparison) { $comparison.FileSize } else { $null }
    MatchesPrimary = $matchesPrimary
}

if ($AsJson) {
    $result | ConvertTo-Json -Depth 4
}
else {
    $result
}
