###################
# CommitManager client library
###################
set(CLIENT_SRCS
    ClientSocket.cpp
)

set(CLIENT_PUBLIC_HDR
    ClientSocket.hpp
    ErrorCode.hpp
)

# Transform public header list to use absolute paths
foreach(hdr ${CLIENT_PUBLIC_HDR})
    list(APPEND CLIENT_PUBLIC_HDR_ABSOLUTE ${PROJECT_SOURCE_DIR}/commitmanager/${hdr})
endforeach()

# Retrieve the public header list for the common library (client has to install those headers)
get_target_property(COMMON_PUBLIC_HDR_ABSOLUTE commitmanager-common PUBLIC_HEADER)

# Add CommitManager client library
add_library(commitmanager-client ${CLIENT_SRCS} ${CLIENT_PUBLIC_HDR_ABSOLUTE})
set_target_properties(commitmanager-client PROPERTIES PUBLIC_HEADER
        "${COMMON_PUBLIC_HDR_ABSOLUTE};${CLIENT_PUBLIC_HDR_ABSOLUTE}")
target_include_directories(commitmanager-client PUBLIC $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${INCLUDE_INSTALL_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

# Link against CommitManager common
target_link_libraries(commitmanager-client PRIVATE commitmanager-common)

# Link against Crossbow
target_include_directories(commitmanager-client PRIVATE ${Crossbow_INCLUDE_DIRS})
target_link_libraries(commitmanager-client PUBLIC crossbow_infinio crossbow_logger)

# Install the library
install(TARGETS commitmanager-client EXPORT CommitManagerClientTargets
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        PUBLIC_HEADER DESTINATION ${INCLUDE_INSTALL_DIR}/commitmanager)

# Install the cmake target
install(EXPORT CommitManagerClientTargets DESTINATION ${CMAKE_INSTALL_DIR})
