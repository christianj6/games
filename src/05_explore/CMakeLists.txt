cmake_minimum_required(VERSION 3.15)
if (WIN32)
    set(CMAKE_C_COMPILER "cl")
    set(CMAKE_CXX_COMPILER "cl")
endif()

project(Game)

if(EMSCRIPTEN)
    # Web build: fmt + behaviortree.cpp fetched directly (vcpkg's BT pulls
    # ZeroMQ, which cannot run in a browser). Eigen is unused by game code.
    # NOTE: web compile flags MUST be set before every FetchContent target
    # is created — mixed flag compilation breaks heap growth (OOM).
    add_definitions(-DPLATFORM_WEB)
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
    include(FetchContent)
    FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 11.2.0)
    FetchContent_Declare(behaviortree_cpp
        GIT_REPOSITORY https://github.com/BehaviorTree/BehaviorTree.CPP
        GIT_TAG 4.7.3
        # The vendored minicoro picks its emscripten fiber backend even in
        # C++ TUs, where em_asm.h (C++-only) breaks inside extern "C".
        # Gate it off so minicoro falls through to the ASYNCIFY backend.
        PATCH_COMMAND bash
            ${CMAKE_CURRENT_SOURCE_DIR}/web/patch_minicoro.sh
            3rdparty/minicoro/minicoro.h
        )
    set(BTCPP_GROOT_INTERFACE OFF CACHE BOOL "" FORCE)
    set(BTCPP_SQLITE_LOGGING OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BTCPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(BTCPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BTCPP_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BTCPP_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BTCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(FMT_OS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(fmt behaviortree_cpp)
else()
    find_package(fmt REQUIRED)
    find_package(Eigen3 REQUIRED)
    find_package(behaviortree_cpp REQUIRED)
endif()

# Force compile commands generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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
    GIT_TAG master
)
if(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "" FORCE)
    # WebGL2 (GLSL ES 3.00) — matches resources/shaders/glsl300 variants.
    set(GRAPHICS GRAPHICS_API_OPENGL_ES3 CACHE STRING "" FORCE)
endif()
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

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

# NOTE: PLATFORM_WEB define + web compile flags are hoisted above (they must
# precede every FetchContent target).

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

# Link other packages
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
if(NOT EMSCRIPTEN)
    target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE BT::behaviortree_cpp)
# Web (Emscripten) output: .html + .js + .wasm with resources preloaded.
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PRIVATE
        -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 -sASYNCIFY
        -sFORCE_FILESYSTEM=1
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources@resources
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()

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
