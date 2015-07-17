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


###################
# CommitManager common
###################
set(COMMON_SRCS
    commitmanager/CommitManager.hpp
    commitmanager/Descriptor.hpp
    commitmanager/SnapshotDescriptor.hpp
    common/CommitManager.cpp
    common/Descriptor.cpp
    common/SnapshotDescriptor.cpp
)

# Add CommitManager common library
add_library(commitmanager-common STATIC ${COMMON_SRCS})
target_include_directories(commitmanager-common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link against Crossbow
target_include_directories(commitmanager-common PRIVATE ${Crossbow_INCLUDE_DIRS})
target_link_libraries(commitmanager-common PRIVATE crossbow_infinio crossbow_logger)


###################
# CommitManager client
###################
set(CLIENT_SRCS
    commitmanager/ClientSocket.hpp
    commitmanager/ErrorCode.hpp
    commitmanager/MessageTypes.hpp
    client/ClientSocket.cpp
)

# Add CommitManager client library
add_library(commitmanager-client STATIC ${CLIENT_SRCS})
target_include_directories(commitmanager-client PUBLIC
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${INCLUDE_INSTALL_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# Link against CommitManager common
target_link_libraries(commitmanager-client PRIVATE commitmanager-common)

# Link against Crossbow
target_include_directories(commitmanager-client PRIVATE ${Crossbow_INCLUDE_DIRS})
target_link_libraries(commitmanager-client PRIVATE crossbow_logger)
target_link_libraries(commitmanager-client PUBLIC crossbow_infinio)

# Install the library
install(TARGETS commitmanager-client
        EXPORT CommitManagerClientTargets
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

# Install CommitManager client headers
install(DIRECTORY commitmanager DESTINATION ${INCLUDE_INSTALL_DIR} FILES_MATCHING PATTERN "*.hpp")

# Install the cmake target
install(EXPORT CommitManagerClientTargets DESTINATION ${CMAKE_INSTALL_DIR})


###################
# CommitManager server
###################
set(SERVER_SRCS
    server/ServerConfig.hpp
    server/ServerSocket.cpp
    server/ServerSocket.hpp
)

# Add CommitManager server executable
add_executable(commitmanagerd server/main.cpp ${SERVER_SRCS})
target_include_directories(commitmanagerd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link against CommitManager common
target_link_libraries(commitmanagerd PRIVATE commitmanager-common)

# Link against Crossbow
target_include_directories(commitmanagerd PRIVATE ${Crossbow_INCLUDE_DIRS})
target_link_libraries(commitmanagerd PRIVATE crossbow_infinio crossbow_logger)

# Link against Jemalloc
target_include_directories(commitmanagerd PRIVATE ${Jemalloc_INCLUDE_DIRS})
target_link_libraries(commitmanagerd PRIVATE ${Jemalloc_LIBRARIES})

# Install the binary
install(TARGETS commitmanagerd
        RUNTIME DESTINATION ${BIN_INSTALL_DIR})
