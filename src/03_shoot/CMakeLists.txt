cmake_minimum_required(VERSION 3.15)
if (WIN32)
    set(CMAKE_C_COMPILER "cl")
    set(CMAKE_CXX_COMPILER "cl")
endif()

project(Game)
if(EMSCRIPTEN)
    # Web build: web compile flags MUST be set before any FetchContent target
    # is created — mixed flag compilation breaks heap growth (OOM).
    add_definitions(-DPLATFORM_WEB)
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
endif()

# Force compile commands generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()

# Debug specific settings
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

# Release specific settings
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

# Raylib configuration
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6f5f7acd
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
    # WebGL2 (GLSL ES 3.00) — raylib's ES3/WebGL2 backend path.
    set(GRAPHICS GRAPHICS_API_OPENGL_ES3 CACHE STRING "" FORCE)
endif()

# Platform specific settings
if(APPLE)
    add_definitions(-D_GLFW_COCOA)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    add_definitions(-DPLATFORM_DESKTOP)
    # Disable deprecation warnings on macOS
    add_compile_options(-Wno-deprecated-declarations)
elseif(WIN32)
    add_definitions(-D_GLFW_WIN32)
    add_definitions(-DPLATFORM_DESKTOP)
    # Prevent Windows.h from including too much
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    # Disable CRT secure warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT EMSCRIPTEN)
    add_definitions(-D_GLFW_X11)
    add_definitions(-D_GNU_SOURCE)
    add_definitions(-DPLATFORM_DESKTOP)
endif()

# OpenGL settings: desktop uses GL 3.3; web uses raylib's ES3/WebGL2 path.
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

# Set output directories to match premake
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -sASYNCIFY
        -sFORCE_FILESYSTEM=1
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${raylib_SOURCE_DIR}/src
    ${raylib_SOURCE_DIR}/src/external
    ${raylib_SOURCE_DIR}/src/external/glfw/include
    ${raylib_SOURCE_DIR}/examples/shaders
)

# Link raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Platform specific linking
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework CoreVideo"
        "-framework AudioToolbox"
    )
elseif(UNIX AND NOT EMSCRIPTEN)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pthread
        m
        dl
        rt
        X11
    )
endif()
