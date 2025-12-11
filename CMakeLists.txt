cmake_minimum_required(VERSION 4.0)
project(grustnify)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(ENABLE_TESTS "Build tests" OFF)

# Compile commads for lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavcodec libavformat libavutil libswresample libswscale)

include(cmake/FetchDependencies.cmake)
add_subdirectory(src)
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()
