cmake_minimum_required(VERSION 2.8.12)
project(CommitManager)

# For vim autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Declare build options
option(USE_ASAN "Use the address sanitizer" OFF)

# Set default install paths
set(BIN_INSTALL_DIR bin CACHE PATH "Installation directory for binaries")
set(CMAKE_INSTALL_DIR cmake CACHE PATH "Installation directory for CMake files")
set(INCLUDE_INSTALL_DIR include CACHE PATH "Installation directory for header files")
set(LIB_INSTALL_DIR lib CACHE PATH "Installation directory for libraries")

# Set the CommitManager directory
set(CommitManager_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Path to the CommitManager binaries and configuration")

# Set CMake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Build with PIC by default
if(NOT CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Set compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Enable ASAN
if(USE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
endif()

# Find dependencies
find_package(Crossbow COMPONENTS InfinIO Logger REQUIRED)
find_package(Jemalloc REQUIRED)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(test)

# Create cmake config file
configure_file(CommitManagerConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/CommitManagerConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CommitManagerConfig.cmake DESTINATION ${CMAKE_INSTALL_DIR})
