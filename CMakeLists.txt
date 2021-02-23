cmake_minimum_required(VERSION 3.16)

set(APP_NAME http-server)

project(${APP_NAME})

include(conandir/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB APP_FILES
    main.cpp
    src/listener/listener.cpp
    src/session/session.cpp
    src/common/common.h
)
source_group ("" FILES ${APP_FILES})
add_executable(${APP_NAME} ${APP_FILES})

target_include_directories(${APP_NAME}
    PUBLIC
        ${CMAKE_BINARY_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
        # ${Boost_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(${APP_NAME} ${CONAN_LIBS})