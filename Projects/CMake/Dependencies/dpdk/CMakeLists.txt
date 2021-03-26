project(dpdk CXX)

message(STATUS "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
include(ExternalProject)

set(SOURCE_DIRECTORY ${REPO_ROOT}/Dependencies/dpdk)
set(BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ByProducts/build)
set(INSTALL_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ByProducts/install)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIRECTORY}/include/)

ExternalProject_Add(${PROJECT_NAME}_BUILD
    SOURCE_DIR "${SOURCE_DIRECTORY}"
    BINARY_DIR "${BUILD_DIRECTORY}"
    INSTALL_DIR "${INSTALL_DIRECTORY}"
    CONFIGURE_COMMAND cd ${SOURCE_DIRECTORY} && meson ${BUILD_DIRECTORY} -Dprefix=${INSTALL_DIRECTORY}
    BUILD_COMMAND ninja
    INSTALL_COMMAND ninja install)

function(_create_dpdk_targe_library library_name)
    add_library(${library_name} INTERFACE IMPORTED GLOBAL)

    set_target_properties(${library_name} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIRECTORY}/include/)

    message("${INSTALL_DIRECTORY}/bin/lib${library_name}-21${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_dependencies(${library_name} ${PROJECT_NAME}_BUILD)

    #set_target_properties(${library_name} PROPERTIES
    #    IMPORTED_LOCATION "${INSTALL_DIRECTORY}/bin/lib${library_name}-21${CMAKE_SHARED_LIBRARY_SUFFIX}")
endfunction()

_create_dpdk_targe_library(rte_ring)
message("RTE RING  ${INSTALL_DIRECTORY}/bin/lib${library_name}-21${CMAKE_SHARED_LIBRARY_SUFFIX}")

#_create_dpdk_targe_library(rte_eal)
