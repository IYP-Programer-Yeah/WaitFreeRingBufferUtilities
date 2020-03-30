cmake_minimum_required (VERSION 3.5)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(CMAKE_PROJECT_ROOT ${REPO_ROOT}/Projects/CMake)

if(WIN32)
	set(LOCAL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/Include)
else()
	set(LOCAL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED, ON)

add_subdirectory(googletest)
add_subdirectory(boost)
add_subdirectory(WaitFreeRingBufferUtilities)
add_subdirectory(Test)
add_subdirectory(Example)

