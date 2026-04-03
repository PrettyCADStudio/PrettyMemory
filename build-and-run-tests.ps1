[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$preferredBuildDirs = @(
    (Join-Path $repoRoot 'build\vs2026'),
    (Join-Path $repoRoot 'build\vs2022')
)

$buildDir = $null

foreach ($directory in $preferredBuildDirs) {
    if (-not (Test-Path -LiteralPath $directory)) {
        continue
    }

    $solution = Get-ChildItem -LiteralPath $directory -File | Where-Object {
        $_.Extension -in '.slnx', '.sln'
    } | Select-Object -First 1

    if ($solution) {
        $buildDir = $directory
        break
    }
}

if (-not $buildDir) {
    & (Join-Path $repoRoot 'make-solution-vs-2026.ps1')
    $buildDir = Join-Path $repoRoot 'build\vs2026'
}

try {
    Write-Host "Using build directory: $buildDir"
    Write-Host 'Building solution in Release configuration...'
    cmake --build $buildDir --config Release

    Write-Host 'Running all tests...'
    ctest --test-dir $buildDir -C Release --output-on-failure
}
finally {
    Read-Host 'Press Enter to exit'
}
