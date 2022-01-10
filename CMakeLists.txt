cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(BreakMD5)

include_directories(inc/)

set(SOURCES "src/main.cpp")

set(MD5_SOURCES src/md5.cpp)
add_library(MD5 ${MD5_SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror -Wpedantic -Wextra -Wno-unused-parameter -g)
target_link_libraries(${PROJECT_NAME} MD5)