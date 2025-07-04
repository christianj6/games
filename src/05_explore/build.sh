#!/bin/bash

# change to build different games
PROJECT="05_explore"
set -e 

# format
cd src/${PROJECT}
find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

# vcpkg+cmake+ninja
mkdir -p build_wsl && cd build_wsl
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake
ninja

# copy the compile commands for configuring lsp
cp compile_commands.json ../

# run game
./bin/Debug/Game
