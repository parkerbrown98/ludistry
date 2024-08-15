cmake_minimum_required(VERSION 3.12)
project(lserver)

set(CMAKE_CXX_STANDARD 17)
set(LUA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/lua)
set(LUA_DEST_DIR ${CMAKE_BINARY_DIR}/lua)

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

add_custom_command(
    TARGET lserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
    ${LUA_DEST_DIR}
    COMMENT "Removing old Lua files"
)

add_custom_command(
    TARGET lserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${LUA_SOURCE_DIR}
    ${LUA_DEST_DIR}
    COMMENT "Copying Lua files"
)

target_include_directories(lserver PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(lserver PRIVATE lua dl m nlohmann_json::nlohmann_json)