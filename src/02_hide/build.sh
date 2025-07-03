#!/bin/bash

PROJECT="02_hide"
set -e 

# autoformat
cd src/${PROJECT}/src 
clang-format -i -- **.cpp **.h

cd ../build
./premake5 gmake2

# generate compile_commands.json using bear
cd ..
make

# run the game 
./bin/Debug/${PROJECT}
