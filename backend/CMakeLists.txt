cmake_minimum_required(VERSION 3.14)
project(crow_server)

set(CMAKE_CXX_STANDARD 17)

# Add Crow subdirectory
add_subdirectory(Crow)

# Main source file
add_executable(crow_server src/main.cpp)

# Link Crow
target_link_libraries(crow_server PRIVATE Crow::Crow)
