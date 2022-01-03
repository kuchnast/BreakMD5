cmake_minimum_required(VERSION 2.8)
project(Break_MD5)

set(CMAKE_C_FLAGS "-Wextra -Wall -Werror")

include_directories()

set(LIBS gtest gtest_main)

add_executable(MD5_test MD5_test.c)
target_link_libraries(MD5_test ${LIBS})

find_package(OpenSSL REQUIRED)
target_link_libraries(MD5_test OpenSSL::SSL)

