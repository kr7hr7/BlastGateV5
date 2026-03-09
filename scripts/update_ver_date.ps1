param(
    [string]$TargetFile = "src/globals.cpp"
)

$repoRoot = Split-Path -Parent $PSScriptRoot
$filePath = Join-Path $repoRoot $TargetFile

if (-not (Test-Path $filePath)) {
    Write-Error "Version file not found: $filePath"
    exit 1
}

$content = Get-Content -Path $filePath -Raw
$today = Get-Date -Format "MM_dd_yyyy"
$pattern = '(?m)^const\s+char\*\s+ver\s*=\s*"\d{2}_\d{2}_\d{4}";.*$'
$replacement = "const char* ver = `"$today`";  // MM_DD_YYYY"

if (-not [regex]::IsMatch($content, $pattern)) {
    Write-Error "Could not find expected ver assignment pattern in $TargetFile"
    exit 1
}

$updated = [regex]::Replace(
    $content,
    $pattern,
    $replacement,
    1
)

[System.IO.File]::WriteAllText($filePath, $updated, [System.Text.UTF8Encoding]::new($false))

# Ensure the updated file is part of the commit.
git add -- "$TargetFile" | Out-Null
Write-Host "Updated ver to $today in $TargetFile"
