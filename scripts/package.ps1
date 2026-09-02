#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Package a Release build into a distributable zip archive.

.DESCRIPTION
    Installs the selected build into a staging directory, smoke-checks the
    cproject wizard, then zips the full install tree (bin/include/lib/lib/cmake/
    and the template sources under share/) plus a SHA-256 checksum.

    Local only — no network access. Publishing to GitHub is handled by
    scripts/publish.ps1. The WiX/MSI installer remains available via CPack.

    Usage:
      cmake --preset ninja-release
      cmake --build --preset ninja-release
      ./scripts/package.ps1
#>

[CmdletBinding()]
param(
    [string]$Preset = "ninja-release"
)

$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

Push-Location $RepoRoot
try {

    function Invoke-Captured {
        param(
            [Parameter(Mandatory = $true)]
            [string]$Command,

            [Parameter(Mandatory = $true)]
            [string[]]$Arguments
        )

        # Native commands writing to stderr can surface as NativeCommandError
        # under $ErrorActionPreference="Stop" on PowerShell 5.1. Drop to Continue
        # for the call and judge success by $LASTEXITCODE instead.
        $saved = $ErrorActionPreference
        $ErrorActionPreference = "Continue"
        try {
            $output = @(& $Command @Arguments 2>&1)
            $code = $LASTEXITCODE
        } finally {
            $ErrorActionPreference = $saved
        }
        if ($code -ne 0) {
            throw "$Command $($Arguments -join ' ') failed with exit code $code."
        }
        return $output
    }

    $wizard = Join-Path "build" (Join-Path $Preset "bin\cproject.exe")
    if (-not (Test-Path -LiteralPath $wizard -PathType Leaf)) {
        throw "cproject.exe not found: $wizard. Build it first: cmake --preset $Preset; cmake --build --preset $Preset"
    }

    $version = ((Invoke-Captured -Command $wizard -Arguments @("--version")) -join "`n").Trim()
    if ([string]::IsNullOrWhiteSpace($version)) {
        throw "Could not read a version from '$wizard --version'."
    }
    $version = $version -replace '[^\w.\-]', '-'
    Write-Host "Packaging version: $version"

    $distDir = Join-Path $RepoRoot "dist"
    $stageDir = Join-Path $RepoRoot "build\$Preset\package-stage"
    if (Test-Path -LiteralPath $stageDir) {
        Remove-Item -LiteralPath $stageDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $distDir, $stageDir | Out-Null

    # Full install tree: bin/ include/ lib/ lib/cmake/ share/.
    Invoke-Captured -Command "cmake" -Arguments @("--install", "build\$Preset", "--prefix", $stageDir) | Out-Null

    # Smoke check: the installed wizard must report the same version.
    $installedWizard = Join-Path $stageDir "bin\cproject.exe"
    if (-not (Test-Path -LiteralPath $installedWizard -PathType Leaf)) {
        throw "Installed wizard not found: $installedWizard"
    }
    $installedVersion = ((Invoke-Captured -Command $installedWizard -Arguments @("--version")) -join "`n").Trim()
    if ($installedVersion -ne $version) {
        throw "Smoke check failed: installed wizard reports '$installedVersion', expected '$version'."
    }
    Write-Host "Smoke check passed."

    $zipName = "cproject-standard-win64-$version.zip"
    $zipPath = Join-Path $distDir $zipName
    if (Test-Path -LiteralPath $zipPath) {
        Remove-Item -LiteralPath $zipPath -Force
    }
    Compress-Archive -Path (Join-Path $stageDir "*") -DestinationPath $zipPath -Force

    $sumsPath = Join-Path $distDir "SHA256SUMS.txt"
    $hash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()
    "$hash  $zipName" | Set-Content -LiteralPath $sumsPath -Encoding ascii

    Write-Host "Packaged: $zipPath"
    Write-Host "SHA256:   $hash"
} finally {
    Pop-Location
}
