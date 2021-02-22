cmake_minimum_required(VERSION 3.16)

set(APP_NAME http-server)

project(${APP_NAME})

include(conandir/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${APP_NAME} main.cpp)

target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(${APP_NAME} ${CONAN_LIBS})