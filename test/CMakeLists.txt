###################
# CommitManager test
###################
set(TEST_SRCS
    TestClient.cpp
)

# Add CommitManager server executable
add_executable(commitmanager-test ${TEST_SRCS})
target_include_directories(commitmanager-test PRIVATE ${PROJECT_SOURCE_DIR})

# Link against Commit Manager client
target_link_libraries(commitmanager-test PRIVATE commitmanager-client)

# Link against Crossbow
target_include_directories(commitmanager-test PRIVATE ${Crossbow_INCLUDE_DIRS})
target_link_libraries(commitmanager-test PRIVATE crossbow_infinio crossbow_logger)

# Link against Jemalloc
target_include_directories(commitmanager-test PRIVATE ${Jemalloc_INCLUDE_DIRS})
target_link_libraries(commitmanager-test PRIVATE ${Jemalloc_LIBRARIES})
