#!/bin/bash

PROJECT="02_hide"
set -e 

# autoformat
cd src/${PROJECT}/src 
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
./bin/Debug/${PROJECT}
