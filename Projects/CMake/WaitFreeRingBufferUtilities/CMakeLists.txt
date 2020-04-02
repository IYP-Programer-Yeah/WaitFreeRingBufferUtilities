project(WaitFreeRingBufferUtilities)

if(WIN32)
    set(INCLUDE_FOLDER_NAME Include)
else()
    set(INCLUDE_FOLDER_NAME include)
endif()

set(LOCAL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${INCLUDE_FOLDER_NAME})

add_custom_target(${PROJECT_NAME}_HEADERS ALL
    ${CMAKE_COMMAND} -E make_directory ${LOCAL_INCLUDE_DIR}/Iyp/${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${REPO_ROOT}/${PROJECT_NAME}/Include/ ${LOCAL_INCLUDE_DIR}/Iyp/${PROJECT_NAME})

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${LOCAL_INCLUDE_DIR})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_HEADERS)
target_link_libraries(${PROJECT_NAME} INTERFACE Boost::optional)

install(DIRECTORY ${REPO_ROOT}/${PROJECT_NAME}/Include/ DESTINATION ${INCLUDE_FOLDER_NAME}/Iyp/${PROJECT_NAME})
