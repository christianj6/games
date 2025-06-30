@echo off
SET PROJECT=04_teleport

REM Format code
cd src\%PROJECT%
where clang-format >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /R src %%f in (*.cpp *.h *.hpp) do clang-format -i "%%f"
)

REM Setup VS environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Build with CMake + Ninja
if not exist build mkdir build
cd build
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake
ninja

REM Run the game
.\bin\Debug\Game.exe

REM go back to superior dir 
cd ..\..\..
