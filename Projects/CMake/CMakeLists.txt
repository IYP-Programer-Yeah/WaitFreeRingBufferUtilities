cmake_minimum_required (VERSION 3.5)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(CMAKE_PROJECT_ROOT ${REPO_ROOT}/Projects/CMake)

if(WIN32)
    set(INCLUDE_FOLDER_NAME Include)
else()
    set(INCLUDE_FOLDER_NAME include)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED, ON)

add_subdirectory(googletest)
add_subdirectory(boost)
add_subdirectory(WaitFreeRingBufferUtilities)
add_subdirectory(Test)
add_subdirectory(Example)

