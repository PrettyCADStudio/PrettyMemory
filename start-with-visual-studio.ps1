[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$preferredSearchOrder = @(
    (Join-Path $repoRoot 'build\vs2026'),
    (Join-Path $repoRoot 'build\vs2022')
)

$solution = $null

foreach ($directory in $preferredSearchOrder) {
    if (-not (Test-Path -LiteralPath $directory)) {
        continue
    }

    $solution = Get-ChildItem -LiteralPath $directory -File -Filter '*.slnx' | Select-Object -First 1
    if (-not $solution) {
        $solution = Get-ChildItem -LiteralPath $directory -File -Filter '*.sln' | Select-Object -First 1
    }

    if ($solution) {
        break
    }
}

if (-not $solution) {
    throw "No Visual Studio solution was found under build\vs2026 or build\vs2022. Run .\make-solution-vs-2026.ps1 or .\make-solution-vs-2022.ps1 first."
}

Start-Process -FilePath $solution.FullName

