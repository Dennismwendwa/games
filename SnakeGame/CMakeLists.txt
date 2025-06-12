cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

# Look for SFML
find_package(SFML 2.5 COMPONENTS graphics window audio system REQUIRED)

# Add source files
add_executable(SnakeGame src/main.cpp)

# Link SFML libraries
target_link_libraries(SnakeGame sfml-graphics sfml-window sfml-audio sfml-system)

# Pass asset path as a macro definition to C++
target_compile_definitions(SnakeGame PRIVATE ASSET_PATH="${CMAKE_SOURCE_DIR}/assets")