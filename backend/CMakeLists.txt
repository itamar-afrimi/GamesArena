# cmake_minimum_required(VERSION 3.13)
# project(test_dynamodb)
# set(CMAKE_CXX_STANDARD 17)

# find_package(AWSSDK REQUIRED COMPONENTS dynamodb)
# add_executable(test_dynamodb src/test_dynamodb.cpp)
# target_link_libraries(test_dynamodb PRIVATE ${AWSSDK_LINK_LIBRARIES})

# cmake_minimum_required(VERSION 3.13)
# project(test_pqxx)
# set(CMAKE_CXX_STANDARD 17)

# add_executable(test_pqxx src/test_pqxx.cpp)
# target_include_directories(test_pqxx PRIVATE /usr/local/include /usr/local/opt/libpq/include)
# target_link_directories(test_pqxx PRIVATE /usr/local/lib /usr/local/opt/libpq/lib)
# target_link_libraries(test_pqxx PRIVATE pqxx pq)


###########################################################
cmake_minimum_required(VERSION 3.14)
project(crow_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

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
add_executable(crow_server
    src/main.cpp
    src/compoments/tic_tac_toe.cpp
    src/compoments/game_manager.cpp
    src/api/ws_game.cpp
    src/api/auth_routes.cpp
    src/api/game_routes.cpp
    src/api/lobby_routes.cpp
    src/models/users.cpp
    src/services/online_service.cpp
    src/services/session_service.cpp
    src/services/user_services.cpp
    src/types.cpp
)
# Enable pthread support
find_package(Threads REQUIRED)
# enable AWS configuration
find_package(AWSSDK REQUIRED COMPONENTS dynamodb)
target_link_libraries(crow_server PRIVATE ${AWSSDK_LINK_LIBRARIES})
# 
target_include_directories(crow_server PRIVATE /usr/local/include /usr/local/opt/libpq/include)
target_link_directories(crow_server PRIVATE /usr/local/lib /usr/local/opt/libpq/lib)
target_link_libraries(crow_server PRIVATE pqxx pq)

# Main source file



# Link Crow and pthreads
target_link_libraries(crow_server PRIVATE Crow::Crow Threads::Threads)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.0 # Or latest stable
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests/test_basic.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
