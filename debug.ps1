$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.6.3\mingw_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;" + $env:PATH
C:\Qt\Tools\mingw1310_64\bin\gdb.exe -batch -ex "run" -ex "bt" -ex "quit" .\build_mingw\OSTuner.exe
