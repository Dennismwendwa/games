cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(SnakeGame main.cpp)
target_link_libraries(SnakeGame sfml-graphics sfml-window sfml-system)
