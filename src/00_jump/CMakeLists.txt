cmake_minimum_required(VERSION 3.15)

project(Game)

# Web-only build (desktop stays on the Premake/Make flow).
# See 05_explore/CMakeLists.txt for the reference pipeline.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Web compile flags MUST be set before any FetchContent target is created —
# mixed flag compilation breaks heap growth (OOM).
if(EMSCRIPTEN)
    add_definitions(-DPLATFORM_WEB)
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
endif()

# Raylib configuration
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    # Pinned to the exact commit used by 05_explore's proven web build —
    # its web backend handles pointer lock + mouse deltas correctly.
    GIT_TAG 6f5f7acd
)
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
    # WebGL2 (GLSL ES 3.00)
    set(GRAPHICS GRAPHICS_API_OPENGL_ES3 CACHE STRING "" FORCE)
endif()
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# Collect source files
file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Set output directories to match the premake layout
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Web (Emscripten) output: .html + .js + .wasm with resources preloaded.
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -sASYNCIFY
        -sFORCE_FILESYSTEM=1
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources@resources
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()
