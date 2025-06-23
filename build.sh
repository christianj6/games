#!/bin/bash

# change to build different games
PROJECT="04_teleport"
set -e 

# format
cd src/${PROJECT}/src 
clang-format -i -- **.cpp **.h

# cmake+ninja
cd ..
mkdir -p build
cd build
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja

# run the game 
./bin/Debug/Game
