#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Check or format the project's C source and header files.

.DESCRIPTION
    Scans src/, include/, and tests/ by default (excluding tests/vendor/), or a
    single directory passed with -Path.

    -Check   Verify formatting without modifying files. Exits 2 if any file
             needs formatting, 1 on error, 0 when everything is formatted.
    -Format  Rewrite files in place with `clang-format -i`.

.EXAMPLE
    ./scripts/format.ps1 -Check
    ./scripts/format.ps1 -Format
    ./scripts/format.ps1 -Path src -Check

.NOTES
    Requires clang-format on PATH. Vendored third-party code under tests/vendor/
    is always excluded.
#>

[CmdletBinding()]
param(
    [string]$Path,
    [switch]$Check,
    [switch]$Format
)

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

if ($Check -and $Format) {
    Write-Error "-Check and -Format are mutually exclusive."
    exit 1
}
if (-not $Check -and -not $Format) {
    Write-Error "Specify either -Check or -Format."
    exit 1
}

Push-Location $RepoRoot
try {
    if (-not (Get-Command clang-format -ErrorAction SilentlyContinue)) {
        Write-Error "clang-format was not found on PATH."
        exit 1
    }

    if ($Path) {
        $roots = @($Path)
    } else {
        $roots = @("include", "src", "tests")
    }

    $files = @()
    foreach ($root in $roots) {
        if (Test-Path -LiteralPath $root) {
            $files += Get-ChildItem -LiteralPath $root -Recurse -File -Include "*.c", "*.h" |
                Where-Object { $_.FullName -notlike "*\tests\vendor\*" }
        }
    }
    $files = @($files | Sort-Object FullName)

    if ($files.Count -eq 0) {
        Write-Host "No *.c or *.h files found."
        exit 0
    }

    if ($Format) {
        $formatArgs = @("-i")
        $formatArgs += $files.FullName
        & clang-format @formatArgs
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
        Write-Host "Formatted $($files.Count) file(s)."
        exit 0
    }

    # Check mode. clang-format --dry-run --Werror exits non-zero when any file
    # would be reformatted and lists those files on stderr.
    $checkArgs = @("--dry-run", "--Werror")
    $checkArgs += $files.FullName
    & clang-format @checkArgs 2>&1 | Out-Host
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "Some file(s) need formatting. Run ./scripts/format.ps1 -Format to fix."
        exit 2
    }

    Write-Host "All $($files.Count) file(s) match clang-format."
    exit 0
} finally {
    Pop-Location
}
