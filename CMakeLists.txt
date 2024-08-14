cmake_minimum_required(VERSION 3.12)
project(lserver)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

link_directories(${PROJECT_SOURCE_DIR}/lib)

set(SOURCE_FILES
    src/main.cpp
    src/logger.cpp
    src/core/game.cpp
    src/core/net.cpp
)

add_executable(lserver ${SOURCE_FILES})

target_include_directories(lserver PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(lserver PRIVATE lua dl m nlohmann_json::nlohmann_json)