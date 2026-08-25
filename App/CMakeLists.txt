cmake_minimum_required(VERSION 3.16)
project(game LANGUAGES CXX)

file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(game ${GAME_SRC} 
    "${CMAKE_SOURCE_DIR}/Vendor/tracy/public/TracyClient.cpp"
)

target_include_directories(game PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Dawn REQUIRED CONFIG)
target_link_libraries(game PRIVATE
    glfw
    imgui
    spdlog
    glm
    dawn::webgpu_dawn
    Tracy::TracyClient
)

set_target_properties(game PROPERTIES OUTPUT_NAME "App")
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Assets
            $<TARGET_FILE_DIR:game>/assets
)

target_compile_definitions(game PRIVATE
    $<$<CONFIG:Debug>:MC_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:MC_DEBUG>
    $<$<CONFIG:Release>:MC_RELEASE>
    $<$<CONFIG:MinSizeRel>:MC_RELEASE>
)