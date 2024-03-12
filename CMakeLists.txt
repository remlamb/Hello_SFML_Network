cmake_minimum_required (VERSION 3.18)
project(MiniGame)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(PORT "12000" CACHE STRING "Set the port")
set(HOST_NAME "localhost" CACHE STRING "Set the host name")

add_definitions("-DPORT=${PORT}")
add_definitions("-DHOST_NAME=\"${HOST_NAME}\"")

find_package(SFML COMPONENTS system window network graphics CONFIG REQUIRED)

add_executable(client client.cpp)
add_executable(server server.cpp)

target_link_libraries(client PRIVATE sfml-system sfml-window sfml-network sfml-graphics)
target_link_libraries(server PRIVATE sfml-system sfml-window sfml-network sfml-graphics)
