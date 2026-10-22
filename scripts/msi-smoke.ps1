#Requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$MsiPath,
    [string]$WorkDirectory = (Join-Path $env:TEMP ("cproject-msi-smoke-" + [guid]::NewGuid().ToString("N"))),
    [switch]$Keep
)

$ErrorActionPreference = "Stop"

$msi = (Resolve-Path -LiteralPath $MsiPath).Path
if (-not (Test-Path -LiteralPath $msi -PathType Leaf)) {
    throw "MSI not found: $MsiPath"
}

New-Item -ItemType Directory -Path $WorkDirectory -Force | Out-Null
$extract = Join-Path $WorkDirectory "install"
New-Item -ItemType Directory -Path $extract -Force | Out-Null

Write-Host "Administrative MSI extraction: $msi -> $extract"
$arguments = @("/a", "`"$msi`"", "/qn", "TARGETDIR=`"$extract`"")
$process = Start-Process -FilePath "msiexec.exe" -ArgumentList $arguments -Wait -PassThru
if ($process.ExitCode -ne 0) {
    throw "msiexec administrative install failed with exit code $($process.ExitCode)"
}

$wizard = Get-ChildItem -Path $extract -Recurse -Filter "cproject.exe" -File |
    Select-Object -First 1
$share = Get-ChildItem -Path $extract -Recurse -Directory -Filter "c-project-standard" |
    Select-Object -First 1
if (-not $wizard) {
    throw "Installed cproject.exe not found under $extract"
}
if (-not $share) {
    throw "Installed share/c-project-standard not found under $extract"
}
Write-Host "Installed wizard: $($wizard.FullName)"
Write-Host "Installed template: $($share.FullName)"

$forbidden = @(
    "installer",
    "src/wizard",
    "tests/wizard",
    "cmake/Wizard.cmake",
    "cmake/PackagingMSI.cmake",
    "docs/maintainer",
    "scripts/package.ps1",
    "scripts/publish.ps1",
    "scripts/gate_floor_probe.sh",
    "res/run-wizard.bat"
)
foreach ($relative in $forbidden) {
    if (Test-Path -LiteralPath (Join-Path $share.FullName $relative)) {
        throw "Template-only path leaked into installed payload: $relative"
    }
}

$output = Join-Path $WorkDirectory "generated"
Push-Location $WorkDirectory
try {
    $json = & $wizard.FullName new MsiProbe --output $output --yes --json
    if ($LASTEXITCODE -ne 0) {
        throw "Installed wizard failed to generate a project (exit $LASTEXITCODE)"
    }
} finally {
    Pop-Location
}
$payload = $json | ConvertFrom-Json
if (-not $payload.ok) {
    throw "Installed wizard returned an error payload: $json"
}
$resolvedTemplate = [System.IO.Path]::GetFullPath([string]$payload.template).TrimEnd([char[]]@('\', '/'))
$expectedTemplate = [System.IO.Path]::GetFullPath($share.FullName).TrimEnd([char[]]@('\', '/'))
if ($resolvedTemplate -ne $expectedTemplate) {
    throw "Wizard used template '$resolvedTemplate' instead of installed '$expectedTemplate'"
}
Write-Host "Generated project: $($payload.destination)"

$project = Join-Path $output "MsiProbe"
if (-not (Test-Path -LiteralPath (Join-Path $project "CMakeLists.txt"))) {
    throw "Generated project is missing CMakeLists.txt"
}
if (Test-Path -LiteralPath (Join-Path $project "templates")) {
    throw "Generation asset directory leaked into the generated project"
}
if (Test-Path -LiteralPath (Join-Path $project "docs/maintainer")) {
    throw "Maintainer documentation leaked into the generated project"
}

Push-Location $project
try {
    cmake --preset ninja-debug
    if ($LASTEXITCODE -ne 0) {
        throw "Configure failed"
    }
    cmake --build --preset ninja-debug
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    ctest --preset ninja-debug --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        throw "CTest failed"
    }
} finally {
    Pop-Location
}

# Also verify the installed library package, not just the template payload.
$installRoot = Split-Path -Parent (Split-Path -Parent $share.FullName)
$consumer = Join-Path $WorkDirectory "consumer"
New-Item -ItemType Directory -Path $consumer -Force | Out-Null
@'
cmake_minimum_required(VERSION 3.21)
project(MsiConsumer C)
find_package(CProjectStandard CONFIG REQUIRED)
add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE CProjectStandard::cproject_core)
'@ | Set-Content -LiteralPath (Join-Path $consumer "CMakeLists.txt") -Encoding utf8
@'
#include <cproject/example.h>
#include <stdio.h>
int main(void) {
    cproject_version_t version = cproject_version();
    printf("%d.%d.%d\n", version.major, version.minor, version.patch);
    return 0;
}
'@ | Set-Content -LiteralPath (Join-Path $consumer "main.c") -Encoding utf8
cmake -G Ninja -S $consumer -B (Join-Path $consumer "build") `
    "-DCMAKE_BUILD_TYPE=Debug" "-DCMAKE_PREFIX_PATH=$installRoot"
if ($LASTEXITCODE -ne 0) {
    throw "Installed package find_package configure failed"
}
cmake --build (Join-Path $consumer "build")
if ($LASTEXITCODE -ne 0) {
    throw "Installed package consumer build failed"
}

Write-Host "MSI smoke passed"
if (-not $Keep) {
    Remove-Item -LiteralPath $WorkDirectory -Recurse -Force
}
