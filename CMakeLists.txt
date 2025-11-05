# Set the minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Define the project
project(bolt-pm LANGUAGES CXX)

# --- C++ Standard ---
# Set to C++17, which is required for <filesystem>
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find Source Files ---
# Define our executable from the source file in the 'src' directory
add_executable(bolt-pm src/main.cpp)

# --- Find Dependencies (Locally) ---
# Add the 'src' directory as an include path.
# This allows the compiler to find "#include "toml.hpp"
target_include_directories(bolt-pm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Linking ---
# <filesystem> support is handled natively by modern compilers (like GNU 12.2.0).
# The conditional linking for old compilers has been removed to fix the CMake error.