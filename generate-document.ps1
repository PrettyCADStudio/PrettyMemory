[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\docs'
$docDir = Join-Path $repoRoot 'doc'
$indexFile = Join-Path $docDir 'html\index.html'

New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
New-Item -ItemType Directory -Path $docDir -Force | Out-Null

try {
    Write-Host 'Generating API documentation...'
    cmake -S $repoRoot -B $buildDir -DBUILD_DOCS=ON -DDOXYGEN_OUTPUT_DIRECTORY="$docDir"
    cmake --build $buildDir --target docs

    if (-not (Test-Path -LiteralPath $indexFile)) {
        throw "Documentation generation finished, but '$indexFile' was not found."
    }

    Write-Host "Documentation generated: $indexFile"
}
finally {
    Read-Host 'Press Enter to exit'
}
