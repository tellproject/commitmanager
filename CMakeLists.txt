cmake_minimum_required(VERSION 2.8.12)
project(CommitManager)

# For vim autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Declare build options
option(USE_ASAN "Use the address sanitizer" OFF)

# Set CMake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Enabel ASAN
if(USE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
endif()

# Find dependencies
find_package(Crossbow COMPONENTS Logger REQUIRED)
find_package(Jemalloc REQUIRED)

add_subdirectory(server)
