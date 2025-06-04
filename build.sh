#!/bin/bash

set -e 

# autoformat
cd src/01_coins/src 
clang-format -i -- **.cpp #**.h

# change the project dir to build different games
cd ../build
./premake5.osx gmake2

# generate compile_commands.json using bear
cd ..
# comment out once compile_commands has been generated
# bear -- make
make

# run the game 
./bin/Debug/01_coins
