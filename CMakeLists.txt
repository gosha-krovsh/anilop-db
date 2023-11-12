cmake_minimum_required(VERSION 3.20)
project(Anilop)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -fsanitize=address,leak")

add_subdirectory(googletest-master)
include_directories(googletest-master/googletest/include)
include_directories(googletest-master/googlemock/include)

add_executable(Anilop main.cpp)

target_link_libraries(Anilop gtest gtest_main)
