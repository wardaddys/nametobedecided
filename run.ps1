$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.9.1\mingw_64\bin;C:\Qt\Tools\CMake_64\bin;" + $env:PATH
cmake --build build_mingw --config Release
if ($LASTEXITCODE -eq 0) {
    echo "Running TunerPro..."
    .\build_mingw\Release\TunerPro.exe
}
