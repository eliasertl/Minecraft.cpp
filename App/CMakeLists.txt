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

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Dawn REQUIRED CONFIG)
target_link_libraries(game PRIVATE
    glfw
    spdlog
    glm
    dawn::webgpu_dawn
)

set_target_properties(game PROPERTIES OUTPUT_NAME "App")
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Assets
            $<TARGET_FILE_DIR:game>/assets
)