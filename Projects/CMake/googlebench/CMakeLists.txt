set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Google Benchmark tests." FORCE)
add_subdirectory(${REPO_ROOT}/Dependencies/benchmark/ googlebench/)
