#!/bin/bash

# change to build different games
PROJECT="04_teleport"
set -e 

# format
cd src/${PROJECT}
find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

# vcpkg+cmake+ninja
mkdir -p build_wsl && cd build_wsl
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake
ninja

# copy the compile commands for configuring lsp
cp src/$PROJECT/build_wsl/compile_commands.json src/$PROJECT
