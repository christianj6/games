#!/bin/bash

# # change to build different games
# PROJECT="04_teleport"
# set -e 
#
# # format
# cd src/${PROJECT}
# find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i
#
# # vcpkg+cmake+ninja
# mkdir -p build && cd build
# cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake
# ninja
#
# # run the game 
# ./bin/Debug/Game

# alternative command to hook into windows from wsl
cmd.exe /c "C:\Users\Christian\Desktop\repos\games\src\04_teleport\build.bat"
