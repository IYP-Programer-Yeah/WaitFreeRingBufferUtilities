project(Benchmark CXX)

file(GLOB_RECURSE CPP_FILES ${REPO_ROOT}/Example/${PROJECT_NAME}/Source/*.cpp)
add_executable(${PROJECT_NAME} ${CPP_FILES})
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} WaitFreeRingBufferUtilities Threads::Threads)

if(WIN32)
    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION Lib
            RUNTIME DESTINATION Bin
            ARCHIVE DESTINATION Lib)
else()
    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib)
endif()
