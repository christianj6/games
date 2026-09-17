cmake_minimum_required(VERSION 3.15)
project(Game)

# Web-only build (the desktop flow uses Premake via build.sh and is untouched).
# NOTE: PLATFORM_WEB define + web compile flags MUST be set before every
# FetchContent target is created — mixed flag compilation breaks heap growth.
if(EMSCRIPTEN)
    add_definitions(-DPLATFORM_WEB)
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
endif()

# Force compile commands generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib configuration (pinned: web backend handles pointer lock/mouse deltas)
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6f5f7acd
)
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
    # WebGL2 (GLSL ES 3.00)
    set(GRAPHICS GRAPHICS_API_OPENGL_ES3 CACHE STRING "" FORCE)
else()
    set(GRAPHICS GRAPHICS_API_OPENGL_33 CACHE STRING "" FORCE)
endif()
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

if(APPLE)
    add_definitions(-D_GLFW_COCOA)
    add_definitions(-DPLATFORM_DESKTOP)
    add_compile_options(-Wno-deprecated-declarations)
elseif(WIN32)
    add_definitions(-D_GLFW_WIN32)
    add_definitions(-DPLATFORM_DESKTOP)
elseif(UNIX AND NOT EMSCRIPTEN)
    add_definitions(-D_GLFW_X11)
    add_definitions(-D_GNU_SOURCE)
    add_definitions(-DPLATFORM_DESKTOP)
endif()

if(NOT EMSCRIPTEN)
    add_definitions(-DGRAPHICS_API_OPENGL_33)
endif()

# Collect source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Create executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${raylib_SOURCE_DIR}/src
)

# Link raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Web (Emscripten) output: .html + .js + .wasm (no assets, so no Game.data).
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -sASYNCIFY
        -sFORCE_FILESYSTEM=1
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()
