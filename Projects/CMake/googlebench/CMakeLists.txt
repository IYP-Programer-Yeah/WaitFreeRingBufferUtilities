set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(GOOGLETEST_PATH ${REPO_ROOT}/Dependencies/googletest/)
add_subdirectory(${REPO_ROOT}/Dependencies/benchmark/ googlebench/)
