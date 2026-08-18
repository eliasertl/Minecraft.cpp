cmake_minimum_required(VERSION 4.0)
project(Minecraft.cpp)

# ---- Defaults ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Subdirectories ----
add_subdirectory(App)
add_subdirectory(Vendor)