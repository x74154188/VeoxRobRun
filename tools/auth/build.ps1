param(
    [Parameter(Mandatory=$true)][string]$CoreSource,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release'
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$core = (Resolve-Path -LiteralPath $CoreSource).Path
if (-not (Test-Path -LiteralPath (Join-Path $core 'LicenseClient.cs'))) { throw 'BaoBao.Auth.Core source not found.' }
$output = [IO.Path]::GetFullPath($OutputDirectory)
$dotnet = (Get-Command dotnet -ErrorAction Stop).Source
$dotnetRoot = Split-Path $dotnet -Parent
$pack = Get-ChildItem -LiteralPath (Join-Path $dotnetRoot 'packs\Microsoft.NETCore.App.Host.win-x64') -Directory |
    Where-Object { $_.Name -match '^9\.0\.\d+$' } | Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
if (-not $pack) { throw 'Install the x64 .NET 9 SDK (native hosting pack required).' }
New-Item -ItemType Directory -Path $output -Force | Out-Null
# C# has no native linker inputs. Filter inherited VS paths only for this call.
$savedLib = $env:LIB
try {
    $env:LIB = ''
    & $dotnet publish (Join-Path $root 'auth\Veox.AuthBridge\Veox.AuthBridge.csproj') -c $Configuration -o $output `
        "-p:BaoBaoAuthCoreDir=$core" --nologo
    $publishExit = $LASTEXITCODE
} finally { $env:LIB = $savedLib }
if ($publishExit -ne 0) { throw "Auth bridge build failed: $publishExit" }
Copy-Item -LiteralPath (Join-Path $pack.FullName 'runtimes\win-x64\native\nethost.dll') -Destination $output -Force
foreach ($name in @('Veox.AuthBridge.dll','Veox.AuthBridge.runtimeconfig.json','Veox.AuthBridge.deps.json',
    'BaoBao.Auth.Core.dll','System.Management.dll','nethost.dll')) {
    if (-not (Test-Path -LiteralPath (Join-Path $output $name))) { throw "Missing auth runtime dependency: $name" }
}
Write-Output 'PASS: BaoBao.Auth.Core bridge and x64 hosting dependencies staged'
