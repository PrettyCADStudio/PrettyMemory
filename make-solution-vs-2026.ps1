[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\vs2026'

New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

cmake -S $repoRoot -B $buildDir -G 'Visual Studio 18 2026' -A x64

