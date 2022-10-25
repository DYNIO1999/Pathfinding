#include "Logger.h"

namespace VulkanPathfinding
{
    std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
    void Logger::Init()
    {
        auto consoleSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        consoleSink->set_pattern("%^[%T] %n: %v%$");
        std::vector<spdlog::sink_ptr> sinks{consoleSink};
        s_logger = std::make_shared<spdlog::logger>("App", sinks.begin(), sinks.end());
        s_logger->set_level(spdlog::level::trace);
    }
}