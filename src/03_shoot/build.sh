#!/bin/bash

PROJECT="03_shoot"
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
