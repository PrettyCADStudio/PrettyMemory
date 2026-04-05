[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Path
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$content = Get-Content -LiteralPath $Path -Raw
$content = [regex]::Replace(
    $content,
    '@~chinese\{([^}]*)\}',
    '\htmlonly <span class="doc-lang doc-lang-chinese">\endhtmlonly $1 \htmlonly </span>\endhtmlonly'
)
$content = [regex]::Replace(
    $content,
    '@~english\{([^}]*)\}',
    '\htmlonly <span class="doc-lang doc-lang-english">\endhtmlonly $1 \htmlonly </span>\endhtmlonly'
)

[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
[Console]::Write($content)
