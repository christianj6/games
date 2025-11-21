#!/bin/bash

# alternative command to hook into windows from wsl
# cmd.exe /c "C:\Users\Christian\Desktop\repos\games\src\05_explore\build.bat"

# change to build different games
PROJECT="05_explore"
set -e 

# format
cd src/${PROJECT}
# find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

# vcpkg+cmake+ninja
mkdir -p build
cmake -S . -B build -G Ninja \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE=../../../../../tallence/repos/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build build
cd build

# copy the compile commands for configuring lsp
# cp compile_commands.json ../

# run game
cd ..
./build/bin/Debug/Game

