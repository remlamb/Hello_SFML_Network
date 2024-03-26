cmake_minimum_required (VERSION 3.22)
project(MiniGame)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(PORT "12000" CACHE STRING "Set the port")
set(HOST_NAME "localhost" CACHE STRING "Set the host name")

add_definitions("-DPORT=${PORT}")
add_definitions("-DHOST_NAME=\"${HOST_NAME}\"")

find_package(SFML COMPONENTS system window network graphics CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/data)
file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_BINARY_DIR}/release)
file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_BINARY_DIR}/debug)

include_directories(include) 

add_executable(client src/client.cpp "include/gameNetwork.h" "src/gameNetwork.cpp" "include/gameLogic.h" "src/gameLogic.cpp" "include/gameView.h" "src/gameView.cpp")
add_executable(server src/server.cpp "include/gameNetwork.h" "src/gameNetwork.cpp" "include/gameServerLogic.h" "src/gameServerLogic.cpp" "include/lobby.h" "src/lobby.cpp")

target_link_libraries(client PRIVATE sfml-system sfml-window sfml-network sfml-graphics ImGui-SFML::ImGui-SFML)
target_link_libraries(server PRIVATE sfml-system sfml-window sfml-network sfml-graphics ImGui-SFML::ImGui-SFML)
