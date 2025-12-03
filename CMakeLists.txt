cmake_minimum_required(VERSION 4.0)
project(grustnify)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



# Compile commads for lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS REQUIRED COMPONENTS Widgets)

include(cmake/FetchDependencies.cmake)
add_subdirectory(src)
