#!/bin/bash
# Build 02_hide for the browser (WebAssembly) and publish into the Pages site.
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

# Publish into the GitHub Pages site (docs/games/02_hide)
SITE="$PROJECT_DIR/../../docs"
mkdir -p "$SITE/games/02_hide"
cp build-web/bin/Release/Game.html "$SITE/games/02_hide/index.html"
for f in Game.js Game.wasm Game.data; do
  [ -f "build-web/bin/Release/$f" ] && cp "build-web/bin/Release/$f" "$SITE/games/02_hide/"
done
# Cache-bust: stamp the shell + asset URLs so browsers never serve a stale build
STAMP=$(date +%Y%m%d%H%M%S)
sed -i '' "s/STAMP/$STAMP/; s|src=\"Game.js\"|src=\"Game.js?v=$STAMP\"|; s|src=Game\.js|src=\"Game.js?v=$STAMP\"|" "$SITE/games/02_hide/index.html" 2>/dev/null || \
  sed -i "s/STAMP/$STAMP/; s|src=\"Game.js\"|src=\"Game.js?v=$STAMP\"|; s|src=Game\.js|src=\"Game.js?v=$STAMP\"|" "$SITE/games/02_hide/index.html"
echo "Published web build -> docs/games/02_hide/ (v=$STAMP)"
