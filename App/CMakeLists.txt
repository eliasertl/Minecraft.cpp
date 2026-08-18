cmake_minimum_required(VERSION 3.16)
project(game LANGUAGES CXX)

# ---- Sources ----
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# ---- Executable ----
add_executable(game ${GAME_SRC})

# ---- Include directories ----
target_include_directories(game PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# ---- C++ Standard ----
target_compile_features(game PUBLIC cxx_std_23)

target_compile_options(game PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4068>
)

# ---- Output Name ----
set_target_properties(game PROPERTIES OUTPUT_NAME "Minecraft")