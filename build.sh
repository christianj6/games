#!/bin/bash

# change to build different games
PROJECT="04_teleport"
set -e 

# format
cd src/${PROJECT}/src 
clang-format -i -- **.cpp **.h

# vcpkg+cmake+ninja
cd ..
mkdir -p build && cd build
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake
ninja

# run the game 
./bin/Debug/Game
