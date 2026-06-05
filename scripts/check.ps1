[CmdletBinding()]
param(
    [string]$Preset = "debug",
    [switch]$SkipFormat
)

$ErrorActionPreference = "Stop"

if (-not $SkipFormat -and (Get-Command clang-format -ErrorAction SilentlyContinue)) {
    $files = Get-ChildItem -Path include,src,tests -Recurse -File -Include *.c,*.h |
        Sort-Object FullName
    if ($files.Count -gt 0) {
        & clang-format --dry-run --Werror @($files.FullName)
    }
} elseif (-not $SkipFormat) {
    Write-Host "Skipping clang-format: command not found."
}

cmake --preset $Preset
cmake --build --preset $Preset
ctest --preset $Preset --output-on-failure
