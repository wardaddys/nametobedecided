param(
    [string]$BuildDir = "build_qt_manual",
    [string]$BuildType = "Debug",
    [switch]$Clean,
    [switch]$SmokeRun
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$cmakeExe = "C:/Qt/Tools/CMake_64/bin/cmake.exe"
$ctestExe = "C:/Qt/Tools/CMake_64/bin/ctest.exe"
$ninjaExe = "C:/Qt/Tools/Ninja/ninja.exe"
$mingwBin = "C:/Qt/Tools/mingw1310_64/bin"
$qtBin = "C:/Qt/6.10.2/mingw_64/bin"
$qtPrefix = "C:/Qt/6.10.2/mingw_64"

foreach ($tool in @($cmakeExe, $ctestExe, $ninjaExe)) {
    if (-not (Test-Path $tool)) {
        throw "Required tool not found: $tool"
    }
}

$env:PATH = "C:/Qt/Tools/CMake_64/bin;C:/Qt/Tools/Ninja;$mingwBin;$qtBin;" + $env:PATH

if (-not (Test-Path $BuildDir)) {
    Write-Host "Configuring $BuildDir..."
    & $cmakeExe -S . -B $BuildDir -G Ninja -DCMAKE_BUILD_TYPE=$BuildType -DCMAKE_PREFIX_PATH=$qtPrefix -DCMAKE_MAKE_PROGRAM=$ninjaExe -DBUILD_TESTING=ON
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }
}

if ($Clean) {
    Write-Host "Cleaning build directory..."
    & $ninjaExe -C $BuildDir -t clean
    if ($LASTEXITCODE -ne 0) { throw "Clean failed." }
}

Write-Host "Building..."
& $cmakeExe --build $BuildDir --config $BuildType
if ($LASTEXITCODE -ne 0) { throw "Build failed." }

Write-Host "Running tests..."
& $ctestExe --test-dir $BuildDir --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Tests failed." }

if ($SmokeRun) {
    $appExe = Join-Path $BuildDir "TunerPro.exe"
    if (Test-Path $appExe) {
        Write-Host "Running smoke launch for TunerPro.exe..."
        $proc = Start-Process -FilePath $appExe -PassThru
        Start-Sleep -Seconds 5
        if (-not $proc.HasExited) {
            Stop-Process -Id $proc.Id -Force
            Write-Host "Smoke run: launch OK (terminated after 5s)."
        } else {
            Write-Host "Smoke run: process exited with code $($proc.ExitCode)."
        }
    } else {
        throw "Smoke run requested but executable not found at $appExe"
    }
}

Write-Host "Build and test completed successfully."
