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
#define VK_CHECK_RESULT(f)             \
    {                                  \
        VkResult res = (f);            \
        if (res != VK_SUCCESS)         \
        {                              \
                                       \
            assert(res == VK_SUCCESS); \
        }                              \
    }
#define CHECK_ERROR(exp, msg) assert(((void)msg, exp))
#define APP_ERROR_VALUE 0

#define APP_TRACE(...) VulkanPathfinding::Logger::GetLogger()->trace(__VA_ARGS__)
#define APP_INFO(...) VulkanPathfinding::Logger::GetLogger()->info(__VA_ARGS__)
#define APP_WARN(...) VulkanPathfinding::Logger::GetLogger()->warn(__VA_ARGS__)
#define APP_ERROR(...) VulkanPathfinding::Logger::GetLogger()->error(__VA_ARGS__)
#endif
