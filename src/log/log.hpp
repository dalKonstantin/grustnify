// log.hpp
#pragma once
#include "spdlog/logger.h"

namespace grustnify {
class Log {
public:
  static void Init();

  inline static std::shared_ptr<spdlog::logger> &GetCoreLogger() {
    return s_core_logger;
  }

  inline static std::shared_ptr<spdlog::logger> &GetClientLogger() {
    return s_client_logger;
  }

private:
  static std::shared_ptr<spdlog::logger> s_core_logger;
  static std::shared_ptr<spdlog::logger> s_client_logger;
};

// core::log::getcorelogger()->warn("init log");

#define TE_CORE_TRACE(...) ::grustnify::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TE_CORE_INFO(...) ::grustnify::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TE_CORE_WARN(...) ::grustnify::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TE_CORE_ERROR(...) ::grustnify::Log::GetCoreLogger()->error(__VA_ARGS__)
#define TE_CORE_CRITICAL(...)                                                  \
  ::grustnify::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define TE_TRACE(...) ::grustnify::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TE_INFO(...) ::grustnify::Log::GetClientLogger()->info(__VA_ARGS__)
#define TE_WARN(...) ::grustnify::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TE_ERROR(...) ::grustnify::Log::GetClientLogger()->error(__VA_ARGS__)
#define TE_CRITICAL(...)                                                       \
  ::grustnify::Log::GetClientLogger()->critical(__VA_ARGS__)
} // namespace grustnify
