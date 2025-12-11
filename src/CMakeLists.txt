add_library(grustnify_core STATIC
    core/audio_decoder.cpp
    core/audio_encoder.cpp
    core/audio_buffer.cpp
    log/log.cpp
    ui/main_window.cpp
    app/app.cpp
)

target_include_directories(grustnify_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(grustnify_core
    PUBLIC
        spdlog::spdlog
        Qt6::Widgets
        PkgConfig::FFMPEG
)

# --- executable (macOS bundle + icon / other platforms plain) ---

if(APPLE)
  set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/../assets/icons/AppIcon.icns")
  set(MACOSX_BUNDLE_ICON_FILE "AppIcon.icns")

  add_executable(grustnify
        MACOSX_BUNDLE
        app/main.cpp
        ${APP_ICON_MACOS}
    )

  set_source_files_properties(${APP_ICON_MACOS}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
else()
  add_executable(grustnify
        app/main.cpp
    )
endif()

target_link_libraries(grustnify
    PRIVATE
        grustnify_core
)

install(TARGETS grustnify
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
