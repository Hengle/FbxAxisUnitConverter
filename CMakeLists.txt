cmake_minimum_required(VERSION 3.20)
project(FbxAxisUnitConverter LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC-specific flags
if(MSVC)
    add_compile_options(/utf-8 /openmp)
endif()

# Custom Find module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# FBX SDK
find_package(FBXSDK REQUIRED)

# Main target
add_subdirectory(FbxAxisUnitConverter)

# Google Test (submodule)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/googletest)

# Tests
enable_testing()
add_subdirectory(FbxAxisUnitConverterTests)
