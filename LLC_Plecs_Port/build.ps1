param([ValidateSet('Debug','Release')][string]$Configuration='Debug')
$ErrorActionPreference='Stop'
$portRoot=$PSScriptRoot
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsPath=& $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (!$vsPath) { throw 'Visual Studio MSBuild installation not found' }
$builder=Join-Path $vsPath 'MSBuild/Current/Bin/MSBuild.exe'
& $builder (Join-Path $portRoot 'LLC_Plecs_Port.sln') /m /nologo /verbosity:minimal "/p:Configuration=$Configuration" /p:Platform=x64
if ($LASTEXITCODE -ne 0) { throw 'LLC build failed' }
foreach($test in @('test_timer','test_gate','test_core','test_slow')) {
    & (Join-Path $portRoot "bin/x64/$test.exe")
    if ($LASTEXITCODE -ne 0) { throw "$test failed" }
}
