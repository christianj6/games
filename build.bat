@echo off
setlocal enabledelayedexpansion
REM Enable error checking
set "ERRORLEVEL="
set "exitcode=0"
SET PROJECT=04_teleport

REM Format code
cd src\%PROJECT% || exit /b 1
where clang-format >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /R src %%f in (*.cpp *.h *.hpp) do clang-format -i "%%f"
)

REM Setup VS environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" || exit /b 1

REM Build with CMake + Ninja
if not exist build mkdir build || exit /b 1
cd build || exit /b 1
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake || exit /b 1
ninja || exit /b 1

REM Run the game
.\bin\Debug\Game.exe || exit /b 1

REM go back to superior dir 
cd ..\..\..\.. || exit /b 1
