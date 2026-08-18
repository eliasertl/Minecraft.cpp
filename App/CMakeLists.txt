cmake_minimum_required(VERSION 3.16)
project(game LANGUAGES CXX)

file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(game ${GAME_SRC})

target_include_directories(game PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(game PRIVATE
    glfw
    spdlog
)

set_target_properties(game PROPERTIES OUTPUT_NAME "App")