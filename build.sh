#!/bin/bash

set -e 

# change the project dir to build different games
cd src/?/build
./premake5.osx gmake2

# generate compile_commands.json using bear
cd ..
# comment out once compile_commands has been generated
# bear -- make
make

# run the game 
./bin/Debug/?
