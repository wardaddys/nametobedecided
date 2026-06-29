$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.6.3\mingw_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;" + $env:PATH
cmake -B build_mingw -DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\mingw_64" -G Ninja
cmake --build build_mingw --config Release
if ($LASTEXITCODE -eq 0) {
    echo "Running OS Tuner..."
    .\build_mingw\OSTuner.exe
}
