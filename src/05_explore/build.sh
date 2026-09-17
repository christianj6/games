#!/bin/bash

# Windows/WSL alternative — hook into Windows from WSL:
# cmd.exe /c "C:\Users\Christian\Desktop\repos\games\src\05_explore\build.bat"

# change to build different games
PROJECT="05_explore"
set -e

cd src/${PROJECT}
# format
find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

# vcpkg lives in a sibling checkout; the path differs per machine:
#   macOS: ~/Desktop/tallence/repos/vcpkg
#   WSL:   repos/vcpkg  (../../../../vcpkg from src/05_explore)
if [[ "$(uname)" == "Darwin" ]]; then
  TOOLCHAIN="$HOME/Desktop/tallence/repos/vcpkg/scripts/buildsystems/vcpkg.cmake"
  BUILD_DIR=build
else
  TOOLCHAIN=../../../../vcpkg/scripts/buildsystems/vcpkg.cmake
  BUILD_DIR=build_wsl
fi

# vcpkg+cmake+ninja
mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"
cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
ninja

# copy the compile commands for configuring lsp
cp compile_commands.json ../

# run game
./bin/Debug/Game
