param(
    [string]$Version = ""
)

$ErrorActionPreference = "Stop"
$repo = "abhraneeldhar7/xommit"
$headers = @{ "User-Agent" = "xommit-installer" }

if (-not $Version) {
    $release = Invoke-RestMethod -Headers $headers -Uri "https://api.github.com/repos/$repo/releases/latest"
    $Version = $release.tag_name.TrimStart("v")
}

$dir = Join-Path $env:LOCALAPPDATA "xommit\bin"
New-Item -ItemType Directory -Force -Path $dir | Out-Null

$exe = Join-Path $dir "xommit.exe"
$temporary = "$exe.download"

Write-Host "downloading xommit $Version"
Invoke-WebRequest -Headers $headers -Uri "https://github.com/$repo/releases/download/v$Version/xommit.exe" -OutFile $temporary

if ((Get-Item $temporary).Length -lt 100000) {
    Remove-Item $temporary -Force
    throw "download looks invalid"
}

Move-Item -Force $temporary $exe
Set-Content -Path (Join-Path $dir "version.txt") -Value $Version

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($userPath -notlike "*$dir*") {
    [Environment]::SetEnvironmentVariable("Path", "$userPath;$dir", "User")
    Write-Host "added $dir to your user PATH, open a new terminal"
}

Write-Host "installed xommit $Version to $exe"
