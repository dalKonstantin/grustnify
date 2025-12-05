file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp *.hpp)


add_executable(${PROJECT_NAME} ${SOURCES})


target_include_directories(${PROJECT_NAME}
      PUBLIC
          ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(${PROJECT_NAME} PUBLIC
  spdlog::spdlog
  Qt6::Widgets
  PkgConfig::FFMPEG
)
