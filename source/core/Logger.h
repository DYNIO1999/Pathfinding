#ifndef _CORE_LOGGER_H_
#define _CORE_LOGGER_H_

#include <memory>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace VulkanPathfinding
{
    class Logger
    {
    public:
        static void Init();
        static std::shared_ptr<spdlog::logger> GetLogger()
        {

            return s_logger;
        }

    private:
        static std::shared_ptr<spdlog::logger> s_logger;
    };
}
// Core Logging Macros
#define ERROR(exp, msg) assert(((void)msg, exp))

#define APP_TRACE(...) VulkanPathfinding::Logger::GetLogger()->trace(__VA_ARGS__)
#define APP_INFO(...) VulkanPathfinding::Logger::GetLogger()->info(__VA_ARGS__)
#define APP_WARN(...) VulkanPathfinding::Logger::GetLogger()->warn(__VA_ARGS__)
#define APP_ERROR(...) VulkanPathfinding::Logger::GetLogger()->error(__VA_ARGS__)
#endif
