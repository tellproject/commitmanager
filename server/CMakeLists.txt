###################
# CommitManager server
###################
set(SERVER_SRCS
)

# Add CommitManager server executable
add_executable(commitmanagerd main.cpp ${SERVER_SRCS})

# Link against Crossbow
target_include_directories(commitmanagerd PRIVATE ${Crossbow_INCLUDE_DIRS})
target_link_libraries(commitmanagerd PRIVATE crossbow_logger)

# Link against Jemalloc
target_include_directories(commitmanagerd PRIVATE ${Jemalloc_INCLUDE_DIRS})
target_link_libraries(commitmanagerd PRIVATE ${Jemalloc_LIBRARIES})
