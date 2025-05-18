cmake_minimum_required(VERSION 3.14)
project(crow_server)

set(CMAKE_CXX_STANDARD 17)

# Add Asio include directory
set(ASIO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/asio/include")
include_directories(${ASIO_INCLUDE_DIR})

# Define asio::asio target
add_library(asio INTERFACE)
add_library(asio::asio ALIAS asio)
target_include_directories(asio INTERFACE "${ASIO_INCLUDE_DIR}")
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

# Add Crow subdirectory
add_subdirectory(Crow)

# Enable pthread support
find_package(Threads REQUIRED)

# Main source file
add_executable(crow_server src/main.cpp)

# Link Crow and pthreads
target_link_libraries(crow_server PRIVATE Crow::Crow Threads::Threads)
