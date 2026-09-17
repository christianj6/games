#!/bin/bash
# Build 05_explore for the browser (WebAssembly) and publish into the Pages site.
# Requires: emscripten (brew install emscripten)
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
EM_BIN="$(readlink -f "$(which emcc)")"
EMSCRIPTEN_CMAKE="$(dirname "$(dirname "$EM_BIN")")/cmake/Modules/Platform/Emscripten.cmake"
[ -f "$EMSCRIPTEN_CMAKE" ] || EMSCRIPTEN_CMAKE="$(dirname "$(dirname "$EM_BIN")")/libexec/cmake/Modules/Platform/Emscripten.cmake"

cd "$PROJECT_DIR"
cmake -B build-web -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$EMSCRIPTEN_CMAKE" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build-web

# Publish into the GitHub Pages site (docs/games/05_explore)
SITE="$PROJECT_DIR/../../docs"
mkdir -p "$SITE/games/05_explore"
cp build-web/bin/Release/Game.html "$SITE/games/05_explore/index.html"
for f in Game.js Game.wasm Game.data; do
  [ -f "build-web/bin/Release/$f" ] && cp "build-web/bin/Release/$f" "$SITE/games/05_explore/"
done
echo "Published web build -> docs/games/05_explore/"
