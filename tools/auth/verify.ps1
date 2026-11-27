param(
    [Parameter(Mandatory=$true)][string]$CoreSource,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [string]$SourceRoot = ''
)
$ErrorActionPreference = 'Stop'
if (-not $SourceRoot) { $SourceRoot = Join-Path $PSScriptRoot '..\..' }
$root = (Resolve-Path -LiteralPath $SourceRoot).Path
$output = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $output -Force | Out-Null
$main = Get-Content -LiteralPath (Join-Path $root 'VeoxDMA\Main.cpp') -Raw
$hostSource = Join-Path $root 'VeoxDMA\Security\LicenseAuth.cpp'
if (-not (Test-Path -LiteralPath $hostSource)) {
    'FAIL: BaoBao network authorization integration missing'
    exit 1
}
$loginAt = $main.IndexOf('if (!LicenseAuth::Login(')
$startAt = $main.IndexOf('StartAttachLoop();', $main.IndexOf('int WINAPI WinMain'))
if ($loginAt -lt 0 -or $loginAt -gt $startAt -or -not $main.Contains('if (!LicenseAuth::IsAuthorized())')) {
    throw 'Native authentication gating missing or out of order'
}
& (Join-Path $root 'tools\auth\build.ps1') -CoreSource $CoreSource -OutputDirectory (Join-Path $output 'auth')
$nativeExit = $LASTEXITCODE
if ($nativeExit -ne 0) { exit $nativeExit }
$managed = Join-Path $output 'managed'
dotnet build (Join-Path $root 'tools\auth\tests\Veox.AuthBridge.Tests.csproj') -c Release -o $managed "-p:BaoBaoAuthCoreDir=$CoreSource" --nologo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
dotnet (Join-Path $managed 'Veox.AuthBridge.Tests.dll')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
if (-not (Get-Command cl -ErrorAction SilentlyContinue)) { throw 'Run from an x64 VS Developer shell.' }
@'
#pragma once
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <iterator>
#include <string>
'@ | Set-Content -LiteralPath (Join-Path $output 'pch.h') -Encoding utf8
Push-Location $output
try {
    cl /nologo /EHsc /std:c++20 /utf-8 /W3 /I. "/I$root\VeoxDMA" /Fe:native_host.exe `
        (Join-Path $root 'tools\auth\tests\native_host.cpp') $hostSource /link user32.lib
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & (Join-Path $output 'native_host.exe') $output
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} finally { Pop-Location }
'PASS: auth integration; native gating, controller and real SDK dependency tests'
exit 0
