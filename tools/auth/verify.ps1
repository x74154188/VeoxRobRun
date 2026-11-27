param(
    [Parameter(Mandatory=$true)][string]$SdkSource,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [string]$SourceRoot = ''
)
$ErrorActionPreference = 'Stop'
if (-not $SourceRoot) { $SourceRoot = Join-Path $PSScriptRoot '..\..' }
$root = (Resolve-Path -LiteralPath $SourceRoot).Path
$sdk = (Resolve-Path -LiteralPath $SdkSource).Path
$output = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $output -Force | Out-Null
$native = Join-Path $root 'VeoxDMA\Security\AuthController.cpp'
if (-not (Test-Path -LiteralPath (Join-Path $root 'VeoxDMA\Security\ConsoleLogin.cpp'))) {
    'FAIL: console login missing (dialog baseline)'
    exit 1
}
if (-not (Test-Path -LiteralPath (Join-Path $root 'VeoxDMA\Security\ConsoleInput.cpp'))) {
    'FAIL: native console key/paste reader missing (hidden-echo stdin baseline)'
    exit 1
}
$project = Get-Content -LiteralPath (Join-Path $root 'VeoxDMA\VeoxDMA.vcxproj') -Raw
$main = Get-Content -LiteralPath (Join-Path $root 'VeoxDMA\Main.cpp') -Raw
$console = Get-Content -LiteralPath (Join-Path $root 'VeoxDMA\Security\ConsoleLogin.cpp') -Raw
$hostSource = Get-Content -LiteralPath (Join-Path $root 'VeoxDMA\Security\LicenseAuth.cpp') -Raw
if ($project.Contains('Security\LoginDialog') -or $console.Contains('DialogBox') `
    -or $hostSource.Contains('MessageBox') -or -not $hostSource.Contains('ShowConsoleLogin(')) {
    throw 'Authorization still uses a dialog instead of the console'
}
$at = $main.IndexOf('if (!LicenseAuth::Login(')
if ($project.Contains('StageBaoBaoAuth') -or $project.Contains('dotnet') -or -not $project.Contains('BaoBaoAuthCppDir') `
    -or $at -lt 0 -or $at -gt $main.IndexOf('StartAttachLoop();', $main.IndexOf('int WINAPI WinMain'))) {
    throw 'Native SDK project binding or startup gate missing'
}
if (-not (Get-Command cl -ErrorAction SilentlyContinue)) { throw 'Run from an x64 VS Developer shell.' }
Push-Location $output
try {
    $libs = @('user32.lib','shell32.lib','winhttp.lib','bcrypt.lib','crypt32.lib','wbemuuid.lib',
        'ole32.lib','oleaut32.lib','advapi32.lib','version.lib')
    cl /nologo /EHsc /std:c++17 /utf-8 /W3 /MD /DNDEBUG /DNOMINMAX /DWIN32_LEAN_AND_MEAN `
        "/I$sdk\include" "/I$sdk\third_party" /c "/Fo:sdk.obj" (Join-Path $sdk 'src\BaoBaoAuth.cpp')
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cl /nologo /EHsc /std:c++20 /utf-8 /W3 /MD /DNDEBUG "/I$sdk\include" "/I$root\VeoxDMA" /Fe:native_tests.exe `
        (Join-Path $root 'tools\auth\native_tests.cpp') $native `
        (Join-Path $root 'VeoxDMA\Security\LicenseAuth.cpp') (Join-Path $root 'VeoxDMA\Security\ConsoleLogin.cpp') `
        (Join-Path $root 'VeoxDMA\Security\ConsoleInput.cpp') `
        sdk.obj /link $libs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & (Join-Path $output 'native_tests.exe')
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cl /nologo /EHsc /std:c++20 /utf-8 /W3 /MD /DNDEBUG "/I$sdk\include" "/I$root\VeoxDMA" /Fe:console_tests.exe `
        (Join-Path $root 'tools\auth\console_tests.cpp') $native `
        (Join-Path $root 'VeoxDMA\Security\ConsoleLogin.cpp') (Join-Path $root 'VeoxDMA\Security\ConsoleInput.cpp') sdk.obj /link $libs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & (Join-Path $output 'console_tests.exe')
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cl /nologo /EHsc /std:c++20 /utf-8 /W3 /MD /DNDEBUG "/I$root\VeoxDMA" /Fe:console_input_tests.exe `
        (Join-Path $root 'tools\auth\console_input_tests.cpp') (Join-Path $root 'VeoxDMA\Security\ConsoleInput.cpp') /link user32.lib
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    python (Join-Path $root 'tools\auth\run_console_input_tests.py') (Join-Path $output 'console_input_tests.exe')
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    cl /nologo /EHsc /std:c++17 /utf-8 /W3 /MD /DNDEBUG "/I$sdk\include" "/I$sdk\third_party" `
        /Fe:sdk_tests.exe (Join-Path $sdk 'tests\tests.cpp') sdk.obj /link $libs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & (Join-Path $output 'sdk_tests.exe')
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} finally { Pop-Location }
'PASS: console auth integration; no dialog, managed runtime or production HTTP used'
exit 0
