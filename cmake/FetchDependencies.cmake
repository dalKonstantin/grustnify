include(FetchContent)


FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.16.0
)

FetchContent_Declare(
  ffmpeg
  GIT_REPOSITORY https://git.ffmpeg.org/ffmpeg.git
  GIT_TAG master
)


FetchContent_MakeAvailable(
  spdlog
  ffmpeg
)
