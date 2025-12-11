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

add_executable(grustnify
    app/main.cpp
)

target_link_libraries(grustnify
    PRIVATE
        grustnify_core
)
install(TARGETS grustnify
    RUNTIME DESTINATION bin
)
