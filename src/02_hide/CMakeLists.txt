cmake_minimum_required(VERSION 3.15)

# Web-only build (Emscripten). The desktop flow for this game is
# Premake/Make via build.sh — this file is never configured for desktop.
project(Game)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "02_hide: use build.sh (Premake/Make) for desktop; this CMakeLists.txt is web-only (configure with the Emscripten toolchain via build-web.sh).")
endif()

# Web compile flags MUST be set before any FetchContent target is created.
add_definitions(-DPLATFORM_WEB)
add_compile_options(-fexceptions)
add_link_options(-fexceptions)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Raylib via FetchContent, pinned to the exact commit used by 05_explore's
# web build (its web backend handles the canvas/keyboard input path well).
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6f5f7acd
)
set(PLATFORM Web CACHE STRING "" FORCE)
# WebGL2 (GLES 3.0) backend — no custom shaders in this game, but ES3 is the
# correct target for the browser.
set(GRAPHICS GRAPHICS_API_OPENGL_ES3 CACHE STRING "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Output to bin/<config> so build-web.sh mirrors the other games' layout.
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Web (Emscripten) output: .html + .js + .wasm. No assets exist in this game,
# so there is no --preload-file and no Game.data.
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -sASYNCIFY
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()
